#include "Arduino.h"
#include "fancyLock.h"
#include "testPins.h"
#include "allComponents.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "testerControl.h"
#include "componentSignature.h"
#include "cpuID.h"
#include "pinConfiguration.h"
#include "waveForm.h"
#include "tester.h"

extern TestPin pin1,pin2,pin3;

Tester::Tester()
{
    pinTable[0]=&pin1;
    pinTable[1]=&pin2;
    pinTable[2]=&pin3;
    pinTable[3]=&pin1;
    pinTable[4]=&pin2;
    pinTable[5]=&pin3;
}

/**
 * \fn probe for 3 poles 
 * @param a
 * @param b
 * @param c
 * @param comp
 */
bool Tester::probe3Pins(int dex,Component **comp)
{
    COMPONENT_TYPE xtype;
    TestPin **pins=pinTable+dex-1;
    
    Component *c2=Component::identity3(*pins[0],*pins[1],*pins[2],xtype);
    if(!c2)
        return false;
  
    delete *comp;
    *comp=c2;
    return true;
}
/**
 * \fn probe for 2 poles
 * @param a
 * @param b
 * @param c
 * @param comp
 */
bool  Tester::probe2Pins(int dex,Component **comp)
{
    COMPONENT_TYPE xtype;
    TestPin **pins=pinTable+dex-1;
    
    Component *c2=Component::identity2(*pins[0],*pins[1],*pins[2],xtype);
    if(!c2)
        return false;
    
    if(!*comp) 
    {
        *comp=c2;
        return true;
    }        
    bool replace=false;
    if(c2->likely()>(*comp)->likely())
    {
        replace=true;
    }
    if(replace)
    {
        delete *comp;
        *comp=c2;
    }else
    {
        delete c2;
    } 
    return true;
}

/**
 * 
 * @param dex
 * @param p1
 * @param p2
 * @param p3
 * @param c
 */
void Tester::scan2Pins(int dex, Component **c)
{
    TesterGfx::print(dex*20,75,"x");
    probe2Pins(dex,c);
}
/**
 * 
 * @param dex
 * @param p1
 * @param p2
 * @param p3
 * @param c
 */
void Tester::scan3Pins(int dex, Component **c)
{
    TesterGfx::print(dex*20,75,"x");
    probe3Pins(dex,c);
}


/**
 * 
 */
bool Tester::probe(void)
{
    COMPONENT_TYPE type;
    
    // 1 : Zeroing
    TesterGfx::clear();
    TesterGfx::print(30,40,"Zeroing..");
    zeroAllPins();
    TesterGfx::clear();
    TesterGfx::print(0,40,"Click to probe");
next:
    TesterGfx::clear();
    TesterGfx::print(0,40,"Detecting");
    
    Component *c=NULL;

    
    scan3Pins(1,&c);
    scan3Pins(2,&c);
    scan3Pins(3,&c);
        
    if(!c)
    {
        scan2Pins(1,&c);
        scan2Pins(2,&c);
        scan2Pins(3,&c);
    }
    
    if(!c)
    {
        TesterGfx::clear();
        TesterGfx::print(0,60,"Found nothing");
#if 0        
        if(TesterControl::waitForEvent() & CONTROL_LONG)
            menuSystem();
#endif        
        return true;
        
    }
    TesterGfx::clear();
    const char *sname=c->getShortName();
    TesterGfx::print(8,40,sname);
    TesterGfx::print(0,60,"  Measuring");

    // Valid component detected ?
    if(c->compute())
    {   
        zeroAllPins();
        TesterGfx::clear();
        c->draw(0);     
        while(1)
        {
            int evt=TesterControl::waitForEvent();            
//            if(evt & CONTROL_LONG) menuSystem(); // probe next
            if(evt & CONTROL_SHORT) goto next; // probe next
            // TODO LONG => menu
            if(evt & CONTROL_ROTARY)
            {
                int count=TesterControl::getRotary();
                c->changePage(count);
            }
        }
    }
    delete c;
    return true;
}