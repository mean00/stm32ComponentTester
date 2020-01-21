/*
 * Resistance tester
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "Diode.h"

/**
 * 
 * @param yOffset
 * @return 
 */
bool Diode::draw(int yOffset)
{
    char st[32];        
    Component::prettyPrint(forward, "V",st);
    TesterGfx::drawDiode(yOffset, st,_pA.pinNumber(), _pB.pinNumber());
    return true;
}

/**
 * 
 * @return 
 */
bool Diode::compute()
{
    
    // even with the lowest resistance we are at max at 
    // 3.3v/470 Ohm= 7 mA, which is fine
    _pA.pullUp(TestPin::PULL_LOW);
    _pB.setToGround();
    xDelay(5);
    

    int adcA,nbA;
    int adcB,nbB;
    
    //for(int i=0;i<5;i++)
    {
      _pA.slowDmaSample(adcA,nbA);
      _pB.slowDmaSample(adcB,nbB);
    }
    float vf=(float)(adcA-adcB);
    vf/=(float)nbA;
    forward=adcToVolt(vf);
    return true;
}
 
// EOF
