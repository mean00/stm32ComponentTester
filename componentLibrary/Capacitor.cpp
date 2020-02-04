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

#define pPICO (1000.*1000.*1000.*1000.)

typedef struct CapScale
{
    float           capMin; // in PF
    float           capMax; // in pF
    adc_smp_rate    rate;
    adc_prescaler   scale;
    float           tickUs;
    TestPin::PULL_STRENGTH strength;
    bool            doubled;

};

const CapScale capScales[]=
{
    {  20     ,800,         ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_6,2.17,   TestPin::PULL_HI,     true}, // 20pf-->800pf
    { 800     ,1.8*1000,    ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_6,2.17,   TestPin::PULL_HI,     false},  // max 50nF
    { 1.8*1000,5*1000,      ADC_SMPR_28_5,  ADC_PRE_PCLK2_DIV_6,3.42,   TestPin::PULL_HI,     false},  // max 1Uf
    {   5*1000,20*1000,     ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_6,2.17,   TestPin::PULL_MED,    true},  // and up
    {  20*1000,100*1000,    ADC_SMPR_41_5,  ADC_PRE_PCLK2_DIV_6,4.5,    TestPin::PULL_MED,    false},  // and up
    { 100*1000,200*1000,    ADC_SMPR_71_5,  ADC_PRE_PCLK2_DIV_8,9.33,   TestPin::PULL_MED,    false},  // and up
    { 600*1000,1200*1000,   ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_8,2.89,    TestPin::PULL_LOW,    true},  // 600nf-> 1.2uf
    {1200*1000,5000*1000,   ADC_SMPR_41_5,  ADC_PRE_PCLK2_DIV_8,6,       TestPin::PULL_LOW,    false},  // 1.2uF-> 5uf
    {5000*1000,20*1000*10000,ADC_SMPR_239_5, ADC_PRE_PCLK2_DIV_6,21,    TestPin::PULL_LOW,    false},  // 1.2uF-> 5uf
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

bool Capacitor::doOne(int dex, float &cap)
{
    int resistance;
    if(!zero(10)) return false;    
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
    
    if(!_pA.prepareDmaSample(capScales[dex].rate,capScales[dex].scale,512)) 
        return false;        
    // Go!
    _pA.pullUp(strength);   
    if(!_pA.finishDmaSample(nbSamples,&samples)) 
    {
        return false;
    }
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    _pA.pullDown(TestPin::PULL_LOW);   
    
    
    int limitA,limitB;

    
    limitA=10;
    limitB=2784;

    if(doubled)
    {
        limitA=4095/2+1;
        limitB=4095*4/5+1;
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
    if(pointA==-1 || pointA >512) 
        return false;
    for(int i=pointA+1;i<nbSamples;i++)
    {
        if(samples[i]>limitB) // 68%
        {
            pointB=i;
            i=4095;
        }
    }
    if(pointB==-1) pointB=nbSamples-1;
    
    if((pointB-pointA)<1) return false; // not enough points, need at least one
    
    // Compute
    float timeElapsed=(pointB-pointA);
    timeElapsed*=capScales[dex].tickUs;
    timeElapsed/=1000000.; // In seconds

    float valueA=samples[pointA];
    float valueB=samples[pointB];

    
    if(doubled)
    {
        valueA=2*valueA-4095;
        valueB=2*valueB-4095;
    }
    
    if(fabs(valueA-4095.)<2) return false;
    if(fabs(valueB-4095.)<2) return false;
    
    float den=(4095-valueA)/(4095-valueB);
    
    if(fabs(den-2.718)<0.01) 
        return false;
    den=log(den);
    cap=timeElapsed/(resistance*den);
    return true;
}



/**
 */
bool Capacitor::computeHiCap(int dex,int overSampling,float &Cest)
{    
    float cap;
    capacitance=0;
    for(int i=0;i<overSampling;i++)
    {
         if(!doOne(dex,cap))
             return false;        
         capacitance+=cap;
    }
    Cest/=overSampling;
    return true;
}
/**
 * 
 * @return 
 */
bool Capacitor::compute()
{
    capacitance=0;
    float CEstimated;
    
    if(!Capacitor::quickEval(CEstimated))
        return false;
    
    // Depending on CEst, pick the best scale...
    if(CEstimated>(4.6/1000000.)) // more than 5 uf,use time based method
    {
        int timeUs,resistance,value;
        if(!Capacitor::doOneQuick(TestPin::PULL_LOW, false, 0.68,timeUs,resistance,value))
            return false;
        capacitance=computeCapacitance(timeUs,resistance,value);
        return true;
    }
    // Search the best range...
    int n=LAST_SCALE;    
    int gotit=-1;
    for(int i=0;i<n;i++)
    {
        if(capScales[i].doubled) continue;
        if(!computeHiCap(i,4,CEstimated))
            continue;
        if(CEstimated<capScales[i].capMin)
        {
            
            gotit=i;
            i=n+1;
        }
    }
    if(gotit==-1)
        return false;
    float offset=INTERNAL_CAPACITANCE_IN_PF;
    offset/=pPICO; // In pf
    capacitance=CEstimated-offset;
    if(capacitance<0.) capacitance=0.;    
    return true;
}
/**
 * 
 * @param time
 * @param resistance
 * @param actualValue
 * @return 
 */

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
