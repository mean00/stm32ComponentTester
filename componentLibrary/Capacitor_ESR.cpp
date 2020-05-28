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
//
#define pPICO (1000.*1000.*1000.*1000.)
/**
 */
bool Capacitor::minMax(bool high,int &minmax)
{
    int resistance;
   
    _pB.setToGround();

    // start the DMA
    // max duration ~ 512 us
    uint16_t *samples;
    int nbSamples;
    TestPin *samplePin=&_pB;
    if(high)
    {
        samplePin=&_pA;
    }
    if(!samplePin->prepareDmaSample( ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_6 ,512))
        return false;        
    // Go!
    _pA.pullUp(TestPin::PULL_LOW);   
    if(!samplePin->finishDmaSample(nbSamples,&samples)) 
    {
        return false;
    }
   
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    _pA.pullDown(TestPin::PULL_LOW);   
    
    if(high) // look for the minimum
    {
        for(int i=0;i<nbSamples-1;i++)
        {
            if(samples[i+1]<samples[i]) 
            {
                minmax=samples[i+1];
                return true;
            }
        }
        return false;
    }
    // Search maximum
    int mx=0;
    int dex=-1;
     for(int i=0;i<nbSamples;i++)
        {
            if(samples[i]>mx) 
            {
                mx=samples[i];
                dex=i;
            }
        }
    if(dex==-1) return false;
    minmax=mx;
    return true;
}
/**
 * 
 * @param yOffset
 * @return 
 */
bool Capacitor::getEsr(float &esr)
{
    int high,low;
    if(!minMax(false,low)) return false;
    if(!minMax(true,high)) return false;
    
    float delta=high-low;
    if(delta<=0) return false;
    esr=delta;
    
    return true;
}



// EOF
