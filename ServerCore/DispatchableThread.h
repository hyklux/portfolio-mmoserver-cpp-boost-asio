#pragma once

// _QUEUE는 thread-safty하여야 한다..
template <typename _PARAMETER, typename _ALLOCATOR, template <typename T = _PARAMETER, typename A = _ALLOCATOR> class _QUEUE, typename THREAD = std::thread>
class DispatchableThread
{
public:
	using TQueueParam = _PARAMETER;

protected:
	using TAllocator = _ALLOCATOR;
	using TQueue = _QUEUE<TQueueParam, TAllocator>;
	using TThread = THREAD;

public:
	DispatchableThread()
	{
		m_bEnd = false;
	}

	virtual ~DispatchableThread() = default;

	bool Init(int _cnt, int _timeout = 0x7FFFFFFF)
	{
		try
		{
			if (false == m_ThreadList.empty()) return false;

			for (int i = 0; i < _cnt; ++i)
			{
				std::unique_ptr<TThread> thr = std::make_unique<TThread>([this, _timeout]()
				{
					while (false == m_bEnd)
					{
						TQueueParam queueParam;
						if (false == m_Queue.try_pop(queueParam))
						{
							PreIdle();

							std::cv_status cv = m_cSync.Wait(_timeout, [this]() -> bool {
								return false == m_Queue.empty() || true == m_bEnd;
							});

							PostIdle(cv);
							continue;
						}

						this->Pop(std::move(queueParam));
					}
				});

				m_ThreadList.push_back(std::move(thr));
			}
		}
		catch (std::exception&)
		{
			return false;
		}

		return true;
	}

	bool DoEnd()
	{
		m_bEnd = true;
		m_cSync.SetNotificationAll();
		return true;
	}

	void Uninit()
	{
		for (auto& itr : m_ThreadList)
		{
			if (true == (*itr).joinable())
			{
				(*itr).join();
			}
		}
	}

	inline bool Push(TQueueParam&& _queueParam)
	{
		m_Queue.push(std::forward<TQueueParam>(_queueParam));

		m_cSync.SetNotification([this]() -> bool {
			return false == m_Queue.empty();
		});

		return true;
	}

	virtual void Pop(TQueueParam&& _queueParam) = 0;
	virtual void PreIdle() {}
	virtual void PostIdle(std::cv_status _cv) {}
	
	inline bool Empty()
	{
		return m_Queue.empty();
	}

	inline const TQueue* const GetQueue() const
	{
		return &m_Queue;
	}

	inline TThread* GetThisThread()
	{
		auto currId = std::this_thread::get_id();
		for (auto& itr : m_ThreadList)
		{
			if (currId == (*itr).get_id())
			{
				return itr.get();
			}
		}
		return nullptr;
	}

	inline bool IsSameThread()
	{
		auto currId = std::this_thread::get_id();
		for (auto& itr : m_ThreadList)
		{
			if (currId == (*itr).get_id())
			{
				return true;
			}
		}
		return false;
	}

private:
	TQueue m_Queue;
	std::vector<std::unique_ptr<TThread>> m_ThreadList;
	ConditionThread m_cSync;
	bool m_bEnd;
};