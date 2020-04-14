
#include "MapleFreeRTOS1000_pp.h"

#pragma once
/**
 */
class Rotary;
enum WavDirection
{
  WavNone=0,
  WavLeft,
  WavRight
};
/**
 */
class WavRotary
{
public:
  enum EVENTS
  {
    NONE=0,
    SHORT_PRESS=1,
    LONG_PRESS=2,
    ROTARY_CHANGE=4
  };
  
  
                    WavRotary(int pinA,int pinB, int pinPush );
        void        start();
        int         getCount();
        EVENTS      readEvent();
        EVENTS      waitForEvent();
        
public:        
        void        rotaryInterrupt();
        void        pushInterrupt();
protected:
        Rotary       *_rotary;
        int          _count;
        int          _pinA,_pinB,_pinPush;
        int           _event;
        uint32_t      _lastRead;
        uint32_t      _down; // time down was detected
        xEventGroup   _events;
};
