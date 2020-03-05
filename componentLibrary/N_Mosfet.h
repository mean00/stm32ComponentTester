#pragma once
#include "testPins.h"
#include "Component.h"
#include "Mosfet.h"
/**
 */
class NMosFet : public  Mosfet
{
public:                 
                    // Order is Gate, Drain Source
                    NMosFet( TestPin &A, TestPin &B,TestPin &C) :  Mosfet(A,B,C)
                    {
                      _capacitance=0;
                      _rdsOn=0;
                      _diodeVoltage=0;
                      _vGsOn=0;
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
