#ifndef KCP_SESSION_H
#define KCP_SESSION_H
#include "kcp_config.h"
#include "SocketConfig.h"
#include <functional>
#include <mutex>
#define SESSION_KEEP_ALIVE_TIME 120000	//2mins
class KcpSession
{
	friend class SocketServer;
public:
	KcpSession(IUINT32 conv,const SockAddr_t& addr);
	~KcpSession();
	void Close();
	void Send(char* buff, int length,bool immediately = false);
	void OnReceive(const uint8* buff, int length);
	void Update(int32_t time);
	bool IsDead() { return !m_bAlive && m_nTick >= SESSION_KEEP_ALIVE_TIME; }
	bool IsAlive() { return m_nSessionStatus == SessionStatus::Connected; }
	bool IsClosed() { return m_nSessionStatus == SessionStatus::Disconnected; }
	bool IsConnecting() { return m_nSessionStatus == SessionStatus::Connecting; }
	void Disconnect();
	inline const socketSessionId& GetSessionId() { return m_stSessionId; }
public:
	void SetPacketReceivedCallback(std::function<void(KcpSession*)> callback);
	void SetConnectFinishedCallback(std::function<void(KcpSession*)> callback);
	void SetDisconnectedCallback(std::function<void(KcpSession*)> callback);
private:
	ReadDataHandlerResult ReadDataHandler();
	bool ReadHeaderHandler();
	void ReadHandler();
	void OnDisconnected(bool immediately);
	void OnConnected(IUINT32 conv,const SockAddr_t& addr);
	void OnConnected();
	static int fnWriteDgram(const char *buf, int len, ikcpcb *kcp, void *user);
	void SendHeartBeat();
private:
	std::function<void(KcpSession*)> m_cbPacketReceived;
	std::function<void(KcpSession*)> m_cbConnectFinished;
	std::function<void(KcpSession*)> m_cbDisconnected;
private:
	SocketDataReceiveHandler m_pDataHandler;
	MessageBuffer m_headerBuffer;
	MessageBuffer m_packetBuffer;
	MessageBuffer m_readBuffer;
	SockAddr_t m_stremote;
	socketSessionId m_stSessionId;
	ikcpcb* m_pKcp;
	int32 m_nSessionStatus;
	int32 m_nTick;
	uint64 m_nLastSendTime;
	uint64 m_nNeedUpdateTime;
	bool m_bNeedUpdate;
	bool m_bAlive;
	std::mutex m_updateMtx;
};

#endif
