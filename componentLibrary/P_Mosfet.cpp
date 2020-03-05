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

#define pinGate   _pA
#define pinDown   _pB  // DOWN
#define pinUp     _pC  //  UP

/**
 * 
 * @param yOffset
 * @return 
 */
bool PMosFet::draw(int yOffset)
{    
     TesterGfx::drawPMosFet(_rdsOn,_capacitance,_vGsOn, _diodeVoltage,pinGate.pinNumber(),pinUp.pinNumber(),pinDown.pinNumber());
    return true;
}
/**
 * 
 * Diode is in reverse : Anode is source, cathode is drain
 * 
 * @return 
 */
bool PMosFet::computeDiode()
{   
    // Pulldown everything
    pinGate.pullDown(TestPin::PULL_LOW);           
    return Mosfet::computeDiode(pinDown,pinUp,_diodeVoltage)   ;
}
/**
 * 
 * @return 
 */
bool PMosFet::computeRdsOn()
{
    
     // Pull the gate to Ground so it is passing
    pinGate.pullDown(TestPin::PULL_LOW);    // make it passing current 
    
    pinUp.setToVcc();   
    pinDown.pullDown(TestPin::PULL_LOW);    
    
    xDelay(100);
    float R=pinUp.getCurrentRes()+pinDown.getCurrentRes();    
    DeltaADC delta(pinUp,pinDown);
    delta.setup(ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_6,128);
    int nbSamples;
    uint16_t *samples;
    float period;
    if(!delta.get(nbSamples, &samples,period))
    {
        return false;
    }
    pinGate.pullDown(TestPin::PULL_LOW);
    pinUp.pullDown(TestPin::PULL_LOW);
    pinDown.pullDown(TestPin::PULL_LOW);

    float sum=0;
    for(int i=nbSamples/2;i<nbSamples;i++)
    {
        sum+=samples[i];
    }
    sum=sum/(float)(nbSamples/2.);
    
    this->_rdsOn= TestPin::resistanceDivider(sum,R);
    return true;
}
/**
 * 
 * @return 
 */
bool PMosFet::computeVgOn()
{
    AutoDisconnect ad;
     // Pull the gate to Ground so it is not passing
    pinGate.pullUp(TestPin::PULL_LOW);    
    
    pinUp.setToVcc();
    pinDown.pullDown(TestPin::PULL_LOW);
        
    xDelay(100);
    int nbSamples;
    uint16_t *samples;
    
    pinGate.prepareDualDmaSample(pinDown,ADC_SMPR_13_5,ADC_PRE_PCLK2_DIV_6,512);    
    // now charge the gate 
    pinGate.pullDown(TestPin::PULL_HI);
    if(!pinGate.finishDmaSample(nbSamples,&samples)) 
    {
            return false;
    }    
    pinGate.disconnect();
    pinUp.disconnect();
     
    // search for blocked
    int blocked=-1;
    for(int i=0;i<50;i++)
        if(samples[i*2+1]<100)
        {
            blocked=i;
            i=100;
        }
    if(blocked==-1)
        return false;
    
    for(int i=blocked;i<nbSamples;i++)
    {
        if(samples[2*i+1]>1000) // It's passing !
        {
            this->_vGsOn=adcToVolt(4095-samples[2*i]);
            return true;            
        }
    }
    return false;
}


/**
 * 
 * @return 
 */
bool PMosFet::compute()
{
    zeroAllPins();
    TesterGfx::printStatus("Mos Diode"); 
    // Compute reverse diode
    if(!computeDiode())
        return false;

   TesterGfx::printStatus("Mos Cap"); 
    // First compute G-S capacitance, pin1/pin3
    Capacitor cap(pinGate,pinDown,pinUp);
    if(!cap.calibrationValue(_capacitance))
    {
        _capacitance=0;
        return false;
    }

    // and RDS on    
    zeroAllPins();
    TesterGfx::printStatus("Mos Rds"); 
    if(!computeRdsOn())
    {
     //   return false;
    }
    zeroAllPins();

    TesterGfx::printStatus("Mos VgOn"); 
    if(!computeVgOn())
    {
     //   return false;
    }
    return true;
}
// EOF
