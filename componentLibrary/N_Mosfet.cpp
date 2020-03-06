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
    return Mosfet::computeDiode(pinSource,pinDrain,_diodeVoltage);

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
    bool r=Mosfet::computeRdsOn(pinDrain,pinSource,_rdsOn);    
    pinGate.pullDown(TestPin::PULL_LOW);
    return r;    
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
bool NMosFet::computeCg()
{
    return Mosfet::computeCg(pinGate, pinSource,_capacitance);
}
/**
 * 
 * @return 
 */
bool NMosFet::compute()
{
    return Mosfet::compute();
}
// EOF
