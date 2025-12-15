#define P_DEFINE_TYPE_SHORTHANDS
#include "stl.c"

#ifdef P_PLATFORM_LINUX_X86_64
#include "entrypoint_linux_x86_64.c"
#endif

#ifdef P_PLATFORM_WINDOWS_X86_64
#include "entrypoint_windows_x86_64.c"
#endif