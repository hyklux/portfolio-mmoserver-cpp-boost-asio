#pragma once

class LockSlot
{
public:
	LockSlot()
	{
		m_SlotSize = 0;
		m_Locks = nullptr;
	}

	~LockSlot()
	{
		if (m_Locks) delete[] m_Locks;
	}

	bool Init(int32_t _slotSize)
	{
		_slotSize = std::max<int32_t>(1, _slotSize);

		m_Locks = new std::mutex[_slotSize];
		if (nullptr == m_Locks) return false;
		
		m_SlotSize = _slotSize;
		return true;
	}

	void Uninit()
	{
		if (m_Locks)
		{
			delete[] m_Locks;
			m_Locks = nullptr;
		}
		m_SlotSize = 0;
	}

	inline void Lock(int32_t _value)
	{
		m_Locks[_value % m_SlotSize].lock();
	}

	inline void Unlock(int32_t _value)
	{
		m_Locks[_value % m_SlotSize].unlock();
	}

private:
	int32_t m_SlotSize;
	std::mutex* m_Locks;
};

// 상수 정의
enum
{
	MMGR_POOLINDEX_NULL = (-1),
	ALIGNMENT_DEFAULT_SIZE = (sizeof(void*)),
	CHUNK_DEFAULT_SIZE = 32768,
	CHUNK_MAX_SIZE = 65536,
};

inline int GET_OFFSETCOUNT(int c, int b)
{
	return (((c)+(b)-1) / (b));
}

inline int GET_ALIGNMENT_BY_SIZE(int c, int a)
{
	return (((c)+(a)-1) - (((c)+(a)-1) % (a)));
}

inline int CHECK_POWOFTWO(int c)
{
	return (!((c) & ((c)-1)));
}

template <class ALLOC, int ALIGNMENT = ALIGNMENT_DEFAULT_SIZE>
class ChunkMemory
{
	typedef unsigned char CHUNK_TYPE;
	typedef CHUNK_TYPE CHUNK_HEAD_TYPE;

	enum
	{
		MNGR_CHUNK_ALIGNMENT = ALIGNMENT,
		MNGR_CHUNK_TYPE_SIZE = sizeof(CHUNK_TYPE*),
		MNGR_CHUNK_HEAD_TYPE_SIZE = sizeof(CHUNK_HEAD_TYPE*),
	};

public:
	enum CHKMEMORY
	{
		CHKMEMORY_OK,
		CHKMEMORY_NOFOUNDED,
		CHKMEMORY_INVALIDPOINTER,
	};

public:
	ChunkMemory();
	virtual ~ChunkMemory();

	bool Init(int _blockSize, int _blockCount, int _maxBlockCount);
	void Uninit();
	void* Allocate();
	void Deallocate(void* _pBack);
	CHKMEMORY CheckMemory(void* _pBack);
	bool DEBUG_Trace(char* _buffer, int _len);

	inline bool IsInitialize() { return (0 != m_ChunkSize || 0 != m_BlockSize); }
	inline int GetAllocationCount() { return m_AllocCount; }
	inline int GetFreeCount() { return m_FreeCount; }
	inline int GetBlockSize() { return m_BlockSize; }

private:
	bool AddChunk();

private:
	CHUNK_TYPE* m_pAvailableFirstBlock = nullptr;
	CHUNK_HEAD_TYPE* m_pAvailableLaskChunk = nullptr;
	int m_GrIdx = 0;
	int m_GrIdxExt = 0;
	int m_BlockSize = 0;
	int m_ChunkSize = 0;
	int m_MaxChunkSize = 0;
	int m_ChunkHeadSize = 0;
	int m_AllocCount = 0;
	int m_FreeCount = 0;
};

template <class ALLOC, int ALIGNMENT>
ChunkMemory<ALLOC, ALIGNMENT>::ChunkMemory()
{
	m_pAvailableFirstBlock = nullptr;
	m_pAvailableLaskChunk = nullptr;
	m_GrIdx = 0;
	m_GrIdxExt = 0;
	m_ChunkSize = 0;
	m_MaxChunkSize = 0;
	m_BlockSize = 0;
	m_ChunkHeadSize = 0;
	m_AllocCount = 0;
	m_FreeCount = 0;
}

