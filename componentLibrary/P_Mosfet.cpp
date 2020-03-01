/*
 * PMosFet tester
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "P_Mosfet.h"
#include "math.h"
#include "cycleClock.h"
#include "MapleFreeRTOS1000_pp.h"
#include "Capacitor.h"
//

/**
 * 
 * @param yOffset
 * @return 
 */
bool PMosFet::draw(int yOffset)
{
    
    char st[32];
    char st2[32];
    char st3[32]="Rds=";
    
   
    Component::prettyPrint(_rdsOn, "O Vf=",st3+4);
    Component::prettyPrint(_diodeVoltage, "V ",st2);
    strcat(st3,st2);
    Component::prettyPrint(_vGsOn, "V Cg=",st);
    Component::prettyPrint(_capacitance, "F ",st2);
    strcat(st,st2);
    TesterGfx::drawPMosFet(st3,st,_pA.pinNumber(),_pB.pinNumber(),_pC.pinNumber());
    return true;
}
/**
 * 
 * @return 
 */
bool PMosFet::computeDiode()
{
     AutoDisconnect ad;
     // Pull the gate to VCC so that it is blocked
    _pA.pullUp(TestPin::PULL_LOW);
    xDelay(100); // should be enough     
    _pC.pullUp(TestPin::PULL_LOW);
    _pB.pullDown(TestPin::PULL_LOW);   
    
    xDelay(5);
    

    int adcA,nbA;
    int adcB,nbB;
    
    //for(int i=0;i<5;i++)
    {
      _pC.slowDmaSample(adcA,nbA);
      _pB.slowDmaSample(adcB,nbB);
    }
    float vf=(float)(adcA-adcB);
    vf/=(float)nbA;
    this->_diodeVoltage=adcToVolt(vf);
    return true;
}
/**
 * 
 * @return 
 */
bool PMosFet::computeRdsOn()
{
     AutoDisconnect ad;
     // Pull the gate to Ground so it is passing
    _pA.pullDown(TestPin::PULL_LOW);    
    _pC.setToGround();
    _pB.pullUp(TestPin::PULL_LOW);   
    
    xDelay(100);
    

    int adcA,nbA;
    int adcB,nbB;
    
    //for(int i=0;i<5;i++)
    {
      _pB.slowDmaSample(adcA,nbA);
      _pC.slowDmaSample(adcB,nbB);
    }
    float vf=(float)(adcA-adcB);
    vf/=(float)nbA;
    
    float R=_pB.getCurrentRes()+_pC.getCurrentRes();    
    this->_rdsOn= TestPin::resistanceDivider(vf,R);
    return true;
}

/**
 * 
 * @return 
 */
bool PMosFet::compute()
{
    AutoDisconnect ad;
    
    // First compute G-S capacitance, pin1/pin3
    Capacitor cap(_pA,_pC,_pB);
    if(!cap.calibrationValue(_capacitance))
    {
        _capacitance=0;
        return false;
    }
    // Compute reverse diode
    if(!computeDiode())
        return false;
    // and RDS on
    if(!computeRdsOn())
        return false;
    return true;
}
// EOF
