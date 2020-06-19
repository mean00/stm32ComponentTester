/*
 * This controls one pin : pull it up, down, etc.. and measure
 * 
 */
#include "vector"
#include "testPins.h"
#include "dso_adc.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testerGfx.h"
#include "helpers/helper_pwm.h"

 DSOADC *adc;
uint32_t lastCR2=0;
#define WRITECR2(reg,x) {lastCR2=x;reg->CR2=x;}
void xFail(const char *message);
/**
 * 
 * @param p
 */
class AllPins
{
public:
    void registerMe(TestPin *p) {_pins.push_back(p);}
    
    // We dont allow one pin set to VCC and the other one set to ground
    // check for that
    void checkVcc(TestPin &me)
    {
        int n=_pins.size();
        for(int i=0;i<n;i++)
        {
            if(me._pinNumber==_pins[i]->_pinNumber) continue;
            if(_pins[i]->getState()==TestPin::GND)
                xFail("VCC NOT ALLOWED");
        }
    }        
    void checkGnd(TestPin &me)
    {
        int n=_pins.size();
        for(int i=0;i<n;i++)
        {
            if(me._pinNumber==_pins[i]->_pinNumber) continue;
            if(_pins[i]->getState()==TestPin::VCC)
                xFail("GND NOT ALLOWED");
        }
    }     
    void disconnectAll()
    {
        int n=_pins.size();
        for(int i=0;i<n;i++)
        {
            _pins[i]->disconnect();
        }
    }
    bool zero()
    {
        int n=_pins.size();
        for(int i=0;i<n;i++)
        {
            _pins[i]->pullDown(TestPin::PULL_LOW);
        }    
        xDelay(10);
        for(int i=0;i<n;i++)
        {
            _pins[i]->pullDown(TestPin::PULL_LOW);
        }

        int v,tus;
        for(int i=0;i<n;i++)
        {
            _pins[i]->fastSampleDown(PIN_ZERO_THRESHOLD,v,tus);
        }
      
        xDelay(10);
        return true;
    }
    
public:    
    std::vector<TestPin *>_pins;
};
AllPins allPins;

/**
 * 
 */
void zeroAllPins()
{
    TesterGfx::printStatus("Zeroing");
    allPins.zero();
}
/**
 */
#pragma once
#include "Arduino.h"
/**
 * 
 * @param pinNo
 * @param pin
 * @param pinDriveHighRes
 * @param pinDriveLow
 * @param lowRes
 * @param hiRes
 * @param internal
 */
 TestPin::TestPin(int pinNo, int pinAdc,int pinVolt, int pinDriveHighRes, int pinDriveMed,int pinDriveLow, int hiRes, int medRes,int lowRes)
{
     _pinNumber=pinNo;
     _pin=pinAdc;
     _pinVolt=pinVolt;
     _pinDriveHighRes=pinDriveHighRes;
     _pinDriveLowRes=pinDriveLow;
     _pinDriveMedRes=pinDriveMed;
     _lowRes=lowRes;
     _hiRes=hiRes;
     _medRes=medRes;
     
}
/**
 * 
 */
void TestPin::init()
{
      
     pinMode(_pin,OUTPUT);     
     digitalWrite(_pin,0);
     
     // Init PWM frequency for LOW RES
     setPWMPinFrequency(_pinDriveLowRes,52127); // 50 khz should be fast enough (?) we want something with a bit of noise in it
     
     pinMode(_pin,INPUT_ANALOG);     
     pinMode(_pinVolt,INPUT_PULLDOWN);
     pinMode(_pinDriveHighRes,INPUT_PULLDOWN);
     pinMode(_pinDriveLowRes,INPUT_PULLDOWN);
     pinMode(_pinDriveMedRes,INPUT_PULLDOWN);
    
    disconnect();
    allPins.registerMe(this);
    NVM::loadTestPin(this->_pinNumber,_calibration);
}
/**
 * 
 * @param pinNo
 * @param state
 */
