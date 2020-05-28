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
bool NMosFet::draw2(int yOffset)
{    
    TesterGfx::drawNMosFet(pinGate.pinNumber(),pinDrain.pinNumber(),pinSource.pinNumber());
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

/**
 * 
 * @return 
 */
bool NMosFet::computeVgOn()
{
    
    Debug("NMosfet VgOn");
    AutoDisconnect ad;
     // Pull the gate to Ground so it is passing
    pinGate.pullDown(TestPin::PULL_LOW);    
    
    pinDrain.pullUp(TestPin::PULL_LOW);   
    pinSource.setToGround();
        
    xDelay(100);
    int nbSamples;
    uint16_t *samples;
//
    adc_smp_rate sampleRate=evaluateSampleRate();
        
    pinGate.prepareDualDmaSample(pinDrain,sampleRate, DSOADC::ADC_PRESCALER_6,1024);    
    // now charge the gate 
    pinGate.pullUp(TestPin::PULL_HI);
    if(!pinGate.finishDmaSample(nbSamples,&samples)) 
    {
        Debug("Dma failed");
        return false;
    }    
    pinGate.disconnect();
    pinDrain.disconnect();
     
    // search for blocked
    int blocked=-1;
    for(int i=1;i<50;i++)
        if(samples[i*2+1]>3900)
        {
            blocked=i;
            i=100;
        }
    if(blocked==-1)
        return false;
    
    int nbPair=nbSamples/2;
    for(int i=blocked;i<nbPair;i++)
    {
        if(samples[2*i+1]<2000) // It's passing !
        {
            this->_vGsOn=adcToVolt(samples[2*i]);
            Debug("NMosfet VgOn OK");            
            return true;            
        }
    }
    Debug("No passing \n");
    Debug(nbPair);
    Debug("-- last sample -- \n");
    Debug(samples[nbPair*2]);
    return false;
}
// EOF
