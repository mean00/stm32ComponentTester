#include "Arduino.h"
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




const Capacitor::CapScale medCaps[]=
{
    {2*1000,   ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_MED,false}, //  5 uf
    {6*1000,   ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_MED,false}, //  2 uf
    {12*1000,  ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_MED,false}, //  1 uf
    {20*1000,  ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_MED,false}, //  500 nf
    {50*1000,  ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_MED,false}, //  200 nf
    {100*1000, ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4 ,TestPin::PULL_MED,false}, //   100 nf
};

bool Capacitor::computeMediumCap()
{    
    int n=sizeof(medCaps)/sizeof(Capacitor::CapScale);

    CapCurve curve;
    int deltaTime;
    for(int i=0;i<n;i++)
    {
        if(eval(medCaps[i],curve, deltaTime)==EVAL_OK)
        {
            if(deltaTime>245)
            {
                capacitance=computeCapacitance(curve);
                return true;
            }
        }
    }
    return false;
}

// EOF