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
/**
 * 
 * @param count
 */
void  Mosfet::changePage(int count)
{
  _curPage+=count;
  _curPage%=_maxPage;
  TesterGfx::drawMosInfo(_curPage,_rdsOn,_capacitance,_vGsOn, _diodeVoltage);    
}

/**
 * 
 * @param yOffset
 * @return 
 */
bool Mosfet::draw(int yOffset)
{    
    draw2(yOffset);    
    TesterGfx::drawMosInfo(0,_rdsOn,_capacitance,_vGsOn, _diodeVoltage);    
    return true;
}
 
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
    delta.setup(ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_6 ,512);
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
    delta.setup(ADC_SMPR_239_5,DSOADC::ADC_PRESCALER_6,128);
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
    if(!cap.compute1nfRange(value))
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

/**
 * \brief return the sampling frequency
 * @return 
 */
int Mosfet::evaluateSampleRate()
{
    
    
    // we want the value to be ~ 300 samples
    // With a divider of 6
    
    float rc=(float)_pA.getRes(TestPin::PULLUP_HI)*this->_capacitance; // in seconds        
    // Rc is the time to charge the cap up to 70%
    // if we take 2*Rc, the cap is charged up to 86 % (1-exp(-2)) =~ 2.8 V
    // and we add 15 us , time to setup the charge
    rc=2*rc+15./1000000;
    // we want i/300 of that
    rc=rc/300.;
    
    // that gives us the frequency or about
    // it will be round up when the actual timer is programmed
    xAssert(rc!=0);
    rc=1./rc;
    return (int)rc;
}

// EOF
