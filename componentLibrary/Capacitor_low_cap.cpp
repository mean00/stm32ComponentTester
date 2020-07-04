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
    {8*1000,  TestPin::PULL_HI,false}, //  50 nf
    {16*1000, TestPin::PULL_HI,false}, //  20 nf
    {36*1000, TestPin::PULL_HI,false}, //  10 nf
    {60*1000, TestPin::PULL_HI,false}, //  5 nf
    {100*1000,TestPin::PULL_HI,false}, //  2 nf
    {360*1000,TestPin::PULL_HI,false}, //  1 nf
    {250*1000,TestPin::PULL_HI,true}, //  660 pf nf
    {360*1000,TestPin::PULL_HI,true}, //  500 pf nf
    {500*1000,TestPin::PULL_HI,true}, //  330 pf nf
    //{900*1000, ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_2 ,TestPin::PULL_HI,true}, //  200 pf nf
};

bool Capacitor::computeLowCap()
{    
    int n=sizeof(lowCaps)/sizeof(Capacitor::CapScale);
    bool r= computeCapRange(n,lowCaps,8);
    if(r)
    { // take parasitic cap into consideration
        float offset=(float)_pA._calibration.capOffsetInPf/pPICO;
        if(capacitance<offset)
            return false;
        capacitance-=offset;
        return true;
        
    }
    return false;
}
//EOF