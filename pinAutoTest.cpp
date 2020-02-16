


#include "testPins.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "MapleFreeRTOS1000_pp.h"
extern TestPin   pin1;
extern TestPin   pin2;
extern TestPin   pin3;
extern uint32_t  deviceId;
#define LED PC13


static int singleTwoPints(TestPin &A, int adc)
{
    pinMode(adc,INPUT_ANALOG);
    A.setToGround();
    xDelay(5);
    int value=analogRead(adc);
    A.setToVcc();
    xDelay(5);
    pinMode(adc,INPUT_ANALOG);
    int value2=analogRead(adc);
    return (value<<16)+value2;
    
}

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
#define RUNTEST(PIN,LINE) \
{ \
    const char *failure; \
    TesterGfx::print(2,LINE,"PIN " #PIN ":"); \
    if(singlePinTest(pin##PIN,pin##PIN,&failure)) \
        TesterGfx::print(80,LINE,"OK"); \
    else \
    { \
        TesterGfx::print(80,LINE,failure); \
    } \
}



void pinTest()
{
    TesterGfx::clear();
#if 0
    uint32_t zz=singleTwoPints(pin2,PA1);
    while(1)
    {
        
    }
#endif
    RUNTEST(1,20)
    RUNTEST(2,50)
    RUNTEST(3,80)
    while(1)
    {
        digitalWrite(LED,LOW);
        xDelay(300);
        digitalWrite(LED,HIGH);
        xDelay(600);
    }
}

