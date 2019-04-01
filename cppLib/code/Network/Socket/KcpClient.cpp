#include "KcpClient.h"
#include "SocketTime.h"
#include "CoreOpcodes.h"
#include "Logger/Logger.h"
KcpClient::KcpClient(int16 sid) :
	m_nServerId(sid),
	m_bRecv(false),
	m_readBuffer(RECV_BUFFER_SIZE),
	m_headerBuffer(sizeof(PacketHeader)),
	m_nConnectTimeOut(6000),
	m_nReconnectTime(0)
{
	m_pSocket = new Socket(SocketType::SOCKET_UDP);
	m_pKcp = ikcp_create((IUINT32)0 + m_nServerId, this);
	ikcp_setmtu(m_pKcp, 512);
	ikcp_nodelay(m_pKcp, 1, 5, 1, 1);
	ikcp_wndsize(m_pKcp, 128, 128);
	ikcp_setoutput(m_pKcp, &KcpClient::fnWriteDgram);
	m_pKcp->rx_minrto = 10;
	m_pSocket->SetRecvCallback(std::bind(&KcpClient::ReadHandlerInternal, this, std::placeholders::_1, std::placeholders::_2));
	//m_pSocket->SetReadBuffer(m_recvBuffer.GetBasePointer());
	m_pSocket->SetSocketMode(SocketSyncMode::SOCKET_ASYNC);
	m_pSocket->SetBlock(true);
	/*m_pKcp->writelog = [](const char* msg, ikcpcb* kcp, void* user)->void {
		LogFormat(msg);
	};
	m_pKcp->logmask = 0;// 0xffff;*/
}

KcpClient::~KcpClient()
{
}

void KcpClient::SetAddress(const char * ip, int16 port)
{
	m_pSocket->SetAddress(ip, port);
	m_stremote = m_pSocket->GetSocketAddr_t();
	//m_stSessionId = socketSessionId(m_pKcp->conv, m_stremote);
}

void KcpClient::Connect()
{
	m_pSocket->Connect();
	char accept_pack_data[12] = { 0 };
	char kcphead[] = { 0,0,0,0, };
	PacketHeader acptHeader;
	acptHeader.Command = C2S_CONNECT;
	acptHeader.Size = 4;
	memcpy(accept_pack_data, &acptHeader, sizeof(acptHeader));
	memcpy(accept_pack_data + sizeof(acptHeader), "acpt", 4);
	//Send(accept_pack_data, sizeof(accept_pack_data), true);
	//ikcp_send(m_pKcp, accept_pack_data, sizeof(accept_pack_data));
	//ikcp_update(m_pKcp, m_pKcp->current);
	fnWriteDgram(accept_pack_data, 12, m_pKcp, this);
	m_nSessionStatus = SessionStatus::Connecting;
}

void KcpClient::Close()
{
	m_bRecv = false;
	m_bAlive = false;
}

void KcpClient::Send(char * buff, int length, bool immediately)
{
	if (m_nSessionStatus != SessionStatus::Connected)
	{
		return;
	}
	//sSocketServer->Send(buff, length, GetSessionId());
	ikcp_send(m_pKcp, buff, length);
	m_nLastSendTime = m_pKcp->current;
	if (immediately)
	{
		ikcp_update(m_pKcp, m_pKcp->current + 10);
	}
	else
	{
		m_bNeedUpdate = true;
	}
}

void KcpClient::OnReceive(const uint8 * buff, int length)
{
	int bytes = ikcp_input(m_pKcp, (char*)buff, length);
	int recv_size = ikcp_recv(m_pKcp, (char*)m_readBuffer.GetWritePointer(), m_readBuffer.GetRemainingSpace());
	//LogFormat("KcpClient recvsize %d", recv_size);
	if (recv_size > 0)
	{
		if (!CheckCrc((const char*)m_packetBuffer.GetReadPointer(), m_packetBuffer.GetActiveSize()))
		{
			return;
		}
		m_readBuffer.WriteCompleted(recv_size);
		ReadHandler();
	}
	else if (recv_size == -3)
	{
		LogErrorFormat("OnReceive size %d", recv_size);
	}
}

