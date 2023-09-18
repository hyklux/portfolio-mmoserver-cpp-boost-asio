#pragma once

class ConditionThread : protected std::condition_variable
{
	typedef std::condition_variable TParent;

public:
	ConditionThread()
	{
	}

	virtual ~ConditionThread()
	{
	}

	void SetNotification()
	{
		std::unique_lock<std::mutex> lock(m_Lock);
		TParent::notify_one();
	}

	void SetNotificationAll()
	{
		std::unique_lock<std::mutex> lock(m_Lock);
		TParent::notify_all();
	}

	std::cv_status Wait(int _waitTime)
	{
		std::unique_lock<std::mutex> lock(m_Lock);
		return TParent::wait_for(lock, std::chrono::duration<int, std::milli>(_waitTime));
	}

	template <class T>
	void SetNotification(T& _func)
	{
		std::unique_lock<std::mutex> lock(m_Lock);
		if (true == _func())
		{
			TParent::notify_one();
		}
	}

	template <class T>
	void SetNotificationAll(T& _func)
	{
		std::unique_lock<std::mutex> lock(m_Lock);
		if (true == _func())
		{
			TParent::notify_all();
		}
	}

	template <class T>
	std::cv_status Wait(int _waitTime, T& _func)
	{
		std::unique_lock<std::mutex> lock(m_Lock);
		if (false == _func())
		{
			return TParent::wait_for(lock, std::chrono::duration<int, std::milli>(_waitTime));
		}

		return std::cv_status::no_timeout;
	}

	template <class T>
	void Wait(T& _func)
	{
		std::unique_lock<std::mutex> lock(m_Lock);
		if (false == _func())
		{
			TParent::wait(lock);
		}
	}

private:
	std::mutex m_Lock;
};