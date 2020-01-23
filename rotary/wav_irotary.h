
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
                    WavRotary(int pinA,int pinB );
        void        start();
        int         getCount();
        void        interrupt();
protected:
        Rotary       *_rotary;
        int          _count;
        int          _pinA,_pinB;
};
