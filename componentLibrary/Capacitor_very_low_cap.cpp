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
/**
 * 
 * @return 
 */
Capacitor::CapEval Capacitor::evalSmall(int fq, int clockPerSample, float &cap)
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
        TesterGfx::clear();
        TesterGfx::print(10,60,"Connect one leg");
        TesterGfx::print(10,90,"to center");
        TesterControl::waitForAnyEvent();
        return EVAL_ERROR;      
    }
    p2->setToGround();
    p1->pullDown(TestPin::PULL_LOW);
    xDelay(10);
    
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
    
    if(!p1->pulseTimeDelta(*p2,clockPerSample, 1024,fq,TestPin::PULL_HI,nbSamples,&samples,resistance))
    {
        return EVAL_ERROR;
    }
    
    float period=F_CPU;
    period=(float)(clockPerSample)/period;
#define OFFSET 5    
    WaveForm wave(nbSamples-OFFSET,samples+OFFSET);
    int mn,mx;
    wave.searchMinMax(mn,mx);
    
    if( (mx-mn)<100) // flat
    {
        if(mx<150) // stuck to zero
            return EVAL_BIGGER_CAP;
        else
            return EVAL_SMALLER_CAP;
    }
    // look for the starting minimum
    // Search start of ramp up above noise
    int iA,iB,vA,vB;
    int tgt=mn+(((mx-mn)*70)/100); // look for 0.666= ~ e-1
    wave.searchValueAbove(mn+50, iA, vA, OFFSET);
    wave.searchValueAbove(tgt, iB, vB, iA);
    
    if(vB<(4095/3)) return EVAL_BIGGER_CAP; // still charging...
    
    // If largeWindow is on, we dont require as much difference between min & max
    // it is for probing support
    // if largeWindow is off, we must have at leasst nbSample/8 samples, i.e. about 60
    int minSamples=0;

    minSamples=nbSamples/8;
    
    if((iB-iA)<(minSamples)) return EVAL_SMALLER_CAP; // the pulse is too quick 
    if((vB-vA)<400) return EVAL_BIGGER_CAP; // A & B are too close, we must zoom out
    
    cap=computeCapacitance(  nbSamples,   samples, resistance,  period);
    
    TesterGfx::drawCurve(nbSamples,samples);
    
    char st[20];
    Component::prettyPrint(cap,"F",st);
    TesterGfx::print(10,20,st);
    TesterControl::waitForAnyEvent();
    
    return EVAL_OK;  

}
/**
 * 
 * @return 
 */
bool Capacitor::computeVeryLowCap()
{   
   
   Capacitor::CapEval er;
   float cap;
   
   er=evalSmall(1000,8,cap);
   er=evalSmall(1000,16,cap);
   er=evalSmall(2000,8,cap);
   er=evalSmall(2000,16,cap);
   
   capacitance=cap;
   
   return er==EVAL_OK;
}