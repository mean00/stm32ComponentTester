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
#include "pinConfiguration.cpp"
#include "tester.h"

#define LED PC13

extern void calibration();
extern void menuSystem(void);
extern void grapher( TestPin *p1,TestPin *p2);

// Free RTOS heap

uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

extern void menu(void);
bool Tester_waitForkeyPress(const char *msg);
/**
 * @brief 
 * 
 */
class MainTask : public xTask
{
public:
            MainTask() : xTask("MainTask",10,400)
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
#if 0    
    Coil c(pin1,pin2,pin3);
    c.compute();
#endif    
#if 0    
    grapher(&pin2,&pin1);
#endif
    
#if 0
        menuSystem();
#endif
#if 0   
    Capacitor c(pin1,pin2,pin3);
    
    c.compute();
    char st[40];
    Component::prettyPrint(c.getValue(),"F",st);
    TesterGfx::print(10,20,st);
    TesterControl::waitForAnyEvent();
    int sum,nb;
    pin1.summedRead(sum,nb);
    Capacitor::quickEval(pin1,pin2,pin3);
    c.compute();
    Component::prettyPrint(c.getValue(),"F",st);
    TesterGfx::print(10,20,st);
    TesterControl::waitForAnyEvent();
#endif    
#if 0 // calibration
  // Stroboscopic calibration, only on pin2
    
    TesterGfx::print(5,28,"Fine cal");
    
    // dummy scan
    Capacitor xcap(pin1,pin2,pin3);
    float ccc;
    xcap.compute1nfRange(ccc);    
    
    pin1.setToGround();
    pin3.setToGround();
    pin2.setToGround();
    TestPinCalibration cal1,cal3;
    xDelay(10);
    {
        float cap;
        Capacitor::calibrationVeryLow(pin1,pin2,cal1);
        Capacitor::calibrationVeryLow(pin3,pin2,cal3);
    }
#endif    
    
  
    
    
    // All ready
    Tester tester;
    
    Tester_waitForkeyPress("Press to start");

    while(1)
    {
        TesterGfx::clear();
        bool found=tester.probe();
        if(!found)
        {
            Tester_waitForkeyPress("Found nothing");
        }
    }
}

/**
 * 
 * @param msg
 * @return 
 */
bool Tester_waitForkeyPress(const char *msg)
{
    TesterGfx::clear();
    TesterGfx::print(0,60,msg);

    int evt=TesterControl::waitForEvent();            
        if(evt & CONTROL_LONG) 
        {
            menu();
        }
    return true;
}

void menu()
{
    
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
  Logger("Starting...\n");  
  
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
// Defining this mostly disable usb altogether and we gain ~ 4 kB of code
extern "C" void __irq_usb_lp_can_rx0()
{

}
// EOF
