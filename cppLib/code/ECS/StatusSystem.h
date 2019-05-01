#ifndef Status_system_h
#define Status_system_h
#include "ISystem.h"
#include "StatusComponent.h"
namespace ecs
{
	//test code
	class StatusSystem :public System<StatusComponent>, public ISystem
	{
	public:
		StatusSystem();
		~StatusSystem();
		void OnUpdate(int32_t time_diff) override;
		SystemCatalog GetCatalog()override;
	};
}
#endif
