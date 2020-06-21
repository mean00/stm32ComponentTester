/**
 
 *  This is the slow capture mode
 * i.e. we setup n Samples acquisition through DMA
 * and a timer interrupt grabs the result 
 */

//#include "dso_global.h"
#include "dso_adc.h"
#include "dso_adc_priv.h"
#include "fancyLock.h"



/**
 *  This is called only once when we change mode
 * @return 
 */
bool    DSOADC::setupTimerSampling()
{   
  ADC_TIMER.pause();
  setSource(ADC_SOURCE_TIMER);    
  _oldTimerFq=0;
  return true;
}

/**
 * 
 * @param fq
 * @return 
 */
bool    DSOADC::prepareTimerSampling (int fq,bool overSampling,adc_smp_rate rate , DSOADC::Prescaler scale)
{   
  ADC_TIMER.pause();
  if(fq!=_oldTimerFq)
  {
    _oldTimerFq=fq;
    _timerSamplingRate=rate;
    _timerScale=scale;
    _overSampling=false;
    int scaler=F_CPU/(fq*65535);
    scaler+=1;
    int high=F_CPU/scaler;
    int overFlow=(high+fq/2)/fq;

    ADC_TIMER.pause();
    ADC_TIMER.setPrescaleFactor(scaler);
    ADC_TIMER.setOverflow(overFlow);
    ADC_TIMER.setCompare(ADC_TIMER_CHANNEL,overFlow-1);
    timer_cc_enable(ADC_TIMER.c_dev(), ADC_TIMER_CHANNEL);
  }
  return true;    
}

/**
 * 
 * @param overSamp
 * @return 
 */ 
bool    DSOADC::setOverSamplingFactor  (int overSamp)
{   
    return true;
}


/**
 * 
 * @return 
 */
int DSOADC::pollingRead()
{    
  adc_reg_map *regs=ADC1->regs;  
  uint32_t oldCr2=regs->CR2;
  
  cr2=regs->CR2;
  cr2&= ~(ADC_CR2_SWSTART+ADC_CR2_CONT+ADC_CR2_DMA);   
  // Set source to SWSTART
   cr2=ADC1->regs->CR2;  
   cr2 &=~ ADC_CR2_EXTSEL_SWSTART;
   ADC1->regs->CR2=cr2;
   cr2 |= ((int)ADC_CR2_EXTSEL_SWSTART) << 17;
   ADC1->regs->CR2=cr2;         
   cr2=ADC1->regs->CR2;
  
  regs->CR2=cr2;
  // then poll
  cr2|=ADC_CR2_SWSTART;
  regs->CR2=cr2;
  // wait for end of sampling  
  while (!(regs->SR & ADC_SR_EOC))
  {
      
  }      
  uint16_t val= (uint16)(regs->DR & ADC_DR_DATA);
  regs->CR2=oldCr2;
  return val;
}
// EOF

