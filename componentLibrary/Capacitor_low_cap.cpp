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

//EOF