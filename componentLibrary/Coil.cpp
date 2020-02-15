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



typedef struct coilScale
{   
    adc_smp_rate    rate;
    adc_prescaler   scale;
    float           tickUs;
};
const coilScale coilScales[]=
{
    { ADC_SMPR_1_5,  ADC_PRE_PCLK2_DIV_6,   1.17},  // T=500 us, LMax =~ 50 mH
    { ADC_SMPR_41_5,  ADC_PRE_PCLK2_DIV_6,  4.5},   // T=2.3 ms, LMax =~ 200 mH
    { ADC_SMPR_239_5,  ADC_PRE_PCLK2_DIV_8, 28.},  //  T=14 ms   LMax =~ 1500 mH
};

#define LAST_SCALE ((sizeof(coilScales)/sizeof(coilScale))-1)


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
    sprintf(st3,"%s \n R=%s",st,st2);
    TesterGfx::drawCoil(yOffset, st3,_pA.pinNumber(), _pB.pinNumber());
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
    if(r>3500) // above = very high resistance > 2.7 k
    {
        resistance=0;        
        return false;
    }
    float alpha=r/4095.;    
    resistance=(alpha*Ra+Rb*(1.-alpha))/(1-alpha);
    return true;
}
/**
 * 
 * @return 
 */
bool Coil::computeInductance()
{    
    int range=0;
    int nbSamples;
    uint16_t *samples=NULL;
    float Ra,Rb;
   // first compute resistance
    zeroAllPins();
    _pB.setToGround();
    if(!_pA.prepareDmaSample(coilScales[range].rate,coilScales[range].scale,512))
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
    
  // Rescale to get voltage across coil
    // taking internal resitance into account
    float beta=(resistance+Rb)/Ra;
    float offset=4095.*beta;
    float mul=1.+beta;
    for(int i=0;i<nbSamples;i++)
    {
        float z=samples[i];
        z=z*mul-offset;
        if(z<0) z=0;
        if(z>4095) z=4095;
        samples[i]=(uint16_t)(z+0.49);        
    }
    // Look up the max, happens withing the 200 first samples
    int zmax=0,zmin;
    int top=-1,bottom=-1;
    for(int i=0;i<200;i++)
    {
        if(samples[i]>zmax)
        {
            zmax=samples[i];
            top=i;
        }
    }
    if(zmax<3000) return false; // something is wrong
    // take a 2nd point at less than 10%
    for(int i=top+1;i<(nbSamples-1) && bottom==-1;i++)
    {
        if(samples[i]<409 ) // must not be zero !
        {
            zmin=samples[i];
            bottom=i;
        }
    }
    
    
    
    float totalResitance=Ra+Rb+resistance;
    float totalTime=(bottom-top)*coilScales[range].tickUs;
    float den=-1.*log((float)zmin/(float)zmax);
    
    inductance=(totalResitance*totalTime)/den;
    inductance=inductance/1000000.; // us -> second    
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
    
    if(!computeInductance())
        return false;
    
    return true;
}

// EOF
