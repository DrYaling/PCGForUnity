#include "SocketServer.h"
#include "Logger/Logger.h"
#include <algorithm>
#include <time.h>
#include "SocketTime.h"
#include "CoreOpcodes.h"
static SocketServer* m_pInstance = nullptr;
const static int PACKET_HEADER_SIZE = IKCP_OVERHEAD_LEN + 4 + sizeof(PacketHeader);
char accept_pack_data[PACKET_HEADER_SIZE - IKCP_OVERHEAD_LEN] = { 0 };
SocketServer * SocketServer::GetInstance()
{
	if (nullptr == m_pInstance)
	{
		m_pInstance = new SocketServer(SocketType::SOCKET_UDP);
	}
	return m_pInstance;
}
void SocketServer::Destroy()
{
	if (nullptr != m_pInstance)
	{
		m_pInstance->Stop();
		delete m_pInstance;
	}
}
SocketServer::SocketServer(SocketType sock) :m_nMTU(512), m_pSendNotifier(nullptr), m_nServerId(0)
{
	m_pSocket = new	Socket(sock);
	m_pSocket->SetSocketMode(SocketSyncMode::SOCKET_ASYNC);
	LogFormat("Socket Server fd %d",m_pSocket->GetHandle());
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
	m_pSendNotifier = new ConditionNotifier(std::bind(&SocketServer::SendDataHandler, this));
	m_pSendNotifier->Start();
	m_pSocket->SetReadBuffer(m_readBuffer.GetBasePointer());
	auto callback = std::bind(&SocketServer::ReadHandlerInternal, this,std::placeholders::_1,std::placeholders::_2);
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
		m_pSendNotifier->Exit();
		delete m_pSendNotifier;
	}
	return true;
}

bool SocketServer::SetAddress(const char * ip, unsigned short port)
{
	m_pSocket->SetAddress(ip, port);
	return true;
}

void SocketServer::Update(uint32_t diff)
{
	for (auto aliveSession : m_mSessions)
	{
		std::shared_ptr<KcpSession>& session = aliveSession.second;
		if (session)
		{
			aliveSession.second->Update(diff);
		}
	}
	//clear dead sessions
	m_vClosingSessions.clear();
	for (auto sSession  = m_mSleepSessions.begin();sSession != m_mSleepSessions.end();sSession++)
	{
		std::shared_ptr<KcpSession>& session = sSession->second;
		if (session)
		{
			sSession->second->Update(10);
			if (sSession->second->IsDead())
			{
				m_vClosingSessions.push_back(sSession);
			}
		}
	}
	/*if (m_vClosingSessions.size()>0)
	{
		for (auto itr :m_vClosingSessions)
		{
			OnSessionDead(itr->second);
			m_mSleepSessions.erase(itr);
		}
		m_vClosingSessions.clear();
	}*/
}

int32 SocketServer::Send(const char * data, int32 dataSize, const socketSessionId& session)
{
	//auto map_client = std::find(m_mSocketClients.begin(), m_mSocketClients.end(), client);

	std::lock_guard<std::mutex> lock_guard(_sendLock);
	if (m_writeQueue.Push(data, dataSize, session.addr))
	{
		m_pSendNotifier->Notify();
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


bool SocketServer::SendDataHandler()
{
	//LogFormat("SendDataHandler buffer queue size %d",m_writeQueue.GetActiveSize());
	if (m_writeQueue.GetActiveSize() > 0)
	{
		auto buffer = m_writeQueue.Front();
		SockError err;
		size_t sendSize = buffer->buffer.GetActiveSize() > GetMTU() ? GetMTU() : buffer->buffer.GetActiveSize();
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
			LogFormat("Socket Server Send fail %d",err.nresult);
		}
	}
	return m_writeQueue.GetActiveSize() <= 0;
}


bool SocketServer::CloseSession(const SockAddr_t & client)
{
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
	for (auto mc = m_mSessions.begin(); mc != m_mSessions.end(); mc++)
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
	std::shared_ptr<KcpSession> session = nullptr;
	//not valid pack
	//a accept pack should allways simple as accept_pack_data,or its bad connection
	IUINT32 conv = 0;
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
			if (!AcceptPack(msgBuffer->GetReadPointer(),msgBuffer->GetActiveSize()))
			{
				// not accept pack
				return nullptr;
			}
			conv = IUINT32(GetNewConv() << 16) + GetServerId();
			session = CreateSession(conv, remote);
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
			m_mSessions.insert(std::make_pair(conv, session));
		}
	}
	return session;
}

void ConditionNotifier::Run()
{
	m_bExited = false;
	std::unique_lock <std::mutex> lck(m_mtx);
	while (m_bRunning.load(std::memory_order_relaxed))
	{
		{
			if (m_pRunner)
			{
				if (m_pRunner())
				{
					m_condition.wait(lck);
					if (!m_bRunning.load(std::memory_order_relaxed))
					{
						break;
					}
					//Âú×ãÌõ¼þ,ÐÝÃß
				}
			}
			else
			{
				sleep(1000);
				LogError("ConditonNotifier has no runner");
			}
		}
	}
	m_bExited = true;
}

void ConditionNotifier::Start()
{
	std::thread t(std::bind(&ConditionNotifier::Run, this));
	t.detach();
}

void ConditionNotifier::Notify()
{
	std::unique_lock <std::mutex> lck(m_mtx);
	m_condition.notify_all();
	//LogFormat("ConditionNotifier::Notify");
}

void ConditionNotifier::Exit()
{
	m_bRunning.store(false, std::memory_order_release);
	Notify();
	while (!m_bExited) sleep(1);
	m_pRunner = nullptr;
}
