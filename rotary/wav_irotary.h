
#pragma once
/**
 */
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
        int         getCount();
        void        interrupt();
protected:
        Rotary       _rotary;
        int          _count;
};
