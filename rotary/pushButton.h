
#pragma once
class PushButton
{
public:
  enum EVENTS
  {
    NONE=0,
    SHORT_PRESS=1,
    LONG_PRESS=2
  };
                PushButton(int pin);
    EVENTS      getEvent();
    
    
    void        interrupt();
protected:
  int           _pin;
  int           _event;
  uint32_t      _lastRead;
  uint32_t      _down; // time down was detected
};