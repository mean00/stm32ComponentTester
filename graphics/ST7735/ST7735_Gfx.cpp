#include "ST7735_ex.h"
#include "testerGfx.h"
#include "assets.h"
#include "Fonts/waree9.h"
#include "Fonts/waree12.h"
static Adafruit_ST7735Ex *instance=NULL;

/**
 */
void TesterGfx::init()
{
    instance=new Adafruit_ST7735Ex(PA4,PA3,PB0);
    instance->init();    
    instance->setFontFamily(&Waree9pt7b, &Waree9pt7b, &Waree12pt7b);  
    instance->fillScreen(0);
    instance->drawRLEBitmap(128,96,0,0,0xffff,0,splash);
}
/**
 * 
 */
void TesterGfx::clear()
{
    instance->fillScreen(0);
}
/**
 */
void TesterGfx::print(int x, int y, const char *txt)
{
    
    instance->setCursor(x,y);
    instance->print(txt);
};
/**
 * 
 * @param offset
 * @param value
 * @param pinA
 * @param pinB
 */
void TesterGfx::drawCapacitor(int offset, const char *value,int pinA, int pinB)
{
      instance->drawRLEBitmap(128,68,0,0,0xffff,0,cap);
      instance->setCursor(20,48);
      instance->print(pinA);
      instance->setCursor(128-20-20,48);
      instance->print(pinB);
      instance->setCursor(50,100);
      instance->print(value);
}