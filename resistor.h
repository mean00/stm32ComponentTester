#pragma once
#include "testPins.h"
#include "component.h"
/**
 */
class Resistor : public Component
{
public:                 
                    Resistor( TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C)
                    {
                      resistance=0;
                    }
            virtual bool compute()            ;
            virtual bool draw(Ucglib *u,int yOffset);
                    int  getValue() {return resistance;}
            
protected:
            int resistance;
            float twoPinsResistor(TestPin::PULL_STRENGTH strength, TestPin &A, TestPin &B);
};