void TestPin::configureOutput(int pinNo, int state)
{
    digitalWrite(pinNo,state);
    pinMode(pinNo,OUTPUT);
    digitalWrite(pinNo,state);
}
/**
 * 
 * @param mode
 */
void    TestPin::setMode(TESTPIN_STATE mode)
{
    switch(mode)
    {
            case PULLUP_PWM:        pullUp(PULL_LOW);
                                    pinMode(_pinDriveLowRes ,PWM);
                                    _state=PULLUP_PWM;
                                    break;
            case PULLUP_LOW:        pullUp(PULL_LOW);break;
            case PULLUP_MED:        pullUp(PULL_MED);break;
            case PULLUP_HI:         pullUp(PULL_HI);break;
//
            case PULLDOWN_LOW:        pullDown(PULL_LOW);break;
            case PULLDOWN_MED:        pullDown(PULL_MED);break;
            case PULLDOWN_HI:         pullDown(PULL_HI);break;    
//
            case   VCC:               setToVcc();break;
            case   GND:               setToGround();break;
            default:
                xFail("Invalid mode");
                break;
            
    }
}
/**
 * 
 * @param strength
 * @param fq
 */
void    TestPin::pwm(PULL_STRENGTH strength, int fq)
{
    // Assume it is low drive
    int pinNo=_pinDriveLowRes;
    
    
    digitalWrite(pinNo,1);
    pinMode(pinNo,PWM);
    digitalWrite(pinNo,1);
    pwmWrite(pinNo, 65535/2); // 50% duty cycle
    
}

/**
 * 
 * @param hiRes
 */
void    TestPin::pullUp(PULL_STRENGTH strength)
{
    disconnectAll();
    switch(strength)
    {
    case PULL_LOW:
        configureOutput(_pinDriveLowRes,1);
        _state=PULLUP_LOW;        
        break;
    case PULL_MED:
        configureOutput(_pinDriveMedRes,1);
        _state=PULLUP_MED;
        break;
    case PULL_HI:
        configureOutput(_pinDriveHighRes,1);
        _state=PULLUP_HI;        
        break;
    }
   
}
/**
 * 
 * @param hiRes
 */

void    TestPin::pullDown(PULL_STRENGTH strength)
{
    disconnectAll();
    switch(strength)
    {
    case PULL_LOW:
        configureOutput(_pinDriveLowRes,0);
        _state=PULLDOWN_LOW;
        break;
    case PULL_MED:
        configureOutput(_pinDriveMedRes,0);
        _state=PULLDOWN_MED;
        break;
    case PULL_HI:
        configureOutput(_pinDriveHighRes,0);
        _state=PULLDOWN_HI;
        break;
    }                
}
/**
 * 
 */
void    TestPin::setToVcc()
{
    allPins.checkVcc(*this);
    disconnectAll();
    digitalWrite(_pinVolt,1);
    pinMode(_pinVolt,OUTPUT);
    _state=VCC;    
}
/**
 * 
 */
void    TestPin::setToGround()
{
    allPins.checkGnd(*this);
    disconnectAll();
    digitalWrite(_pinVolt,0);
    pinMode(_pinVolt,OUTPUT);
    _state=GND;    
}
/**
 * 
 */
void    TestPin::disconnectAll()
{
    
    pinMode(_pin,INPUT_ANALOG); // protected by op amp
    pinMode(_pinDriveHighRes,INPUT_FLOATING);
    pinMode(_pinDriveLowRes,INPUT_FLOATING);
    pinMode(_pinDriveMedRes,INPUT_FLOATING);
    pinMode(_pinVolt,INPUT_FLOATING);
 
}       
/**
 * 
 */
void    TestPin::disconnect()
{
        disconnectAll();
        _state=DISCONNECTED;
}        
/**
 * 
 */
AutoDisconnect::~AutoDisconnect()
{
    allPins.zero();
    allPins.disconnectAll();    
}
/**
 * 
 * @param frequency
 * @return 
 */
