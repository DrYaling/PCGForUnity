#ifndef Status_component_h
#define Status_component_h
#include "IComponent.h"
namespace ecs
{
	//test code
	class StatusComponent :public IComponent
	{
	public:
		StatusComponent() :IComponent(0) { SetCatalog(ComponentCatalog::STATUS); }
		StatusComponent(int32_t id) :IComponent(id) { SetCatalog(ComponentCatalog::STATUS); }
		StatusComponent(const StatusComponent& copy) :IComponent(copy) {}
		~StatusComponent() {}
		uint32_t status;
	private:

	};
}
#endif
