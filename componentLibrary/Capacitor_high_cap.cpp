#include <SPI.h>
#include "fancyLock.h"
#include "testPins.h"
#include "Capacitor.h"
#include "math.h"
#include "cycleClock.h"
#include "MapleFreeRTOS1000_pp.h"
#include "waveForm.h"
#include "testerControl.h"
#include "myPwm.h"
#include "math.h"


/**
 */
const Capacitor::CapScale hiCaps[]=
{
    {36*1000,TestPin::PULL_LOW,false}, //   10 uf    
    {16*1000,TestPin::PULL_LOW,false}, //   20 uf
    {8*1000, TestPin::PULL_LOW,false}, //   50 uf
    {3600,   TestPin::PULL_LOW,false}, //   100 uf
    {1200,   TestPin::PULL_LOW,false}, //   200 uf
    {600,    TestPin::PULL_LOW,false}, //   500 uf
    {500,    TestPin::PULL_LOW,false}, //   600 uf
};
/**
 * 
 * @return 
 */

bool Capacitor::computeHighCap(bool overSample)
{    
    int oversampling=1;
    if(!overSample) oversampling=0;
     
    int n=sizeof(hiCaps)/sizeof(Capacitor::CapScale);
    return computeCapRange(n,hiCaps,oversampling);
}