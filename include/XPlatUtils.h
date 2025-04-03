#ifndef X_PLATFORM_UTILS
#define X_PLATFORM_UTILS

#include <string.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    
    #define geModule HMODULE
#else
    #include <limits.h>
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


/* File attribute constants */
typedef enum 
{
    GE_ATTR_DIRECTORY = 0x0001,
    GE_ATTR_READONLY  = 0x0002,
    GE_ATTR_HIDDEN    = 0x0004,
    GE_ATTR_SYSTEM    = 0x0008,
    GE_ATTR_ARCHIVE   = 0x0010,
    GE_ATTR_NORMAL    = 0x0020,
    GE_ATTR_TEMPORARY = 0x0040,
}
geAttribute;

typedef struct 
geFindData
{
    char         fileName[256];     /* Current file name */
    char         fullPath[PATH_MAX];    /* Full path to file */
    uint64       fileSize;          /* File size in bytes */
    
    /* Time values using SDL timestamp format */
    uint64       creationTime;
    uint64       lastAccessTime;
    uint64       lastWriteTime;
    
    /* File attributes as bitflags */
    geAttribute  attributes;        /* Use defined constants below */
    
    /* Internal data used by implementation */
    void        *internalData;      /* Platform-specific data, opaque to caller */
} 
geFindData;

typedef struct DynamicLibrary DynamicLibrary;
typedef struct geFindData     geFindData;


inline bool    geGetUserName(            char     *player_name,      uint32     *size);
inline uint32  geGetCurrentDir(          uint32    size,             char       *path);
inline char   *geGetExecutablePath();

       void   *geFindFirstFile(    const char      *file_name,       geFindData *find_data);

char *itoa(int value, char* str, int base);
void splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);


#endif /* X_PLATFORM_UTILS */
