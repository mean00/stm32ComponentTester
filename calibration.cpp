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
extern bool pinTest();

/**
 * 
 */

void title(const char *v)
{
    TesterGfx::title(v);
}
#define TINTER(x) (36+16*x)
#define ZINTER(x) (32+18*x)
     
void calibration()
{    
    TesterGfx::title("Calibration");
    
    TesterGfx::drawZif();
    
    TesterGfx::print(10,TINTER(0),"Clear socket");
    TesterGfx::print(10,TINTER(1),"and press ");
    TesterGfx::print(10,TINTER(2),"button");
    TesterControl::waitForAnyEvent();
     
    
    

    //--------------------------------------
    // Check pins are correctly connected
    //--------------------------------------
     if(!pinTest()) 
     {
         TesterGfx::print(1,120,"*** FAIL ***");    
         while(1)
         {
             
         }
     }
     
     //---------------------------------------     
     // Eval internal up & down resistances
     // Around 20 Ohms
     //---------------------------------------
     TesterGfx::clear();
     title("Resistor");
    
    int resup3,resdown3;
    int resup1,resdown1;
    int resup2,resdown2;
    pin1.evalInternalResistance (resdown1,resup1);
    pin2.evalInternalResistance (resdown2,resup2);
    pin3.evalInternalResistance (resdown3,resup3);
    
    TestPinCalibration calibration1,calibration2,calibration3;
    
    char st[20];
    
#define ALLRES(x) {calibration##x.resUp=resup##x;calibration##x.resDown=resdown##x;sprintf(st,"RU:%d:RD:%d",resup##x,resdown##x),TesterGfx::print(1,ZINTER(x-1),st);}
    
    ALLRES(1);
    ALLRES(2);
    ALLRES(3);
    
    TesterGfx::bottomLine("Press button");
    TesterControl::waitForAnyEvent();
    
    float val;
#define ALLCAP(pin,a,b,c) {   \
                                float avgCap=0;\
                                for(int i=0;i<8;i++) \
                                {\
                                    float cap;\
                                    Capacitor::calibrationLow(a,b,cap); \
                                    avgCap+=cap;\
                                }\
                                avgCap/=8.;\
                                calibration##pin.capOffsetInPf=(int)(avgCap*pPICO+0.49);\
                                sprintf(st,"%d pf",calibration##pin.capOffsetInPf);TesterGfx::print(1,ZINTER(pin),st); \
                                }
    
    
    //--------------------------------------
    // Eval internal cap in // with the test pins
    // They are in the 20 pf range
    //--------------------------------------
    
    TesterGfx::clear();
    TesterGfx::title("Cap");

    
    ALLCAP(1,pin1,pin2,pin3);
    ALLCAP(2,pin2,pin1,pin3);
    ALLCAP(3,pin3,pin2,pin1);
    
    TesterGfx::bottomLine("Press button");
    TesterControl::waitForAnyEvent();

    
#if 0
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
    int nbSamples;
    uint16_t *samples;
    float c=0;
    // Dummy first one
    if(!pin2.pulseTime(8,50,fq,TestPin::PULL_HI,nbSamples,&samples,resistance))
    {
       
    }
    else
    {
    #define AVG 8
        c=0;
        for(int i=0;i<AVG;i++)
        {
            pin2.pulseTime(8,1024,fq,TestPin::PULL_HI,nbSamples,&samples,resistance);
            // compute C
            float period=F_CPU;
            period=(float)(8)/period;
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
#endif    
    // and save
    TesterGfx::clear();
    TesterGfx::title("Saving");

    
    NVM::reset();
    NVM::saveTestPin(1,calibration1);
    NVM::saveTestPin(2,calibration2);
    NVM::saveTestPin(3,calibration3);
    NVM::doneWriting();    
    TesterGfx::bottomLine("RESTART");
    TesterControl::waitForAnyEvent();
    while(1)
    {
        
    }
    
}