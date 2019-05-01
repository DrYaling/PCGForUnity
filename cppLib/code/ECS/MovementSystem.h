#ifndef Movement_system_h
#define Movement_system_h
#include "ISystem.h"
#include "MovementComponent.h"
namespace ecs
{
	//test code
	class MovementSystem :public System<MovementComponent>, public ISystem
	{
		typedef System base;
	public:
		MovementSystem();
		MovementSystem(const MovementSystem& copy) { LogError("wow!its here!"); }
		~MovementSystem();
		SystemCatalog GetCatalog()override;
		void OnUpdate(int32_t time_diff) override;
	};
}
#endif
