/*
 * This controls one pin : pull it up, down, etc.. and measure
 * 
 */
#include "vector"
#include "testPins.h"
#include "dso_adc.h"
#include "MapleFreeRTOS1000_pp.h"
extern DSOADC *adc;

void xFail(const char *message);
/**
 * 
 * @param p
 */
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
    void disconnectAll()
    {
        int n=_pins.size();
        for(int i=0;i<n;i++)
        {
            _pins[i]->disconnect();
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

TestPin::TestPin(int pinNo, int pin, int pinDriveHighRes, int pinDriveLow,int lowRes, int hiRes,int internal)
{
     _pinNumber=pinNo;
     _pin=pin;
     _pinDriveHighRes=pinDriveHighRes;
     _pinDriveLowRes=pinDriveLow;
     _lowRes=lowRes;
     _hiRes=hiRes;
     _internalPull=internal;
   
}
void TestPin::init()
{
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
void    TestPin::pullUp(PULL_STRENGTH strength)
{
    disconnectAll();
    switch(strength)
    {
    case PULL_LOW:
        configureOutput(_pinDriveLowRes,1);
        _state=PULLUP_LOW;        
        break;
    case PULL_INTERNAL:
        pinMode(_pin, INPUT_PULLUP);
        _state=INTERNAL_PULLUP;
        break;
    case PULL_HI:
        configureOutput(_pinDriveHighRes,1);
        _state=PULLUP_HI;        
        break;
    }
   
}
/**
 * 
 * @param hiRes
 */

void    TestPin::pullDown(PULL_STRENGTH strength)
{
    disconnectAll();
    switch(strength)
    {
    case PULL_LOW:
        configureOutput(_pinDriveLowRes,0);
        _state=PULLDOWN_LOW;
        break;
    case PULL_INTERNAL:
        pinMode(_pin, INPUT_PULLDOWN);
        _state=INTERNAL_PULLDOWN;
        break;
    case PULL_HI:
        configureOutput(_pinDriveHighRes,0);
        _state=PULLDOWN_HI;
        break;
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
 */
AutoDisconnect::~AutoDisconnect()
{
    allPins.disconnectAll();
}

/**
 * 
 * @param adc
 * @param voltage
 */
void    TestPin::sample(int &xadc, float &voltage)
{
    uint16_t *samples;
    int nbSamples;
    xadc=0;
    
    adc->setADCPin(_pin);
    adc->prepareDMASampling(ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_8);    
    adc->startDMASampling(16);
    xAssert(true==adc->getSamples(&samples,nbSamples));
    int r=0;
    for(int i=0;i<nbSamples;i++)
        r+=samples[i];
    r/=nbSamples;

    xadc=r;
    voltage=(float)xadc*3.3/4095.;
}
      

/**
 */
void xFail(const char *message)
{
    allPins.disconnectAll();
    __asm__  ("bkpt 1");
    while(1)
    {
        
    };
}
int TestPin::getCurrentRes()
{
    switch(_state)
    {
        case DISCONNECTED: 
        case VCC:
        case GND:
                    xFail("Invalid");
                    break;
        case PULLUP_HI:         return _hiRes+WIRE_RESISTANCE_AND_INTERNAL;break;
        case PULLUP_LOW:        return _lowRes+WIRE_RESISTANCE_AND_INTERNAL;break;
        case INTERNAL_PULLUP:   return _internalPull+WIRE_RESISTANCE_AND_INTERNAL;break;
        case PULLDOWN_HI:       return _hiRes+WIRE_RESISTANCE_AND_INTERNAL;break;
        case PULLDOWN_LOW:      return _lowRes+WIRE_RESISTANCE_AND_INTERNAL;break;
        case INTERNAL_PULLDOWN: return _internalPull+WIRE_RESISTANCE_AND_INTERNAL;break;        
    }
    xFail("Invalid");
    return 0;
}

// EOF