#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H
#include <map>
#include <queue>
#include "CSocket.h"
#include "Network\Socket\MessageBuffer.h"
#include <condition_variable>    // std::condition_variable
#include "WriteBufferQueue.h"
#include "KcpSession.h"
#include <memory>
#include <atomic>
#include "Threading/threading.h"
#include "Threading/ConditionNotifier.h"
class ConditionNotifier;
class SocketServer;
#define Opcode_Accept_Cmd 0
typedef std::function<void(std::shared_ptr<KcpSession>, uint32_t)> OnAcceptSessionHandle;
class SocketServer :public std::enable_shared_from_this<SocketServer> {
public:
	static std::shared_ptr<SocketServer> GetInstance();
	static void Destroy();
	SocketServer(SocketType sock);
	~SocketServer();
	std::shared_ptr<KcpSession> CreateSession(IUINT32 conv, const SockAddr_t& addr);
	bool StartUp();
	bool Stop();
	bool SetAddress(const char * ip, unsigned short port) const;
	size_t GetClientCount() {
		return m_mSessions.size();
	}
	void Update(uint32_t diff);
	int32 Send(const char* data, int32 dataSize, const socketSessionId& session);
	int16 GetMTU() { return m_nMTU; }
	void SetMTU(int16 mtu) { m_nMTU = mtu; }
	bool CloseSession(const SockAddr_t& client);
	bool CloseSession(uint32_t session, bool remove = false);
	std::map<IUINT32, std::shared_ptr<KcpSession>>::iterator OnSessionDead(std::shared_ptr<KcpSession> session);
	bool CheckCrc(const char* buff, int length);
	uint16_t GetServerId() { return m_nServerId; }
	void SetServerId(uint16_t sid) { m_nServerId = sid; }
	void SetAcceptSessionHandle(OnAcceptSessionHandle handle) { m_cbAcceptHandle = handle; }
protected:
	void ReadHandlerInternal(int size, const char* buffer);
	void ReadHandler();
	int16 GetNewConv() { return ++m_nClientConv; }
private:
	SocketServer() {}
	MessageBuffer& GetReadBuffer() { return m_readBuffer; }
	std::shared_ptr<KcpSession> ContainsRemote(const SockAddr_t& remote, IUINT32 conv);
	std::shared_ptr<KcpSession> GetSessionByRemote(const SockAddr_t& remote);
	std::shared_ptr<KcpSession> GetSleepSessionByRemote(const SockAddr_t& remote);
	//查找正在等待连接的session
	MessageBuffer* GetWaitingConnectSession(const SockAddr_t& remote);
	bool AcceptPack(const uint8* buff, int length);
	std::shared_ptr<KcpSession> OnAccept(const SockAddr_t& remote);
	//in same thread as SendDataHandler,so no need to use mutex
	bool IsSendQueueNotEmpty() const
	{
		return m_pSocket ? m_pSocket->Connected() && !m_writeQueue.GetActiveSize() : false;
	}
	void SendDataHandler();
private:
	Socket * m_pSocket;
	threading::ConditionNotifier* m_pSendNotifier;
	WriteBufferQueue m_writeQueue;
	OnAcceptSessionHandle m_cbAcceptHandle;
	/*MessageBuffer m_headerBuffer;
	MessageBuffer m_packetBuffer;*/
	MessageBuffer m_readBuffer;
	std::mutex _sendLock;
	std::mutex _sessionLock;
	std::map<IUINT32, std::shared_ptr<KcpSession>> m_mSessions;
	std::map<IUINT32, std::shared_ptr<KcpSession>> m_mSleepSessions;
	std::map<SockAddr_t, MessageBuffer> m_mWaitingForConnections;
	int16 m_nClientConv;
	int16 m_nMTU;
	uint16_t m_nServerId;
};
#define sSocketServer SocketServer::GetInstance()
#endif // !SOCKET_SERVER_H
