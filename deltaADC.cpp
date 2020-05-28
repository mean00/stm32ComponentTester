#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "deltaADC.h"

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
    _pA.disconnect();
    _pB.disconnect();
    // convert to delta
    if(_rate==ADC_SMPR_1_5 && _scale== DSOADC::ADC_PRESCALER_6)
    {
        _pA.dualInterleavedDelta(nbSamples,*samples); // skip first samples
        period=0.5/1000000.; // 0.5 us
        return true;
    }
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
    period=72000000./period;
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