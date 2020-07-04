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
    return computeCapRange(n,medCaps,4);
}
/**
 * 
 * @param n
 * @param scale
 * @param overSampling
 * @return 
 */
bool Capacitor::computeCapRange(int n, const Capacitor::CapScale *scale, int overSampling)
{    
    
    CapCurve curve;
    int deltaTime;
    for(int i=0;i<n;i++)
    {
        if(eval(scale[i],curve, deltaTime)==EVAL_OK)
        {
            // We have X sample total
            bool taken=false;
            if(deltaTime>(curve.nbSamples/3)) taken=true; // That one looks valid, we have enough point            
            if(i==(n-1) && deltaTime>=curve.nbSamples/4) taken=true; // if it is the last try we allow less point (~ 100)
            if(taken)
            {
                capacitance=computeCapacitance(curve);;
                for(int j=0;j<overSampling-1;j++)
                {
                    eval(scale[i],curve, deltaTime);
                    capacitance+=computeCapacitance(curve);
                }
                capacitance=capacitance/(float)overSampling;
                return true;
            }
        }
    }
    return false;
}

// EOF