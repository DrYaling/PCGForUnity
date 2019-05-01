#include "WorldSession.h"
#include "Logger/Logger.h"
namespace server
{
	WorldSession::WorldSession(uint32 id, std::string&& name, std::shared_ptr<KcpSession> sock) :
		m_bInQueue(false),
		m_nSessionId(id),
		m_pSocket(sock),
		m_nTimeOutTime(GameTime::GetGameTime() + 30000)
	{
		m_pSocket->SetPacketReceivedCallback(std::bind(&WorldSession::OnReceivePacket, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
	}

	WorldSession::~WorldSession()
	{
		m_pSocket->Close();
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
		if (IsConnectionIdle() )
			m_pSocket->Close();
		//todo update socket packets
		return true;
	}
	bool WorldSession::OnReceivePacket(int cmd, uint8* buffer, int size)
	{
		LogFormat("WorldSession %d data handler %d: %d",GetSessionId(), cmd, size);
		return true;
	}
}
