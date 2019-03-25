/************************************************************************/
/* surrport platform :windows/linux/android/macos/ios                   */
/************************************************************************/
#ifndef _SOCKET_CONFIG_H
#define _SOCKET_CONFIG_H
#include "MessageBuffer.h"
#if WIN32 
#define _WIN32_PLATFROM_
#pragma comment(lib,"ws2_32.lib")
#endif

#if defined(_WIN32_PLATFROM_)
#include <winsock2.h>
#include <WS2tcpip.h>
typedef SOCKET SocketHandle;
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef int SocketHandle;
#define SOCKET_ERROR  (-1)
#define INVALID_SOCKET  0
#endif


typedef struct
{
	int block;
	int sendbuffersize;
	int recvbuffersize;
	int lingertimeout;
	int recvtimeout;
	int sendtimeout;
} socketoption_t;

typedef struct
{
	int nbytes;
	int nresult;
} SockError;

typedef struct SockAddr_s
{
#if(_WIN32_WINNT < 0x0600)
	short   family;
#else //(_WIN32_WINNT < 0x0600)
	ADDRESS_FAMILY family;
#endif //(_WIN32_WINNT < 0x0600)

	USHORT port;
	ULONG addr;
	SockAddr_s(const sockaddr_in& in) :addr(in.sin_addr.S_un.S_addr),port(in.sin_port), family(in.sin_family){}
	bool operator == (const SockAddr_s& right) const { return addr == right.addr && port == right.port; }
	bool operator < (const SockAddr_s& right) const { return addr < right.addr; }
	bool Equals(const SockAddr_s& right) const { return addr == right.addr && port == right.port; }
	sockaddr_in toSockAddr_in() {
		sockaddr_in in;
		in.sin_addr.S_un.S_addr = addr;
		in.sin_port = port;
		in.sin_family = family;
		return in;
	}
	sockaddr_in& toSockAddr_in(sockaddr_in& in) const{
		in.sin_addr.S_un.S_addr = addr;
		in.sin_port = port;
		in.sin_family = family;
		return in;
	}
	 sockaddr_in toCSockAddr_in()const {
		sockaddr_in in;
		in.sin_addr.S_un.S_addr = addr;
		in.sin_port = port;
		in.sin_family = family;
		return in;
	}
} SockAddr_t;

enum class ReadDataHandlerResult
{
	Ok = 0,
	Error = 1,
	WaitingForQuery = 2
}; 
struct PacketHeader
{
	uint32 Size;
	uint16 Command;

	bool IsValidSize() { return Size < 0x10000; }
	bool IsValidOpcode()
	{
		return Command < 1024;
	}
};
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#endif

inline  char* GetSocketAddrStr(const sockaddr_in& in) {
	return inet_ntoa(in.sin_addr);
}
inline  char* GetSocketAddrStr(const SockAddr_t& in) {
	auto addr = in.toCSockAddr_in();
	return inet_ntoa(addr.sin_addr);
}
#endif