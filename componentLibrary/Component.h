
#pragma once

#include "testPins.h"
#include "testerGfx.h"
#include "deltaADC.h"
#include "printf.h"
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
                    Component(TestPin &A, TestPin &B,TestPin &C,const char *shortName) : _pA(A),_pB(B),_pC(C),_shortName(shortName)
                    {
                      _maxPage=0,
                      _curPage=0;        
                    }
            virtual const char *getShortName() {return _shortName;};
            virtual bool compute()=0;
            virtual bool draw(int yOffset)=0;
            static  void prettyPrint(float value, const char *unit,  char *output);
            virtual int  nbPins() {return 2;};
            static  void prettyPrintPrefix(const char *prefix,float value, const char *unit,  char *output);
            static  float adcToVolt(float adc);
            static  int  evaluate(TestPin &pin);
            virtual int  likely()=0; // higher means detection is more reliable
            virtual void  changePage(int count) {};            
protected:
            TestPin &_pA, &_pB, &_pC;
            const char    *_shortName;
            
public:
    static          Component *identity2(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type);
    static          Component *identity3(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type);    
    static          bool      computeDiode(TestPin &Anode, TestPin &Cathode,float &vfOut);
    
protected:
    static          Component *identify3poles(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type);
    static          Component *identify2poles(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type);
protected:
            int  _maxPage;
            int  _curPage;            
};
