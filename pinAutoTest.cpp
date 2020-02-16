


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
        TesterGfx::print(80,LINE,failure); \
    } \
}



void pinTest()
{
    TesterGfx::clear();
#if 0
    RUNTEST(1,1,20)
    RUNTEST(2,2,50)
    RUNTEST(3,3,80)
#else
    RUNTEST(1,2,20)
    RUNTEST(2,1,50)
    RUNTEST(3,3,80)            
#endif
            
    while(1)
    {
        digitalWrite(LED,LOW);
        xDelay(300);
        digitalWrite(LED,HIGH);
        xDelay(600);
    }
}

