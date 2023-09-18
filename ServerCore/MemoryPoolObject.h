#pragma once

template <class ALLOC>
class MemoryPoolObject
{
protected:
	using TAllocator = ALLOC;

public:
	MemoryPoolObject()
	{
	}

	virtual ~MemoryPoolObject()
	{
	}

	static inline void* operator new (std::size_t _size)
	{
		void* pAlloc = TAllocator::Allocate(static_cast<int>(_size));
		if (nullptr == pAlloc) throw std::bad_alloc();
		return pAlloc;
	}

	static inline void* operator new (std::size_t _size, const std::nothrow_t& _nothrow_constant) noexcept
	{
		return TAllocator::Allocate(static_cast<int>(_size));
	}

	static inline void* operator new (std::size_t _size, void* _ptr) noexcept
	{
		return _ptr;
	}

	static inline void* operator new[](std::size_t _size)
	{
		void* pAlloc = TAllocator::Allocate(static_cast<int>(_size));
		if (nullptr == pAlloc) throw std::bad_alloc();
		return pAlloc;
	}

	static inline void* operator new[](std::size_t _size, const std::nothrow_t& _nothrow_constant)
	{
		return TAllocator::Allocate(static_cast<int>(_size));
	}

	static inline void operator delete (void* _ptr, void* _pBack) noexcept
	{
	}

	static inline void operator delete (void* _pBack) noexcept
	{
		TAllocator::Deallocate(reinterpret_cast<unsigned char*>(_pBack));
	}

	static inline void operator delete (void* _pBack, const std::nothrow_t& _nothrow_constant) noexcept
	{
		TAllocator::Deallocate(reinterpret_cast<unsigned char*>(_pBack));
	}

	static inline void operator delete[](void* _pBack) noexcept
	{
		TAllocator::Deallocate(reinterpret_cast<unsigned char*>(_pBack));
	}

	static inline void operator delete[](void* _pBack, const std::nothrow_t& _nothrow_constant) noexcept
	{
		TAllocator::Deallocate(reinterpret_cast<unsigned char*>(_pBack));
	}
};
