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
    resistance=twoPinsResistor(TestPin::PULL_INTERNAL,_pA,_pB);
    return !!resistance;
}


float Resistor::twoPinsResistor(TestPin::PULL_STRENGTH strength, TestPin &A, TestPin &B)
{
      AutoDisconnect ad;
      A.pullUp(strength);
      B.pullDown(strength);
      xDelay(5);
      int hiAdc, loAdc;
      float hiVolt,loVolt;      
      A.sample(hiAdc,hiVolt);
      B.sample(loAdc,loVolt);
      
      int n=hiAdc-loAdc;
      if(n<5) return 0.; // cant read
      
      int r=A.getCurrentRes()+B.getCurrentRes();
      float result= computeResistance(n,r);
      return result;
}
     
#if 0    
/**
 * 
 * @param A
 * @param B
 * @return 
 */
float checkResistor(TestPin &A, TestPin &B)
{
    return twoPinsResistor(false,A,B);

    AutoDisconnect ad;
    // set pinA To ground
    A.setToGround();
    // and pinB to VCC via High Res
    B.pullUp(true);
    xDelay(5);
    int adcValue;
    float volt;
    float a,r;
    
    B.sample(adcValue, volt);
    if(adcValue>4090)
    {
        return 0; // Cannot measure
    }
    if(adcValue>2*100)
    {
        return computeResistance(adcValue,B.getHiRes());
    }
    // If the ADC value is too low, try pull up with the low resistor
    B.pullUp(false);
    B.sample(adcValue, volt);
    xDelay(5);
    if(adcValue>4090)
    {
        return 0; // Cannot measure
    }    
    return computeResistance(adcValue,B.getLowRes());

}
#endif    
float computeResistance(int adcValue, int resistance)
{
      float a=adcValue;    
      if(a>4093) return 0;
      float r=(float)(resistance)*a/((4095.-a));
      return r;
}
// EOF