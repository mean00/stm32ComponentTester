/*
 * 
*/

#include <SPI.h>
#include "Ucglib.h"
#include "fancyLock.h"
#include "testPins.h"
#include "resistor.h"
static float checkResistor(TestPin &A, TestPin &B);
static float computeResistance(int adcValue, int resistance);

/**
 * 
 * @param yOffset
 * @return 
 */
bool Resistor::draw(Ucglib *ucg,int yOffset)
{
    char st[16];    
    sprintf(st,"%d",resistance);     
    ucg->drawString(10,30,0,st); 
    return true;
}
/**
 */
bool Resistor::compute()
{
    // 
    int adcMid, adcLow, adcHi;
    int rMid,rLow,rHi;
    
    probe(_pA,  TestPin::PULLUP_LOW,        _pB,TestPin::PULLDOWN_LOW,  adcLow,rLow);
    if(adcLow > 600 && adcLow<3500) // good !
    {
        resistance=computeResistance(adcLow,rLow);
        return true;
    }
    if(adcLow<600)
    {
        probe(_pA,  TestPin::PULLUP_LOW,        _pB,TestPin::GND,  adcLow,rLow);
        resistance=computeResistance(adcLow,rLow);
        return true;        
    }
    // ok, so it is too high, try with HIGH
    probe(_pA,  TestPin::PULLUP_HI,        _pB,TestPin::GND,  adcLow,rLow);
    if(adcLow > 600 && adcLow<3500) // good !
    {
        resistance=computeResistance(adcLow,rLow);
        return true;
    }
     if(adcLow<600)
    {
        probe(_pA,  TestPin::PULLUP_INTERNAL,        _pB,TestPin::GND,  adcLow,rLow);
        resistance=computeResistance(adcLow,rLow);
        return true;        
    }
    probe(_pA,  TestPin::PULLUP_HI,        _pB,TestPin::PULLUP_HI,  adcLow,rLow);
    resistance=computeResistance(adcLow,rLow);
    return !!resistance;
}

/**
 * 
 * @param A
 * @param stateA
 * @param B
 * @param stateB
 * @return 
 */
bool Resistor::probe( TestPin &A,TestPin::TESTPIN_STATE stateA, TestPin &B,TestPin::TESTPIN_STATE stateB,int &adc, int &resistance)
{
      AutoDisconnect ad;
      A.setMode(stateA);
      B.setMode(stateB);
      int hiAdc, loAdc;
      float hiVolt,loVolt;      
      A.sample(hiAdc,hiVolt);
      B.sample(loAdc,loVolt);
      
      int n=hiAdc-loAdc;
      resistance=A.getCurrentRes()+B.getCurrentRes();
      adc=n;
      return true;
}

/**
 * 
 * @param adcValue
 * @param resistance
 * @return 
 */     
float computeResistance(int adcValue, int resistance)
{
      float a=adcValue;    
      if(a>4093) return 0;
      float r=(float)(resistance)*a/((4095.-a));
      return r;
}
// EOF