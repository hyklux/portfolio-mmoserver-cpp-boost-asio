#pragma once
#include <new>

class DefaultAllocator
{
public:
	DefaultAllocator() {};

	template <class T, class ..._ARGS_T>
	static inline T* Allocate(_ARGS_T&&... _ARGX)
	{
		return new T(std::forward<_ARGS_T>(_ARGX) ...);
	}

	template <class T, class ..._ARGS_T>
	static inline T* Allocate(const char* funcName, _ARGS_T&&... _ARGX)
	{
		return new T(std::forward<_ARGS_T>(_ARGX) ...);
	}

	template <class T, class ..._ARGS_T>
	static inline T* Allocate(const std::nothrow_t&, _ARGS_T&&... _ARGX) noexcept
	{
		return new (std::nothrow) T(std::forward<_ARGS_T>(_ARGX) ...);
	}

	template <class T, class ..._ARGS_T>
	static inline T* Allocate(const char* funcName, const std::nothrow_t&, _ARGS_T&&... _ARGX) noexcept
	{
		return new (std::nothrow) T(std::forward<_ARGS_T>(_ARGX) ...);
	}

	template <class T>
	static inline void Deallocate(T* _pT) noexcept
	{
		if (nullptr == _pT) return;
		delete _pT;
	}

	template <class T>
	static inline T* AllocateArray(int _size)
	{
		return new T[_size];
	}

	template <class T>
	static inline T* AllocateArray(const char* _funcName, int _size)
	{
		return new T[_size];
	}

	template <class T>
	static inline T* AllocateArray(const std::nothrow_t&, int _size)
	{
		return new (std::nothrow) T[_size];
	}

	template <class T>
	static inline T* AllocateArray(const char* _funcName, const std::nothrow_t&, int _size)
	{
		return new (std::nothrow) T[_size];
	}

	template <class T>
	static inline void AllocateArray(int _size, T* _pT) noexcept
	{
		if (nullptr == _pT) return;
		delete[] _pT;
	}

	static inline void* AllocateMemory(int _size) noexcept
	{
		return reinterpret_cast<void*>(::malloc(_size));
	}

	static inline void* AllocateMemory(const char* _funcName, int _size) noexcept
	{
		return reinterpret_cast<void*>(::malloc(_size));
	}

	template <class T>
	static inline void* AllocateMemory(int _size, T _t) noexcept
	{
		return reinterpret_cast<void*>(::malloc(_size));
	}

	template <class T>
	static inline void* Allocate(const char* _funcName, int _size, T _t) noexcept
	{
		return reinterpret_cast<void*>(::malloc(_size));
	}

	static inline void* Reallocate(void* _pT, int /*nSizeForCopy*/, int _size) noexcept
	{
		return reinterpret_cast<void*>(::realloc(_pT, _size));
	}

	static inline void* Reallocate(const char* _funcName, void* _pT, int /*nSizeForCopy*/, int _size) noexcept
	{
		return reinterpret_cast<void*>(::realloc(_pT, _size));
	}

	static inline void Deallocate(void* _pT) noexcept
	{
		if (nullptr == _pT) return;
		::free(_pT);
	}

	virtual ~DefaultAllocator()
	{
	};
};

template<typename T, typename CAllocator>
class STLDefaultAllocator
{
public:
	template<class Other> 
	struct Rebind { typedef STLDefaultAllocator<Other, CAllocator> other; };

	STLDefaultAllocator() throw() {}
	STLDefaultAllocator(const STLDefaultAllocator<T, CAllocator>&) throw() {}

	template<class Other> 
	STLDefaultAllocator(const STLDefaultAllocator<Other, CAllocator>&) throw() {}

	template<class Other> 
	STLDefaultAllocator<T, CAllocator>& operator=(const STLDefaultAllocator<Other, CAllocator>&) { return (*this); }

	inline T* GetAddress(T& _val) const { return (&_val); }
	inline const T* GetAddress(const T& _val) const { return (&_val); }

	inline T* Allocate(size_t _cnt)
	{ 
		return reinterpret_cast<T*>(CAllocator::Allocate(static_cast<int>(_cnt * sizeof(T))));
	}

	inline T* Allocate(size_t _cnt, const void*)
	{ 
		return Allocate(_cnt);
	}

	inline void Deallocate(T* _pT, size_t /*count*/) 
	{ 
		CAllocator::Deallocate(reinterpret_cast<void*>(_pT));
	}

	inline void ConstructAllocate(T* _pT, const T& _val)
	{ 
		new (_pT) T(_val);
	}

	inline void ConstructAllocate(T* _pT, T&& _val)
	{ 
		new (_pT) T(std::move(_val));
	}

	inline void ConstructDeallocate(T* _pT)
	{ 
		(_pT)->~T();
	}

	inline size_t GetMaxSize() const 
	{ 
		size_t cnt = static_cast<size_t>(-1) / sizeof(T); 
		return (0 < cnt ? cnt : 1);
	}
};

template<class T, class Other, class CAllocator>
inline bool operator==(const STLDefaultAllocator<T, CAllocator>&, const STLDefaultAllocator<Other, CAllocator>&)
{
	return true;
}

template<class T, class Other, class CAllocator>
inline bool operator!=(const STLDefaultAllocator<T, CAllocator>&, const STLDefaultAllocator<Other, CAllocator>&)
{
	return false;
}