


#include "testPins.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testerControl.h"
#include "adc_limit.h"

extern TestPin   pin1;
extern TestPin   pin2;
extern TestPin   pin3;
extern uint32_t  deviceId;
#define LED PC13

#define Y_OFFSET 20


void trace(TestPin &A, const char *label, int value)
{
    Logger("Test pin %d : %s : %d \n ",A.pinNumber(),label,value);
}


int easySample(TestPin &M);

static bool singlePinTest(TestPin &A, TestPin &MeasurePin, const char **failure)
{
    int sum;
    {
        AutoDisconnect ad2;
    }
    AutoDisconnect ad;
    {
        pinMode(MeasurePin.pinADC(),INPUT_ANALOG);
        A.setToGround();        
        sum=easySample(MeasurePin);    
        trace(A,"Gnd",sum);
        if(sum>LOW_FLOOR)
        {
            *failure="G";
            return false;
        }
    }
    {
        A.setToVcc();
        sum=easySample(MeasurePin);   
        trace(A,"Vcc",sum);
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
        trace(A,"Pup",sum);
        if(sum<HIGH_CEIL) // might be wrong at high end of the spectrum, take extra margin
        {
            return false;
        }
        A.pullDown(strength);
        xDelay(5);
        sum=easySample(MeasurePin);      
        trace(A,"Pdown",sum);
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
    const char *lb;
    
    if(onoff) // should be to VCC
        for(int i=1;i<nbSamples;i++) // Skip 1st sample!
        {
            trace(A, "U", samples[i]);
            if(samples[i]<(ADC_HIGH)) return false;
            
        }
    else  // to ground
        for(int i=1;i<nbSamples;i++)
        {
            trace(A, "D", samples[i]);
            if(samples[i]>ADC_LOW) return false;
        }
    
    return true;
}

/**
 * Do a simple one pin dma sampling, by pulling that pin up / down
 * it is mostly the same as the pinTest above
 * @param text
 * @param A
 * @param onoff
 * @return 
 */

bool T1TimeTest(const char *text, TestPin &A,  int onoff)
{
    AutoDisconnect ad;
    if(onoff)
        A.setToVcc();
    else
        A.pullDown(TestPin::PULL_MED);    
    xDelay(5);    
    A.prepareTimerSample(10*1000,32);
    int nbSamples;
    uint16_t *samples;
    if(!A.finishDmaSample(nbSamples,&samples)) 
    {
            return false;
    }    
    const char *lb;
    
    if(onoff) // should be to VCC
        for(int i=1;i<nbSamples;i++) // Skip 1st sample!
        {
            trace(A, "U", samples[i]);
            if(samples[i]<(ADC_HIGH)) return false;
            
        }
    else  // to ground
        for(int i=1;i<nbSamples;i++)
        {
            trace(A, "D", samples[i]);
            if(samples[i]>ADC_LOW) return false;
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
bool dualDmaTest(const char *text, TestPin &A, TestPin & B, TestPin &C, bool thirdState)
{
    AutoDisconnect ad;
    A.setToVcc();
    B.pullDown(TestPin::PULL_MED);
    if(thirdState)
        C.setToVcc();
    else
        C.pullDown(TestPin::PULL_MED);
    xDelay(5);
    A.prepareDualDmaSample(B,ADC_SMPR_28_5, DSOADC::ADC_PRESCALER_6 ,64);
    int nbSamples;
    uint16_t *samples;
    if(!A.finishDmaSample(nbSamples,&samples)) 
    {
            return false;
    }    
    int nbPair=nbSamples/2;
   // We should have 32 pairs alternating 0 && 4095 
    // B A B A
    int hi=0,low=0;
    for(int i=1;i<nbPair;i++)
    {
        hi+=samples[i*2];
        low+=samples[i*2+1];
    }
    low/=nbPair;
    hi/=nbPair;
    
    trace(A,"low",low);
    trace(A,"hi",hi);
    
    if(hi<ADC_HIGH) return false;
    if(low>ADC_LOW) return false;
            
    return true;
}
#define RUNDDMATEST(PIN,MPIN,CPIN,LINE) \
{ \
    const char *failure; \
    TesterGfx::print(2,LINE,"DMA " #PIN #MPIN ":"); \
    DSOADC::clearSamples(); \
    if(dualDmaTest("",pin##PIN,pin##MPIN,pin##CPIN,false)) \
        TesterGfx::print(100,LINE,"+"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(100,LINE,"-"); \
    } \
    if(dualDmaTest("",pin##PIN,pin##MPIN,pin##CPIN,true)) \
        TesterGfx::print(110,LINE,"+"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(110,LINE,"-"); \
    } \    
}
#define RUN1DMATEST(PIN,MPIN,LINE) \
{ \
    const char *failure; \
    TesterGfx::print(2,LINE,"1DMA " #PIN  ":"); \
    if(T1DmaTest("DMAL",pin##PIN,0)) \
        TesterGfx::print(100,LINE,"+"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(80,LINE,"KO"); \
        Logger("1DMA-0 fail\n"); \
    } \
    if(T1DmaTest("DMAH",pin##PIN,1)) \
        TesterGfx::print(110,LINE,"+"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(80,LINE,"KO2"); \
        Logger("1DMA-1 fail\n");\
    } \
}

#define RUN1TIMETEST(PIN,MPIN,LINE) \
{ \
    const char *failure; \
    TesterGfx::print(2,LINE,"1TIME " #PIN  ":"); \
    if(T1TimeTest("TimeL",pin##PIN,0)) \
        TesterGfx::print(100,LINE,"+"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(80,LINE,"KO"); \
        Logger("1TIME-0 fail\n"); \
    } \
    if(T1TimeTest("TimeU",pin##PIN,1)) \
        TesterGfx::print(110,LINE,"+"); \
    else \
    { \
        testFailed=true; \
        TesterGfx::print(80,LINE,"KO2"); \
        Logger("1TIME-1 fail\n");\
    } \
}
bool pinTest()
{
    
    bool testFailed=false;

    TesterGfx::title("PinTest");
    RUNTEST(2,2,50+Y_OFFSET)
    RUNTEST(3,3,80+Y_OFFSET)    
    RUNTEST(1,1,20+Y_OFFSET)
    if(testFailed)
    {
       return false;
    }
        
    TesterGfx::title("DMA");
    RUN1DMATEST(1,2,20+Y_OFFSET);
    RUN1DMATEST(1,3,50+Y_OFFSET);
    RUN1DMATEST(2,3,80+Y_OFFSET);
    if( testFailed)        
        return false;
    
    TesterGfx::title("TIME");
    RUN1TIMETEST(1,2,20+Y_OFFSET);
    RUN1TIMETEST(1,3,50+Y_OFFSET);
    RUN1TIMETEST(2,3,80+Y_OFFSET);
    if( testFailed)        
        return false;
    
    
    
    
    
    TesterGfx::title("2DMA");
    RUNDDMATEST(1,2,3,20+Y_OFFSET);
    RUNDDMATEST(1,3,2,50+Y_OFFSET);
    RUNDDMATEST(2,3,1,80+Y_OFFSET);
    if( testFailed)        
        return false;    
    return true;
}

int easySample(TestPin &M)
{
    int sum,nb;
    xDelay(10);
    xAssert(M.summedRead(sum, nb)); // should never fail
    sum/=nb;
    return sum;
}

