class TestPinCalibration
{
public:
        uint16_t  resDown;
        uint16_t  resUp;
        uint16_t  capOffsetInPf;
        uint16_t  inductanceInUF;
};

class NVM
{
public:  
        static void    init();
        static bool    load();
        static bool    loadTestPin(int pin, TestPinCalibration &calibration);
protected:
        static bool    loaded;
        
};