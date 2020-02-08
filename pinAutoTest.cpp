


#include "testPins.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "MapleFreeRTOS1000_pp.h"
extern TestPin   pin1;
extern TestPin   pin2;
extern TestPin   pin3;

#define LED PC13


static bool singlePinTest(TestPin &A)
{
    {
        AutoDisconnect ad;
    }
    {
        A.setToGround();
        xDelay(5);
        int sum,nb;
        if(!A.slowDmaSample(sum, nb))
            return false;
        sum/=nb;
        if(sum>11)
            return false;
    }
    {
        A.setToVcc();
        xDelay(5);
        int sum,nb;
        if(!A.slowDmaSample(sum, nb))
            return false;
        sum/=nb;
        if(sum<3900) // might be wrong at high end of the spectrum, take extra margin
            return false;
    }
    return true;
    
}
#define RUNTEST(PIN,LINE) \
    TesterGfx::print(2,LINE,"PIN " #PIN ":"); \
    if(singlePinTest(pin##PIN)) \
        TesterGfx::print(80,LINE,"OK"); \
    else \
        TesterGfx::print(80,LINE,"KO");



void pinTest()
{
    TesterGfx::clear();
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

