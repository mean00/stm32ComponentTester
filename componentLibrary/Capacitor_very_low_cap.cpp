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
};

const VeryLowScale veryLowScales[]=
{   
    {2000,32}, 
    {1000,64}, 
    {500,128},     
};


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
    er=quickEvalSmall(p1,p2,veryLowScales[0].signalFrequency,veryLowScales[0].s512*8,d);
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
    
    if(!p1->pulseTimeDelta(*p2,clockPerSample, 64*2,fq,TestPin::PULL_HI,nbSamples,&samples,resistance,true))
    {
        return EVAL_ERROR;
    }
    
    
    
    
    WaveForm wave(nbSamples-1,samples+1);
    int mn,mx;
    wave.searchMinMax(mn,mx);

#if 0
    TesterGfx::clear();
     char st[20];
    TesterGfx::drawCurve(nbSamples-2,samples+2);
    sprintf(st,"Mn:%d",mn);
    TesterGfx::print(10,100,st);
    sprintf(st,"Mx:%d",mx);
    TesterControl::waitForAnyEvent();

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
    if(!wave.searchValueAbove(mn+50, iA, vA, 0)) return EVAL_BIGGER_CAP;
    if(!wave.searchValueAbove(tgt, iB, vB, iA)) return EVAL_BIGGER_CAP;
    Logger("iA:%d vA:%d iB:%d vB:%d\n",iA,vA,iB,vB);
    if( (vB-vA)<2000) return EVAL_BIGGER_CAP;
    
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
    
    if(!p1->pulseTimeDelta(*p2,clockPerSample, 512*2,fq,TestPin::PULL_HI,nbSamples,&samples,resistance,true))
    {
        return EVAL_ERROR;
    }
    
    float period=F_CPU;
    period=(float)(clockPerSample)/period;
    
    
    
#define OFFSET 4    
    WaveForm wave(nbSamples-OFFSET,samples+OFFSET);
    int mn,mx;
    wave.searchMinMax(mn,mx);
#if 0    
    char st[20];
    TesterGfx::drawCurve(nbSamples,samples);
    sprintf(st,"Mn:%d",mn);
    TesterGfx::print(10,100,st);
    
    sprintf(st,"Mx:%d",mx);
    TesterControl::waitForAnyEvent();
#endif    
    
    if( (mx-mn)<100) // flat
    {
        if(mx<150) // stuck to zero
            return EVAL_BIGGER_CAP;      
    }
    // look for the starting minimum
    // Search start of ramp up above noise
    int iA,iB,vA,vB;
   
    wave.searchValueAbove(mn+(5*4095)/100, iA, vA, 0);
    wave.searchValueAbove(4095.*0.68, iB, vB, iA);
    
    if(vB<(4095/3)) return EVAL_BIGGER_CAP; // still charging...
    
    // If largeWindow is on, we dont require as much difference between min & max
    // it is for probing support
    // if largeWindow is off, we must have at leasst nbSample/8 samples, i.e. about 60
    int minSamples=0;

    minSamples=nbSamples/8;
    
    //if((iB-iA)<(minSamples)) return EVAL_SMALLER_CAP; // the pulse is too quick 
    if((vB-vA)<400) return EVAL_BIGGER_CAP; // A & B are too close, we must zoom out
    
    
    Capacitor::CapCurve curve;
    curve.iMax=iB;
    curve.iMin=iA;
    curve.resistance=resistance;
    curve.vMax=vB;
    curve.vMin=vA;
    curve.period=period;
    curve.nbSamples=nbSamples;
    //cap=Capacitor::computeCapacitance(iA,iB,vA,vB,resistance,period);
    cap=Capacitor::computeCapacitance(curve);
    
#if 0
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
#endif    
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
        Logger("Quick eval :%d/%d\n",i,n);
        er=quickEvalSmall(p1,p2,veryLowScales[i].signalFrequency,veryLowScales[i].s512*8,d);
        if(er==EVAL_OK)
        {
            bool takeIt=false;
            Logger("D=%d\n",d);
            if(d>=150) takeIt=true;
            if(i==0 && d>80) takeIt=true;
            if(takeIt )
            {
                found=i;
                Logger("Using scale %d, d=%d\n",found,d);
            }
        }
   }
   if(found<0)
       return false;
   {
    Logger("scale=%d",found);
    er=evalSmall(p1,p2,veryLowScales[found].signalFrequency,veryLowScales[found].s512,cap);

    if(er!=EVAL_OK)
    {
        capacitance=0;
        return false;
    }

    float cal=p2->_calibration.capOffsetHighInPfMu16[found];
    cal=(cal/16.)/pPICO;
    if(cap<cal+1./pPICO) // less than 1pF => noise
    {
        capacitance=0;
        return false;
    }
    Logger("Raw Cap=%d",(int)(cap*pPICO));
    capacitance=cap-cal;   
    Logger("Adj Cap=%d",(int)(capacitance*pPICO));
   }
   return true;
}

/**
 * 
 * @return 
 */
bool  Capacitor::calibrationVeryLow(int dex,TestPin &_pA, TestPin &_pB,int &calMul16)
{
    Capacitor::CapEval er;
  
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
   
    
    int n=sizeof(veryLowScales)/sizeof(VeryLowScale);
    if(dex>=n) 
    {
        calMul16=0;
        return true;
    }
    float cap=0;
    int   den=0;
    for(int j=0;j<8;j++)
    {
        float thisCap;

         er=Capacitor::evalSmall(   p1,p2,veryLowScales[dex].signalFrequency,veryLowScales[dex].s512,thisCap)    ;
         if(er==  EVAL_OK)
         {
             cap+=thisCap;
             den++;
         }else
         {
             Logger("Nope");
         }
    }
    if(den)
     cap/=(int)den;
    else
    {
      cap=0;
      Logger("No range match\n");
    }
    calMul16=(int)(16.*cap*pPICO);
    return true;
}
// EO