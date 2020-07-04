#pragma once
#include "testPins.h"
#include "Component.h"
/**
 */
class Capacitor : public Component
{
public:    

            typedef struct CapScale
            {
                int                 fq;
                adc_smp_rate        rate;
                DSOADC::Prescaler   scale;
                TestPin::PULL_STRENGTH strength;
                bool                doubled;
            };
            typedef enum CapEval
            {
                EVAL_OK=0,
                EVAL_BIGGER_CAP=1,
                EVAL_SMALLER_CAP=2,
                EVAL_ERROR=99
            };
            struct CapCurve
            {
                int iMin;
                int iMax;
                int vMin;
                int vMax;
                float resistance;
                float period;
            };
public:                 
                    Capacitor( TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C,"Capacitor")
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
            CapEval eval(const CapScale &sc,CapCurve &curve, int &deltaTime);
            bool  computeMediumCap();
            bool  computeHiCap();
            bool  computeLowCap();
            bool  computeVeryLowCap();
            
            bool doOneQuick(TestPin::PULL_STRENGTH strength, bool doubled, float percent,int &timeUs, int &resistance,int &value);
            bool getRange(int dex, int &range);
            bool getEsr(float &esr);
            bool minMax(bool high,int &minmax);
            bool computeWrapper();
public:
            bool calibrationValue(float &c);
            bool quickEval(float &c);
            bool computed;
static      float computeCapacitance(int time, int iresistance, int actualValue);
static      float computeCapacitance(int nbSample, uint16_t *samples, int resistance, float period);
static      float computeCapacitance(CapCurve &curve);
};