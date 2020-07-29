

#pragma once

class Tester
{
public:
                    Tester();
            bool    probe(); // return true if we have found something

protected:
            bool probe3Pins(int dex,Component **comp);
            bool probe2Pins(int dex,Component **comp);
            void scan2Pins(int dex, Component **c);
            void scan3Pins(int dex, Component **c);
            
            TestPin *pinTable[6]; // sort list of pins to probe all combinaisons
    
};