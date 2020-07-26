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
/**
 * 
 * @return 
 */


const Capacitor::CapScale lowCaps[]=
{
    {8*1000,  TestPin::PULL_HI,false}, //  50 nf
    {16*1000, TestPin::PULL_HI,false}, //  20 nf
    {36*1000, TestPin::PULL_HI,false}, //  10 nf
    {60*1000, TestPin::PULL_HI,false}, //  5 nf
    {200*1000,TestPin::PULL_HI,false}, //  5 nf
    {360*1000,TestPin::PULL_HI,false}, //  1 nf
    
    {250*1000,TestPin::PULL_HI,true}, //  2 nf
    {360*1000,TestPin::PULL_HI,true}, //  1 nf
    {400*1000,TestPin::PULL_HI,true}, //  330 pf nf   
    
};

bool Capacitor::computeLowCap(bool overSample)
{    
    int oversampling=7;
    if(!overSample) oversampling=0;
    
    int n=sizeof(lowCaps)/sizeof(Capacitor::CapScale);
    bool r= computeCapRange(n,lowCaps,oversampling);
    if(r)
    { // take parasitic cap into consideration
        float offset=(float)_pA._calibration.capOffsetInPf/pPICO;
        if(capacitance<offset)
            return false;
        capacitance-=offset;
        return true;
        
    }
    return false;
}
/**
 * 
 * @param overSample
 * @return 
 */
bool Capacitor::computeMedInternalCap(float &c)
{
    int n=sizeof(lowCaps)/sizeof(Capacitor::CapScale);
    const Capacitor::CapScale *scale=lowCaps+n-1;
    c=0.;
    int overSampling=0;
    capacitance=0;
    for(int i=0;i<8;i++)
    {
        CapCurve curve;
        int deltaTime;
        if(eval(*scale,curve, deltaTime)==EVAL_OK)
        {
            capacitance+=computeCapacitance(curve);;
            overSampling++;            
        }
    }
    c=(capacitance)/(float)overSampling;
    return true;
}


/**
 * 
 * @param sc
 * @param c
 * @param deltaTime
 * @return 
 */
bool Capacitor::calibrationLow(TestPin &_pA, TestPin &_pB,float &cap)
{
    CapCurve curve;
    int resistance;
    zeroAllPins();
    // go
    bool doubled=true;
    TestPin::PULL_STRENGTH strength=TestPin::PULL_HI;
    _pB.pullDown(strength);

    uint16_t *samples;
    int nbSamples;
    DeltaADCTime delta(_pA,_pB);
    float period;
    
    
    if(!delta.setup(360000,1024)) 
    {
        xAssert(0);
        return EVAL_ERROR;
    }
    _pA.pullUp(strength);   
    
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    bool r=delta.get(nbSamples,&samples,period);
    _pA.pullDown(TestPin::PULL_LOW); 
    zeroAllPins();   
    if(!r) 
        return EVAL_ERROR;    
    
    WaveForm wave(nbSamples-1,samples+1);
    int mn,mx;
    wave.searchMinMax(mn,mx);
    
    if(mx<150) // stuck to zero
            return EVAL_BIGGER_CAP;
    
    // Search start of ramp up above noise
    int iA,iB,vA,vB;
    int tgt=mn+(((mx-mn)*85)/100); // look for 0.666= ~ e-1
    wave.searchValueAbove(mn+10, iA, vA, 0);
    wave.searchValueAbove(tgt, iB, vB, iA);
    
    if(vB<(4095/3)) return EVAL_BIGGER_CAP; // still charging...
    
    curve.iMax=iB;
    curve.iMin=iA;
    curve.resistance=resistance;
    curve.vMax=vB;
    curve.vMin=vA;
    curve.period=1./(float)360000;
    curve.nbSamples=nbSamples;
    
    cap=computeCapacitance(curve);
#if 0    
    char st[20];
    TesterGfx::clear();
    TesterGfx::print(10,60,"Low cal");
    Component::prettyPrint(cap,"F",st);
    TesterGfx::print(10,90,st);
    TesterControl::waitForAnyEvent();
#endif    
    return true;    
}


//EOF