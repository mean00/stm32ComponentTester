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
#define INTER     18
  
/**
 * 
 * @param cal
 * @param p1
 * @param p2
 */
void veryLowCapCalibration(int pinNumber,TestPinCalibration &cal,TestPin &pin1,TestPin &pin2)
{
#if 1
        float cap;
        char st[20];
        for(int i=0;i<CALIBRATION_VERY_SMALL_SIZE;i++)
        {
            int mul16;
            Capacitor::calibrationVeryLow(i,pin1,pin2,mul16);
            TesterGfx::clear();
            sprintf(st,"Fine Pin%d",pinNumber);
            TesterGfx::title(st);
            sprintf(st," %d/%d",i+1,CALIBRATION_VERY_SMALL_SIZE);
            TesterGfx::print(5,INTER*3,st);
            
            sprintf(st,"C=%2.2f pf",(float)mul16/16.);
            TesterGfx::print(5,INTER*4,st);    
            cal.capOffsetHighInPfMu16[i]=mul16;
        }
#endif        
}

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
    // They are in the 200 pf range
    //--------------------------------------
    
    TesterGfx::clear();
    TesterGfx::title("Cap");

    
    ALLCAP(1,pin1,pin2,pin3);
    ALLCAP(2,pin2,pin1,pin3);
    ALLCAP(3,pin3,pin2,pin1);
    
    TesterGfx::bottomLine("Press button");
    TesterControl::waitForAnyEvent();

    
    // Cap calibration for lower value i.e. stroboscopic mode
    // below ~ 200 Pf
    // One of the pin must be the 2nd one
    
    
    
    // Stroboscopic calibration, only on pin2
    TesterGfx::clear();
    TesterGfx::title("Fine Cap");
    TesterGfx::print(5,INTER*2,"This is slow");
    TesterGfx::print(5,INTER*3,"Please wait");
    
    pin1.setToGround();
    pin3.setToGround();
    pin2.setToGround();
    
    xDelay(10);
    
    veryLowCapCalibration(1,calibration1,pin1,pin2);
    veryLowCapCalibration(3,calibration3,pin3,pin2);

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