/**
 
 
 */
#pragma once
#include "Arduino.h"
#include "nvm.h"
#include "tester_constant.h"
#include "dso_adc.h"
#include "dso_debug.h"
#include "adc_limit.h"
#define ADC_OFFSET  0 

class AutoDisconnect
{
public:
        ~AutoDisconnect();
};

class TestPin
{
  friend class AllPins;
  friend class PulseSetting;
public:
        enum TESTPIN_STATE
        {
          IDLE=0,
          DISCONNECTED=1,
          PULLUP_HI=2,
          PULLUP_LOW=3,
          PULLDOWN_HI=4,
          PULLDOWN_LOW=5,
          PULLUP_MED=6,
          PULLDOWN_MED=7,
          PULLUP_PWM=8,
          VCC=10,
          GND=11                    
        };
  
        enum PULL_STRENGTH
        {
          PULL_LOW,
          PULL_MED,
          PULL_HI,
          PULL_NONE=99
        };
        
  
                TestPin(int pinNo, int pinAdc,int pinVolt, int pinDriveHighRes, int pinDriveMed,int pinDriveLow, int hiRes, int medRes,int lowRes);
        void    setMode(TESTPIN_STATE mode);
        void    init();
static  void    initADC(int pin);
        void    pullUp(PULL_STRENGTH strength);
        void    pwm(PULL_STRENGTH strength, int fq);
        void    pullDown(PULL_STRENGTH strength);
        void    setToVcc();
        void    setToGround();
        void    disconnect();
        bool    summedRead(int &adcSum, int &nbSamples); // Read and sum ~ 32 samples
        
                // For this one, we actually sample 2 pins alternatively : pin & otherPin
                // The nbSample asked is sample Pin + sample OtherPin, i.e. you'll get half of nbSample for pin1 and half of nbSample for otherPin
                // Odd / even. the first pair may be incorrect
        bool    prepareDualDmaSample(TestPin &otherPin,  adc_smp_rate rate,  DSOADC::Prescaler scale,int nbSamples);
        bool    prepareDualTimeSample(int fq,TestPin &otherPin,adc_smp_rate rate,   DSOADC::Prescaler scale ,int nbSamples);
        
        bool    prepareDmaSample(adc_smp_rate rate,  DSOADC::Prescaler scale,int nbSamples);        
        bool    prepareTimerSample(int frequency,int nbSamples);
        
        bool    finishTimer(int &nbSamples, uint16_t **xsamples);
        bool    finishDmaSample(int &nbSamples, uint16_t **xsamples);
        
        bool    pulseTimeDelta(TestPin &otherPin, int &clockPerSample,int nbSampleAsked, int samplingFrequency, TestPin::PULL_STRENGTH strength,   int &nbSample,  uint16_t **xsamples,int &res,bool highspeed);
        bool    pulseTime(int clockPerSample, int nbSampleAsked,int samplingFrequency, TestPin::PULL_STRENGTH strength, int &sampleOut,  uint16_t **xsamples,int &r);
        bool    pulseDma(int nbSamples,  DSOADC::Prescaler prescaler, adc_smp_rate   rate, TestPin::PULL_STRENGTH strength,   int &sampleOut,  uint16_t **xsamples);
        
        //
        bool    sample(int &value);
        bool    fastSampleUp(int threshold1,int threshold2,int &value1,int &value2, int &timeUs1,int &timeUs2);
        bool    fastSampleDown(int threshold,int &value, int &timeUs)  ;
        TESTPIN_STATE getState() {return _state;}
        void    disconnectAll();
        int     getCurrentRes();
        int     getRes(TESTPIN_STATE state);
        int     pinNumber() {return _pinNumber;}
        int     pinADC() {return _pin;}
        bool    dualInterleavedDelta (int &nbSamples,uint16_t *samples);
        bool    dualSimulatenousDelta (int &nbSamples,uint16_t *samples);

        // For calibration
        bool   evalInternalResistance ( int &resDown,int &resUp);
        
protected:  
 
        void configureOutput(int pinNo, int state);
        int _pinNumber,_pin,_pinDriveHighRes, _pinDriveLowRes,_pinDriveMedRes;
        int _pinVolt; // GND or VCC
        int _lowRes,_hiRes,_medRes;
        TESTPIN_STATE _state;
public:        
        TestPinCalibration _calibration;
        
        static float resistanceDivider(float value, float otherResistance);
        
};

void zeroAllPins();
#define pPICO (1000.*1000.*1000.*1000.)

// Value considered as valid


#define Debug Serial.print

// EOF
