#include "ConditionNotifier.h"
namespace threading
{
	ConditionNotifier::ConditionNotifier(ContionPredicate predicate) :
		m_cbPredicate(predicate)
	{
	}

	void ConditionNotifier::NotifyOne()
	{
		std::unique_lock <std::mutex> lck(m_mtx);
		m_condition.notify_one();
	}
	void ConditionNotifier::NotifyAll()
	{
		std::unique_lock <std::mutex> lck(m_mtx);
		m_condition.notify_all();
	}

	void ConditionNotifier::Wait()
	{
		std::unique_lock <std::mutex> lck(m_mtx);
		if (!m_cbPredicate)
		{
			m_condition.wait(lck);
		}
		else
		{
			m_condition.wait(lck, m_cbPredicate);
		}
	}
}
