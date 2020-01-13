/*
 * Resistance tester
*/

#include <SPI.h>
#include "Ucglib.h"
#include "fancyLock.h"
#include "testPins.h"
#include "Capacitor.h"
#include "math.h"
#include "calibration.h"
/**
 * 
 * @param yOffset
 * @return 
 */
bool Capacitor::draw(Ucglib *ucg,int yOffset)
{
#if 1
    char st[32];    
    Component::prettyPrint(capacitance, "F",st);
    ucg->drawString(10,30,0,st); 
#endif 
    return true;
}

/**
 * 
 * @return 
 */

bool Capacitor::doOne(TestPin::PULL_STRENGTH strength, float percent,int &timeUs, int &resistance,int &value)
{
    if(!zero(10)) return false;    
    // go
    _pB.setToGround();
    _pA.pullUp(strength);
    
    if(!_pA.fastSampleUp(4095*percent,value,timeUs)) 
        return false;
    // compensate for B resistance
    float v;
    v=((4095.-(float)value)*(float)_pB.getCurrentRes())/(float)_pA.getCurrentRes()    ;
    value-=v;
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    return true;
}
/**
 * 
 * @param time
 * @param resistance
 * @param actualValue
 * @return 
 */
#define pPICO (1000.*1000.*1000.*1000.)
float Capacitor::computeCapacitance(int time, int iresistance, int actualValue)
{
    float cap;
    float t=(float)time/1000.;        
    float resistance=iresistance;
    float den=1.-(float)(actualValue)/4095.;
    
    den=log(den);
    cap=-t/(resistance*den);
    cap/=1000.;
#if 1
    float offset=INTERNAL_CAPACITANCE_IN_PF;
    offset=offset/pPICO;
    cap=cap-offset;
    if(cap<=0) cap=1/pPICO;
#endif    
    return cap;
}
float cLow,cMed,cHi;
/**
 * 
 * @return 
 */
bool Capacitor::compute()
{
    capacitance=0;
    int timeLow,resistanceLow;
    int timeMed,resistanceMed;
    int timeHi,resistanceHi;
    int valueLow,valueMed,valueHigh;
    
    
    // do an estimate of the cap
     if(!doOne(TestPin::PULL_MED,0.10,timeLow,resistanceLow,valueLow))
         return false;
    // if time is big, it means we have to use a lower resistance = bigger current
    // if it is small, it means we have to use a bigger resitance = lower current
    // we target 200 ms
#define AIM 200*1000
    timeLow=timeLow*14; // 10% => 75%, estimate convert time
    float targetPc=0.75;
    TestPin::PULL_STRENGTH strength=TestPin::PULL_MED;
    if(timeLow<(AIM/2))
    {
        strength=TestPin::PULL_HI;
    }
    if(timeLow>(AIM*2))
    {       
        strength=TestPin::PULL_LOW;
        float targetPc=0.60;
    }
    
    // do the real one
     if(!doOne(strength,targetPc,timeLow,resistanceLow,valueLow))
         return false;
    capacitance=computeCapacitance(timeLow,resistanceLow,valueLow);   
    return true;
}
/**
 * 
 * @return 
 */
bool Capacitor::zero(int threshold)
{
    _pA.pullDown(TestPin::PULL_LOW);
    _pB.pullDown(TestPin::PULL_LOW);
    
    int v,tus;
    _pA.fastSampleDown(threshold,v,tus);
    _pB.fastSampleDown(threshold,v,tus);
    
    xDelay(10);
    return true;
}

// EOF