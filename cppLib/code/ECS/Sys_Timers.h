#ifndef _sys_timers_h
#define _sys_timers_h
#include "ISystem.h"
#include "TimeComponents.h"
namespace ecs
{
	class Sys_IntervalTimer :public ISystem, public ecs::System<IntervalTimer>
	{
	public:
		Sys_IntervalTimer();
		virtual ~Sys_IntervalTimer();
		void OnUpdate(int32_t time_diff) override;
		virtual SystemCatalog GetCatalog() override;
	};
}
#endif
