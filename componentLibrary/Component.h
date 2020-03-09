
#pragma once
#include "testPins.h"
#include "testerGfx.h"
#include "deltaADC.h"

enum COMPONENT_TYPE
{
  COMPONENT_UNKNOWN=0,
  COMPONENT_OPEN=4,
  COMPONENT_RESISTOR=1,
  COMPONENT_CAPACITOR=2,
  COMPONENT_DIODE=3,
};

/**
 * 
 * @param A
 * @param B
 * @param C
 */
class Component
{
public:
                    Component(TestPin &A, TestPin &B,TestPin &C) : _pA(A),_pB(B),_pC(C)
                    {
                    }
            virtual bool compute()=0;
            virtual bool draw(int yOffset)=0;
            static  void prettyPrint(float value, const char *unit,  char *output);
            virtual int  nbPins() {return 2;};
            static  void prettyPrintPrefix(const char *prefix,float value, const char *unit,  char *output);
            static  float adcToVolt(float adc);
protected:
            TestPin &_pA, &_pB, &_pC;
            
public:
    static          Component *identity(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type);
    static          bool      computeDiode(TestPin &Anode, TestPin &Cathode,float vfOut);
    
protected:
    static          Component *identify3poles(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type);
    static          Component *identify2poles(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type);
};