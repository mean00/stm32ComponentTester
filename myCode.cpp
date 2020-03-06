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
#define LED PC13
void myLoop(void);
extern void calibration();
uint32_t  deviceId;

uint32_t  memDensity=0;
uint32_t cr2;
//
int result[20];
int z;
extern void  pinTest();
//
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
//
TestPin   pin1(1,PA2, PA8,  PB5, PB12,PB4,303800,20110,470);  //TestPin(int pinNo, int pinADC, int pinVolt int pinDriveHighRes, int pinDriveMed,int pinDriveLow, int hiRes, int medRes,int lowRes);
TestPin   pin2(2,PA1, PA9,  PB7, PB13,PB6,303300,20100,470);
TestPin   pin3(3,PA0, PA10, PB9, PB14,PB8,303000,20130,468);
WavRotary rotary(PB10,PB11); // PB1,PB10,PB11, PB1 is the button

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
    int  rot=0;
    int  c=0;
    char st[32];
    xDelay(100);
    while(1)
    {
        rot+=rotary.getCount();        
        sprintf(st,"%d-%d",rot,c);
        TesterGfx::clear();
        TesterGfx::print(20,20,st);
        z=micros();
        TesterGfx::print(2,60,"TEST STRING"); // takes 0.3 ms
        z=micros()-z;
        c++;
        //xDelay(200);
    }
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
  
  xTaskCreate( MainTask, "MainTask", 750,NULL, 10, NULL );   
  vTaskStartScheduler();        
}

/**
 * 
 */
void myLoop(void)
{
    COMPONENT_TYPE type;
#if 0   
    Component *c=Component::identity(pin1,pin2,pin3,type);
#else
    TesterGfx::printStatus("Probing");
    Component *c=new NMosFet(pin1,pin2,pin3);
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
