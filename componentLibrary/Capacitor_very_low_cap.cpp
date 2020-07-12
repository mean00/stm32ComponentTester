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
 * @param fq
 * @param clockPerSample
 * @param cap
 * @return 
 */
Capacitor::CapEval Capacitor::quickEvalSmall(int fq, int clockPerSample)
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
   
    p2->setToGround();
    p1->pullDown(TestPin::PULL_LOW);
    xDelay(10);
    
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
    
    if(!p1->pulseTimeDelta(*p2,clockPerSample, 64,fq,TestPin::PULL_HI,nbSamples,&samples,resistance))
    {
        return EVAL_ERROR;
    }
    
    
    TesterGfx::clear();
    TesterGfx::drawCurve(nbSamples,samples);
    
    WaveForm wave(nbSamples,samples);
    int mn,mx;
    wave.searchMinMax(mn,mx);
    
    if( (mx-mn)<100) // flat
    {
        if(mx<150) // stuck to zero
            return EVAL_BIGGER_CAP;
        else
            return EVAL_SMALLER_CAP;
    }
    int iA,iB,vA,vB;    
    
    wave.searchValueAbove(mn+((mx-mn)/2), iB, vB, 0);
    wave.searchValueAbove(mx-1, iA, vA, iB);
    bool ok=true;
    
    // B is middle point between min & max
    
    vA=4095-vA;
    vB=4095-mx;
    
    
    char st[20];
    sprintf(st,"M:%d",iB);
    TesterGfx::print(10,20,st);
    
    Component::prettyPrint(fq,"Hz",st);
    TesterGfx::print(10,50,st);
    
    sprintf(st,"%d",clockPerSample);
    TesterGfx::print(10,80,st);
    
    sprintf(st,"Md:%d",iB);
    TesterGfx::print(70,80,st);
    
    
    TesterControl::waitForAnyEvent();
   
    float r=(float)iB/(float)nbSamples;
    if(r<1/5.) return EVAL_SMALLER_CAP;
    return EVAL_OK;
}

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
    
    TesterGfx::drawCurve(nbSamples,samples);
    
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
    
    cap=computeCapacitance(  iA,iB,vA,vB, resistance,  period);
    
    
    
    char st[20];
    Component::prettyPrint(cap,"F",st);
    TesterGfx::print(10,20,st);
    
    Component::prettyPrint(fq,"Hz",st);
    TesterGfx::print(10,50,st);
    
    sprintf(st,"%d",clockPerSample);
    TesterGfx::print(10,80,st);
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
   
   int strobo=64; // actual sampling freq=F_CPU/strobo =~ 1 Mhz
   bool found=false;
   for(int i=0;i<6;i++)
   {
       if(quickEvalSmall(1200,strobo*8)==EVAL_OK)
       {
           found=true;
            break;
       }
       strobo>>=1;
   }
   if(!found)
       return false;
   
   er=evalSmall(1200,strobo,cap);
   capacitance=cap;   
   return er==EVAL_OK;
}