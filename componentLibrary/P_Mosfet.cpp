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
bool PMosFet::draw2(int yOffset)
{    
     TesterGfx::drawPMosFet(pinGate.pinNumber(),pinUp.pinNumber(),pinDown.pinNumber());
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
    pinDown.pullDown(TestPin::PULL_LOW);    
    pinUp.setToVcc();           
    bool r=Mosfet::computeRdsOn(pinUp,pinDown,_rdsOn);    
    pinGate.pullDown(TestPin::PULL_LOW);
    return r;
}


/**
 * 
 * @return 
 */
bool PMosFet::computeCg()
{
    return Mosfet::computeCg(pinGate, pinDown,_capacitance);
}

/**
 * 
 * @return 
 */
bool PMosFet::compute()
{
    return Mosfet::compute();
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
    
    adc_smp_rate sampleRate=evaluateSampleRate();
    
    pinGate.prepareDualDmaSample(pinDown,sampleRate,ADC_PRE_PCLK2_DIV_6,512);    
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

// EOF
