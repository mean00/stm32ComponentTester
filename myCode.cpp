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
void myLoop(void);
extern void calibration();
extern void menuSystem(void);
uint32_t  deviceId;
uint32_t  memDensity=0;
extern DSOADC *adc;
//
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
//
#include "pinConfiguration.cpp"

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
void testMonoTime()
{
    int resistance;
    uint16_t *samples;
    int nbSamples;
    float period;
    
    
    TesterGfx::print(5,30,"MonoT");
    zeroAllPins();
    
    // go
    TestPin::PULL_STRENGTH strength=TestPin::PULL_HI;
    pin1.setToGround();
    pin2.pullDown(strength);

    pin2.prepareTimerSample(2000,1024);
    pin2.pullUp(strength);       
    resistance=pin2.getCurrentRes()+pin1.getCurrentRes();
    pin2.finishTimer(nbSamples,&samples);
    pin2.pullDown(TestPin::PULL_LOW);   

    TesterGfx::clear();
    TesterGfx::drawCurve(nbSamples,samples);
    float c=Capacitor::computeCapacitance(nbSamples, samples,   resistance,   period);
    
    char st[20];
    Component::prettyPrint(c,"F",st);
    TesterGfx::print(10,10,st);
   
}
void testDualTime()
{
    int resistance;
    uint16_t *samples;
    int nbSamples;
    float period;
    
    
    TesterGfx::print(5,30,"DualTime");
    zeroAllPins();
    
    // go
    TestPin::PULL_STRENGTH strength=TestPin::PULL_HI;
    pin1.setToGround();
    pin2.pullDown(strength);

    // start the DMA
    // max duration ~ 512 us
    DeltaADCTime delta(pin2,pin1);
    if(!delta.setup(500*1000,1024)) 
        xAssert(0);
    
    pin2.pullUp(strength);   
    
    resistance=pin2.getCurrentRes()+pin1.getCurrentRes();
    bool r=delta.get(nbSamples,&samples,period);
    pin2.pullDown(TestPin::PULL_LOW);   
    if(!r) 
        xAssert(0);
    
    TesterGfx::clear();
    TesterGfx::drawCurve(nbSamples,samples);
    float c=Capacitor::computeCapacitance(nbSamples, samples,   resistance,   period);
    
    char st[20];
    Component::prettyPrint(c,"F",st);
    TesterGfx::print(10,10,st);
    
}
void testDualDma()
{
     int resistance;
    zeroAllPins();
    // go
    TestPin::PULL_STRENGTH strength=TestPin::PULL_HI;
    pin1.setToGround();
    pin2.pullDown(strength);

    // start the DMA in time mode    
    uint16_t *samples;
    int nbSamples;
    DeltaADC delta(pin2,pin1);
    float period;
    
    if(!delta.setup(ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_4,1024)) xAssert(0);
    
    pin2.pullUp(strength);   
    
    resistance=pin2.getCurrentRes()+pin1.getCurrentRes();
    bool r=delta.get(nbSamples,&samples,period);
    pin2.pullDown(TestPin::PULL_LOW);   
    if(!r) 
        xAssert(0);
    
    TesterGfx::drawCurve(nbSamples,samples);
    float c=Capacitor::computeCapacitance(nbSamples, samples,   resistance,   period);
    
    char st[20];
    Component::prettyPrint(c,"F",st);
    TesterGfx::print(10,10,st);
   
}
        

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
    
    // Do a dummy capture to make sure everything is fine    
    pin1.setMode(TestPin::GND);   
    adc->setADCPin(PA0);

    adc->setupDmaSampling();
    adc->prepareDMASampling(ADC_SMPR_239_5,DSOADC::ADC_PRESCALER_8);    
    adc->stopDmaCapture();
    
    TesterGfx::clear();
    testDualDma();
    testMonoTime();
    testDualTime();
    testDualTime();
    while(1)
    {
        
    }
            
  // 
    if(!NVM::hasCalibration())
        calibration();
    
    TesterGfx::clear();
    TesterGfx::print(6,70,"Press to start");
    TesterControl::waitForAnyEvent();
    
    while(1)
    {
        myLoop();
        
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
void myLoop(void)
{
  
}
