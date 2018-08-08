#include "CSocket.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "TimeSpan.h"
//#include "Logger/Logger.h"
#define INVALIDSOCKHANDLE   INVALID_SOCKET

#if defined(_WIN32_PLATFROM_)
#include <windows.h>
#define ISSOCKHANDLE(x)  (x!=INVALID_SOCKET)
#define BLOCKREADWRITE      0
#define NONBLOCKREADWRITE   0
#define SENDNOSIGNAL        0
#define ETRYAGAIN(x)     (x==WSAEWOULDBLOCK||x==WSAETIMEDOUT)
#define gxsprintf   sprintf_s
#else
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define ISSOCKHANDLE(x)    (x>0)
#define BLOCKREADWRITE      MSG_WAITALL
#define NONBLOCKREADWRITE   MSG_DONTWAIT
#define SENDNOSIGNAL        MSG_NOSIGNAL
#define ETRYAGAIN(x)        (x==EAGAIN||x==EWOULDBLOCK)
#define gxsprintf           snprintf
#endif

bool env_inited = false;
void GetAddressFrom(sockaddr_in *addr, const char *ip, int port)
{
	memset(addr, 0, sizeof(sockaddr_in));
	addr->sin_family = AF_INET;            /*地址类型为AF_INET*/
	if (ip)
	{
		addr->sin_addr.s_addr = inet_addr(ip);
	}
	else
	{
		/*网络地址为INADDR_ANY，这个宏表示本地的任意IP地址，因为服务器可能有多个网卡，每个网卡也可能绑定多个IP地址，
		这样设置可以在所有的IP地址上监听，直到与某个客户端建立了连接时才确定下来到底用哪个IP地址*/
		addr->sin_addr.s_addr = htonl(INADDR_ANY);
	}
	addr->sin_port = htons(port);   /*端口号*/
}
void GetIpAddress(char *ip, sockaddr_in *addr)
{
	unsigned char *p = (unsigned char *)(&(addr->sin_addr));
	gxsprintf(ip, 17, "%u.%u.%u.%u", *p, *(p + 1), *(p + 2), *(p + 3));
}

int GetLastSocketError()
{
#if defined(_WIN32_PLATFROM_)
	return WSAGetLastError();
#else
	return errno;
#endif
}

bool IsValidSocketHandle(SocketHandle handle)
{
	return ISSOCKHANDLE(handle);
}

void SocketClose(SocketHandle &handle)
{
	if (ISSOCKHANDLE(handle))
	{
#if defined(_WIN32_PLATFROM_)
		closesocket(handle);
#else
		close(handle);
#endif
		handle = INVALIDSOCKHANDLE;
	}
}

int SocketConnect(SocketHandle &handle, sockaddr_in *addr)
{
	if (ISSOCKHANDLE(handle))
	{
		return connect(handle, (sockaddr*)addr, sizeof(sockaddr_in));
	}
	else
	{
		return -1;
	}
}

const char* GetSocketError(int r)
{
	switch (r)
	{
	case 1:
		return "TIME_OUT";
	case -1:
		return "SOCKET_ERROR";
	default:
		break;
	}
	return "SUCCESS";
}

