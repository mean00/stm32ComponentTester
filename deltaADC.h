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
    bool setup(const adc_smp_rate rate, const   DSOADC::Prescaler  scale,const  int nbSamples);
    bool get(int &nbSamples, uint16_t **ptr, float &period);
    
protected:
  TestPin            &_pA,&_pB;
  adc_smp_rate       _rate;
  DSOADC::Prescaler  _scale;
  int                _nb;
  
};