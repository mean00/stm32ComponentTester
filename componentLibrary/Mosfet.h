
#pragma once
#include "testPins.h"
#include "Component.h"



class Mosfet : public Component
{
public:
    Mosfet(TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C,"MosFet")
    {
        _capacitance=0;
        _rdsOn=0;
        _diodeVoltage=0;
        _vGsOn=0;
        _maxPage=2;
    
    }
    int  nbPins() {return 3;};
    bool computeDiode(TestPin &top, TestPin &bottom,float &value);
    bool computeRdsOn(TestPin &top, TestPin &bottom,float &value);
    bool computeCg(TestPin &top, TestPin &bottom,float &value);
    virtual bool  compute();
            void  changePage(int count);
public:
    virtual bool draw(int yOffset);
    virtual bool draw2(int yOffset)=0;
    virtual bool computeDiode()=0;
    virtual bool  computeRdsOn()=0;
    virtual bool  computeVgOn()=0;
    virtual bool  computeCg()=0;
    int  likely() {return 100;} // Highly likely
    
    
protected:
            adc_smp_rate evaluateSampleRate();
protected:
        float _rdsOn;
        float _diodeVoltage;
        float _capacitance;
        float _vGsOn;
};