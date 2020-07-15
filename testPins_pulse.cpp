/*
 * This controls one pin : pull it up, down, etc.. and measure
 * 
 */
#include "vector"
#include "testPins.h"
#include "dso_adc.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testerGfx.h"
#include "myPwm.h"
#include "testerControl.h"

extern DSOADC *adc;
void xFail(const char *message);

/**
 * 
 * @param samplingFrequency
 * @param strength
 * @param prescaler
 * @param rate
 * @param sampleOut
 * @param xsamples
 * @return 
 */

typedef struct ScalerTable
{
    float              div;
    DSOADC::Prescaler  scaler;
};

typedef struct RateTable
{
    float              cycle;
    adc_smp_rate       rate;
};
const ScalerTable scalerTable[]=
{
    {2.0, DSOADC::ADC_PRESCALER_2},
    {4.0, DSOADC::ADC_PRESCALER_4},
    {6.0, DSOADC::ADC_PRESCALER_6},
    {8.0, DSOADC::ADC_PRESCALER_8}
};


         
#define RATE_MK(x)          { 12+x##.5, ADC_SMPR_##x##_5},
         
const RateTable rateTable[]=
{
    RATE_MK(1)
    RATE_MK(7)
    RATE_MK(13)
    RATE_MK(28)
    RATE_MK(41)
    RATE_MK(55)
    RATE_MK(71)
    RATE_MK(239)
};
#define INCREMENT 1
/**
 */
class PulseSetting
{
public:
    PulseSetting( TestPin &xtestPin, TestPin::PULL_STRENGTH strength) : testPin(xtestPin)
    {        
        
        switch(strength)
        {
            case  TestPin::PULL_LOW:  pin=testPin._pinDriveLowRes;pinState=TestPin::PULLUP_LOW;break;
            case  TestPin::PULL_MED:  pin=testPin._pinDriveMedRes;pinState=TestPin::PULLUP_MED;break;
            case  TestPin::PULL_HI:   pin=testPin._pinDriveHighRes;pinState=TestPin::PULLUP_HI;break;
            default:
                break;
        }
        res=testPin.getRes(pinState)+testPin.getRes(TestPin::GND);
    }
    /**
     * 
     * @param samplingFrequency
     * @return 
     */
    bool init(int samplingFrequency)
    {
        DSOADC::frequencyToRateScale(samplingFrequency,prescaler,rate);
        pwmGetScaleOverFlow(samplingFrequency,timerScaler,timerOvf);
        return true;
    }
    /**
     * \brief compute the offset so that the ramp up is near the beginning
     * increasing the shift makes the curve  later
     * @param adc
     * @param timerScaler
     * @param timerOvf
     * @param prescaler
     * @param rate
     * @param offset
     * @return 
     */
    bool  createWaveForm(    int clockPerSample, int sampleAsked,    int &nbSamples,    uint16_t  **samples)
    {
        // The apparent sampling frequency is F_CPU/(timerScaler*apprentDivider)
        // make it so timerSCaler*apparentDivier=8
        // so we end up with a know sampling frequency of F_CPU/8 => 9 Mhz with CPU@72 Mhz whatever timerScale is
        // as long as it is 1 2 or 4, from 500 Hz to ~ 10 khz
        
        int apparentDivider=clockPerSample/timerScaler;
        if(!apparentDivider) xAssert(0);
        // ADC is running X cycles faster than repeat
        // Same thing as ~ adc running at  X Cycle
        if(!adc->prepareTimerSampling(timerScaler,timerOvf+apparentDivider,false,rate,prescaler))
        {
            xAssert(0);
            return false;
        }
        adc->clearSemaphore();
        int before=millis();
        nbSamples=0;

        pwmQuickRestart quick(pin);
        noInterrupts();
        adc->startTimerSampling(sampleAsked);
        quick.go();
        interrupts();
        if(!adc->getSamples(samples,nbSamples))    
        {
            xAssert(0);
            adc->stopTimeCapture();
            return false;
        }
        // skip 1st sample
        nbSamples-=1;
        (*samples)+=1;

        adc->stopTimeCapture();
        pwmPause(pin);
        offset=0;
        return true;  
    }
    bool  createWaveFormDelta(TestPin &otherPin,    int &clockPerSample, int sampleAsked,    int &nbSamples,    uint16_t  **samples)
    {
        // The apparent sampling frequency is F_CPU/(timerScaler*apprentDivider)
        // make it so timerSCaler*apparentDivier=8
        // so we end up with a know sampling frequency of F_CPU/8 => 9 Mhz with CPU@72 Mhz whatever timerScale is
        // as long as it is 1 2 or 4, from 500 Hz to ~ 10 khz
        
        int apparentDivider=clockPerSample/timerScaler;
        clockPerSample=apparentDivider*timerScaler;
        if(!apparentDivider) xAssert(0);
        // ADC is running X cycles faster than repeat
        // Same thing as ~ adc running at  X Cycle
        if(!adc->prepareDualTimerSampling(timerScaler,timerOvf+apparentDivider,false,rate,prescaler))
        {
            xAssert(0);
            return false;
        }
        adc->clearSemaphore();
        int before=millis();
        nbSamples=0;

        pwmQuickRestart quick(pin);
        noInterrupts();
        adc->startDualTimeSampling(otherPin._pin,sampleAsked,0*(timerOvf-apparentDivider));
        quick.go();
        interrupts();
        if(!adc->getSamples(samples,nbSamples))    
        {
            xAssert(0);
            adc->stopTimeCapture();
            return false;
        }
        //
        adc->stopTimeCapture();
        nbSamples=(nbSamples)/2;
        // Delta        
        testPin.dualSimulatenousDelta(nbSamples,*samples);
        // skip the 2 first ones
        nbSamples-=2;
        (*samples)+=2;
        pwmPause(pin);
        offset=0;
        return true;  
    }
public:    
    int             pin;
    int             timerScaler;
    int             timerOvf;
    DSOADC::Prescaler  prescaler;   
    adc_smp_rate    rate ;
    int             offset;
    TestPin::TESTPIN_STATE        pinState;
    int             res;
    TestPin         &testPin;
};

