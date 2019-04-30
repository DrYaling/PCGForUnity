#ifndef Movement_system_h
#define Movement_system_h
#include "ISystem.h"
#include "MovementComponent.h"
//test code
class MovementSystem :public ISystem<MovementComponent>,public ISystemInterface
{
	typedef ISystem base;
public:	
	MovementSystem();
	MovementSystem(const MovementSystem& copy) { LogError("wow!its here!"); }
	~MovementSystem();
	SystemCatalog GetCatalog()override;
	void OnUpdate(int32_t time_diff) override;
};
#endif
