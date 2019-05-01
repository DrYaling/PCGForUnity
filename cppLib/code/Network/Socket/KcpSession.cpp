#include "KcpSession.h"
#include "Logger/Logger.h"
#include "SocketServer.h"
#include "CoreOpcodes.h"
#include "SocketConfig.h"
#include "SocketTime.h"
KcpSession::KcpSession(IUINT32 conv, const SockAddr_t& addr) :m_stSessionId(conv, addr), m_bNeedUpdate(false), m_bAlive(true), m_readBuffer(RECV_BUFFER_SIZE), m_headerBuffer(sizeof(PacketHeader))
{
	LogFormat("KcpSession Session Addr %d", m_stSessionId.addr.port);
	m_pKcp = ikcp_create(conv, this);
	ikcp_setmtu(m_pKcp, sSocketServer->GetMTU());
	ikcp_nodelay(m_pKcp, 1, 5, 1, 1);
	ikcp_wndsize(m_pKcp, 128, 128);
	ikcp_setoutput(m_pKcp, &KcpSession::fnWriteDgram);
	m_pKcp->rx_minrto = 10;

	/*m_pDataHandler = [&](int cmd, uint8* data, int size)->bool {
		LogFormat("data handler %d: %d", cmd, size);
		char buffer[512] = { 26 };
		return true;
	};*/
	/*m_pKcp->logmask = 0;// 0xffff;
	m_pKcp->writelog = [](const char* msg, ikcpcb* kcp, void* user)->void {
		LogFormat(msg);
	};*/
}
/*
KcpSession::KcpSession(const KcpSession& kcp)
{
	LogErrorFormat("copy");
}
KcpSession::KcpSession(const KcpSession&& kcp)
{
	LogErrorFormat("copy1");
}*/
KcpSession::~KcpSession()
{
	Close();
	ikcp_release(m_pKcp);
	m_pKcp = nullptr;
}

void KcpSession::Close()
{
	if (m_nSessionStatus == SessionStatus::Disconnected)
	{
		return;
	}
	SocketServer::GetInstance()->CloseSession(GetSessionId().conv, false);
	//m_pDataHandler = nullptr;
}
int KcpSession::fnWriteDgram(const char *buf, int len, ikcpcb *kcp, void *user) {
	KcpSession* session = (KcpSession*)user;
	IUINT32 cmd = 0;
	utils_decode32u(buf + IKCP_OVERHEAD_LEN + 4, &cmd);
	if (cmd == S2C_ACCEPT_ACT)
	{
		MessageBuffer acceptBuffer(len);
		acceptBuffer.Write(buf, len);
		utils_encode32u((char*)acceptBuffer.GetBasePointer(), 0);//conv set to 0
		sSocketServer->Send((const char*)acceptBuffer.GetBasePointer(), len, session->GetSessionId());
	}
	else
	{
		sSocketServer->Send(buf, len, session->GetSessionId());
	}
	return len;
}

void KcpSession::SendHeartBeat()
{
	PacketHeader head = { 0,S2C_HEARTBEAT };
	Send((char*)&head, sizeof(head), true);
	Log("KcpSession::SendHeartBeat()");
}

void KcpSession::Send(char * buff, int length, bool immediately)
{
	//sSocketServer->Send(buff, length, GetSessionId());
	ikcp_send(m_pKcp, buff, length);
	m_nLastSendTime = m_pKcp->current;
	if (immediately)
	{
		m_updateMtx.lock();
		ikcp_update(m_pKcp, m_pKcp->current + m_pKcp->interval);
		m_updateMtx.unlock();
	}
	else
	{
		m_bNeedUpdate = true;
	}
}

