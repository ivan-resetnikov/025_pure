#ifndef STL

// NOTE(vanya): Detect platform
#ifdef __linux__
    #define P_PLATFORM_LINUX_X86_64
#endif
#ifdef WIN32
    #define P_PLATFORM_WINDOWS_X86_64
#endif

// NOTE(vanya): Include correct STL
#ifdef P_PLATFORM_LINUX_X86_64
    #include "stl_linux_x86_64.c"
#endif

#ifdef P_PLATFORM_WINDOWS_X86_64
    #include "stl_windows_x86_64.c"
#endif

#define STL
#endif