template <class ALLOC, int ALIGNMENT>
ChunkMemory<ALLOC, ALIGNMENT>::~ChunkMemory()
{
	CHUNK_TYPE** ppFreeChunk = reinterpret_cast<CHUNK_TYPE**>(m_pAvailableLaskChunk);
	CHUNK_TYPE* pNextChunk;

	while (ppFreeChunk)
	{
		pNextChunk = *ppFreeChunk;
		ALLOC::DestroyMemory(reinterpret_cast<CHUNK_TYPE*>(ppFreeChunk));
		ppFreeChunk = reinterpret_cast<CHUNK_TYPE**>(pNextChunk);
	}
}

template <class ALLOC, int ALIGNMENT>
bool ChunkMemory<ALLOC, ALIGNMENT>::Init(int _rawBlockSize, int _blockCount, int _maxBlockCount)
{
	if (0 >= _rawBlockSize) return false;

	int blockSize = GET_ALIGNMENT_BY_SIZE(_rawBlockSize, MNGR_CHUNK_ALIGNMENT);

	// 기본 사이즈보다 작다면.. 확보하자..
	if (blockSize < ALIGNMENT_DEFAULT_SIZE) blockSize = ALIGNMENT_DEFAULT_SIZE;

	m_ChunkSize = blockSize * _blockCount;
	m_MaxChunkSize = blockSize * _maxBlockCount;
	m_BlockSize = blockSize;
	m_ChunkHeadSize = GET_ALIGNMENT_BY_SIZE(MNGR_CHUNK_HEAD_TYPE_SIZE, MNGR_CHUNK_ALIGNMENT);

	return true;
}

template <class ALLOC, int ALIGNMENT>
void ChunkMemory<ALLOC, ALIGNMENT>::Uninit()
{
}

template <class ALLOC, int ALIGNMENT>
bool ChunkMemory<ALLOC, ALIGNMENT>::AddChunk()
{
	int newAllocSize = m_ChunkHeadSize + (m_ChunkSize << m_GrIdx);

	CHUNK_HEAD_TYPE** ppNewChunk = reinterpret_cast<CHUNK_HEAD_TYPE**>(ALLOC::AllocateMemory(newAllocSize));
	if (nullptr == ppNewChunk) return false;

	if (nullptr != m_pAvailableLaskChunk)
	{
		*ppNewChunk = m_pAvailableLaskChunk;
	}
	else
	{
		*ppNewChunk = nullptr;
	}

	m_pAvailableLaskChunk = reinterpret_cast<CHUNK_HEAD_TYPE*>(ppNewChunk);
	m_pAvailableFirstBlock = reinterpret_cast<CHUNK_TYPE*>(reinterpret_cast<unsigned char*>(m_pAvailableLaskChunk) + m_ChunkHeadSize);
	CHUNK_TYPE* pAvailableLastBlock = reinterpret_cast<CHUNK_TYPE*>(reinterpret_cast<unsigned char*>(m_pAvailableLaskChunk) + newAllocSize - m_BlockSize);
	CHUNK_TYPE* pNextBlock = m_pAvailableFirstBlock;
	CHUNK_TYPE** ppCurBlock = reinterpret_cast<CHUNK_TYPE**>(m_pAvailableFirstBlock);

	for (; pNextBlock < pAvailableLastBlock;)
	{
		pNextBlock = reinterpret_cast<CHUNK_TYPE*>(reinterpret_cast<unsigned char*>(pNextBlock) + m_BlockSize);
		*ppCurBlock = pNextBlock;
		ppCurBlock = reinterpret_cast<CHUNK_TYPE**>(pNextBlock);
	}

	m_FreeCount += (newAllocSize - m_ChunkHeadSize) / m_BlockSize;
	*ppCurBlock = nullptr;

	int nextNewAllocSize = m_nChunkHeadSize + (m_nChunkSize << (m_nGrIdx + 1));
	if (m_nMaxChunkSize < nextNewAllocSize || 0 > nextNewAllocSize)
	{
		m_GrIdxExt++;
		return true;
	}

	m_GrIdx++;
	return true;
}

