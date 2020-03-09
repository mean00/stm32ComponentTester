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
    
    return Component::computeDiode(_pA,_pB,forward);
}
 
// EOF
