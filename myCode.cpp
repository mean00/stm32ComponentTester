/*
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "resistor.h"
#include "Capacitor.h"
#include "Diode.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "wav_irotary.h"
#define LED PC13
void myLoop(void);
DSOADC *adc=NULL;
//
int result[20];
int z;
//
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
//
TestPin   pin1(1,PA2, PB5, PB12,PB4,303800,20110,470);  //TestPin(int pinNo, int pin, int pinDriveHighRes, int pinDriveMed,int pinDriveLow, int hiRes, int medRes,int lowRes);
TestPin   pin2(2,PA1, PB7, PB13,PB6,303300,20100,470);
TestPin   pin3(3,PA0, PB9, PB14,PB8,303000,20130,468);
WavRotary rotary(PB10,PB11); // PB1,PB10,PB11

void MainTask( void *a )
{
  
    TesterGfx::init();
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
        TesterGfx::clear();
        sprintf(st,"%d-%d",rot,c);
        TesterGfx::print(20,20,st);
        z=micros();
        TesterGfx::print(2,60,"TEST STRING"); // takes 0.3 ms
        z=micros()-z;
        c++;
        xDelay(200);
    }
#endif
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

  
  adc=new DSOADC(PA0);
  adc->setupADCs();

  
  
  xTaskCreate( MainTask, "MainTask", 750,NULL, 10, NULL );   
  vTaskStartScheduler();      
  
}

/**
 * 
 */
void myLoop(void)
{
    //ucg->clearScreen(); 
    Capacitor r(pin1,pin2,pin3);
    //Resistor r(pin1,pin2,pin3);
    //Diode r(pin1,pin2,pin3);
    while(1)
    {        
        if(r.compute())
        {     
            TesterGfx::clear();
            r.draw(0);
        }
        xDelay(500);
    } 
}
