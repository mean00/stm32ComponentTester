#include "ST7735_ex.h"
#include "testerGfx.h"

static Adafruit_ST7735Ex *instance=NULL;

/**
 */
void TesterGfx::init()
{
    instance=new Adafruit_ST7735Ex(PA4,PA3,PB0);
    instance->initR();
    instance->setRotation(2);
    instance->fillScreen(0x1f<<11);
    instance->print("Hi!");            
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
