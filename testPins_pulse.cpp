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
    bool init(int samplingFrequency)
    {
        if(!  findRateScale(samplingFrequency, prescaler,rate))
        {
            xAssert(0);
            return false;
        }   
        pwmGetScaleOverFlow(samplingFrequency,timerScaler,timerOvf);
        return false;
    }

    /**
     * \brief compute rate & scale so that the ADC sampling fq is larger than timer frequency
     * @param frequency
     * @param prescaler
     * @param rate
     * @return 
     */
    bool findRateScale(int frequency,  DSOADC::Prescaler  &prescaler,  adc_smp_rate   &rate)
    {
#if 0        
        // The parasitic cap varies depending on the rate/scale
        // hardcode it to the one we know
        prescaler= DSOADC::ADC_PRESCALER_2;
        rate=ADC_SMPR_1_5;
        return true;
#endif        
        float alpha=(float)F_CPU;
        alpha/=(float)(frequency+1);

        int dex=-1;
        for(int i=sizeof(scalerTable)/sizeof(ScalerTable)-1;i>0 && dex==-1;i--)
        {
            float one=alpha/scalerTable[i].div;
            if(one>(239.5+12))
            {
                dex=i;
                break;
            }
        }
        if(dex==-1)
        {
            // Take the biggest prescaler
            dex=0; //sizeof(scalerTable)/sizeof(ScalerTable)-1;
        }
        prescaler=scalerTable[dex].scaler;

        // now rate
        alpha=(float)F_CPU;
        alpha/=(float)(frequency+1);
        alpha/=scalerTable[dex].div;
        dex=-1;
        for(int i=sizeof(rateTable)/sizeof(RateTable)-1;i>0 && dex==-1;i--)
        {
            // Alpha > rate
            if(alpha>rateTable[i].cycle)
            {
                dex=i;
                break;
            }
        }
        if(dex==-1)
        {
            dex=0;
        }
        rate=rateTable[dex].rate;
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
    bool  createWaveForm(     int sampleAsked,    int &nbSamples,    uint16_t  **samples)
    {
        // The apparent sampling frequency is F_CPU/(timerScaler*apprentDivider)
        // make it so timerSCaler*apparentDivier=8
        // so we end up with a know sampling frequency of F_CPU/8 => 9 Mhz with CPU@72 Mhz whatever timerScale is
        // as long as it is 1 2 or 4, from 500 Hz to ~ 10 khz
        
        int apparentDivider=8/timerScaler;
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
            adc->startTimerSampling(sampleAsked);
            quick.go();
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
            this->stroboscopeSampling=apparentDivider;            
            return true;  
    }
    
    int             pin;
    int             timerScaler;
    int             timerOvf;
    DSOADC::Prescaler  prescaler;   
    adc_smp_rate    rate ;
    int             offset;
    int             stroboscopeSampling;
    TestPin::TESTPIN_STATE        pinState;
    int             res;
    TestPin         &testPin;
};

/**
 * 
 * @param nbSampleAsked
 * @param samplingFrequency : The input signal is repeated every samplingFrequency
 * @param strength          : Pull up strength
 * @param nbSample
 * @param xsamples
 * @param samplingTime      : The "Apparent" sampling frequency is F_CPU/samplingTime
 * @param res               : Load Resistance
 * @return 
 */
bool  TestPin::pulseTime(int nbSampleAsked, int samplingFrequency, TestPin::PULL_STRENGTH strength,   int &nbSample,  uint16_t **xsamples,int &samplingTime,int &res)
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
    if(!settings.createWaveForm(nbSampleAsked,nbSample,xsamples  ))
    {
        return false;
    }
   
    pullDown(strength);
    xDelay(10);
    
    // Apparent divider compared to F_CPU
    samplingTime=settings.stroboscopeSampling*settings.timerScaler;
    res=settings.res;   
    disconnectAll();
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
