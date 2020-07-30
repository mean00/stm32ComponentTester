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
/**
 */

void wipeCal()
{
    
}
void swInfo()
{
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