#pragma once
#include "testPins.h"
#include "Component.h"
/**
 */
class Coil : public Component
{
public:                 
                    Coil( TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C)
                    {
                      inductance=0;
                      resistance=0;
                    }
            virtual bool compute()            ;
            virtual bool draw(int yOffset);
                    int  getValue() {return inductance;}
                    int  getRValue() {return resistance;}
            
protected:
            float inductance,resistance;
            bool  doOne(float target,int dex, float &cap);
            
            bool doOneQuick(TestPin::PULL_STRENGTH strength, bool doubled, float percent,int &timeUs, int &resistance,int &value);
            bool computeResistance();
};
