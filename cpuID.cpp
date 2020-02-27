#include "Arduino.h"
#include "cpuID.h"

static uint32_t family , designer;

void cpuID::identify()
{
        uint32_t * pid = (uint32_t *)(0xF0000FE0);
	family = pid[0] | ((pid[1] & 0xf) << 8);
	designer = ((pid[1] & 0xf0) >> 4) | ((pid[2] & 0xf) << 4);
}

uint32_t cpuID::getFamily()
{
    return family;
}

uint32_t cpuID::getDesigner()
{
    return designer;
}

uint32_t cpuID::getFlashSize()
{
    return 64;
}

