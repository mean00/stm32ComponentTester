/*
*/


#include "fancyLock.h"
#include "testPins.h"
#include "allComponents.h"
#include "testerGfx.h"
extern TestPin pin1,pin2,pin3;
extern void pinTest();
#define Y_OFFSET 20
void calibration()
{    
     pinTest();
    
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
    
    NVM::reset();
    NVM::saveTestPin(1,calibration1);
    NVM::saveTestPin(2,calibration2);
    NVM::saveTestPin(3,calibration3);
    NVM::doneWriting();    
    
}