template <class ALLOC, int ALIGNMENT>
void* ChunkMemory<ALLOC, ALIGNMENT>::Allocate()
{
	if (nullptr == m_pAvailableFirstBlock)
	{
		if (false == AddChunk()) return nullptr;
	}

	m_AllocCount++;
	m_FreeCount--;

	CHUNK_TYPE** ppNextBlock = reinterpret_cast<CHUNK_TYPE**>(m_pAvailableFirstBlock);
	m_pAvailableFirstBlock = *ppNextBlock;

	return reinterpret_cast<void*>(ppNextBlock);
}

template <class ALLOC, int ALIGNMENT>
void ChunkMemory<ALLOC, ALIGNMENT>::Deallocate(void* _pBack)
{
	if (nullptr == _pBack) return;

	* (reinterpret_cast<CHUNK_TYPE**>(_pBack)) = m_pAvailableFirstBlock;
	m_pAvailableFirstBlock = reinterpret_cast<CHUNK_TYPE*>(_pBack);

	m_AllocCount--;
	m_FreeCount++;
}

template <class ALLOC, int ALIGNMENT>
bool ChunkMemory<ALLOC, ALIGNMENT>::DEBUG_Trace(char* _buffer, int _len)
{
	if (nullptr == _buffer || _len < 28) return false;

	int gridx = m_GrIdx;
	int gridxExt = m_GrIdxExt;
	int chunkSize = m_ChunkSize;
	int blockSize = m_BlockSize;
	int chunkHeadSize = m_ChunkHeadSize;
	int allockCount = m_AllocCount;
	int freeCount = m_FreeCount;

	int offset = 0;
	memcpy(buffer + offset, &gridx, sizeof(gridx)); offset += sizeof(gridx);
	memcpy(buffer + offset, &gridxExt, sizeof(gridxExt)); offset += sizeof(gridxExt);
	memcpy(buffer + offset, &chunkSize, sizeof(chunkSize)); offset += sizeof(chunkSize);
	memcpy(buffer + offset, &blockSize, sizeof(blockSize)); offset += sizeof(blockSize);
	memcpy(buffer + offset, &chunkHeadSize, sizeof(chunkHeadSize)); offset += sizeof(chunkHeadSize);
	memcpy(buffer + offset, &allockCount, sizeof(allockCount)); offset += sizeof(allockCount);
	memcpy(buffer + offset, &freeCount, sizeof(freeCount)); offset += sizeof(freeCount);

	return true;
}

template <class ALLOC, int ALIGNMENT>
typename ChunkMemory<ALLOC, ALIGNMENT>::CHKMEMORY
ChunkMemory<ALLOC, ALIGNMENT>::CheckMemory(void* _pBack)
{
	int gridx = m_GrIdx;
	int gridxExt = m_GrIdxExt;

	CHUNK_TYPE** ppFreeChunk = reinterpret_cast<CHUNK_TYPE**>(m_pAvailableLaskChunk);
	CHUNK_TYPE* pNextChunk;
	CHUNK_TYPE* pHead;
	CHUNK_TYPE* pTail;

	while (ppFreeChunk)
	{
		// gridxExt는 maxblocksize이상으로 할당하고자 하는 메모리 블럭이 커지는 것을 막았기 떄문에...
		// m_GrIdx이것으로는 사이즈를 표현할 수 없게 되었다.. gridxExt를 사용하여 gridx의 사이즈를 표현함과 동시에 동일 사이즈의 갯수를 파악하도록 했다
		if (0 < gridxExt)
		{
			gridxExt--;
		}
		else
		{
			gridx--;
		}

		pNextChunk = *ppFreeChunk;
		pHead = reinterpret_cast<CHUNK_TYPE*>(ppFreeChunk) + m_ChunkHeadSize;
		pTail = reinterpret_cast<CHUNK_TYPE*>(reinterpret_cast<CHUNK_TYPE*>(ppFreeChunk) + m_ChunkHeadSize + (m_nChunkSize << gridx));

		if (reinterpret_cast<CHUNK_TYPE*>(pHead) <= reinterpret_cast<CHUNK_TYPE*>(_pBack) && reinterpret_cast<CHUNK_TYPE*>(pTail) > reinterpret_cast<CHUNK_TYPE*>(_pBack))
		{
			if (0 != ((reinterpret_cast<CHUNK_TYPE*>(_pBack) - reinterpret_cast<CHUNK_TYPE*>(pHead)) % m_blockSize)) return CHKMEMORY_INVALIDPOINTER;
			return CHKMEMORY_OK;
		}

		ppFreeChunk = reinterpret_cast<CHUNK_TYPE**>(pNextChunk);
	}

	return CHKMEMORY_NOFOUNDED;
}

