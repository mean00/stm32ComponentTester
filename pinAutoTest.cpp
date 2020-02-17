


#include "testPins.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "MapleFreeRTOS1000_pp.h"
extern TestPin   pin1;
extern TestPin   pin2;
extern TestPin   pin3;
extern uint32_t  deviceId;
#define LED PC13



static bool singlePinTest(TestPin &A, TestPin &MeasurePin, const char **failure)
{
    AutoDisconnect ad;
    {
        A.setToGround();
        xDelay(5);
        int sum,nb;
        if(!MeasurePin.slowDmaSample(sum, nb))
            return false;
        sum/=nb;
        if(sum>11)
        {
            *failure="G";
            return false;
        }
    }
    {
        A.setToVcc();
        xDelay(5);
        int sum,nb;
        if(!MeasurePin.slowDmaSample(sum, nb))
            return false;
        sum/=nb;
        if(sum<3900) // might be wrong at high end of the spectrum, take extra margin
        {
            *failure="V";
            return false;
        }
    }    
    const char *label[3]=   {"L","M","H"};
    
    for(int i=0;i<2;i++)
    {
        TestPin::PULL_STRENGTH strength=(TestPin::PULL_STRENGTH )i;
        *failure=label[i];
        A.pullUp(strength);
        xDelay(5);
        int sum,nb;
        if(!MeasurePin.slowDmaSample(sum, nb))
            return false;
        sum/=nb;
        if(sum<3900) // might be wrong at high end of the spectrum, take extra margin
        {
            
            return false;
        }
        A.pullDown(strength);
        xDelay(5);
        if(!MeasurePin.slowDmaSample(sum, nb))
            return false;
        sum/=nb;
        if(sum>11) // might be wrong at high end of the spectrum, take extra margin
        {
            return false;
        }
    }
    
    return true;
    
}
#define RUNTEST(PIN,MPIN,LINE) \
{ \
    const char *failure; \
    TesterGfx::print(2,LINE,"PIN " #PIN ":"); \
    if(singlePinTest(pin##PIN,pin##MPIN,&failure)) \
        TesterGfx::print(80,LINE,"OK"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(80,LINE,failure); \
    } \
}

bool dualDmaTest(const char *text, TestPin &A, TestPin & B, int line)
{
    AutoDisconnect ad;
    A.setToVcc();
    B.pullUp(TestPin::PULL_MED);
    xDelay(5);
    A.prepareDualDmaSample(B,ADC_SMPR_28_5,ADC_PRE_PCLK2_DIV_6,32);
    int nbSamples;
    uint16_t *samples;
    if(!A.finishDmaSample(nbSamples,&samples)) 
    {
            return false;
    }    
   // We should have 32 alternating 0 && 4095 
    int hi=0,low=0;
    for(int i=0;i<nbSamples>>1;i++)
    {
        low+=samples[i*2];
        hi+=samples[i*2+1];
    }
    if(hi<4090) return false;
    if(low>400) return false;
            
    return true;
}

void pinTest()
{
    bool testFailed=false;
    TesterGfx::clear();
    dualDmaTest("Pin12",pin2,pin3,20)        ;
    RUNTEST(1,1,20)
    RUNTEST(2,2,50)
    RUNTEST(3,3,80)
    if(testFailed)
    {
        while(1) {};
    }
    while(1)
    {
        digitalWrite(LED,LOW);
        xDelay(300);
        digitalWrite(LED,HIGH);
        xDelay(600);
    }
}

