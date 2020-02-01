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
    
    if(!_pA.fastSampleUp(4095*percent,value,timeUs)) 
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
    return true;
}

/**
 * 
 * @return 
 */
bool Capacitor::quickEval(float &cap)
{
    
    int timeLow,resistanceLow,valueLow;
    
    // do a quick  estimate of the cap at 10%
     if(!doOneQuick(TestPin::PULL_MED,false,0.10,timeLow,resistanceLow,valueLow))
         return false;
    
    //--
    char st[20];
    sprintf(st,"T=%d\n",timeLow);
    TesterGfx::print(10,10,st);

    while(1)
    {
        
    }
    
    if(timeLow<20)
    {
     if(!doOneQuick(TestPin::PULL_HI,false,0.10,timeLow,resistanceLow,valueLow))
         return false;        
    }else
    {
        if(timeLow>2000)
        {
             if(!doOneQuick(TestPin::PULL_LOW,false,0.10,timeLow,resistanceLow,valueLow))
                return false;        
        }
    }
    // we target 200 ms
    timeLow=timeLow*10; // Estimated value of RC   to charge the cap at 68% = RC
    float Cest=(float)timeLow/(float)resistanceLow;
    cap=Cest;
    return true;
}
// EOF
