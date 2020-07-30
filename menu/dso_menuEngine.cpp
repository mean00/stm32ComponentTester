/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "Arduino.h"
#include "dso_menuEngine.h"
#include "testerControl.h"
#include "testerGfx.h"
#include "MapleFreeRTOS1000_pp.h"

#define INTERLINE 20
#define BG_COLOR GREEN   
/**
 * 
 * @param menu
 */
MenuManager::MenuManager(const MenuItem *menu)
{
    _menu=menu;
}
/**
 * 
 */
MenuManager::~MenuManager()
{
    _menu=NULL;
}
/**
 * 
 * @param onoff
 * @param line
 * @param text
 * @return 
 */
void MenuManager::printMenuEntry(bool onoff, int line,const char *text)
{
     
    //if(onoff)
     //   tft->setTextColor(BLACK,BG_COLOR); 
    //else  
      //  tft->setTextColor(BG_COLOR,BLACK);
    TesterGfx::print(10,10+INTERLINE*line,text);
}
void MenuManager::printMenuTitle(const char *text)
{
    //tft->setTextColor(GREEN,BLACK); 
    TesterGfx::print(10,10+INTERLINE,text);  
}
/**
 * 
 */
void MenuManager::run(void)
{
     TesterGfx::clear();
     runOne(_menu);     
};
/**
 */
void MenuManager::redraw(const char *title, int n,const MenuItem *xtop, int current)
{
    TesterGfx::clear();
    printMenuTitle(title); 
    for(int i=0;i<n;i++)
    {
        printMenuEntry(current==i,i,xtop[i].menuText);
    }        
}

void MenuManager::blink(int current, const char *text)
{
    for(int i=0;i<5;i++)
    {
           printMenuEntry(false,current,text);
           xDelay(80);
           printMenuEntry(true,current,text);
           xDelay(80);
    }     
}

/**
 */
void MenuManager::runOne( const MenuItem *xtop)
{
     TesterGfx::clear();
     const char *title=xtop->menuText;
     xtop=xtop+1;
     int n=0;
     {
        const MenuItem  *top=xtop;
        while(top->type!=MenuItem::MENU_END)
        {
            top++;
            n++;
        }
     }
     
     
     
     // draw them 
     // 0 to N-1
     int current=0;    
next:         
        redraw(title,n,xtop,current);
        while(1)
        { 
                  xDelay(10); // dont busy loop
                  int event=  TesterControl::waitForEvent();
                  if( event & CONTROL_LONG)
                    return;
                  if(event & CONTROL_SHORT)
                  {
                   //   Serial.print("Menu \n");
                   //   Serial.print(xtop[current].type);
                      switch(xtop[current].type)
                      {
                      case MenuItem::MENU_BACK: return; break;
                      case MenuItem::MENU_KEY_VALUE:  break;
                      case MenuItem::MENU_SUBMENU: 
                            {
                                const MenuItem *sub=(const MenuItem *)xtop[current].cookie;
                                runOne(sub);
                                goto next;
                            }
                            break;
                      case MenuItem::MENU_CALL: 
                            {
                                typedef void cb(void);
                                cb *c=(cb *)xtop[current].cookie;
                                blink(current,xtop[current].menuText);
                                c();
                                goto next;
                            }
                          break;
                      case MenuItem::MENU_END: return;break;
                      case MenuItem::MENU_TOGGLE: break;
                      default: xAssert(0);break;
                      }
                      break;
                  }
                  int inc=TesterControl::getRotary();
                  if(inc)
                  {
                    current+=inc;
                    while(current<0) current+=n;
                    while(current>=n) current-=n;
                    goto next;
                  }
        }
};
// EOF