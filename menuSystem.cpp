/*
*/

#include "../../SPI/src/SPI.h"
#include "fancyLock.h"
#include "testPins.h"
#include "allComponents.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "testerControl.h"
#include "componentSignature.h"
#include "cpuID.h"
#include "pinConfiguration.h"
#include "dso_menuEngine.h"
#include "testerVersion.h"
/**
 */
extern TestPin   pin1,pin2,pin3;

void wipeCal()
{
     NVM::reset();
}
/**
 * 
 */
void swInfo()
{
     TesterGfx::clear();
     TesterGfx::topLine("Info");
     
     char st[20];
     sprintf(st,"v%2.2f",TESTER_VERSION);
          
     TesterGfx::printSmall(10,40,st);
     TesterGfx::printSmall(10,80,cpuID::getIdAsString());
     
     const char *build="??";
     
#ifdef  MCU_GD32F103C8
     build="GD32F1";
#elif      MCU_GD32F303C8
     build="GD32F3";
#elif MCU_STM32F103C8
     build="STM32F1";
#elif
#error 
#endif
     TesterGfx::printSmall(10,60,build);
     TesterGfx::bottomLine("Press Ok");
     
     TesterControl::waitForAnyEvent();
}
/**
 * 
 */

static void fmtLine(const char *patt,int up, int down, int line)
{
   char st[60];
   sprintf(st,patt,up, down);
   TesterGfx::print(1,line,st);
}

#define ZINTER(x) (32+18*x)
void calibrationInfo()
{
    
    
    TesterGfx::clear();
    TesterGfx::topLine("Calibration-R");     

     //#define ALLRES(x) {sprintf(st,"RU:%d:RD:%d",pin##x._calibration.resUp,pin##x._calibration.resUp);TesterGfx::print(1,ZINTER(x-1),st);}
     #define ALLRES(x) {fmtLine("RU:%d:RD:%d",pin##x._calibration.resUp,pin##x._calibration.resUp,ZINTER(x-1));;}
    
    ALLRES(1);
    ALLRES(2);
    ALLRES(3);     
    TesterGfx::bottomLine("Press Ok");
    TesterControl::waitForAnyEvent();
    
    TesterGfx::clear();
    TesterGfx::topLine("Calibration-C");     
    #undef ALLRES
    //#define ALLRES(x) {sprintf(st,"Ch:%d:Cl:%d",pin##x._calibration.capOffsetInPf,pin##x._calibration.capOffsetHighInPfMu16[0]/16);TesterGfx::print(1,ZINTER(x-1),st);}
    #define ALLRES(x) {fmtLine("Ch:%d:Cl:%d",pin##x._calibration.capOffsetInPf,pin##x._calibration.capOffsetHighInPfMu16[0]/16,ZINTER(x-1));}
    
    ALLRES(1);
    ALLRES(2);
    ALLRES(3);     
    TesterGfx::bottomLine("Press Ok");
    TesterControl::waitForAnyEvent();
    
}     

/**
 * 
 */

#define SHOW(x) {TesterGfx::x;TesterControl::waitForAnyEvent();}

void showIcons()
{
  SHOW(drawDiode(0, "x",1,3));
  SHOW(drawCoil(0, "x",1,2));
  SHOW(drawCapacitor(0, "x",1,2));
  SHOW(drawResistor(0, "x",1,2));
  SHOW(drawPMosFet( 1,2,3));
  SHOW(drawNMosFet(1,2,3));
  SHOW(drawMosInfo(0, 1.0, 1.0,1.0, 1.0));
  SHOW(drawNPN(100,1,1,2,3));
  SHOW(drawPNP(100,1,1,2,3));      
}
/**
 */
const MenuItem  topMenu[]={
    {MenuItem::MENU_TITLE, "Main Menu",NULL},
    {MenuItem::MENU_CALL, "Info",(const void *)swInfo},
    {MenuItem::MENU_CALL, "Icons",(const void *)showIcons},
    {MenuItem::MENU_CALL, "Show cal.",(const void *)calibrationInfo},    
    {MenuItem::MENU_CALL, "Wipe cal",(const void *)&wipeCal},    
    {MenuItem::MENU_END, NULL,NULL}
};



/**
 * 
 */
void menuSystem(void)
{
     const MenuItem *tem=topMenu;
     // clear events if any
     TesterControl::clearEvent();
     TesterControl::clearEvent();
     MenuManager man(tem);
     man.run();
    return;
}