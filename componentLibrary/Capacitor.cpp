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
#include "MapleFreeRTOS1000_pp.h"
//
CycleClock clk;



typedef struct CapScale
{
    float capMax; // in pF
    adc_smp_rate rate;
    adc_prescaler scale;
    float tickUs;
};

const CapScale capScales[]=
{
    {500,ADC_SMPR_13_5,ADC_PRE_PCLK2_DIV_2,0.72}, // max 500pf
    {50*1000,ADC_SMPR_55_5,ADC_PRE_PCLK2_DIV_4,3.78},  // max 50nF
    {1000*1000,ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_4,14},  // max 1Uf
    {10,ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_8,28}  // and up
};

#define LAST_SCALE ((sizeof(capScales)/sizeof(CapScale))-1)

/**
 * 
 * @param yOffset
 * @return 
 */
bool Capacitor::draw(int yOffset)
{
    char st[32];        
    Component::prettyPrint(capacitance, "F",st);
    TesterGfx::drawCapacitor(yOffset, st,_pA.pinNumber(), _pB.pinNumber());
    return true;
}

/**
 * 
 * @return 
 */

bool Capacitor::doOne(int dex,TestPin::PULL_STRENGTH strength, bool grounded, float &cap)
{
    int resistance;
    if(!zero(10)) return false;    
    // go
    if(grounded)
        _pB.setToGround();
    else
        _pB.pullDown(strength);
    
        
    uint32_t begin,end;
    // Discharge cap
    begin=micros();    
    // start the DMA
    // max duration ~ 512 us
    uint16_t *samples;
    int nbSamples;
    clk.start();        
    
    if(!_pA.prepareDmaSample(capScales[dex].rate,capScales[dex].scale,512)) 
        return false;        
    _pA.pullUp(TestPin::PULL_HI);   
    
    
    if(!_pA.finishDmaSample(nbSamples,&samples)) 
    {
        return false;
    }
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    _pA.pullDown(TestPin::PULL_LOW);   
    
    
    int limitA,limitB;
    if(grounded)
    {
        limitA=50;
        limitB=2784;
    }else
    {
        xAssert(0);
    }
    
    // We need 2 points...
    // Lookup up 5% and 1-1/e
    int pointA=-1,pointB=-1;
    for(int i=1;i<nbSamples;i++)
    {
        if(samples[i]>limitA) // 5%
        {
            pointA=i;
            i=4095;
        }
    }
    if(pointA==-1) return false;
    for(int i=pointA+1;i<nbSamples;i++)
    {
        if(samples[i]>limitB) // 68%
        {
            pointB=i;
            i=4095;
        }
    }
    if(pointB==-1) pointB=nbSamples-1;
    
    // Compute
    float timeElapsed=(pointB-pointA);
    timeElapsed*=capScales[dex].tickUs;
    timeElapsed/=1000000.; // In seconds

    float valueA=samples[pointA];
    float valueB=samples[pointB];

    float den=(4095-valueA)/(4095-valueB);
    den=log(den);
    cap=timeElapsed/(resistance*den);
    return true;
}



/**
 * 
 * @return 
 */
extern int z;
bool Capacitor::computeLowCap(int dex)
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
    
    if(!_pA.prepareDmaSample(capScales[dex].rate,capScales[dex].scale,512)) 
        return false;        
    _pA.pullUp(TestPin::PULL_HI);   
    
    
    if(!_pA.finishDmaSample(nbSamples,&samples)) 
    {
        return false;
    }
    _pA.pullDown(TestPin::PULL_LOW);   
#if 0
    char st[32];    
    Component::prettyPrint((float)samples[nbSamples-1], "F",st);
    ucg->drawString(10,30,0,st); 
    while(1)
    {
        
    };

#endif    
    return false;
}
/**
 */
bool Capacitor::computeHiCap(float Cest)
{
    int overSampling=2;
    TestPin::PULL_STRENGTH    strength=TestPin::PULL_LOW;
    float targetPc=0.6281;
    // do the real ones
    float capSum=0;
    int totalTime=0;
    int totalR=0;
    int totalAdc=0;
    int timeLow;
    int resistanceLow;
    int valueLow;
    for(int i=0;i<overSampling;i++)
    {
         if(!doOne(LAST_SCALE,strength,true,capacitance))
             return false;
         totalTime+=timeLow;
         totalR+=resistanceLow;
         totalAdc+=valueLow;
        
    }
 
    return true;
}
/**
 * 
 * @return 
 */
bool Capacitor::compute()
{
    capacitance=0;
    float cap;
     if(!doOne(1,TestPin::PULL_MED,true,cap))
         return false;
    
    float offset=INTERNAL_CAPACITANCE_IN_PF;
    offset/=1000000000000.; // In pf
    capacitance=cap-offset;
    if(capacitance<0.) capacitance=0.;    
    return true;
#if 0    
    // if time is big, it means we have to use a lower resistance = bigger current
    // if it is small, it means we have to use a bigger resistance = lower current
    // we target 200 ms
    timeLow=timeLow*10; // Estimated value of RC   to charge the cap at 68% = RC
    float Cest=(float)timeLow/(float)resistanceLow;
    
    // it is actually 1E12*Cest, i.e. in pF
    Cest=Cest*1000000.;    
    int n=sizeof(capScales)/sizeof(CapScale);
    int scale=0;
    for(int i=0;i<n;i++)
    {
        if(Cest<capScales[i].capMax)
        {
            scale=i;
            i=n;
        }
    }
    return computeHiCap(Cest);
#endif    

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
