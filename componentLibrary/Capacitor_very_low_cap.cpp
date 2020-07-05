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

bool Capacitor::computeVeryLowCap()
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
        return false;      
    }
    p2->setToGround();
    p1->pullDown(TestPin::PULL_LOW);
    xDelay(10);
    //bool  TestPin::pulseTime(int nbSamples, int samplingFrequency, TestPin::PULL_STRENGTH strength,   int &nbSample,  uint16_t **xsamples,int &samplingTime,int &res)
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
    
    int fq=2000;
    if(!p1->pulseTime(1024,fq,TestPin::PULL_HI,nbSamples,&samples,samplingTime,resistance))
    {
        return false;
    }
    
    float period=F_CPU;
    period=(float)(samplingTime)/period;
    float c=computeCapacitance(  nbSamples,  samples, resistance,period);
    float mn=(float)p1->_calibration.capOffsetHighInPfMu16;
    mn/=(pPICO*16);
    if(c>mn)
        c-=mn;
    else
    {
        capacitance=0;
        return false;
    }
    capacitance=c;
    return true;
}