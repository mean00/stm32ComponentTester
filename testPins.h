/**
 
 
 */
#pragma once
#include "Arduino.h"

class TestPin
{
public:
        enum TESTPIN_STATE
        {
          IDLE=0,
          DISCONNECTED=1,
          PULLUP_HI=2,
          PULLUP_LOW=3,
          PULLDOWN_HI=4,
          PULLDOWN_LOW=5,
          VCC=6,
          GND=7,                    
        };
  
  
                TestPin(int pinNo, int pin, int pinDriveHighRes, int pinDriveLow);
        void    pullUp(bool hiRes);
        void    pullDown(bool hiRes);
        void    setToVcc();
        void    setToGround();
        void    disconnect();
        void    sample(int &adc, float &voltage);
        TESTPIN_STATE getState() {return _state;}
        void    disconnectAll();

protected:  
        void configureOutput(int pinNo, int state);
        int _pinNumber,_pin,_pinDriveHighRes, _pinDriveLowRes;
        TESTPIN_STATE _state;
};


