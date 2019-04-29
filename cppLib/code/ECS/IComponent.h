#ifndef I_component_h
#define I_component_h
#include "define.h"
class IComponent
{
public:
	IComponent() {}
	~IComponent() {}

protected:
	uint32_t m_ID;
	bool m_Dirty;
};
#endif
