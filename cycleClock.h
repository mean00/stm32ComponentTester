

#include "Arduino.h"

class CycleClock
{
public:
                CycleClock();
                ~CycleClock();
        void    start();
        void    stop();
        int     elapsedUs();
protected:
        uint32_t _start;
        uint32_t _end;
  
};