/*
 * NPN Bjt tester
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "P_Mosfet.h"
#include "math.h"
#include "cycleClock.h"
#include "MapleFreeRTOS1000_pp.h"
#include "NPN_bjt.h"
//    NPNBjt( TestPin &Base, TestPin &Emitter,TestPin &Collector
#define pinBase         _pA
#define pinEmitter      _pB
#define pinCollector    _pC

/**
 * 
 * @param vf
 * @return 
 */
bool NPNBjt::computeVbe(float &vf)
{
    return Component::computeDiode(pinBase,pinEmitter,vf);
}

/**
 * 
 * @param vf
 * @return 
 */
bool NPNBjt::computeHfe(float &hfe)
{
    zeroAllPins();
    pinEmitter.setToGround();
    pinBase.pullUp(TestPin::PULL_HI);
    pinCollector.pullUp(TestPin::PULL_LOW);
    xDelay(50);
    
    DeltaADC delta(pinBase,pinCollector);
    delta.setup(ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_6,512);
    int nbSamples;
    uint16_t *samples;
    float period;
    if(!pinBase.finishDmaSample(nbSamples,&samples)) 
    {
            return false;
    }    
    pinBase.pullDown(TestPin::PULL_LOW);
    pinCollector.pullDown(TestPin::PULL_LOW);
    
    int nb=nbSamples/2;
    float sumBase=0;
    float sumCollector=0;
    for(int i=0;i<nb;i++)
    {
        sumBase=sumBase+samples[nb*2+i*2];
        sumCollector=sumCollector+samples[nb*2+i*2+1];        
    }
    sumBase/=(float)nb;
    sumCollector/=(float)nb;
    
    // Compute base current
    float baseCurrent=(4095.-sumBase)/(float)pinBase.getRes(TestPin::PULLUP_HI);
    // Compute CE current
    float ceCurrent=(4095.-sumCollector)/((float)pinCollector.getRes(TestPin::PULLUP_MED)+(float)pinEmitter.getRes(TestPin::GND));
    
    hfe=ceCurrent/baseCurrent;
    return true;
}

/**
 * 
 * @return 
 */
bool NPNBjt::compute()
{
    AutoDisconnect ad;
    // First compite Vbe
    if(!computeVbe(Vf)) return false;
    // then hfe
    if(!computeHfe(beta)) return false;
    return true;
}
/**
 * 
 * @param yOffset
 * @return 
 */
bool NPNBjt::draw(int yOffset)
{
    return true;
}
// EOF
