/*
 * Coil tester
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "Coil.h"
#include "math.h"
#include "calibration.h"
#include "cycleClock.h"
#include "MapleFreeRTOS1000_pp.h"
/**
 * 
 * @param yOffset
 * @return 
 */
bool Coil::draw(int yOffset)
{
    char st[32];        
    char st2[32];    
    char st3[64];    
    Component::prettyPrint(inductance, "H",st);
    Component::prettyPrint(resistance, "O",st2);
    sprintf(st3,"%s \n %s",st,st2);
    TesterGfx::drawCoil(yOffset, st3,_pA.pinNumber(), _pB.pinNumber());
    return true;
}

/**
 * Perform a DMA sampling buffer and extract 2 points
 * @return 
 */

bool Coil::doOne(float target,int dex, float &cap)
{
    int resistance;
    zeroAllPins();
    // go
    bool doubled=false;
    TestPin::PULL_STRENGTH strength=TestPin::PULL_LOW;
    if(doubled)
        _pB.pullDown(strength);
    else    
        _pB.setToGround();

    // start the DMA
    // max duration ~ 512 us
    uint16_t *samples;
    int nbSamples;
    
    if(!_pA.prepareDmaSample(ADC_SMPR_1_5,ADC_PRE_PCLK2_DIV_6,512)) 
        return false;        
    // Go!
    _pA.pullUp(strength);   
    if(!_pA.finishDmaSample(nbSamples,&samples)) 
    {
        return false;
    }
    resistance=_pA.getCurrentRes()+_pB.getCurrentRes();
    _pA.pullDown(TestPin::PULL_LOW);   
    
    
    int limitA,limitB;

    
    limitA=10;
    limitB=4095.*target;

    if(doubled)
    {
        limitA=4095/2+10;
        limitB=4095.*(1.+target)/2.+1;
    }
  
    
    // We need 2 points...
    // Lookup up 5% and 1-1/e
    int pointA=-1,pointB=-1;
    for(int i=1;i<nbSamples;i++)
    {
        if(samples[i]>limitA) // 5%
        {
            pointA=i;
            i=4095;
        }
    }
    if(pointA==-1 || pointA >512) 
        return false;
    for(int i=pointA+1;i<nbSamples;i++)
    {
        if(samples[i]>limitB) // 68%
        {
            pointB=i;
            i=4095;
        }
    }
    if(pointB==-1) pointB=nbSamples-1;
    
    if((pointB-pointA)<1) return false; // not enough points, need at least one
    
    // Compute
    float timeElapsed=(pointB-pointA);
    timeElapsed*=2.17;
    timeElapsed/=1000000.; // In seconds

    float valueA=samples[pointA];
    float valueB=samples[pointB];

    
    if(doubled)
    {
        valueA=2*valueA-4095;
        valueB=2*valueB-4095;
    }
    
    if(fabs(valueA-4095.)<2) return false;
    if(fabs(valueB-4095.)<2) return false;
    
    float den=(4095.-(float)valueA)/(4095.-(float)valueB);
    
    if(fabs(den-2.718)<0.01) 
        return false;
    den=log(den);
    cap=timeElapsed/(resistance*den);
    return true;
}
/**
 * 
 * @return 
 */

/**
 * 
 * @return 
 */
bool Coil::computeResistance()
{
    int nbSamples;
    uint16_t *samples=NULL;
    float Ra,Rb;
   // first compute resistance
    zeroAllPins();
    _pB.setToGround();
    if(!_pA.prepareDmaSample(ADC_SMPR_239_5,ADC_PRE_PCLK2_DIV_6,512)) 
        return false;        
    // Go!
    _pA.pullUp(TestPin::PULL_LOW);   
    if(!_pA.finishDmaSample(nbSamples,&samples)) 
    {
        return false;
    }    
    Ra=_pA.getCurrentRes();
    Rb=_pB.getCurrentRes();
            
    _pA.pullDown(TestPin::PULL_LOW);   
    
    // The last value are the resitance divider
    
    float r=(samples[nbSamples-2]+samples[nbSamples-1])/2;
    if(r<4090)
    {
        float alpha=r/4095.;    
        resistance=(alpha*Ra+Rb*(1.-alpha))/(1-alpha);
    }else
    {
        resistance=0;
    }
    return true;
}
/**
 * 
 * @return 
 */
bool Coil::compute()
{
    AutoDisconnect ad;
    
    
    inductance=0;
    resistance=0;
    
    if(!computeResistance())
        return false;
    
    
    
    return true;
}

// EOF
