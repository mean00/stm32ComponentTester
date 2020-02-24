
#include "EEPROM.h"
#include "nvm.h"
#include "nvm_default.h"


#define HASH 0x4567

bool NVM::loaded=false;
/**
 * 
 * @param pin
 * @param calibration
 * @return 
 */

bool    NVM::hasCalibration()
{    
    EEPROMClass eep;
    eep.init();
    if(eep.read(0)==HASH)
        return true;
    return false;
}

bool    NVM::loadTestPin(int pin, TestPinCalibration &calibration)
{    
    EEPROMClass eep;
    eep.init();
    int calibrationHash=eep.read(0);
    if(calibrationHash != HASH)
    {
        calibration.resUp           =WIRE_RESISTANCE_AND_INTERNAL;
        calibration.resDown         =WIRE_RESISTANCE_AND_INTERNAL;
        calibration.capOffsetInPf   =INTERNAL_CAPACITANCE_IN_PF;
        calibration.inductanceInUF  =INTERNAL_INDUCTANCE_IN_UF;
        return true; // default value
    }
    calibration.resUp=          eep.read(10*pin+1+0);
    calibration.resDown=        eep.read(10*pin+1+1);;
    calibration.capOffsetInPf=  eep.read(10*pin+1+2);;
    calibration.inductanceInUF= eep.read(10*pin+1+3);;
    return true;
}
/**
 * 
 * @param pin
 * @param calibration
 * @return 
 */
bool    NVM::saveTestPin(int pin, const TestPinCalibration &calibration)
{
      EEPROMClass eep;
      eep.init();
      eep.write(10*pin+1+0, calibration.resUp);
      eep.write(10*pin+1+1, calibration.resDown);;
      eep.write(10*pin+1+2,calibration.capOffsetInPf);;
      eep.write(10*pin+1+3,calibration.inductanceInUF);;
      return true;
}

bool    NVM::reset()
{
    EEPROMClass eep;
    eep.init();
    eep.format();
    return true;    
}
bool    NVM::doneWriting()
{
      EEPROMClass eep;
      eep.init();
      eep.write(0,HASH);
      return true;
}
// EOF
