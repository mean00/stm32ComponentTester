
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
  EVENTS        _event;
};