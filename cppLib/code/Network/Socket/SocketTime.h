#ifndef SOCKET_TIME_H
#define SOCKET_TIME_H
#include "define.h"
//#include "Logger/Logger.h"
class SocketTime_t;
static SocketTime_t* pInstance;
static int index = 0;
class SocketTime_t
{
public:
	SocketTime_t() :idx(++index) 
	{ 
		//LogFormat("socket time %d",idx); 
	}
	int idx;
	int32 timeDiff;
	uint64 timeStampSinceStartUp;
	void Init(uint32 time) { timeStampSinceStartUp = time; }
	void Update(int32 time_diff) {
		//timeDiff = time_diff;
		//timeStampSinceStartUp += time_diff;
	};
	inline static SocketTime_t* GetInstance() {
		if (nullptr == pInstance)
		{
			pInstance = new SocketTime_t();
		}
		return pInstance;
	}
	static void Destroy() {
		if (pInstance)
		{
			delete pInstance;
			pInstance = nullptr;
		}
	}
};
static SocketTime_t SocketTime = *SocketTime_t::GetInstance();
#endif
