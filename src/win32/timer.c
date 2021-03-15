#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <include/oslib/platform.h>

static LARGE_INTEGER OSLIB_TimerFrequency;

typedef struct OSLIB_Timer
{
	LARGE_INTEGER lastTimeStamp;
} OSLIB_Timer;

OSLIB_Timer *OSLIB_AllocateTimer()
{
	OSLIB_Timer *timer = Allocate(sizeof(OSLIB_Timer));

	return timer;
}

void OSLIB_DeallocateTimer(OSLIB_Timer *timer)
{
	Deallocate(timer);
}

void OSLIB_TimerStart(OSLIB_Timer *timer)
{
	QueryPerformanceFrequency(&OSLIB_TimerFrequency);
	QueryPerformanceCounter(&timer->lastTimeStamp);
}

f32 OSLIB_TimerReset(OSLIB_Timer *timer)
{
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);

	u32 diff = (u32) (current.QuadPart - timer->lastTimeStamp.QuadPart);

	float diffSeconds = (float) (diff / (OSLIB_TimerFrequency.QuadPart / 1000));

	timer->lastTimeStamp = current;

	return diffSeconds;
}