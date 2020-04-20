/*
*/

#include "../../SPI/src/SPI.h"
#include "fancyLock.h"
#include "testPins.h"
#include "allComponents.h"
#include "dso_adc.h"
#include "testerGfx.h"
#include "testerControl.h"
#include "componentSignature.h"
#include "cpuID.h"
#include "pinConfiguration.h"
/**
 */
void menuSystem(void)
{
    TesterGfx::clear();
    TesterGfx::print(0,60,"MenuSystel");
    TesterControl::waitForEvent();
    return;
}