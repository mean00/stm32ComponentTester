#include <Wire.h>
#include "Rotary.h"
#include "wav_irotary.h"

static WavRotary *current=NULL;
#define THRESHOLD           3        // 3ms
#define TIME_LONG_PRESS     1000     // 2sec
#define TIME_SHORT_PRESS    8


 /**
  */
 
static void myInterrupt()
{
  if(!current) return;
  current->rotaryInterrupt();
}
static void myPushInterrupt()
{
  if(!current) return;
  current->pushInterrupt(); 
}
/**
 * 
 */
void WavRotary::pushInterrupt()
{
    bool state=digitalRead(_pinPush);
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
        {
                _events.setEventsFromISR(LONG_PRESS);
        }
        if(time>TIME_SHORT_PRESS)
        {
                _events.setEventsFromISR(SHORT_PRESS);
        }
    }
}

 /**
  */
 void WavRotary::rotaryInterrupt()
 {
     switch(_rotary->process())
     {
        case DIR_CCW:  _count++;_events.setEventsFromISR(ROTARY_CHANGE);break;
        case DIR_CW:   _count--;_events.setEventsFromISR(ROTARY_CHANGE);break;
        default:       break;
     }
 }
/**
 */
 WavRotary::WavRotary(int pinA,int pinB, int pinPush ) 
 {
    current=this;
    _count=0;
    _rotary=new Rotary(pinA,pinB);
    _event=NONE;
    _down=0;
    _lastRead=0;
    
    _pinA=pinA;
    _pinB=pinB;
    _pinPush=pinPush;
    pinMode(_pinA,INPUT_PULLUP); 
    pinMode(_pinB,INPUT_PULLUP); 
    pinMode(_pinPush,INPUT_PULLUP);
    
    
 }
 
 /**
  * 
  */
 void        WavRotary::start()
 {
    noInterrupts(); 
    attachInterrupt(_pinA, myInterrupt, CHANGE);
    attachInterrupt(_pinB, myInterrupt, CHANGE);
    attachInterrupt(_pinPush,myPushInterrupt,CHANGE );
    interrupts();
    _rotary->begin(true);
     
 }
 /*
  */
 int          WavRotary::getCount()
 {
     noInterrupts();
     int c=_count;
     _count=0;
     interrupts();
     return c;
 }
/**
 * 
 * @return 
 */
WavRotary::EVENTS      WavRotary::readEvent()
{
    return (EVENTS)_events.readEvents(  SHORT_PRESS |     LONG_PRESS |    ROTARY_CHANGE);
}
/**
 * 
 * @return 
 */
WavRotary::EVENTS      WavRotary::waitForEvent()
{
    return (EVENTS)_events.waitEvents(  SHORT_PRESS |     LONG_PRESS |    ROTARY_CHANGE);
}
 // EOF
