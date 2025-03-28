#ifndef X_PLATFORM_UTILS
#define X_PLATFORM_UTILS

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    
    #define geModule HMODULE
#else
    #define geModule void*
#endif

#include "BaseType.h"


typedef struct DynamicLibrary DynamicLibrary;


inline geModule  geLoadLibrary( const char     *path);
inline void     *geGetProcAddress(    geModule  lib,        const char   *symbol);
inline int       geFreeLibrary(       geModule  lib);
       bool      geGetUserName(       char     *player_name,      uint32 *size);
inline uint32    geGetCurrentDir(     uint32    size,             char   *path);

#endif /* X_PLATFORM_UTILS */
