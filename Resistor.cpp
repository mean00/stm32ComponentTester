/*
 * 
*/

#include <SPI.h>
#include "Ucglib.h"
#include "fancyLock.h"
#include "testPins.h"
#include "resistor.h"
static float checkResistor(TestPin &A, TestPin &B);


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
    resistance=checkResistor(_pA,_pB);
    return !!resistance;
}
/**
 * 
 * @param A
 * @param B
 * @return 
 */
float checkResistor(TestPin &A, TestPin &B)
{
    AutoDisconnect ad;
    // set pinA To ground
    A.setToGround();
    // and pinB to VCC via High Res
    B.pullUp(true);
    delay(5);
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
        a=adcValue;    
        r=(float)(B.getHiRes())*a/(4095.-a);
        return r;
    }
    // If the ADC value is too low, try pull up with the low resistor
    B.pullUp(false);
    B.sample(adcValue, volt);
    delay(5);
    if(adcValue>4090)
    {
        return 0; // Cannot measure
    }    
    a=adcValue;    
    r=(float)(B.getLowRes())*a/(4095.-a);
    return r;
}

// EOF