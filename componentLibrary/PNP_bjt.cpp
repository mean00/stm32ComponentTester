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
#include "PNP_bjt.h"
//    PNPBjt( TestPin &Base, TestPin &Emitter,TestPin &Collector
#define pinBase         _pA
#define pinEmitter      _pB
#define pinCollector    _pC

/**
 * 
 * @param vf
 * @return 
 */
bool PNPBjt::computeVbe(float &vf)
{
    bool r=Component::computeDiode(pinBase,pinEmitter,vf);
    return r;
}

/**
 * 
 * @param vf
 * @return 
 */
bool PNPBjt::computeHfe(float &hfe)
{
    zeroAllPins();
    pinEmitter.setToGround();
    pinBase.pullUp(TestPin::PULL_HI);
    pinCollector.pullUp(TestPin::PULL_LOW);
    xDelay(50);
    float collectorRes=pinCollector.getCurrentRes();
    float baseRes=pinBase.getCurrentRes();
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
    float baseCurrent=(4095.-sumBase)/baseRes;
    // Compute CE current
    float ceCurrent=(4095.-sumCollector)/collectorRes;
    
    if(baseCurrent<1./(1000.*1000.*1000.)) // do not divide by zero
            return false;
    
    hfe=ceCurrent/baseCurrent;
    return true;
}

/**
 * 
 * @return 
 */
bool PNPBjt::compute()
{
    AutoDisconnect ad;
    // First compute Vbe
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
bool PNPBjt::draw(int yOffset)
{
    TesterGfx::drawPNP(beta,Vf);
    return true;
}
// EOF
