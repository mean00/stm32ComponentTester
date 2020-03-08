
#pragma once
#include "testPins.h"
#include "Component.h"

class Mosfet : public Component
{
public:
    Mosfet(TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C)
    {
        _capacitance=0;
        _rdsOn=0;
        _diodeVoltage=0;
        _vGsOn=0;
    }
    int  nbPins() {return 3;};
    bool computeDiode(TestPin &top, TestPin &bottom,float &value);
    bool computeRdsOn(TestPin &top, TestPin &bottom,float &value);
    bool computeCg(TestPin &top, TestPin &bottom,float &value);
    virtual bool compute();
public:
    virtual bool computeDiode()=0;
    virtual bool  computeRdsOn()=0;
    virtual bool  computeVgOn()=0;
    virtual bool  computeCg()=0;

protected:
            adc_smp_rate evaluateSampleRate();
protected:
        float _rdsOn;
        float _diodeVoltage;
        float _capacitance;
        float _vGsOn;
};