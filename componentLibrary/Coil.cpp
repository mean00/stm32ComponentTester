/*
 * Coil tester
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "Coil.h"
#include "math.h"

#include "cycleClock.h"
#include "MapleFreeRTOS1000_pp.h"
#include "deltaADC.h"


typedef struct coilScale
{   
    adc_smp_rate        rate;
    DSOADC::Prescaler   scale;
    float               tickUs;
};
const coilScale coilScales[]=
{
    { ADC_SMPR_1_5,  DSOADC::ADC_PRESCALER_6  ,  1.17},  // T=500 us, LMax =~ 50 mH
    { ADC_SMPR_41_5, DSOADC::ADC_PRESCALER_6  ,  4.5},   // T=2.3 ms, LMax =~ 200 mH
    { ADC_SMPR_239_5,DSOADC::ADC_PRESCALER_8   , 28.},  //  T=14 ms   LMax =~ 1500 mH
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
    int range=2;
    uint16_t *samples=NULL;
    float Ra,Rb;
   // first compute resistance
    zeroAllPins();
    _pA.pullUp(TestPin::PULL_LOW);   
    //_pB.pullDown(TestPin::PULL_LOW);   
    _pB.setToGround();
    Ra=_pA.getCurrentRes();
    Rb=_pB.getCurrentRes();

    xDelay(20); // let it stabilize
    DeltaADC deltaAdc(_pA,_pB);
    deltaAdc.setup(coilScales[range].rate,coilScales[range].scale,32);
    
    float period;
    if(!deltaAdc.get(nbSamples,&samples,period))
    {
            return false;
    }    
            
    _pA.pullDown(TestPin::PULL_LOW);   
    
    // The last value are the resistance divider
    float sum=0;
    for(int i=2;i<nbSamples;i++)
        sum+=samples[i];
    sum/=((float)(nbSamples-2)); // skip the 2 first
    if(sum>4090.)
        return false;
    float r=sum*(Ra+Rb);  
    r=r/(4095.-sum);
    resistance=r;
    return true;
}
/**
 * 
 * @return 
 */
bool Coil::computeInductance(int range,int &minIndex, int &maxIndex,float &ductance)
{        
    int nbSamples;
    uint16_t *samples=NULL;
    float Ra,Rb;
    float timeMul;
   // first compute resistance
    zeroAllPins();
    _pB.setToGround();
    
    DeltaADC deltaAdc(_pA,_pB);
    deltaAdc.setup(coilScales[range].rate,coilScales[range].scale,511);
    _pA.pullUp(TestPin::PULL_LOW);   
    float period;
    Ra=_pA.getCurrentRes();
    Rb=_pB.getCurrentRes();    
    if(!deltaAdc.get(nbSamples,&samples,period))
    {
            return false;
    }    
    _pA.pullDown(TestPin::PULL_LOW);   
    _pB.pullDown(TestPin::PULL_LOW);  
    
    // Rescale to get voltage across coil
    // taking internal resitance into account
    float alpha=(resistance)/(Ra+Rb);
    float offset=4095.*alpha;
    float mul=1.+alpha;
    for(int i=2;i<nbSamples;i++)
    {
        float z=samples[i];
        z=z*mul-offset;
        if(z<0) z=0;
        if(z>4095) z=4095;
        samples[i]=(uint16_t)(z+0.49);        
    }
    // Look up the max, happens within the 200 first samples
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
    if(zmax<20) return false; // something is wrong
    // take a 2nd point at less than 10%
    // not zero
    for(int i=top+1;i<(nbSamples-1) && bottom==-1;i++)
    {
        if(samples[i]<(zmax/8) || samples[i+1]<5 ) // must not be zero !
        {
            zmin=samples[i];
            bottom=i;
        }
    }
    if(zmin==0) return false;
    minIndex=bottom;
    maxIndex=top;
    
    float totalResitance=Ra+Rb+resistance;
    float totalTime=((float)(bottom-top))*(period);
    float den=log((float)zmax/(float)zmin);
    
    ductance=(totalResitance*totalTime)/den;
    return true;
}

/**
 * 
 * @return 
 */
bool Coil::compute()
{
    AutoDisconnect ad;
    int minIndex, maxIndex;
    
    inductance=0;
    resistance=0;
    
    if(!computeResistance())
        return false; 
    if(resistance>4*_pA.getRes(TestPin::PULLUP_LOW))
        return false;
    // start with large range
    int range=-1;
    float ductance;
    for(int i=2;i>=0;i--)
    {        
        if(computeInductance(i,minIndex,maxIndex,ductance))
        {
            range=i;
            if((minIndex-maxIndex)>150) 
            {
                
                i=-1; // got it
            }
        }
    }
    // oversample it now
    if(range==-1) return false;
    int overSampling=8;
    for(int i=0;i<overSampling;i++)
    {
        computeInductance(range,minIndex,maxIndex,ductance);
        inductance+=ductance;
    }
    inductance/=overSampling;
    inductance-=(float)(_pA._calibration.inductanceInUF)/1000000.;
    if(inductance<0.) inductance=0.;
    return true;
}

// EOF
