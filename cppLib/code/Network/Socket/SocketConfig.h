/************************************************************************/
/* surrport platform :windows/linux/android/macos/ios                   */
/************************************************************************/
#ifndef _SOCKET_CONFIG_H
#define _SOCKET_CONFIG_H

#include "MessageBuffer.h"
#if WIN32 || MSVC
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
	SockAddr_s() :port(0), addr(0) {}
	SockAddr_s(const sockaddr_in& in) :addr(in.sin_addr.S_un.S_addr), port(in.sin_port), family(in.sin_family) {}
	SockAddr_s(const SockAddr_s& in) :addr(in.addr), port(in.port), family(in.family) {}
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

#define  RECV_BUFFER_SIZE 4096
inline  char* GetSocketAddrStr(const sockaddr_in& in) {
	return inet_ntoa(in.sin_addr);
}
inline  char* GetSocketAddrStr(const SockAddr_t& in) {
	auto addr = in.toCSockAddr_in();
	return inet_ntoa(addr.sin_addr);
}
#include "ikcp.h"

/* encode 8 bits unsigned int */
static inline char *utils_encode8u(char *p, unsigned char c)
{
	*(unsigned char*)p = c;
	return p;
}

/* decode 8 bits unsigned int */
static inline const char *utils_decode8u(const char *p, unsigned char *c)
{
	*c = *(unsigned char*)p;
	return p;
}

/* encode 16 bits unsigned int (lsb) */
static inline char *utils_encode16u(char *p, unsigned short w)
{
#if IWORDS_BIG_ENDIAN
	*(unsigned char*)(p + 0) = (w & 255);
	*(unsigned char*)(p + 1) = (w >> 8);
#else
	*(unsigned short*)(p) = w;
#endif
	return p;
}

/* decode 16 bits unsigned int (lsb) */
static inline const char *utils_decode16u(const char *p, unsigned short *w)
{
#if IWORDS_BIG_ENDIAN
	*w = *(const unsigned char*)(p + 1);
	*w = *(const unsigned char*)(p + 0) + (*w << 8);
#else
	*w = *(const unsigned short*)p;
#endif
	return p;
}

/* encode 32 bits unsigned int (lsb) */
static inline char *utils_encode32u(char *p, IUINT32 l)
{
#if IWORDS_BIG_ENDIAN
	*(unsigned char*)(p + 0) = (unsigned char)((l >> 0) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >> 8) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
#else
	*(IUINT32*)p = l;
#endif
	return p;
}

/* decode 32 bits unsigned int (lsb) */
static inline const char *utils_decode32u(const char *p, IUINT32 *l)
{
#if IWORDS_BIG_ENDIAN
	*l = *(const unsigned char*)(p + 3);
	*l = *(const unsigned char*)(p + 2) + (*l << 8);
	*l = *(const unsigned char*)(p + 1) + (*l << 8);
	*l = *(const unsigned char*)(p + 0) + (*l << 8);
#else 
	*l = *(const IUINT32*)p;
#endif
	return p;
}
#endif