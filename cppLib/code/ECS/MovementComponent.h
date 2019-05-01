#ifndef Movement_Component_h
#define Movement_Component_h
#include "IComponent.h"
namespace ecs
{
	struct MovementComponent :public IComponent {
	public:
		MovementComponent() :IComponent(0) { SetCatalog(ComponentCatalog::MOVEMENT); }
		MovementComponent(int32_t id) :IComponent(id) { SetCatalog(ComponentCatalog::MOVEMENT); }
		MovementComponent(const MovementComponent& copy) :IComponent(copy) {}
		~MovementComponent() {}
	};
}
#endif
