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
//
CycleClock clk;
/**
 * 
 * @param yOffset
 * @return 
 */
bool Capacitor::draw(int yOffset)
{
    char st[32];    
    Component::prettyPrint(capacitance, "F",st);
    TesterGfx::print(10,10,st);
    return true;
}

/**
 * 
 * @return 
 */

bool Capacitor::doOne(TestPin::PULL_STRENGTH strength, bool grounded, float percent,int &timeUs, int &resistance,int &value)
{
    if(!zero(10)) return false;    
    // go
    if(grounded)
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
extern int z;
bool Capacitor::computeLowCap()
{
    capacitance=0;
    uint32_t begin,end;
    // Discharge cap
    if(!zero(6)) return false;        
    // go
    _pB.setToGround();
    begin=micros();
    
    // start the DMA
    // max duration ~ 512 us
    uint16_t *samples;
    int nbSamples;
    clk.start();        
    
    if(!_pA.prepareDmaSample(true,512)) 
        return false;        
    _pA.pullUp(TestPin::PULL_HI);   
    
    
    if(!_pA.finishDmaSample(nbSamples,&samples)) 
    {
        return false;
    }
#if 0
    char st[32];    
    Component::prettyPrint((float)samples[nbSamples-1], "F",st);
    ucg->drawString(10,30,0,st); 
    while(1)
    {
        
    };

#endif    
    
    clk.stop();
    end=micros();
    //clk.stop();
    z=end-begin;
    // Each sample is ~ 1us
    // check which one reaches 0.615
    int candidate=-1;
    for(int i=0;i<nbSamples;i++)
    {
        if(samples[i]>(4095.*0.6815))
        {
            candidate=i;
            i=nbSamples+1;
        }
    }
    if(candidate==-1)        
        return false;
    capacitance=computeCapacitance(candidate,_pA.getCurrentRes()+_pB.getCurrentRes(),samples[candidate]);   
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
    // if it is small, it means we have to use a bigger resistance = lower current
    // we target 200 ms
    timeLow=timeLow*10; // Estimated value of RC   to charge the cap at 68% = RC
    float Cest=(float)timeLow/(float)resistanceLow;
    
    // it is actually 1E6*Cest, i.e. in uF
    TestPin::PULL_STRENGTH strength=TestPin::PULL_MED;
    int overSampling=2;
    if(Cest<2)
    {
#warning FIXME
        if(Cest<0.05) // less than  50 pf
        {
          // return computeLowCap();
        }
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
