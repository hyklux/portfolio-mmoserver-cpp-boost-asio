#pragma once

template <typename KEY, typename PARAM, template <typename X, typename Y> class QUEUE, typename ALLOC>
class DispatchableBaseObject : public MemoryPoolObject<ALLOC>
{
public:
	using TKey = KEY;
	using TParam = PARAM;
	using TAllocator = ALLOC;
	using TObjJobQueue = QUEUE<PARAM, STLDefaultAllocator<TParam, TAllocator>>;

public:
	DispatchableBaseObject(TKey inKey) : m_Refs(0), m_Key(inKey)
	{
	}

	inline void Push(TParam&& inParam)
	{
		m_Queue.push(std::forward<TParam>(inParam));
	}

	virtual void Pop(TParam& inParam)
	{
	}

	virtual void Process()
	{
		TParam param;
		if (true == m_Queue.try_pop(param))
		{
			Pop(param);
		}
	}

	inline int AddRef()
	{
		return ++m_Refs;
	}

	inline int Release()
	{
		int refs = --m_Refs;
		if (0 == refs)
		{
			delete this;
			return 0;
		}
		return refs;
	}

	inline void Clear()
	{
		m_Queue.clear();
	}

	inline void SetKey(const TKey& inKey)
	{
		m_Key = inKey;
	}

	inline TKey GetKey() const
	{
		return m_Key;
	}

protected:
	inline TObjJobQueue& GetQueueInstance()
	{
		return m_Queue;
	}

private:
	std::atomic_int m_Refs = 0;
	TKey m_Key;
	TObjJobQueue m_Queue;
};

template <typename KEY, typename PARAM, template <typename X, typename Y> class QUEUE, typename ALLOC, typename THREAD = std::thread, typename OBJ = DispatchableBaseObject<KEY, PARAM, QUEUE, ALLOC>, bool IS_OBJ_WITH_QUEUECONTROL = false>
class DispatchableObjectPool
{
public:
	using TKey = KEY;
	using TParam = PARAM;
	using TAllocator = ALLOC;
	using TDispatchableObj = OBJ;
	using TThis = DispatchableObjectPool<KEY, PARAM, QUEUE, ALLOC, THREAD, OBJ, IS_OBJ_WITH_QUEUECONTROL>;

private:
	struct TInternalJob : public MemoryPoolObject<TAllocator>
	{
		TInternalJob(TDispatchableObj* _obj) : m_Refs(0), m_bNewJob(true), m_Obj(_obj)
		{
		}

		inline int AddRef()
		{
			return ++m_Refs;
		}

		inline int Release()
		{
			int refs = --m_Refs;
			if (0 == refs)
			{
				delete this;
				return 0;
			}
			return refs;
		}

		int m_Refs;
		bool m_bNewJob;
		std::shared_ptr<TDispatchableObj> m_Obj;
	};

	using DispatchableThreadWithJob = DispatchableThread<std::shared_ptr<TInternalJob>, STLDefaultAllocator<std::shared_ptr<TInternalJob>, typename TThis::TAllocator>, QUEUE, THREAD>;
	class TWorker : public DispatchableThreadWithJob
	{
	public:
		TWorker(TThis* _pool) : m_Pool(_pool)
		{
			m_WorkCount = 0;
		}

		virtual void Pop(typename DispatchableThreadWithJob::TQueueParam&& _queueParam) override
		{
			_queueParam->m_Obj->Process();

			++m_WorkCount;
			m_Pool->m_Controller.Push(std::forward<TQueueParam>(_queueParam));
		}

		inline auto WorkCount()
		{
			return m_WorkCount;
		}

	private:
		TThis* m_Pool;
		int64_t m_WorkCount;
	};

	class TWorkerWithQueueControl : public DispatchableThreadWithJob
	{
	public:
		TWorkerWithQueueControl(TThis* _pool) : m_Pool(_pool)
		{
			m_WorkCount = 0;
		}

		virtual void Pop(typename DispatchableThreadWithJob::TQueueParam&& _queueParam) override
		{
			m_WorkCount += inQueueParam->m_Obj->Process(std::forward<typename DispatchableThreadWithJob::TQueueParam>(_queueParam), &m_Pool->m_Controller);
		}

		inline auto WorkCount()
		{
			return m_WorkCount;
		}

	private:
		TThis* m_Pool;
		int64_t m_WorkCount;
	};

	class TController : public DispatchableThreadWithJob
	{
		using TContainer = std::unordered_map<TKey, int, std::hash<TKey>, std::equal_to<TKey>, STLDefaultAllocator<std::pair<const TKey, int>, typename TThis::TAllocator>>;

	public:
		TController(TThis* _pool) : m_Pool(_pool)
		{
			m_WorkCount = 0;
		}

		virtual void Pop(typename DispatchableThreadWithJob::TQueueParam&& _queueParam) override
		{
			++m_WorkCount;

			auto itrFind = m_Container.find(_queueParam->m_Obj->GetKey());

			if (true == _queueParam->m_NewJob)
			{
				if (itrFind == m_Container.end())
				{
					auto pairRet = m_Container.emplace(_queueParam->m_Obj->GetKey(), 1);
					if (false == pairRet.second) return;
				}
				else
				{
					itrFind->second++;
					return;
				}

				_queueParam->m_NewJob = false;
			}
			else
			{
				itrFind->second--;

				if (0 >= itrFind->second)
				{
					m_Container.erase(itrFind);
					return;
				}
			}

			m_Pool->m_Workers.Push(std::forward<TQueueParam>(_queueParam));
		}

		inline auto WorkCount()
		{
			return m_WorkCount;
		}

	private:
		TContainer m_Container;
		TThis* m_Pool;
		int64_t m_WorkCount;
	};

	// 내부 함수를 호출하도록
	friend TDispatchableObj;
	friend TController;
	friend TWorker;
	friend TWorkerWithQueueControl;

public:
	DispatchObjectPool() : m_Controller(this), m_Workers(this)
	{
	}

	bool Init(int _threadCount)
	{
		return m_Controller.Init(1) && m_Workers.Init(_threadCount);
	}

	void Uninit()
	{
		m_Workers.DoEnd();
		m_Controller.DoEnd();
		m_Workers.Uninit();
		m_Controller.Uninit();
	}

	bool Push(TDispatchableObj* _obj)
	{
		std::shared_ptr<TInternalJob> newJob = new TInternalJob(_obj);
		if (nullptr == newJob) return false;

		m_Controller.Push(std::move(newJob));
		return true;
	}

	bool Push(TDispatchableObj* _obj, PARAM&& _param)
	{
		std::shared_ptr<TInternalJob> newJob = new TInternalJob(_obj);
		if (nullptr == newJob) return false;

		inObj->Push(std::forward<PARAM>(_param));

		m_Controller.Push(std::move(newJob));
		return true;
	}

	inline bool Empty()
	{
		return m_Controller.Empty() && m_Workers.Empty();
	}

	inline auto WorkerWorkCount()
	{
		return m_Workers.WorkCount();
	}

	inline auto ControllerWorkCount()
	{
		return m_Controller.WorkCount();
	}

	inline THREAD* ThisThr()
	{
		m_Workers.ThisThr();
	}

private:
	TController m_Controller;
	std::conditional_t<IS_OBJ_WITH_QUEUECONTROL, TWorkerWithQueueControl, TWorker> m_Workers;
};