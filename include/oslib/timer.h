#ifndef OSLIB_TIMER_H
#define OSLIB_TIMER_H

#include <include/oslib/platform.h>

struct OSLIB_Timer;

OSLIB_Timer *OSLIB_AllocateTimer();

void OSLIB_DeallocateTimer(OSLIB_Timer *timer);

void OSLIB_TimerStart(OSLIB_Timer *timer);

f32 OSLIB_TimerReset(OSLIB_Timer *timer);

#endif