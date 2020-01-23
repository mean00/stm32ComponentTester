#include <Wire.h>
#include "Rotary.h"
#include "wav_irotary.h"

static WavRotary *current=NULL;


 /**
  */
 
static void myInterrupt()
{
  if(!current) return;
  current->interrupt();
}

/**
 */
 WavRotary::WavRotary(int pinA,int pinB ) : _rotary(pinA,pinB)
 {
    current=this;
    _count=0;
    noInterrupts(); 
#define SETPIN(x) {    pinMode(x,INPUT_PULLUP);    attachInterrupt(x, myInterrupt, CHANGE); }
    
    SETPIN(pinA);
    SETPIN(pinB);

    interrupts();
 }
 int          WavRotary::getCount()
 {
     noInterrupts();
     int c=_count;
     _count=0;
     interrupts();
     return c;
 }
 
 /**
  */
 void WavRotary::interrupt()
 {
     switch(_rotary.process())
     {
        case DIR_CCW:  _count++;break;
        case DIR_CW:   _count--;break;
        default:       break;
     }
 }

 // EOF
