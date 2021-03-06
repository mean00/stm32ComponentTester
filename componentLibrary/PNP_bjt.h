#pragma once
#include "testPins.h"
#include "Component.h"
/**
 */
class PNPBjt : public Component
{
public:                
                    // PinA is Anode, pinB is cathode
                    PNPBjt( TestPin &Base, TestPin &Emitter,TestPin &Collector) :  Component(Base,Emitter,Collector,"PNP")
                    {
                      beta=0;
                      Vf=0;
                    }
            virtual bool compute()            ;
            virtual bool draw(int yOffset);
                    int  nbPins() {return 3;};       
                    int  likely() {return 100;} // Highly likely
            
protected:
            float beta;  
            float Vf;   // Base emitter diode
protected:
            bool computeVbe(float &vf);
            bool computeHfe(float &vf);
};
