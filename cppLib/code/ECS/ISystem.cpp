#include "ISystem.h"
namespace ecs
{
	static uint32_t _current = 0;
	uint32_t ecs::ComponentIDGenerator::GetComponentID()
	{
		return _current++;
	}
}
