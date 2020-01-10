/*
 * 
*/

#include <SPI.h>
#include "Ucglib.h"
#include "fancyLock.h"
#include "testPins.h"
#include "resistor.h"
static float checkResistor(TestPin &A, TestPin &B);
static float computeResistance(int adcValue, int resistance);

/**
 * 
 * @param yOffset
 * @return 
 */
bool Resistor::draw(Ucglib *ucg,int yOffset)
{
    char st[16];    
    sprintf(st,"%d",resistance);     
    ucg->drawString(10,30,0,st); 
    return true;
}

#define GOOD_HI 3500
#define GOOD_LO 600

#define CHECK_GOOD(adc)  if(adc >= GOOD_LO && adc<=GOOD_HI)\
        { \
            resistance=computeResistance(adcLow,rLow); \
            return true; \
        }

/**
 */
extern int result[20];
bool Resistor::compute()
{
    typedef struct probePoints
    {
        TestPin::TESTPIN_STATE A,B;        
    };
    const probePoints probes[]={  
        {TestPin::PULLUP_LOW,TestPin::GND},
        {TestPin::PULLUP_LOW,TestPin::PULLDOWN_LOW},        
//        {TestPin::PULLUP_INTERNAL,TestPin::GND},        
//        {TestPin::PULLUP_INTERNAL,TestPin::PULLDOWN_INTERNAL},        
        {TestPin::PULLUP_HI,TestPin::GND},
        {TestPin::PULLUP_HI,TestPin::PULLDOWN_HI},        
    };
    int n=sizeof(probes)/sizeof(probePoints);
    int adcs[n];
    int resistances[n];
    
    
    for(int i=0;i<n;i++)
    {
       probe( _pA,probes[i].A,_pB,probes[i].B,adcs[i],resistances[i]);       
    }
    // find the ADC closest to 4095/2=2047
    int candidate=-1;
    int match=4095;
    for(int i=0;i<n;i++)
    {
        int a=adcs[i];
        if(a<10 || a > (4095-10)) continue; // not reliable
        int r=abs(a-2047);
        if(r<match)
        {
            match=r;
            candidate=i;
        }
    }
    if(candidate==-1) return false;
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
bool Resistor::probe( TestPin &A,TestPin::TESTPIN_STATE stateA, TestPin &B,TestPin::TESTPIN_STATE stateB,int &adc, int &resistance)
{
      AutoDisconnect ad;
      A.setMode(stateA);
      B.setMode(stateB);
      int hiAdc, loAdc;
      float hiVolt,loVolt;    
      A.sample(hiAdc,hiVolt);
      B.sample(loAdc,loVolt);
      
      int n=abs(hiAdc-loAdc);
      resistance=A.getCurrentRes()+B.getCurrentRes();
      adc=n;
      return true;
}

/**
 * 
 * @param adcValue
 * @param resistance
 * @return 
 */     
float computeResistance(int adcValue, int resistance)
{
      float a=adcValue;    
      if(a>4093) return 0;
      float r=(float)(resistance)*a/((4095.-a));
      return r;
}
// EOF