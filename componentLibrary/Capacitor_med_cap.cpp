#include "Arduino.h"
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



const Capacitor::CapScale capScalesSmall={500000,  ADC_SMPR_13_5,DSOADC::ADC_PRESCALER_6 ,TestPin::PULL_HI,true}; // Best we can do for small cap, i.e; between 200pf & 100 nf
const Capacitor::CapScale capScaleHigh={4000,      ADC_SMPR_41_5,DSOADC::ADC_PRESCALER_8 ,TestPin::PULL_LOW,false}; // Best we can do for big cap, i.e; between 10 uf and ~ 200 uf
const Capacitor::CapScale capScaleMed={100000,     ADC_SMPR_41_5,DSOADC::ADC_PRESCALER_8 ,TestPin::PULL_LOW,false}; // Best we can do for big cap, i.e; between 100 nf and ~ 10f uf



/**
 * Perform a DMA sampling buffer and extract 2 points
 * @return 
 */
#if 0
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
#endif

/**
 * For medium value cap, we take the sampling time needed so that
 * we have a full charge within 512 samples
 * Then we can take 2 points in the samples and compute C
 * The aim is to have a wide enough span so that the computation is somehow accurate
 */
bool Capacitor::computeMediumCap()
{    

    return false;
}


/**
 *  comput the fullness of the DMA buffer for a given sampling freq
 * The idea is if is ==511 it means that we need a slower sampling freq
 * @param dex
 * @param cap
 * @return 
 */
#if 0
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
#endif
/**
 * 
 * @return 
 */

bool Capacitor::compute()
{
    CapCurve curve;
    int deltaTime;
    switch(eval(capScalesSmall,curve, deltaTime))
    {
        case  EVAL_SMALLER_CAP:
                return computeVeryLowCap();
                break;
        case  EVAL_OK:
        case  EVAL_BIGGER_CAP:;
                break;
        case  EVAL_ERROR:
                return false;
                break;
    }
    switch(eval(capScaleMed,curve, deltaTime))
    {
        case  EVAL_SMALLER_CAP:
                return computeLowCap();
                break;
        case  EVAL_OK:
        case  EVAL_BIGGER_CAP:;
                break;
        case  EVAL_ERROR:
                return false;
                break;
    }
     switch(eval(capScaleHigh,curve, deltaTime))
     {
        case  EVAL_SMALLER_CAP:
                return computeMediumCap();
                break;
        case  EVAL_OK:
        case  EVAL_BIGGER_CAP:;
                return computeHiCap();
                break;
        case  EVAL_ERROR:
                return false;
                break;
     }
    return false;
}
#if 0
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
#endif



#if 0
{
    {  20     ,800,         ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_6  ,  2.17,   TestPin::PULL_HI,     true},   // 20pf   -> 800pf    2.17 us  500khz
    { 800     ,1.8*1000,    ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_6  ,  2.17,   TestPin::PULL_HI,     false},  // 800 pf -> 1.8 nf   2.17 us  500khz
    { 1.8*1000,5*1000,      ADC_SMPR_28_5,  DSOADC::ADC_PRESCALER_6  ,  3.42,   TestPin::PULL_HI,     false},  // 1.8nf  -> 5nf      3.42 us  300 khz
    {   5*1000,20*1000,     ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_6  ,  2.17,   TestPin::PULL_MED,    true},   // 5nf    -> 20 nf    2.17     500 khz
    {  20*1000,100*1000,    ADC_SMPR_41_5,  DSOADC::ADC_PRESCALER_6  ,  4.5,    TestPin::PULL_MED,    false},  // 20 nf  -> 100 nf   4.5 us   200khz
    { 100*1000,200*1000,    ADC_SMPR_71_5,  DSOADC::ADC_PRESCALER_8  ,  9.33,   TestPin::PULL_MED,    false},  // 100 nf -> 200 nF   7.56 us  150 khz
    { 600*1000,1200*1000,   ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_8  ,  2.89,   TestPin::PULL_LOW,    true},   // 600nf  -> 1.2uf    2.89     350 khz
    {1200*1000,5000*1000,   ADC_SMPR_41_5,  DSOADC::ADC_PRESCALER_8  ,  6,      TestPin::PULL_LOW,    false},  // 1.2uF  -> 5uf      6 us     150 khz
    {5000*1000,20*1000*10000,ADC_SMPR_239_5,DSOADC::ADC_PRESCALER_6  ,  21,     TestPin::PULL_LOW,    false},  // 5uF    -> 20uf     21 us    50 khz
};
#define LAST_SCALE ((sizeof(capScales)/sizeof(CapScale))-1)
#endif
/**
 * 
 * @param sc
 * @param c
 * @param deltaTime
 * @return 
 */
Capacitor::CapEval Capacitor::eval(const CapScale &sc,CapCurve &curve, int &deltaTime)
{
    int resistance;
    zeroAllPins();
    // go
    bool doubled=sc.doubled;
    TestPin::PULL_STRENGTH strength=sc.strength;
    if(doubled)
        _pB.pullDown(strength);
    else    
        _pB.setToGround();

    uint16_t *samples;
    int nbSamples;
    DeltaADCTime delta(_pA,_pB);
    float period;
    
    if(!delta.setup(sc.fq,1024)) return EVAL_ERROR;
    
    _pA.pullUp(strength);   
    
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    bool r=delta.get(nbSamples,&samples,period);
    _pA.pullDown(TestPin::PULL_LOW);   
    if(!r) return EVAL_ERROR;
    
    
    WaveForm wave(nbSamples-1,samples+1);
    int mn,mx;
    wave.searchMinMax(mn,mx);
    
    if( (mx-mn)<100) // flat
    {
        if(mx<50) // stuck to zero
            return EVAL_BIGGER_CAP;
        else
            return EVAL_SMALLER_CAP;
    }
    
    // Search start of ramp up above noise
    int iA,iB,vA,vB;
    int tgt=mn+(((mx-mn)*63)/100); // look for 0.666= ~ e-1
    wave.searchValueAbove(mn+50, iA, vA, 0);
    wave.searchValueAbove(tgt, iB, vB, iA);
    
    if((iB-iA)<100) return EVAL_SMALLER_CAP; // the pulse is too quick 
    if((vB-vA)<400) return EVAL_BIGGER_CAP; // A & B are too close, we must zoom out
    
#if 1    
    TesterGfx::drawCurve(nbSamples,samples);
    //TesterControl::waitForAnyEvent();
#endif    
    
    deltaTime=iB-iA;
    curve.iMax=iB;
    curve.iMin=iA;
    curve.resistance=resistance;
    curve.vMax=vB;
    curve.vMin=vA;
    curve.period=1./(float)sc.fq;
    return EVAL_OK;    
}