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
 */
bool Capacitor::computeHiCap()
{    
    int timeUs,resistance,value;
    int overSampling=2;
    int resTotal=0,timeTotal=0,valueTotal=0;
    
    // with large cap, only charge them up to 29% so that it does not exceed 1v
    // in case they are reversed polarised
    for(int i=0;i<overSampling;i++)
    {
#if 1 // full charge
        if(!Capacitor::doOneQuick(TestPin::PULL_LOW, false, 0.7,timeUs,resistance,value))
            return false;
#else         // or only up to 1v ?
        if(!Capacitor::doOneQuick(TestPin::PULL_LOW, false, 0.28,timeUs,resistance,value))
            return false;
#endif
        resTotal+=resistance;
        timeTotal+=timeUs;
        valueTotal+=value;
    }
    // correct B
    // Some voltage is dropped due to the parasitic resistor on the B pin
    // compensate for that
    float v=valueTotal;
    float alpha=(float)_pB.getRes(TestPin::GND)/(float)(_pA.getRes(TestPin::PULLUP_LOW));
    float coef=(v)*(alpha)-4095.*alpha*overSampling;
    v=v+coef;
    v=(v+(float)overSampling/2.)/(float)overSampling;
    valueTotal=v;
    capacitance=computeCapacitance(timeTotal,resTotal,valueTotal);
    return true;
}