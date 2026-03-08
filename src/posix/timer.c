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
	OSLIB_Timer *timer = OSLIB_Allocate(sizeof(OSLIB_Timer));
	return timer;
}

void OSLIB_DeallocateTimer(OSLIB_Timer *timer)
{
	if (timer == NULL)
	{
		return;
	}

	OSLIB_Deallocate(timer);
	timer = NULL;
}

void OSLIB_TimerStart(OSLIB_Timer *timer)
{
	if (timer == NULL)
	{
		return;
	}

	gettimeofday(&timer->last, NULL);
}


static const uint64_t s_Multiplier = 1000000;
static const f32 s_Converter = 1.0f / s_Multiplier;;

f32 OSLIB_TimerReset(OSLIB_Timer *timer)
{
	if (timer == NULL)
	{
		return 0.0f;
	}

	struct timeval now;
	if (gettimeofday (&now, NULL) != 0)
	{
		return 0.0f;
	}

	uint64_t diff = (now.tv_sec * s_Multiplier + now.tv_usec) - (timer->last.tv_sec * s_Multiplier + timer->last.tv_usec);
	f32 fdiff = diff * s_Converter;
	timer->last = now;
	return fdiff;
}