template <class ALLOC, int ALIGNMENT = ALIGNMENT_DEFAULT_SIZE>
class MemoryManager
{
	typedef int MNGR_HEAD_TYPE;

	enum
	{
		MNGR_ALIGNMENT = ALIGNMENT,
		MNGR_HEAD_TYPE_SIZE = sizeof(MNGR_HEAD_TYPE)
	};

public:
	typedef ChunkMemory<ALLOC, ALIGNMENT> _MEM_CHUNK_T;
	typedef typename _MEM_CHUNK_T::CHKMEMORY _CHKMEMORY_T;

public:
	MemoryManager();
	virtual ~MemoryManager();

	bool Init(int _minObjAlignmentSize, int _maxObjAlignmentSize, int _minExtansionBlockCount, int _maxExtansionBlockCount);
	void Uninit();
	void* Allocate(int _size);
	void Deallocate(void* _pBack);
	bool CheckMemory();
	typename _CHKMEMORY_T CheckMemory(void* _pBack);
	int GetAllocationCount();
	int GetFreeCount();
	bool DEBUG_Trace(char** _buffer, int& _len);

public:
	inline bool IsInitialize() { return (0 != m_MinObjAlignmentSize || 0 != m_ChunkObjectCount); }
	inline int GetCorruptCount() { return m_CorruptCount; }

private:
	int m_MinObjAlignmentSize;
	int m_ChunkObjectCount;
	int m_MngrHeadSize;
	int m_CorruptCount;
	lock_slot m_LockSlot;
	_MEM_CHUNK_T* m_pChunk;
};

template <class ALLOC, int ALIGNMENT>
MemoryManager<ALLOC, ALIGNMENT>::MemoryManager()
{
	m_MinObjAlignmentSize = 0;
	m_ChunkObjectCount = 0;
	m_MngrHeadSize = 0;
	m_CorruptCount = 0;
	m_pChunk = nullptr;
}

