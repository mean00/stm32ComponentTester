#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "deltaADC.h"
#include "myPwm.h"
/**
 * 
 * @param rate
 * @param scale
 * @param nbSamples
 * @return 
 */
bool DeltaADC::setup(const adc_smp_rate rate,  const  DSOADC::Prescaler  scale, const int nbSamples)
{
    _rate=rate;
    _scale=scale;
    return _pA.prepareDualDmaSample(_pB,_rate,_scale,nbSamples);    
}
/**
 * 
 * @param nbSamples
 * @param ptr
 * @param period
 * @return 
 */
bool DeltaADC::get(int &nbSamples, uint16_t **samples, float &period)
{
    if(!_pA.finishDmaSample(nbSamples,samples)) 
    {
            return false;
    }    
    nbSamples/=2; // we deal with pairs from here on
    _pA.disconnect();
    _pB.disconnect();
    
    _pA.dualSimulatenousDelta(nbSamples,*samples);
    switch(_scale)
    {
        case  DSOADC::ADC_PRESCALER_2 : period=2;break;
        case  DSOADC::ADC_PRESCALER_4 : period=4;break;
        case  DSOADC::ADC_PRESCALER_6 : period=6;break;
        case  DSOADC::ADC_PRESCALER_8 : period=8;break;
        default:
            xAssert(0);
            break;
    }
    period=(float)F_CPU/period;
    // now divide by rate
    float r;
    switch(_rate)
    {
        case   ADC_SMPR_1_5 :   r=1.5;break;
        case   ADC_SMPR_7_5 :   r=7.5;break;
        case   ADC_SMPR_13_5 :  r=13.5;break;
        case   ADC_SMPR_28_5 :  r=28.5;break;
        case   ADC_SMPR_41_5 :  r=41.5;break;
        case   ADC_SMPR_55_5 :  r=55.5;break;
        case   ADC_SMPR_71_5 :  r=71.5;break;
        case   ADC_SMPR_239_5 : r=239.5;break;
        default:
            xAssert(0);
            break;
    }
    period=period/(r+12.5); 
    period=1/period;
    return true;
}
/**
 * 
 * @param A
 * @param B
 */
 DeltaADCTime::DeltaADCTime(TestPin &A, TestPin &B)  :DeltaADC(A,B)
{
      
}
/**
 * 
 */
DeltaADCTime::~DeltaADCTime()
{

}
/**
 * 
 * @param frequency
 * @param nbSamples
 * @return 
 */
bool DeltaADCTime::setup(int frequency,const  int nbSamples)
{
    _fq=frequency;
    return _pA.prepareDualTimeSample(_fq,_pB,ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_6,nbSamples);
    
}

/**
 * 
 * @param nbSamples
 * @param ptr
 * @param period
 * @return 
 */
#include "testerGfx.h"
bool DeltaADCTime::get(int &nbSamples, uint16_t **ptr, float &period)
{
    int timerScaler;
    int timerOvf;
    pwmGetScaleOverFlow(_fq,timerScaler, timerOvf);    
    if(!_pA.finishDmaSample(nbSamples,ptr)) 
    {
            return false;
    }  
    nbSamples/=2; // we deal with pairs
    _pA.dualSimulatenousDelta(nbSamples,*ptr);
    period=1./_fq;
    return true;
}
// EOF
    