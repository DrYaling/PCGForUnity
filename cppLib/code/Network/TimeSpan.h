#ifndef _TIME_SPAN_H
#define  _TIME_SPAN_H
#include "Socket/SocketConfig.h"

#if defined(_WIN32_PLATFROM_)
#include <Windows.h>
#define timelong_t ULARGE_INTEGER
#else
#include <sys/time.h>
#include <linux/errno.h>
#define timelong_t struct timeval
#endif



class TimeSpan
{
public:
	TimeSpan();
	void Reset();
	unsigned long long GetSpaninMicroseconds();
	unsigned int GetSpaninMilliseconds();
	unsigned int GetSpaninSeconds();

private:
	timelong_t m_start;
	void getCurrentTimeLong(timelong_t *tl);

};
//=====================================================================================

TimeSpan::TimeSpan()
{
	getCurrentTimeLong(&m_start);
}
void TimeSpan::Reset()
{
	getCurrentTimeLong(&m_start);
}
unsigned int TimeSpan::GetSpaninMilliseconds()
{
	return (unsigned int)(GetSpaninMicroseconds() / 1000LL);
}
unsigned int TimeSpan::GetSpaninSeconds()
{
	return (unsigned int)(GetSpaninMicroseconds() / 1000000LL);
}
unsigned long long TimeSpan::GetSpaninMicroseconds()
{
	timelong_t end;
	getCurrentTimeLong(&end);
#if defined(_WIN32_PLATFROM_)
	return (end.QuadPart - m_start.QuadPart) / 10;
#else
	return 1000000LL * (end.tv_sec - m_start.tv_sec) + end.tv_usec - m_start.tv_usec;
#endif
}
void TimeSpan::getCurrentTimeLong(timelong_t *tl)
{
#if defined(_WIN32_PLATFROM_)
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	tl->HighPart = ft.dwHighDateTime;
	tl->LowPart = ft.dwLowDateTime;
#else
	gettimeofday(tl, NULL);
#endif
}

#endif