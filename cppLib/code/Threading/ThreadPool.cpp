#include "ThreadPool.h"
#include "Logger/Logger.h"
#include <sstream>

namespace threading
{
	struct ThreadExitLog
	{
		ThreadExitLog() = delete;
		explicit ThreadExitLog(const std::thread::id& threadId) :m_nthreadId(0)
		{
			std::ostringstream oss;
			oss << threadId;
			std::string stid = oss.str();
			m_nthreadId = std::stoull(stid);
		}
		~ThreadExitLog()
		{
			LogFormat("ThreadPool thread %lld exit", m_nthreadId);
		}
		uint64_t m_nthreadId;
	};
	ThreadPool::ThreadPool(size_t threads) :
		m_condition(std::bind(&ThreadPool::Predicate, this)),
		m_bStop(false)
	{
		LogFormat("ThreadPool start size %d", threads);
		for (size_t i = 0; i < threads; ++i)
		{
			m_vWorkers.emplace_back([this]
			{
				ThreadExitLog threadLog(std::this_thread::get_id());
				for (;;)
				{
					ThreadTask task;

					{
						this->m_condition.Wait();
						if (this->m_bStop && this->m_qTasks.empty())
							return;
						task = std::move(this->m_qTasks.front());
						this->m_qTasks.pop();
					}

					task();
				}
			}
			);
		}
	}

	void ThreadPool::enqueue(const ThreadTask& task)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		// don't allow enqueueing after stopping the pool
		if (m_bStop)
			throw std::runtime_error("enqueue on stopped ThreadPool");
		m_qTasks.emplace(task);
		m_condition.NotifyOne();
	}

	ThreadPool::~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_bStop = true;
		}
		m_condition.NotifyAll();
		for (std::thread &worker : m_vWorkers)
			worker.join();
	}

	bool ThreadPool::Predicate() const
	{
		return this->m_bStop || !this->m_qTasks.empty();
	}
}