SocketHandle SocketOpen(int tcpudp)
{
	printf_s("SocketOpen %d", InitializeSocketEnvironment());
	int protocol = 0;
	SocketHandle hs;
#if defined(_WIN32_PLATFROM_)
	if (tcpudp == SOCK_STREAM) protocol = IPPROTO_TCP;
	else if (tcpudp == SOCK_DGRAM) protocol = IPPROTO_UDP;
#endif
	hs = socket(AF_INET, tcpudp, protocol);
	printf_s("SocketOpen hs %d,tcpudp %d,protocol %d\n", hs, tcpudp, protocol);
	return hs;
}
int SocketBind(SocketHandle hs, sockaddr_in *paddr)
{
	return bind(hs, (struct sockaddr *)paddr, sizeof(sockaddr_in));
}
int SocketListen(SocketHandle hs, int maxconn)
{
	return listen(hs, maxconn);
}
SocketHandle SocketAccept(SocketHandle hs, sockaddr_in *paddr)
{
#if defined(_WIN32_PLATFROM_)
	int cliaddr_len = sizeof(sockaddr_in);
#else
	socklen_t cliaddr_len = sizeof(sockaddr_in);
#endif
	return accept(hs, (struct sockaddr *)paddr, &cliaddr_len);
}
//
// if timeout occurs, nbytes=-1, nresult=1
// if socket error, nbyte=-1, nresult=-1
// if the other side has disconnected in either block mode or nonblock mode, nbytes=0, nresult=-1
// otherwise nbytes= the count of bytes sent , nresult=0
void SocketSend(SocketHandle hs, const char *ptr, int nbytes, SockError &rt)
{
	rt.nbytes = 0;
	rt.nresult = 0;
	if (!ptr || nbytes<1) return;

	//Linux: flag can be MSG_DONTWAIT, MSG_WAITALL, 使用MSG_WAITALL的时候, socket 必须是处于阻塞模式下，否则WAITALL不能起作用
	rt.nbytes = send(hs, ptr, nbytes, NONBLOCKREADWRITE | SENDNOSIGNAL);
	if (rt.nbytes>0)
	{
		rt.nresult = (rt.nbytes == nbytes) ? 0 : 1;
	}
	else if (rt.nbytes == 0)
	{
		rt.nresult = -1;
	}
	else
	{
		rt.nresult = GetLastSocketError();
		rt.nresult = ETRYAGAIN(rt.nresult) ? 1 : -1;
	}
}



// if timeout occurs, nbytes=-1, nresult=1
// if socket error, nbyte=-1, nresult=-1
// if the other side has disconnected in either block mode or nonblock mode, nbytes=0, nresult=-1
void SocketRecv(SocketHandle hs, char *ptr, int nbytes, SockError &rt)
{
	rt.nbytes = 0;
	rt.nresult = 0;
	if (!ptr || nbytes<1) return;

	rt.nbytes = recv(hs, ptr, nbytes, BLOCKREADWRITE);
	if (rt.nbytes>0)
	{
		return;
	}
	else if (rt.nbytes == 0)
	{
		rt.nresult = -1;
	}
	else
	{
		rt.nresult = GetLastSocketError();
		rt.nresult = ETRYAGAIN(rt.nresult) ? 1 : -1;
	}

}
//  nbytes= the count of bytes sent
// if timeout occurs, nresult=1
// if socket error,  nresult=-1,
// if the other side has disconnected in either block mode or nonblock mode, nresult=-2
void SocketTrySend(SocketHandle hs, const char *ptr, int nbytes, int milliseconds, SockError &rt)
{
	rt.nbytes = 0;
	rt.nresult = 0;
	if (!ptr || nbytes<1) return;


	int n;
	TimeSpan start;
	while (1)
	{
		n = send(hs, ptr + rt.nbytes, nbytes, NONBLOCKREADWRITE | SENDNOSIGNAL);
		if (n>0)
		{
			rt.nbytes += n;
			nbytes -= n;
			if (rt.nbytes >= nbytes) { rt.nresult = 0;  break; }
		}
		else if (n == 0)
		{
			rt.nresult = -2;
			break;
		}
		else
		{
			n = GetLastSocketError();
			if (ETRYAGAIN(n))
			{
				printf_s("SocketTrySend time out %d\n", n);
			}
			else
			{
				rt.nresult = -1;
				break;
			}
		}
		if (start.GetSpaninMilliseconds()>milliseconds) { rt.nresult = 1; break; }
	}
}
// if timeout occurs, nbytes=-1, nresult=1
// if socket error, nbyte=-1, nresult=-1
// if the other side has disconnected in either block mode or nonblock mode, nbytes=0, nresult=-1
void SocketTryRecv(SocketHandle hs, char *ptr, int nbytes, int milliseconds, SockError &rt)
{
	rt.nbytes = 0;
	rt.nresult = 0;
	if (!ptr || nbytes<1) return;

	if (milliseconds>2)
	{
		TimeSpan start;
		while (1)
		{
			rt.nbytes = recv(hs, ptr, nbytes, NONBLOCKREADWRITE);
			if (rt.nbytes>0)
			{
				break;
			}
			else if (rt.nbytes == 0)
			{
				rt.nresult = -1;
				break;
			}
			else
			{
				rt.nresult = GetLastSocketError();
				if (ETRYAGAIN(rt.nresult))
				{
					if (start.GetSpaninMilliseconds() > milliseconds) { rt.nresult = 1; break; }
					printf_s("SocketTryRecv time out %d\n", rt.nresult);
				}
				else
				{
					rt.nresult = -1;
					break;
				}
			}

		}
	}
	else
	{
		SocketRecv(hs, ptr, nbytes, rt);
	}
}

