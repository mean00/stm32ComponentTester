
#include "EEPROM.h"
#include "nvm.h"
#include "nvm_default.h"
#include "cpuID.h"

#define HASH 0x456F

#define SLOTS_PER_PIN 20
extern void *eeprom_begin; // exported by linker

/**
 * 
 */
class NVMeeprom  : public EEPROMClass
{
    public:
        NVMeeprom()
        {
            uint32_t flahSize=cpuID::getFlashSize();
             uint32 pageBase0=(uint32_t)&eeprom_begin;    
            EEPROMClass::init(pageBase0,pageBase0+0x800,0x800);
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
        for(int i=0;i<CALIBRATION_VERY_SMALL_SIZE;i++)
            calibration.capOffsetHighInPfMu16[i] =0;        
        calibration.inductanceInUF    =INTERNAL_INDUCTANCE_IN_UF;
        return true; // default value
    }
    calibration.resUp=          eep.read(SLOTS_PER_PIN*pin+1+0);
    calibration.resDown=        eep.read(SLOTS_PER_PIN*pin+1+1);;
    calibration.capOffsetInPf=  eep.read(SLOTS_PER_PIN*pin+1+2)+1;; // there is a ~ 3/4 pf Error
    calibration.inductanceInUF= eep.read(SLOTS_PER_PIN*pin+1+3);;
    for(int i=0;i<CALIBRATION_VERY_SMALL_SIZE;i++)        
        calibration.capOffsetHighInPfMu16[i]= eep.read(SLOTS_PER_PIN*pin+5+i);;    
    //calibration.capOffsetHighInPfMu16[0]=16.*40.2;
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
      eep.write(SLOTS_PER_PIN*pin+1+0, calibration.resUp);
      eep.write(SLOTS_PER_PIN*pin+1+1, calibration.resDown);;
      eep.write(SLOTS_PER_PIN*pin+1+2,calibration.capOffsetInPf);;
      eep.write(SLOTS_PER_PIN*pin+1+3,calibration.inductanceInUF);;
      for(int i=0;i<CALIBRATION_VERY_SMALL_SIZE;i++)        
            eep.write(SLOTS_PER_PIN*pin+5+i,calibration.capOffsetHighInPfMu16[i]);;
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
