/*
 * Resistance tester
*/

#include <SPI.h>
#include "Ucglib.h"
#include "fancyLock.h"
#include "testPins.h"
#include "Capacitor.h"
#include "math.h"
/**
 * 
 * @param yOffset
 * @return 
 */
bool Capacitor::draw(Ucglib *ucg,int yOffset)
{
#if 1
    char st[32];    
    Component::prettyPrint(capacitance, "F",st);
    ucg->drawString(10,30,0,st); 
#endif 
    return true;
}

/**
 * 
 * @return 
 */

bool Capacitor::doOne(TestPin::PULL_STRENGTH strength, int &timeUs, int &resistance,int &value)
{
    if(!zero(10)) return false;    
    // go
    _pB.setToGround();
    _pA.pullUp(strength);
    int start=micros();
    _pA.fastSampleUp((4095*3)/4,value);
    int end=micros();
    timeUs=end-start;    
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    return true;
}
/**
 * 
 * @param time
 * @param resistance
 * @param actualValue
 * @return 
 */
float Capacitor::computeCapacitance(int time, int iresistance, int actualValue)
{
   
    float t=(float)time/1000000.;        
    float resistance=iresistance;
    float den=1.-(float)(actualValue)/4095.;
    
    den=log(den);
    capacitance=-t/(resistance*den);
    return capacitance;
}
float cLow,cMed,cHi;
/**
 * 
 * @return 
 */
bool Capacitor::compute()
{
    int timeLow,resistanceLow;
    int timeMed,resistanceMed;
    int timeHi,resistanceHi;
    int valueLow,valueMed,valueHigh;
    
    
    
    doOne(TestPin::PULL_LOW,timeLow,resistanceLow,valueLow);
    doOne(TestPin::PULL_MED,timeMed,resistanceMed,valueMed);
    doOne(TestPin::PULL_HI,timeHi,resistanceHi,valueHigh);
    
    cLow=computeCapacitance(timeLow,resistanceLow,valueLow);
    cMed=computeCapacitance(timeMed,resistanceMed,valueMed);
    cHi=computeCapacitance(timeHi,resistanceHi,valueHigh);
    
    capacitance=cMed;
    return true;
}
/**
 * 
 * @return 
 */
bool Capacitor::zero(int threshold)
{
    _pA.pullDown(TestPin::PULL_LOW);
    _pB.pullDown(TestPin::PULL_LOW);
    
    int v;
    _pA.fastSampleDown(threshold,v);
    _pB.fastSampleDown(threshold,v);
    
    xDelay(10);
    return true;
}

// EOF