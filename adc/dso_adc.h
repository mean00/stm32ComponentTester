/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once
#include "Arduino.h"

// Sampling Queue

#define ADC_INTERNAL_BUFFER_SIZE 1024



/**
 * \class DSOADC
 * \brief wrapper on top of the DMA-ADC engine
 */
class DSOADC
{
public:  
  
  
public:
                    DSOADC(int pin);
            bool    setTimeScale(adc_smp_rate one, adc_prescaler two);
            bool    prepareDMASampling (adc_smp_rate rate,adc_prescaler scale);                        
            void    clearSemaphore() ;
    static  uint32_t getVCCmv(); // this one is cached
    static  float    readVCCmv();    
            bool getSamples(uint16_t **samples, int &nbSamples);
            

            bool startDMASampling (int count);

    static  void adc_dma_disable(const adc_dev * dev) ;            
    static  void adc_dma_enable(const adc_dev * dev) ;    

            void setupADCs ();
protected:            
            
    static  void DMA1_CH1_Event();
    static  void DMA1_CH1_TriggerEvent() ;            
            bool startInternalDmaSampling ();
                        
    
public:        
    static void setupAdcDmaTransfer(   int count,uint16_t *buffer, void (*handler)(void) );
    static void nextAdcDmaTransfer( int count,uint16_t *buffer);

    static void enableDisableIrqSource(bool onoff, int interruptMask);
    static void enableDisableIrq(bool onoff);
    static void defaultAdcIrqHandler();
    static void attachWatchdogInterrupt(voidFuncPtr handler);
    static void setWatchdogTriggerValue(uint32_t high, uint32_t low);
            void stopDmaCapture();
            void stopTimeCapture();
    
            void captureComplete();
protected:
  
            int             _sampled;
            int             _pin;
static      uint16_t        adcInternalBuffer[ADC_INTERNAL_BUFFER_SIZE];            
};

// EOF

