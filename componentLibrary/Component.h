
#pragma once
#include "testPins.h"
#include "testerGfx.h"
#include "deltaADC.h"

enum COMPONENT_TYPE
{
  COMPONENT_OPEN=0,
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
            static  void prettyPrintPrefix(const char *prefix,float value, const char *unit,  char *output);
                    float adcToVolt(float adc);
protected:
            TestPin &_pA, &_pB, &_pC;
            
public:
    static          Component *identity(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type);
};