void KcpClient::Update(int32_t time)
{
	if (!m_bAlive)
	{
		m_nTick += time;
		if (m_nSessionStatus == SessionStatus::Connecting)
		{
			if (m_nTick >= reconnectInterval)
			{
				m_nTick = 0;
				m_nReconnectTime ++ ;
				if (m_nReconnectTime >= m_nConnectTimeOut/reconnectInterval)
				{
					OnConnectFailed();
				}
				else
				{
					LogFormat("reconnect %d",m_nReconnectTime);
					Connect();
				}
			}
		}
	}
	else
	{
		m_nTick += time;
		if (m_nTick > HEART_BEAT_INTERVAL)
		{
			m_nTick = 0;
			SendHeartBeat();
		}
		if (m_bNeedUpdate || m_pKcp->current + 15 >= m_nNeedUpdateTime) {
			ikcp_update(m_pKcp, m_pKcp->current + 15);
			m_nNeedUpdateTime = ikcp_check(m_pKcp, time);
			m_bNeedUpdate = false;
		}
		else
		{

		}
	}
}

void KcpClient::ReadHandlerInternal(int size, const char * buffer)
{
	if (size <= 0)
	{
		LogFormat("read data error from server %s", GetSocketAddrStr(m_stremote));
		return;
	}
	//m_readBuffer.WriteCompleted(size);
	//LogFormat("read data size  %d  from %s", size, inet_ntoa(m_pSocket->GetRecvSockAddr().sin_addr));
	OnReceive((uint8*)buffer, size);
	//m_readBuffer.Reset();
}

ReadDataHandlerResult KcpClient::ReadDataHandler()
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(m_headerBuffer.GetReadPointer());
	if (header->Command == S2C_ACCEPT_ACT)
	{
		//connect success
		IUINT32 conv = 0;
		utils_decode32u((char*)m_packetBuffer.GetReadPointer(), &conv);
		if (conv > 0)
		{
			m_stSessionId = socketSessionId(conv, m_stremote);
			m_pKcp->conv = conv;
		}
		OnConnected(conv > 0);
		return ReadDataHandlerResult::Ok;
	}
	else if (header->Command == S2C_DISCONNECT)
	{
		OnDisconnected(true);
		return ReadDataHandlerResult::Ok;
	}
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

bool KcpClient::ReadHeaderHandler()
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(m_headerBuffer.GetReadPointer());

	if (!header->IsValidSize() || !header->IsValidOpcode())
	{
		LogErrorFormat("SocketServer::ReadHeaderHandler(): client %s sent malformed packet (size: %u, cmd: %u)", GetSocketAddrStr(m_stremote), header->Size, header->Command);
		return false;
	}
	m_packetBuffer.Reset();
	m_packetBuffer.Resize(header->Size);
	//LogFormat("ReadHeaderHandler data size %d",header->Size);
	return true;
}

void KcpClient::ReadHandler()
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
				// Couldn't receive the whole header this time.
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
				// Couldn't receive the whole data this time.
				break;
			}
		}
		// just received fresh new payload
		ReadDataHandlerResult result = ReadDataHandler();
		m_headerBuffer.Reset();
		if (result != ReadDataHandlerResult::Ok)
		{
			if (result != ReadDataHandlerResult::WaitingForQuery)//error message ,disconnect
				;
		}
	}
}

void KcpClient::OnDisconnected(bool immediately)
{
	m_bAlive = false;
	m_nTick = 0;
	m_readBuffer.Reset();
	m_packetBuffer.Reset();
	m_headerBuffer.Reset();
	m_nSessionStatus = SessionStatus::Disconnected;
	//m_pKcp->conv = 0;
	LogFormat("Socket OnDisconnected from server %d", immediately);
}

void KcpClient::OnConnected(bool success)
{
	LogFormat("Socket Connect success %d", success);
	m_nSessionStatus = success ? SessionStatus::Connected : SessionStatus::Disconnected;
	m_bAlive = m_nSessionStatus == SessionStatus::Connected;
}

int KcpClient::fnWriteDgram(const char * buf, int len, ikcpcb * kcp, void * user)
{
	KcpClient* client = (KcpClient*)user;
	IUINT32 conv;
	utils_decode32u(buf, &conv);
	//LogFormat("client %d send size %d-cdata %d",client->m_stSessionId.conv,len, conv);
	if (conv == 0)
	{
		LogError("KcpClient Error");
	}
	client->m_pSocket->Send((void*)buf, len);
	return len;
}

void KcpClient::SendHeartBeat()
{
	PacketHeader head = { 0,C2S_HEARTBEAT };
	Send((char*)&head, sizeof(head), true);
	Log("KcpClient::SendHeartBeat()");
}

void KcpClient::OnConnectFailed()
{
	m_readBuffer.Reset();
	m_packetBuffer.Reset();
	m_headerBuffer.Reset();
	m_nSessionStatus = SessionStatus::Disconnected;
	LogFormat("OnConnectFail");
}
