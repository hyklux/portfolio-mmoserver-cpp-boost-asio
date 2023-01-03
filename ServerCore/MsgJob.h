#pragma once

#include <functional>

using CallbackType = std::function<void()>;

class MsgJob
{
public:
	MsgJob(CallbackType&& callback) : m_CallbackType(std::move(callback))
	{

	}

	int AddRef()
	{
		return ++m_ref;
	}

	int Release()
	{
		int count = --m_ref;
		
		if (0 == count)
		{
			delete this;
			return 0;
		}

		return count;
	}

	void Execute()
	{
		if (m_CallbackType != nullptr)
		{
			m_CallbackType();
		}
	}

private:
	std::atomic_int m_refs = 0;
	CallbackType m_CallbackType;
};

