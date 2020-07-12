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


typedef struct VeryLowScale
{
    int signalFrequency;
    int s512; // apparent sampling for 512 sample
    int s64;  // apparent sampling for 64 samples   
};

#if F_CPU==72000000
const VeryLowScale veryLowScales[]=
{    
    {2000,32,256}, // For 100..500 pf
    {2000,16,128}, // For 100..500 pf
    {2000,8,64}, // For 100..500 pf
    {2000,2,16}, // For 100..500 pf
    {2000,2,8}, // For 100..500 pf
    
};
#elif
const VeryLowScale veryLowScales[]=
{
#error update
    {1000,32,256}, // For 200..100 pf
    {2000,32,256}, // For 100..500 pf
    {2000,16,128}, // For 100..500 pf
    {2000,2,16}, // For 100..500 pf    
};

#else
#error unsupport fq!
#endif
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
    
    if(!p1->pulseTimeDelta(*p2,clockPerSample, 64*2,fq,TestPin::PULL_HI,nbSamples,&samples,resistance))
    {
        return EVAL_ERROR;
    }
    
    
    TesterGfx::clear();
    TesterGfx::drawCurve(nbSamples,samples);
    
    WaveForm wave(nbSamples-2,samples+2);
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
        
    
    char st[20];
    sprintf(st,"iB:%d",iB);
    TesterGfx::print(10,20,st);
    
    sprintf(st,"mx:%d",vA);
    TesterGfx::print(10,80,st);

    
    Component::prettyPrint(fq,"Hz",st);
    TesterGfx::print(10,40,st);
    
    sprintf(st,"CPS:%d",clockPerSample);
    TesterGfx::print(10,60,st);
        
    
    
   
    vA=4095-vA;
    vB=4095-mx;

    
    
    float r=(float)iB/(float)nbSamples;
    
    sprintf(st,"r:%d",(int)(r*100));
    TesterGfx::print(10,100,st);
    
    TesterControl::waitForAnyEvent();
    
    if(r<0.09) return EVAL_SMALLER_CAP;
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
  
   int found=-1;
   int n=sizeof(veryLowScales)/sizeof(VeryLowScale);
   for(int i=0;i<n;i++)
   {
       if(quickEvalSmall(veryLowScales[i].signalFrequency,veryLowScales[i].s64)==EVAL_OK)
       {
           found=i;
           break;
       }
   }
   if(found<0)
       return false;
   
   er=evalSmall(veryLowScales[found].signalFrequency,veryLowScales[found].s512,cap);
   capacitance=cap;   
   return er==EVAL_OK;
}