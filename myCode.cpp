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
#define LED PC13
void myLoop(void);
extern void pinTest();
uint32_t  deviceId;


//
int result[20];
int z;
//
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
//
TestPin   pin1(1,PA2, PA8,  PB5, PB12,PB4,303800,20110,470);  //TestPin(int pinNo, int pinADC, int pinVolt int pinDriveHighRes, int pinDriveMed,int pinDriveLow, int hiRes, int medRes,int lowRes);
TestPin   pin2(2,PA1, PA9,  PB7, PB13,PB6,303300,20100,470);
TestPin   pin3(3,PA0, PA10, PB9, PB14,PB8,303000,20130,468);
WavRotary rotary(PB10,PB11); // PB1,PB10,PB11, PB1 is the button

void MainTask( void *a )
{
  
    TesterGfx::init();
 
    TestPin::initADC(PA0);
    
    
    pin1.init();
    pin2.init();
    pin3.init();
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
    
  //  pinTest();
    
    
    
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
  
  Serial.begin(38400);
  interrupts();

  delay(100);


  
  
  xTaskCreate( MainTask, "MainTask", 750,NULL, 10, NULL );   
  vTaskStartScheduler();      
  
}

/**
 * 
 */
void myLoop(void)
{
    COMPONENT_TYPE type;
#if 1    
    Component *c=Component::identity(pin3,pin2,pin1,type);
#else
    Component *c=new Coil(pin3,pin2,pin1);
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
    xDelay(2000);
}