/**
 * 
 * @param nbSampleAsked
 * @param clockPerSample    : We target clockPerSample vs F_CPU as apparent sampling frequency
 *                                          internally it is F_CPU/(samplingTime)
 * @param samplingFrequency : The input signal is repeated every samplingFrequency
 * @param strength          : Pull up strength
 * @param nbSample
 * @param xsamples
 * @param res               : Load Resistance
 * @return 
 */
bool  TestPin::pulseTime(int clockPerSample,int nbSampleAsked, int samplingFrequency, TestPin::PULL_STRENGTH strength,   int &nbSample,  uint16_t **xsamples,int &res)
{
    disconnectAll();
    
    PulseSetting settings(*this,strength);
    
    pullDown(TestPin::PULL_LOW);
    xDelay(10);
    
    settings.init(samplingFrequency);    
    
    pwm(strength,samplingFrequency);
    pwmPause(settings.pin);
     
    
    adc->setADCPin(this->_pin);
    adc->setupTimerSampling();
    //
    if(!settings.createWaveForm(clockPerSample,nbSampleAsked,nbSample,xsamples  ))
    {
        return false;
    }
   
    pullDown(strength);
    xDelay(10);
    
    res=settings.res;   
    disconnectAll();
    return true;
}
/**
 * 
 * @param otherPin
 * @param clockPerSample
 * @param nbSampleAsked
 * @param samplingFrequency
 * @param strength
 * @param nbSample
 * @param xsamples
 * @param res
 * @return 
 */
bool  TestPin::pulseTimeDelta(TestPin &otherPin, int &clockPerSample,int nbSampleAsked, int samplingFrequency, TestPin::PULL_STRENGTH strength,   int &nbSample,  uint16_t **xsamples,int &res)
{
    disconnectAll();
    
    PulseSetting settings(*this,strength);
    
    pullDown(TestPin::PULL_LOW);
    xDelay(10);
    
    settings.init(samplingFrequency);    
    
    pwm(strength,samplingFrequency);
    pwmPause(settings.pin);
     
    
    adc->setADCPin(this->_pin);
    adc->setupDualTimerSampling();
    //
    if(!settings.createWaveFormDelta(otherPin,clockPerSample,nbSampleAsked,nbSample,xsamples  ))
    {
        return false;
    }
   
    pullDown(strength);
    xDelay(10);
    
    res=settings.res;   
    disconnectAll();
    
    // Merge
    
    return true;
}

/**
 * 
 * @param nbSamples
 * @param samplingFrequency
 * @param strength
 * @param sampleOut
 * @param xsamples
 * @return 
 */
bool  TestPin::pulseDma(int nbSamples,  DSOADC::Prescaler prescaler, adc_smp_rate   rate, TestPin::PULL_STRENGTH strength,   int &sampleOut,  uint16_t **xsamples)
{
    
    
    pullDown(TestPin::PULL_LOW);
    xDelay(10);
    
    adc->setADCPin(this->_pin);
    adc->setupDmaSampling();
    adc->prepareDMASampling(rate,prescaler);     
    adc->clearSemaphore();
    adc->startDMASampling(nbSamples);    
    pullUp(strength);
    bool r=adc->getSamples(xsamples,sampleOut);
    adc->stopDmaCapture();
    if(!r)
    {
        xAssert(0);
        return false;
    }
    pullDown(strength);
    xDelay(10);
    return true;
}

// EOF