void KcpSession::OnReceive(const uint8 * buff, int length)
{
	//todo after deserilize real packet
	IUINT32 conv = ikcp_getconv(buff);
	//error Server target

	if ((conv & 0xffff) != sSocketServer->GetServerId())
	{
		//ignore erro pack
		LogErrorFormat("error serverId %d", conv >> 16);
		sSocketServer->CloseSession(GetSessionId().conv, false);
		return;
	}
	//error conv
	if (GetSessionId().conv != conv)
	{
		LogErrorFormat("error conv %d-should be %d", conv, GetSessionId().conv);
		sSocketServer->CloseSession(GetSessionId().conv, false);
		return;
	}
	int bytes = ikcp_input(m_pKcp, (char*)buff, length);
	int recv_size = ikcp_recv(m_pKcp, (char*)m_readBuffer.GetWritePointer(), m_readBuffer.GetRemainingSpace());
	if (recv_size > 0)
	{
		if (!sSocketServer->CheckCrc((const char*)m_packetBuffer.GetReadPointer(), m_packetBuffer.GetActiveSize()))
		{
			return;
		}
		m_readBuffer.WriteCompleted(recv_size);
		ReadHandler();
		m_readBuffer.Reset();
	}
	else if (recv_size == -3)
	{
		LogErrorFormat("OnReceive size %d", recv_size);
	}
}

void KcpSession::Update(uint32_t diff)
{
	if (!m_bAlive)
	{
		m_nTick += diff;
	}
	else
	{
		m_nTick += diff;
		if (m_nTick > HEART_BEAT_INTERVAL)
		{
			m_nTick = 0;
			SendHeartBeat();
		}
		uint32_t current = m_pKcp->current + diff;
		if (m_bNeedUpdate || current >= m_nNeedUpdateTime) {
			m_updateMtx.lock();
			ikcp_update(m_pKcp, current);
			m_updateMtx.unlock();
			m_nNeedUpdateTime = ikcp_check(m_pKcp, current);
			m_bNeedUpdate = false;
		}
		else
		{

		}
		if (ikcp_peeksize(m_pKcp) > 0)
		{
			int recv_size = ikcp_recv(m_pKcp, (char*)m_readBuffer.GetReadPointer(), m_readBuffer.GetActiveSize());
			if (recv_size > 0)
			{
				if (!sSocketServer->CheckCrc((const char*)m_packetBuffer.GetReadPointer(), m_packetBuffer.GetActiveSize()))
				{
					return;
				}
				m_readBuffer.ReadCompleted(recv_size);
				ReadHandler();
				m_readBuffer.Reset();
			}
			else if (recv_size == -3)
			{
				LogErrorFormat("OnReceive size %d", recv_size);
			}
		}
	}
}

void KcpSession::Disconnect()
{
	Log("SessionDisconnect");
	PacketHeader header = { 0,S2C_DISCONNECT };
	Send((char*)&header, sizeof(header), true);
	m_nSessionStatus = SessionStatus::Disconnected;
	sSocketServer->CloseSession(GetSessionId().conv, true);
}

void KcpSession::SetPacketReceivedCallback(SocketDataReceiveHandler callback)
{
	m_pDataHandler = callback;
}

void KcpSession::SetConnectFinishedCallback(std::function<void(std::shared_ptr<KcpSession>)> callback)
{
	m_cbConnectFinished = callback;
}

void KcpSession::SetDisconnectedCallback(std::function<void(std::shared_ptr<KcpSession>)> callback)
{
	m_cbDisconnected = callback;
}

