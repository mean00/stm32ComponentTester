
#if 1
    #define PRINTF(x) Serial1.println(x)
#else
    #define PRINTF(...) {}
#endif