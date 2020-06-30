
#include "EEPROM.h"
#include "nvm.h"
#include "nvm_default.h"
#include "cpuID.h"

#define HASH 0x456B


/**
 * 
 */
class NVMeeprom  : public EEPROMClass
{
    public:
        NVMeeprom()
        {
            uint32_t flahSize=cpuID::getFlashSize();
            uint32_t topAddress=(uint32)(0x8000000 + flahSize * 1024); ;
            EEPROMClass::init(topAddress-2*EEPROM_PAGE_SIZE,topAddress-EEPROM_PAGE_SIZE,EEPROM_PAGE_SIZE);
        }
};

bool NVM::loaded=false;
/**
 * 
 * @param pin
 * @param calibration
 * @return 
 */

bool    NVM::hasCalibration()
{        
   NVMeeprom eep;
    
#if 1
    if(eep.read(0)==HASH)
        return true;
#endif    
    return false;
}

bool    NVM::loadTestPin(int pin, TestPinCalibration &calibration)
{    
    NVMeeprom eep;
    int calibrationHash=eep.read(0);
    if(calibrationHash != HASH)
    {
        calibration.resUp             =WIRE_RESISTANCE_AND_INTERNAL;
        calibration.resDown           =WIRE_RESISTANCE_AND_INTERNAL;
        calibration.capOffsetInPf     =INTERNAL_CAPACITANCE_IN_PF;
        calibration.capOffsetHighInPf =INTERNAL_CAPACITANCE_IN_PF_HIGH;        
        calibration.inductanceInUF    =INTERNAL_INDUCTANCE_IN_UF;
        return true; // default value
    }
    calibration.resUp=          eep.read(10*pin+1+0);
    calibration.resDown=        eep.read(10*pin+1+1);;
    calibration.capOffsetInPf=  eep.read(10*pin+1+2)+1;; // there is a ~ 3/4 pf Error
    calibration.inductanceInUF= eep.read(10*pin+1+3);;
    calibration.capOffsetHighInPf= eep.read(10*pin+1+4);;
    calibration.capOffsetHighInPf =INTERNAL_CAPACITANCE_IN_PF_HIGH;        
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
      NVMeeprom eep;
      eep.write(10*pin+1+0, calibration.resUp);
      eep.write(10*pin+1+1, calibration.resDown);;
      eep.write(10*pin+1+2,calibration.capOffsetInPf);;
      eep.write(10*pin+1+3,calibration.inductanceInUF);;
      eep.write(10*pin+1+4,calibration.capOffsetHighInPf);;
      return true;
}

bool    NVM::reset()
{
    NVMeeprom eep;
    eep.format();
    return true;    
}
bool    NVM::doneWriting()
{
      NVMeeprom eep;
      eep.write(0,HASH);
      return true;
}
// EOF
