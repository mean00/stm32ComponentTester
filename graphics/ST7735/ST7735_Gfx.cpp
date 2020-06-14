#include "ST7735_ex.h"
#include "testerGfx.h"
#include "assets.h"
#include "cpuID.h"
#include "testerVersion.h"
#include "Component.h"
#include "pinConfiguration.h"
#include "testerControl.h"

static Adafruit_ST7735Ex *instance=NULL;

#define INTERLINE 15
#define BASELINE_LAST    126
#define BASELINE_PRELAST (BASELINE_LAST-INTERLINE)
#define BASELINE_PRELAST2 (BASELINE_LAST-INTERLINE*2)
#define BASELINE_PRELAST3 (BASELINE_LAST-INTERLINE*3)


#define CAP_LINE 54
#define CAP_COL1 8
#define CAP_COL2 128-20
#define COMPONENT_COLOR (0x1f<<6)

#define FONT1 FreeSans7pt7b
#define FONT2 FreeSansBold9pt7b

extern const GFXfont  FONT1  ;
extern const GFXfont  FONT2  ;
//const GFXfont  FreeSansBold9pt7b 


static void print3Pins(Adafruit_ST7735Ex *instance, int pinA, int pinB, int pinC);

/**
 * 
 * @param title
 */
void xtitle(const char *title) 
{
   // guestimate width
    int l=strlen(title);
    l*=11;
    int center=(128-l)/2;
    if(center<0) center=0;
    instance->setCursor(center,INTERLINE);
    instance->print(title) ;
}


void simple3Pole(int offset, const char *title, int pinA,int pinB,int pinC, int w, int h, const uint8_t *data)
{
    TesterGfx::clear();

    int mid=(86-h)/2;    
    if(h>86) mid=0;
    int center=(128-w)/2;
    if(center<0) center=0;
    instance->drawRLEBitmap(w,h,center,mid+INTERLINE,COMPONENT_COLOR,0,data);
    xtitle(title);      
    print3Pins(instance,pinA,pinB,pinC);
}


/**
 */
void TesterGfx::init()
{
    instance=new Adafruit_ST7735Ex(PIN_ST7735_CS,PIN_ST7735_RS,PIN_ST7735_RST);    
    instance->init();    
    instance->setRotation(2);
    instance->setFontFamily(&FONT1, &FONT2, &FONT2);  
    instance->setFont(&FONT2 /*&Waree9pt7b*/);
    instance->fillScreen(0x0);      
    instance->setFontSize(Adafruit_ST7735Ex::MediumFont);
    instance->setTextColor(0xFFFF,0);    
}
/**
 */
