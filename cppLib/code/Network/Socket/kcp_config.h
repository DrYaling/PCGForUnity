#ifndef KCP_CONFIG_H
#define KCP_CONFIG_H
#include "ikcp.h"
#include "SocketConfig.h"
#include <functional>
#define IKCP_OVERHEAD_LEN  24
typedef struct kcp_sessionId
{
	IUINT32 conv;
	SockAddr_t addr;
	explicit kcp_sessionId() {}
	explicit  kcp_sessionId(IUINT32 cv, const SockAddr_t& ar) :conv(cv), addr(ar) {}
	bool operator<(const kcp_sessionId& r) const {
		if (conv != r.conv)
		{
			return conv < r.conv;
		}
		return addr < r.addr;
	}
	bool operator == (const kcp_sessionId& r) const {
		return conv == r.conv && addr == r.addr;
	}
} socketSessionId;

enum SessionStatus
{
	Disconnected,
	Connecting,
	Connected,
};
typedef std::function<bool(int, uint8*, int)> SocketDataReceiveHandler;
#endif
