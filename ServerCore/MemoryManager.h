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

	bool Init(int nBlockSize, int nBlockCount, int nMaxBlockCount);
	void Uninit();
	void* Allocate();
	void Deallocate(void* pBack);
	CHKMEMORY CheckMemory(void* pBack);
	bool DEBUG_Trace(char* buffer, int len);

	inline bool IsInitialize() { return (0 != m_nChunkSize || 0 != m_nBlockSize); }
	inline int GetAllocationCount() { return m_nAllocCount; }
	inline int GetFreeCount() { return m_nFreeCount; }
	inline int GetBlockSize() { return m_nBlockSize; }

private:
	bool AddChunk();

private:
	CHUNK_TYPE* m_pAvailableFirstBlock = nullptr;
	CHUNK_HEAD_TYPE* m_pAvailableLaskChunk = nullptr;
	int m_nGrIdx = 0;
	int m_nGrIdxExt = 0;
	int m_nBlockSize = 0;
	int m_nChunkSize = 0;
	int m_nMaxChunkSize = 0;
	int m_nChunkHeadSize = 0;
	int m_nAllocCount = 0;
	int m_nFreeCount = 0;
};

template <class ALLOC, int ALIGNMENT>
ChunkMemory<ALLOC, ALIGNMENT>::ChunkMemory()
{
	m_pAvailableFirstBlock = nullptr;
	m_pAvailableLaskChunk = nullptr;
	m_nGrIdx = 0;
	m_nGrIdxExt = 0;
	m_nChunkSize = 0;
	m_nMaxChunkSize = 0;
	m_nBlockSize = 0;
	m_nChunkHeadSize = 0;
	m_nAllocCount = 0;
	m_nFreeCount = 0;
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
bool ChunkMemory<ALLOC, ALIGNMENT>::Init(int nRawBlockSize, int nBlockCount, int nMaxBlockCount)
{
	if (0 >= nRawBlockSize) return false;

	int nBlockSize = GET_ALIGNMENT_BY_SIZE(nRawBlockSize, MNGR_CHUNK_ALIGNMENT);

	// 기본 사이즈보다 작다면.. 확보하자..
	if (nBlockSize < ALIGNMENT_DEFAULT_SIZE) nBlockSize = ALIGNMENT_DEFAULT_SIZE;

	m_nChunkSize = nBlockSize * nBlockCount;
	m_nMaxChunkSize = nBlockSize * nMaxBlockCount;
	m_nBlockSize = nBlockSize;
	m_nChunkHeadSize = GET_ALIGNMENT_BY_SIZE(MNGR_CHUNK_HEAD_TYPE_SIZE, MNGR_CHUNK_ALIGNMENT);

	return true;
}

template <class ALLOC, int ALIGNMENT>
void ChunkMemory<ALLOC, ALIGNMENT>::Uninit()
{
}

template <class ALLOC, int ALIGNMENT>
bool ChunkMemory<ALLOC, ALIGNMENT>::AddChunk()
{
	int nNewAllocSize = m_nChunkHeadSize + (m_nChunkSize << m_nGrIdx);

	CHUNK_HEAD_TYPE** ppNewChunk = reinterpret_cast<CHUNK_HEAD_TYPE**>(ALLOC::AllocateMemory(nNewAllocSize));
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
	m_pAvailableFirstBlock = reinterpret_cast<CHUNK_TYPE*>(reinterpret_cast<unsigned char*>(m_pAvailableLaskChunk) + m_nChunkHeadSize);
	CHUNK_TYPE* pAvailableLastBlock = reinterpret_cast<CHUNK_TYPE*>(reinterpret_cast<unsigned char*>(m_pAvailableLaskChunk) + nNewAllocSize - m_nBlockSize);
	CHUNK_TYPE* pNextBlock = m_pAvailableFirstBlock;
	CHUNK_TYPE** ppCurBlock = reinterpret_cast<CHUNK_TYPE**>(m_pAvailableFirstBlock);

	for (; pNextBlock < pAvailableLastBlock;)
	{
		pNextBlock = reinterpret_cast<CHUNK_TYPE*>(reinterpret_cast<unsigned char*>(pNextBlock) + m_nBlockSize);
		*ppCurBlock = pNextBlock;
		ppCurBlock = reinterpret_cast<CHUNK_TYPE**>(pNextBlock);
	}

	m_nFreeCount += (nNewAllocSize - m_nChunkHeadSize) / m_nBlockSize;
	*ppCurBlock = nullptr;

	int nNextNewAllocSize = m_nChunkHeadSize + (m_nChunkSize << (m_nGrIdx + 1));
	if (m_nMaxChunkSize < nNextNewAllocSize || 0 > nNextNewAllocSize)
	{
		m_nGrIdxExt++;
		return true;
	}

	m_nGrIdx++;
	return true;
}

template <class ALLOC, int ALIGNMENT>
void* ChunkMemory<ALLOC, ALIGNMENT>::Allocate()
{
	if (nullptr == m_pAvailableFirstBlock)
	{
		if (false == AddChunk()) return nullptr;
	}

	m_nAllocCount++;
	m_nFreeCount--;

	CHUNK_TYPE** ppNextBlock = reinterpret_cast<CHUNK_TYPE**>(m_pAvailableFirstBlock);
	m_pAvailableFirstBlock = *ppNextBlock;

	return reinterpret_cast<void*>(ppNextBlock);
}

template <class ALLOC, int ALIGNMENT>
void ChunkMemory<ALLOC, ALIGNMENT>::Deallocate(void* pBack)
{
	if (nullptr == pBack) return;

	* (reinterpret_cast<CHUNK_TYPE**>(pBack)) = m_pAvailableFirstBlock;
	m_pAvailableFirstBlock = reinterpret_cast<CHUNK_TYPE*>(pBack);

	m_nAllocCount--;
	m_nFreeCount++;
}

template <class ALLOC, int ALIGNMENT>
bool ChunkMemory<ALLOC, ALIGNMENT>::DEBUG_Trace(char* buffer, int len)
{
	if (nullptr == buffer || len < 28) return false;

	int nGrIdx = m_nGrIdx;
	int nGridxExt = m_nGrIdxExt;
	int nChunkSize = m_nChunkSize;
	int nBlockSize = m_nBlockSize;
	int nChunkHeadSize = m_nChunkHeadSize;
	int nAllockCount = m_nAllocCount;
	int nFreeCount = m_nFreeCount;

	int offset = 0;
	memcpy(buffer + offset, &nGrIdx, sizeof(nGrIdx)); offset += sizeof(nGrIdx);
	memcpy(buffer + offset, &nGridxExt, sizeof(nGridxExt)); offset += sizeof(nGridxExt);
	memcpy(buffer + offset, &nChunkSize, sizeof(nChunkSize)); offset += sizeof(nChunkSize);
	memcpy(buffer + offset, &nBlockSize, sizeof(nBlockSize)); offset += sizeof(nBlockSize);
	memcpy(buffer + offset, &nChunkHeadSize, sizeof(nChunkHeadSize)); offset += sizeof(nChunkHeadSize);
	memcpy(buffer + offset, &nAllockCount, sizeof(nAllockCount)); offset += sizeof(nAllockCount);
	memcpy(buffer + offset, &nFreeCount, sizeof(nFreeCount)); offset += sizeof(nFreeCount);

	return true;
}

template <class ALLOC, int ALIGNMENT>
typename ChunkMemory<ALLOC, ALIGNMENT>::CHKMEMORY
ChunkMemory<ALLOC, ALIGNMENT>::CheckMemory(void* pBack)
{
	int nGrIdx = m_nGrIdx;
	int nGridxExt = m_nGrIdxExt;

	CHUNK_TYPE** ppFreeChunk = reinterpret_cast<CHUNK_TYPE**>(m_pAvailableLaskChunk);
	CHUNK_TYPE* pNextChunk;
	CHUNK_TYPE* pHead;
	CHUNK_TYPE* pTail;

	while (ppFreeChunk)
	{
		// nGridxExt는 maxblocksize이상으로 할당하고자 하는 메모리 블럭이 커지는 것을 막았기 떄문에...
		// m_nGrIdx이것으로는 사이즈를 표현할 수 없게 되었다.. nGridxExt를 사용하여 nGrIdx의 사이즈를 표현함과 동시에 동일 사이즈의 갯수를 파악하도록 했다
		if (0 < nGridxExt)
		{
			nGridxExt--;
		}
		else
		{
			nGrIdx--;
		}

		pNextChunk = *ppFreeChunk;
		pHead = reinterpret_cast<CHUNK_TYPE*>(ppFreeChunk) + m_nChunkHeadSize;
		pTail = reinterpret_cast<CHUNK_TYPE*>(reinterpret_cast<CHUNK_TYPE*>(ppFreeChunk) + m_nChunkHeadSize + (m_nChunkSize << nGrIdx));

		if (reinterpret_cast<CHUNK_TYPE*>(pHead) <= reinterpret_cast<CHUNK_TYPE*>(pBack) && reinterpret_cast<CHUNK_TYPE*>(pTail) > reinterpret_cast<CHUNK_TYPE*>(pBack))
		{
			if (0 != ((reinterpret_cast<CHUNK_TYPE*>(pBack) - reinterpret_cast<CHUNK_TYPE*>(pHead)) % m_nBlockSize)) return CHKMEMORY_INVALIDPOINTER;
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

	bool Init(int nMinObjAlignmentSize, int nMaxObjAlignmentSize, int nMinExtansionBlockCount, int nMaxExtansionBlockCount);
	void Uninit();
	void* Allocate(int nSize);
	void Deallocate(void* pBack);
	bool CheckMemory();
	typename _CHKMEMORY_T CheckMemory(void* pBack);
	int GetAllocationCount();
	int GetFreeCount();
	bool DEBUG_Trace(char** buffer, int& len);

public:
	inline bool IsInitialize() { return (0 != m_nMinObjAlignmentSize || 0 != m_nChunkObjectCount); }
	inline int GetCorruptCount() { return m_nCorruptCount; }

private:
	int m_nMinObjAlignmentSize;
	int m_nChunkObjectCount;
	int m_nMngrHeadSize;
	int m_nCorruptCount;
	lock_slot m_lockSlot;
	_MEM_CHUNK_T* m_pChunk;
};

template <class ALLOC, int ALIGNMENT>
MemoryManager<ALLOC, ALIGNMENT>::MemoryManager()
{
	m_nMinObjAlignmentSize = 0;
	m_nChunkObjectCount = 0;
	m_nMngrHeadSize = 0;
	m_nCorruptCount = 0;
	m_pChunk = nullptr;
}

template <class ALLOC, int ALIGNMENT>
MemoryManager<ALLOC, ALIGNMENT>::~MemoryManager()
{
	if (m_pChunk)
	{
		ALLOC::template DeallocateArray<ChunkMemory<ALLOC, ALIGNMENT>>(m_nChunkObjectCount, m_pChunk);
	}
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManager<ALLOC, ALIGNMENT>::Init(int nRawMinObjAlignmentSize, int nRawMaxObjAlignmentSize, int nMinExtansionBlockCount, int nMaxExtansionBlockCount)
{
	// 뭔가 초기화 후 재 초기화가 되는 걸까 ?
	if (0 != m_nChunkObjectCount) return false;

	// 움.. 아무래도 헤더 사이즈를 넣어서 만드는게 이치에 맞는듯 해서..
	int nMinObjAlignmentSize = GET_ALIGNMENT_BY_SIZE(nRawMinObjAlignmentSize + MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	//nMinObjAlignmentSize += GET_ALIGNMENT_BY_SIZE(MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	int nMaxObjAlignmentSize = GET_ALIGNMENT_BY_SIZE(nRawMaxObjAlignmentSize + MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	//nMaxObjAlignmentSize += GET_ALIGNMENT_BY_SIZE(MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);

	int nCount = GET_OFFSETCOUNT(nMaxObjAlignmentSize, nMinObjAlignmentSize);
	if (0 >= nCount) return false;

	// lock의 수를 1/64 정도로 줄이자
	if (false == m_lockSlot.Init(nCount >> 6)) return false;

	m_pChunk = ALLOC::template AllocateArray<ChunkMemory<ALLOC, ALIGNMENT>>(nCount);
	if (nullptr == m_pChunk) return false;

	for (int nStep = 0; nStep < nCount; nStep++)
	{
		if (false == m_pChunk[nStep].Init((nStep + 1) * nMinObjAlignmentSize, nMinExtansionBlockCount, nMaxExtansionBlockCount)) return false;
	}

	m_nMinObjAlignmentSize = nMinObjAlignmentSize;
	m_nMngrHeadSize = GET_ALIGNMENT_BY_SIZE(MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	m_nChunkObjectCount = nCount;

	return true;
}

template <class ALLOC, int ALIGNMENT>
void MemoryManager<ALLOC, ALIGNMENT>::Uninit()
{
	for (int nStep = 0; nStep < m_nChunkObjectCount; nStep++)
	{
		m_pChunk[nStep].Uninit();
	}

	m_lockSlot.Uninit();
}

template <class ALLOC, int ALIGNMENT>
void* MemoryManager<ALLOC, ALIGNMENT>::Allocate(int nSize)
{
	if (0 == m_nChunkObjectCount) return nullptr;
	if (0 > nSize) return nullptr;
	
	if (0 == nSize)
	{
		nSize = 1;
	}

	int nRealSize = m_nMngrHeadSize + nSize;
	int nRealIndex = GET_OFFSETCOUNT(nRealSize, m_nMinObjAlignmentSize) - 1;
	void* pReturn;

	if (nRealIndex >= m_nChunkObjectCount)
	{
		pReturn = ALLOC::Allocate(nRealSize);
		if (nullptr == pReturn) return nullptr;
		*(reinterpret_cast<MNGR_HEAD_TYPE*>(pReturn)) = static_cast<MNGR_HEAD_TYPE>(MMGR_POOLINDEX_NULL);
	}
	else
	{
		if (!(0 <= nRealIndex && nRealIndex < m_nChunkObjectCount)) (m_nCorruptCount)++;
		if (!(nRealSize <= m_pChunk[nRealIndex].GetBlockSize())) (m_nCorruptCount)++;

		m_lockSlot.Lock(nRealIndex);

		pReturn = m_pChunk[nRealIndex].Allocate();
		if (nullptr == pReturn)
		{
			m_lockSlot.Unlock(nRealIndex);
			return nullptr;
		}

		m_lockSlot.Unlock(nRealIndex);
		*(reinterpret_cast<MNGR_HEAD_TYPE*>(pReturn)) = nRealIndex;
	}

	return (reinterpret_cast<unsigned char*>(pReturn) + m_nMngrHeadSize);
}

template <class ALLOC, int ALIGNMENT>
void MemoryManager<ALLOC, ALIGNMENT>::Deallocate(void* pBack)
{
	if (0 == m_nChunkObjectCount) return;
	if (nullptr == pBack) return;

	void* pOrignLoc = reinterpret_cast<unsigned char*>(pBack) - m_nMngrHeadSize;
	int nRealIndex = *(reinterpret_cast<MNGR_HEAD_TYPE*>(pOrignLoc));

	if (static_cast<MNGR_HEAD_TYPE>(MMGR_POOLINDEX_NULL) == nRealIndex)
	{
		ALLOC::Deallocate(reinterpret_cast<unsigned char*>(pOrignLoc));
		return;
	}

	if (!(0 <= nRealIndex && nRealIndex < m_nChunkObjectCount))
	{
		(m_nCorruptCount)++;
	}

	m_lockSlot.Lock(nRealIndex);
	m_pChunk[nRealIndex].Deallocate(pOrignLoc);
	m_lockSlot.Unlock(nRealIndex);
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManager<ALLOC, ALIGNMENT>::CheckMemory()
{
	return true;
}

template <class ALLOC, int ALIGNMENT>
typename MemoryManager<ALLOC, ALIGNMENT>::_CHKMEMORY_T
MemoryManager<ALLOC, ALIGNMENT>::CheckMemory(void* pBack)
{
	void* pOrignLoc = reinterpret_cast<unsigned char*>(pBack) - m_nMngrHeadSize;
	int nRealIndex = *(reinterpret_cast<MNGR_HEAD_TYPE*>(pOrignLoc));

	if (0 <= nRealIndex && nRealIndex < m_nChunkObjectCount)
	{
		m_lockSlot.Lock(nRealIndex);
		bool ret = m_pChunk[nRealIndex].CheckMemory(pOrignLoc);
		m_lockSlot.Unlock(nRealIndex);
		return ret;
	}
	else if (MMGR_POOLINDEX_NULL == nRealIndex)
	{
		return _MEM_CHUNK_T::CHKMEMORY_OK;
	}

	return _MEM_CHUNK_T::CHKMEMORY_NOFOUNDED;
}

template <class ALLOC, int ALIGNMENT>
int MemoryManager<ALLOC, ALIGNMENT>::GetAllocationCount()
{
	int nTotal = 0;

	for (int nStep = 0; nStep < m_nChunkObjectCount; nStep++)
	{
		nTotal += m_pChunk[nStep].GetAllocationCount();
	}

	return nTotal;
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManager<ALLOC, ALIGNMENT>::DEBUG_Trace(char** buffer, int& len)
{
	int nChunkObjectCount = m_nChunkObjectCount;
	int bufferSize = nChunkObjectCount * 28 + sizeof(nChunkObjectCount);
	*buffer = new char[bufferSize];
	memcpy(*buffer, &nChunkObjectCount, sizeof(nChunkObjectCount));

	int offset = sizeof(nChunkObjectCount);
	for (int nStep = 0; nStep < m_nChunkObjectCount; nStep++)
	{
		m_pChunk[nStep].DEBUG_Trace(*buffer + offset, bufferSize - offset);
		offset += 28;
	}

	len = offset;
	return true;
}

template <class ALLOC, int ALIGNMENT>
int MemoryManager<ALLOC, ALIGNMENT>::GetFreeCount()
{
	int nTotal = 0;
	for (int nStep = 0; nStep < m_nChunkObjectCount; nStep++)
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

	bool Init(int nMinObjAlignmentSize, int nMaxObjAlignmentSize, int nMinExtansionBlockCount = 16, int nMaxExtansionBlockCount = 16);
	void Uninit();
	void* Allocate(int nSize);
	void Deallocate(void* pBack);
	bool CheckMemory();
	typename _CHKMEMORY_T CheckMemory(void* pBack);
	int GetAllocationCount();
	int GetFreeCount();

	bool DEBUG_Trace(char** buffer, int& len);

public:
	inline bool IsInitialize() { return (0 != m_nMinObjAlignmentSize || 0 != m_nChunkObjectCount); }
	inline int GetCorruptCount() { return m_nCorruptCount; }

private:
	int m_nMinObjAlignmentSize;
	int m_nChunkObjectCount;
	int m_nMngrHeadSize;
	int m_nCorruptCount;
	_MEM_CHUNK_T* m_pChunk;
};

template <class ALLOC, int ALIGNMENT>
MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::MemoryManagerWithoutLock()
{
	m_nMinObjAlignmentSize = 0;
	m_nChunkObjectCount = 0;
	m_nMngrHeadSize = 0;
	m_nCorruptCount = 0;
	m_pChunk = nullptr;
}

template <class ALLOC, int ALIGNMENT>
MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::~MemoryManagerWithoutLock()
{
	if (m_pChunk)
	{
		ALLOC::template DestroyArray<ChunkMemory<ALLOC, ALIGNMENT>>(m_nChunkObjectCount, m_pChunk);
	}
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::Init(int nRawMinObjAlignmentSize, int nRawMaxObjAlignmentSize, int nMinExtansionBlockCount, int nMaxExtansionBlockCount)
{
	if (0 != m_nChunkObjectCount) return false;

	// 움.. 아무래도 헤더 사이즈를 넣어서 만드는게 이치에 맞는듯 해서..
	int nMinObjAlignmentSize = GET_ALIGNMENT_BY_SIZE(nRawMinObjAlignmentSize + MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	//nMinObjAlignmentSize += GET_ALIGNMENT_BY_SIZE(MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	int nMaxObjAlignmentSize = GET_ALIGNMENT_BY_SIZE(nRawMaxObjAlignmentSize + MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	//nMaxObjAlignmentSize += GET_ALIGNMENT_BY_SIZE(MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);

	int nCount = GET_OFFSETCOUNT(nMaxObjAlignmentSize, nMinObjAlignmentSize);
	if (0 >= nCount) return false;

	m_pChunk = ALLOC::template CreateArray<ChunkMemory<ALLOC, ALIGNMENT>>(nCount);
	if (nullptr == m_pChunk) return false;

	for (int nStep = 0; nStep < nCount; nStep++)
	{
		if (false == m_pChunk[nStep].Init((nStep + 1) * nMinObjAlignmentSize, nMinExtansionBlockCount, nMaxExtansionBlockCount)) return false;
	}

	m_nMinObjAlignmentSize = nMinObjAlignmentSize;
	m_nMngrHeadSize = GET_ALIGNMENT_BY_SIZE(MNGR_HEAD_TYPE_SIZE, MNGR_ALIGNMENT);
	m_nChunkObjectCount = nCount;

	return true;
}

template <class ALLOC, int ALIGNMENT>
void MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::Uninit()
{
	for (int nStep = 0; nStep < m_nChunkObjectCount; nStep++)
	{
		m_pChunk[nStep].Uninit();
	}
}

template <class ALLOC, int ALIGNMENT>
void* MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::Allocate(int _size)
{
	if (0 == m_nChunkObjectCount) return nullptr;
	if (0 > _size) return nullptr;

	if (0 == _size)
	{
		_size = 1;
	}

	int nRealSize = m_nMngrHeadSize + _size;
	int nRealIndex = GET_OFFSETCOUNT(nRealSize, m_nMinObjAlignmentSize) - 1;
	void* pReturn;

	if (nRealIndex >= m_nChunkObjectCount)
	{
		pReturn = ALLOC::Allocate(nRealSize);
		if (nullptr == pReturn) return nullptr;

		*(reinterpret_cast<MNGR_HEAD_TYPE*>(pReturn)) = static_cast<MNGR_HEAD_TYPE>(MMGR_POOLINDEX_NULL);
	}
	else
	{
		if (!(0 <= nRealIndex && nRealIndex < m_nChunkObjectCount)) (m_nCorruptCount)++;
		if (!(nRealSize <= m_pChunk[nRealIndex].GetBlockSize())) (m_nCorruptCount)++;

		pReturn = m_pChunk[nRealIndex].Allocate();
		if (nullptr == pReturn) return nullptr;

		*(reinterpret_cast<MNGR_HEAD_TYPE*>(pReturn)) = nRealIndex;
	}

	return (reinterpret_cast<unsigned char*>(pReturn) + m_nMngrHeadSize);
}

template <class ALLOC, int ALIGNMENT>
void MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::Deallocate(void* _pBack)
{
	if (0 == m_nChunkObjectCount) return;
	if (nullptr == _pBack) return;

	void* pOrignLoc = reinterpret_cast<unsigned char*>(_pBack) - m_nMngrHeadSize;
	int nRealIndex = *(reinterpret_cast<MNGR_HEAD_TYPE*>(pOrignLoc));

	if (static_cast<MNGR_HEAD_TYPE>(MMGR_POOLINDEX_NULL) == nRealIndex)
	{
		ALLOC::Deallocate(reinterpret_cast<unsigned char*>(pOrignLoc));
		return;
	}

	if (!(0 <= nRealIndex && nRealIndex < m_nChunkObjectCount))
	{
		(m_nCorruptCount)++;
	}

	m_pChunk[nRealIndex].Deallocate(pOrignLoc);
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
	void* pOrignLoc = reinterpret_cast<unsigned char*>(_pBack) - m_nMngrHeadSize;
	int nRealIndex = *(reinterpret_cast<MNGR_HEAD_TYPE*>(pOrignLoc));

	if (0 <= nRealIndex && nRealIndex < m_nChunkObjectCount)
	{
		return m_pChunk[nRealIndex].CheckMemory(pOrignLoc);
	}
	else if (MMGR_POOLINDEX_NULL == nRealIndex)
	{
		return _MEM_CHUNK_T::CHKMEMORY_OK;
	}

	return _MEM_CHUNK_T::CHKMEMORY_NOFOUNDED;
}

template <class ALLOC, int ALIGNMENT>
int MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::GetAllocationCount()
{
	int nTotal = 0;

	for (int nStep = 0; nStep < m_nChunkObjectCount; nStep++)
	{
		nTotal += m_pChunk[nStep].GetAllocationCount();
	}

	return nTotal;
}

template <class ALLOC, int ALIGNMENT>
bool MemoryManagerWithoutLock<ALLOC, ALIGNMENT>::DEBUG_Trace(char** _buffer, int& _len)
{
	int nChunkObjectCount = m_nChunkObjectCount;
	int bufferSize = nChunkObjectCount * 28 + sizeof(nChunkObjectCount);
	*_buffer = new char[bufferSize];
	memcpy(*buffer, &nChunkObjectCount, sizeof(nChunkObjectCount));

	int offset = sizeof(nChunkObjectCount);

	for (int nStep = 0; nStep < m_nChunkObjectCount; nStep++)
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

	for (int nStep = 0; nStep < m_nChunkObjectCount; nStep++)
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
