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


const Capacitor::CapScale SmallBegin={100*1000,  TestPin::PULL_HI,false}; // Best we can do for small cap, i.e; between 200pf & 100 nf
const Capacitor::CapScale SmallEnd={  8*1000,  TestPin::PULL_HI,false}; // Best we can do for small cap, i.e; between 200pf & 100 nf
const Capacitor::CapScale HighBegin={ 4000,       TestPin::PULL_LOW,false}; // 
const Capacitor::CapScale MedEnd={100*1000,      TestPin::PULL_MED,false}; // Best we can do for big cap, i.e; between 100 nf and ~ 10f uf
const Capacitor::CapScale MedBegin={2*1000,      TestPin::PULL_MED,false}; // Best we can do for big cap, i.e; between 100 nf and ~ 10f uf



const Capacitor::CapScale probePoints[]={
    {400*1000,  TestPin::PULL_HI,   true},
    {400*1000,  TestPin::PULL_HI,   false},
    {60*1000,   TestPin::PULL_HI,   false},
    {35*1000,   TestPin::PULL_HI,   false},
    
    {100*1000,  TestPin::PULL_MED, false},
    {50*1000,   TestPin::PULL_MED,  false},    
    {12*1000,   TestPin::PULL_MED,  false},    
    {6*1000,    TestPin::PULL_MED,   false},    
    
    {35*1000,    TestPin::PULL_LOW,  false},
    {8*1000,     TestPin::PULL_LOW,  false},
    {1200,       TestPin::PULL_LOW,  false}
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
    CapCurve curve;
    int deltaTime;

    int n=sizeof(probePoints)/sizeof(Capacitor::CapScale);
    Capacitor::CapEval ev=eval(probePoints[0],curve, deltaTime,true);
    switch(ev)
    {
        case EVAL_OK:    
            return true;
            break;
        case EVAL_SMALLER_CAP:
            // quick eval very low cap 
            switch(quickProbe())
            {
                case EVAL_OK:
                case EVAL_BIGGER_CAP: return true;break;
                default: return false;
            }
            break;
        default:
            return false; // bigger cap
    }
    for(int i=1;i<n;i++)
    {
        switch(eval(probePoints[i],curve, deltaTime,true))
        {
            case  EVAL_OK:
                    return true;
            case  EVAL_SMALLER_CAP:
                    return false; // no need to go further
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
    eval(SmallBegin,curve, deltaTime); // dummy scan to avoid 1st one being garbage
        
    switch(eval(SmallBegin,curve, deltaTime))
    {
        case  EVAL_SMALLER_CAP:
                Logger("Very Low Cap-1\n");
                return computeVeryLowCap();
                break;
        case  EVAL_OK:
        case  EVAL_BIGGER_CAP:;
                break;
        case  EVAL_ERROR:
                return false;
                break;
    }
    switch(eval(MedEnd,curve, deltaTime))
    {
        case  EVAL_SMALLER_CAP:
                // It's either a small or the beginning of med or a little bit too
                // small for low cap
                if(EVAL_SMALLER_CAP==eval(MedBegin,curve,deltaTime))
                {
                    if(false==computeLowCap())
                    {
                        Logger("Very Low Cap-2\n");
                        return computeVeryLowCap();
                    }
                }
                return true;
                break;
        case  EVAL_OK:
        case  EVAL_BIGGER_CAP:;
                break;
        case  EVAL_ERROR:
                return false;
                break;
    }
     switch(eval(HighBegin,curve, deltaTime))
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
    
    if(sc.fq>400000) xAssert(0);
    if(!delta.setup(sc.fq,1024)) 
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
    
#if 0   
    char st[20];
    sprintf(st,"%d k",sc.fq/1000);    
    TesterGfx::drawCurve(nbSamples,samples);
    TesterGfx::print(10,25,st);
    TesterControl::waitForAnyEvent();
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
    int tgt=mn+(((mx-mn)*85)/100); // look for 0.666= ~ e-1
    wave.searchValueAbove(mn+10, iA, vA, 0);
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
bool Capacitor::compute1nfRange(float &c)
{        
    if(!computeLowCap()) return false;
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
float Capacitor::computeCapacitance(int ia, int ib, int va, int vb,int resistance, float period)
{    
   
    //
    float c=(ib-ia);
    c/=(float)resistance;
    c=c/log( (float)(4095.-vb)/(float)(4095.-va));
    c=c*period;        
    return c;
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
    return computeCapacitance(iA, iB, vA,vB, resistance,  period);
    //
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
