#ifndef _PLANTS_H
#define _PlANTS_H
#include "../generator.h"
NS_GNRT_START
class plantsGen
{
public:
	virtual bool Init(int32_t seed) { return true; }
protected:
	virtual void Start() = 0;
	virtual void L_System_Gen() = 0;
};
NS_GNRT_END
#endif // _PLANTS_H