template <class ALLOC, int ALIGNMENT>
MemoryManager<ALLOC, ALIGNMENT>::~MemoryManager()
{
	if (m_pChunk)
	{
		ALLOC::template DeallocateArray<ChunkMemory<ALLOC, ALIGNMENT>>(m_ChunkObjectCount, m_pChunk);
	}
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManager<ALLOC, ALIGNMENT>::Init(int _rawMinObjAlignmentSize, int _rawMaxObjAlignmentSize, int _minExtensionBlockCount, int _maxExtensionBlockCount)
{
	// 뭔가 초기화 후 재 초기화가 되는 걸까 ?
	if (0 != m_ChunkObjectCount) return false;

	// 움.. 아무래도 헤더 사이즈를 넣어서 만드는게 이치에 맞는듯 해서..
	int minObjAlignmentSize = GET_ALIGNMENT_BY_SIZE(_rawMinObjAlignmentSize + MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	int maxObjAlignmentSize = GET_ALIGNMENT_BY_SIZE(_rawMaxObjAlignmentSize + MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);

	int cnt = GET_OFFSETCOUNT(maxObjAlignmentSize, minObjAlignmentSize);
	if (0 >= cnt) return false;

	// lock의 수를 1/64 정도로 줄이자
	if (false == m_lockSlot.Init(cnt >> 6)) return false;

	m_pChunk = ALLOC::template AllocateArray<ChunkMemory<ALLOC, ALIGNMENT>>(cnt);
	if (nullptr == m_pChunk) return false;

	for (int nStep = 0; nStep < cnt; nStep++)
	{
		if (false == m_pChunk[nStep].Init((nStep + 1) * minObjAlignmentSize, _minExtensionBlockCount, _maxExtensionBlockCount)) return false;
	}

	m_MinObjAlignmentSize = minObjAlignmentSize;
	m_MngrHeadSize = GET_ALIGNMENT_BY_SIZE(MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	m_ChunkObjectCount = cnt;

	return true;
}

template <class ALLOC, int ALIGNMENT>
void MemoryManager<ALLOC, ALIGNMENT>::Uninit()
{
	for (int nStep = 0; nStep < m_ChunkObjectCount; nStep++)
	{
		m_pChunk[nStep].Uninit();
	}

	m_LockSlot.Uninit();
}

template <class ALLOC, int ALIGNMENT>
void* MemoryManager<ALLOC, ALIGNMENT>::Allocate(int _size)
{
	if (0 == m_ChunkObjectCount) return nullptr;
	if (0 > _size) return nullptr;
	
	if (0 == _size)
	{
		_size = 1;
	}

	int realSize = m_MngrHeadSize + _size;
	int realIndex = GET_OFFSETCOUNT(realSize, m_MinObjAlignmentSize) - 1;
	void* pReturn;

	if (realIndex >= m_ChunkObjectCount)
	{
		pReturn = ALLOC::Allocate(realSize);
		if (nullptr == pReturn) return nullptr;
		*(reinterpret_cast<MNGR_HEAD_TYPE*>(pReturn)) = static_cast<MNGR_HEAD_TYPE>(MMGR_POOLINDEX_NULL);
	}
	else
	{
		if (!(0 <= realIndex && realIndex < m_nChunkObjectCount)) (m_nCorruptCount)++;
		if (!(realSize <= m_pChunk[realIndex].GetBlockSize())) (m_nCorruptCount)++;

		m_LockSlot.Lock(realIndex);

		pReturn = m_pChunk[realIndex].Allocate();
		if (nullptr == pReturn)
		{
			m_LockSlot.Unlock(realIndex);
			return nullptr;
		}

		m_LockSlot.Unlock(realIndex);
		*(reinterpret_cast<MNGR_HEAD_TYPE*>(pReturn)) = realIndex;
	}

	return (reinterpret_cast<unsigned char*>(pReturn) + m_nMngrHeadSize);
}

template <class ALLOC, int ALIGNMENT>
void MemoryManager<ALLOC, ALIGNMENT>::Deallocate(void* _pBack)
{
	if (0 == m_ChunkObjectCount) return;
	if (nullptr == _pBack) return;

	void* pOrignLoc = reinterpret_cast<unsigned char*>(_pBack) - m_MngrHeadSize;
	int realIndex = *(reinterpret_cast<MNGR_HEAD_TYPE*>(pOrignLoc));

	if (static_cast<MNGR_HEAD_TYPE>(MMGR_POOLINDEX_NULL) == realIndex)
	{
		ALLOC::Deallocate(reinterpret_cast<unsigned char*>(pOrignLoc));
		return;
	}

	if (!(0 <= realIndex && realIndex < m_ChunkObjectCount))
	{
		(m_CorruptCount)++;
	}

	m_LockSlot.Lock(realIndex);
	m_pChunk[realIndex].Deallocate(pOrignLoc);
	m_LockSlot.Unlock(realIndex);
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManager<ALLOC, ALIGNMENT>::CheckMemory()
{
	return true;
}

template <class ALLOC, int ALIGNMENT>
typename MemoryManager<ALLOC, ALIGNMENT>::_CHKMEMORY_T
MemoryManager<ALLOC, ALIGNMENT>::CheckMemory(void* _pBack)
{
	void* pOrignLoc = reinterpret_cast<unsigned char*>(_pBack) - m_MngrHeadSize;
	int realIndex = *(reinterpret_cast<MNGR_HEAD_TYPE*>(pOrignLoc));

	if (0 <= realIndex && realIndex < m_nChunkObjectCount)
	{
		m_LockSlot.Lock(realIndex);
		bool ret = m_pChunk[realIndex].CheckMemory(pOrignLoc);
		m_LockSlot.Unlock(realIndex);
		return ret;
	}
	else if (MMGR_POOLINDEX_NULL == realIndex)
	{
		return _MEM_CHUNK_T::CHKMEMORY_OK;
	}

	return _MEM_CHUNK_T::CHKMEMORY_NOFOUNDED;
}

template <class ALLOC, int ALIGNMENT>
int MemoryManager<ALLOC, ALIGNMENT>::GetAllocationCount()
{
	int nTotal = 0;

	for (int nStep = 0; nStep < m_ChunkObjectCount; nStep++)
	{
		nTotal += m_pChunk[nStep].GetAllocationCount();
	}

	return nTotal;
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManager<ALLOC, ALIGNMENT>::DEBUG_Trace(char** _buffer, int& _len)
{
	int chunkObjectCount = m_ChunkObjectCount;
	int bufferSize = chunkObjectCount * 28 + sizeof(chunkObjectCount);
	*_buffer = new char[bufferSize];
	memcpy(*_buffer, &chunkObjectCount, sizeof(chunkObjectCount));

	int offset = sizeof(chunkObjectCount);
	for (int nStep = 0; nStep < m_ChunkObjectCount; nStep++)
	{
		m_pChunk[nStep].DEBUG_Trace(*_buffer + offset, bufferSize - offset);
		offset += 28;
	}

	_len = offset;
	return true;
}

template <class ALLOC, int ALIGNMENT>
int MemoryManager<ALLOC, ALIGNMENT>::GetFreeCount()
{
	int nTotal = 0;
	for (int nStep = 0; nStep < m_ChunkObjectCount; nStep++)
	{
		nTotal += m_pChunk[nStep].GetFreeCount();
	}

	return nTotal;
}

template <class ALLOC, int ALIGNMENT = ALIGNMENT_DEFAULT_SIZE>
class MemoryManagerWithoutLock
{
	typedef int MNGR_HEAD_TYPE;

	enum
	{
		MNGR_ALIGNMENT = ALIGNMENT,
		MNGR_HEAD_TYPE_SIZE = sizeof(MNGR_HEAD_TYPE)
	};

public:
	typedef ChunkMemory<ALLOC, ALIGNMENT> _MEM_CHUNK_T;
	typedef typename _MEM_CHUNK_T::CHKMEMORY _CHKMEMORY_T;

public:
	MemoryManagerWithoutLock();
	virtual ~MemoryManagerWithoutLock();

	bool Init(int _minObjAlignmentSize, int _maxObjAlignmentSize, int _minExtensionBlockCount = 16, int _maxExtensionBlockCount = 16);
	void Uninit();
	void* Allocate(int _size);
	void Deallocate(void* _pBack);
	bool CheckMemory();
	typename _CHKMEMORY_T CheckMemory(void* _pBack);
	int GetAllocationCount();
	int GetFreeCount();

	bool DEBUG_Trace(char** _buffer, int& _len);

public:
	inline bool IsInitialize() { return (0 != m_MinObjAlignmentSize || 0 != m_ChunkObjectCount); }
	inline int GetCorruptCount() { return m_CorruptCount; }

private:
	int m_MinObjAlignmentSize;
	int m_ChunkObjectCount;
	int m_MngrHeadSize;
	int m_CorruptCount;
	_MEM_CHUNK_T* m_pChunk;
};

template <class ALLOC, int ALIGNMENT>
MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::MemoryManagerWithoutLock()
{
	m_MinObjAlignmentSize = 0;
	m_ChunkObjectCount = 0;
	m_MngrHeadSize = 0;
	m_CorruptCount = 0;
	m_Chunk = nullptr;
}

template <class ALLOC, int ALIGNMENT>
MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::~MemoryManagerWithoutLock()
{
	if (m_pChunk)
	{
		ALLOC::template DestroyArray<ChunkMemory<ALLOC, ALIGNMENT>>(m_ChunkObjectCount, m_pChunk);
	}
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::Init(int _rawMinObjAlignmentSize, int _rawMaxObjAlignmentSize, int _minExtensionBlockCount, int _maxExtensionBlockCount)
{
	if (0 != m_nChunkObjectCount) return false;

	// 움.. 아무래도 헤더 사이즈를 넣어서 만드는게 이치에 맞는듯 해서..
	int minObjAlignmentSize = GET_ALIGNMENT_BY_SIZE(_rawMinObjAlignmentSize + MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	int maxObjAlignmentSize = GET_ALIGNMENT_BY_SIZE(_rawMaxObjAlignmentSize + MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);

	int nCount = GET_OFFSETCOUNT(maxObjAlignmentSize, minObjAlignmentSize);
	if (0 >= nCount) return false;

	m_pChunk = ALLOC::template CreateArray<ChunkMemory<ALLOC, ALIGNMENT>>(nCount);
	if (nullptr == m_pChunk) return false;

	for (int nStep = 0; nStep < nCount; nStep++)
	{
		if (false == m_pChunk[nStep].Init((nStep + 1) * minObjAlignmentSize, _minExtensionBlockCount, _maxExtensionBlockCount)) return false;
	}

	m_MinObjAlignmentSize = minObjAlignmentSize;
	m_MngrHeadSize = GET_ALIGNMENT_BY_SIZE(MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	m_ChunkObjectCount = nCount;

	return true;
}

template <class ALLOC, int ALIGNMENT>
void MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::Uninit()
{
	for (int nStep = 0; nStep < m_ChunkObjectCount; nStep++)
	{
		m_pChunk[nStep].Uninit();
	}
}

template <class ALLOC, int ALIGNMENT>
void* MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::Allocate(int _size)
{
	if (0 == m_ChunkObjectCount) return nullptr;
	if (0 > _size) return nullptr;

	if (0 == _size)
	{
		_size = 1;
	}

	int realSize = m_MngrHeadSize + _size;
	int realIndex = GET_OFFSETCOUNT(realSize, m_MinObjAlignmentSize) - 1;
	void* pReturn;

	if (realIndex >= m_ChunkObjectCount)
	{
		pReturn = ALLOC::Allocate(realSize);
		if (nullptr == pReturn) return nullptr;

		*(reinterpret_cast<MNGR_HEAD_TYPE*>(pReturn)) = static_cast<MNGR_HEAD_TYPE>(MMGR_POOLINDEX_NULL);
	}
	else
	{
		if (!(0 <= realIndex && realIndex < m_ChunkObjectCount)) (m_CorruptCount)++;
		if (!(realSize <= m_pChunk[realIndex].GetBlockSize())) (m_CorruptCount)++;

		pReturn = m_pChunk[realIndex].Allocate();
		if (nullptr == pReturn) return nullptr;

		*(reinterpret_cast<MNGR_HEAD_TYPE*>(pReturn)) = realIndex;
	}

	return (reinterpret_cast<unsigned char*>(pReturn) + m_MngrHeadSize);
}

template <class ALLOC, int ALIGNMENT>
void MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::Deallocate(void* _pBack)
{
	if (0 == m_ChunkObjectCount) return;
	if (nullptr == _pBack) return;

	void* pOrignLoc = reinterpret_cast<unsigned char*>(_pBack) - m_MngrHeadSize;
	int realIndex = *(reinterpret_cast<MNGR_HEAD_TYPE*>(pOrignLoc));

	if (static_cast<MNGR_HEAD_TYPE>(MMGR_POOLINDEX_NULL) == realIndex)
	{
		ALLOC::Deallocate(reinterpret_cast<unsigned char*>(pOrignLoc));
		return;
	}

	if (!(0 <= realIndex && realIndex < m_ChunkObjectCount))
	{
		(m_CorruptCount)++;
	}

	m_pChunk[realIndex].Deallocate(pOrignLoc);
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::CheckMemory()
{
	return true;
}

template <class ALLOC, int ALIGNMENT>
typename MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::_CHKMEMORY_T
MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::CheckMemory(void* _pBack)
{
	void* pOrignLoc = reinterpret_cast<unsigned char*>(_pBack) - m_MngrHeadSize;
	int realIndex = *(reinterpret_cast<MNGR_HEAD_TYPE*>(pOrignLoc));

	if (0 <= realIndex && realIndex < m_ChunkObjectCount)
	{
		return m_pChunk[realIndex].CheckMemory(pOrignLoc);
	}
	else if (MMGR_POOLINDEX_NULL == realIndex)
	{
		return _MEM_CHUNK_T::CHKMEMORY_OK;
	}

	return _MEM_CHUNK_T::CHKMEMORY_NOFOUNDED;
}

template <class ALLOC, int ALIGNMENT>
int MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::GetAllocationCount()
{
	int nTotal = 0;

	for (int nStep = 0; nStep < m_ChunkObjectCount; nStep++)
	{
		nTotal += m_pChunk[nStep].GetAllocationCount();
	}

	return nTotal;
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::DEBUG_Trace(char** _buffer, int& _len)
{
	int chunkObjectCount = m_ChunkObjectCount;
	int bufferSize = chunkObjectCount * 28 + sizeof(chunkObjectCount);
	*_buffer = new char[bufferSize];
	memcpy(*buffer, &chunkObjectCount, sizeof(chunkObjectCount));

	int offset = sizeof(chunkObjectCount);

	for (int nStep = 0; nStep < m_ChunkObjectCount; nStep++)
	{
		m_pChunk[nStep].DEBUG_Trace(*_buffer + offset, bufferSize - offset);
		offset += 28;
	}

	_len = offset;
	return true;
}

template <class ALLOC, int ALIGNMENT>
int MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::GetFreeCount()
{
	int nTotal = 0;

	for (int nStep = 0; nStep < m_ChunkObjectCount; nStep++)
	{
		nTotal += m_pChunk[nStep].GetFreeCount();
	}

	return nTotal;
}

template <class T, class ALLOC = DefaultAllocator, int t_lMinChunkSize = CHUNK_DEFAULT_SIZE, int t_lMaxChunkSize = CHUNK_MAX_SIZE, int ALIGNMENT = ALIGNMENT_DEFAULT_SIZE>
class MemoryManagerObj
{
public:
	MemoryManagerObj()
	{
	}
	virtual ~MemoryManagerObj()
	{
	}

	static inline void* operator new (size_t _size)
	{
		if (false == GetAllocator().IsInitialize())
		{
			GetAllocator().Init(sizeof(T), t_lMinChunkSize, t_lMaxChunkSize);
		}

#ifdef __MMGR_ARRAY_OBJECT_
		return GetAllocator().Allocate(static_cast<int>(_size));
#else
		return GetAllocator().Allocate();
#endif
	}

	static inline void	operator delete(void* _pBack)
	{
		GetAllocator().Deallocate(_pBack);
	}

#ifdef __MMGR_ARRAY_OBJECT_
	static inline void* operator new [](size_t _size)
	{
		if (false == GetAllocator().IsInitialize())
		{
			GetAllocator().Init(sizeof(T), t_lChunkSize);
		}

		return GetAllocator().Allocate(static_cast<int>(_size));
	}

	static inline void	operator delete[](void* _pBack)
	{
		GetAllocator().Deallocate(_pBack);
	}
#endif

	static inline bool CheckMemory()
	{
		return true;
	}

private:
#ifdef __MMGR_ARRAY_OBJECT_
	static inline MemoryManager<ALLOC, ALIGNMENT>& GetAllocator()
	{
		static MemoryManager<ALLOC, ALIGNMENT> m_cPoolT;
		return m_cPoolT;
	}
#else
	static inline ChunkMemory<ALLOC, ALIGNMENT>& GetAllocator()
	{
		static ChunkMemory<ALLOC, ALIGNMENT> m_cPoolT;
		return m_cPoolT;
	}
#endif
	};
