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



typedef struct CapScale
{
    float capMax; // in pF
    adc_smp_rate rate;
    adc_prescaler scale;
};

const CapScale capScales[]=
{
    {500,ADC_SMPR_13_5,ADC_PRE_PCLK2_DIV_2}, // max 500pf
    {50*1000,ADC_SMPR_55_5,ADC_PRE_PCLK2_DIV_4},  // max 50nF
    {1000*1000,ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_4},  // max 1Uf
    {10,ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_8}  // and up
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

bool Capacitor::doOne(int dex,TestPin::PULL_STRENGTH strength, bool grounded, float percent,int &timeUs, int &resistance,int &value)
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
         if(!doOne(LAST_SCALE,strength,true,targetPc,timeLow,resistanceLow,valueLow))
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
 * @return 
 */
bool Capacitor::compute()
{
    capacitance=0;
    int timeLow,resistanceLow,valueLow;
    
    // do a quick  estimate of the cap at 10%
     if(!doOne(2,TestPin::PULL_MED,true,0.10,timeLow,resistanceLow,valueLow))
         return false;
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
    //if(Cest>200000.) // more than 200 nf, do slow
        return computeHiCap(Cest);

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
