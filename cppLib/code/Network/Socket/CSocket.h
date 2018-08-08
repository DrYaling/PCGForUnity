#ifndef _TCP_SOCKET_H
#define _TCP_SOCKET_H
#include "define.h"
#include "SocketConfig.h"


int InitializeSocketEnvironment();
void FreeSocketEnvironment();
void GetAddressFrom(sockaddr_in *addr, const char *ip, int port);
void GetIpAddress(char *ip, sockaddr_in *addr);
bool IsValidSocketHandle(SocketHandle handle);
int GetLastSocketError();

SocketHandle SocketOpen(int tcpudp);
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
class Socket {
public :
	Socket(SocketType tp);
	~Socket();
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
	SockError Recv(void *ptr, int nbytes);
	SockError TrySend(void *ptr, int nbytes, int milliseconds);
	SockError TryRecv(void *ptr, int nbytes, int  milliseconds);
	void ClearRecvBuffer();

protected:
	SocketHandle  m_Socket;
	sockaddr_in m_stAddr;
	SocketType m_socketType;

};
#endif

