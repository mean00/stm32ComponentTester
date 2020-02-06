/*
 * Capacitor tester
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "Capacitor.h"
#include "math.h"
#include "calibration.h"
#include "cycleClock.h"

/**
 * 
 * @return 
 */
extern float capz;
bool Capacitor::doOneQuick(TestPin::PULL_STRENGTH strength, bool doubled, float percent,int &timeUs, int &resistance,int &value)
{
    if(!zero(10)) return false;    
    // go
    if(!doubled)
        _pB.setToGround();
    else
        _pB.pullDown(strength);
    _pA.pullUp(strength);
    
    // Wait for the ADC value to go over 4095*percent
    // We introduce a small error here due to the fact the ADC
    // is starting too late
    // compensated by calibration
    int timeOne,valueOne;
    if(!_pA.fastSampleUp(10,4095.*percent,valueOne,value,timeOne,timeUs)) 
    {
        //zero(6);
        return false;
    }
    
    //zero(6);
    // compensate for B resistance
    float v;
    v=((4095.-(float)value)*(float)_pB.getCurrentRes())/(float)_pA.getCurrentRes()    ;
    value-=v;
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();    
    _pA.setToGround();
    
    
    
    float den=(4095.-(float)valueOne)/(4095.-(float)value);
    
    if(fabs(den-2.718)<0.01) 
        return false;
    den=log(den);
    capz=(timeUs-timeOne)/(resistance*den);
    capz=capz/1000000.; // us -> sec
    
    return true;
}

// EOF
