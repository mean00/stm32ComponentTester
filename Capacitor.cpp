/*
 * Capacitor tester
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
    char st[32];    
    Component::prettyPrint(capacitance, "F",st);
    ucg->drawString(10,30,0,st); 
    return true;
}

/**
 * 
 * @return 
 */

bool Capacitor::doOne(TestPin::PULL_STRENGTH strength, bool grounded, float percent,int &timeUs, int &resistance,int &value)
{
    if(!zero(6)) return false;    
    // go
    if(grounded)
        _pB.setToGround();
    else
        _pB.pullDown(strength);
    _pA.pullUp(strength);
    
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
bool Capacitor::compute()
{
    capacitance=0;
    int timeLow,resistanceLow,valueLow;
    
    // do a quick  estimate of the cap at 10%
     if(!doOne(TestPin::PULL_MED,true,0.10,timeLow,resistanceLow,valueLow))
         return false;
    // if time is big, it means we have to use a lower resistance = bigger current
    // if it is small, it means we have to use a bigger resitance = lower current
    // we target 200 ms
    timeLow=timeLow*10; // Estimated value of RC   to charge the cap at 75% = RC
    float Cest=(float)timeLow/(float)resistanceLow;
    
    // it is actually 1E6*Cest, i.e. in uF
    TestPin::PULL_STRENGTH strength=TestPin::PULL_MED;
    int overSampling=2;
    if(Cest<2)
    {
         strength=TestPin::PULL_HI;
         overSampling=10;
    }else
        if(Cest>40)
        {
            overSampling=1;
            strength=TestPin::PULL_LOW;
        }
    
    float targetPc=0.6281;
    // do the real ones
    float capSum=0;
    int totalTime=0;
    int totalR=0;
    int totalAdc=0;
    for(int i=0;i<overSampling;i++)
    {
         if(!doOne(strength,true,targetPc,timeLow,resistanceLow,valueLow))
             return false;
         totalTime+=timeLow;
         totalR+=resistanceLow;
         totalAdc+=valueLow;
        
    }
    totalAdc/=overSampling;
    capacitance=computeCapacitance(totalTime,totalR,totalAdc);   
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
    float den;
    
    den=1.-(float)(actualValue)/4095.;
    
    den=log(den);
    if(-den<0.000001) return 0;    
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

/**
 * \brief discharge the capacitor
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