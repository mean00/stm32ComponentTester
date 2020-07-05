/*
*/

#include "../../SPI/src/SPI.h"
#include "fancyLock.h"
#include "testPins.h"
#include "allComponents.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "testerControl.h"
#include "componentSignature.h"
#include "cpuID.h"
#include "pinConfiguration.h"
#include "waveForm.h"

#define LED PC13

extern void calibration();
extern void menuSystem(void);
uint32_t  deviceId;
uint32_t  memDensity=0;
extern DSOADC *adc;
//



uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
//
#include "pinConfiguration.cpp"
#include "tester.h"

// PB1,PB10,PB11, PB1 is the button
/**
 * @brief 
 * 
 */
class MainTask : public xTask
{
public:
            MainTask() : xTask("MainTask",10,300)
            {

            }
    void    run(void);
protected:
};
/**
 * 
 */
/**
 */
void MainTask::run()
{
    cpuID::identify();
    
    TesterGfx::init();
    TesterGfx::splash();
    
    TestPin::initADC(PA0);
    
    pin1.init();
    pin2.init();
    pin3.init();

    xDelay(100);
    TesterControl::init();
 // The fist Time based sampling is always wrong
    // do a dummy one
    {
        uint16_t *samples;
        int nbSamples;    
        pin2.prepareTimerSample(100*1000,10);      
        pin2.finishTimer(nbSamples,&samples);
    }

    // 
    if(!NVM::hasCalibration())
        calibration();

    TesterGfx::clear();
    TesterGfx::print(6,70,"Press to start");
    TesterControl::waitForAnyEvent();
   
    Tester tester;
    while(1)
    {
        TesterGfx::clear();
        tester.probe();
        TesterControl::waitForAnyEvent();
    }
}

/**
 * 
 */
void mySetup(void)
{
  afio_cfg_debug_ports( AFIO_DEBUG_SW_ONLY); // get PB3 & PB4
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
  
  // Remap Timer from PA6/7 tp PB4/5
  afio_remap(AFIO_REMAP_TIM3_PARTIAL);
  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); // Set the SPI bit order
  SPI.setDataMode(SPI_MODE0); //Set the  SPI data mode 0
  SPI.setClockDivider (SPI_CLOCK_DIV4); // Given for 10 Mhz...    
  
  Serial.end(); // dont let usb bother us
  interrupts();
  Serial1.begin(38400);
  PRINTF("Starting...\n");  
  
  MainTask *mainTask=new MainTask();
  vTaskStartScheduler();        
}
/**
 * 
 */
void myLoop()
{
    xAssert(0);
}
// EOF
