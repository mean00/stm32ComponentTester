#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"

namespace DSOADC
{
typedef int Prescaler;
}
#define adc_smp_rate int
#define F_CPU 72000000

#define ADC_PRESCALER_2 2
#define ADC_PRESCALER_4 4
#define ADC_PRESCALER_6 6
#define ADC_PRESCALER_8 8

#define ADC_SMPR_1_5 1
#define ADC_SMPR_7_5 7
#define ADC_SMPR_13_5 13
#define ADC_SMPR_28_5 28
#define ADC_SMPR_41_5 41
#define ADC_SMPR_55_5 55
#define ADC_SMPR_71_5 71
#define ADC_SMPR_239_5 239

typedef struct ScalerTable
{
    float              div;
    DSOADC::Prescaler  scaler;
};

typedef struct RateTable
{
    float              cycle;
    adc_smp_rate       rate;
};
const ScalerTable scalerTable[]=
{
    {2.0, ADC_PRESCALER_2},
    {4.0, ADC_PRESCALER_4},
    {6.0, ADC_PRESCALER_6},
    {8.0, ADC_PRESCALER_8}
};


         
#define RATE_MK(x)          { 12+x##.5, ADC_SMPR_##x##_5},
         
const RateTable rateTable[]=
{
    RATE_MK(1)
    RATE_MK(7)
    RATE_MK(13)
    RATE_MK(28)
    RATE_MK(41)
    RATE_MK(55)
    RATE_MK(71)
    RATE_MK(239)
};
/**
 * 
 * @param frequency
 * @param prescaler
 * @param rate
 * @return 
 */
bool findRateScale(int frequency,  DSOADC::Prescaler  &prescaler,  adc_smp_rate   &rate)
{
    float alpha=(float)F_CPU;
    alpha/=(float)(frequency+1);
    
    int dex=-1;
    for(int i=sizeof(scalerTable)/sizeof(ScalerTable)-1;i>0 && dex==-1;i--)
    {
        float one=alpha/scalerTable[i].div;
        if(one>(239.5+12))
        {
            dex=i;
            break;
        }
    }
    if(dex==-1)
    {
        // Take the biggest prescaler
        dex=0; //sizeof(scalerTable)/sizeof(ScalerTable)-1;
    }
    prescaler=scalerTable[dex].scaler;
    
    // now rate
    alpha=(float)F_CPU;
    alpha/=(float)(frequency+1);
    alpha/=scalerTable[dex].div;
    dex=-1;
    for(int i=sizeof(rateTable)/sizeof(RateTable)-1;i>0 && dex==-1;i--)
    {
        // Alpha > rate
        if(alpha>rateTable[i].cycle)
        {
            dex=i;
            break;
        }
    }
    if(dex==-1)
    {
        dex=0;
    }
    rate=rateTable[dex].rate;
    return true;
}

void test(int fq)
{
   int scale,rate; 
   if(false== findRateScale(fq,scale,rate))
    {
        printf("Cannot find fq for %d\n",fq);
        exit(-1);
    }
    float smp=(float)F_CPU;
    smp/=(float)scale;
    smp/=12.5+(float)rate;
    printf("%d => Rate=%d scale=%d adc fq=%4.0f\n",fq,rate,scale,smp);
if(smp>fq) printf("==OK\n");
    else   printf("*** KO **\n");

}
int main(int a, char **b)
{
    test(1000);
    test(5000);
    test(10000);
    test(50000);
    test(100000);
    test(500000);
    test(1000000);
    return 0;


}
