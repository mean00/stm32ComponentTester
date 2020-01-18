
#include <SPI.h>
#include "Component.h"
#include "math.h"

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
    while(1)
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
    sprintf(output,"%3.2f%s%s",value,scale[dex],unit);
}