#pragma once
#include "testPins.h"
#include "Component.h"
#include "Capacitor.h"
/**
 */
class Coil : public Component
{
public:                 
                    Coil( TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C,"Coil")
                    {
                      inductance=0;
                      resistance=0;
                    }
            virtual bool compute()            ;
            virtual bool draw(int yOffset);
                    int  getValue() {return inductance;}
                    int  getRValue() {return resistance;}
                    int  likely() {return 10;}
            
protected:
            float inductance,resistance;
            bool  doOne(float target,int dex, float &cap);
            
            bool doOneQuick(TestPin::PULL_STRENGTH strength, bool doubled, float percent,int &timeUs, int &resistance,int &value);
            bool computeResistance();
            bool computeInductance(int range,int &minIndex, int &maxIndex,float &ductance);
            
            Capacitor::CapEval evalSmall(  TestPin *p1,TestPin *p2,int fq, int clockPerSample, float &inductance);
            
};
