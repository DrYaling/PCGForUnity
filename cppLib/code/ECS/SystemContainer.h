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
	priorityList<ISystem> m_lSystem;
};
#endif

template<class T>
T * SystemContainer::RegisterComponent(const T & com, SystemCatalog catalog)
{
	return NULL;
}

template<class T>
void SystemContainer::UnRegisterComponent(T * comPtr, SystemCatalog catalog)
{
}
