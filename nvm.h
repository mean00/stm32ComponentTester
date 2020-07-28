
#define CALIBRATION_VERY_SMALL_SIZE 10
class TestPinCalibration
{
public:
        uint16_t  resDown;
        uint16_t  resUp;
        uint16_t  capOffsetInPf; // when using fast ADC
        uint16_t  capOffsetHighInPfMu16[CALIBRATION_VERY_SMALL_SIZE]; // when using slow adc
        uint16_t  inductanceInUF;
        TestPinCalibration()
        {
          resDown=0;
          resUp=0;
          capOffsetInPf=0;
          inductanceInUF=0;
          for(int i=0;i<CALIBRATION_VERY_SMALL_SIZE;i++)
            capOffsetHighInPfMu16[i]=0.0;
        }
};

class NVM
{
public:  
        static void    init();
        static bool    load();
        static bool    loadTestPin(int pin, TestPinCalibration &calibration);
        static bool    saveTestPin(int pin, const TestPinCalibration &calibration);
        static bool    reset();
        static bool    doneWriting();
        static bool    hasCalibration();
protected:
        static bool    loaded;
        
};