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
    
    if(!_pA.fastSampleUp((4095*3)/4,value,timeUs)) return false;
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
    
    const TestPin::PULL_STRENGTH discharge[]=
    {
        TestPin::PULL_LOW,TestPin::PULL_MED,TestPin::PULL_HI
    };
    int n=sizeof(discharge)/sizeof(TestPin::PULL_STRENGTH);
    int candidate=-1;
#define CAP_MIN_CHARGE_TIME 30000 // 30 ms    
    for(int i=0;i<n&& candidate==-1;i++)
    {
         if(doOne(discharge[i],timeLow,resistanceLow,valueLow))
         {
             if(timeLow>CAP_MIN_CHARGE_TIME)
             {
                 candidate=i;
             }
         }
    }
    if(candidate==-1)
        return false;
    capacitance=computeCapacitance(timeLow,resistanceLow,valueLow);   
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
    
    int v,tus;
    _pA.fastSampleDown(threshold,v,tus);
    _pB.fastSampleDown(threshold,v,tus);
    
    xDelay(10);
    return true;
}

// EOF