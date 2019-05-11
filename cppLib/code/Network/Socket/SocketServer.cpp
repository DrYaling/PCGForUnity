#include "SocketServer.h"
#include "Logger/Logger.h"
#include "CoreOpcodes.h"
#include "Threading/ThreadManager.h"
static std::shared_ptr<SocketServer> m_pInstance = nullptr;
const static int PACKET_HEADER_SIZE = IKCP_OVERHEAD_LEN + 4 + sizeof(PacketHeader);
char accept_pack_data[PACKET_HEADER_SIZE - IKCP_OVERHEAD_LEN] = { 0 };
std::shared_ptr<SocketServer> SocketServer::GetInstance()
{
	if (nullptr == m_pInstance)
	{
		m_pInstance = std::make_shared<SocketServer>(SocketType::SOCKET_UDP);
	}
	return m_pInstance;
}
void SocketServer::Destroy()
{
	if (nullptr != m_pInstance)
	{
		m_pInstance->Stop();
		m_pInstance = nullptr;
	}
}
SocketServer::SocketServer(SocketType sock) :
	m_pSendNotifier(nullptr),
	m_nClientConv(0),
	m_nMTU(512),
	m_nServerId(0)
{
	m_pSocket = new Socket(sock);
	m_pSocket->SetSocketMode(SocketSyncMode::SOCKET_ASYNC);
	//LogFormat("Socket Server fd %d",m_pSocket->GetHandle());
	m_readBuffer.Resize(RECV_BUFFER_SIZE);
	PacketHeader acptHeader;
	acptHeader.Command = C2S_CONNECT;
	acptHeader.Size = 4;
	memcpy(accept_pack_data, &acptHeader, sizeof(acptHeader));
	memcpy(accept_pack_data + sizeof(acptHeader), "acpt", 4);
	//LogFormat("accept_pack_data %s", accept_pack_data);
}

SocketServer::~SocketServer()
{
}

std::shared_ptr<KcpSession> SocketServer::CreateSession(IUINT32 conv, const SockAddr_t & addr)
{
	std::shared_ptr<KcpSession> session = std::make_shared<KcpSession>(conv, addr);
	return session;
}

bool SocketServer::StartUp()
{
	m_pSendNotifier = new threading::ConditionNotifier(std::bind(&SocketServer::IsSendQueueNotEmpty, shared_from_this()));
	sThreadManager->AddTask(threading::ThreadTask(std::bind(&SocketServer::SendDataHandler, shared_from_this())));
	m_pSendNotifier->NotifyAll();
	m_pSocket->SetReadBuffer(m_readBuffer.GetBasePointer());
	auto callback = std::bind(&SocketServer::ReadHandlerInternal, this, std::placeholders::_1, std::placeholders::_2);
	m_pSocket->SetRecvCallback(callback);
	/*m_pSocket->SetRecvCallback([&](int size, const char* data)->void {
		this->ReadHandlerInternal(size, data);
	});*/
	m_pSocket->SetBlock(true);
	m_pSocket->Bind();
	return m_pSocket->StartListen() == 0;
}

bool SocketServer::Stop()
{
	m_pSocket->SetRecvCallback(nullptr);
	m_pSocket->Close();
	if (nullptr != m_pSendNotifier)
	{
		m_pSendNotifier->NotifyAll();
		safe_delete(m_pSendNotifier);
	}
	return true;
}

bool SocketServer::SetAddress(const char * ip, unsigned short port) const
{
	m_pSocket->SetAddress(ip, port);
	return true;
}

void SocketServer::Update(uint32_t diff)
{
	{
		std::lock_guard<std::mutex>lock(_sessionLock);
		for (auto aliveSession : m_mSessions)
		{
			std::shared_ptr<KcpSession>& session = aliveSession.second;
			if (session)
			{
				aliveSession.second->Update(diff);
			}
		}
	}
	for (auto sSession = m_mSleepSessions.begin(); sSession != m_mSleepSessions.end();)
	{
		std::shared_ptr<KcpSession>& session = sSession->second;
		if (session)
		{
			sSession->second->Update(diff);
			if (sSession->second->IsDead())
			{
				sSession = m_mSleepSessions.erase(sSession);
			}
			else
			{
				sSession++;
			}
		}
	}
}

int32 SocketServer::Send(const char * data, int32 dataSize, const socketSessionId& session)
{
	//auto map_client = std::find(m_mSocketClients.begin(), m_mSocketClients.end(), client);

	std::lock_guard<std::mutex> lock_guard(_sendLock);
	if (m_writeQueue.Push(data, dataSize, session.addr))
	{
		m_pSendNotifier->NotifyOne();
		return dataSize;
	}
	return -1;
}

