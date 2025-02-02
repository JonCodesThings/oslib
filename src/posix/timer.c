#include <include/oslib/platform.h>

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

typedef struct OSLIB_Timer
{
	struct timeval last;
} OSLIB_Timer;

OSLIB_Timer *OSLIB_AllocateTimer()
{
	OSLIB_Timer *timer = Allocate(sizeof(OSLIB_Timer));

	return timer;
}

void OSLIB_DeallocateTimer(OSLIB_Timer *timer)
{
	if (timer != NULL)
	{
		Deallocate(timer);
		timer = NULL;
	}
}

void OSLIB_TimerStart(OSLIB_Timer *timer)
{
	gettimeofday(&timer->last, NULL);
}

f32 OSLIB_TimerReset(OSLIB_Timer *timer)
{
	if (timer == NULL)
	{
		return 0.0f;
	}

	struct timeval now;
	if (gettimeofday (&now, NULL) == 0)
	{
		uint64_t diff = (now.tv_sec * 1000000 + now.tv_usec) - (timer->last.tv_sec * 1000000 + timer->last.tv_usec);
		return diff * (1 / 1000000);
	}
	return 0.0f;
}