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
	bool AddSystem(SystemCatalog catalog,int priority = 0);
	void OnUpdate(int32_t time_diff);
	template<class T>
	T* RegisterComponent(const T& com,SystemCatalog catalog);
	void UnRegisterComponent(int componentId, SystemCatalog catalog);
	template<class T>
	void UnRegisterComponent(T* comPtr, SystemCatalog catalog);
	void SetPriority(SystemCatalog catalog, int32_t priority);
private:
	priorityList<ISystemInterface> m_lSystem;
};
#endif

template<class T>
T * SystemContainer::RegisterComponent(const T & com, SystemCatalog catalog)
{
	m_lSystem.ReadReset();
	while (ISystemInterface* system = m_lSystem.ReadNext())
	{
		if (system->GetCatalog() == catalog)
		{
			ISystem<T>* sys = dynamic_cast<ISystem<T>*>(system);
			if (sys)
			{
				m_lSystem.ReadReset();
				return sys->RegisterComponent(com);
			}
		}
	}
	m_lSystem.ReadReset();
	return nullptr;
}

template<class T>
void SystemContainer::UnRegisterComponent(T * comPtr, SystemCatalog catalog)
{
	m_lSystem.ReadReset();
	while (ISystemInterface* system = m_lSystem.ReadNext())
	{
		if (system->GetCatalog() == catalog)
		{
			ISystem<T>* sys = dynamic_cast<ISystem<T>*>(system);
			if (sys)
			{
				m_lSystem.ReadReset();
				return sys->RegisterComponent(com);
			}
		}
	}
	m_lSystem.ReadReset();
}
