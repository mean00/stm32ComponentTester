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
/**
 * \brief compute rate & scale so that the ADC sampling fq is larger than timer frequency
 * @param frequency
 * @param prescaler
 * @param rate
 * @return 
 */
bool findRateScale(int frequency,  DSOADC::Prescaler  &prescaler,  adc_smp_rate   &rate)
{
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
 * 
 * @param adc
 * @param timerScaler
 * @param timerOvf
 * @param prescaler
 * @param rate
 * @param offset
 * @return 
 */
bool  lookupForRampUp(DSOADC *adc,int pin, int timerScaler, int timerOvf, DSOADC::Prescaler  prescaler,   adc_smp_rate   rate , int &offset   )
{
    int shift=1;
    int nbSamples;
    uint16_t  *samples;
    while(1)
    {
        
        // ADC is running X cycles faster than repeat
        // Same thing as ~ adc running at  X Cycle
        if(!adc->prepareTimerSampling(timerScaler,timerOvf+4,false,rate,prescaler))
        {
            xAssert(0);
            return false;
        }
        adc->clearSemaphore();
        int before=millis();
        int nbSamples=128;

        adc->startTimerSampling(nbSamples);


        // 5k => 200 us
        delayMicroseconds(shift);
        pwmRestart(pin);
        if(!adc->getSamples(&samples,nbSamples))    
        {
            xAssert(0);
            adc->stopTimeCapture();
            return false;
        }
        adc->stopTimeCapture();
        pwmPause(pin);
        TesterGfx::drawCurve(nbSamples, samples);
        
        // Look for min/max
        int xmin=4095;
        int xmax=0;
        int minIndx;
        for(int i=0;i<nbSamples;i++)
        {
            int x=samples[i];
            if(x<xmin)
            {
                xmin=x;
                minIndx=i;
            }
            if(x>xmax) xmax=x;
        }
        //TesterControl::waitForAnyEvent();
        if(xmax-xmin<20) // just noise
        {
            shift++;
            continue;
        }
        if(xmin<20)
        { // gotcha
            offset=shift;
            return true;
        }
    }
    return false;
}
/**
 * 
 * @param adc
 * @param pin
 * @param timerScaler
 * @param timerOvf
 * @param prescaler
 * @param rate
 * @param offset
 * @return 
 */
bool  lookupForTime(DSOADC *adc,int pin, int timerScaler, int timerOvf, DSOADC::Prescaler  prescaler,   adc_smp_rate   rate , int offset ,uint16_t **samples, int &nbSamples ,int &sampling )
{
    sampling=1;
    while(1)
    {
        
        // ADC is running X cycles faster than repeat
        // Same thing as ~ adc running at  X Cycle
        if(!adc->prepareTimerSampling(timerScaler,timerOvf+sampling,false,rate,prescaler))
        {
            xAssert(0);
            return false;
        }
        adc->clearSemaphore();
        int before=millis();
        int nbSamples=1024;

        adc->startTimerSampling(nbSamples);


        // 5k => 200 us
        delayMicroseconds(offset);
        pwmRestart(pin);
        if(!adc->getSamples(samples,nbSamples))    
        {
            xAssert(0);
            adc->stopTimeCapture();
            return false;
        }
        adc->stopTimeCapture();
        pwmPause(pin);
        TesterGfx::drawCurve(nbSamples, *samples);
        
        // Look for min/max
        int xmin=4095;
        int xmax=0;
        int minIndx,maxIndx;
        for(int i=1;i<nbSamples;i++)
        {
            int x=(*samples)[i];
            if(x<xmin)
            {
                xmin=x;
                minIndx=i;
            }
            if(x>xmax) 
            {
                xmax=x;
                maxIndx=i;
            }
        }
        //TesterControl::waitForAnyEvent();
        if((xmax-xmin)<20) // just noise
        {
            sampling++;
            continue;
        }
        if((maxIndx-minIndx)<20)
        { // gotcha
            sampling++;
            continue;
        }
        return true;
    }
    return false;
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
int timeToCapture=0;
bool  TestPin::pulseTime(int nbSamples, int samplingFrequency, TestPin::PULL_STRENGTH strength,   int &sampleOut,  uint16_t **xsamples,int &samplingTime)
{
    disconnectAll();
    DSOADC::Prescaler  prescaler;
    adc_smp_rate   rate;    
    int pin;
    switch(strength)
    {
        case  PULL_LOW:  pin=_pinDriveLowRes;break;
        case  PULL_MED:  pin=_pinDriveMedRes;break;
        case  PULL_HI:   pin=_pinDriveHighRes;break;
        default:
            break;
    }
    
    pullDown(TestPin::PULL_LOW);
    xDelay(10);
    if(!  findRateScale(samplingFrequency, prescaler,rate))
    {
        xAssert(0);
        return false;
    }    
    
    pwm(strength,samplingFrequency);
    pwmPause(pin);
 
    int timerScaler;
    int timerOvf;
    
    pwmGetScaleOverFlow(samplingFrequency,timerScaler,timerOvf);
    
    adc->setADCPin(this->_pin);
    adc->setupTimerSampling();
    //
    int offset;
    if(!lookupForRampUp(adc,pin,   timerScaler,   timerOvf,   prescaler,       rate , offset  ))
    {
        return false;
    }
   
    if(!lookupForTime(adc,pin,   timerScaler,   timerOvf,   prescaler,       rate , offset ,xsamples,sampleOut,samplingTime ))
    {
        return false;
    }
   
   
    pullDown(strength);
    xDelay(10);
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
