#include "ST7735_ex.h"
#include "testerGfx.h"
#include "assets.h"
#include "Fonts/waree9.h"
#include "Fonts/FreeSansBold9pt7b.h"
static Adafruit_ST7735Ex *instance=NULL;
#define BASELINE_LAST    126
#define BASELINE_PRELAST (126-18)

/**
 */
void TesterGfx::init()
{
    instance=new Adafruit_ST7735Ex(PA4,PA3,PB0);    
    instance->init();    
    instance->setRotation(2);
    instance->setFontFamily(&Waree9pt7b, &Waree9pt7b, &FreeSansBold9pt7b);  
    instance->setFont(&FreeSansBold9pt7b /*&Waree9pt7b*/);
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


#define CAP_LINE 54
#define CAP_COL1 8
#define CAP_COL2 128-20
#define COMPONENT_COLOR (0x1f<<6)

static void printPins(Adafruit_ST7735Ex *instance, int pinA, int pinB)
{
      instance->setTextColor(0x1f,0);      
      instance->setCursor(CAP_COL1,CAP_LINE);
      instance->print(pinA);
      instance->setCursor(CAP_COL2,CAP_LINE);
      instance->print(pinB);
      instance->setTextColor(0xffff,0);      
}

/**
 * 
 * @param offset
 * @param value
 * @param pinA
 * @param pinB
 */
void TesterGfx::drawCapacitor(int offset, const char *value,int pinA, int pinB)
{

      instance->drawRLEBitmap(cap_width,cap_height,0,0,COMPONENT_COLOR,0,cap);
      printPins(instance,pinA,pinB);
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
void TesterGfx::drawPMosFet(const char *line1, const char *line2, int pinGate, int pinUp, int pinDown)
{
      instance->setCursor(5,BASELINE_PRELAST);
      instance->print(line1);
      instance->setCursor(5,BASELINE_LAST);
      instance->print(line2);
      
}

/**
 * 
 * @param offset
 * @param value
 * @param pinA
 * @param pinB
 */
void TesterGfx::drawCoil(int offset, const char *value,int pinA, int pinB)
{

      instance->drawRLEBitmap(coil_width,coil_height,0,0,COMPONENT_COLOR,0,coil);
      printPins(instance,pinA,pinB);
      instance->setCursor(5,98);
      instance->print("H=");
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

      instance->drawRLEBitmap(resistor_width,resistor_height,0,0,COMPONENT_COLOR,0,resistor);
      printPins(instance,pinA,pinB);
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

      instance->drawRLEBitmap(diode_width,diode_height,0,0,COMPONENT_COLOR,0,diode);
      printPins(instance,pinA,pinB);      
      instance->setCursor(5,98);
      instance->print("Vf=");
      instance->print(value);    
}
