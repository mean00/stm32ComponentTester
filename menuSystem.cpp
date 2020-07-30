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

void wipeCal()
{
     NVM::reset();
}
void swInfo()
{
     TesterGfx::clear();
     TesterGfx::topLine("Info");
     
     char st[20];
     sprintf(st,"v%2.2f",TESTER_VERSION);
          
     TesterGfx::print(10,40,st);
     TesterGfx::print(10,80,cpuID::getIdAsString());
     
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
     TesterGfx::print(10,60,build);
     TesterGfx::bottomLine("Press Ok");
     
     TesterControl::waitForAnyEvent();
     
     
}

const MenuItem  topMenu[]={
    {MenuItem::MENU_TITLE, "Main Menu",NULL},
    {MenuItem::MENU_CALL, "Wipe cal",(const void *)&wipeCal},
    {MenuItem::MENU_CALL, "Info",(const void *)swInfo},
    {MenuItem::MENU_END, NULL,NULL}
};




void menuSystem(void)
{
     const MenuItem *tem=topMenu;
     MenuManager man(tem);
     man.run();
    return;
}