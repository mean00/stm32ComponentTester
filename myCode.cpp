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
TestPin::PULL_STRENGTH strength=TestPin::PULL_HI;
int resistance;
uint16_t *samples;
int nbSamples;
float period;

void Preamble()
{
    zeroAllPins();
    
    // go
    
    pin1.setToGround();
    pin2.pullDown(strength);
}
void PostAmble(const char *title)
{
    TesterGfx::clear();
    TesterGfx::drawCurve(nbSamples,samples);
    float c=Capacitor::computeCapacitance(nbSamples, samples,   resistance,   period);
    
    char st[20];
    Component::prettyPrint(c,"F",st);
    TesterGfx::print(10,10,st);
    TesterGfx::print(10,50,title);
}
void testMonoTime()
{
    Preamble();

    pin2.prepareTimerSample(600*1000,1024);
    pin2.pullUp(strength);       
    resistance=pin2.getCurrentRes()+pin1.getCurrentRes();
    pin2.finishTimer(nbSamples,&samples);
    pin2.pullDown(TestPin::PULL_LOW);   
    PostAmble("MonoTime");
}
void testDualTime()
{
    Preamble();
    DeltaADCTime delta(pin2,pin1);
    if(!delta.setup(600*1000,1024)) 
        xAssert(0);
    
    pin2.pullUp(strength);   
    
    resistance=pin2.getCurrentRes()+pin1.getCurrentRes();
    bool r=delta.get(nbSamples,&samples,period);
    pin2.pullDown(TestPin::PULL_LOW);   
    PostAmble("DualTime");    
}
void testDualDma()
{
    Preamble();
    DeltaADC delta(pin2,pin1);
    float period;
    
    if(!delta.setup(ADC_SMPR_41_5,DSOADC::ADC_PRESCALER_8,1024)) xAssert(0); // 0.5us
    
    pin2.pullUp(strength);   
    
    resistance=pin2.getCurrentRes()+pin1.getCurrentRes();
    bool r=delta.get(nbSamples,&samples,period);
    pin2.pullDown(TestPin::PULL_LOW);   
    PostAmble("DualDMA");   
}
        
void testMonoDma()
{
    Preamble();

    pin2.prepareDmaSample(ADC_SMPR_41_5,DSOADC::ADC_PRESCALER_8,1024);
    pin2.pullUp(strength);       
    resistance=pin2.getCurrentRes()+pin1.getCurrentRes();
    pin2.finishDmaSample(nbSamples,&samples);
    pin2.pullDown(TestPin::PULL_LOW);   
    PostAmble("MonoDMA");
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
   
    // The fist Time based sampling is always wrong
    // do a dummy one
    {
    uint16_t *samples;
    int nbSamples;    
    pin2.prepareTimerSample(100*1000,10);      
    pin2.finishTimer(nbSamples,&samples);
    }
    
    
    
    
#if 0
    adc->setupDmaSampling();
    adc->prepareDMASampling(ADC_SMPR_239_5,DSOADC::ADC_PRESCALER_8);    
    adc->stopDmaCapture();
#endif    
#if 1    
    TesterGfx::clear();    
#if 0    
    testDualDma();
    TesterControl::waitForAnyEvent();
#endif
#if 1       
    testMonoDma();
    TesterControl::waitForAnyEvent();
    testMonoTime();
    TesterControl::waitForAnyEvent();
    testMonoTime();
    TesterControl::waitForAnyEvent();
    testMonoTime();
    testDualDma();
    TesterControl::waitForAnyEvent();
    testMonoTime();
    TesterControl::waitForAnyEvent();
    testMonoTime();
    TesterControl::waitForAnyEvent();
    
#endif
#if 0
    
    testDualTime();
    TesterControl::waitForAnyEvent();
    testDualTime();
#endif    
    while(1)
    {
        
    }
#endif            
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
 * \fn probe for 3 poles 
 * @param a
 * @param b
 * @param c
 * @param comp
 */
bool probeMe3(TestPin &a,TestPin &b,TestPin &c,Component **comp)
{
    COMPONENT_TYPE xtype;
    Component *c2=Component::identity3(a,b,c,xtype);
    if(!c2)
        return false;
  
    delete *comp;
    *comp=c2;
    return true;
}
/**
 * \fn probe for 2 poles
 * @param a
 * @param b
 * @param c
 * @param comp
 */
void probeMe2(TestPin &a,TestPin &b,TestPin &c,Component **comp)
{
    COMPONENT_TYPE xtype;
    Component *c2=Component::identity2(a,b,c,xtype);
    if(!c2)
        return;
    
    if(!*comp) 
    {
        *comp=c2;
        return ;
    }        
    bool replace=false;
    if(c2->likely()>(*comp)->likely())
    {
        replace=true;
    }
    if(replace)
    {
        delete *comp;
        *comp=c2;
    }else
    {
        delete c2;
    } 
}


/**
 * 
 */
void myLoop(void)
{
    COMPONENT_TYPE type;
    
    // 1 : Zeroing
    TesterGfx::clear();
    TesterGfx::print(30,40,"Zeroing..");
    zeroAllPins();
    TesterGfx::clear();
    TesterGfx::print(0,40,"Click to probe");
next:
    TesterGfx::clear();
    TesterGfx::print(0,40,"Detecting");
    
#if 1      
    Component *c=NULL;

#define PROBEME3(column, alpha, beta,gamma)     {TesterGfx::print(column,75,"x");   probeMe3(alpha,beta,gamma,&c);}     
#define PROBEME(column, alpha, beta,gamma)     {TesterGfx::print(column,75,"*");    probeMe2(alpha,beta,gamma,&c);} 
    
    PROBEME3(20,pin1,pin2,pin3)
    PROBEME3(40,pin1,pin3,pin2)
    PROBEME3(60,pin2,pin3,pin1)
    
    if(!c)
    {
        PROBEME(20,pin1,pin2,pin3)
        PROBEME(40,pin1,pin3,pin2)
        PROBEME(60,pin2,pin3,pin1)
        
    }
    
    
#else
    TesterGfx::printStatus("Probing");
    Component *c=new NPNBjt(pin2,pin3,pin1);
    //Component *c=new Coil(pin1,pin2,pin3);
    //Component *c=new Capacitor(pin3,pin2,pin1);
#endif
    if(!c)
    {
        TesterGfx::clear();
        TesterGfx::print(0,60,"Found nothing");
        if(TesterControl::waitForEvent() & CONTROL_LONG)
            menuSystem();
        return;
    }
    TesterGfx::clear();
    const char *sname=c->getShortName();
    TesterGfx::print(8,40,sname);
    TesterGfx::print(0,60,"  Measuring");

    // Valid component detected ?
    if(c->compute())
    {   
        zeroAllPins();
        TesterGfx::clear();
        c->draw(0);     
        while(1)
        {
            int evt=TesterControl::waitForEvent();            
            if(evt & CONTROL_LONG) menuSystem(); // probe next
            if(evt & CONTROL_SHORT) goto next; // probe next
            // TODO LONG => menu
            if(evt & CONTROL_ROTARY)
            {
                int count=TesterControl::getRotary();
                c->changePage(count);
            }
        }

        
    }
    delete c;
    TesterGfx::printStatus("-------------");
    for(int i=0;i<5;i++)
    {
        digitalWrite(LED,HIGH);
        xDelay(200);
        digitalWrite(LED,LOW);
        xDelay(200);
    }
}
// EOF
