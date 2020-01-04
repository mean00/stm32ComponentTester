#include "Arduino.h"
#include "fancyLock.h"

#if 0
    #define CHECK_SLOW xAssert
#else
    #define CHECK_SLOW(...) {}
#endif
/**
 * 
 */
int nesting=0;
int intCurrent=0;
int intMax=0;
/**
 * 
 */
void FancyInterrupts::disable()
{
    noInterrupts();
    if(!nesting)
        intCurrent=micros();
    nesting++;
    
    
}
/**
 * 
 */
void FancyInterrupts::enable()
{
    if(nesting==1)
    {
        int c=micros()-intCurrent;
        if(c>intMax) intMax=c;
      
    }
    nesting--;
    interrupts();
}
