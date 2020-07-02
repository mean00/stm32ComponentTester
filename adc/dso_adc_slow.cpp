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
 */
uint32_t lastStartedCR2=0;
uint32_t lastStartedCR1=0;
uint32_t lastStartedSR;
//

CaptureState captureState=Capture_idle;


/**
 * 
 */
void DSOADC::stopTimeCapture(void)
{
     ADC_TIMER.pause();
     adc_dma_disable(ADC1);
}
/**
 * 
 * @return 
 */
bool DSOADC::startDMATime()
{    
#define USE_CONT 0
  cr2=ADC1->regs->CR2;  
  cr2&= ~(ADC_CR2_SWSTART+ADC_CR2_CONT);   
  ADC1->regs->CR2=cr2;
  setSourceInternal();   
  cr2|=ADC_CR2_CONT*USE_CONT+ADC_CR2_DMA;    
  ADC1->regs->CR2=cr2;    
#if 0  
  cr2|= ADC_CR2_SWSTART;   
  ADC1->regs->CR2=cr2;    
#endif  
  return true;  
}
/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startInternalDmaSampling ()
{
  //  slow is always single channel
  ADC1->regs->CR1&=~ADC_CR1_DUALMASK;
  setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_Event,false );
  
  startDMATime();
  volatile uint32_t s =ADC1->regs->DR;
  s =ADC2->regs->DR;
  ADC_TIMER.resume();  
  lastStartedCR2=ADC1->regs->CR2;
  return true;
}

/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startTimerSampling (int count)
{

   if(count>ADC_INTERNAL_BUFFER_SIZE)
        count=ADC_INTERNAL_BUFFER_SIZE;
    requestedSamples=count;

    FancyInterrupts::disable();    
    captureState=Capture_armed;   
    startInternalDmaSampling();           
    FancyInterrupts::enable();
    return true;
} 
/**
 * 
 * @param count
 * @return 
 */

uint32_t regs1[16];
uint32_t regs2[16];
void dumpAdcRegs()
{
    uint32_t *p1=(uint32_t *)ADC1->regs;
    uint32_t *p2=(uint32_t *)ADC2->regs;
     for(int i=0;i<15;i++)
    {
        regs1[i]=p1[i];
        regs2[i]=p2[i];
    } 
}
/**
 * 
 * @param count
 * @return 
 */
bool DSOADC::startDualTimeSampling (int count)
{
    
    if(count>ADC_INTERNAL_BUFFER_SIZE)
        count=ADC_INTERNAL_BUFFER_SIZE;
    requestedSamples=count;
    
    FancyInterrupts::disable();    
    captureState=Capture_armed;   
    
    dumpAdcRegs();   
    
    setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_Event,false );
  
    startDMATime();
    volatile uint32_t s =ADC1->regs->DR;
    s =ADC2->regs->DR;

    dumpAdcRegs();   
    
    ADC_TIMER.resume();  
    lastStartedCR2=ADC1->regs->CR2;
    FancyInterrupts::enable();
    return true;
} 

/**
 * 
 * @param fq
 * @param otherPin
 * @param rate
 * @param scale
 * @return 
 */
bool    DSOADC::prepareDualTimeSampling (int fq,int otherPin, adc_smp_rate rate,DSOADC::Prescaler  scale)
{  
    
    _dual=DSOADC::ADC_CAPTURE_DUAL_SIMULTANEOUS;
    ADC1->regs->CR1&=~ADC_CR1_DUALMASK;  
    ADC1->regs->CR1|=ADC_CR1_DUAL_REGULAR_SIMULTANEOUS;
    ADC2->regs->CR1&=~ADC_CR1_DUALMASK;  
    ADC2->regs->CR1|=ADC_CR1_DUAL_REGULAR_SIMULTANEOUS;
    
    dumpAdcRegs();   
    
    ADC2->regs->SQR3 = PIN_MAP[otherPin].adc_channel ;          
    ADC2->regs->CR2 &= ~ADC_CR2_CONT;
    ADC1->regs->CR2 &= ~ADC_CR2_CONT ;
    ADC1->regs->CR2 |= ADC_CR2_DMA;
    
    dumpAdcRegs();   
    
    adc_set_sample_rate(ADC2, rate); 
    adc_set_sample_rate(ADC1, rate);      
    setTimeScale(rate,scale);        
    setSourceInternal();
    
    uint32_t cr2=ADC2->regs->CR2;  
    cr2 &=~ ADC_CR2_EXTSEL_SWSTART;
    ADC2->regs->CR2=cr2;
    cr2 |= ((int)_source) << 17;
    ADC2->regs->CR2=cr2;         
    
    dumpAdcRegs();   
    
    return true;
}

// EOF

