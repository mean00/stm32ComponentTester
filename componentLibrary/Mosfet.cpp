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

static int clockDivier[]=
{
    12.5+1.5, //ADC_SMPR_1_5,               /**< 1.5 ADC cycles */
    12.5+7.5, //ADC_SMPR_7_5,               /**< 7.5 ADC cycles */
    12.5+13.5, //ADC_SMPR_13_5,              /**< 13.5 ADC cycles */
    12.5+28.8, //ADC_SMPR_28_5,              /**< 28.5 ADC cycles */
    12.5+41.5, //ADC_SMPR_41_5,              /**< 41.5 ADC cycles */
    12.5+55.5, //ADC_SMPR_55_5,              /**< 55.5 ADC cycles */
    12.5+71.5, //ADC_SMPR_71_5,              /**< 71.5 ADC cycles */
    12.5+239.5 //ADC_SMPR_239_5,             /**< 239.5 ADC cycles */
};
/**
 * 
 * @return 
 */
adc_smp_rate Mosfet::evaluateSampleRate()
{
    adc_smp_rate rate=ADC_SMPR_13_5;
    
    // we want the value to be ~ 300 samples
    // With a divider of 6
    float clock=72000000/6;
    float rc=(float)_pA.getRes(TestPin::PULLUP_HI)*this->_capacitance; // in seconds        
    // Rc is the time to charge the cap up to 70%
    // add 15 us for setup overhead
    rc+=15./1000000;
    // we want i/300 of that
    rc=rc/300.;
    int n=sizeof(clockDivier)/sizeof(int);
    for(int i=n-1;i>0;i--)
    {
        float curClock=clock/clockDivier[i];
        curClock=1./curClock; // in sec
        // RC=t
        if(curClock<=rc)
            return (adc_smp_rate)(i);
    }    
    return ADC_SMPR_1_5; // Default
    
}

// EOF
