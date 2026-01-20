#include <include/timer>

int main()
{
    OSLIB_Timer* timer = AllocateTimer();
    if (!timer)
    {
        return -1;
    }

    f32 accumTest = 0.0f;

    while (accumTest < 10.0f)
    {
        accumTest += OSLIB_TimerReset(timer);
    }

    OSLIB_DeallocateTimer(timer);
    return 0;
}
