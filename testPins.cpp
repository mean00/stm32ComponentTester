/*
 * This controls one pin : pull it up, down, etc.. and measure
 * 
 */
#include "vector"
#include "testPins.h"

/**
 */
void xFail(const char *message)
{
    __asm__  ("bkpt 1");
    while(1)
    {
        
    };
}

class AllPins
{
public:
    void registerMe(TestPin *p) {_pins.push_back(p);}
    
    // We dont allow one pin set to VCC and the other one set to ground
    // check for that
    void checkVcc(TestPin &me)
    {
        int n=_pins.size();
        for(int i=0;i<n;i++)
        {
            if(me._pinNumber==_pins[i]->_pinNumber) continue;
            if(_pins[i]->getState()==TestPin::GND)
                xFail("VCC NOT ALLOWED");
        }
    }        
    void checkGnd(TestPin &me)
    {
        int n=_pins.size();
        for(int i=0;i<n;i++)
        {
            if(me._pinNumber==_pins[i]->_pinNumber) continue;
            if(_pins[i]->getState()==TestPin::VCC)
                xFail("GND NOT ALLOWED");
        }
    }            
public:    
    std::vector<TestPin *>_pins;
};
AllPins allPins;

/**
 */
#pragma once
#include "Arduino.h"

TestPin::TestPin(int pinNo, int pin, int pinDriveHighRes, int pinDriveLow)
{
     _pinNumber=pinNo;
     _pin=pin;
     _pinDriveHighRes=pinDriveHighRes;
     _pinDriveLowRes=pinDriveLow;
     disconnect();
     allPins.registerMe(this);
}

void TestPin::configureOutput(int pinNo, int state)
{
    digitalWrite(pinNo,state);
    pinMode(pinNo,OUTPUT);
    digitalWrite(pinNo,state);
}
/**
 * 
 * @param hiRes
 */
void    TestPin::pullUp(bool hiRes)
{
    disconnectAll();
    if(hiRes)
    {
        configureOutput(_pinDriveHighRes,1);
        _state=PULLUP_HI;
    }
    else
    {
        configureOutput(_pinDriveLowRes,1);
        _state=PULLUP_LOW;
    }
}
/**
 * 
 * @param hiRes
 */
void    TestPin::pullDown(bool hiRes)
{
    disconnectAll();
    if(hiRes)
    {
        configureOutput(_pinDriveHighRes,0);
        _state=PULLDOWN_HI;
    }
    else
    {
        configureOutput(_pinDriveLowRes,0);
        _state=PULLDOWN_LOW;
    }
}
/**
 * 
 */
void    TestPin::setToVcc()
{
    allPins.checkVcc(*this);
    disconnectAll();
    digitalWrite(_pin,1);
    pinMode(_pin,OUTPUT);
    _state=VCC;    
}
/**
 * 
 */
void    TestPin::setToGround()
{
    allPins.checkGnd(*this);
    disconnectAll();
    digitalWrite(_pin,0);
    pinMode(_pin,OUTPUT);
    _state=GND;    
}
/**
 * 
 */
void    TestPin::disconnectAll()
{
    pinMode(_pin,INPUT_FLOATING);
    pinMode(_pinDriveHighRes,INPUT_FLOATING);
    pinMode(_pinDriveLowRes,INPUT_FLOATING);
}       
/**
 * 
 */
void    TestPin::disconnect()
{
        disconnectAll();
        _state=DISCONNECTED;
}        

/**
 * 
 * @param adc
 * @param voltage
 */
void    TestPin::sample(int &adc, float &voltage)
{
#warning TODO
}
// EOF