/*
*/


#include "fancyLock.h"
#include "testPins.h"
#include "allComponents.h"
#include "testerGfx.h"
extern TestPin pin1,pin2,pin3;
extern void pinTest();
#define Y_OFFSET 20
void calibration()
{
    
     pinTest();
    
    int resup3,resdown3;
    int resup1,resdown1;
    int resup2,resdown2;
    pin1.evalInternalResistance (resdown1,resup1);
    pin2.evalInternalResistance (resdown2,resup2);
    pin3.evalInternalResistance (resdown3,resup3);
    
    TesterGfx::clear();
    char st[30];
    TesterGfx::print(10,Y_OFFSET,"Internal Res :");
    sprintf(st,"1: D:%d U:%d",resdown1,resup1);
    TesterGfx::print(10,20+Y_OFFSET,st);
    sprintf(st,"2: D:%d U:%d",resdown2,resup2);
    TesterGfx::print(10,50+Y_OFFSET,st);
    sprintf(st,"3: D:%d U:%d",resdown3,resup3);
    TesterGfx::print(10,80+Y_OFFSET,st);
    
    while(1)
    {
        
    };
}