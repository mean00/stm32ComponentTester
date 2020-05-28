


#include "testPins.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testerControl.h"

extern TestPin   pin1;
extern TestPin   pin2;
extern TestPin   pin3;
extern uint32_t  deviceId;
#define LED PC13

#define Y_OFFSET 20




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
/**
 * Do a simple one pin dma sampling, by pulling that pin up / down
 * it is mostly the same as the pinTest above
 * @param text
 * @param A
 * @param onoff
 * @return 
 */
#define MARGIN 10
bool T1DmaTest(const char *text, TestPin &A,  int onoff)
{
    AutoDisconnect ad;
    if(onoff)
        A.setToVcc();
    else
        A.pullDown(TestPin::PULL_MED);    
    xDelay(5);
    A.prepareDmaSample(ADC_SMPR_28_5, DSOADC::ADC_PRESCALER_6 ,32);
    int nbSamples;
    uint16_t *samples;
    if(!A.finishDmaSample(nbSamples,&samples)) 
    {
            return false;
    }    

    if(onoff) // should be to VCC
        for(int i=1;i<nbSamples;i++) // Skip 1st sample!
        {
            if(samples[i]<(4095-MARGIN)) return false;
        }
    else  // to ground
        for(int i=1;i<nbSamples;i++)
        {
            if(samples[i]>MARGIN) return false;
        }
    
    return true;
}
/**
 * Do a dual DMA test i.e. sample 2 different pins at the same time
 * @param text
 * @param A
 * @param B
 * @param line
 * @return 
 */
bool dualDmaTest(const char *text, TestPin &A, TestPin & B, int line)
{
    AutoDisconnect ad;
    A.setToVcc();
    B.pullDown(TestPin::PULL_MED);
    xDelay(5);
    A.prepareDualDmaSample(B,ADC_SMPR_28_5, DSOADC::ADC_PRESCALER_6 ,64);
    int nbSamples;
    uint16_t *samples;
    if(!A.finishDmaSample(nbSamples,&samples)) 
    {
            return false;
    }    
   // We should have 32 pairs alternating 0 && 4095 
    // B A B A
    int hi=0,low=0;
    for(int i=1;i<nbSamples/2;i++)
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
    DSOADC::clearSamples(); \
    if(dualDmaTest("",pin##PIN,pin##MPIN,0)) \
        TesterGfx::print(100,LINE,"OK"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(100,LINE,"KO"); \
    } \
}
#define RUN1DMATEST(PIN,MPIN,LINE) \
{ \
    const char *failure; \
    TesterGfx::print(2,LINE,"1DMA " #PIN  ":"); \
    if(T1DmaTest("",pin##PIN,0)) \
        TesterGfx::print(100,LINE,"+"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(80,LINE,"KO"); \
    } \
    if(T1DmaTest("",pin##PIN,1)) \
        TesterGfx::print(110,LINE,"+"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(80,LINE,"KO2"); \
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
    TesterGfx::print(1,Y_OFFSET,"1DMA Test");
    RUN1DMATEST(1,2,20+Y_OFFSET);
    RUN1DMATEST(1,3,50+Y_OFFSET);
    RUN1DMATEST(2,3,80+Y_OFFSET);
    if( testFailed)        
        while(1)
        {

        }
    
    TesterGfx::clear();
    TesterGfx::print(1,Y_OFFSET,"2DMA Test");
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

