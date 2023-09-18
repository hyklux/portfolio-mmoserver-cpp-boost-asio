#pragma once

// 넷워커 쓰레드(networksinkimpl)와 유저 워커 쓰레드(dispatch_obj_pool) 간 통신을 위해 제작
class DispatchableObjectParam : public MemoryPoolObject<DefaultAllocator>
{
	using TCallType = std::function<void(DispatchableObjectParam*)>;

public:
	using TSharedParam = std::shared_ptr<DispatchableObjectParam>;

	DispatchableObjectParam() : m_Ref(0)
	{
	}

	virtual ~DispatchableObjectParam()
	{
	}

	template <typename T>
	DispatchableObjectParam(T&& _inCall) : m_Call(std::forward<T>(_inCall))
	{
	}

	virtual void operator ()()
	{
		if (m_Call)
		{
			m_Call(this);
		}
	};

	virtual int Load(int _len, const unsigned char* _pchData) = 0;
	
	inline int AddRef()
	{
		return ++m_Ref;
	}

	int Release(void)
	{
		int cnt = --m_Ref;
		if (0 == cnt)
		{
			delete this;
			return 0;
		}

		return cnt;
	}

public:
	inline bool IsCallable() { return nullptr != m_Call; }

public:
	TCallType m_Call;
	std::atomic_int m_Ref = 0;
};

template <typename MSG>
class DispatchableObjectMsgParam : public DispatchableObjectParam
{
	using TMsg = MSG;

public:
	template <typename T>
	DispatchableObjectMsgParam(T&& _inCall) : DispatchableObjectParam(std::forward<T>(_inCall))
	{
	}

	typename pods::Error Load(int _len, const unsigned char* _pchData)
	{
		pods::InputBuffer in(reinterpret_cast<const char*>(_pchData), _len);
		pods::BinaryDeserializer<decltype(in)> deserializer(in);
		return deserializer.load(m_Msg);
	}

	inline TMsg* GetMessage()
	{
		return &m_Msg;
	}

private:
	TMsg m_Msg = TMsg();
};

template <>
class DispatchableObjectMsgParam<void> : public DispatchableObjectParam
{
	using TMsg = void;

public:
	template <typename T>
	DispatchableObjectMsgParam(T&& _inCall) : DispatchableObjectParam(std::forward<T>(_inCall))
	{
	}

	typename pods::Error Load(int _len, const unsigned char* _pchData)
	{
		return pods::Error::NoError;
	}

	inline void* GetMessage()
	{
		return nullptr;
	}

private:
};

class WorkderThreadExt : public std::thread
{
public:
	template <typename CALL>
	WorkderThreadExt(CALL&& _call) : std::thread(std::forward<CALL>(_call))
	{

	}
};

using TEntityId = int64_t;
using WorkerPool = DispatchableObjectPool<TEntityId,DispatchableObjectParam::TSharedParam,concurrency::concurrent_queue,PoolAllocator<>,WorkderThreadExt>;
