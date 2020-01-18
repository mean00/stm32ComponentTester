#include "ST7735_ex.h"
/**
*/
void Adafruit_ST7735Ex::init()
{
    Adafruit_ST7735::initR();    
}
/**
 * 
 * @param CS
 * @param RS
 * @param RST
 */
Adafruit_ST7735Ex::Adafruit_ST7735Ex(int8_t CS, int8_t RS, int8_t RST ) : Adafruit_ST7735(CS,RS,RST)
{
  _width = 128;
  _height = 128;
  rotation = 1;
}

        
  