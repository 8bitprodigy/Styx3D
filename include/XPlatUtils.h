#ifndef X_PLATFORM_UTILS
#define X_PLATFORM_UTILS

#include <string.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    
    #define geModule HMODULE
    #define PATH_SEPARATOR '\\'
    #define PATH_MAX MAX_PATH
#else
    #include <limits.h>
    #include <dirent.h>
    #include <dlfcn.h>
    #include <limits.h>
    #include <pwd.h>
    #include <sys/stat.h>
    #include <time.h>
    #include <unistd.h>
    #define PATH_SEPARATOR '/'
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


// Constants equivalent to Windows file attributes
#define GE_FILE_ATTRIBUTE_READONLY    0x00000001
#define GE_FILE_ATTRIBUTE_HIDDEN      0x00000002
#define GE_FILE_ATTRIBUTE_SYSTEM      0x00000004
#define GE_FILE_ATTRIBUTE_DIRECTORY   0x00000010
#define GE_FILE_ATTRIBUTE_ARCHIVE     0x00000020
#define GE_FILE_ATTRIBUTE_NORMAL      0x00000080

// Invalid handle value equivalent
#define GE_INVALID_HANDLE_VALUE ((void*)-1)
#define GE_INVALID_FILE_ATTRIBUTES ((uint32)-1)

// File access rights
#define GE_GENERIC_READ               0x80000000
#define GE_GENERIC_WRITE              0x40000000
#define GE_GENERIC_EXECUTE            0x20000000
#define GE_GENERIC_ALL                0x10000000

// Share modes
#define GE_FILE_SHARE_READ            0x00000001
#define GE_FILE_SHARE_WRITE           0x00000002
#define GE_FILE_SHARE_DELETE          0x00000004

// Creation dispositions
#define GE_CREATE_NEW                   1
#define GE_CREATE_ALWAYS                2
#define GE_OPEN_EXISTING                3
#define GE_OPEN_ALWAYS                  4
#define GE_TRUNCATE_EXISTING            5

// File attributes
#define GE_FILE_ATTRIBUTE_NORMAL      0x00000080

// Seek methods
#define GE_FILE_BEGIN                   0
#define GE_FILE_CURRENT                 1
#define GE_FILE_END                     2

// Error codes
#define GE_ERROR_SUCCESS                0
#define GE_ERROR_FILE_NOT_FOUND         2
#define GE_ERROR_ACCESS_DENIED          5
#define GE_ERROR_INVALID_HANDLE         6
#define GE_ERROR_INVALID_PARAMETER     87
#define GE_ERROR_INSUFFICIENT_BUFFER  122
#define GE_ERROR_IO_PENDING           997

enum {
    GE_LOW  = 0,
    GE_HIGH = 1,
};

typedef struct 
GE_BY_HANDLE_FILE_INFORMATION
{
    uint32 dwFileAttributes;
    uint32 ftCreationTime[2];
    uint32 ftLastAccessTime[2];
    uint32 ftLastWriteTime[2];
    uint32 dwVolumeSerialNumber;
    uint32 nFileSizeHigh;
    uint32 nFileSizeLow;
    uint32 nNumberOfLinks;
    uint32 nFileIndexHigh;
    uint32 nFileIndexLow;
} 
GE_BY_HANDLE_FILE_INFORMATION;

#ifdef _WIN32
    #define GE_FIND_DATA WIN32_FIND_DATA
#else
    typedef struct 
    GE_FIND_DATA 
    {
        uint32 fileAttributes;
        uint32 ftCreationTime[2];
        uint32 ftLastAccessTime[2];
        uint32 ftLastWriteTime[2];
        uint32 fileSizeHigh;
        uint32 fileSizeLow;
        uint32 reserved[2];
        char   fileName[PATH_MAX];
        char   alternateFileName[14];
    } 
    GE_FIND_DATA;
#endif

typedef struct
GE_FIND_HANDLE
{
    bool         valid;
    char         searchPath[260];
    char         searchPattern[260];
    GE_FIND_DATA currentData;
    
    #ifdef _WIN32
               HANDLE  winHandle;
    #else
               DIR    *dir;
        struct dirent *entry;
               char    basePath[260];
    #endif
} 
GE_FIND_HANDLE;

typedef struct 
GE_FILE_HANDLE 
{
    bool valid;
    #ifdef _WIN32
        HANDLE winHandle;
    #else
        int   fd;
        FILE *fp;  // Optional stdio FILE* for convenience
        char  filename[260];
    #endif
} 
GE_FILE_HANDLE;

typedef struct
GE_FIND_FILE
{
    DIR  *dir;
    char *filter;
}
GE_FIND_FILE;

//#define GE_INVALID_HANDLE_VALUE ((GE_FIND_HANDLE*)NULL)

typedef struct DynamicLibrary DynamicLibrary;
typedef struct geFindData     geFindData;

static void            geNormalizePath(             char           *path); 
inline bool            geGetUserName(               char           *player_name,              uint32                        *size);
inline uint32          geGetCurrentDir(             uint32          size,                     char                          *path);
inline char           *geGetExecutablePath(void);

       GE_FIND_FILE   *geFindFirstFile(    const    char           *lpFileName,               GE_FIND_DATA                  *lpFindFileData);
       bool            geFindNextFile(              GE_FIND_FILE   *hFindFile,                GE_FIND_DATA                  *lpFindFileData);
       bool            geFindClose(                 GE_FIND_FILE   *hFindFile);
       bool            geCloseHandle(               void           *hObject);
       bool            geGetFileSize(               void           *hFile,                    uint32                        *lpFileSizeHigh);
       bool            geGetFileInformationByHandle(void           *hFile,                    GE_BY_HANDLE_FILE_INFORMATION *lpFileInformation);
       bool            geSetEndOfFile(              void           *hFile);
       bool            geSetFileAttributes(const    char           *lpFileName,               uint32                         dwFileAttributes);
       bool            geDeleteFile(       const    char           *lpFileName);
       bool            geMoveFile(         const    char           *lpExistingFileName, const char                          *lpNewFileName);
       bool            geCreateDirectory(  const    char           *lpPathName,               void                          *lpSecurityAttributes);
       uint32          geGetFileAttributes(const    char           *lpFileName);
inline uint32          geGetLastError(void);

void *geCreateFile(
    const char   *lpFileName, 
          uint32  dwDesiredAccess,
          uint32  dwShareMode, 
          void   *lpSecurityAttributes,
          uint32  dwCreationDisposition, 
          uint32  dwFlagsAndAttributes,
          void   *hTemplateFile
);
bool geWriteFile(
          void   *hFile, 
    const void   *lpBuffer, 
          uint32  nNumberOfBytesToWrite,
          uint32 *lpNumberOfBytesWritten, 
          void   *lpOverlapped
);
uint32 geSetFilePointer(
    void   *hFile, 
    int32   lDistanceToMove,
    int32  *lpDistanceToMoveHigh, 
    uint32  dwMoveMethod
);
bool geReadFile(
    void *hFile, 
    void *lpBuffer, 
    uint32 nNumberOfBytesToRead,
    uint32 *lpNumberOfBytesRead, 
    void *lpOverlapped
);
bool geSetFileTime(
          void   *hFile, 
    const uint32 *lpCreationTime,
    const uint32 *lpLastAccessTime,
    const uint32 *lpLastWriteTime
);


char *itoa(int value, char* str, int base);
void splitpath(
    const char *path, 
          char *drive, 
          char *dir, 
          char *fname, 
          char *ext
);


#endif /* X_PLATFORM_UTILS */