void SocketClearRecvBuffer(SocketHandle hs)
{
#if defined(_WIN32_PLATFROM_)
	struct timeval tmOut;
	tmOut.tv_sec = 0;
	tmOut.tv_usec = 0;
	fd_set    fds;
	FD_ZERO(&fds);
	FD_SET(hs, &fds);
	int   nRet = 1;
	char tmp[100];
	int rt;
	while (nRet>0)
	{
		nRet = select(FD_SETSIZE, &fds, NULL, NULL, &tmOut);
		if (nRet>0)
		{
			nRet = recv(hs, tmp, 100, 0);
		}
	}
#else
	char tmp[100];
	while (recv(hs, tmp, 100, NONBLOCKREADWRITE) > 0);
#endif
}

int SocketBlock(SocketHandle hs, bool bblock)
{
	unsigned long mode;
	if (ISSOCKHANDLE(hs))
	{
#if defined(_WIN32_PLATFROM_)
		mode = bblock ? 0 : 1;
		return ioctlsocket(hs, FIONBIO, &mode);
#else
		mode = fcntl(hs, F_GETFL, 0);                  //获取文件的flags值。
													   //设置成阻塞模式      非阻塞模式
		return bblock ? fcntl(hs, F_SETFL, mode&~O_NONBLOCK) : fcntl(hs, F_SETFL, mode | O_NONBLOCK);
#endif
	}
	return -1;
}

