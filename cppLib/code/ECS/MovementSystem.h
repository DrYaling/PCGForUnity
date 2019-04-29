#ifndef Movement_system_h
#define Movement_system_h
#include "ISystem.h"
#include "MovementComponent.h"
class MovementSystem :public ISystem
{
	typedef ISystem base;
public:	
	MovementSystem();
	~MovementSystem();
	SystemCatalog GetCatalog()override;
	void OnUpdate(int32_t time_diff) override;
};
#endif
