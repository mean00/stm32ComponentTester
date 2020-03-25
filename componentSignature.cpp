

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

extern int easySample(TestPin &M);

/**
 * 
 * @param A
 * @return 
 */
static int getSignature(TestPin &A)
{
    int sum,nb;
    xDelay(10);
    sum=easySample(A);    
    if(sum<LOW_FLOOR)
        return PIN_SIG_LOW;
    if(sum>HIGH_CEIL)
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
 */
int  Component::evaluate(TestPin &pin)
{
    int sum,nb;
    xAssert(pin.slowDmaSample(sum,nb));
    
    sum=sum/nb;    
    return (int)sum;
}
/*
 * 
 */
Component *Component::identify3poles(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type)
{
    // We know the gate/base is C
    // Where is the diode A/B ?
    
    // The mosfet is the other way
    
    return NULL;
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
    
    // We do the same on B & C
    // but invert A
    {
            AutoDisconnect ad;

            A.pullUp(st);
            B.pullDown(st);

            C.pullDown(st);    
            xDelay(50); // wait for discharge
            topLeft=getSignature(A,B);
            C.pullUp(st);
            xDelay(50); // wait for discharge
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
    AutoDisconnect ad;
    
    // topLeft = Top/bottom with 3rd=0, topRight=top/bottom with rd=1
    // bottomLeft = bottom/top with 3rd=0, topRight=bottom/Top with rd=1
    PRINTF("Top Left");
    PRINTF(topLeft);
    PRINTF("Bottom Left");
    PRINTF(bottomLeft);
    PRINTF("Top Right");
    PRINTF(topRight);
    PRINTF("Bottom Right");
    PRINTF(bottomRight);

    // if the result differs depending on C, its a transistor of some sort
    if((bottomLeft!=bottomRight) || (topLeft!=topRight)) // tripole
    {        
        if(topLeft==topRight && topLeft==SIG(MEDIUM,MEDIUM))
        {
            //
            if((bottomLeft==SIG(LOW,HIGH) ||  bottomLeft==SIG(LOW,MEDIUM) )&&  bottomRight==SIG(MEDIUM,MEDIUM))
            {
                PRINTF("N MOSFET");
                return new NMosFet(C,B,A); // k
            }
            if(bottomLeft==SIG(MEDIUM,MEDIUM) && ( bottomRight==SIG(LOW,HIGH)|| bottomRight==SIG(LOW,MEDIUM)))
            {
                PRINTF("P MOSFET");
                return new PMosFet(C,A,B); //k
            }
        }
        
        if(bottomLeft==bottomRight && bottomLeft==SIG(MEDIUM,MEDIUM))
        {
            if((topLeft==SIG(HIGH,LOW)|| topLeft==SIG(MEDIUM,LOW)) && topRight==SIG(MEDIUM,MEDIUM))
            {
                PRINTF("N MOSFET");
                return new NMosFet(C,A,B);
            }
            if(topLeft==SIG(MEDIUM,MEDIUM) && topRight==SIG(HIGH,LOW)|| topRight==SIG(MEDIUM,LOW))
            {
                PRINTF("P MOSFET");
                return new PMosFet(C,B,A);  //k
            }
        }
        if(topRight==bottomRight && topRight==SIG(MEDIUM,MEDIUM))
        {
            if(topLeft==SIG(HIGH,LOW)&&bottomLeft==SIG(LOW,HIGH)) // NPN
            {
                // C is the base, but we are not sure yet about collector & emitter
                // E &C are hard to distinguish
                // We'll check the bigger HFE to spot Emitter
                A.pullDown(TestPin::PULL_LOW);
                B.pullDown(TestPin::PULL_LOW);
                C.pullUp(TestPin::PULL_HI);
                xDelay(10);
                A.pullUp(TestPin::PULL_LOW);
                int forward=evaluate(A);
                A.pullDown(TestPin::PULL_LOW);
                B.pullUp(TestPin::PULL_LOW);
                xDelay(10);
                int backward=evaluate(B);
                
                // Smaller = Emitter
                // B E C
                PRINTF("NPN");
                if(backward<forward)                
                    return new NPNBjt(C,A,B);
                else
                    return new NPNBjt(C,B,A);
                
            }
        }
        if(topLeft==bottomLeft && topLeft==SIG(MEDIUM,MEDIUM))
        {
            // same story here
            // Collector & emitter behave the same
            // with just a very bad Hfe when connected backward
            
            if(topRight==SIG(HIGH,LOW)&&bottomRight==SIG(LOW,HIGH)) // PNP
            {
                A.pullDown(TestPin::PULL_LOW);
                B.pullDown(TestPin::PULL_LOW);
                C.pullDown(TestPin::PULL_HI);  // Base                
                A.setToVcc();
                xDelay(10);
                int forward=evaluate(B);
                A.pullDown(TestPin::PULL_LOW);
                xDelay(10);
                B.setToVcc();
                int backward=evaluate(A);                
                B.pullDown(TestPin::PULL_LOW);
                PRINTF("PNP");
                if(forward>backward)
                    return new PNPBjt(C,A,B);
                else
                    return new PNPBjt(C,B,A);
            }
        }
    }
    // Stay the same => dipole
    return identify2poles(A,B,C,type);
}
/**
 * 
 * @param A
 * @param B
 * @param C
 * @param type
 * @return 
 */
Component *Component::identify2poles(TestPin &A, TestPin &B, TestPin &C, COMPONENT_TYPE &type)
{
     int topLeft,bottomLeft;
    TestPin::PULL_STRENGTH st=TestPin::PULL_LOW;
    // We do the same on B & C
    // but invert A
    {
            AutoDisconnect ad;

            A.pullUp(st);
            B.pullDown(st);
            topLeft=getSignature(A,B);
            
    }
    {
        AutoDisconnect ad;
        A.pullDown(st);
        B.pullUp(st);        
        bottomLeft=getSignature(A,B);
    }

    if(topLeft==SIG(MEDIUM,MEDIUM) && bottomLeft==SIG(LOW,HIGH)) // Diodie anode = A
    {
        PRINTF("DIODE");
        type=COMPONENT_DIODE;
        return new Diode(A,B,C);
    }
    if(bottomLeft==SIG(MEDIUM,MEDIUM) && topLeft==SIG(HIGH,LOW)) // Diodie cathode = A
    {
       PRINTF("DIODE");
       type=COMPONENT_DIODE;
       return new Diode(B,A,C);
    }        
    // Resistor, coil or capacitor
    // if it is a capacitor, it will keep its charge
    // let's charge it
    A.pullUp(TestPin::PULL_LOW);
    B.setToGround();
    xDelay(100); // 100 ms should give a decent charge

    // if it is a cap, it will "slowly" discharge due to leakage
    // Go!    
    A.disconnect();
    B.pullDown(  TestPin::PULL_HI );   
    int nbSamples;
    uint16_t *samples;
    xDelay(10); // let it stabilize
    if(!A.prepareDmaSample(  ADC_SMPR_13_5,  ADC_PRE_PCLK2_DIV_6, 512)) 
        return false;        

    if(!A.finishDmaSample(nbSamples,&samples)) 
    {
        return false;
    }
    if(samples[100]>100)  // ok it's a cap (or nothing)
    {
        PRINTF("CAPACITOR");
        // We need to evaluate it to confirm
        // too bad we'll do it twice
        type=COMPONENT_CAPACITOR;
        Capacitor *c= new Capacitor(A,B,C);
        if(c->compute())
        {
            PRINTF("CONFIRMED");
            return c;
        }
        delete c;
        PRINTF("NOT");
        zeroAllPins();
        return NULL;
    }
    PRINTF("RESISTOR");
    type=COMPONENT_RESISTOR;
    zeroAllPins();    
    return new Resistor(A,B,C);
}