void SocketServer::ReadHandlerInternal(int size, const char * buffer)
{
	if (size <= 0)
	{
		LogFormat("read data error from %s", inet_ntoa(m_pSocket->GetRecvSockAddr().sin_addr));
		return;
	}
	m_readBuffer.WriteCompleted(size);
	//LogFormat("read data size  %d  from %s", size, inet_ntoa(m_pSocket->GetRecvSockAddr().sin_addr));
	ReadHandler();
	m_readBuffer.Reset();
	//m_readBuffer.Normalize();
	//m_readBuffer.EnsureFreeSpace(size);
}

void SocketServer::ReadHandler()
{
	if (!m_pSocket->Connected())
		return;
	auto addr = m_pSocket->GetRecvSockAddr_t();
	std::shared_ptr<KcpSession> session = GetSessionByRemote(addr);
	/*if (session == nullptr)
	{
		session = GetSleepSessionByRemote(addr);
		if (session)
		{
			session->OnConnected();
			if (m_cbAcceptHandle)
			{
				m_cbAcceptHandle(session, session->GetSessionId().conv);
			}
		}
	}*/
	//if cant find session,try create new session
	if (nullptr == session)
	{
		//accept
		session = OnAccept(addr);
		if (nullptr != session)
		{
			session->OnConnected();
			if (m_cbAcceptHandle)
			{
				m_cbAcceptHandle(session, session->GetSessionId().conv);
			}
		}
		return;
	}
	//maybe receive a real pack
	session->OnReceive(m_readBuffer.GetReadPointer(), m_readBuffer.GetActiveSize());

}


void SocketServer::SendDataHandler()
{
	//LogFormat("SendDataHandler buffer queue size %d",m_writeQueue.GetActiveSize());
	if (m_writeQueue.GetActiveSize() > 0)
	{
		auto buffer = m_writeQueue.Front();
		SockError err;
		size_t send_size = buffer->buffer.GetActiveSize() > GetMTU() ? GetMTU() : buffer->buffer.GetActiveSize();
		if (buffer->buffer.GetActiveSize() > GetMTU())
		{
			err = m_pSocket->SendTo(buffer->buffer.GetReadPointer(), GetMTU(), buffer->addr);
		}
		else
		{
			err = m_pSocket->SendTo(buffer->buffer.GetReadPointer(), buffer->buffer.GetActiveSize(), buffer->addr);
		}
		if (err.nresult == 0)
		{
			buffer->buffer.ReadCompleted(err.nbytes);
			//LogFormat("Send to %s:%d,size %d",GetSocketAddrStr(buffer->addr),buffer->addr.port,err.nbytes);
			if (buffer->buffer.GetActiveSize() <= 0)
			{
				m_writeQueue.Pop();
			}
		}
		else
		{
			//LogFormat("Socket Server Send fail %d",err.nresult);
		}
		m_pSendNotifier->Wait();
	}
}


bool SocketServer::CloseSession(const SockAddr_t & client)
{
	std::lock_guard<std::mutex> lock(_sessionLock);
	for (auto mc = m_mSessions.begin(); mc != m_mSessions.end(); mc++)
	{
		if (mc->second->GetSessionId().addr == client)
		{
			LogFormat("client %d ,ip :%s,%d is closed by Server!", mc->first, GetSocketAddrStr(mc->second->GetSessionId().addr), mc->second->GetSessionId().addr.port);
			mc->second->OnDisconnected(true);
			m_mSessions.erase(mc);
			return true;
		}
	}
	return false;
}

bool SocketServer::CloseSession(uint32_t session, bool remove)
{
	std::lock_guard<std::mutex> lock(_sessionLock);
	for (auto mc = m_mSessions.begin(); mc != m_mSessions.end(); mc++)
	{
		if (mc->first == session)
		{
			LogFormat("client %d ,ip :%s,%d is closed by Server!", mc->first, GetSocketAddrStr(mc->second->GetSessionId().addr), mc->second->GetSessionId().addr.port);
			mc->second->OnDisconnected(remove);
			if (!remove)
			{
				m_mSleepSessions.insert(std::make_pair(session, mc->second));
			}
			m_mSessions.erase(mc);
			return true;
		}
	}
	return false;
}
std::map<IUINT32, std::shared_ptr<KcpSession>>::iterator SocketServer::OnSessionDead(std::shared_ptr<KcpSession> session)
{
	for (auto mc = m_mSleepSessions.begin(); mc != m_mSleepSessions.end(); mc++)
	{
		if (mc->second == session)
		{
			LogFormat("client %d ,ip :%s,%d is dead !", mc->first, GetSocketAddrStr(mc->second->GetSessionId().addr), mc->second->GetSessionId().addr.port);
			char buff[] = { "disconnected by Server" };
			Send(buff, sizeof(buff), mc->second->GetSessionId());
			session->OnDisconnected(true);
			return m_mSleepSessions.erase(mc);
		}
	}
	return m_mSleepSessions.end();
}
std::shared_ptr<KcpSession> SocketServer::ContainsRemote(const SockAddr_t & remote, IUINT32 conv)
{
	std::lock_guard<std::mutex> lock(_sessionLock);
	for (auto mc = m_mSessions.begin(); mc != m_mSessions.end(); mc++)
	{
		if (mc->second->GetSessionId().addr == remote && mc->second->GetSessionId().conv == conv)
		{
			return mc->second;
		}
	}
	return nullptr;
}