void KcpSession::ReadHandler()
{
	while (m_readBuffer.GetActiveSize() > 0)
	{
		if (m_headerBuffer.GetRemainingSpace() > 0)
		{
			// need to receive the header
			std::size_t readHeaderSize = min(m_readBuffer.GetActiveSize(), m_headerBuffer.GetRemainingSpace());
			//LogFormat("header size %d,pack active size %d",readHeaderSize,packet.GetActiveSize());
			m_headerBuffer.Write(m_readBuffer.GetReadPointer(), readHeaderSize);
			m_readBuffer.ReadCompleted(readHeaderSize);

			if (m_headerBuffer.GetRemainingSpace() > 0)
			{
				// Couldn't receive the whole header this diff.
				break;
			}

			// We just received nice new header
			if (!ReadHeaderHandler())
			{
				return;
			}
		}
		// We have full read header, now check the data payload
		if (m_packetBuffer.GetRemainingSpace() > 0)
		{
			// need more data in the payload
			std::size_t readDataSize = min(m_readBuffer.GetActiveSize(), m_packetBuffer.GetRemainingSpace());
			m_packetBuffer.Write(m_readBuffer.GetReadPointer(), readDataSize);
			m_readBuffer.ReadCompleted(readDataSize);

			if (m_packetBuffer.GetRemainingSpace() > 0)
			{
				// Couldn't receive the whole data this diff.
				break;
			}
		}
		// just received fresh new payload
		ReadDataHandlerResult result = ReadDataHandler();
		m_headerBuffer.Reset();
		if (result != ReadDataHandlerResult::Ok)
		{
			if (result != ReadDataHandlerResult::WaitingForQuery)//error message ,disconnect
				sSocketServer->CloseSession(GetSessionId().conv);
		}
	}
}

void KcpSession::OnDisconnected(bool immediately)
{
	if (m_nSessionStatus != SessionStatus::Disconnected)
	{
		m_nSessionStatus = SessionStatus::Disconnected;
		PacketHeader header = { 0,S2C_DISCONNECT };
		//disconnect msg 
		Send((char*)&header, sizeof(header), true);
	}
	if (immediately)
	{
		delete this;
		return;
	}
	else
	{
		m_bAlive = false;
		m_nTick = 0;
		m_readBuffer.Reset();
		m_packetBuffer.Reset();
		m_headerBuffer.Reset();
	}
}

void KcpSession::OnConnected(IUINT32 conv, const SockAddr_t& addr)
{
	m_stSessionId = socketSessionId(conv, addr);
	m_bAlive = true;
	if (m_pKcp)
		ikcp_release(m_pKcp);
	m_pKcp = ikcp_create(conv, this);
	m_pKcp->mtu = sSocketServer->GetMTU();
	ikcp_nodelay(m_pKcp, 1, 10, 2, 1);
	ikcp_wndsize(m_pKcp, 128, 128);
	ikcp_setmtu(m_pKcp, 512);
	ikcp_setoutput(m_pKcp, &KcpSession::fnWriteDgram);
	OnConnected();
}

void KcpSession::OnConnected()
{
	PacketHeader header = { 4,S2C_ACCEPT_ACT };
	char buff[sizeof(PacketHeader) + 4] = {};
	memcpy(buff, &header, sizeof(PacketHeader));
	memcpy(buff + sizeof(PacketHeader), &GetSessionId().conv, sizeof(IUINT32));
	Send(buff, sizeof(buff), true);
	m_bAlive = true;
	m_nSessionStatus = SessionStatus::Connected;
}

ReadDataHandlerResult KcpSession::ReadDataHandler()
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(m_headerBuffer.GetReadPointer());
	//LogFormat("UdpSocket read remote cmd %d,data size %d,%s", header->Command, header->Size, m_packetBuffer.GetReadPointer());
	if (nullptr != m_pDataHandler)
	{
		if (m_pDataHandler(header->Command, m_packetBuffer.GetReadPointer(), m_packetBuffer.GetActiveSize()))
		{
			return ReadDataHandlerResult::Ok;
		}
	}
	return ReadDataHandlerResult::Error;
}

bool KcpSession::ReadHeaderHandler()
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(m_headerBuffer.GetReadPointer());

	if (!header->IsValidSize() || !header->IsValidOpcode())
	{
		LogErrorFormat("SocketServer::ReadHeaderHandler(): client %s sent malformed packet (size: %u, cmd: %u)", GetSocketAddrStr(GetSessionId().addr), header->Size, header->Command);
		return false;
	}
	m_packetBuffer.Reset();
	m_packetBuffer.Resize(header->Size);
	//LogFormat("ReadHeaderHandler data size %d",header->Size);
	return true;
}
