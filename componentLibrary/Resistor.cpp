/*
 * Resistance tester
*/

#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "Resistor.h"
static float checkResistor(TestPin &A, TestPin &B);
static float computeResistance(float adcValue, int resistance);

/**
 * 
 * @param yOffset
 * @return 
 */
bool Resistor::draw(int yOffset)
{
    char st[32];        
    Component::prettyPrint(resistance, "O",st);
    TesterGfx::drawResistor(yOffset, st,_pA.pinNumber(), _pB.pinNumber());
    return true;
}

/**
 * 
 * @return 
 */
bool Resistor::compute()
{
    typedef struct probePoints
    {
        TestPin::TESTPIN_STATE A,B;        
    };
    
    // do all the possibilities
    // the most accurate one is when the ADC is closer to center = 2047
    
    const probePoints probes[]={  

        {TestPin::PULLUP_LOW,TestPin::GND},
        {TestPin::PULLUP_LOW,TestPin::PULLDOWN_LOW},        
        {TestPin::PULLUP_MED,TestPin::GND},
        {TestPin::PULLUP_MED,TestPin::PULLDOWN_MED},                
        {TestPin::PULLUP_HI,TestPin::GND},
        {TestPin::PULLUP_HI,TestPin::PULLDOWN_HI},        
    };
    int n=sizeof(probes)/sizeof(probePoints);
    float adcs[n];
    int resistances[n];
    
    
    for(int i=0;i<n;i++)
    {
       probe( _pA,probes[i].A,_pB,probes[i].B,adcs[i],resistances[i]);       
    }
    // find the ADC closest to 4095/2=2047
    int candidate=-1;
    float match=4095.;
    for(int i=0;i<n;i++)
    {
        float a=adcs[i];
        if(a<10. || a > (4095-10)) continue; // not reliable
        float r=fabs(a-2047.); // delta to center of ADC
        if(r<match)
        {
            match=r;
            candidate=i;
        }
    }
    resistance=computeResistance(adcs[candidate],resistances[candidate]);    
    return true;
    
}

/**
 * 
 * @param A
 * @param stateA
 * @param B
 * @param stateB
 * @return 
 */
bool Resistor::probe( TestPin &A,TestPin::TESTPIN_STATE stateA, TestPin &B,TestPin::TESTPIN_STATE stateB,float &adc, int &resistance)
{
      AutoDisconnect ad;
      A.setMode(stateA);
      B.setMode(stateB);
      int hiAdc, loAdc;
      int hiNb,loNb;    
      A.slowDmaSample(hiAdc,hiNb);
      B.slowDmaSample(loAdc,loNb);
      
      float delta=abs(hiAdc-loAdc);      
      if(delta<0.) delta=0.;
      adc=delta/(float)(hiNb); // assume hiNB=loNB   
      resistance=A.getCurrentRes()+B.getCurrentRes();      
      return true;
}

/**
 * 
 * @param adcValue
 * @param resistance
 * @return 
 */     
float computeResistance(float adcValue, int resistance)
{
      if(adcValue>4090.) return 0;
      float r=((float)(resistance))*adcValue/((4095.-adcValue));
      return r;
}
// EOF
