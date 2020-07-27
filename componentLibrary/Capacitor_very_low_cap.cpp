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

    {1600,48,48*8}, // For 100..500 pf
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


Capacitor::CapEval Capacitor::quickProbe()
{
   Capacitor::CapEval er;
   float cap;
   int d;
  
    TestPin *p1,*p2;
    if(_pB.pinNumber()==2)
    {
        p1=&_pB;
        p2=&_pA;
    }else if(_pA.pinNumber()==2)
    {
        p1=&_pA;
        p2=&_pB;        
    } else
    {
        return EVAL_ERROR;
    }
    er=quickEvalSmall(p1,p2,veryLowScales[0].signalFrequency,veryLowScales[0].s64,d);
    switch(er)
    {
        case   EVAL_OK:
        case   EVAL_BIGGER_CAP: return EVAL_OK;break;
        default: return EVAL_SMALLER_CAP;break;        
    }
    return EVAL_ERROR;
      
}
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
    
    if(!p1->pulseTimeDelta(*p2,clockPerSample, 64*2,fq,TestPin::PULL_HI,nbSamples,&samples,resistance))
    {
        return EVAL_ERROR;
    }
    
    
    
    
    WaveForm wave(nbSamples-2,samples+2);
    int mn,mx;
    wave.searchMinMax(mn,mx);


    TesterGfx::clear();
     char st[20];
    TesterGfx::drawCurve(nbSamples,samples);
    sprintf(st,"Mn:%d",mn);
    TesterGfx::print(10,100,st);
    sprintf(st,"Mx:%d",mx);

    
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
    
    return EVAL_OK;
}

/**
 * 
 * @return 
 */
Capacitor::CapEval Capacitor::evalSmall(  TestPin *p1,TestPin *p2,int fq, int clockPerSample, float &cap)
{   
       
    p2->setToGround();
    p1->pullDown(TestPin::PULL_LOW);
    xDelay(10);
    
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
    
    if(!p1->pulseTimeDelta(*p2,clockPerSample, 512*2,fq,TestPin::PULL_HI,nbSamples,&samples,resistance))
    {
        return EVAL_ERROR;
    }
    
    float period=F_CPU;
    period=(float)(clockPerSample)/period;
    
    
    
#define OFFSET 4    
    WaveForm wave(nbSamples-OFFSET,samples+OFFSET);
    int mn,mx;
    wave.searchMinMax(mn,mx);
    
    char st[20];
    TesterGfx::drawCurve(nbSamples,samples);
    sprintf(st,"Mn:%d",mn);
    TesterGfx::print(10,100,st);
    
    sprintf(st,"Mx:%d",mx);
    
    
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
    int tgt=(mx*80)/100; // look for 0.666= ~ e-1
    wave.searchValueAbove(mn+20, iA, vA, OFFSET);
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
    
    
    
    
    Component::prettyPrint(cap,"F",st);
    TesterGfx::print(10,20,st);
    
    Component::prettyPrint(fq,"Hz",st);
    TesterGfx::print(10,50,st);
    
    sprintf(st,"%d",clockPerSample);
    TesterGfx::print(10,80,st);
    

    sprintf(st,"A%d",iA);
    TesterGfx::print(80,80,st);

    sprintf(st,"B%d",iB);
    TesterGfx::print(80,100,st);
    
    
    
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
   int found=-1;
   int apparent;
   int d;
   int n=sizeof(veryLowScales)/sizeof(VeryLowScale);
   
   // Search for good distance between 150 & 200
    for(int i=0;i<n && found==-1;i++)
   {
        er=quickEvalSmall(p1,p2,veryLowScales[i].signalFrequency,veryLowScales[i].s64,d);
        if(er==EVAL_OK)
        {
            if(d>=150)
            {
                found=i;
            }
        }
   }
   if(found<0)
       return false;
   
   er=evalSmall(p1,p2,veryLowScales[found].signalFrequency,veryLowScales[found].s512,cap);
   
   capacitance=cap;   
   return er==EVAL_OK;
}
/**
 * 
 * @return 
 */
bool Capacitor::calibration()
{
#if 0
    Capacitor::CapEval er;
    float cap;
  
    TestPin *p1,*p2;
    if(_pB.pinNumber()==2)
    {
        p1=&_pB;
        p2=&_pA;
    }else if(_pA.pinNumber()==2)
            {
                p1=&_pA;
                p2=&_pB;        
            }else
            {
                xAssert(0);
            }
   
    calibrationLow(_pA,_pB);
    
    int n=sizeof(veryLowScales)/sizeof(VeryLowScale);
    for(int i=0;i<n;i++)
    {
       er=Capacitor::evalSmall(   p1,p2,veryLowScales[i].signalFrequency,veryLowScales[i].s512,cap)    ;
        if(er==  EVAL_OK)
        {
            char st[20];
            TesterGfx::clear();
            TesterGfx::print(10,60,"Calibration");
            Component::prettyPrint(cap,"F",st);
            TesterGfx::print(10,90,st);
            sprintf(st,"I=%d",i);
            TesterGfx::print(10,20,st);
            TesterControl::waitForAnyEvent();
        }
    }
#endif
    return false;
}
