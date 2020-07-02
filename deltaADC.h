#pragma once
#include "testPins.h"

/**
 */
class DeltaADC
{
public:  
    DeltaADC(TestPin &A, TestPin &B): _pA(A),_pB(B)
    {
      
    }
    virtual ~DeltaADC() {};
    bool setup(const adc_smp_rate rate, const   DSOADC::Prescaler  scale,const  int nbSamples);
    virtual bool get(int &nbSamples, uint16_t **ptr, float &period);
    
protected:
  TestPin            &_pA,&_pB;
  adc_smp_rate       _rate;
  DSOADC::Prescaler  _scale;
  int                _nb;
  
};
/**
 * 
 * @param A
 * @param B
 */
class DeltaADCTime : public DeltaADC
{
public:  
                DeltaADCTime(TestPin &A, TestPin &B);
    virtual     ~DeltaADCTime();
    bool        setup(int frequency,const  int nbSamples);
    virtual bool get(int &nbSamples, uint16_t **ptr, float &period);    
protected:  
  int                _fq;
};
