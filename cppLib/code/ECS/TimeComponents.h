#ifndef _time_components_h
#define _time_components_h
#include "IComponent.h"
#include <chrono>
namespace ecs
{
	struct IntervalTimer :IComponent
	{
		IntervalTimer() :IComponent(0) { SetCatalog(ComponentCatalog::INTERVAL_TIMER); }
		IntervalTimer(uint32_t id) :IComponent(id),
			current(0),
			interval(0)
		{
			SetCatalog(ComponentCatalog::INTERVAL_TIMER);
		}
		IntervalTimer(const IntervalTimer& it) :IComponent(it),
			current(it.current),
			interval(it.interval)
		{
			SetCatalog(ComponentCatalog::INTERVAL_TIMER);
		}
		time_t interval;
		time_t current;
		inline bool Passed()
		{
			return current >= interval;
		}

		inline void Reset()
		{
			if (current >= interval)
				current %= interval;
			SetDirty(true);
		}
	};
}
#endif