std::shared_ptr<KcpSession> SocketServer::GetSessionByRemote(const SockAddr_t & remote)
{
	std::lock_guard<std::mutex> lock(_sessionLock);
	for (auto mc = m_mSessions.begin(); mc != m_mSessions.end(); mc++)
	{
		if (mc->second->GetSessionId().addr == remote)
		{
			return mc->second;
		}
	}
	return nullptr;
}

std::shared_ptr<KcpSession> SocketServer::GetSleepSessionByRemote(const SockAddr_t & remote)
{
	for (auto mc = m_mSleepSessions.begin(); mc != m_mSleepSessions.end(); mc++)
	{
		if (mc->second->GetSessionId().addr == remote)
		{
			return mc->second;
		}
	}
	return nullptr;
}

MessageBuffer* SocketServer::GetWaitingConnectSession(const SockAddr_t & remote)
{
	for (auto mc = m_mWaitingForConnections.begin(); mc != m_mWaitingForConnections.end(); mc++)
	{
		if (mc->first == remote)
		{
			return &mc->second;
		}
	}
	return nullptr;
}

bool SocketServer::CheckCrc(const char * buff, int length)
{
	return true;
}

bool SocketServer::AcceptPack(const uint8 * buff, int length)
{
	return memcmp(buff, accept_pack_data, length) == 0;
}

std::shared_ptr<KcpSession> SocketServer::OnAccept(const SockAddr_t& remote)
{
	std::shared_ptr<KcpSession> session;
	//not valid pack
	//a accept pack should allways simple as accept_pack_data,or its bad connection
	IUINT32 conv;
	auto msgBuffer = GetWaitingConnectSession(remote);
	//is not waiting for connection,parse this one
	if (nullptr == msgBuffer)
	{

		//if ((conv>>16 & 0xffff) == 0)//new connection
		{
			//m_readBuffer.ReadCompleted(IKCP_OVERHEAD_LEN);
			MessageBuffer buff(m_readBuffer.GetActiveSize());
			buff.Write(m_readBuffer.GetReadPointer(), m_readBuffer.GetActiveSize());
			msgBuffer = &buff;
			//not a accept pack
			if (msgBuffer->GetActiveSize() > sizeof(accept_pack_data))
				return nullptr;
			// Couldn't receive the whole accept pack this time.
			if (msgBuffer->GetActiveSize() != sizeof(accept_pack_data))
			{
				m_mWaitingForConnections.insert(std::make_pair(remote, buff));
				return nullptr;
			}
			if (!AcceptPack(msgBuffer->GetReadPointer(), msgBuffer->GetActiveSize()))
			{
				// not accept pack
				return nullptr;
			}
			conv = IUINT32(GetNewConv() << 16) + GetServerId();
			session = CreateSession(conv, remote);
			std::lock_guard<std::mutex> lock(_sessionLock);
			m_mSessions.insert(std::make_pair(conv, session));
			return session;
		}
	}
	else
	{
		//if ((conv>>16 & 0xffff) == 0)//new connection
		{
			//will never goes here,but for exception
			//m_readBuffer.ReadCompleted(IKCP_OVERHEAD_LEN);
			msgBuffer->EnsureFreeSpace(m_readBuffer.GetActiveSize());
			msgBuffer->Write(m_readBuffer.GetReadPointer(), m_readBuffer.GetActiveSize());
			for (auto itr = m_mWaitingForConnections.begin(); itr != m_mWaitingForConnections.end(); itr++)
			{
				if (itr->first == remote)
				{
					m_mWaitingForConnections.erase(itr);
					break;
				}
			}
			//not a accept pack,close this waiting session
			if (msgBuffer->GetActiveSize() != sizeof(accept_pack_data))
			{
				return nullptr;
			}
			//invalid accept pack
			if (!AcceptPack(msgBuffer->GetReadPointer(), msgBuffer->GetActiveSize()))
			{
				return nullptr;
			}
			conv = IUINT32(GetNewConv() << 16) + GetServerId();
			session = CreateSession(conv, remote);
			std::lock_guard<std::mutex> lock(_sessionLock);
			m_mSessions.insert(std::make_pair(conv, session));
		}
	}
	return session;
}