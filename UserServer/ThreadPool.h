#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <stdexcept>

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

	void Activate(size_t threadCnt);
	
	// job 을 추가한다.
	void EnqueueJob(std::function<void()> job);

	/*
	template <class F, class... Args>
	std::future<typename std::result_of<F(Args...)>::type> EnqueueJob(F&& f, Args&&... args);
	*/

private:
	// 총 Worker 쓰레드의 개수.
	size_t m_ThreadCnt;
	// Worker 쓰레드를 보관하는 벡터.
	std::vector<std::thread> m_WorkerThreadList;
	// 할일들을 보관하는 job 큐.
	std::queue<std::function<void()>> m_JobQueue;
	// job 큐를 위한 condition variable
	std::condition_variable m_JobQueueCV;
	// job 큐를 위한 mutex
	std::mutex m_JobQueueMutex;
	// 모든 쓰레드 종료
	bool m_StopAll;
	// Worker 쓰레드
	void WorkerThread();
};

