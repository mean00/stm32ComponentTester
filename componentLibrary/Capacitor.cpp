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
float capz;
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
    {  20     ,800,         ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_6,  2.17,   TestPin::PULL_HI,     true},   // 20pf   -> 800pf
    { 800     ,1.8*1000,    ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_6,  2.17,   TestPin::PULL_HI,     false},  // 800 pf -> 1.8 nf
    { 1.8*1000,5*1000,      ADC_SMPR_28_5,  ADC_PRE_PCLK2_DIV_6,  3.42,   TestPin::PULL_HI,     false},  // 1.8nf  -> 5nf
    {   5*1000,20*1000,     ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_6,  2.17,   TestPin::PULL_MED,    true},   // 5nf    -> 20 nf
    {  20*1000,100*1000,    ADC_SMPR_41_5,  ADC_PRE_PCLK2_DIV_6,  4.5,    TestPin::PULL_MED,    false},  // 20 nf  -> 100 nf
    { 100*1000,200*1000,    ADC_SMPR_71_5,  ADC_PRE_PCLK2_DIV_8,  9.33,   TestPin::PULL_MED,    false},  // 100 nf -> 200 nF
    { 600*1000,1200*1000,   ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_8,  2.89,   TestPin::PULL_LOW,    true},   // 600nf  -> 1.2uf
    {1200*1000,5000*1000,   ADC_SMPR_41_5,  ADC_PRE_PCLK2_DIV_8,  6,      TestPin::PULL_LOW,    false},  // 1.2uF  -> 5uf
    {5000*1000,20*1000*10000,ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_6,  21,     TestPin::PULL_LOW,    false},  // 5uF    -> 20uf
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
 * Perform a DMA sampling buffer and extract 2 points
 * @return 
 */

bool Capacitor::doOne(float target,int dex, float &cap)
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
    limitB=4095.*target;

    if(doubled)
    {
        limitA=4095/2+10;
        limitB=4095.*(1.+target)/2.+1;
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
    
    float den=(4095.-(float)valueA)/(4095.-(float)valueB);
    
    if(fabs(den-2.718)<0.01) 
        return false;
    den=log(den);
    cap=timeElapsed/(resistance*den);
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
    
    
    int limitB;   
    limitB=2784;
    if(doubled)
    {        
        limitB=4095*4/5+1; // Make it better
    }
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
 * 
 * @return 
 */
bool Capacitor::computeLowCap()
{    
    float cap;
    float Cest=0;
    int overSampling=4;
    for(int i=0;i<overSampling;i++)
    {
         if(!doOne(0.9,0,cap))
             return false;        
         Cest+=cap;
    }
    Cest/=overSampling;
    capacitance=Cest;
    if(capacitance<300./pPICO)
    {
        capacitance=Cest-INTERNAL_CAPACITANCE_IN_PF/pPICO;
    }
    if(capacitance<0.) capacitance=0.;
    return true;
}

/**
 * 
 * @return 
 */
bool Capacitor::compute()
{
    capacitance=0;
    int range;
    float CEstimated;
    
    // check for big cap
    if(getRange(LAST_SCALE,range))
    {
        if(range>505) // out of scale, it is high cap..
        {
            return computeHiCap();
        }
    }
    // Check for small cap
     if(getRange(0,range))
    {
        if(range<120) // Low value, it is low cap..
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
        if(range<=480)
        {
            gotit=i;
            i=n+1;
        }
    }
    if(gotit==-1)
        return false;
    
    // Now loop on the range
    computeMediumCap(0*1+1*gotit,4,CEstimated);    
    capacitance=CEstimated;
    if(capacitance<0.) capacitance=0.;    
    return true;
}
/**
 * This is for high value cap. They need a lot of time to charge so we can't use a dma filled buffer
 * instead we'll poll their voltage
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

/**
 */
bool Capacitor::computeHiCap()
{    
    int timeUs,resistance,value;
    int overSampling=2;
    int resTotal=0,timeTotal=0,valueTotal=0;
    for(int i=0;i<overSampling;i++)
    {
        if(!Capacitor::doOneQuick(TestPin::PULL_LOW, false, 0.70,timeUs,resistance,value))
            return false;
        resTotal+=resistance;
        timeTotal+=timeUs;
        valueTotal+=value;
    }
    // correct B
    // Some voltage is dropped due to the parasitic resistor on the B pin
    // compensate for that
    float v=valueTotal;
    float alpha=(float)_pB.getRes(TestPin::GND)/(float)(_pA.getRes(TestPin::PULLUP_LOW));
    float coef=(v)*(alpha)-4095.*alpha*overSampling;
    v=v+coef;
    v=(v+(float)overSampling/2.)/(float)overSampling;
    valueTotal=v;
    capacitance=computeCapacitance(timeTotal,resTotal,valueTotal);
    return true;
}
/**
 * 
 * @param strength
 * @param doubled
 * @param percent
 * @param timeUs
 * @param resistance
 * @param value
 * @return 
 */
bool Capacitor::doOneQuick(TestPin::PULL_STRENGTH strength, bool doubled, float percent,int &timeUs, int &resistance,int &value)
{
    if(!zero(10)) return false;    
    // go
    if(!doubled)
        _pB.setToGround();
    else
        _pB.pullDown(strength);
    _pA.pullUp(strength);
    
    // Wait for the ADC value to go over 4095*percent
    // We introduce a small error here due to the fact the ADC
    // is starting too late
    // compensated by calibration
    int timeOne,valueOne;
    if(!_pA.fastSampleUp(10,4095.*percent,valueOne,value,timeOne,timeUs)) 
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
    _pA.setToGround();
    
    
    
    float den=(4095.-(float)valueOne)/(4095.-(float)value);
    
    if(fabs(den-2.718)<0.01) 
        return false;
    den=log(den);
    capz=(timeUs-timeOne)/(resistance*den);
    capz=capz/1000000.; // us -> sec
    
    return true;
}


// EOF