int SocketTimeOut(SocketHandle hs, int recvtimeout, int sendtimeout, int lingertimeout)   //in milliseconds
{
	int rt = -1;
	if (ISSOCKHANDLE(hs))
	{
		rt = 0;
#if defined(_WIN32_PLATFROM_)
		if (lingertimeout>-1)
		{
			struct linger  lin;
			lin.l_onoff = lingertimeout;
			lin.l_linger = lingertimeout;
			rt = setsockopt(hs, SOL_SOCKET, SO_DONTLINGER, (const char*)&lin, sizeof(linger)) == 0 ? 0 : 0x1;
		}
		if (recvtimeout>0 && rt == 0)
		{
			rt = rt | (setsockopt(hs, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvtimeout, sizeof(int)) == 0 ? 0 : 0x2);
		}
		if (sendtimeout>0 && rt == 0)
		{
			rt = rt | (setsockopt(hs, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendtimeout, sizeof(int)) == 0 ? 0 : 0x4);
		}
#else
		struct timeval timeout;
		if (lingertimeout > -1)
		{
			struct linger  lin;
			lin.l_onoff = lingertimeout > 0 ? 1 : 0;
			lin.l_linger = lingertimeout / 1000;
			rt = setsockopt(hs, SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(linger)) == 0 ? 0 : 0x1;
		}
		if (recvtimeout > 0 && rt == 0)
		{
			timeout.tv_sec = recvtimeout / 1000;
			timeout.tv_usec = (recvtimeout % 1000) * 1000;
			rt = rt | (setsockopt(hs, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0 ? 0 : 0x2);
		}
		if (sendtimeout > 0 && rt == 0)
		{
			timeout.tv_sec = sendtimeout / 1000;
			timeout.tv_usec = (sendtimeout % 1000) * 1000;
			rt = rt | (setsockopt(hs, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == 0 ? 0 : 0x4);
		}
#endif
	}
	return rt;
}


int InitializeSocketEnvironment()
{
	if (env_inited)
	{
		return 0;
	}
#if defined(_WIN32_PLATFROM_)
	WSADATA  Ws;
	//Init Windows Socket
	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
	{
		return -1;
	}
#endif
	env_inited = true;
	return 0;
}
void FreeSocketEnvironment()
{
	if (!env_inited)
	{
		return;
	}
#if defined(_WIN32_PLATFROM_)
	WSACleanup();
#endif
	env_inited = false;
}

//==============================================================================================================
//================================================================================================================
Socket::Socket(SocketType tp)
{
	memset(&m_stAddr, 0, sizeof(sockaddr_in));
	m_socketType = tp;
	m_Socket = INVALIDSOCKHANDLE;
	Reopen(false);
}


Socket::~Socket()
{
	SocketClose(m_Socket);
}
void Socket::Reopen(bool bForceClose)
{
	printf_s("Reopen %d\n", m_Socket);
	if (ISSOCKHANDLE(m_Socket) && bForceClose) 
		SocketClose(m_Socket);
	if (!ISSOCKHANDLE(m_Socket))
	{
		m_Socket = SocketOpen(m_socketType);
	}
	printf_s("Reopen %d\n", m_Socket);
}
void Socket::Close()
{
	SocketClose(m_Socket);
	ClearRecvBuffer();	
}

void Socket::Connect()
{
	int ret = SocketConnect(m_Socket, &m_stAddr);
	printf_s("Socket Connect ret %d,error %d", ret,GetLastSocketError());
}
void Socket::Connect(const char* ip, int port)
{
	this->SetAddress(ip, port);
	this->Connect();
}


void Socket::SetAddress(const char *ip, int port)
{
	GetAddressFrom(&m_stAddr, ip, port);
}
void Socket::SetAddress(sockaddr_in *addr)
{
	memcpy(&m_stAddr, addr, sizeof(sockaddr_in));
}

int Socket::SetTimeOut(int recvtimeout, int sendtimeout, int lingertimeout)   //in milliseconds
{
	return SocketTimeOut(m_Socket, recvtimeout, sendtimeout, lingertimeout);
}

int Socket::SetBufferSize(int recvbuffersize, int sendbuffersize)   //in bytes
{
	int rt = -1;
	if (ISSOCKHANDLE(m_Socket))
	{
#if defined(_WIN32_PLATFROM_)
		if (recvbuffersize > -1)
		{
			rt = setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (const char*)&recvbuffersize, sizeof(int));
		}
		if (sendbuffersize > -1)
		{
			rt = rt | (setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (char *)&sendbuffersize, sizeof(int)) == 0 ? 0 : 0x2);
		}
#endif
	}
	return rt;
}

int Socket::SetBlock(bool bblock)
{
	return SocketBlock(m_Socket, bblock);
}
SockError Socket::Send(void *ptr, int nbytes)
{
	SockError rt;
	SocketSend(m_Socket, (const char *)ptr, nbytes, rt);
	return rt;
}
SockError Socket::Recv(void *ptr, int nbytes)
{
	SockError rt;
	SocketRecv(m_Socket, (char *)ptr, nbytes, rt);
	return rt;
}
SockError Socket::TrySend(void *ptr, int nbytes, int milliseconds)
{
	SockError rt;
	SocketTrySend(m_Socket, (const char *)ptr, nbytes, milliseconds, rt);
	return rt;
}
SockError Socket::TryRecv(void *ptr, int nbytes, int  milliseconds)
{
	SockError rt;
	SocketTryRecv(m_Socket, (char *)ptr, nbytes, milliseconds, rt);
	return rt;
}

void Socket::ClearRecvBuffer()
{
	SocketClearRecvBuffer(m_Socket);
}
