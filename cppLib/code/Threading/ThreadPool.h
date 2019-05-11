#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include "threading.h"
/*
 *https://github.com/progschj/ThreadPool/blob/master/ThreadPool.h
 */
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "ConditionNotifier.h"
namespace  threading
{
	class ThreadPool {
	public:
		ThreadPool(size_t);
		template<class F, class... Args>
		std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& f, Args&&... args);
		void enqueue(const ThreadTask& task);
		~ThreadPool();
	private:
		bool Predicate() const;
	private:
		// need to keep track of threads so we can join them
		std::vector< std::thread > m_vWorkers;
		// the task queue
		std::queue<ThreadTask> m_qTasks;

		// synchronization
		std::mutex m_mutex;
		ConditionNotifier m_condition;
		bool m_bStop;
	};

	template <class F, class ... Args>
	std::future<typename std::result_of<F(Args...)>::type> ThreadPool::enqueue(F&& f, Args&&... args)
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared< std::packaged_task<return_type()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			// don't allow enqueueing after stopping the pool
			if (m_bStop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			m_qTasks.emplace([task]() { (*task)(); });
		}
		m_condition.NotifyOne();
		return res;
	}
}
#endif