bool     TestPin::prepareTimerSample(int frequency,int nbSamples)
{
    adc->setADCPin(_pin);    
    adc->setupTimerSampling();
    adc->prepareTimerSampling(frequency,false,ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_2);
    adc->clearSemaphore();
    return adc->startTimerSampling(nbSamples);
}

/**
 * 
 * @param nbSamples
 * @return 
 */
bool    TestPin::prepareDmaSample(adc_smp_rate rate,  DSOADC::Prescaler scale,int nbSamples)
{
    
    adc->setADCPin(_pin);    
    adc->setupDmaSampling();
    adc->prepareDMASampling(rate,scale);     
    adc->clearSemaphore();
    adc->startDMASampling(nbSamples);
    return true;    
}
/**
 * 
 * @param nbSamples
 * @param xsamples
 * @return 
 */
bool    TestPin::finishDmaSample(int &nbSamples, uint16_t **xsamples)
{
    xAssert(true==adc->getSamples(xsamples,nbSamples));
    return true;    
}
bool    TestPin::finishTimer(int &nbSamples, uint16_t **xsamples)
{
    return finishDmaSample(nbSamples,xsamples); 
}

/**
 * 
 * @param nbSamples
 * @return 
 */
bool    TestPin::prepareDualDmaSample(TestPin &otherPin,adc_smp_rate rate,   DSOADC::Prescaler scale ,int nbSamples)
{    
    adc->setADCPin(_pin); 
    adc->prepareFastDualDMASampling(otherPin._pin,rate,scale);     
    adc->clearSemaphore();
    adc->startDualDMASampling(otherPin._pin,nbSamples);
    return true;    
}
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
 * @param nbSamples
 * @param samplingFrequency
 * @param strength
 * @param sampleOut
 * @param xsamples
 * @return 
 */
