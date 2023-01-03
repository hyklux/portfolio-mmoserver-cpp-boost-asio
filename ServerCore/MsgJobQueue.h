#pragma once

#include <queue>

#include "MsgJob.h"

class MsgJobQueue
{
public:
	MsgJobQueue()
	{

	}

	~MsgJobQueue()
	{

	}

	bool Init(int32_t threadCnt);
	void Terminate();

protected:
	queue<MsgJob> m_MsgJobQueue;

private:
	bool m_IsActivated = false;
	std::vector<std::unique_ptr<std::thread>> m_ThreadList;
};

