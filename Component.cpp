
#include <SPI.h>
#include "Component.h"
#include "math.h"

void Component::prettyPrint(float value, const char *unit,  char *output)
{
    float cValue=value;
    const char *prefix="";
    if(value<1)
    {
        if(value>=0.001) // milli
        {
            cValue=value*1000.;
            prefix="m";
        }else
        if(value>=0.000001)
        {
            cValue=value*1000000.;
            prefix="u";
        }else
        if(value>=0.000000001)
        {
            cValue=value*1000000000.;
            prefix="n";
        }else
        if(value>=0.000000000001)
        {
            cValue=value*1000000000000.;
            prefix="p";
        }
    }
    
    sprintf(output,"%3.2f%s%s",cValue,prefix,unit);
}