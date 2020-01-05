/*
*/

#include <SPI.h>
#include "Ucglib.h"
#include "fancyLock.h"
#include "testPins.h"
#define LED PC13

/*
 LCD : PA3, PA4, PA2 + PA5/Ä7
 */

Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ PA3, /*cs=*/ PA4, /*reset=*/ PA2);

TestPin   pin1(1,PA0, PB5, PB4);
TestPin   pin2(2,PA1, PB7, PB6);

/**
 * 
 */
void mySetup(void)
{
  FancyInterrupts::enable();
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); // Set the SPI bit order
  SPI.setDataMode(SPI_MODE0); //Set the  SPI data mode 0
  SPI.setClockDivider (SPI_CLOCK_DIV4); // Given for 10 Mhz...
  delay(100);
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.clearScreen();
  ucg.setColor(255, 255, 255);
  for(int i=0;i<16;i++)
     ucg.drawHLine(1, 10*i, 120);

}

/**
 * 
 */
void myLoop(void)
{
   
}
