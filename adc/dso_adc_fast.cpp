/**
 * Derived from https://github.com/pingumacpenguin/STM32-O-Scope/blob/master/STM32-O-Scope.ino
 */
/*.
(c) Andrew Hull - 2015
STM32-O-Scope - aka "The Pig Scope" or pigScope released under the GNU GENERAL PUBLIC LICENSE Version 2, June 1991
https://github.com/pingumacpenguin/STM32-O-Scope
Adafruit Libraries released under their specific licenses Copyright (c) 2013 Adafruit Industries.  All rights reserved.
*/
/**
 We use PA0 as input pin
 * DMA1, channel 0
 
 * Vref is using PWM mode for Timer4/Channel 3
 * 
 */
#include "dso_adc.h"
#include "fancyLock.h"
#include "dma.h"
#include "adc.h"
/**
 */
#ifndef ADC_CR1_FASTINT
    #define ADC_CR1_FASTINT 0x70000
#endif ADC_CR1_FASTINT

adc_reg_map *adc_Register;



uint16_t DSOADC::adcInternalBuffer[ADC_INTERNAL_BUFFER_SIZE] __attribute__ ((aligned (8)));;;

int dmaSpuriousInterrupt=0;
extern HardwareTimer Timer4;
static bool triggered=false;

/**
 */
int requestedSamples;
uint32_t vcc; // power Supply in mv


FancySemaphore      *dmaSemaphore;
DSOADC             *instance=NULL;
/**
 * 
 */
DSOADC::DSOADC(int pin)
{
  instance=this;
  _pin=pin;

 
  // Set up our sensor pin(s)  
  dmaSemaphore=new FancySemaphore;  
  adc_Register=  PIN_MAP[_pin].adc_device->regs;
  
  
  enableDisableIrq(false);
  enableDisableIrqSource(false,ADC_AWD);
  enableDisableIrqSource(false,ADC_EOC);  
}
  
/**
 * 
 * @param adc
 * @return 
 */
float DSOADC::adcToVolt(float adc)
{
    adc=adc*vcc;
    adc/=4095000.;
    return adc;
}

/**
 */
bool    DSOADC::setADCPin(int pin)
{
    _pin=pin;
     adc_Register=  PIN_MAP[_pin].adc_device->regs;
     setChannel(PIN_MAP[_pin].adc_channel);
     return true;
}

// Grab the samples from the ADC
// Theoretically the ADC can not go any faster than this.
//
// According to specs, when using 72Mhz on the MCU main clock,the fastest ADC capture time is 1.17 uS. As we use 2 ADCs we get double the captures, so .58 uS, which is the times we get with ADC_SMPR_1_5.
// I think we have reached the speed limit of the chip, now all we can do is improve accuracy.
// See; http://stm32duino.com/viewtopic.php?f=19&t=107&p=1202#p1194


bool DSOADC::startDMASampling (int count)
{
  if(count>ADC_INTERNAL_BUFFER_SIZE)
        count=ADC_INTERNAL_BUFFER_SIZE;  
  requestedSamples=count;    
  enableDisableIrqSource(false,ADC_AWD);
  enableDisableIrq(true);
  setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_Event );
  return true;
}
/**
 * 
 * @param count
 * @return 
 */
bool DSOADC::startDualDMASampling (int otherPin, int count)
{
  if(count>ADC_INTERNAL_BUFFER_SIZE/2)
        count=ADC_INTERNAL_BUFFER_SIZE/2;  
  requestedSamples=count;    
  enableDisableIrqSource(false,ADC_AWD);
  enableDisableIrq(true);
  setupAdcDualDmaTransfer( otherPin, requestedSamples,(uint32_t *)adcInternalBuffer, DMA1_CH1_Event );
  return true;
}


/**
 * 
 */
void SPURIOUS_INTERRUPT()
{
    
}


void DSOADC::stopDmaCapture(void)
{
    // disable interrupts
    enableDisableIrq(false);
    enableDisableIrqSource(false,ADC_AWD);
    // Stop dma
     adc_dma_disable(ADC1);
}


volatile uint32_t lastCR1=0;

#define SetCR1(x) {lastCR1=ADC1->regs->CR1=(x);}

static voidFuncPtr adcIrqHandler=NULL;
/**
 */
uint32_t DSOADC::getVCCmv()
{
    return vcc;
}
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
    adc_Register->SQR3 = _pin;
}
/**
 * 
 */
void DSOADC::setupADCs ()
{
   adc_Register = ADC1->regs;

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
   
  ADC1->regs->CR2 |=ADC_CR2_DMA +ADC_CR2_EXTSEL_SWSTART +ADC_CR2_CONT  ;     
  ADC2->regs->CR2 |=ADC_CR2_DMA +ADC_CR2_EXTSEL_SWSTART +ADC_CR2_CONT ;     
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
    ADC1->regs->CR2 |= ADC_CR2_DMA;    
    ADC2->regs->CR2 |= ADC_CR2_DMA;    
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
    ADC2->regs->CR2 |= ADC_CR2_CONT+ADC_CR2_DMA;
    ADC1->regs->CR2 |= ADC_CR2_CONT+ADC_CR2_DMA;
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


/**
 * 
 * @param reg
 */
void DSOADC::resetCR2(adc_reg_map *regs)
{
    uint32_t cr2=regs->CR2;
    cr2&=~ADC_CR2_ADON;
    regs->CR2=cr2;
    delayMicroseconds(50);
    regs->CR2=cr2|ADC_CR2_ADON;
    delayMicroseconds(50);
}


