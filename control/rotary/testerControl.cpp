//
#include "wav_irotary.h"
#include "testerControl.h"
#include "pinConfiguration.h"
#include "testerGfx.h"
WavRotary *rotary=NULL;

void TesterControl::init()
{
    rotary=new WavRotary(PIN_ROTARY_LEFT,PIN_ROTARY_RIGHT,PIN_ROTARY_PUSH); 
    rotary->start();
}

int  TesterControl::getRotary()
{
    return rotary->getCount();
}

/**
 * 
 * @return 
 */
 int  TesterControl::waitForEvent()
 {
     int out=0;
     while(1)
     {
          WavRotary::EVENTS evt=rotary->waitForEvent();
          if(evt & WavRotary::SHORT_PRESS ) out |=CONTROL_SHORT;
          if(evt & WavRotary::LONG_PRESS ) out |=CONTROL_LONG;
          if(evt & WavRotary::ROTARY_CHANGE ) out |=CONTROL_ROTARY;
          return out;          
     }
 }
/**
 * 
 */
void TesterControl::waitForAnyEvent()
{
    
    while(!rotary->waitForEvent())
    {
        
    }
}


/**
 * 
 */    
void TesterControl::test()
{
    int  rot=0;
    int  c=0;
    char st[32];
    xDelay(100);
    int z;
    TesterGfx::clear();
    TesterGfx::print(2,60,"ROTARY TST"); // takes 0.3 ms

    int nbShort=0;
    int nbLong=0;
    
    while(1)
    {
        bool refresh=false;
        WavRotary::EVENTS evt=rotary->waitForEvent();
        
        if(evt & WavRotary::ROTARY_CHANGE)
        {
            int inc=rotary->getCount();        
            if(inc)
            {
                rot+=inc;
                refresh=true;
            }
        }
        if(evt & WavRotary::LONG_PRESS )
        {
             {nbLong++;refresh=true;}
        }
         if(evt & WavRotary::SHORT_PRESS )
        {
             {nbShort++;refresh=true;}
        }
        
       
        if(refresh)
        {
            TesterGfx::clear();
            sprintf(st,"%d-%d",rot,c);
            TesterGfx::print(20,20,st);
            sprintf(st,"Long %d",nbLong);
            TesterGfx::print(20,40,st);
            sprintf(st,"Short %d",nbShort);
            TesterGfx::print(20,60,st);

        }
        c++;
    }
}

 // EOF