int timeToCapture=0;
bool  TestPin::pulseTime(int nbSamples, int samplingFrequency, TestPin::PULL_STRENGTH strength,   int &sampleOut,  uint16_t **xsamples)
{
    
    DSOADC::Prescaler  prescaler;
    adc_smp_rate   rate;    
    pullDown(strength);
    xDelay(10);
    if(!  findRateScale(samplingFrequency, prescaler,rate))
    {
        xAssert(0);
        return false;
    }    
    
    adc->setADCPin(this->_pin);
    adc->setupTimerSampling();
    if(!adc->prepareTimerSampling(samplingFrequency,false,rate,prescaler))
    {
        xAssert(0);
        return false;
    }
    adc->clearSemaphore();
    int before=millis();
    adc->startTimerSampling(nbSamples);
    pullUp(strength);
    
    if(!adc->getSamples(xsamples,sampleOut))    
    {
        xAssert(0);
        adc->stopTimeCapture();
        return false;
    }
    int after=millis();
    timeToCapture=after-before;
    adc->stopTimeCapture();
    pullDown(strength);
    xDelay(10);
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
/**
 * 
 * @param adc
 * @param voltage
 */
bool    TestPin::slowDmaSample(int &xadc, int &nbSamples)
{
    uint16_t *samples;
    xadc=0;
    adc->setADCPin(_pin);    
    xDelay(10); // wait a bit
    adc->clearSamples();
    xAssert(prepareDmaSample(ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8 ,32));
    xAssert(true==adc->getSamples(&samples,nbSamples));
    int r=0;
   // if(nbSamples!=32) return false;
    for(int i=0;i<nbSamples;i++)
        r+=samples[i];
    xadc=r;
    return true;
}

/**
 * 
 * @param pin
 * @return 
 */
void TestPin::initADC(int pin)
{
  adc=new DSOADC(PA0);
  adc->setupADCs();
}

/**
 * 
 * @param threshold
 * @param value
 * @return 
 */
bool    TestPin::fastSampleUp(int threshold1,int threshold2,int &value1,int &value2, int &timeUs1,int &timeUs2)
{
    return adc->fastSampleUp(threshold1,threshold2,value1,value2,timeUs1,timeUs2);

}
/**
 * 
 * @param threshold
 * @param value
 * @return 
 */


bool    TestPin::fastSampleDown(int threshold,int &value, int &timeUs)  
{
    
    return adc->fastSampleDown( threshold,value, timeUs)   ; 
}

/**
 */
void xFail(const char *message)
{
    allPins.disconnectAll();
    __asm__  ("bkpt 1");
    while(1)
    {
        
    };
}
/**
 * 
 * @return 
 */
int TestPin::getCurrentRes()
{
    return getRes(_state);
}
/**
 * 
 * @param state
 * @return 
 */
int TestPin::getRes(TESTPIN_STATE state)
{
    switch(state)
    {
        case DISCONNECTED: 
                    xFail("Invalid");
        case VCC:
        case GND:
                    return _calibration.resDown;
                    break;
                    
        case PULLUP_HI:         return _hiRes+ _calibration.resUp;;break;
        case PULLUP_PWM:        
        case PULLUP_LOW:        return _lowRes+_calibration.resUp;break;
        case PULLUP_MED:        return _medRes+_calibration.resUp;break;
        case PULLDOWN_HI:       return _hiRes+ _calibration.resDown;;break;
        case PULLDOWN_LOW:      return _lowRes+ _calibration.resDown;;break;
        case PULLDOWN_MED:       return _medRes+ _calibration.resDown;;break;        
    }
    xFail("Invalid");
    return 0; 
}

/**
 * 
 * @param count
 * @return 
 */
bool TestPin::dualInterleavedDelta ( int &nbSamples,uint16_t *samples)
{
   
    volatile uint16_t *c=samples;   
    for(int i=0;i<nbSamples-1;i++)
    {
        int left,right;
        
        int base=2*i;
        left=(int)samples[base]+(int)samples[base+2]-(int)samples[base+1]*2;
        if(left<0) left=0;
        else left=left/2;
      
        right=(int)2*samples[base+2]-(int)samples[base+3]-(int)samples[base+1];
        if(right<0) right=0;
        else right=right/2;
      
        c[0]=left;
        c[1]=right;
        c+=2;
    }
  nbSamples=(nbSamples-1)*2;
  return true;
}


/**
 * 
 * @param count
 * @return 
 */
bool TestPin::evalInternalResistance ( int &resDown,int &resUp)
{
    int valUp,valDown;
    
    setMode(PULLUP_LOW);
    digitalWrite(_pinVolt,0);
    pinMode(_pinVolt,OUTPUT);

    xDelay(50);
    int sum,nb;
    xAssert(slowDmaSample(sum,nb));
    sum=(sum+nb/2)/nb;
    valDown=sum;    
    
    disconnect();
    setMode(PULLDOWN_LOW);
    digitalWrite(_pinVolt,1);
    pinMode(_pinVolt,OUTPUT);
    
     xDelay(50);
    xAssert(slowDmaSample(sum,nb));
    sum=(sum+nb/2)/nb;
    valUp=4095-sum;    
    
    disconnect();
    
    int totalRes=_lowRes;
    resDown=(totalRes*valDown)/4095;
    resUp=(totalRes*valUp)/4095;
    
    for(int i=0;i<5;i++) // converge to real value
    {
        totalRes=_lowRes+valUp+resDown;
        resDown=(totalRes*valDown)/4095;
        resUp  =(totalRes*valUp)/4095;
    }
    
    
    return true;
}


/**
 * 
 * @param count
 * @return 
 */
bool TestPin::dualSimulatenousDelta ( int &nbSamples,uint16_t *samples)
{
   
    volatile uint16_t *c=samples;   
    volatile uint16_t *o=samples;  
    for(int i=0;i<nbSamples;i++)
    {
        int left;
        left=(int)c[0]-(int)c[1];
        if(left<0) left=0;
      
        *o=left;
        o++;
        c+=2;
    }
  return true;
}

/**
 * 
 * @param value
 * @param otherResistance
 * @return 
 */
float TestPin::resistanceDivider(float value, float otherResistance)
{
    if(value>=4095.) return 10000000.;
    float r=value*otherResistance/(4095.-value);
    return r;
    
}


// EOF
