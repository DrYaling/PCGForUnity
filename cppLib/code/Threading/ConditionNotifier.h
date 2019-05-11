#ifndef Condition_notifier_h
#define Condition_notifier_h
#include <functional>
#include <mutex>

namespace threading
{

	typedef std::function<bool(void)> ContionPredicate;
	class ConditionNotifier {
	public:
		ConditionNotifier(ContionPredicate predicate);
		void NotifyOne();
		void NotifyAll();
		void Wait();
	private:
		std::mutex m_mtx;
		std::condition_variable m_condition;
		ContionPredicate m_cbPredicate;
	};


}
#endif
