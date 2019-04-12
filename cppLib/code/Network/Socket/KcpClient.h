#ifndef KCP_CLIENT_H
#define KCP_CLIENT_H
#include "kcp_config.h"
#include "SocketConfig.h"
#include "CSocket.h"
#include <mutex>
enum ClientResponse
{
	CR_CONNECT_SUCCESS = 0,
	CR_CONNECT_FAIL,
	CR_DISCONNECTED,
};
typedef std::function<void(ClientResponse)> ClientRespanceCallBack;
class KcpClient
{
public:
	KcpClient(int16 sid);
	~KcpClient();
	void SetAddress(const char* ip, int16 port);
	void Connect();
	void Close();
	void Send(char* buff, int length, bool immediately = false);
	void Update(int32_t time);
	bool IsAlive() { return m_nSessionStatus == SessionStatus::Connected; }
	bool IsClosed() { return m_nSessionStatus == SessionStatus::Disconnected; }
	bool IsConnecting() { return m_nSessionStatus == SessionStatus::Connecting; }
	bool IsConnected() { return m_nSessionStatus == SessionStatus::Connected; }
	void SetConnectTimeout(int16 timeout) { m_nConnectTimeOut = timeout; }
public:
	void SetReceiveCallBack(SocketDataReceiveHandler cb) { m_pDataHandler = cb; }
	void SetResponseCallBack(ClientRespanceCallBack cb) { m_pResponce = cb; }
private:
	void OnReceive(const uint8* buff, int length);
	void ReadHandlerInternal(int size, const char * buffer);
	ReadDataHandlerResult ReadDataHandler();
	bool ReadHeaderHandler();
	void ReadHandler();
	void OnDisconnected(bool immediately);
	void OnConnected(bool success);
	bool CheckCrc(const char* buff, int length) { return true; }
	static int fnWriteDgram(const char *buf, int len, ikcpcb *kcp, void *user);
	void SendHeartBeat();
	void OnConnectFailed();
	inline void	OnResponse(ClientResponse response);
private:
	SocketDataReceiveHandler m_pDataHandler;
	ClientRespanceCallBack m_pResponce;
	MessageBuffer m_headerBuffer;
	MessageBuffer m_packetBuffer;
	MessageBuffer m_readBuffer;
	SockAddr_t m_stremote;
	socketSessionId m_stSessionId;
	ikcpcb* m_pKcp;
	Socket* m_pSocket;
	int32 m_nSessionStatus;
	int32 m_nTick;
	uint64 m_nLastSendTime;
	uint64 m_nNeedUpdateTime;
	bool m_bNeedUpdate;
	bool m_bAlive;
	int16 m_nServerId;
	bool m_bRecv;
	int16 m_nConnectTimeOut;
	int16 m_nReconnectTime;
	const static int16 reconnectInterval = 1000;
	std::mutex m_updateMtx;

};
#endif
