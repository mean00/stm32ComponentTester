#include "dso_adc.h"
#include "dso_adc_priv.h"
#include "fancyLock.h"
#include "dma.h"
#include "adc.h"



/**
 * 
 * @return 
 */
#define NB_SAMPLE 16
float DSOADC::readVCCmv()
{
   float fvcc=0;   
   adc_Register->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
   adc_Register->SMPR1 =  ADC_SMPR1_SMP17;  // sample ra
   for(int i=0;i<NB_SAMPLE;i++)
   {
       delay(10);   
       fvcc+=  adc_read(ADC1, 17); 
   }
    fvcc=(1200. * 4096.*NB_SAMPLE) /fvcc;   
    adc_Register->CR2 &= ~ADC_CR2_TSVREFE;    // disable VREFINT and temp sensor
    vcc=fvcc;
    return fvcc;
}
/**
 * 
 * @param dev
 */
static void initSeqs(adc_dev *dev)
{
    adc_disable(dev);    
    delayMicroseconds(50);
    adc_init(dev);
    delayMicroseconds(50);
    adc_set_extsel(dev, ADC_SWSTART);
    delayMicroseconds(50);
    adc_set_exttrig(dev, 1);
    delayMicroseconds(50);
    adc_enable(dev);
    delayMicroseconds(50);
    adc_calibrate(dev);
    delayMicroseconds(50);
}
/**
 * 
 * @param channel
 */

void DSOADC::setChannel(int channel)
{    
    adc_Register->SQR3 = channel;
}
/**
 * 
 */
void DSOADC::setupADCs ()
{   
  // Restart from the beginning
  initSeqs(ADC1);
  initSeqs(ADC2);

  adc_set_sample_rate(ADC1, ADC_SMPR_1_5); //=0,58uS/sample.  ADC_SMPR_13_5 = 1.08uS - use this one if Rin>10Kohm,
  adc_set_sample_rate(ADC2, ADC_SMPR_1_5);    // if not may get some sporadic noise. see datasheet.
  adc_set_prescaler(ADC_PRE_PCLK2_DIV_6);
   
  adc_set_reg_seqlen(ADC1, 1);
  
  int channel = PIN_MAP[_pin].adc_channel;
  setChannel(channel);
  
  readVCCmv();
  

  cr2=0;
  ADC1->regs->CR2=cr2;
  ADC2->regs->CR2=cr2;
  
  cr2=ADC_CR2_EXTSEL_SWSTART|ADC_CR2_EXTTRIG; 
  ADC1->regs->CR2=cr2;
  ADC2->regs->CR2=cr2;
  
  cr2 |=ADC_CR2_ADON;
  ADC1->regs->CR2=cr2;
  ADC2->regs->CR2=cr2; // Power on
  
  
}
/**
 * 
 * @param timeScaleUs
 * @return 
 */
 bool    DSOADC::setTimeScale(adc_smp_rate one, adc_prescaler two)
 {
    adc_set_sample_rate(ADC1, one); //=0,58uS/sample.  ADC_SMPR_13_5 = 1.08uS - use this one if Rin>10Kohm,
    adc_set_prescaler(two);
    return true;
 }
 /**
  * 
  * @param count
  * @return 
  */
bool    DSOADC::prepareDMASampling (adc_smp_rate rate,adc_prescaler scale)
{    
    cr2= ADC1->regs->CR2;
    cr2|=ADC_CR2_DMA | ADC_CR2_CONT;    
    ADC1->regs->CR2 = cr2;    
    setTimeScale(rate,scale);         
    return true;
}/**
  * 
  * @param count
  * @return 
  */
bool    DSOADC::prepareDualDMASampling (int otherPin, adc_smp_rate rate,adc_prescaler scale)
{  
    ADC1->regs->CR1|=ADC_CR1_FASTINT; // fast interleaved mode
    ADC2->regs->SQR3 = PIN_MAP[otherPin].adc_channel ;      
    ADC2->regs->CR2 |= ADC_CR2_CONT |ADC_CR2_DMA;
    ADC1->regs->CR2 |= ADC_CR2_CONT |ADC_CR2_DMA;
    adc_set_sample_rate(ADC2, rate); 
    setTimeScale(rate,scale);
    
    return true;
}
/**
 * 
 */
void  DSOADC::clearSemaphore()
{
    dmaSemaphore->reset();    
}

/**
* @brief Enable DMA requests
* @param dev ADC device on which to enable DMA requests
*/

