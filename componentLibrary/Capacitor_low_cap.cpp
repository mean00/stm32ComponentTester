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


const Capacitor::CapScale lowCaps[]=
{
    {8*1000,   ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_HI,false}, //   50 nf
    {36*1000,  ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_HI,false}, //  10 nf
    {60*1000,  ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_HI,false}, //  5 nf
    {360*1000, ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_HI,false}, //  1 nf
    {250*1000, ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_HI,true}, //  660 pf nf
    {360*1000, ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_HI,true}, //  500 pf nf
    {500*1000, ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_HI,true}, //  330 pf nf
};

bool Capacitor::computeLowCap()
{    
    int n=sizeof(lowCaps)/sizeof(Capacitor::CapScale);

    CapCurve curve;
    int deltaTime;
    for(int i=0;i<n;i++)
    {
        if(eval(lowCaps[i],curve, deltaTime)==EVAL_OK)
        {
            if(deltaTime>200)
            {
                capacitance=computeCapacitance(curve);
                return true;
            }
        }
    }
    return false;
}
//EOF