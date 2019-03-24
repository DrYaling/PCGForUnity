#include "SocketServer.h"
#include "Logger/Logger.h"
#include <algorithm>
#include <time.h>
SocketServer::SocketServer(SocketType sock)
{
	m_pSocket = new	Socket(sock);
	m_pSocket->SetSocketMode(SocketSyncMode::SOCKET_ASYNC);
	m_readBuffer.Resize(4096);
	m_headerBuffer.Resize(sizeof(PacketHeader));
}

SocketServer::~SocketServer()
{
}

bool SocketServer::StartUp()
{
	m_pSocket->SetReadBuffer(m_readBuffer.GetBasePointer());
	m_pSocket->SetRecvCallback([&](int size, const char* data)->void {
		this->ReadHandlerInternal(size, data);
	});
	m_pDataHandler = [&](int cmd, uint8* data, int size)->bool {
		LogFormat("data handler %d: %s",cmd,data);
		char buffer[204018] = { 26 };
		for (auto client :this->m_mSocketClients)
		{
			this->Send(buffer, sizeof(buffer), client.second);
		}
		return true;
	};
	m_pSocket->Bind();
	return m_pSocket->StartListen() == 0;
}

bool SocketServer::Stop()
{
	m_pSocket->SetRecvCallback(nullptr);
	m_pSocket->Close();
	m_pDataHandler = nullptr;
	return true;
}

bool SocketServer::SetAddress(const char * ip, unsigned short port)
{
	m_pSocket->SetAddress(ip, port);
	return true;
}

void SocketServer::Update()
{
}

bool SocketServer::Send(char * data, int32 dataSize, int client)
{
	//auto map_client = std::find(m_mSocketClients.begin(), m_mSocketClients.end(), client);
	
	for (auto mc : m_mSocketClients)
	{
		if (mc.second == client)
		{
			//std::lock_guard<std::mutex> lock_guard(_sendLock);
			//SocketWriteBuffer buffer(client, data, dataSize);
			//_writeQueue.push(std::move(SocketWriteBuffer(client, data, dataSize)));
			clock_t startTime,endTime;
			startTime = clock();//��ʱ��ʼ
			auto err = m_pSocket->SendTo(data, dataSize, (SockAddr_t&)mc.first);
			endTime = clock();//��ʱ����
			LogFormat("socket server send data size %d,cost time %d,err %d",dataSize,endTime-startTime,err.nresult);
			return true;
		}
	}
	return false;
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
	if (!ContainsRemote(addr))
	{
		m_mSocketClients.insert(std::make_pair<SockAddr_t&, int>(addr,GetNewClientId()));
	}
	MessageBuffer& packet = GetReadBuffer();
	while (packet.GetActiveSize() > 0)
	{
		if (m_headerBuffer.GetRemainingSpace() > 0)
		{
			// need to receive the header
			std::size_t readHeaderSize = min(packet.GetActiveSize(), m_headerBuffer.GetRemainingSpace());
			//LogFormat("header size %d,pack active size %d",readHeaderSize,packet.GetActiveSize());
			m_headerBuffer.Write(packet.GetReadPointer(), readHeaderSize);
			packet.ReadCompleted(readHeaderSize);

			if (m_headerBuffer.GetRemainingSpace() > 0)
			{
				// Couldn't receive the whole header this time.
				//ASSERT(packet.GetActiveSize() == 0);
				break;
			}

			// We just received nice new header
			if (!ReadHeaderHandler())
			{
				CloseClient(m_pSocket->GetRecvSockAddr());
				return;
			}
		}

		// We have full read header, now check the data payload
		if (m_packetBuffer.GetRemainingSpace() > 0)
		{
			// need more data in the payload
			std::size_t readDataSize = min(packet.GetActiveSize(), m_packetBuffer.GetRemainingSpace());
			m_packetBuffer.Write(packet.GetReadPointer(), readDataSize);
			packet.ReadCompleted(readDataSize);

			if (m_packetBuffer.GetRemainingSpace() > 0)
			{
				// Couldn't receive the whole data this time.
				//ASSERT(packet.GetActiveSize() == 0);
				break;
			}
		}

		// just received fresh new payload
		ReadDataHandlerResult result = ReadDataHandler();
		m_headerBuffer.Reset();
		if (result != ReadDataHandlerResult::Ok)
		{
			if (result != ReadDataHandlerResult::WaitingForQuery)
				CloseClient(m_pSocket->GetRecvSockAddr());

			return;
		}
	}
}

ReadDataHandlerResult SocketServer::ReadDataHandler()
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

bool SocketServer::ReadHeaderHandler()
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(m_headerBuffer.GetReadPointer());

	if (!header->IsValidSize() || !header->IsValidOpcode())
	{
		LogErrorFormat("SocketServer::ReadHeaderHandler(): client %s sent malformed packet (size: %u, cmd: %u)", GetSocketAddrStr(m_pSocket->GetRecvSockAddr()), header->Size, header->Command);
		return false;
	}
	m_packetBuffer.Reset();
	m_packetBuffer.Resize(header->Size);
	//LogFormat("ReadHeaderHandler data size %d",header->Size);
	return true;
}

void SocketServer::SendDataHandler(int size)
{
}

void SocketServer::WriteThread()
{
	while (true)
	{

	}
}

bool SocketServer::CloseClient(const SockAddr_t & client)
{
	auto mc = m_mSocketClients.find(client);

	if (mc != m_mSocketClients.end())
	{
		LogFormat("client %d ,ip :%s,%d is closed by server!", mc->second, GetSocketAddrStr(mc->first),mc->first.port);
		char buff[] = { "disconnected by server" };
		Send(buff, sizeof(buff), mc->second);
		m_mSocketClients.erase(mc);
		return true;
	}
	return false;
}

bool SocketServer::ContainsRemote(const SockAddr_t & remote)
{
	for (auto mc = m_mSocketClients.begin(); mc != m_mSocketClients.end(); mc++)
	{
		//if (mc->first == remote)
		{
			return true;
		}
	}
	return false;
}