#ifndef System_container_h
#define System_container_h
#include "define.h"
#include "ISystem.h"
#include "PriorityList.h"
class MovementSystem;
class SystemContainer
{
public:
	SystemContainer();
	~SystemContainer();
	void Initilize();
	bool AddSystem(SystemCatalog catalog, int priority = 0);
	void OnUpdate(int32_t time_diff);
	template<class T>
	void RegisterComponent(const T& com, SystemCatalog catalog);
	void UnRegisterComponent(int componentId, SystemCatalog catalog);
	void SetPriority(SystemCatalog catalog, int32_t priority);
	template<class T>
	void RegisterComponentChangeEvent(ComponentChangeEvent event_call, uint32_t componentId, SystemCatalog catalog)
	{
		auto pSys = GetSystem<T>(catalog);
		if (pSys)
		{
			pSys->RegisterComponentChangeEvent(event_call, componentId);
		}
	}
	template<class T>
	void UnRegisterComponentChangeEvent(uint32_t componentId, SystemCatalog catalog)
	{
		auto pSys = GetSystem<T>(catalog);
		if (pSys)
		{
			pSys->UnRegisterComponentChangeEvent(componentId);
		}
	}
private:
	const ISystemInterface * GetSystemInterface(SystemCatalog catalog)
	{
		for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
		{
			if (itr->GetCatalog() == catalog)
			{
				return &*itr;
			}
		}
		return nullptr;
	}
	template<class T>
	ISystem<T> * GetSystem(SystemCatalog catalog)
	{
		for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
		{
			if (itr->GetCatalog() == catalog)
			{
				return dynamic_cast<ISystem<T>*>(itr.ptr());;
			}
		}
		return nullptr;
	}
private:
	priorityList<ISystemInterface> m_lSystem;
};

template<class T>
void SystemContainer::RegisterComponent(const T & com, SystemCatalog catalog)
{
	for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
	{
		if (itr->GetCatalog() == catalog)
		{
			ISystem<T>* sys = dynamic_cast<ISystem<T>*>(itr.ptr());
			if (sys)
			{
				return sys->RegisterComponent(com);
			}
		}
	}
}

#endif