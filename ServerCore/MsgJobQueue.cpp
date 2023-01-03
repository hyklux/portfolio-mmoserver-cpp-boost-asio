#include "pch.h"
#include "MsgJobQueue.h"

bool MsgJobQueue::Init(int32_t threadCnt)
{
	for (int i = 0; i < threadCnt; ++i)
	{
		/*
		std::unique_ptr<_thread_t> thr = std::make_unique<_thread_t>([this, timeout]()
		{
			while (false == m_bEnd)
			{
				_queue_param_t n;
				if (false == m_queue.try_pop(n))
				{
					PreIdle();

					std::cv_status cv = m_cSync.Wait(timeout, [this]() -> bool {
						return !m_queue.empty() || true == m_bEnd;
					});

					PostIdle(cv);
					continue;
				}

				this->Pop(std::move(n));
			}
		});

		m_vecThr.push_back(std::move(thr));
		*/
	}
	m_IsActivated = true;

	return m_IsActivated;
}

void MsgJobQueue::Terminate()
{
	m_IsActivated = false;

	for (auto& itr : m_ThreadList)
	{
		if (true == (*itr).joinable())
		{
			(*itr).join();
		}
	}
}
