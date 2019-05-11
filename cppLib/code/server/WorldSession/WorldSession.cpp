#include "WorldSession.h"
#include "Logger/Logger.h"
#include "ECS/TimeComponents.h"
using namespace ecs;
namespace server
{
#define  WORLD_SESSION_UPDATE_INTERVAL 50
#define  WORLD_SESSION_TIMEOUT 18
	WorldSession::WorldSession(uint32 id, std::string&& name, std::shared_ptr<KcpSession> sock) :
		m_bInQueue(false),
		m_nSessionId(id),
		m_pSocket(sock),
		m_pTimer(nullptr),
		m_pHeatBeatTimer(nullptr),
		m_nTimeOutTime(GameTime::GetGameTime() + WORLD_SESSION_TIMEOUT)
	{
		m_pSocket->SetPacketReceivedCallback(std::bind(&WorldSession::OnReceivePacket, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	WorldSession::~WorldSession()
	{
		m_pSocket->Close();
		if (m_pContainer && m_pTimer)
		{
			m_pContainer->UnRegisterComponentChangeEvent<ecs::IntervalTimer>(m_pTimer->GetID(), ecs::SystemCatalog::INTERVAL_TIMER);
			m_pContainer->UnRegisterComponent(m_pTimer->GetID(), SystemCatalog::INTERVAL_TIMER);
		}
		if (m_pContainer && m_pHeatBeatTimer)
		{
			m_pContainer->UnRegisterComponentChangeEvent<ecs::IntervalTimer>(m_pHeatBeatTimer->GetID(), ecs::SystemCatalog::INTERVAL_TIMER);
			m_pContainer->UnRegisterComponent(m_pHeatBeatTimer->GetID(), SystemCatalog::INTERVAL_TIMER);
		}

		m_pTimer = nullptr;
		m_pHeatBeatTimer = nullptr;
		m_pContainer = nullptr;
		LogFormat("world Session destroyed");
	}

	void WorldSession::KickPlayer()
	{
		m_pSocket->Close();
	}

	void WorldSession::InitializeSession()
	{
	}

	void WorldSession::SendAuthWaitQue(uint32 position)
	{
	}

	bool WorldSession::Update(uint32_t diff)
	{
		///- Before we process anything:
		/// If necessary, kick the player because the client didn't send anything for too long
		/// (or they've been idling in character select)
		if (IsConnectionIdle())
		{
			m_pSocket->Close();
			LogFormat("session %d time out,remove", GetSessionId());
			return false;
		}
		//lost connection
		if (!m_pSocket->IsAlive())
		{
			m_pSocket->Close();
			LogFormat("session %d lost connection,remove", GetSessionId());
			return false;
		}
		if (m_pTimer)
		{
			if (!m_pTimer->Passed())
			{
				return true;
			}
			//LogFormat("Session %d timer %d passed",m_nSessionId,m_pTimer->GetID());
			m_pTimer->Reset();
		}
		if (m_pHeatBeatTimer && m_pHeatBeatTimer->Passed())
		{
			m_pHeatBeatTimer->Reset();
			m_pSocket->SendHeartBeat();
		}
		//todo update socket packets
		return true;
	}
	void WorldSession::Initilize(ecs::SystemContainer * pContainer)
	{
		if (!pContainer)
		{
			LogError(" WorldSession::Initilize fail!null container");
			return;
		}
		m_pContainer = pContainer;
		uint32_t timerId = ComponentIDGenerator::GetComponentID();
		m_nTimerId = timerId;
		auto event_call = std::bind(&WorldSession::OnComponentChangeEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		pContainer->RegisterComponentChangeEvent<ecs::IntervalTimer>(event_call, timerId, ecs::SystemCatalog::INTERVAL_TIMER);
		pContainer->RegisterComponent(IntervalTimer(timerId), SystemCatalog::INTERVAL_TIMER);
		timerId = ComponentIDGenerator::GetComponentID();
		m_nHeartBeatTimerId = timerId;
		pContainer->RegisterComponentChangeEvent<ecs::IntervalTimer>(event_call, timerId, ecs::SystemCatalog::INTERVAL_TIMER);
		pContainer->RegisterComponent(IntervalTimer(timerId), SystemCatalog::INTERVAL_TIMER);
	}
	void WorldSession::OnComponentChangeEvent(ecs::IComponent * com, uint32_t comId, ecs::ComponentCatalog catalog)
	{
		//LogFormat("OnComponentChangeEvent %d,%d",com,comId);
		if (catalog == ComponentCatalog::INTERVAL_TIMER)
		{
			if (comId == m_nTimerId)
			{
				m_pTimer = static_cast<IntervalTimer*>(com);
				if (m_pTimer)
				{
					m_pTimer->interval = WORLD_SESSION_UPDATE_INTERVAL;
				}
			}
			else if (comId == m_nHeartBeatTimerId)
			{
				m_pHeatBeatTimer = static_cast<IntervalTimer*>(com);
				if (m_pHeatBeatTimer)
				{
					m_pHeatBeatTimer->interval = HEART_BEAT_INTERVAL;
				}
			}
		}
	}
	bool WorldSession::OnReceivePacket(int cmd, uint8* buffer, int size)
	{
		switch (cmd)
		{
			case  C2S_HEARTBEAT:
				{
					m_nTimeOutTime = GameTime::GetGameTime() + WORLD_SESSION_TIMEOUT;
				}
				break;
			default:
				//LogFormat("WorldSession %d data handler %d: %d", GetSessionId(), cmd, size);
				break;
		}
		return true;
	}
}
