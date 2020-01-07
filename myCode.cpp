/*
*/

#include <SPI.h>
#include "Ucglib.h"
#include "fancyLock.h"
#include "testPins.h"
#include "resistor.h"
#define LED PC13

void myLoop(void);
/*
 LCD : PA3, PA4, PA2 + PA5/Ä7
 */

Ucglib_ST7735_18x128x160_HWSPI *ucg=NULL;
//nt pinNo, int pin, int pinDriveHighRes, int pinDriveLow
TestPin   pin1(1,PA0, PB5, PB4,468,301000);
TestPin   pin2(2,PA1, PB7, PB6,471,302000);
TestPin   pin3(3,PA2, PB8, PB3,471,302000);

void MainTask( void *a )
{
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
  
 
  interrupts();
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); // Set the SPI bit order
  SPI.setDataMode(SPI_MODE0); //Set the  SPI data mode 0
  SPI.setClockDivider (SPI_CLOCK_DIV4); // Given for 10 Mhz...
  afio_cfg_debug_ports( AFIO_DEBUG_SW_ONLY); // get PB3 & PB4
  pin1.init();
  pin2.init();
  
  ucg=new Ucglib_ST7735_18x128x160_HWSPI(/*cd=*/ PA3, /*cs=*/ PA4, /*reset=*/ PA2);
  ucg->begin(UCG_FONT_MODE_TRANSPARENT); //UCG_FONT_MODE_SOLID);
  ucg->setFont(ucg_font_helvB12_hr);
  ucg->clearScreen();    
  ucg->setColor(255, 255, 255);
  
  xTaskCreate( MainTask, "MainTask", 250, NULL, 10, NULL );   
  vTaskStartScheduler();      
  
}

/**
 * 
 */
void myLoop(void)
{
    char st[10];    

 // delay(100);
 
  for(int i=0;i<16;i++)
     ucg->drawHLine(1, 10*i, 120);

 #if 1      
    while(1)
    {
    ucg->clearScreen(); 
    Resistor r(pin2,pin1,pin3);
    if(r.compute())
    {
        r.draw(ucg,0);
    }
     delay(3000);
    }
 
  #endif     
#if 0
    int p=PB4;
    pinMode(p,OUTPUT);
    while(1)
    {
        digitalWrite(p,1);
        delay(1000);
        digitalWrite(p,0);
        delay(1000);
    }
#endif
#if 0
    
#define DO_PIN(pin, txt, call) \
        ucg->clearScreen(); \
        ucg->drawString(10,30,0,#pin); \
        ucg->drawString(10,60,0,txt); \
        pin.call  ;\
        delay(5*1000);
#define pin pin1
    DO_PIN(pin,"Disconnect",disconnect())
    DO_PIN(pin,"Ground",setToGround())
    DO_PIN(pin,"Vcc",setToVcc())
    DO_PIN(pin,"PDown -L",pullDown(false))
    DO_PIN(pin,"PUp -H",pullUp(true))
    DO_PIN(pin,"PDown -H",pullDown(true))
    DO_PIN(pin,"PUp -L",pullUp(false))
#endif
 
}
