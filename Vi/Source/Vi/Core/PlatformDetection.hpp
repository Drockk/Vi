#pragma once

#ifdef _WIN32
    #ifdef _WIN64
        #define VI_PLATFORM_WINDOWS
    #else
        #error "x86 Builds are not supported!"
    #endif
#endif
