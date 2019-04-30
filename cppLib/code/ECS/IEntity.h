#ifndef I_Entity_h
#define I_Entity_h
#include "define.h"
#include "IComponent.h"
#include "SystemContainer.h"
#define ComponentCastFailLog(id,type) LogErrorFormat("component %d cast to %s fail!",id,type)
/************************************************************************/
/* IEntity 实体接口类，实现Component数据引用到实体类，直接修改component
Data即可对数据进行操作，无需通过system修改，所有ESC操作都是非线程安全的 */
/************************************************************************/
class IEntity
{
public:
	IEntity() {}
	virtual ~IEntity() {}
	virtual void Initilize(SystemContainer* pContainer) = 0;
	virtual void OnComponentChangeEvent(IComponent* com, uint32_t comId, ComponentCatalog catalog) = 0;
	void SetComponentDirty(IComponent* com, bool dirty)
	{
		if (com)
		{
			com->SetDirty(dirty);
		}
	}
protected:
	SystemContainer * m_pContainer;
};
#endif