void TesterGfx::splash()
{
#define INC 23
#define LINE(x) (23+INC*x)
    instance->fillScreen(0x0);
    instance->setCursor(10,LINE(0));
    instance->print("Component");
    instance->setCursor(20,LINE(1));
    instance->print("Tester");    
    instance->setCursor(40,LINE(2));        
    instance->print( TESTER_VERSION );
    
    
    instance->setFontSize(Adafruit_ST7735Ex::SmallFont);
    instance->setCursor(0,LINE(4)-4);        
    instance->print( TESTER_CONFIGURATION );
    
    instance->setCursor(0,LINE(4)+12);
    instance->print(cpuID::getIdAsString());
    instance->setFontSize(Adafruit_ST7735Ex::MediumFont);    
    xDelay(2000);
    instance->fillScreen(0x0);
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


static void print2Pins(Adafruit_ST7735Ex *instance, int pinA, int pinB, int line)
{
      instance->setTextColor(0x1f,0);      
      instance->setCursor(CAP_COL1,line);
      instance->print(pinA);
      instance->setCursor(CAP_COL2,line);
      instance->print(pinB);
      instance->setTextColor(0xffff,0);      
}

#define P_LEFT_X (30)
#define P_MID_Y ((128-20)/2+8)

#define P_RIGHT_X   ((128/2)+20)
#define P_TOP_Y     (20+12)
#define P_BOTTOM_Y  (128-33)




// Left Bottom Up
void print3Pins(Adafruit_ST7735Ex *instance, int pinA, int pinB, int pinC)
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


void TesterGfx::drawNPN(float hfe, float vf,int base, int emitter,int collector)
{
    char st[64];

   simple3Pole(0, "NPN", base,emitter,collector, NPN_width,NPN_height, NPN);

   instance->setCursor(5,BASELINE_PRELAST);
   Component::prettyPrintPrefix("hfe:",hfe, "",st);      
   instance->print(st);

   instance->setCursor(5,BASELINE_LAST);
   Component::prettyPrintPrefix("Vbe:",vf, "V",st);      
   instance->print(st);      
}
/**
 * 
 * @param hfe
 * @param vf
 * @param base
 * @param emitter
 * @param collector
 */
void TesterGfx::drawPNP(float hfe, float vf,int base, int emitter,int collector)
{
      char st[64];
      
      simple3Pole(0, "PNP", base,emitter,collector, PNP_width,PNP_height, PNP);
      
      instance->setCursor(5,BASELINE_PRELAST);
      Component::prettyPrintPrefix("hfe:",hfe, "",st);      
      instance->print(st);
      
      instance->setCursor(5,BASELINE_LAST);
      Component::prettyPrintPrefix("Vbe:",vf, "V",st);      
      instance->print(st);
      
}
void drawMosfetInfo(Adafruit_ST7735Ex *instance, int page, float RdsOn, float Cg, float VfOn,float Vdiode)
{
    char st[64];
      instance->setFontSize(Adafruit_ST7735Ex::SmallFont);
      instance->fillRect(0,BASELINE_PRELAST2,128,128,0x0);
      if(!page)
      {
        instance->setCursor(5,BASELINE_PRELAST);
        Component::prettyPrintPrefix("RdsOn:",RdsOn, "O",st);      
        instance->print(st);

        instance->setCursor(5,BASELINE_LAST);
        Component::prettyPrintPrefix("Diode:",Vdiode, "V",st);      
        instance->print(st);
      }else
      {
        instance->setCursor(5,BASELINE_PRELAST);
        Component::prettyPrintPrefix("Cg:",Cg, "F",st);      
        instance->print(st);

        instance->setCursor(5,BASELINE_LAST);
        Component::prettyPrintPrefix("Vt:",VfOn, "V",st);      
        instance->print(st);     
      }
      instance->setFontSize(Adafruit_ST7735Ex::MediumFont);          
}
/**
 * 
 * @param offset
 * @param value
 * @param pinA
 * @param pinB
 */
void TesterGfx::drawPMosFet( int pinGate, int pinUp, int pinDown)
{
    simple3Pole(0, "P Mos", pinGate,pinUp,pinDown, Pmosfet_width,Pmosfet_height, Pmosfet);
}
/**
 * 
 * @param offset
 * @param value
 * @param pinA
 * @param pinB
 */
void TesterGfx::drawNMosFet( int pinGate, int pinUp, int pinDown)
{
      
      simple3Pole(0, "N Mos", pinGate,pinDown,pinUp, Nmosfet2_width,Nmosfet2_height, Nmosfet2);        
}
/**
 * 
 * @param RdsOn
 * @param Cg
 * @param VfOn
 * @param Vdiode
 */
void TesterGfx::drawMosInfo(int page, float RdsOn, float Cg, float VfOn, float Vdiode)
{
    drawMosfetInfo(instance, page, RdsOn,   Cg,   VfOn,   Vdiode);     
}

void simple2Pole(int offset, const char *title,const char *value, const char *prefix, int pinA,int pinB, int w, int h, const uint8_t *data)
{
    TesterGfx::clear();

    int mid=(86-h)/2;    
    if(h>86) mid=0;
    int center=(128-w)/2;
    if(center<0) center=0;
    instance->drawRLEBitmap(w,h,center,mid+INTERLINE,COMPONENT_COLOR,0,data);
    xtitle(title);      
    print2Pins(instance,pinA,pinB,mid+INTERLINE+(h/2));
    instance->setCursor(5,BASELINE_PRELAST);
    instance->print(prefix);
    instance->print(value);
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
    simple2Pole(offset,"Coil",value,"H=",pinA,pinB,coil_width,coil_height,coil);
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
    simple2Pole(offset,"Resistor",value,"R=",pinA,pinB,resistor_width,resistor_height,resistor);
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
    simple2Pole(offset,"Diode",value,"Vf=",pinA,pinB,diode_width,diode_height,diode);   
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
    simple2Pole(offset,"Cap",value,"C=",pinA,pinB,cap_width,cap_height,cap);   
}
/**
 * 
 * @param status
 */
void TesterGfx::printStatus(const char *status)
{
    Serial.println(status);
}
/**
 * 
 * @param nb
 * @param data
 */
void TesterGfx::drawCurve(int nb, uint16_t *data)
{
    instance->fillScreen(0);
    int n=nb/128; // multiplier
    for(int i=0;i<128;i++)
    {
        float x=data[i*n];
        x=x/32.;
        instance->putPixel(i,128-((int)x),0xfffff);
        instance->putPixel(i,128-i,0xff);
    }
    
}
/**
 * 
 */
void TesterGfx::test()
{
    splash();
    TesterControl::waitForAnyEvent();
    drawDiode(0,"'600mV",1,2);
    TesterControl::waitForAnyEvent();
    drawResistor(0,"'50kO",1,2);
    TesterControl::waitForAnyEvent();
}