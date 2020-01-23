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
 WavRotary::WavRotary(int pinA,int pinB ) 
 {
    current=this;
    _count=0;
    _rotary=new Rotary(pinA,pinB);
    _pinA=pinA;
    _pinB=pinB;
    pinMode(pinA,INPUT_PULLUP); 
    pinMode(pinB,INPUT_PULLUP); 
 }
 /**
  * 
  */
 void        WavRotary::start()
 {
    noInterrupts(); 
    attachInterrupt(_pinA, myInterrupt, CHANGE);
    attachInterrupt(_pinB, myInterrupt, CHANGE);
    interrupts();
     
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
  */
 void WavRotary::interrupt()
 {
     switch(_rotary->process())
     {
        case DIR_CCW:  _count++;break;
        case DIR_CW:   _count--;break;
        default:       break;
     }
 }

 // EOF
