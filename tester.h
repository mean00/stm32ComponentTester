

#pragma once

class Tester
{
public:
                    Tester();
            bool    probe();


protected:
            bool probe3Pins(int dex,Component **comp);
            bool probe2Pins(int dex,Component **comp);
            void scan2Pins(int dex, Component **c);
            void scan3Pins(int dex, Component **c);
            
            TestPin *pinTable[6];
    
};