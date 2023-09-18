#pragma once

// 메모리 메모리풀 관리자를 만든다 mmgr을 이용해서
template <int MINBLOCKSIZE, int MAXBLOCKSIZE, int MINBLOCKCOUNT, int MAXBLOCKCOUNT, int ALIGNMENTSIZE, typename ALLOC>
class MemoryPoolManager : MemoryManager<ALLOC, ALIGNMENTSIZE>
{
	using TParent = MemoryManager<ALLOC, ALIGNMENTSIZE>;

public:
	MemoryPoolManager()
	{
		m_bInitFlag = TParent::Init(MINBLOCKSIZE, MAXBLOCKSIZE, MINBLOCKCOUNT, MAXBLOCKCOUNT);
	}

	virtual ~MemoryPoolManager()
	{
		TParent::Uninit();
	}

	inline void* Allocate(int nSize)
	{
		return TParent::Allocate(nSize);
	}

	inline void Deallocate(void* pBack)
	{
		TParent::Deallocate(pBack);
	}

	inline bool CheckMemory()
	{
		return TParent::CheckMemory();
	}

	inline bool GetFreeStatus()
	{
		return TParent::GetAllocationCount() == 0;
	}

	inline int GetAllocationCount()
	{
		return TParent::GetAllocationCount();
	}

	inline int GetCorruptCount()
	{
		return TParent::GetCorruptCount();
	}

	inline int GetFreeCount()
	{
		return TParent::GetFreeCount();
	}

	inline bool DEBUG_Trace(char** buffer, int& len)
	{
		return TParent::DEBUG_Trace(buffer, len);
	}

public:
	inline bool GetInitFlag() const
	{
		return m_bInitFlag;
	}

private:
	bool m_bInitFlag;
};

// 의의 메모리 관리자를 이용해서... custom allocator를 만든다
template <int MINBLOCKSIZE = 32, int MAXBLOCKSIZE = 65536, int MINBLOCKCOUNT = 2, int MAXBLOCKCOUNT = 64, int ALIGNMENTSIZE = ALIGNMENT_DEFAULT_SIZE, typename ALLOC = DefaultAllocator >
struct PoolAllocator
{
public:
	PoolAllocator()
	{
	}

	virtual ~PoolAllocator()
	{
	}

	template <class T, class ..._ARGS_T>
	static T* Allocate(_ARGS_T&&... _ARGX)
	{
		void* pObject = m_cPoolMngr.Allocate(sizeof(T));
		if (pObject)
		{
			new (pObject) T(std::forward<_ARGS_T>(_ARGX) ...);
		}

		return reinterpret_cast<T*>(pObject);
	}

	template <class T>
	static void Deallocate(T* _pT)
	{
		if (nullptr == _pT) return;
		_pT->~T();
		m_MemoryPoolManager.Deallocate(reinterpret_cast<void*>(_pT));
	}

	template <class T>
	static T* AllocateArray(int _size)
	{
		T* pObject = reinterpret_cast<T*>(m_MemoryPoolManager.Allocate(sizeof(T) * _size));
		if (nullptr == pObject) return nullptr;

		for (int nStep = 0; nStep < _size; nStep++)
		{
			new (pObject + nStep) T;
		}
		return reinterpret_cast<T*>(pObject);
	}

	template <class T>
	static void DeallocateArray(int _size, T* _pT)
	{
		if (nullptr == _pT)
		{
			return;
		}

		for (int nStep = 0; nStep < _size; nStep++)
		{
			_pT[nStep].~T();
		}

		m_MemoryPoolManager.Deallocate(reinterpret_cast<void*>(_pT));
	}

	static void* Allocate(int _size)
	{
		void* pObject = m_MemoryPoolManager.Allocate(_size);
		return reinterpret_cast<void*>(pObject);
	}

	template <class T>
	static void* Allocate(int _size, T _t)
	{
		void* pObject = m_MemoryPoolManager.Allocate(_size);
		return reinterpret_cast<void*>(pObject);
	}

	static void* Reallocate(void* _pT, int _copySize, int _size)
	{
		// 새로운 사이즈가 0일경우는 기존 버퍼를 해제한다.
		if (0 == nSize)
		{
			m_MemoryPoolManager.Deallocate(_pT);
			return nullptr;
		}

		void* pObject = m_cPoolMngr.Allocate(nSize);
		if (pObject && _pT)
		{
			memcpy(pObject, _pT, std::min(_copySize, _size));
			m_MemoryPoolManager.Deallocate(_pT);
		}

		return reinterpret_cast<void*>(pObject);
	}

	static void Deallocate(void* _pT)
	{
		if (nullptr == _pT) return;
		m_MemoryPoolManager.Deallocate(_pT);
	}

	static bool DEBUG_Trace(char** _buffer, int& _len)
	{
		return m_MemoryPoolManager.DEBUG_Trace(_buffer, _len);
	}

	static bool CheckMemory()
	{
		if (false == m_MemoryPoolManager.CheckMemory())
		{
			int32_t* p = nullptr;
			*p = 10;
		}

		return true;
	}

private:
	static MemoryPoolManager<MINBLOCKSIZE, MAXBLOCKSIZE, MINBLOCKCOUNT, MAXBLOCKCOUNT, ALIGNMENTSIZE, ALLOC> m_cPoolMngr;
};

template <int MINBLOCKSIZE, int MAXBLOCKSIZE, int MINBLOCKCOUNT, int MAXBLOCKCOUNT, int ALIGNMENTSIZE, typename ALLOC>
MemoryPoolManager<MINBLOCKSIZE, MAXBLOCKSIZE, MINBLOCKCOUNT, MAXBLOCKCOUNT, ALIGNMENTSIZE, ALLOC> PoolAllocator<MINBLOCKSIZE, MAXBLOCKSIZE, MINBLOCKCOUNT, MAXBLOCKCOUNT, ALIGNMENTSIZE, ALLOC>::m_MemoryPoolManager;