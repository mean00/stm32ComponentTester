#pragma once
#include "testPins.h"
#include "Component.h"
/**
 */
class Capacitor : public Component
{
public:                 
                    Capacitor( TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C)
                    {
                      capacitance=0;
                      computed=false;
                    }
            virtual bool compute()            ;
            virtual bool draw(int yOffset);
                    int  getValue() {return capacitance;}
                    int  likely() {return 50;} // medium likely
            
protected:
            float capacitance;
            bool  doOne(float target,int dex, float &cap);
            float computeCapacitance(int time, int iresistance, int actualValue);
            bool  computeMediumCap(int dex,int overSampling,float &c);
            bool  computeHiCap();
            bool  computeLowCap();
            
            bool doOneQuick(TestPin::PULL_STRENGTH strength, bool doubled, float percent,int &timeUs, int &resistance,int &value);
            bool getRange(int dex, int &range);
            bool getEsr(float &esr);
            bool minMax(bool high,int &minmax);
            bool computeWrapper();
public:
            bool calibrationValue(float &c);
            bool quickEval(float &c);
            bool computed;
};