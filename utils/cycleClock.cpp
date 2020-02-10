#include "cycleClock.h"

CycleClock::CycleClock()
{
 _start=_end=0;       
}
CycleClock::~CycleClock()
{
    
}
/**
 */
void CycleClock::start()
{
    _start=systick_get_count();
}
/**
 */
void CycleClock::stop()
{
    _end=systick_get_count();
}
/**
 */
int CycleClock::elapsedUs()
{
    int delta;
    if(_start<_end)
        delta=_end-_start;
    else
        delta=(SYSTICK_RELOAD_VAL+2-_start)+_end;
    
    return (delta+CYCLES_PER_MICROSECOND/2)/CYCLES_PER_MICROSECOND;
    
}
