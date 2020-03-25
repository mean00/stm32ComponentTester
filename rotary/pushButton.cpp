#include "Wire.h"
#include "pushButton.h"

/**
 */
#define THRESHOLD           3        // 3ms
#define TIME_LONG_PRESS     2000     // 2sec
#define TIME_SHORT_PRESS    8
void pushInterrupt(void *a)
{
    PushButton *b=(PushButton *)a;
    b->interrupt();
}

void PushButton::interrupt()
{
    bool state=digitalRead(_pin);
    uint32_t m=millis();
    if((m-_lastRead)<THRESHOLD) return;
    if(!state) // down
    {
        _down=m;
        return;
    }else
    {
        uint32_t time=m-_down;
        if(time>TIME_LONG_PRESS)
                _event|=LONG_PRESS;
        if(time>TIME_SHORT_PRESS)
                _event|=SHORT_PRESS;
    }
}

PushButton::PushButton(int pin)
{
    _pin=pin;
    _event=NONE;
    _down=0;
    _lastRead=0;
    pinMode(_pin,INPUT_PULLUP);
    attachInterrupt(_pin,pushInterrupt,(void *)this,CHANGE );

}
/**
 */
PushButton::EVENTS      PushButton::getEvent()
{
    int evt = __atomic_exchange_n( &(_event), 0, __ATOMIC_SEQ_CST);
    return (PushButton::EVENTS)evt;
}
// Eof
