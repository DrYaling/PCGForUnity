#ifndef I_System_h
#define I_System_h
#include "define.h"
#include "IComponent.h"
#include "G3D/Array.h"
#include "Logger/Logger.h"
//using namespace G3D;
enum class SystemCatalog
{
	MOVEMENT = 0,
	STATUS,
};
class SystemContainer;
class ISystemInterface {
	friend SystemContainer;
public:
	virtual void OnUpdate(int32_t time_diff) = 0;
	ISystemInterface() {}
	virtual ~ISystemInterface()
	{
		LogFormat("ISystemInterface::~ISystemInterface()");
	}
	virtual SystemCatalog GetCatalog() = 0;
	int32_t GetPriority() { return m_nPriority; }
	void SetEnabled(bool yes) { m_bEnable = yes; }
	bool GetEnabled() { return m_bEnable; }
protected:
	int32_t m_nPriority;
	bool m_bEnable;
};
template<class component = IComponent>
class ISystem
{
	friend SystemContainer;
public:
	ISystem() {}
	virtual ~ISystem() 
	{
		LogFormat("ISystem::~ISystem()");
	}
	component* RegisterComponent(const component& com)
	{
		m_aData.append(com);
		return &m_aData[m_aData.size() - 1];
	}
	void UnRegisterComponent(int componentId)
	{
		for (auto itr = m_aData.begin(); itr != m_aData.end(); itr++)
		{
			if (itr->GetID() == componentId)
			{
				m_aData.remove(itr);
				return;
			}
		}
	}
	void UnRegisterComponent(component* comPtr)
	{
		auto itr = m_aData.find(*comPtr);
		if (itr != m_aData.end())
		{
			m_aData.remove(itr);
		}
	}
	bool operator == (const ISystem& right) const
	{
		return this == &right;
	}
protected:
	G3D::Array<component> m_aData;
};
#endif
