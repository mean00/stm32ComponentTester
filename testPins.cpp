/*
 * This controls one pin : pull it up, down, etc.. and measure
 * 
 */
#include "testPins.h"
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
}

void TestPin::configureOutput(int pinNo, int state)
{
    digitalWrite(pinNo,state);
    pinMode(pinNo,OUTPUT);
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