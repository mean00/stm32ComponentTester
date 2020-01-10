/**
 
 
 */
#pragma once
#include "Arduino.h"

// mostly internal pin resistance when pulled to VCC or GND
#define WIRE_RESISTANCE_AND_INTERNAL 30

class AutoDisconnect
{
public:
        ~AutoDisconnect();
};

class TestPin
{
  friend class AllPins;
public:
        enum TESTPIN_STATE
        {
          IDLE=0,
          DISCONNECTED=1,
          PULLUP_HI=2,
          PULLUP_LOW=3,
          PULLDOWN_HI=4,
          PULLDOWN_LOW=5,
          PULLUP_INTERNAL=6,
          PULLDOWN_INTERNAL=7,
          VCC=10,
          GND=11                    
        };
  
        enum PULL_STRENGTH
        {
          PULL_LOW,
          PULL_INTERNAL,
          PULL_HI
        };
        
  
                TestPin(int pinNo, int pin, int pinDriveHighRes, int pinDriveLow, int lowRes, int hiRes,int internalPull);
        void    setMode(TESTPIN_STATE mode);
        void    init();
        void    pullUp(PULL_STRENGTH strength);
        void    pullDown(PULL_STRENGTH strength);
        void    setToVcc();
        void    setToGround();
        void    disconnect();
        void    sample(int &adc, float &voltage);
        TESTPIN_STATE getState() {return _state;}
        void    disconnectAll();
        int     getCurrentRes();

protected:  
        void configureOutput(int pinNo, int state);
        int _pinNumber,_pin,_pinDriveHighRes, _pinDriveLowRes;
        int _lowRes,_hiRes;
        int _internalPull;
        TESTPIN_STATE _state;
};


