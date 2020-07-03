#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "Capacitor.h"
#include "math.h"
#include "cycleClock.h"
#include "MapleFreeRTOS1000_pp.h"
#include "waveForm.h"
#include "testerControl.h"
#include "myPwm.h"
#include "math.h"


typedef struct CapScale
{
    float           capMin; // in PF
    float           capMax; // in pF
    adc_smp_rate    rate;
    DSOADC::Prescaler   scale;
    float           tick_for_into_at_72M;
    TestPin::PULL_STRENGTH strength;
    bool            doubled;

};

const CapScale capScales[]=
{
    {  20     ,800,         ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_6  ,  2.17,   TestPin::PULL_HI,     true},   // 20pf   -> 800pf
    { 800     ,1.8*1000,    ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_6  ,  2.17,   TestPin::PULL_HI,     false},  // 800 pf -> 1.8 nf
    { 1.8*1000,5*1000,      ADC_SMPR_28_5,  DSOADC::ADC_PRESCALER_6  ,  3.42,   TestPin::PULL_HI,     false},  // 1.8nf  -> 5nf
    {   5*1000,20*1000,     ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_6  ,  2.17,   TestPin::PULL_MED,    true},   // 5nf    -> 20 nf
    {  20*1000,100*1000,    ADC_SMPR_41_5,  DSOADC::ADC_PRESCALER_6  ,  4.5,    TestPin::PULL_MED,    false},  // 20 nf  -> 100 nf
    { 100*1000,200*1000,    ADC_SMPR_71_5,  DSOADC::ADC_PRESCALER_8  ,  9.33,   TestPin::PULL_MED,    false},  // 100 nf -> 200 nF
    { 600*1000,1200*1000,   ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_8  ,  2.89,   TestPin::PULL_LOW,    true},   // 600nf  -> 1.2uf
    {1200*1000,5000*1000,   ADC_SMPR_41_5,  DSOADC::ADC_PRESCALER_8  ,  6,      TestPin::PULL_LOW,    false},  // 1.2uF  -> 5uf
    {5000*1000,20*1000*10000,ADC_SMPR_239_5,DSOADC::ADC_PRESCALER_6  ,  21,     TestPin::PULL_LOW,    false},  // 5uF    -> 20uf
};

#define LAST_SCALE ((sizeof(capScales)/sizeof(CapScale))-1)

/**
 * Perform a DMA sampling buffer and extract 2 points
 * @return 
 */

bool Capacitor::doOne(float target,int dex, float &cap)
{
    int resistance;
    zeroAllPins();
    // go
    bool doubled=(capScales[dex].doubled);
    TestPin::PULL_STRENGTH strength=capScales[dex].strength;
    if(doubled)
        _pB.pullDown(strength);
    else    
        _pB.setToGround();

    // start the DMA
    // max duration ~ 512 us
    uint16_t *samples;
    int nbSamples;
    DeltaADCTime delta(_pA,_pB);
    float period;
    
    if(!delta.setup(500*1000,1024)) return false;
    
    _pA.pullUp(strength);   
    
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    bool r=delta.get(nbSamples,&samples,period);
    _pA.pullDown(TestPin::PULL_LOW);   
    if(!r) return false;    
    
#if 0    
    TesterGfx::drawCurve(nbSamples,samples);
    TesterControl::waitForAnyEvent();
#endif    
    
    
    float c=Capacitor::computeCapacitance(nbSamples, samples,   resistance,   period);
    if(c<=0) return false;
    
    cap=c;
    return true;    
}


/**
 * For medium value cap, we take the sampling time needed so that
 * we have a full charge within 512 samples
 * Then we can take 2 points in the samples and compute C
 * The aim is to have a wide enough span so that the computation is somehow accurate
 */
bool Capacitor::computeMediumCap(int dex,int overSampling,float &Cest)
{    
    float cap;
    Cest=0;
    for(int i=0;i<overSampling;i++)
    {
         if(!doOne(0.63,dex,cap))
             return false;        
         Cest+=cap;
    }
    Cest/=overSampling;
    // We are with cap > 300 pf, internal cap is neglectable Cest=Cest-INTERNAL_CAPACITANCE_IN_PF/pPICO;
    if(Cest<0.) Cest=0.;
    return true;
}


/**
 *  comput the fullness of the DMA buffer for a given sampling freq
 * The idea is if is ==511 it means that we need a slower sampling freq
 * @param dex
 * @param cap
 * @return 
 */
bool Capacitor::getRange(int dex, int &range)
{
    int resistance;
    zeroAllPins();
    // go
    bool doubled=(capScales[dex].doubled);
    TestPin::PULL_STRENGTH strength=capScales[dex].strength;
    if(doubled)
            _pB.pullDown(strength);
    else    
            _pB.setToGround();

    // start the DMA
    // max duration ~ 512 us
    uint16_t *samples;
    int nbSamples;
    
    DeltaADC delta(_pA,_pB);
    float period;
    
    if(!delta.setup(capScales[dex].rate,capScales[dex].scale,512)) return false;
    
    _pA.pullUp(strength);   
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    bool r=delta.get(nbSamples,&samples,period);
    _pA.pullDown(TestPin::PULL_LOW);   
    if(!r) return false;
    
    //    
    int limitB=2784;   
    int pointB=-1;

   
    for(int i=1;i<nbSamples;i++)
    {
        if(samples[i]>limitB) // 68%
        {
            pointB=i;
            i=4095;
        }
    }
    if(pointB==-1) pointB=nbSamples-1;
    range=pointB;
    range=(100*range)/nbSamples; // rescale to 0..100
    return true;
}

/**
 * 
 * @return 
 */

bool Capacitor::compute()
{
    if(computed) return true;
    computed=computeWrapper();
    return computed;
}

bool Capacitor::computeWrapper()
{
    AutoDisconnect ad;
    capacitance=0;
    int range;
    //float est;
    //getEsr(est);
    
    // check for big cap
    if(getRange(LAST_SCALE,range))
    {
        if(range>95) // out of scale, it is high cap..
        {
            return computeHiCap();
        }
    }
    // Check for small cap
     if(getRange(0,range))
    {
        if(range<24) // Low value, it is low cap..
        {
            return computeLowCap();
        }
    }
    
    // Search the best range...
    int n=LAST_SCALE;    
    int gotit=-1;
    for(int i=0;i<n;i++)
    {
      //  if(capScales[i].doubled) continue;
        if(!getRange(i,range)) continue;
        if(range<=93)
        {
            gotit=i;
            i=n+1;
        }
    }
    if(gotit==-1)
        return false;
    
    // Now loop on the range
    computeMediumCap(0*1+1*gotit,4,capacitance);    
    if(capacitance<0.) capacitance=0.;    
    return true;
}