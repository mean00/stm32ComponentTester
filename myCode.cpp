/*
*/

#include "../../SPI/src/SPI.h"
#include "fancyLock.h"
#include "testPins.h"
#include "allComponents.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "wav_irotary.h"
#include "componentSignature.h"
#include "cpuID.h"
#include "pinConfiguration.h"
#define LED PC13
void myLoop(void);
extern void rotaryTest();
extern void calibration();
uint32_t  deviceId;

uint32_t  memDensity=0;
uint32_t cr2;
//
int result[20];
int z,zz;
extern void  pinTest();
//
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
//
#include "pinConfiguration.cpp"

WavRotary rotary(PIN_ROTARY_LEFT,PIN_ROTARY_RIGHT); // PB1,PB10,PB11, PB1 is the button

void MainTask( void *a )
{
    cpuID::identify();
    
    
    TesterGfx::init();
    
    TestPin::initADC(PA0);
    
    
    pin1.init();
    pin2.init();
    pin3.init();
    xDelay(100);
    rotary.start();
#if 0    
    rotaryTest();
#endif
    
#if 0    
    pinTest();
    while(1)
    {
        
    };
#endif
  // 
    if(!NVM::hasCalibration())
        calibration();
    
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
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); // Set the SPI bit order
  SPI.setDataMode(SPI_MODE0); //Set the  SPI data mode 0
  SPI.setClockDivider (SPI_CLOCK_DIV4); // Given for 10 Mhz...    
  
  Serial.end(); // dont let usb bother us
  interrupts();
  Serial1.begin(38400);
  Serial1.println("Starting...\n");
  
  
  xTaskCreate( MainTask, "MainTask", 750,NULL, 10, NULL );   
  vTaskStartScheduler();        
}

void probeMe(TestPin &a,TestPin &b,TestPin &c,Component **comp)
{
    COMPONENT_TYPE xtype;
    Component *c2=Component::identity(a,b,c,xtype);
    if(!c2)
        return;
    
    if(!*comp) 
    {
        *comp=c2;
        return ;
    }        
    bool replace=false;
    if(c2->nbPins()>(*comp)->nbPins())
    {
        replace=true;
    }else
        // is the detection more reliable for the new one ?
        if(c2->nbPins()==(*comp)->nbPins())
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
#if 1  
    Component *c=NULL;
    probeMe(pin1,pin2,pin3,&c);
    probeMe(pin1,pin3,pin2,&c);
    probeMe(pin2,pin3,pin1,&c);    
    
#else
    TesterGfx::printStatus("Probing");
    Component *c=new NPNBjt(pin2,pin3,pin1);
    //Component *c=new Coil(pin1,pin2,pin3);
    //Component *c=new Capacitor(pin3,pin2,pin1);
#endif
    if(!c)
    {
        xDelay(1000);
        return;
    }
      
    if(c->compute())
    {     
        TesterGfx::clear();
        c->draw(0);
        
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
