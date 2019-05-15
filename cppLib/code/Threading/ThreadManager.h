#ifndef thread_manager_h
#define thread_manager_h
#include "ThreadPool.h"
namespace threading
{
	class ThreadManager
	{
	public:
		ThreadManager();
		explicit ThreadManager(uint32_t size);
		~ThreadManager();
		void AddTask(const ThreadTask& task);
		static ThreadManager* GetInstance();
		static void Destroy();
		static void SetThreadCount(uint32_t count);
	private:
		ThreadPool * m_pPool;
		std::mutex m_mtx;
		static uint32_t defaultPoolSize;

	};
}
#define sThreadManager threading::ThreadManager::GetInstance()
#endif