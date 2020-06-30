
#include <SPI.h>
#include "Component.h"
#include "math.h"
#include "dso_adc.h"
#include "MapleFreeRTOS1000_pp.h"
/**
 * 
 * @param prefix
 * @param value
 * @param unit
 * @param output
 */
void Component::prettyPrintPrefix(const char *prefix,float value, const char *unit,  char *output)
{
    int n=strlen(prefix);
    strcpy(output,prefix);
    prettyPrint(value,unit,output+n);
    
}


void Component::prettyPrint(float value, const char *unit,  char *output)
{
    const char *big[]={"","k","M","G","P"};
    const char *small[]={"","m","u","n","p"};
    float mul=0.001; // 1/1000
    const char **scale=big;
    if(value==0.0)
    {
        sprintf(output,"0%s",unit);
        return;
    }
    if(value<1.)
    {
        mul=1000.;
        scale=small;
    }
    int dex=0;
    while(dex<=3)
    {
        if(value>1 && value<1000)
            break;
        dex++;
        if(dex>sizeof(big)/sizeof(char *))
        {
            sprintf(output,"N/A");
            return;
        }
        value*=mul;
    }
    // dont use absurd number of digit
    if(value>=100.) // no need for digit at all
    {
        sprintf(output,"%3d%s%s",(int)value,scale[dex],unit);
    }else
    {
        if(value>=10.)
        {
            sprintf(output,"%2.1f%s%s",value,scale[dex],unit);
        }else            
        {
            sprintf(output,"%1.2f%s%s",value,scale[dex],unit);
        }
    }
    
    
}
/**
 * 
 * @param adc
 * @return 
 */
float Component::adcToVolt(float adc)
{
    return DSOADC::adcToVolt(adc);
}

/**
 * 
 * @param Anode
 * @param Cathode
 * @param vfOut
 * @return 
 */
bool Component::computeDiode(TestPin &Anode, TestPin &Cathode,float &vfOut)
{
     AutoDisconnect ad;
    // even with the lowest resistance we are at max at 
    // 3.3v/470*2 Ohm= 3 mA, which is fine
    Anode.pullUp(TestPin::PULL_LOW);
    Cathode.pullDown(TestPin::PULL_LOW);   
    xDelay(20);
    int adcA,nbA;
    int adcB,nbB;
    
    Anode.slowDmaSample(adcA,nbA);
    Cathode.slowDmaSample(adcB,nbB);

    float vf=(float)(adcA-adcB);
    vf/=(float)nbA;
    vfOut=adcToVolt(vf);
    return true;
}