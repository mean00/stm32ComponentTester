/*
 * NMosFet tester
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "N_Mosfet.h"
#include "math.h"
#include "cycleClock.h"
#include "MapleFreeRTOS1000_pp.h"
#include "Capacitor.h"
//

#define pinGate   _pA
#define pinDrain  _pB
#define pinSource _pC

/**
 * 
 * @param yOffset
 * @return 
 */
bool NMosFet::draw(int yOffset)
{    
     TesterGfx::drawNMosFet(_rdsOn,_capacitance,_vGsOn, _diodeVoltage,pinGate.pinNumber(),pinDrain.pinNumber(),pinSource.pinNumber());
    return true;
}
/**
 * 
 * Diode is in reverse : Anode is source, cathode is drain
 * 
 * @return 
 */
bool NMosFet::computeDiode()
{   
    // Pulldown everything
    pinGate.pullDown(TestPin::PULL_LOW);   
    pinSource.pullDown(TestPin::PULL_LOW); // put it in reverse...
    pinDrain.pullDown(TestPin::PULL_LOW); // put it in reverse...
    xDelay(100);
     // Pull the gate to VCC so that it is blocked    
    pinSource.pullUp(TestPin::PULL_LOW); // put it in reverse...
    pinDrain.setToGround();
    
    xDelay(10);
    
    DeltaADC delta(pinSource,pinDrain);
    delta.setup(ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_6,512);
    int nbSamples;
    uint16_t *samples;
    float period;
    if(!delta.get(nbSamples, &samples,period))
    {
        return false;
    }

    float sum=0;
    for(int i=nbSamples/2;i<nbSamples;i++)
    {
        sum+=samples[i];
    }
    sum=sum/(float)(nbSamples/2);
    this->_diodeVoltage=adcToVolt(sum);
    pinSource.pullDown(TestPin::PULL_LOW);
    xDelay(50);
    return true;
}
/**
 * 
 * @return 
 */
bool NMosFet::computeRdsOn()
{
    
     // Pull the gate to Ground so it is passing
    pinGate.pullUp(TestPin::PULL_LOW);    // make it passing current 
    pinDrain.pullUp(TestPin::PULL_LOW);   
    pinSource.setToGround();    
    
    xDelay(100);
    float R=pinDrain.getCurrentRes()+pinSource.getCurrentRes();    
    DeltaADC delta(pinDrain,pinSource);
    delta.setup(ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_6,64);
    int nbSamples;
    uint16_t *samples;
    float period;
    if(!delta.get(nbSamples, &samples,period))
    {
        return false;
    }

    float sum=0;
    for(int i=8;i<nbSamples;i++)
    {
        sum+=samples[i];
    }
    sum=sum/(float)(nbSamples-8);
    
    
    this->_rdsOn= TestPin::resistanceDivider(sum,R);
    return true;
}
/**
 * 
 * @return 
 */
bool NMosFet::computeVgOn()
{
    AutoDisconnect ad;
     // Pull the gate to Ground so it is passing
    pinGate.pullDown(TestPin::PULL_LOW);    
    
    pinDrain.pullUp(TestPin::PULL_LOW);   
    pinSource.setToGround();
        
    xDelay(100);
    int nbSamples;
    uint16_t *samples;
            
    pinGate.prepareDualDmaSample(pinDrain,ADC_SMPR_13_5,ADC_PRE_PCLK2_DIV_6,512);    
    // now charge the gate 
    pinGate.pullUp(TestPin::PULL_HI);
    if(!pinGate.finishDmaSample(nbSamples,&samples)) 
    {
            return false;
    }    
    pinGate.disconnect();
    pinDrain.disconnect();
     
    // search for blocked
    int blocked=-1;
    for(int i=0;i<50;i++)
        if(samples[i*2+1]>3900)
        {
            blocked=i;
            i=100;
        }
    if(blocked==-1)
        return false;
    
    for(int i=blocked;i<nbSamples;i++)
    {
        if(samples[2*i+1]<2000) // It's passing !
        {
            this->_vGsOn=adcToVolt(samples[2*i]);
            return true;            
        }
    }
    return false;
}


/**
 * 
 * @return 
 */
bool NMosFet::compute()
{
    zeroAllPins();
    TesterGfx::printStatus("Mos Diode"); 
    // Compute reverse diode
    if(!computeDiode())
        return false;

   TesterGfx::printStatus("Mos Cap"); 
    // First compute G-S capacitance, pin1/pin3
    Capacitor cap(pinGate,pinSource,pinDrain);
    if(!cap.calibrationValue(_capacitance))
    {
        _capacitance=0;
        return false;
    }

    // and RDS on    
    zeroAllPins();
    TesterGfx::printStatus("Mos Rds"); 
    if(!computeRdsOn())
        return false;
    zeroAllPins();


    TesterGfx::printStatus("Mos VgOn"); 
    if(!computeVgOn())
        return false;

    return true;
}
// EOF
