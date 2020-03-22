#pragma once
#include "testPins.h"
#include "Component.h"
/**
 */
class Diode : public Component
{
public:                
                    // PinA is Anode, pinB is cathode
                    Diode( TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C)
                    {
                      forward=0;
                    }
            virtual bool compute()            ;
            virtual bool draw(int yOffset);
                    int  getValue() {return forward;}
                    int  likely() {return 100;} // very likely
            
protected:
            float forward;  
};