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


const Capacitor::CapScale capScalesSmall={500000,TestPin::PULL_HI,true}; // Best we can do for small cap, i.e; between 200pf & 100 nf
const Capacitor::CapScale capScaleHigh={4000,    TestPin::PULL_LOW,false}; // Best we can do for big cap, i.e; between 10 uf and ~ 200 uf
const Capacitor::CapScale capScaleMed={100000,   TestPin::PULL_LOW,false}; // Best we can do for big cap, i.e; between 100 nf and ~ 10f uf



const Capacitor::CapScale probePoints[]={
    {500*1000,  TestPin::PULL_HI,   true},
    {500*1000,  TestPin::PULL_HI,   false},
    {60*1000,   TestPin::PULL_HI,   false},
    {35*1000,   TestPin::PULL_HI,   false},
    
    {100*1000,   TestPin::PULL_MED,   false},
    {50*1000,   TestPin::PULL_MED,   false},    
    {12*1000,   TestPin::PULL_MED,   false},    
    {6*1000,   TestPin::PULL_MED,   false},    
    
    {4*1000,    TestPin::PULL_LOW,  false},
    {100*1000,  TestPin::PULL_LOW,false}
};

/**
 * 
 * @return 
 */
bool Capacitor::quickEval(TestPin &a, TestPin &b,TestPin &c)
{
    Capacitor cap(a,b,c);
    return cap.quickEval();
}

/**
 * Try to just detect if a cap is connected or not
 * It will NOT detec small cap (less than 100 pf)
 * 
 * @return 
 */
bool Capacitor::quickEval()
{
    int n=sizeof(probePoints)/sizeof(Capacitor::CapScale);
    for(int i=0;i<n;i++)
    {
        CapCurve curve;
        int deltaTime;
        switch(eval(probePoints[i],curve, deltaTime,true))
        {
            case  EVAL_OK:
                    return true;
            default:
                    break;
        }
    }
    return false;
}

//--

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
                return computeHighCap();
                break;
        case  EVAL_ERROR:
                return false;
                break;
     }
    return false;
}

/**
 * 
 * @param sc
 * @param c
 * @param deltaTime
 * @return 
 */
Capacitor::CapEval Capacitor::eval(const CapScale &sc,CapCurve &curve, int &deltaTime, bool largeWindow)
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
    
#if 1    
    TesterGfx::drawCurve(nbSamples,samples);
    //TesterControl::waitForAnyEvent();
#endif    

    WaveForm wave(nbSamples-1,samples+1);
    int mn,mx;
    wave.searchMinMax(mn,mx);
    
    
    if( (mx-mn)<100) // flat
    {
        if(mx<150) // stuck to zero
            return EVAL_BIGGER_CAP;
        else
            return EVAL_SMALLER_CAP;
    }
    
    // Search start of ramp up above noise
    int iA,iB,vA,vB;
    int tgt=mn+(((mx-mn)*70)/100); // look for 0.666= ~ e-1
    wave.searchValueAbove(mn+50, iA, vA, 0);
    wave.searchValueAbove(tgt, iB, vB, iA);
    
    if(vB<(4095/3)) return EVAL_BIGGER_CAP; // still charging...
    
    // If largeWindow is on, we dont require as much difference between min & max
    // it is for probing support
    // if largeWindow is off, we must have at leasst nbSample/8 samples, i.e. about 60
    int minSamples=0;
    if(!largeWindow)
        minSamples=nbSamples/8;
    else
        minSamples=25;
    
    if((iB-iA)<(minSamples)) return EVAL_SMALLER_CAP; // the pulse is too quick 
    if((vB-vA)<400) return EVAL_BIGGER_CAP; // A & B are too close, we must zoom out
    
    
    deltaTime=iB-iA;
    curve.iMax=iB;
    curve.iMin=iA;
    curve.resistance=resistance;
    curve.vMax=vB;
    curve.vMin=vA;
    curve.period=1./(float)sc.fq;
    curve.nbSamples=nbSamples;
    return EVAL_OK;    
}

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
bool Capacitor::calibrationValue(float &c)
{        
#if 0
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
#endif    
}
/**
 * 
 * @param c
 * @return 
 */
bool Capacitor::quickEval(float &c)
{
    if(!computeHighCap()) return false;
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

/**
 * 
 * @param curve
 * @return 
 */
float Capacitor::computeCapacitance(CapCurve &curve)
{
      float c=(curve.iMax-curve.iMin);
        c/=(float)curve.resistance;
        c=c/log( (float)(4095.-curve.vMin)/(float)(4095.-curve.vMax));
        c=c*curve.period;
        return c;
}
// EOF
