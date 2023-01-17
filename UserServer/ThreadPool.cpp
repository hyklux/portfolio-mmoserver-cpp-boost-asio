#include "pch.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool() : m_ThreadCnt(0), m_StopAll(false)
{
}

ThreadPool::~ThreadPool() 
{
	m_StopAll = true;
	m_JobQueueCV.notify_all();

	for (auto& thr : m_WorkerThreadList) 
	{
		thr.join();
	}
}

void ThreadPool::Activate(size_t threadCnt)
{
	m_ThreadCnt = threadCnt;

	m_WorkerThreadList.reserve(m_ThreadCnt);

	for (size_t i = 0; i < m_ThreadCnt; ++i)
	{
		m_WorkerThreadList.emplace_back([this]() { this->WorkerThread(); });
	}
}

void ThreadPool::EnqueueJob(std::function<void()> job) 
{
	if (m_StopAll) 
	{
		return;
	}

	cout << "[ThreadPool] Pushing job into queue..." << endl;

	{
		std::lock_guard<std::mutex> lock(m_JobQueueMutex);
		m_JobQueue.push(std::move(job));
	}

	m_JobQueueCV.notify_one();
}

void ThreadPool::WorkerThread() 
{
	while (true) 
	{
		std::unique_lock<std::mutex> lock(m_JobQueueMutex);
		m_JobQueueCV.wait(lock, [this]() 
		{ 
			return !this->m_JobQueue.empty() || m_StopAll; 
		});
		
		if (m_StopAll && this->m_JobQueue.empty()) 
		{
			return;
		}

		// 맨 앞의 job 을 뺀다.
		std::function<void()> job = std::move(m_JobQueue.front());
		m_JobQueue.pop();
		lock.unlock();

		cout << "[ThreadPool] Pushing job out of queue..." << endl;

		// 해당 job 을 수행한다
		job();
	}
}

/*
template <class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::EnqueueJob(F&& f, Args&&... args)
{
	if (m_StopAll) 
	{
		return;
	}

	using return_type = typename std::result_of<F(Args...)>::type;
	auto job = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
	
	std::future<return_type> jobResultFuture = job->get_future();
	{
		std::lock_guard<std::mutex> lock(m_JobQueueMutex);
		m_JobQueue.push([job]() { (*job)(); });
	}
	m_JobQueueCV.notify_one();

	return jobResultFuture;
}
*/