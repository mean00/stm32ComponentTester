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
    instance->setFont(&Waree9pt7b);
    instance->fillScreen(0x0);  
    
    instance->setFontSize(Adafruit_ST7735Ex::MediumFont);
    instance->setTextColor(0xFFFF,0);    
    instance->setCursor(4,30);
    instance->print("Component");
    instance->setCursor(4,50);
    instance->print("   Tester");

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
#define CAP_COL (128-20-10)
      instance->drawRLEBitmap(cap_width,cap_height,0,0,0x1fff,0,cap);
      instance->setCursor(CAP_COL,16);
      instance->print(pinA);
      instance->setCursor(CAP_COL,60);
      instance->print(pinB);
      instance->setCursor(5,98);
      instance->print("C=");
      instance->print(value);
}

/**
 * 
 * @param offset
 * @param value
 * @param pinA
 * @param pinB
 */
void TesterGfx::drawResistor(int offset, const char *value,int pinA, int pinB)
{
#define CAP_COL (128-20-10)
      instance->drawRLEBitmap(resistor_width,resistor_height,0,0,0x1fff,0,resistor);
      instance->setCursor(CAP_COL,16);
      instance->print(pinA);
      instance->setCursor(CAP_COL,60);
      instance->print(pinB);
      instance->setCursor(5,98);
      instance->print("R=");
      instance->print(value);
}
/**
 * 
 * @param offset
 * @param value
 * @param pinA
 * @param pinB
 */
void TesterGfx::drawDiode(int offset, const char *value,int pinA, int pinB)
{
#define CAP_COL (128-20-10)
      instance->drawRLEBitmap(resistor_width,resistor_height,0,0,0x1fff,0,resistor);
      instance->setCursor(CAP_COL,16);
      instance->print(pinA);
      instance->setCursor(CAP_COL,60);
      instance->print(pinB);
      instance->setCursor(5,98);
      instance->print("V=");
      instance->print(value);
    
}