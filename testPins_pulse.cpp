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
    bool  lookupForRampUp(         int &nbSamples,    uint16_t  **samples)
    {
        int round=0;
        int shift=0;
        while(1)
        {
next:
            // ADC is running X cycles faster than repeat
            // Same thing as ~ adc running at  X Cycle
            if(!adc->prepareTimerSampling(timerScaler,timerOvf+2,false,rate,prescaler))
            {
                xAssert(0);
                return false;
            }
            adc->clearSemaphore();
            int before=millis();
            nbSamples=1024;

            pwmQuickRestart quick(pin);
            adc->startTimerSampling(nbSamples);


            // 5k => 200 us
            delayMicroseconds(shift);
            quick.go();
            if(!adc->getSamples(samples,nbSamples))    
            {
                xAssert(0);
                adc->stopTimeCapture();
                return false;
            }
            adc->stopTimeCapture();
            pwmPause(pin);
            round++;
            if(round==1) continue;
            TesterGfx::drawCurve(nbSamples, *samples);
            while(1)
            {
            }
            
            {
            offset=round;
            return true;
            }
            // Look for min/max
            int xmin=4095;
            int xmax=0;
            int minIndx;
            for(int i=1;i<nbSamples;i++)
            {
                int x=(*samples)[i];
                if(x<xmin)
                {
                    xmin=x;
                    minIndx=i;
                }
                if(x>xmax) xmax=x;
            }            
            if(xmax-xmin<20 || xmin >1500) // just noise
            {
                shift+=INCREMENT;
                continue;
            }
            
            for(int i=xmin;i<xmax;i++)
            {
                int x=(*samples)[i];
                if(x>xmin+(xmax-xmin)/6)
                {
                    if(x>20) // too far
                    {
                        shift+=INCREMENT;
                        goto next;
                    }
                }
            }         
            
           
            // gotcha
            offset=shift;
            return true;

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
    bool  lookupForTime(uint16_t **samples, int &nbSamples  )
    {
        stroboscopeSampling=1;
        while(1)
        {

            // ADC is running X cycles faster than repeat
            // Same thing as ~ adc running at  X Cycle
            if(!adc->prepareTimerSampling( timerScaler,timerOvf+stroboscopeSampling,false,rate,prescaler))
            {
                xAssert(0);
                return false;
            }
            adc->clearSemaphore();
            int before=millis();        

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
                stroboscopeSampling++;
                continue;
            }
            if((maxIndx-minIndx)<20)
            { // gotcha
                stroboscopeSampling++;
                continue;
            }
            return true;
        }
        return false;
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
 * @param nbSamples
 * @param samplingFrequency
 * @param strength
 * @param sampleOut
 * @param xsamples
 * @return 
 */
int timeToCapture=0;
bool  TestPin::pulseTime(int nbSamples, int samplingFrequency, TestPin::PULL_STRENGTH strength,   int &nbSample,  uint16_t **xsamples,int &samplingTime,int &res)
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
    if(!settings.lookupForRampUp(nbSample,xsamples  ))
    {
        return false;
    }
#if 0   
    if(!settings.lookupForTime(xsamples,nbSample ))
    {
        return false;
    }
#endif   
   
    pullDown(strength);
    xDelay(10);
    
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
