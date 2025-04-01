#ifndef X_PLATFORM_UTILS
#define X_PLATFORM_UTILS

#include <string.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    
    #define geModule HMODULE
#else
    #ifdef _DEBUG
        #include <stdio.h>
        #include <stdlib.h>
        #define _assert(exp, file, line) \
            ((exp) ? (void)0 : (fprintf(stderr, "Assertion failed: %s, file: %s, line: %d\n", #exp, file, line), abort()))
    #else
        #define _assert(exp) ((void)0)  // Disabled in non-debug mode
    #endif
    #define geModule void*
#endif

#include "GETypes.h"



typedef struct DynamicLibrary DynamicLibrary;


//inline geModule  geLoadLibrary( const char     *path);
//inline void     *geGetProcAddress(    geModule  lib,        const char   *symbol);
//inline int       geFreeLibrary(       geModule  lib);
       bool      geGetUserName(       char     *player_name,      uint32 *size);
inline uint32    geGetCurrentDir(     uint32    size,             char   *path);

char *itoa(int value, char* str, int base);
void splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);


#endif /* X_PLATFORM_UTILS */
