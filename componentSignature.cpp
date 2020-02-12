

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
Component *Component::identity(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type)
{
    TestPin::PULL_STRENGTH st=TestPin::PULL_LOW;
         
    zeroAllPins();
    
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
    zeroAllPins();
    if(bottomLeft==bottomRight && topLeft==topRight) // dipole
    {
#if 0        
        if((topLeft==SIG(HIGH,LOW)) && bottomLeft==SIG(LOW,HIGH))
        {               
               return NULL;
        }
#endif
        
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
    
    // Resistor, coil or capacitor
    // if it is a capacitor, it will keep its charge
    // let's charge it
    A.pullUp(TestPin::PULL_MED);
    B.setToGround();
    xDelay(100); // 100 ms should give a decent charge
    
    //now connect both to ground
    
    // Disconnect them now, they should be connected to Vcc high impedance
    
    // connect A & B to ground through 300k
    // if it is a cap, it will "slowly" discharge
    
     if(!A.prepareDmaSample(  ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_6, 512)) 
        return false;        
    // Go!    
    A.disconnect();
    B.pullDown(  TestPin::PULL_HI );   
    int nbSamples;
    uint16_t *samples;
    if(!A.finishDmaSample(nbSamples,&samples)) 
    {
        return false;
    }
    if(samples[10]>100)  // ok it's a cap
    {
        type=COMPONENT_CAPACITOR;
        zeroAllPins();
        return new Capacitor(A,B,C);
    }
    type=COMPONENT_RESISTOR;
    zeroAllPins();    
    return new Resistor(A,B,C);
    
    
}