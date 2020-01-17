/*
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "resistor.h"
#include "Capacitor.h"
#include "dso_adc.h"
#define LED PC13
int z;
void myLoop(void);
DSOADC *adc;
/*
 LCD : PA3, PA4, PA2 + PA5/Ä7
 */
int result[20];
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

 //TestPin(int pinNo, int pin, int pinDriveHighRes, int pinDriveMed,int pinDriveLow, int hiRes, int medRes,int lowRes);

TestPin   pin1(1,PA2, PB5, PB12,PB4,303800,20110,470);
TestPin   pin2(2,PA1, PB7, PB13,PB6,303300,20100,470);
TestPin   pin3(3,PA0, PB9, PB14,PB8,303000,20130,468);


void MainTask( void *a )
{
#if 0
    ucg=new Ucglib_ST7735_18x128x160_HWSPI(/*cd=*/ PA3, /*cs=*/ PA4, /*reset=*/ PB0);
    ucg->begin(UCG_FONT_MODE_TRANSPARENT); //UCG_FONT_MODE_SOLID);
    ucg->setFont(ucg_font_helvB12_hr);
    ucg->clearScreen();    
    ucg->setColor(255, 255, 255);
    ucg->clearScreen();    
#endif
    pin1.init();
    pin2.init();
    pin3.init();

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
  
  adc=new DSOADC(PA0);
  adc->setupADCs();

  
  xTaskCreate( MainTask, "MainTask", 500, NULL, 10, NULL );   
  vTaskStartScheduler();      
  
}

/**
 * 
 */
void myLoop(void)
{
    //ucg->clearScreen(); 
    Capacitor r(pin1,pin2,pin3);
    while(1)
    {        
        if(r.compute())
        {
     //       ucg->clearScreen(); 
            //r.draw(ucg,0);
        }
        xDelay(500);
    } 
}
