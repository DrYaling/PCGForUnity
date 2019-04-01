#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H
#include <map>
#include <queue>
#include "CSocket.h"
#include "Network\Socket\MessageBuffer.h"
#include <condition_variable>    // std::condition_variable
#include "WriteBufferQueue.h"
#include "KcpSession.h"
class ConditionNotifier;
class SocketServer;
static SocketServer* m_pInstance;
#define sSocketServer SocketServer::GetInstance()
#define Opcode_Accept_Cmd 0
class SocketServer {
public:
	static SocketServer* GetInstance() {
		if (nullptr == m_pInstance)
		{
			m_pInstance = new SocketServer(SocketType::SOCKET_UDP);
		}
		return m_pInstance;
	}
	static void Destroy() {
		if (nullptr != m_pInstance)
		{
			m_pInstance->Stop();
			delete m_pInstance;
		}
	}
	SocketServer(SocketType sock);
	~SocketServer();
	KcpSession* CreateSession(IUINT32 conv, const SockAddr_t& addr);
	bool StartUp();
	bool Stop();
	bool SetAddress(const char * ip, unsigned short port);
	size_t GetClientCount() {
		return m_mSessions.size();
	}
	void Update();
	int32 Send(const char* data, int32 dataSize, const socketSessionId& session);
	int16 GetMTU() { return m_nMTU; }
	void SetMTU(int16 mtu) { m_nMTU = mtu; }
	bool CloseSession(const SockAddr_t& client);
	bool CloseSession(KcpSession* session, bool remove = false);
	std::map<IUINT32,KcpSession*>::iterator OnSessionDead(KcpSession* session);
	bool CheckCrc(const char* buff, int length);
	int16 GetServerId() { return m_nServerId; }
protected:
	void ReadHandlerInternal(int size, const char* buffer);
	void ReadHandler();
	bool SendDataHandler();
	int16 GetNewConv() { return ++m_nClientConv; }
private:
	SocketServer() {}
	MessageBuffer& GetReadBuffer() { return m_readBuffer; }
	KcpSession* ContainsRemote(const SockAddr_t& remote, IUINT32 conv);
	KcpSession* GetSessionByRemote(const SockAddr_t& remote);
	//查找正在等待连接的session
	MessageBuffer* GetWaitingConnectSession(const SockAddr_t& remote);
	bool AcceptPack(const uint8* buff, int length);
	KcpSession* OnAccept(const SockAddr_t& remote);
private:
	Socket * m_pSocket;
	ConditionNotifier* m_pSendNotifier;
	WriteBufferQueue m_writeQueue;
	/*MessageBuffer m_headerBuffer;
	MessageBuffer m_packetBuffer;*/
	MessageBuffer m_readBuffer;
	std::mutex _sendLock;
	std::map<IUINT32, KcpSession*> m_mSessions;
	std::map<IUINT32, KcpSession*> m_mSleepSessions;
	std::vector<std::map<IUINT32, KcpSession*>::iterator> m_vClosingSessions;
	std::map<SockAddr_t, MessageBuffer> m_mWaitingForConnections;
	int16 m_nClientConv;
	int16 m_nMTU;
	int16 m_nServerId;
};
typedef std::function<bool(void)> ContionThreadRunner;
class ConditionNotifier {
public:
	ConditionNotifier(ContionThreadRunner runner) :m_pRunner(runner), m_bRunning(true) {}
	void Start();
	void Notify();
	void Exit();
private:
	void Run();
	std::mutex m_mtx;
	std::condition_variable m_condition;
	ContionThreadRunner m_pRunner;
	bool m_bRunning;
};
#endif // !SOCKET_SERVER_H
