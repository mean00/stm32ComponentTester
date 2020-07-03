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
bool Capacitor::computeLowCap()
{    
    calibrationValue(capacitance);
    if(capacitance<(MINIMUM_DETECTED_CAP+_pA._calibration.capOffsetInPf)/pPICO) 
    {
        return computeVeryLowCap();
    }
    if(capacitance<300./pPICO)
    {
        capacitance=capacitance-_pA._calibration.capOffsetInPf/pPICO;
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
 * @param nbSample
 * @param samples
 * @param resistance
 * @param period
 * @return 
 */
int vA,vB,iA,iB; // for debug
float Capacitor::computeCapacitance(int nbSamples, uint16_t *samples, int resistance, float period)
{    
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
    int mnTarget=mn+10; 
    int mxTarget=mx-(mx-mn)/4;
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
    c=c*period;
        
    return c;
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
