#include "ThreadManager.h"
#include "Logger/Logger.h"
namespace threading
{
#define default_thread_pool_size 4
	static ThreadManager* instance = nullptr;
	uint32_t ThreadManager::defaultPoolSize = default_thread_pool_size;
	ThreadManager::ThreadManager() : m_pPool(nullptr)
	{
	}

	ThreadManager::ThreadManager(uint32_t size) : m_pPool(nullptr)
	{
		m_pPool = new ThreadPool(size);
	}
	ThreadManager::~ThreadManager()
	{
		safe_delete(m_pPool);
		LogFormat("Thread Manager Destroyed");
	}

	void ThreadManager::AddTask(const ThreadTask & task) const
	{
		try
		{
			m_pPool->enqueue(task);
		}
		catch (const std::runtime_error& ex)
		{
			LogErrorFormat("try to add thread task fail!%s",ex.what());
		}
	}

	ThreadManager* ThreadManager::GetInstance()
	{
		if (!instance)
		{
			instance = new ThreadManager(defaultPoolSize);
		}
		return instance;
	}
	void ThreadManager::Destroy()
	{
		safe_delete(instance);
	}

	void ThreadManager::SetThreadCount(uint32_t count)
	{
		defaultPoolSize = count;
		if (instance)
		{
			LogErrorFormat("ThreadManager::SetThreadCount should be called before GetInstance or sThreadManager is called!");
		}
	}
}
