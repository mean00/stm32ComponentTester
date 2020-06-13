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
//
CycleClock clk;
float capz;


typedef struct CapScale
{
    float           capMin; // in PF
    float           capMax; // in pF
    int             frequency;
    TestPin::PULL_STRENGTH strength;
    bool            doubled;

};


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

bool Capacitor::doOne(float target,int frequency, const TestPin::PULL_STRENGTH strength,float &cap)
{
    int resistance;
    zeroAllPins();
    // go
    _pA.pullDown(strength);

    // start the DMA
    // max duration ~ 512 us
    uint16_t *samples;
    int nbSamples;

    if(!_pA.prepareTimer(frequency,512))
    {
        return false;
    }
    
    _pA.pullUp(strength);   
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    bool r=_pA.finishTimer(nbSamples,&samples);
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
    timeElapsed/=(float)frequency;

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

    _pB.setToGround();

    // start the DMA
    // max duration ~ 512 us
    uint16_t *samples;
    int nbSamples;
    
    float period;
    
    if(!_pA.prepareTimer(1000,512)) return false;
    
    TestPin::PULL_STRENGTH strength=TestPin::PULL_HI;
    
    _pA.pullUp(strength);   
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    bool r=_pA.finishTimer(nbSamples,&samples);
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
         if(!doOne(0.63,1000,TestPin::PULL_HI,cap))
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
        capacitance=0.;
        return false;
    }
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
         if(!doOne(0.9,0,TestPin::PULL_HI,cap))
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
    if(getRange(0,range))
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
    int n=1;    
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
