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
#include "Capacitor.h"
#include "waveForm.h"
#include "testerControl.h"


/**
 * 
 * @return 
 */
Capacitor::CapEval Coil::evalSmall(  TestPin *p1,TestPin *p2,int fq, int clockPerSample, float &inductance)
{   
       
    p2->setToGround();
    p1->pullDown(TestPin::PULL_LOW);
    xDelay(10);
    
    int resistance;
    int samplingTime;
    int nbSamples;
    uint16_t *samples;
    
    if(!p1->pulseTimeDelta(*p2,clockPerSample, 512*2,fq,TestPin::PULL_LOW,nbSamples,&samples,resistance,true))
    {
        return Capacitor::EVAL_ERROR;
    }
    
    float period=F_CPU;
    period=(float)(clockPerSample)/period;
    
    
    
#define OFFSET 4    
    WaveForm wave(nbSamples-OFFSET,samples+OFFSET);
    int mn,mx;
    wave.searchMinMax(mn,mx);
    
    // resistance is the end of curve
    int sum=0;
    for(int i=0;i<16;i++)
    {
        sum+=samples[nbSamples-OFFSET-i];
    }
    float r=(float)sum/16.;
    
    if(r>4090) return Capacitor::EVAL_ERROR;
    
    float otherResistance=p1->getRes(TestPin::PULLUP_LOW);    
    this->resistance = otherResistance*r/(4095.-r);
    Logger("R=%d\n",(int)this->resistance);
    
#if 1    
    char st[20];
    TesterGfx::drawCurve(nbSamples,samples);
    sprintf(st,"Mn:%d",mn);
    TesterGfx::print(10,100,st);
    
    sprintf(st,"Mx:%d",mx);
    TesterControl::waitForAnyEvent();
#endif    
    
    if( (mx-mn)<100) // flat
    {
        if(mx<150) // stuck to zero
            return Capacitor::EVAL_BIGGER_CAP;      
    }
    // look for the starting minimum
    // Search start of ramp up above noise
    int iA,iB,vA,vB;
   
    wave.searchValueAbove(mn+(5*4095)/100, iA, vA, 0);
    wave.searchValueAbove(4095.*0.68, iB, vB, iA);
    
    if(vB<(4095/3)) return Capacitor::EVAL_BIGGER_CAP; // still charging...
    
    // If largeWindow is on, we dont require as much difference between min & max
    // it is for probing support
    // if largeWindow is off, we must have at leasst nbSample/8 samples, i.e. about 60
    int minSamples=0;

    minSamples=nbSamples/8;
    
    //if((iB-iA)<(minSamples)) return EVAL_SMALLER_CAP; // the pulse is too quick 
    if((vB-vA)<400) return Capacitor::EVAL_BIGGER_CAP; // A & B are too close, we must zoom out
    
    
    Capacitor::CapCurve curve;
    curve.iMax=iB;
    curve.iMin=iA;
    curve.resistance=resistance;
    curve.vMax=vB;
    curve.vMin=vA;
    curve.period=period;
    curve.nbSamples=nbSamples;
    //cap=Capacitor::computeCapacitance(iA,iB,vA,vB,resistance,period);
    //inductance=Capacitor::computeCapacitance(curve);
    inductance=0;
    
#if 0
    Component::prettyPrint(cap,"F",st);
    TesterGfx::print(10,20,st);
    
    Component::prettyPrint(fq,"Hz",st);
    TesterGfx::print(10,50,st);
    
    sprintf(st,"%d",clockPerSample);
    TesterGfx::print(10,80,st);
    

    sprintf(st,"A%d",iA);
    TesterGfx::print(80,80,st);

    sprintf(st,"B%d",iB);
    TesterGfx::print(80,100,st);
    
    
    
    TesterControl::waitForAnyEvent();
#endif    
    return Capacitor::EVAL_OK;  

}



/**
 * 
 * @return 
 */
bool Coil::compute()
{   
    
   Capacitor::CapEval er;
   float cap;
  
    TestPin *p1,*p2;
    if(_pB.pinNumber()==2)
    {
        p1=&_pB;
        p2=&_pA;
    }else if(_pA.pinNumber()==2)
    {
        p1=&_pA;
        p2=&_pB;        
    }
    else
    {
        TesterGfx::clear();
        TesterGfx::print(10,60,"Connect one leg");
        TesterGfx::print(10,90,"to center");
        TesterControl::waitForAnyEvent();
        return Capacitor::EVAL_ERROR; 
    }
   
   {
    int found=0;
    Logger("scale=%d",found);
    er=evalSmall(p1,p2,4000,2,cap);

    if(er!=Capacitor::EVAL_OK)
    {
        inductance=0;
        return false;
    }
   }
   return true;
}


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
    return true;
}
#if 0
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
#endif
// EOF
