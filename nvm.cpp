
#include "EEPROM.h"
#include "nvm.h"
#include "nvm_default.h"
EEPROMClass eep;

bool NVM::loaded=false;
/**
 * 
 * @param pin
 * @param calibration
 * @return 
 */
bool    NVM::loadTestPin(int pin, TestPinCalibration &calibration)
{
    calibration.resUp=WIRE_RESISTANCE_AND_INTERNAL;
    calibration.resDown=WIRE_RESISTANCE_AND_INTERNAL;
    calibration.capOffsetInPf=INTERNAL_CAPACITANCE_IN_PF;
    calibration.inductanceInUF=INTERNAL_INDUCTANCE_IN_UF;
}
