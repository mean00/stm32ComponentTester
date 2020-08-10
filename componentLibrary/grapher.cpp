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
/*
 * Capacitor tester
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "Capacitor.h"
#include "math.h"
#include "MapleFreeRTOS1000_pp.h"
//
#define ACC_SIZE 512
uint16_t accumlulator[ACC_SIZE];

/**
 * 
 * @return 
 */
void grapher( TestPin *p1,TestPin *p2,int fq, int clockPerSample,TestPin::PULL_STRENGTH strength)
{   
       
    p2->setToGround();
    p1->pullDown(TestPin::PULL_LOW);
    xDelay(10);
    
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
#define OVERSAMPLE 2    
    memset(accumlulator,0,ACC_SIZE*2);
    for(int i=0;i<OVERSAMPLE;i++)
    {
        if(!p1->pulseTimeDelta(*p2,clockPerSample, 512*2,fq,strength,nbSamples,&samples,resistance,true))
        {
            return ;
        }
        for(int i=0;i<512;i++)
        {
            accumlulator[i]+=samples[i];
        }
    }
   for(int i=0;i<512;i++)
        {
            accumlulator[i]=(accumlulator[i]+3)/OVERSAMPLE;
        }

    
    
#define OFFSET 4    
    WaveForm wave(512-OFFSET,accumlulator+OFFSET);
    TesterGfx::drawCurve(512,accumlulator);
    char st[50];
    for(int i=OFFSET;i<512;i++)
    {
        sprintf(st,"%d;%d",i,samples[i]);
        Logger(st);
    }

    int mn,mx;
    wave.searchMinMax(mn,mx);    
    // Search start of ramp up above noise
    int iA,iB,vA,vB;
    wave.searchValueAbove(mn+4095*15/100, iA, vA, 0);
    wave.searchValueAbove(4095.*0.68, iB, vB, iA);
    
    if(vB<(4095/3)) return;
    
    float period=1./(float)F_CPU;
    period*=(float)clockPerSample;
    
    
    Capacitor::CapCurve curve;
    curve.iMax=iB;
    curve.iMin=iA;
    curve.resistance=resistance;
    curve.vMax=vB;
    curve.vMin=vA;
    curve.period=period;
    curve.nbSamples=nbSamples;
    float c=Capacitor::computeCapacitance(curve);
    int capInPico=(int)(c*1000000000000.);
    Logger("Cap=%d Farad\n",capInPico);
    return ;    
    
    
    TesterControl::waitForAnyEvent();
}

void grapher( TestPin *p1,TestPin *p2)
{
    TestPin::PULL_STRENGTH st=TestPin::PULL_HI;    
    //grapher(p1,p2,2000,64,st);//64
    grapher(p1,p2,500,64,st);//64
    // Cap
    return;
    
    for(int scale=64;scale<=64;scale*=2)
    {
        grapher(p1,p2,2000,scale,st);//64
    }
    
}