#ifndef I_Entity_h
#define I_Entity_h
#include "define.h"
#include "IComponent.h"
#include "SystemContainer.h"
#define ComponentCastFailLog(id,type) LogErrorFormat("component %d cast to %s fail!",id,type)
/************************************************************************/
/* IEntity ʵ��ӿ��࣬ʵ��Component�������õ�ʵ���ֱ࣬���޸�component
Data���ɶ����ݽ��в���������ͨ��system�޸ģ�����ESC�������Ƿ��̰߳�ȫ�� */
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