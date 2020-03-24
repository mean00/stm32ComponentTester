#include "Wire.h"
#include "pushButton.h"

/**
 */

void pushInterrupt(void *a)
{
    PushButton *b=(PushButton *)a;
    b->interrupt();
}

void PushButton::interrupt()
{
    
}

PushButton::PushButton(int pin)
{
    _pin=pin;
    _event=NONE;
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
