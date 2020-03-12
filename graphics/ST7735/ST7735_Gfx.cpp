#include "ST7735_ex.h"
#include "testerGfx.h"
#include "assets.h"
#include "Fonts/FreeSansBold9pt7b.h"
#include "Fonts/FreeSans7pt7b.h"
#include "cpuID.h"
#include "testerVersion.h"
#include "Component.h"
static Adafruit_ST7735Ex *instance=NULL;

#define INTERLINE 15
#define BASELINE_LAST    126
#define BASELINE_PRELAST (BASELINE_LAST-INTERLINE)
#define BASELINE_PRELAST2 (BASELINE_LAST-INTERLINE*2)
#define BASELINE_PRELAST3 (BASELINE_LAST-INTERLINE*3)

/**
 */
void TesterGfx::init()
{
    instance=new Adafruit_ST7735Ex(PA4,PA3,PB0);    
    instance->init();    
    instance->setRotation(2);
    instance->setFontFamily(&FreeSans7pt7b, &FreeSansBold9pt7b, &FreeSansBold9pt7b);  
    instance->setFont(&FreeSansBold9pt7b /*&Waree9pt7b*/);
    instance->fillScreen(0x0);  
    
    instance->setFontSize(Adafruit_ST7735Ex::MediumFont);
    instance->setTextColor(0xFFFF,0);    
    instance->setCursor(4,30);
    instance->print("Component");
    instance->setCursor(4,50);
    instance->print("    Tester");
    
    instance->setCursor(36,70);        
    instance->print( TESTER_VERSION );
    
    
    instance->setFontSize(Adafruit_ST7735Ex::SmallFont);
    instance->setCursor(0,96);
    instance->print(cpuID::getIdAsString());
    instance->setFontSize(Adafruit_ST7735Ex::MediumFont);
    
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

#define P_LEFT_X (8)
#define P_MID_Y ((128-20)/2)

#define P_RIGHT_X   ((128/2)+10)
#define P_TOP_Y     (20+20)
#define P_BOTTOM_Y  (128-24)




// Left Bottom Up
static void print3Pins(Adafruit_ST7735Ex *instance, int pinA, int pinB, int pinC)
{
      instance->setTextColor(0x1f,0);      
      instance->setCursor(P_LEFT_X,P_MID_Y);
      instance->print(pinA);
      
      instance->setCursor(P_RIGHT_X,P_BOTTOM_Y);
      instance->print(pinB);

      instance->setCursor(P_RIGHT_X,P_TOP_Y);
      instance->print(pinC);
            
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
void TesterGfx::drawPMosFet(float RdsOn, float Cg, float VfOn, float Vdiode, int pinGate, int pinUp, int pinDown)
{
    char st[64];
    
      instance->drawRLEBitmap(Pmosfet_width,Pmosfet_height,0,0,COMPONENT_COLOR,0,Pmosfet);
    
      instance->setFontSize(Adafruit_ST7735Ex::SmallFont);
      
      instance->setCursor(5,BASELINE_PRELAST2);
      Component::prettyPrintPrefix("RdsOn:",RdsOn, "O",st);      
      instance->print(st);
      
      instance->setCursor(5,BASELINE_PRELAST);
      Component::prettyPrintPrefix("Diode:",Vdiode, "V",st);      
      instance->print(st);
      
      instance->setCursor(5,BASELINE_LAST);
      Component::prettyPrintPrefix("Cg:",Cg, "F",st);      
      instance->print(st);
      
      instance->setCursor(5,BASELINE_PRELAST3);
      Component::prettyPrintPrefix("Vt:",VfOn, "V",st);      
      instance->print(st);           
}
void TesterGfx::drawNPN(float hfe, float vf,int base, int emitter,int collector)
{
      char st[64];
      
     
      instance->drawRLEBitmap(NPN_width,NPN_height,0,INTERLINE,COMPONENT_COLOR,0,NPN);
      
         print3Pins(instance,base, emitter,collector);
      
      instance->setCursor(5,BASELINE_PRELAST);
      Component::prettyPrintPrefix("hfe:",hfe, "",st);      
      instance->print(st);
      
      instance->setCursor(5,BASELINE_LAST);
      Component::prettyPrintPrefix("Vbe:",vf, "V",st);      
      instance->print(st);
      
}

void TesterGfx::drawPNP(float hfe, float vf,int base, int emitter,int collector)
{
      char st[64];
      
     
      instance->drawRLEBitmap(PNP_width,PNP_height,0,INTERLINE,COMPONENT_COLOR,0,PNP);
      
      
      print3Pins(instance,base, emitter,collector);
      
      instance->setCursor(5,BASELINE_PRELAST);
      Component::prettyPrintPrefix("hfe:",hfe, "",st);      
      instance->print(st);
      
      instance->setCursor(5,BASELINE_LAST);
      Component::prettyPrintPrefix("Vbe:",vf, "V",st);      
      instance->print(st);
      
}
/**
 * 
 * @param offset
 * @param value
 * @param pinA
 * @param pinB
 */
void TesterGfx::drawNMosFet(float RdsOn, float Cg, float VfOn, float Vdiode, int pinGate, int pinUp, int pinDown)
{
    char st[64];
    
      instance->drawRLEBitmap(Nmosfet2_width,Nmosfet2_height,0,0,COMPONENT_COLOR,0,Nmosfet2);
      instance->setFontSize(Adafruit_ST7735Ex::SmallFont);
      
      instance->setCursor(5,BASELINE_PRELAST2);
      Component::prettyPrintPrefix("RdsOn:",RdsOn, "O",st);      
      instance->print(st);
      
      instance->setCursor(5,BASELINE_PRELAST);
      Component::prettyPrintPrefix("Diode:",Vdiode, "V",st);      
      instance->print(st);
      
      instance->setCursor(5,BASELINE_LAST);
      Component::prettyPrintPrefix("Cg:",Cg, "F",st);      
      instance->print(st);
      
      instance->setCursor(5,BASELINE_PRELAST3);
      Component::prettyPrintPrefix("Vt:",VfOn, "V",st);      
      instance->print(st);           
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
/**
 * 
 * @param status
 */
void TesterGfx::printStatus(const char *status)
{
    instance->setCursor(5,16);
    instance->fillRect(0,16-16,128,16,0);
    instance->print(status);
}