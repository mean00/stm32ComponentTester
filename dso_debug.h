#pragma once
#if 0
    void Logger(const char *fmt...);
    void Logger(int val);
#else
    #define Logger(...) {}
#endif²
