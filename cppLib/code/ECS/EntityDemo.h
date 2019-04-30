#pragma once
#include "ISystem.h"
#include "MovementComponent.h"
#include "StatusComponent.h"
#include "IEntity.h"
#include "SystemContainer.h"
#include <map>
class EntityDemo :IEntity
{
public:
	EntityDemo();
	~EntityDemo();
	virtual void Initilize(SystemContainer* pContainer) override;
	void ChangeComponentDirty(uint32_t comId, ComponentCatalog catalog, bool dirty);
	void ReleaseTest();
private:
	MovementComponent * m_pMovement;
	std::map<uint32_t, StatusComponent*> m_mStatus;
	// 通过 IEntity 继承
	virtual void OnComponentChangeEvent(IComponent * com,uint32_t comId, ComponentCatalog catalog) override;
};

EntityDemo::EntityDemo()
{
}

EntityDemo::~EntityDemo()
{
	m_pContainer->UnRegisterComponentChangeEvent<MovementComponent>(1, SystemCatalog::MOVEMENT);
	m_pContainer->UnRegisterComponentChangeEvent<StatusComponent>(2, SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponentChangeEvent<StatusComponent>(3, SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponentChangeEvent<StatusComponent>(4, SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponentChangeEvent<StatusComponent>(5, SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponentChangeEvent<StatusComponent>(6, SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponent(1, SystemCatalog::MOVEMENT);
	m_pContainer->UnRegisterComponent(2, SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponent((3), SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponent((4), SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponent((5), SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponent((6), SystemCatalog::STATUS);
}

inline void EntityDemo::Initilize(SystemContainer* pContainer)
{
	m_pContainer = pContainer;
	auto event_call = std::bind(&EntityDemo::OnComponentChangeEvent, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
	pContainer->RegisterComponentChangeEvent<MovementComponent>(event_call, 1, SystemCatalog::MOVEMENT);
	pContainer->RegisterComponentChangeEvent<StatusComponent>(event_call, 2, SystemCatalog::STATUS);
	pContainer->RegisterComponentChangeEvent<StatusComponent>(event_call, 3, SystemCatalog::STATUS);
	pContainer->RegisterComponentChangeEvent<StatusComponent>(event_call, 4, SystemCatalog::STATUS);
	pContainer->RegisterComponentChangeEvent<StatusComponent>(event_call, 5, SystemCatalog::STATUS);
	pContainer->RegisterComponentChangeEvent<StatusComponent>(event_call, 6, SystemCatalog::STATUS);
	pContainer->RegisterComponent(MovementComponent(1), SystemCatalog::MOVEMENT);
	pContainer->RegisterComponent(StatusComponent(2), SystemCatalog::STATUS);
	pContainer->RegisterComponent(StatusComponent(3), SystemCatalog::STATUS);
	pContainer->RegisterComponent(StatusComponent(4), SystemCatalog::STATUS);
	pContainer->RegisterComponent(StatusComponent(5), SystemCatalog::STATUS);
	pContainer->RegisterComponent(StatusComponent(6), SystemCatalog::STATUS);
}

inline void EntityDemo::ChangeComponentDirty(uint32_t comId, ComponentCatalog catalog, bool dirty)
{
	switch (catalog)
	{
	case ComponentCatalog::MOVEMENT:
		if (m_pMovement && comId == m_pMovement->GetID())
		{
			SetComponentDirty(m_pMovement, dirty);
			LogFormat("EntityDemo set component %d (type MOVEMENT)dirty %d", comId, dirty);
		}
		break;
	case ComponentCatalog::STATUS:
	{
		auto itr = m_mStatus.find(comId);
		if (itr != m_mStatus.end())//有可能是空的
		{
			SetComponentDirty(itr->second, dirty);
			LogFormat("EntityDemo set component %d (type STATUS)dirty %d", comId, dirty);
		}
	}
	break;
	default:
		break;
	}
}

inline void EntityDemo::ReleaseTest()
{
	m_pContainer->UnRegisterComponent(3, SystemCatalog::STATUS);
	m_pContainer->UnRegisterComponent(5, SystemCatalog::STATUS);
}

void EntityDemo::OnComponentChangeEvent(IComponent * com, uint32_t comId, ComponentCatalog catalog)
{
	LogFormat("EntityDemo::OnComponentChangeEvent %d,catalog %d", comId,catalog);
	switch (catalog)
	{
	case ComponentCatalog::MOVEMENT:
		m_pMovement = static_cast<MovementComponent*>(com);
		if (!m_pMovement && com)
		{
			ComponentCastFailLog(comId, "MovementComponent");
		}
		else if(!com)
		{
			LogFormat("MovementComponent %d removed", comId);
		}
		//LogFormat("Movement %d", m_pMovement);
		break;
	case ComponentCatalog::STATUS:
	{
		auto status = static_cast<StatusComponent*>(com);
		if (!status && com)
		{
			//LogErrorFormat("com %d is not StatusComponent", com->GetID());
			ComponentCastFailLog(comId, "StatusComponent");
		}
		auto itr = m_mStatus.find(comId);
		if (itr != m_mStatus.end())
		{
			if (status)
			{
				m_mStatus[comId] = status;
			}
			else
			{
				m_mStatus.erase(itr);
				LogFormat("StatusComponent %d removed",comId);
			}
		}
		else if (status)
		{
			m_mStatus.insert(std::make_pair(comId, status));
		}
	}
	break;
	default:
		break;
	}
}
