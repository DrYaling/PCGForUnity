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


#endif