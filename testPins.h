/**
 
 
 */
#pragma once
#include "Arduino.h"
#include "nvm.h"
#include "tester_constant.h"
#include "debug_conf.h"

#define ADC_OFFSET  0 

class AutoDisconnect
{
public:
        ~AutoDisconnect();
};

class TestPin
{
  friend class AllPins;
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
          VCC=10,
          GND=11                    
        };
  
        enum PULL_STRENGTH
        {
          PULL_LOW,
          PULL_MED,
          PULL_HI
        };
        
  
                TestPin(int pinNo, int pinAdc,int pinVolt, int pinDriveHighRes, int pinDriveMed,int pinDriveLow, int hiRes, int medRes,int lowRes);
        void    setMode(TESTPIN_STATE mode);
        void    init();
static  void    initADC(int pin);
        void    pullUp(PULL_STRENGTH strength);
        void    pullDown(PULL_STRENGTH strength);
        void    setToVcc();
        void    setToGround();
        void    disconnect();
        bool    slowDmaSample(int &adcSum, int &nbSamples);
        bool    finishDmaSample(int &nbSamples, uint16_t **xsamples);
        bool    prepareDmaSample(adc_smp_rate rate,  adc_prescaler scale,int nbSamples);
        bool    prepareDualDmaSample(TestPin &otherPin,  adc_smp_rate rate,  adc_prescaler scale,int nbSamples);
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
#define HIGH_CEIL 3900
#define LOW_FLOOR 10

// EOF
