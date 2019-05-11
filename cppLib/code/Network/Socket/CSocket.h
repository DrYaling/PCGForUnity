#ifndef _TCP_SOCKET_H
#define _TCP_SOCKET_H
#include "define.h"
#include "SocketConfig.h"
#include <mutex>

int InitializeSocketEnvironment();
void FreeSocketEnvironment();
void GetAddressFrom(sockaddr_in *addr, const char *ip, int port);
void GetIpAddress(char *ip, sockaddr_in *addr);
bool IsValidSocketHandle(SocketHandle handle);
int GetLastSocketError();

SocketHandle SocketOpen(int tcpudp, int family);
void SocketClose(SocketHandle &handle);
int SocketConnect(SocketHandle &handle, sockaddr_in *addr);
int SocketBlock(SocketHandle hs, bool bblock);
int SocketTimeOut(SocketHandle hs, int recvtimeout, int sendtimeout, int lingertimeout);
int SocketBind(SocketHandle hs, sockaddr_in *addr);
SocketHandle SocketAccept(SocketHandle hs, sockaddr_in *addr);
int SocketListen(SocketHandle hs, int maxconn);

void SocketSend(SocketHandle hs, const char *ptr, int nbytes, SockError &rt);
void SocketRecv(SocketHandle hs, char *ptr, int nbytes, SockError &rt);
void SocketTryRecv(SocketHandle hs, char *ptr, int nbytes, int milliseconds, SockError &rt);
void SocketTrySend(SocketHandle hs, const char *ptr, int nbytes, int milliseconds, SockError &rt);

void SocketClearRecvBuffer(SocketHandle hs);
enum SocketType
{
	SOCKET_TCP = SOCK_STREAM,
	SOCKET_UDP = SOCK_DGRAM,
};
enum SocketSyncMode
{
	SOCKET_SYNC,//同步模式
	SOCKET_ASYNC//异步模式
};

typedef std::function<void(int, const char*)> SocketRecvCallBack;
//for udp mainly
class Socket {
public:
	Socket(SocketType tp);
	~Socket();
	int Bind();
	int Bind(sockaddr_in *addr);
	void Connect();
	void Connect(const char* ip, int port);
	void SetAddress(const char *ip, int port);
	void SetAddress(sockaddr_in *addr);
	int SetTimeOut(int recvtimeout, int sendtimeout, int lingertimeout);
	int SetBufferSize(int recvbuffersize, int sendbuffersize);
	int SetBlock(bool bblock);

	SocketHandle  GetHandle() { return m_Socket; }
	void Reopen(bool bForceClose);
	void Close();
	SockError Send(void *ptr, int nbytes);
	//udp 阻塞模式下sendto不会阻塞
	SockError SendTo(void *ptr, int nbytes, sockaddr_in& target);
	//udp 阻塞模式下sendto不会阻塞
	SockError SendTo(void *ptr, int nbytes, const SockAddr_t& target);
	SockError Recv(void *ptr, int nbytes);
	SockError TrySend(void *ptr, int nbytes, int milliseconds);
	SockError TryRecv(void *ptr, int nbytes, int  milliseconds);
	void ClearRecvBuffer();
	bool Connected() const { return m_bConnected; }
	int StartListen(int maxconn = 1024);
	//异步模式会设置socket为阻塞模式
	void SetSocketMode(SocketSyncMode mode) { m_eMode = mode; SetBlock(true); }
	SocketSyncMode GetSocketMode() const { return m_eMode; }

	void SetRecvCallback(SocketRecvCallBack receive) { m_pRecvCallback = receive; }
	bool IsServer() const { return m_bIsServer; }
	bool IsSockAddr(const sockaddr_in& addr) const
	{
		return addr.sin_addr.S_un.S_addr == m_stRemoteAddr.sin_addr.S_un.S_addr && addr.sin_port == m_stRemoteAddr
			.sin_port;
	}
	bool IsSockAddr(const SockAddr_t& addr) const
	{
		return addr.addr == m_stRemoteAddr.sin_addr.S_un.S_addr && addr.port == m_stRemoteAddr
			.sin_port;
	}
	const sockaddr_in& GetRecvSockAddr() const
	{
		return m_stRemoteAddr;
	}
	SockAddr_t GetRecvSockAddr_t() const
	{
		return SockAddr_t(m_stRemoteAddr);
	}
	const sockaddr_in& GetSocketAddr() const
	{
		return m_stAddr;
	}
	SockAddr_t GetSocketAddr_t() const
	{
		return SockAddr_t(m_stAddr);
	}
	void SetReadBuffer(void* buffer) { m_pReadBuffer = buffer; }
private:
	void OnDisconnected(SocketHandle socket);
	void OnConnected(SocketHandle socket);
	void SelectThread();
	Socket() = delete;
protected:
	//SocketSendCallBack m_pSendCallback;
	SocketRecvCallBack m_pRecvCallback;
	void* m_pReadBuffer;
	SocketHandle  m_Socket;
	sockaddr_in m_stAddr;
	sockaddr_in m_sendAddr;
	sockaddr_in m_stRemoteAddr;
	SocketType m_socketType;
	SocketSyncMode m_eMode;
	bool m_bConnected;
	bool m_bIsServer;

};
#endif

