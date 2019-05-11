#ifndef _threading_h
#define _threading_h
#include <functional>
namespace threading
{
	class ThreadTask
	{
	public:
		ThreadTask(const std::function<void()>& func) noexcept :
			_func(func)
		{
		};
		ThreadTask(const ThreadTask& in) noexcept :
			_func(in._func)
		{
		};
		void operator()() const
		{
			if (_func)
			{
				_func();
			}
		}
		~ThreadTask();
		ThreadTask():_func(nullptr)
		{
			
		};
		std::function<void()> _func;
	};

	inline ThreadTask::~ThreadTask()
	{
	}
}
#endif
