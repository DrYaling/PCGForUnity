#ifndef I_component_h
#define I_component_h
#include "define.h"
class IComponent
{
public:
	IComponent() {}
	~IComponent() {}
	uint32_t GetID() { return m_ID; }
	bool IsDirty() { return m_Dirty; }
	bool operator == (const IComponent& right)
	{
		return m_ID == right.m_ID;
	}
protected:
	uint32_t m_ID;
	bool m_Dirty;
};
#endif
