

#include "testPins.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "MapleFreeRTOS1000_pp.h"

#include "componentSignature.h"
#include "allComponents.h"

#define PIN_SIG_LOW     0
#define PIN_SIG_HIGH    1
#define PIN_SIG_MEDIUM  2

#define SIG(a,b) (((PIN_SIG_##a)<<2)+(PIN_SIG_##b))

/**
 * 
 * @param A
 * @return 
 */
static int getSignature(TestPin &A)
{
    int sum,nb;
    xDelay(10);
    xAssert(A.slowDmaSample(sum,nb));
    
    sum=sum/nb;    
    if(sum<12)
        return PIN_SIG_LOW;
    if(sum>3950)
        return PIN_SIG_HIGH;
    return PIN_SIG_MEDIUM;    
}
/**
 * 
 * @param 
 * @param 
 * @return 
 */
int getSignature(TestPin &A,TestPin &B)
{
    return (getSignature(A)<<2)+(getSignature(B));
}
/**
 * 
 * @param A
 * @param B
 * @param C
 * @param type
 * @return 
 */
Component *identity(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type)
{
    TestPin::PULL_STRENGTH st=TestPin::PULL_LOW;
            
    int topLeft,topRight,bottomLeft,bottomRight;
    
    {
            AutoDisconnect ad;

            A.pullUp(st);
            B.pullDown(st);

            C.pullDown(st);    
            topLeft=getSignature(A,B);
            C.pullUp(st);
            topRight=getSignature(A,B);
    }
    {
        AutoDisconnect ad;
        A.pullDown(st);
        B.pullUp(st);        
        C.pullDown(st);  
        bottomLeft=getSignature(A,B);
        C.pullUp(st);
        bottomRight=getSignature(A,B);
    }
    
    if(bottomLeft==bottomRight && topLeft==topRight) // dipole
    {
        
        if((topLeft==SIG(HIGH,LOW)) && bottomLeft==SIG(LOW,HIGH))
        {               
               return NULL;
        }
        
        if(topLeft==SIG(MEDIUM,MEDIUM) && bottomLeft==SIG(LOW,HIGH)) // Diodie anode = A
        {
            type=COMPONENT_DIODE;
            return new Diode(A,B,C);
        }
        if(bottomLeft==SIG(MEDIUM,MEDIUM) && topLeft==SIG(HIGH,LOW)) // Diodie cathode = A
        {
           type=COMPONENT_DIODE;
           return new Diode(B,A,C);
        }
    }
    return NULL;
    
}