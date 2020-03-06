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
 
bool Mosfet::computeDiode(TestPin &top, TestPin &bottom,float &vf)
{
    top.pullDown(TestPin::PULL_LOW); // put it in reverse...
    bottom.pullDown(TestPin::PULL_LOW); // put it in reverse...
    xDelay(100);
     // Pull the gate to VCC so that it is blocked    
    top.pullUp(TestPin::PULL_LOW); // put it in reverse...
    bottom.setToGround();
    
    xDelay(10);
    
    DeltaADC delta(top,bottom);
    delta.setup(ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_6,512);
    int nbSamples;
    uint16_t *samples;
    float period;
    if(!delta.get(nbSamples, &samples,period))
    {
        return false;
    }
    bottom.pullDown(TestPin::PULL_LOW);
    float sum=0;
    for(int i=nbSamples/2;i<nbSamples;i++)
    {
        sum+=samples[i];
    }
    sum=sum/(float)(nbSamples/2);
    vf=adcToVolt(sum);
    
    xDelay(50);
    return true;
}


bool Mosfet::computeRdsOn(TestPin &top, TestPin &bottom,float &value)
{    
   
    xDelay(100);
    float R=top.getCurrentRes()+bottom.getCurrentRes();    
    DeltaADC delta(top,bottom);
    delta.setup(ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_6,128);
    int nbSamples;
    uint16_t *samples;
    float period;
    if(!delta.get(nbSamples, &samples,period))
    {
        return false;
    }    
    top.pullDown(TestPin::PULL_LOW);
    bottom.pullDown(TestPin::PULL_LOW);

    float sum=0;
    for(int i=nbSamples/2;i<nbSamples;i++)
    {
        sum+=samples[i];
    }
    sum=sum/(float)(nbSamples/2.);
    
    value= TestPin::resistanceDivider(sum,R);
    return true;
}


bool Mosfet::computeCg(TestPin &top, TestPin &bottom,float &value)
{    
    Capacitor cap(top,bottom,top);
    if(!cap.calibrationValue(value))
    {
        value=0;
        return false;
    }  
    return true;
}


/**
 * 
 * @return 
 */
bool Mosfet::compute()
{
  zeroAllPins();
    TesterGfx::printStatus("Mos Diode"); 
    // Compute reverse diode
    if(!computeDiode())
        return false;

   TesterGfx::printStatus("Mos Cap"); 
    if(!computeCg())
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
