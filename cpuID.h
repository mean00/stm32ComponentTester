
#pragma once

class cpuID
{
public:
        static void         identify();
        static uint32_t     getFamily();
        static uint32_t     getDesigner();
        static uint32_t     getFlashSize();
};
