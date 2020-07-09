/*
 * Do pinTest and basic auto calibration
 * 
*/


#include "fancyLock.h"
#include "testPins.h"
#include "allComponents.h"
#include "testerGfx.h"
#include "nvm_default.h"
#include "testerControl.h"
extern TestPin pin1,pin2,pin3;
extern void pinTest();
#define Y_OFFSET 20
/**
 * 
 */
void calibration()
{    
#define INTER 26        
    //--------------------------------------
    // Check pins are correctly connected
    //--------------------------------------
     pinTest();
     TesterGfx::clear();
     TesterGfx::print(5,0,"Basic ");
    
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
                                cap.compute1nfRange(val);\
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
    TesterGfx::print(5,INTER,"Fine cal");
    TesterGfx::print(5,INTER*2,"(30sec)");
    pin1.setToGround();
    pin3.setToGround();
    // Only pin2 is able to do stroboscopic
    calibration1.capOffsetHighInPfMu16=INTERNAL_CAPACITANCE_IN_PF_HIGH*16;
    calibration2.capOffsetHighInPfMu16=INTERNAL_CAPACITANCE_IN_PF_HIGH*16;
    calibration3.capOffsetHighInPfMu16=INTERNAL_CAPACITANCE_IN_PF_HIGH*16;
    int fq=2000; // 2Khz
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
    float c=0;
    // Dummy first one
    if(!pin2.pulseTime(50,fq,TestPin::PULL_HI,nbSamples,&samples,samplingTime,resistance))
    {
       
    }
    else
    {
    #define AVG 8
        c=0;
        for(int i=0;i<AVG;i++)
        {
            pin2.pulseTime(1024,fq,TestPin::PULL_HI,nbSamples,&samples,samplingTime,resistance);
            // compute C
            float period=F_CPU;
            period=(float)(samplingTime)/period;
            c+=Capacitor::computeCapacitance(  nbSamples,  samples, resistance,period);
        }
        
        c=c/(float)AVG;
        calibration2.capOffsetHighInPfMu16=(uint16_t)(c*pPICO*16);;
    }
    TesterGfx::clear();
    char str[50];
    sprintf(str,"C=%d pF",calibration2.capOffsetHighInPfMu16);
    TesterGfx::print(5,INTER*1,str);
    sprintf(str,"C=%f pF",c);
    TesterGfx::print(2,INTER*3,str);
    
    // and save
    NVM::reset();
    NVM::saveTestPin(1,calibration1);
    NVM::saveTestPin(2,calibration2);
    NVM::saveTestPin(3,calibration3);
    NVM::doneWriting();    
    TesterControl::waitForAnyEvent();
    
}