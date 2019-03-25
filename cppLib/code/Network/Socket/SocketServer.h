#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H
#include <map>
#include <queue>
#include "CSocket.h"
#include "Network\Socket\MessageBuffer.h"
#include <condition_variable>    // std::condition_variable
#include "WriteBufferQueue.h"
typedef std::function<bool(int, uint8*,int)> SocketDataReceiveHandler;
class ConditionNotifier;
class SocketServer {
public:
	SocketServer(SocketType sock);
	~SocketServer();
	bool StartUp();
	bool Stop();
	bool SetAddress(const char * ip,unsigned short port);
	int GetClientCount() {
		return m_mSocketClients.size();
	}
	void Update();
	bool Send(char* data, int32 dataSize, int client);
	int16 GetMTU() { return m_nMTU; }
	void SetMTU(int16 mtu) { m_nMTU = mtu; }
protected:
	void ReadHandlerInternal(int size, const char* buffer);
	void ReadHandler();
	ReadDataHandlerResult ReadDataHandler();
	bool ReadHeaderHandler();
	bool SendDataHandler();
	int GetNewClientId() { return ++m_nClientIndex; }
private:
	SocketServer() {}
	MessageBuffer& GetReadBuffer() { return m_readBuffer; }
	bool CloseClient(const SockAddr_t& client);
	bool ContainsRemote(const SockAddr_t& remote);
private:
	Socket * m_pSocket;
	ConditionNotifier* m_pSendNotifier;
	WriteBufferQueue m_writeQueue;
	MessageBuffer m_headerBuffer;
	MessageBuffer m_packetBuffer;
	MessageBuffer m_readBuffer;
	std::mutex _sendLock;
	std::map<SockAddr_t, int16> m_mSocketClients;
	SocketDataReceiveHandler m_pDataHandler;
	int m_nClientIndex;
	int16 m_nMTU;
};
typedef std::function<bool(void)> ContionThreadRunner;
class ConditionNotifier {
public:
	ConditionNotifier(ContionThreadRunner runner) :m_pRunner(runner), m_bRunning(true){}
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