void DSOADC::adc_dma_enable(const adc_dev * dev) 
{
  bb_peri_set_bit(&dev->regs->CR2, ADC_CR2_DMA_BIT, 1);  
}


/**
* @brief Disable DMA requests
* @param dev ADC device on which to disable DMA requests
*/

void DSOADC::adc_dma_disable(const adc_dev * dev) 
{
  bb_peri_set_bit(&dev->regs->CR2, ADC_CR2_DMA_BIT, 0);
  dma_attach_interrupt(DMA1, DMA_CH1, SPURIOUS_INTERRUPT);
}


/**
 * 
 */
void Oopps()
{
    xAssert(0);
}
/**
 * 
 * @param count
 * @param buffer
 * @param handler
 */
void DSOADC::setupAdcDmaTransfer(   int count,uint16_t *buffer, void (*handler)(void) )
{
  dma_init(DMA1);
  dma_attach_interrupt(DMA1, DMA_CH1, handler); 
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, (uint32_t *)buffer, DMA_SIZE_16BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
  dma_set_num_transfers(DMA1, DMA_CH1, count );
  adc_dma_enable(ADC1);
  dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.

}
void DSOADC::setupAdcDualDmaTransfer( int otherPin,  int count,uint32_t *buffer, void (*handler)(void) )
{
 
    
  dma_init(DMA1);
  dma_attach_interrupt(DMA1, DMA_CH1, handler); 
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, buffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
  dma_set_num_transfers(DMA1, DMA_CH1, count );
  adc_dma_enable(ADC1);
  dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.

}

void DSOADC::nextAdcDmaTransfer( int count,uint16_t *buffer)
{
    dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, (uint32_t *)buffer, DMA_SIZE_16BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
    dma_set_num_transfers(DMA1, DMA_CH1, count );
    dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.
}

/**
 * 
 * @param interruptMask
 */
void DSOADC::enableDisableIrqSource(bool onoff, int interrupt)
{
    if(onoff)
    {
        // Enable Watchdog or EndOfCapture interrupt flags
        switch(interrupt)
        {
            case ADC_AWD:        
            {
                int channel=0;
                uint32_t cr1=ADC1->regs->CR1;
                cr1 &=~ 0x1f;
                cr1|= (channel & ADC_CR1_AWDCH) | 0*ADC_CR1_AWDSGL ;
                cr1|= ADC_CR1_AWDEN  | ADC_CR1_AWDIE;                
                SetCR1(cr1);
            }
                break;
            case ADC_EOC:
                {
                  uint32_t cr1=ADC1->regs->CR1;
                  cr1 |= ADC_CR1_EOCIE;
                  SetCR1(cr1);
                }
                 break;
            default:
                xAssert(0);
                break;
        }
    }else // disable
    {
        switch(interrupt)
        {
            case ADC_AWD:        
            {
                int channel=0;
                uint32_t cr1=ADC1->regs->CR1;
                cr1 &=~ 0x1f;
                cr1&= ~(ADC_CR1_AWDEN  | ADC_CR1_AWDIE);
                SetCR1(cr1);
            }
                break;
            case ADC_EOC:
                {
                  uint32_t cr1=ADC1->regs->CR1;
                  cr1 &= ~ADC_CR1_EOCIE;
                  SetCR1(cr1);
                }
                 break;
            default:
                xAssert(0);
                break;
        }
    }
}
/**
 * 
 * @param onoff
 */
void DSOADC::enableDisableIrq(bool onoff)
{
    if(onoff)
    {
        nvic_irq_enable(ADC1->irq_num);
    }
    else
        nvic_irq_disable(ADC1->irq_num);
}
/**
 * 
 */
void DSOADC::defaultAdcIrqHandler()
{    
    if(adcIrqHandler)
        adcIrqHandler();
}
    
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

/**
 * 
 * @param count
 * @return 
 */
bool DSOADC::getSamples(uint16_t **samples, int &nbSamples)
{
    if(!dmaSemaphore->take(200)) // dont busy loop
        return false;       
    *samples=adcInternalBuffer;
    nbSamples=requestedSamples;
    return true;
}


/**
 * 
 */
void DSOADC::DMA1_CH1_Event() 
{
    instance->captureComplete();
    adc_dma_disable(ADC1);
}


/**
 */
void DSOADC::captureComplete()
{
    dmaSemaphore->giveFromInterrupt();
}



extern uint32_t registersX[10];

int  readAllRegisters()
{
    registersX[0]=ADC1->regs->CR2;
    registersX[1]=ADC1->regs->SQR3;
    return 3;
}