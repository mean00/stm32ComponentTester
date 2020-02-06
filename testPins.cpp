/*
 * This controls one pin : pull it up, down, etc.. and measure
 * 
 */
#include "vector"
#include "testPins.h"
#include "dso_adc.h"
#include "MapleFreeRTOS1000_pp.h"
extern DSOADC *adc;

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
public:    
    std::vector<TestPin *>_pins;
};
AllPins allPins;

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
     
#if 1
     pinMode(pinAdc,INPUT_PULLDOWN);     
     pinMode(pinVolt,INPUT_PULLDOWN);
     pinMode(pinDriveHighRes,INPUT_PULLDOWN);
     pinMode(pinDriveLow,INPUT_PULLDOWN);
     pinMode(pinDriveMed,INPUT_PULLDOWN);
#endif   
}
/**
 * 
 */
void TestPin::init()
{
    disconnect();
    allPins.registerMe(this);
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
    
    pinMode(_pin,INPUT_FLOATING);
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
    allPins.disconnectAll();
}
/**
 * 
 * @param nbSamples
 * @return 
 */
bool    TestPin::prepareDmaSample(adc_smp_rate rate,  adc_prescaler scale,int nbSamples)
{
    uint16_t *samples;
    adc->setADCPin(_pin);    
    adc->prepareDMASampling(rate,scale);     
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
    xAssert(prepareDmaSample(ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_8,32));
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
 * @param regs
 * @param v
 * @return 
 */
static bool singleShot(adc_reg_map *regs,int &v)
{
    int start=millis();
    uint32_t oldCr2=regs->CR2;
    uint32_t cr2=ADC_CR2_ADON+ADC_CR2_EXTSEL_SWSTART; //+/*ADC_CR2_EXTTRIG+*/ADC_CR2_CONT+ADC_CR2_DMA;  
    regs->CR2=cr2;  
    while(1)
    {
          uint32_t sr=regs->SR;
          if(!(sr & ADC_SR_EOC))
          {
              int now=millis();
              if((now-start)>10)
              {
                  regs->CR2 &= ~ADC_CR2_SWSTART;
                  return false;
              }
          }
          break;
    }
    v=regs->DR & ADC_DR_DATA;
    return true;
}
/**
 * 
 * @param threshold
 * @param value
 * @param timeUs
 * @return 
 */
adc_reg_map    *TestPin::fastSetup()  
{
/**
 *  ADC_SMPR_1_5,               **< 1.5 ADC cycles *
    ADC_SMPR_7_5,               **< 7.5 ADC cycles *
    ADC_SMPR_13_5,              **< 13.5 ADC cycles *
    ADC_SMPR_28_5,              **< 28.5 ADC cycles *
    ADC_SMPR_41_5,              **< 41.5 ADC cycles *
    ADC_SMPR_55_5,              **< 55.5 ADC cycles *
    ADC_SMPR_71_5,              **< 71.5 ADC cycles *
    ADC_SMPR_239_5,             **< 239.5 ADC cycles *
 */    
    adc->setTimeScale(ADC_SMPR_13_5, ADC_PRE_PCLK2_DIV_6); // Fastest to stay withing 14MH adc clock
    adc_dev *dev = PIN_MAP[_pin].adc_device;
    int channel=PIN_MAP[_pin].adc_channel;    
    adc_reg_map *regs = dev->regs;    
    adc_set_reg_seqlen(dev, 1);
    regs->SQR3 = channel;
    return regs;
}

/**
 * 
 * @param threshold
 * @param value
 * @return 
 */
bool    TestPin::fastSampleUp(int threshold1,int threshold2,int &value1,int &value2, int &timeUs1,int &timeUs2)
{
    
    adc_reg_map *regs=fastSetup();
    // go
    int c;
    uint32_t start=micros();
    uint32_t sampleTime;
    bool first=true;
    int value;
    while(1)
    {
        uint32_t oldCr2=regs->CR2;
        uint32_t cr2=ADC_CR2_ADON+1*ADC_CR2_EXTSEL_SWSTART+0*ADC_CR2_SWSTART; 
        regs->CR2=cr2;  
        uint32_t sampleStart=millis();
        while(1)
        {
            uint32_t sr=regs->SR;
            if(!(sr & ADC_SR_EOC))
            {
                int now=millis();
                if((now-sampleStart)>10)
                {
                    regs->CR2 &= ~ADC_CR2_SWSTART;
                    return false;
                }
            }
            sampleTime=micros();
            break;
        }
        value=regs->DR & ADC_DR_DATA;
        if(first)
        {
            if(value>threshold1)
            {
                timeUs1=sampleTime-start; 
                value1=value;
                first=false;
            }
        }
        else
        {
             if(value>threshold2)
            {
                timeUs2=sampleTime-start; 
                value2=value;
                return true;
            }
        }
     
    }
}
/**
 * 
 * @param threshold
 * @param value
 * @return 
 */


bool    TestPin::fastSampleDown(int threshold,int &value, int &timeUs)  
{
    adc_reg_map *regs=fastSetup();
    // go
    int c;
    uint32_t start=micros();
    while(1)
    {
        if(!singleShot(regs,c)) 
            return false;
        if(c<threshold)
        {
            timeUs=micros()-start; 
#warning fixme wrap around
            value=c;
            return true;
        }
    }
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
    switch(_state)
    {
        case DISCONNECTED: 
                    xFail("Invalid");
        case VCC:
        case GND:
                    return WIRE_RESISTANCE_AND_INTERNAL;
                    break;
        case PULLUP_HI:         return _hiRes+WIRE_RESISTANCE_AND_INTERNAL;break;
        case PULLUP_LOW:        return _lowRes+WIRE_RESISTANCE_AND_INTERNAL;break;
        case PULLUP_MED:        return _medRes+WIRE_RESISTANCE_AND_INTERNAL;break;
        case PULLDOWN_HI:       return _hiRes+WIRE_RESISTANCE_AND_INTERNAL;break;
        case PULLDOWN_LOW:      return _lowRes+WIRE_RESISTANCE_AND_INTERNAL;break;
        case PULLDOWN_MED:       return _medRes+WIRE_RESISTANCE_AND_INTERNAL;break;        
    }
    xFail("Invalid");
    return 0;
}

// EOF