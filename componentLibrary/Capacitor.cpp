/*
 * Capacitor tester
*/

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
//
CycleClock clk;
float capz;


typedef struct CapScale
{
    float           capMin; // in PF
    float           capMax; // in pF
    adc_smp_rate    rate;
    DSOADC::Prescaler   scale;
    float           tickUs;
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
    
    
    int limitA,limitB;

    
    limitA=4095.*0.1;
    limitB=4095.*target;

   
  
    
    // We need 2 points...
    // Lookup up 5% and 1-1/e
    int pointA=-1,pointB=-1;
    for(int i=1;i<nbSamples-2;i++)
    {
        if(samples[i]>limitA && samples[i]<=samples[i+1] && samples[i+1]<=samples[i+2]) // make sure it is not a glitch
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
    timeElapsed*=period;

    float valueA=samples[pointA];
    float valueB=samples[pointB];

 
    
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
    calibrationValue(capacitance);
    if(capacitance<300./pPICO)
    {
        capacitance=capacitance-_pA._calibration.capOffsetInPf/pPICO;
    }
    if(capacitance<MINIMUM_DETECTED_CAP/pPICO) 
    {
        return computeVeryLowCap();
        capacitance=0.;
        return false;
    }
    return true;
}


/**
 * 
 * @return 
 */
int vA,vB,iA,iB;
bool Capacitor::computeVeryLowCap()
{   
    TestPin *p1,*p2;
    if(_pB.pinNumber()==2)
    {
        p1=&_pB;
        p2=&_pA;
    }else if(_pA.pinNumber()==2)
    {
        p1=&_pA;
        p2=&_pB;        
    }
    else
    {
        return false;      
    }
    p2->setToGround();
    p1->pullDown(TestPin::PULL_LOW);
    xDelay(10);
    //bool  TestPin::pulseTime(int nbSamples, int samplingFrequency, TestPin::PULL_STRENGTH strength,   int &nbSample,  uint16_t **xsamples,int &samplingTime,int &res)
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
    if(!p1->pulseTime(1024,500,TestPin::PULL_HI,nbSamples,&samples,samplingTime,resistance))
    {
        return false;
    }
    if(!p1->pulseTime(1024,500,TestPin::PULL_HI,nbSamples,&samples,samplingTime,resistance))
    {
        return false;
    }
    TesterGfx::drawCurve(nbSamples, samples);
    // Search for min & max
    int mn=4095;
    int mx=0;
    int mnIndex=0,mxIndex=0;
    for(int i=1;i<nbSamples;i++)
    {
        int x=samples[i];
        if(x>mx)
        {
            mx=x;
            mxIndex=i;
            continue;
        }
        if(x<mn)
        {
            mn=x;
            mnIndex=i;
            continue;
        }
    }
    int mnTarget=mn+(mx-mn)/8;
    int mxTarget=mx-(mx-mn)/8;
    bool found=false;
    
    for(int i=mnIndex;i<nbSamples && found==false;i++)
    {
        int x=samples[i];
        if(x>mnTarget)
        {
            vA=x;
            iA=i;
            found=true;
        }
    }
    found=false;
    for(int i=iA;i<nbSamples && found==false;i++)
    {
        int x=samples[i];
        if(x>mxTarget)
        {
            vB=x;
            iB=i;
            found=true;
        }
    }
    //
    float c=(iB-iA);
    c/=(float)resistance;
    c=c/log( (float)vB/(float)vA);
    c=c*(float)samplingTime/(float)F_CPU;
    
    if(c>(float)p1->_calibration.capOffsetHighInPf/pPICO)
        c-=(float)p1->_calibration.capOffsetHighInPf/pPICO;
    
    capacitance=c;
    return true;
}
/**
 * 
 * @return 
 */
bool Capacitor::calibrationValue(float &c)
{        
    float Cest=0;
    int overSampling=4;
    for(int i=0;i<overSampling;i++)
    {
        float cap;
         if(!doOne(0.9,0,cap))
             return false;        
         Cest+=cap;
    }
    Cest/=overSampling;
    c=Cest;
    return true;
}
/**
 * 
 * @param c
 * @return 
 */
bool Capacitor::quickEval(float &c)
{
    if(!computeHiCap()) return false;
    if(capacitance<300./pPICO)
    {
        capacitance=capacitance-_pA._calibration.capOffsetInPf/pPICO;
    }
    if(capacitance<MINIMUM_DETECTED_CAP/pPICO) 
    {
        capacitance=0.;
        return false;
    }
    if(capacitance<=0.0)
        return false;
    c=capacitance;
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
 */
bool Capacitor::computeHiCap()
{    
    int timeUs,resistance,value;
    int overSampling=2;
    int resTotal=0,timeTotal=0,valueTotal=0;
    
    // with large cap, only charge them up to 29% so that it does not exceed 1v
    // in case they are reversed polarised
    for(int i=0;i<overSampling;i++)
    {
#if 1 // full charge
        if(!Capacitor::doOneQuick(TestPin::PULL_LOW, false, 0.7,timeUs,resistance,value))
            return false;
#else         // or only up to 1v ?
        if(!Capacitor::doOneQuick(TestPin::PULL_LOW, false, 0.28,timeUs,resistance,value))
            return false;
#endif
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
    zeroAllPins();
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
