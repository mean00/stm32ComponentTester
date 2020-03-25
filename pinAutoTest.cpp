


#include "testPins.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "MapleFreeRTOS1000_pp.h"
#include "wav_irotary.h"
#include "pushButton.h"
extern TestPin   pin1;
extern TestPin   pin2;
extern TestPin   pin3;
extern uint32_t  deviceId;
#define LED PC13

#define Y_OFFSET 20

extern WavRotary rotary;
extern PushButton *pushButton;

int easySample(TestPin &M);

static bool singlePinTest(TestPin &A, TestPin &MeasurePin, const char **failure)
{
    int sum;
    AutoDisconnect ad;
    {
         pinMode(MeasurePin.pinADC(),INPUT_ANALOG);
        A.setToGround();        
        sum=easySample(MeasurePin);       
        if(sum>LOW_FLOOR)
        {
            *failure="G";
            return false;
        }
    }
    {
        A.setToVcc();
        sum=easySample(MeasurePin);       
        if(sum<HIGH_CEIL) // might be wrong at high end of the spectrum, take extra margin
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
        sum=easySample(MeasurePin);       
        if(sum<HIGH_CEIL) // might be wrong at high end of the spectrum, take extra margin
        {
            return false;
        }
        A.pullDown(strength);
        xDelay(5);
        sum=easySample(MeasurePin);      
        if(sum>LOW_FLOOR) // might be wrong at high end of the spectrum, take extra margin
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
    B.pullDown(TestPin::PULL_MED);
    xDelay(5);
    A.prepareDualDmaSample(B,ADC_SMPR_28_5,ADC_PRE_PCLK2_DIV_6,32);
    int nbSamples;
    uint16_t *samples;
    if(!A.finishDmaSample(nbSamples,&samples)) 
    {
            return false;
    }    
   // We should have 32 alternating 0 && 4095 
    // B A B A
    int hi=0,low=0;
    for(int i=0;i<nbSamples;i++)
    {
        hi+=samples[i*2];
        low+=samples[i*2+1];
    }
    low/=nbSamples;
    hi/=nbSamples;
    if(hi<3950) return false;
    if(low>40) return false;
            
    return true;
}
#define RUNDDMATEST(PIN,MPIN,LINE) \
{ \
    const char *failure; \
    TesterGfx::print(2,LINE,"DMA " #PIN #MPIN ":"); \
    if(dualDmaTest("",pin##PIN,pin##MPIN,0)) \
        TesterGfx::print(100,LINE,"OK"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(100,LINE,"KO"); \
    } \
}
void pinTest()
{

    bool testFailed=false;
    TesterGfx::clear();
    
    TesterGfx::print(1, Y_OFFSET ,"Pin Test");
    RUNTEST(2,2,50+Y_OFFSET)
    RUNTEST(3,3,80+Y_OFFSET)    
    RUNTEST(1,1,20+Y_OFFSET)
    if(testFailed)
    {
        while(1) {};
    }
    TesterGfx::clear();
    TesterGfx::print(1,Y_OFFSET,"DMA Test");
    RUNDDMATEST(1,2,20+Y_OFFSET);
    RUNDDMATEST(1,3,50+Y_OFFSET);
    RUNDDMATEST(2,3,80+Y_OFFSET);
    if( testFailed)        
        while(1)
        {

        }
}

int easySample(TestPin &M)
{
    int sum,nb;
    xDelay(10);
    xAssert(M.slowDmaSample(sum, nb)); // should never fail
    sum/=nb;
    return sum;
}


/**
 * 
 */    
void rotaryTest()
{
    int  rot=0;
    int  c=0;
    char st[32];
    xDelay(100);
    int z;
    TesterGfx::clear();
    TesterGfx::print(2,60,"TEST STRING"); // takes 0.3 ms

    int nbShort=0;
    int nbLong=0;
    
    while(1)
    {
        bool refresh=false;
        int evt=pushButton->getEvent();
        if(evt & PushButton::SHORT_PRESS)  {nbShort++;refresh=true;}
        if(evt & PushButton::LONG_PRESS)   {nbLong++;refresh=true;}
        
        int inc=rotary.getCount();        
        if(inc)
        {
            rot+=inc;
            refresh=true;
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
        xDelay(10);
    }
}