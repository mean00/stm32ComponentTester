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

    {1000,128,128*8}, // For 100..500 pf
    {1400,64,64*8}, // For 100..500 pf
    {1600,48,48*8},
    {2000,32,32*8}, // For 100..500 pf
    {2000,24,24*8}, // For 100..500 pf
    {2000,16,16*8}, // For 100..500 pf
    {2000,12,12*8}, // For 100..500 pf
    {2000,8,8*8}, // For 100..500 pf
    

//    {20000,1,32}, // For 100..500 p
    
};
#elif F_CPU==96000000
const VeryLowScale veryLowScales[]=
{   

    {1000,128,128*8}, // For 100..500 pf
    {1400,64,64*8}, // For 100..500 pf
    {1600,48,48*8},
    {2000,32,32*8}, // For 100..500 pf
    {2000,24,24*8}, // For 100..500 pf
    {2000,16,16*8}, // For 100..500 pf
    {2000,12,12*8}, // For 100..500 pf
    {2000,8,8*8}, // For 100..500 pf
    

//    {20000,1,32}, // For 100..500 p
    
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
Capacitor::CapEval Capacitor::quickEvalSmall(TestPin *p1, TestPin *p2,int fq, int clockPerSample, int &d)
{   
  
   
    p2->setToGround();
    p1->pullDown(TestPin::PULL_LOW);
    xDelay(10);
    
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
    
    if(!p1->pulseTimeDelta(*p2,clockPerSample, 64*2,fq,TestPin::PULL_HI,nbSamples,&samples,resistance,true))
    {
        return EVAL_ERROR;
    }
    
    
    
    
    WaveForm wave(nbSamples-2,samples+2);
    int mn,mx;
    wave.searchMinMax(mn,mx);

#if 0
    TesterGfx::clear();
     char st[20];
    TesterGfx::drawCurve(nbSamples,samples);
    sprintf(st,"Mn:%d",mn);
    TesterGfx::print(10,100,st);
    sprintf(st,"Mx:%d",mx);

#endif    
    if( (mx-mn)<100) // flat
    {
        if(mx<150) // stuck to zero
            return EVAL_BIGGER_CAP;
        else
            return EVAL_SMALLER_CAP;
    }
    
    int iA,iB,vA,vB;    
    int tgt=mn+(((mx-mn)*85)/100); // look for 0.666= ~ e-1
    wave.searchValueAbove(mn+50, iA, vA, 0);
    wave.searchValueAbove(tgt, iB, vB, iA);
    
    d=(iB-iA)*8;
#if 0    
    sprintf(st,"iB:%d",iB);
    TesterGfx::print(10,20,st);
    
    sprintf(st,"d:%d",d);
    TesterGfx::print(10,80,st);

    
    Component::prettyPrint(fq,"Hz",st);
    TesterGfx::print(10,40,st);
    
    sprintf(st,"CPS:%d",clockPerSample);
    TesterGfx::print(10,60,st);
        
   
    TesterGfx::print(90,100,st);
    
    TesterControl::waitForAnyEvent();
#endif    
    return EVAL_OK;
}

// EO