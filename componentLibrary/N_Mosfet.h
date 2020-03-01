#pragma once
#include "testPins.h"
#include "Component.h"
/**
 */
class NMosFet : public Component
{
public:                 
                    // Order is Gate, Drain Source
                    NMosFet( TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C)
                    {
                      _capacitance=0;
                      _rdsOn=0;
                      _diodeVoltage=0;
                      _vGsOn;
                    }
            virtual bool compute()            ;
            virtual bool draw(int yOffset);
                    
            
protected:
            float _rdsOn;
            float _diodeVoltage;
            float _capacitance;
            float _vGsOn;
            
            bool  computeDiode();
            bool  computeRdsOn();
            bool  computeVgOn();
public:
            
};
