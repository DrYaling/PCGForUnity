#include "CSocket.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "TimeSpan.h"
#include <thread>
#include "Logger/Logger.h"
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
#if defined(_WIN32_PLATFROM_)
int sockaddr_Len = sizeof(sockaddr_in);
#else
socklen_t sockaddr_Len = sizeof(sockaddr_in);
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
std::mutex _socket_open_mutex;
SocketHandle SocketOpen(int tcpudp,int family)
{
	std::lock_guard<std::mutex> lck(_socket_open_mutex);
	InitializeSocketEnvironment();
	//LogFormat("SocketOpen %d\n", InitializeSocketEnvironment());
	int protocol = 0;
	SocketHandle hs;
#if defined(_WIN32_PLATFROM_)
	if (tcpudp == SOCK_STREAM) protocol = IPPROTO_TCP;
	else if (tcpudp == SOCK_DGRAM) protocol = IPPROTO_UDP;
#endif
	hs = socket(family, tcpudp, protocol);
#if defined(_WIN32_PLATFROM_)
	if (hs > 0 && tcpudp == SOCK_DGRAM)
	{
		//去掉window的udp 10054错误
		BOOL bNewBehavior = FALSE;
		DWORD dwBytesReturned = 0;
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
		WSAIoctl(hs, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	}
#endif
	//LogFormat("SocketOpen hs %d,tcpudp %d,protocol %d\n", hs, tcpudp, protocol);
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
	if (!ptr || nbytes < 1) return;

	//Linux: flag can be MSG_DONTWAIT, MSG_WAITALL, 使用MSG_WAITALL的时候, socket 必须是处于阻塞模式下，否则WAITALL不能起作用
	rt.nbytes = send(hs, ptr, nbytes, NONBLOCKREADWRITE | SENDNOSIGNAL);
	if (rt.nbytes > 0)
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
	if (!ptr || nbytes < 1) return;

	rt.nbytes = recv(hs, ptr, nbytes, BLOCKREADWRITE);
	if (rt.nbytes > 0)
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
	if (!ptr || nbytes < 1) return;


	int n;
	TimeSpan start;
	while (1)
	{
		n = send(hs, ptr + rt.nbytes, nbytes, NONBLOCKREADWRITE | SENDNOSIGNAL);
		if (n > 0)
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
		if (start.GetSpaninMilliseconds() > milliseconds) { rt.nresult = 1; break; }
	}
}
// if timeout occurs, nbytes=-1, nresult=1
// if socket error, nbyte=-1, nresult=-1
// if the other side has disconnected in either block mode or nonblock mode, nbytes=0, nresult=-1
void SocketTryRecv(SocketHandle hs, char *ptr, int nbytes, int milliseconds, SockError &rt)
{
	rt.nbytes = 0;
	rt.nresult = 0;
	if (!ptr || nbytes < 1) return;

	if (milliseconds > 2)
	{
		TimeSpan start;
		while (1)
		{
			rt.nbytes = recv(hs, ptr, nbytes, NONBLOCKREADWRITE);
			if (rt.nbytes > 0)
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
	while (nRet > 0)
	{
		nRet = select(FD_SETSIZE, &fds, NULL, NULL, &tmOut);
		if (nRet > 0)
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
		if (lingertimeout > -1)
		{
			struct linger  lin;
			lin.l_onoff = lingertimeout;
			lin.l_linger = lingertimeout;
			rt = setsockopt(hs, SOL_SOCKET, SO_DONTLINGER, (const char*)&lin, sizeof(linger)) == 0 ? 0 : 0x1;
		}
		if (recvtimeout > 0 && rt == 0)
		{
			rt = rt | (setsockopt(hs, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvtimeout, sizeof(int)) == 0 ? 0 : 0x2);
		}
		if (sendtimeout > 0 && rt == 0)
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
Socket::Socket(SocketType tp) :
	m_bConnected(false),
	m_bIsServer(false),
	m_socketType(tp),
	m_Socket(INVALIDSOCKHANDLE),
	//m_pSendCallback(nullptr),
	m_pRecvCallback(nullptr)
{
	memset(&m_stAddr, 0, sizeof(sockaddr_in));
	Reopen(false);
}


Socket::~Socket()
{
	SocketClose(m_Socket);
}
int Socket::Bind()
{
	return SocketBind(m_Socket, &m_stAddr);
}
int Socket::Bind(sockaddr_in * addr)
{
	return SocketBind(m_Socket, addr);
}
void Socket::Reopen(bool bForceClose)
{
	//printf_s("Reopen %d\n", m_Socket);
	if (ISSOCKHANDLE(m_Socket) && bForceClose)
		SocketClose(m_Socket);
	if (!ISSOCKHANDLE(m_Socket))
	{
		m_Socket = SocketOpen(m_socketType, AF_INET);
	}
	printf_s("Socket %d\n", m_Socket);
}
void Socket::Close()
{
	m_bConnected = false;
	ClearRecvBuffer();
	SocketClose(m_Socket);
}

void Socket::Connect()
{
	if (m_bIsServer)
	{
		LogError("once socket is set to server,cant change it to client!");
		return;
	}
	int ret = SocketConnect(m_Socket, &m_stAddr);
	//printf_s("Socket Connect ret %d,error %d\n", ret, GetLastSocketError());
	m_bIsServer = false;
	if (ret == 0 && !m_bConnected)
	{
		if (m_eMode == SocketSyncMode::SOCKET_ASYNC)
		{
			std::thread trecv(&Socket::SelectThread, this);
			trecv.detach();
		}
		m_bConnected = true;
	}
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
	if (m_socketType == SocketType::SOCKET_TCP)
	{
		SocketSend(m_Socket, (const char *)ptr, nbytes, rt);
	}
	else
	{
		int ret = sendto(m_Socket, (const char *)ptr, nbytes, 0, (sockaddr*)&m_stAddr, sockaddr_Len);
		rt.nresult = ret > 0 ? 0 : -1;
		rt.nbytes = ret;
	}
	return rt;
}
SockError Socket::SendTo(void *ptr, int nbytes, sockaddr_in& target)
{
	SockError rt;
	if (m_socketType == SocketType::SOCKET_TCP)
	{
		SocketSend(m_Socket, (const char *)ptr, nbytes, rt);
	}
	else
	{
		int ret = sendto(m_Socket, (const char *)ptr, nbytes, 0, (sockaddr*)&target, sockaddr_Len);
		rt.nresult = ret > 0 ? 0 : -1;
		rt.nbytes = ret;
	}
	return rt;
}SockError Socket::SendTo(void *ptr, int nbytes,const SockAddr_t& target)
{
	SockError rt;
	if (m_socketType == SocketType::SOCKET_TCP)
	{
		SocketSend(m_Socket, (const char *)ptr, nbytes, rt);
	}
	else
	{
		m_sendAddr = target.toSockAddr_in(m_sendAddr);
		int ret = sendto(m_Socket, (const char *)ptr, nbytes, 0, (sockaddr*)&m_sendAddr, sockaddr_Len);
		rt.nresult = ret > 0 ? 0 : -1;
		rt.nbytes = ret;
	}
	return rt;
}
SockError Socket::Recv(void *ptr, int nbytes)
{
	SockError rt;
	if (m_socketType == SocketType::SOCKET_TCP)
	{
		SocketRecv(m_Socket, (char *)ptr, nbytes, rt);
	}
	else
	{
		int size = recvfrom(m_Socket, (char *)ptr, nbytes, 0, (sockaddr*)&m_stAddr, &sockaddr_Len);
		if (size > 0)
		{
			rt.nbytes = size;
			rt.nresult = 0;
		}
		else
		{
			rt.nresult = size;
		}
	}
	return rt;
}
SockError Socket::TrySend(void *ptr, int nbytes, int milliseconds)
{
	SockError rt;
	if (m_socketType == SocketType::SOCKET_TCP)
	{
		SocketTrySend(m_Socket, (const char *)ptr, nbytes, milliseconds, rt);
	}
	else
	{
		int ret = sendto(m_Socket, (const char *)ptr, nbytes, 0, (sockaddr*)&m_stAddr, sizeof(sockaddr));
		rt.nresult = ret > 0 ? 0 : -1;
		rt.nbytes = ret;
	}
	return rt;
}
SockError Socket::TryRecv(void *ptr, int nbytes, int  milliseconds)
{
	SockError rt;
	if (m_socketType == SocketType::SOCKET_TCP)
	{
		SocketTryRecv(m_Socket, (char *)ptr, nbytes, milliseconds, rt);
	}
	else
	{
		int size = recvfrom(m_Socket, (char *)ptr, nbytes, 0, (sockaddr*)&m_stAddr, &sockaddr_Len);
		if (size > 0)
		{
			rt.nbytes = size;
			rt.nresult = 0;
		}
		else
		{
			rt.nresult = size;
		}
	}
	return rt;
}

void Socket::ClearRecvBuffer()
{
	SocketClearRecvBuffer(m_Socket);
}

int Socket::StartListen(int maxconn)
{
	int err = 0;
	if (m_socketType == SocketType::SOCKET_TCP)
	{
		err = SocketListen(m_Socket, maxconn);
	}
	if (err == 0 && !m_bConnected)
	{
		std::thread trecv(&Socket::SelectThread, this);
		trecv.detach();
		m_bConnected = true;
	}
	m_bIsServer = true;
	return err;
}


void Socket::OnDisconnected(SocketHandle socket)
{
}

void Socket::OnConnected(SocketHandle socket)
{
}

void Socket::SelectThread()
{
	int max_fd = 1;
	fd_set ser_fdset;
	char* recv_buffer;
	if (nullptr != m_pReadBuffer)
	{
		recv_buffer = (char*)m_pReadBuffer;
	}
	else
	{
		recv_buffer = new char[RECV_BUFFER_SIZE];
	}
	Log("RecvThread start");
	struct timeval mytime = { 3,0 };
	std::vector<SocketHandle> m_vListeners;
	m_vListeners.clear();
	while (m_bConnected)
	{
		FD_ZERO(&ser_fdset);
		if (max_fd < 0)
		{
			max_fd = 0;
		}
		FD_SET(m_Socket, &ser_fdset);
		if (max_fd < m_Socket)
		{
			max_fd = m_Socket;
		}
		if (IsServer() && m_socketType == SocketType::SOCKET_TCP)
		{
			for (auto h : m_vListeners)
			{
				FD_SET(h, &ser_fdset);
				if (max_fd < h)
				{
					max_fd = h;
					LogFormat("select max_fd %d", max_fd);
				}
			}
		}
		//select多路复用
		int ret = select(max_fd + 1, &ser_fdset, nullptr, nullptr, &mytime);
		if (ret < 0)
		{
			Log("select failure\n");
			continue;
		}

		else if (ret == 0)//time out
		{
			//Log("select ret = 0");
			continue;
		}
		else
		{
			/*if (FD_ISSET(0, &ser_fdset)) //标准输入是否存在于ser_fdset集合中（也就是说，检测到输入时，做如下事情）
			{
				//do nothing
			}*/
			//LogFormat("select ret %d", ret);
			//accept 在accept线程
			if (FD_ISSET(m_Socket, &ser_fdset))
			{
				if (IsServer())
				{
					if (m_socketType == SocketType::SOCKET_TCP)
					{
						LogFormat("select ret accept %d", ret);
						sockaddr_in client_address;

#if defined(_WIN32_PLATFROM_)
						int cliaddr_len = sizeof(sockaddr_in);
#else
						socklen_t cliaddr_len = sizeof(sockaddr_in);
#endif
						int client_sock_fd = accept(m_Socket, (struct sockaddr *)&client_address, &cliaddr_len);
						if (client_sock_fd > 0)
						{
							int flags = client_sock_fd;
							m_vListeners.push_back(client_sock_fd);
							SockError err;
							SocketSend(client_sock_fd, "connect success", sizeof("connect success"), err);
							if (err.nresult == 0)
							{
								LogFormat("new user client[%d] add sucessfully!\n", flags);

							}

							else //flags=-1
							{

							}
						}
					}
					else//udp
					{
						memset(&m_stRemoteAddr, 0, sizeof(m_stRemoteAddr));
						int dataSize = recvfrom(m_Socket, recv_buffer, RECV_BUFFER_SIZE, 0, (sockaddr*)&m_stRemoteAddr, &sockaddr_Len);
						if (dataSize > 0)
						{
							//LogFormat("message form client[%s:%d]:%s", inet_ntoa(m_stRemoteAddr.sin_addr), m_stRemoteAddr.sin_port, recv_buffer);
							if (nullptr != m_pRecvCallback)
							{
								m_pRecvCallback(dataSize, recv_buffer);
							}
						}
						else
						{
							LogErrorFormat("recvfrom internal error:%d", GetLastSocketError());
						}
					}

				}
				else//client
				{
					memset(recv_buffer, 0, RECV_BUFFER_SIZE);

					int byte_num = recv(m_Socket, recv_buffer, RECV_BUFFER_SIZE, BLOCKREADWRITE);
					if (byte_num > 0)
					{
						//LogFormat("message form server %d", byte_num);
						if (nullptr != m_pRecvCallback)
						{
							m_pRecvCallback(byte_num, recv_buffer);
						}
					}
					else if (byte_num < 0)
					{
						int socketError = GetLastSocketError();
						LogFormat("recv from server error %d!", socketError);
						//connection is reset
						if (10054 == socketError)
						{
							LogFormat("lost connection to server!\n");
							//disconnected
							Close();
							OnDisconnected(m_Socket);
							return;
						}
					}
					else  //cancel fdset and set fd=0
					{
						LogFormat("server closed!\n");
						Close();
						OnDisconnected(m_Socket);
						return;
					}
				}
			}
			if (IsServer() && m_socketType != SocketType::SOCKET_UDP)//tcp 连接
			{
				for (auto itr = m_vListeners.begin(); itr != m_vListeners.end();)
				{
					SocketHandle client = *itr;
					if (ISSOCKHANDLE(client))
					{
						if (FD_ISSET(client, &ser_fdset))
						{
							memset(recv_buffer, 0, RECV_BUFFER_SIZE);

							int byte_num = recv(client, recv_buffer, RECV_BUFFER_SIZE, BLOCKREADWRITE);
							if (byte_num > 0)
							{
								LogFormat("message form client[%d]:%s", client, recv_buffer);
							}
							else if (byte_num < 0)
							{
								int socketError = GetLastSocketError();
								LogFormat("recv from client %d error %d!", client, socketError);
								//connection is reset
								if (10054 == socketError)
								{
									LogFormat("clien[%d] lost connection!\n", client);
									FD_CLR(client, &ser_fdset);
									SocketClearRecvBuffer(client);
									SocketClose(client);
									itr = m_vListeners.erase(itr);
									continue;
								}
							}

							//某个客户端退出
							else  //cancel fdset and set fd=0
							{
								LogFormat("clien[%d] exit!\n", client);
								FD_CLR(client, &ser_fdset);
								SocketClearRecvBuffer(client);
								SocketClose(client);
								itr = m_vListeners.erase(itr);
								continue;
							}
						}
						itr++;
					}
					else
					{
						LogFormat("Invalid socket handle %d", client);
					}
				}
			}

		}
	}
	if (nullptr != m_pReadBuffer)
	{
		m_pReadBuffer = nullptr;
	}
	else
	{
		delete[] recv_buffer;
	}
}
