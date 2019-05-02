#include "Sys_Timers.h"
namespace ecs
{
	Sys_IntervalTimer::Sys_IntervalTimer()
	{
	}
	Sys_IntervalTimer::~Sys_IntervalTimer()
	{
	}
	void Sys_IntervalTimer::OnUpdate(int32_t time_diff)
	{
		for (auto itr = m_aData.begin();itr != m_aData.end();itr++)
		{
			if (itr->IsDirty())
			{
				itr->current += time_diff;
				if (itr->Passed())
				{
					FlushComponent(itr);
				}
			}
		}
	}
	SystemCatalog Sys_IntervalTimer::GetCatalog()
	{
		return SystemCatalog::INTERVAL_TIMER;
	}
}
