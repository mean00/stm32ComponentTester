
#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "Capacitor.h"
#include "math.h"
#include "cycleClock.h"
#include "MapleFreeRTOS1000_pp.h"
#include "waveForm.h"
/**
_nbSamples=nb;
                    _samples=s;
*/  
/**
 *  Underflow : We are seeing only the beginning of the curve => fq too high
 *  Overflow  : We are seeing too much of the end => fq too slow
 * 
 * 
 */
bool WaveForm::searchRampUp(float  target, int &delta,bool &overflow, bool &underflow,int &a, int &b)
{
    underflow=false;
    overflow=false;
    int limitA,limitB;

    
    limitA=4095.*0.1;
    limitB=4095.*target;

    // We need 2 points...
    // Lookup up 5% and 1-1/e
    int pointA=-1,pointB=-1;
    for(int i=2;i<_nbSamples-2;i++)
    {
        if(_samples[i]>limitA && _samples[i]<=_samples[i+1] && _samples[i+1]<=_samples[i+2]) // make sure it is not a glitch
        {
            pointA=i;
            i=4095;
        }
    }
    if(pointA==-1 || pointA >=_nbSamples-1) 
    {
        underflow=true;
        return true;
    }
    for(int i=pointA+1;i<_nbSamples;i++)
    {
        if(_samples[i]>limitB) // 68%
        {
            pointB=i;
            i=4095;
        }
    }
    if(pointB==-1) 
    {
        underflow=true;
        return true;
    }        
    
    if((pointB-pointA)<=2) // Too close
    {
        overflow=true;
        return true; // not enough points, need at least one
    }
    
    // Compute

    float valueA=_samples[pointA];
    float valueB=_samples[pointB];

 
    // too close to the end => 
    if(fabs(valueA-4095.)<2) 
    {
        overflow=true;
        return true;
    }
    if(fabs(valueB-4095.)<2) 
    {
        overflow=true;
        return true;
    }
/*    
 *     float timeElapsed=(pointB-pointA);
    timeElapsed*=period;

    float den=(4095.-(float)valueA)/(4095.-(float)valueB);
    
    if(fabs(den-2.718)<0.01) 
        return false;
    den=log(den);
    cap=timeElapsed/(resistance*den);
 */
    a=pointA;
    b=pointB;
    delta=b-a;
    return true;   
}
