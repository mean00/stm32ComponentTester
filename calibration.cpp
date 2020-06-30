/*
 * Do pinTest and basic auto calibration
 * 
*/


#include "fancyLock.h"
#include "testPins.h"
#include "allComponents.h"
#include "testerGfx.h"
#include "nvm_default.h"
extern TestPin pin1,pin2,pin3;
extern void pinTest();
#define Y_OFFSET 20
/**
 * 
 */
void calibration()
{    
    
    //--------------------------------------
    // Check pins are correctly connected
    //--------------------------------------
     pinTest();
    
     //---------------------------------------     
     // Eval internal up & down resistances
     // Around 20 Ohms
     //---------------------------------------
    int resup3,resdown3;
    int resup1,resdown1;
    int resup2,resdown2;
    pin1.evalInternalResistance (resdown1,resup1);
    pin2.evalInternalResistance (resdown2,resup2);
    pin3.evalInternalResistance (resdown3,resup3);
    
    TestPinCalibration calibration1,calibration2,calibration3;
    
#define ALLRES(x) calibration##x.resUp=resup##x;calibration##x.resDown=resdown##x;
    
    ALLRES(1);
    ALLRES(2);
    ALLRES(3);
    
    float val;
#define ALLCAP(pin,a,b,c) {   \
                                Capacitor cap(a,b,c); \
                                cap.calibrationValue(val);\
                                calibration##pin.capOffsetInPf=(int)(val*pPICO+0.49);\
                                }
    
    //--------------------------------------
    // Eval internal cap in // with the test pins
    // They are in the 20 pf range
    //--------------------------------------
    ALLCAP(1,pin1,pin2,pin3);
    ALLCAP(2,pin2,pin1,pin3);
    ALLCAP(3,pin3,pin2,pin1);
    
    // Stroboscopic calibration, only on pin2
    pin1.setToGround();
    pin3.setToGround();
    // Only pin2 is able to do stroboscopic
    calibration1.capOffsetHighInPf=INTERNAL_CAPACITANCE_IN_PF_HIGH;
    calibration3.capOffsetHighInPf=INTERNAL_CAPACITANCE_IN_PF_HIGH;
    int fq=2000; // 2Khz
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
    
     if(!pin2.pulseTime(50,fq,TestPin::PULL_HI,nbSamples,&samples,samplingTime,resistance))
    {
       
    }
    if(!pin2.pulseTime(1024,fq,TestPin::PULL_HI,nbSamples,&samples,samplingTime,resistance))
    {
        calibration2.capOffsetHighInPf=INTERNAL_CAPACITANCE_IN_PF_HIGH;
    }else
    {
        // compute C
        float period=F_CPU;
        period=(float)(samplingTime)/period;
        float c=Capacitor::computeCapacitance(  nbSamples,  samples, resistance,period);
        c=c*pPICO+0.49;
        calibration2.capOffsetHighInPf=c;
    }
    // and save
    NVM::reset();
    NVM::saveTestPin(1,calibration1);
    NVM::saveTestPin(2,calibration2);
    NVM::saveTestPin(3,calibration3);
    NVM::doneWriting();    
    
}