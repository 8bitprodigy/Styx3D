#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <dlfcn.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <limits.h>
    #include <pwd.h>
    #include <sys/stat.h>
    #include <time.h>
    #include <unistd.h>
    #include <utime.h>
#endif
#if defined(__APPLE__)
    #include <mach-o/dyld.h>
#endif

#include "GETypes.h"
#include "XPlatUtils.h"

static uint32 geLastError = GE_ERROR_SUCCESS;


static void geNormalizePath(char *path) {
    char *p = path;
    while (*p) {
            if (*p == '\\' || *p == '/') *p = PATH_SEPARATOR;
        p++;
    }
}

bool
geGetUserName(char *player_name, uint32 *size)
{
#ifdef _WIN32
    uint16 len = size;
    if (GetUserName(player_name, &len)) {
        *size = len;
        return true;
    }
#else
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        strncpy(player_name, pw->pw_name, *size);
        player_name[*size - 1] = '\0'; 
        *size = strlen(player_name); 
        return true;
    }
#endif
    return false;
}

uint32
geGetCurrentDir(uint32 size, char *path)
{
    if (!path || size == 0) return false;
    
#ifdef _WIN32
    return GetCurrentDirectory((DWORD)size, path);
#else
    if (getcwd(path, size) == NULL) return 0; 
    return strlen(path); 
#endif
}

char * 
geGetExecutablePath() 
{
    char* path = NULL;
    
#ifdef _WIN32
    path = malloc(MAX_PATH);
    if (!path) return NULL;
    
    DWORD result = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (result == 0 || result == MAX_PATH) {
        free(path);
        return NULL;
    }
#elif defined(__APPLE__)
    path = malloc(PATH_MAX);
    if (!path) return NULL;
    
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(path, &size) != 0) {
        free(path);
        return NULL;
    }
#else
    path = malloc(PATH_MAX);
    if (!path) return NULL;
    
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count <= 0) {
        free(path);
        return NULL;
    }
    path[count] = '\0';  /* readlink doesn't null-terminate */
#endif

    return path;
}

static bool 
matchPattern(const char* filename, const char* pattern) 
{
    /* Simple pattern matching - implement more sophisticated matching as needed
        Currently handles * as wildcard for any characters
    */
    
    if (strcmp(pattern, "*") == 0) {
        return true;
    }
    
    // Exact match
    if (strchr(pattern, '*') == NULL && strchr(pattern, '?') == NULL) {
        return strcmp(filename, pattern) == 0;
    }
    
    // Handle *.ext pattern
    if (pattern[0] == '*' && pattern[1] == '.') {
        const char* fileExt = strrchr(filename, '.');
        if (fileExt == NULL) return false;
        return strcmp(fileExt, pattern + 1) == 0;
    }
    
    // For more complex patterns, consider implementing proper glob matching
    return false;
}

static void 
extractPatternFromPath(const char* path, char* dirPath, char* pattern) 
{
    const char* lastSlash = strrchr(path, PATH_SEPARATOR);
    
    if (lastSlash) {
        size_t dirLen = lastSlash - path + 1;
        strncpy(dirPath, path, dirLen);
        dirPath[dirLen] = '\0';
        strcpy(pattern, lastSlash + 1);
    } else {
        /* No directory in path */
        dirPath[0] = '.';
        dirPath[1] = PATH_SEPARATOR;
        dirPath[2] = '\0';
        strcpy(pattern, path);
    }
    
    if (strlen(pattern) == 0) {
        strcpy(pattern, "*");
    }
}

static void 
fillFileData(GE_FIND_DATA* lpFindFileData, const char* filePath, const char* fileName) 
{
    memset(lpFindFileData, 0, sizeof(GE_FIND_DATA));
    strncpy(lpFindFileData->fileName, fileName, sizeof(lpFindFileData->fileName) - 1);
    
#ifdef _WIN32
    WIN32_FIND_DATA winData;
    HANDLE tempHandle = FindFirstFile(filePath, &winData);
    if (tempHandle != INVALID_HANDLE_VALUE) {
        lpFindFileData->dwFileAttributes = winData.dwFileAttributes;
        lpFindFileData->ftCreationTime[0] = winData.ftCreationTime.dwLowDateTime;
        lpFindFileData->ftCreationTime[1] = winData.ftCreationTime.dwHighDateTime;
        lpFindFileData->ftLastAccessTime[0] = winData.ftLastAccessTime.dwLowDateTime;
        lpFindFileData->ftLastAccessTime[1] = winData.ftLastAccessTime.dwHighDateTime;
        lpFindFileData->ftLastWriteTime[0] = winData.ftLastWriteTime.dwLowDateTime;
        lpFindFileData->ftLastWriteTime[1] = winData.ftLastWriteTime.dwHighDateTime;
        lpFindFileData->nFileSizeHigh = winData.nFileSizeHigh;
        lpFindFileData->nFileSizeLow = winData.nFileSizeLow;
        FindClose(tempHandle);
    }
#else
    struct stat statBuf;
    
    if (stat(filePath, &statBuf) == 0) {
        
        if (S_ISDIR(statBuf.st_mode))
            lpFindFileData->fileAttributes |= GE_FILE_ATTRIBUTE_DIRECTORY;
        
        if (!(statBuf.st_mode & S_IWUSR))
            lpFindFileData->fileAttributes |= GE_FILE_ATTRIBUTE_READONLY;
        
        /* File times - convert Unix time to equivalent representation
           This is a simplification; a real implementation might need to convert 
           between time formats more precisely
        */
        lpFindFileData->ftCreationTime[0]   = (uint32)statBuf.st_ctime;
        lpFindFileData->ftLastAccessTime[0] = (uint32)statBuf.st_atime;
        lpFindFileData->ftLastWriteTime[0]  = (uint32)statBuf.st_mtime;
        
        /* File size */
        lpFindFileData->fileSizeLow = (uint32)(statBuf.st_size & 0xFFFFFFFF);
        lpFindFileData->fileSizeHigh = (uint32)((statBuf.st_size >> 32) & 0xFFFFFFFF);
        
        /* Hidden files in Unix-like systems start with '.' */
        if (fileName[0] == '.')
            lpFindFileData->fileAttributes |= GE_FILE_ATTRIBUTE_HIDDEN;
    }
    
    /* Handle special cases for . and .. */
    if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) {
        lpFindFileData->fileAttributes = GE_FILE_ATTRIBUTE_DIRECTORY;
    }
#endif
}

#ifndef _WIN32
static int 
mapAccessModeToFlags(uint32 dwDesiredAccess, uint32 dwCreationDisposition)
{
    int flags = 0;
    
    // Access mode
    if ((dwDesiredAccess & GE_GENERIC_READ) && (dwDesiredAccess & GE_GENERIC_WRITE))
        flags |= O_RDWR;
    else if (dwDesiredAccess & GE_GENERIC_WRITE)
        flags |= O_WRONLY;
    else
        flags |= O_RDONLY;
    
    // Creation disposition
    switch (dwCreationDisposition) {
        case GE_CREATE_NEW:
            flags |= O_CREAT | O_EXCL;
            break;
        case GE_CREATE_ALWAYS:
            flags |= O_CREAT | O_TRUNC;
            break;
        case GE_OPEN_EXISTING:
            // No additional flags needed
            break;
        case GE_OPEN_ALWAYS:
            flags |= O_CREAT;
            break;
        case GE_TRUNCATE_EXISTING:
            flags |= O_TRUNC;
            break;
    }
    
    return flags;
}

// Map Windows error code to equivalent POSIX
static uint32 mapErrnoToWinError() {
    switch (errno) {
        case ENOENT:          return GE_ERROR_FILE_NOT_FOUND;
        case EACCES:          return GE_ERROR_ACCESS_DENIED;
        case EBADF:           return GE_ERROR_INVALID_HANDLE;
        case EINVAL:          return GE_ERROR_INVALID_PARAMETER;
        case ENOMEM:          return GE_ERROR_INSUFFICIENT_BUFFER;
        case EAGAIN:          return GE_ERROR_IO_PENDING;
        default:              return errno; // Return errno as-is for unmatched errors
    }
}
#endif

GE_FIND_FILE * 
geFindFirstFile(const char *File_Name, GE_FIND_DATA *Find_File_Data) 
{
#ifdef _WIN32
    return FindFirstFile(File_Name, Find_File_Data);
#else
    if (!File_Name || !Find_File_Data) {
        DBG_OUT("geFindFirstFile()\t!File_Name || !Find_File_Data");
        return GE_INVALID_HANDLE_VALUE;
    }
    
    memset(Find_File_Data, 0, sizeof(GE_FIND_DATA));
    
    if ('*' != *File_Name) {
        DBG_OUT("geFindFirstFile()\tcould not opendir()");
        return GE_INVALID_HANDLE_VALUE;
    }
    
    DIR *dir = opendir(".");
    if (!dir) {
        DBG_OUT("geFindFirstFile()\tcould not opendir()");
        return GE_INVALID_HANDLE_VALUE;
    }

    GE_FIND_FILE *find_file = (GE_FIND_FILE*)malloc(sizeof(GE_FIND_FILE));

    find_file->dir = dir;
    find_file->filter = strdup(File_Name);

    if(!geFindNextFile(find_file, Find_File_Data)){
        geFindClose(find_file);
        return GE_INVALID_HANDLE_VALUE;
    }
    return (void*)find_file;
#endif
}

bool 
geFindNextFile(GE_FIND_FILE *Find_File, GE_FIND_DATA *Find_File_Data) 
{
#ifdef _WIN32
    return FindNextFile((HANDLE)Find_File, Find_File_Data);
#else
    if (!Find_File || !Find_File_Data) {
        return false;
    }

           int     position;
    struct dirent *dir_ent;
    
    while (dir_ent = readdir(Find_File->dir)) {
        if (
               '.'    == *dir_ent->d_name
            || DT_DIR ==  dir_ent->d_type
        ) continue;

        position = strlen(dir_ent->d_name) - strlen(Find_File->filter+1);

        if (position < 0 || strcasecmp(dir_ent->d_name + position, Find_File->filter+1)) continue;
        
       strcpy(Find_File_Data->fileName, dir_ent->d_name);
       return true;
    }
    
    return false;
#endif
}

bool 
geFindClose(GE_FIND_FILE *Find_File) 
{
#ifdef _WIN32
    return FindClose((HANDLE)Find_file);
#else
    if (!Find_File || Find_File == GE_INVALID_HANDLE_VALUE) {
        DBG_OUT("geFindClose()\t!hFindFile || hFindFile == GE_INVALID_HANDLE_VALUE");
        return false;
    }
    
    if (Find_File->dir) closedir(Find_File->dir);
    
    free(Find_File->filter);
    free(Find_File);

    return false;
#endif
}

void *
geCreateFile(
    const char   *lpFileName, 
          uint32  dwDesiredAccess,
          uint32  dwShareMode, 
          void   *lpSecurityAttributes,
          uint32  dwCreationDisposition, 
          uint32  dwFlagsAndAttributes,
           void  *hTemplateFile
)
{
                       
    if (!lpFileName) {
        geLastError = GE_ERROR_INVALID_PARAMETER;
        return GE_INVALID_HANDLE_VALUE;
    }
    
    GE_FILE_HANDLE* handle = (GE_FILE_HANDLE*)malloc(sizeof(GE_FILE_HANDLE));
    if (!handle) {
        geLastError = GE_ERROR_INSUFFICIENT_BUFFER;
        return GE_INVALID_HANDLE_VALUE;
    }
    
    memset(handle, 0, sizeof(GE_FILE_HANDLE));
    
#ifdef _WIN32
    // Windows implementation
    handle->winHandle = CreateFile(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        (LPSECURITY_ATTRIBUTES)lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        (HANDLE)hTemplateFile
    );
    
    if (handle->winHandle == INVALID_HANDLE_VALUE) {
        geLastError = GetLastError();
        free(handle);
        return GE_INVALID_HANDLE_VALUE;
    }
    
    handle->valid = true;
#else
    // POSIX implementation
    int flags = mapAccessModeToFlags(dwDesiredAccess, dwCreationDisposition);
    mode_t mode = 0666;  // Default file permissions
    
    int fd = open(lpFileName, flags, mode);
    if (fd < 0) {
        geLastError = mapErrnoToWinError();
        free(handle);
        return GE_INVALID_HANDLE_VALUE;
    }
    
    handle->fd = fd;
    
    // For convenience, also open as a FILE* for some operations
    // Determine mode string based on access flags
    const char* mode_str = "r";  // Default to read-only
    
    if ((dwDesiredAccess & GE_GENERIC_READ) && (dwDesiredAccess & GE_GENERIC_WRITE)) {
        if (dwCreationDisposition == GE_TRUNCATE_EXISTING || dwCreationDisposition == GE_CREATE_ALWAYS)
            mode_str = "w+";
        else
            mode_str = "r+";
    } else if (dwDesiredAccess & GE_GENERIC_WRITE) {
        if (dwCreationDisposition == GE_TRUNCATE_EXISTING || dwCreationDisposition == GE_CREATE_ALWAYS)
            mode_str = "w";
        else
            mode_str = "a";
    }
    
    // Use fdopen to create a FILE* from the file descriptor
    handle->fp = fdopen(fd, mode_str);
    
    strncpy(handle->filename, lpFileName, sizeof(handle->filename) - 1);
    handle->valid = true;
#endif

    geLastError = GE_ERROR_SUCCESS;
    return handle;
}

bool 
geCloseHandle(void *hObject)
{
    if (!hObject || hObject == GE_INVALID_HANDLE_VALUE) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return false;
    }
    
    GE_FILE_HANDLE* handle = (GE_FILE_HANDLE*)hObject;
    
    if (!handle->valid) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return false;
    }
    
    bool result = true;
    
#ifdef _WIN32
    // Windows implementation
    result = CloseHandle(handle->winHandle);
    if (!result) {
        geLastError = GetLastError();
    }
#else
    // POSIX implementation
    if (handle->fp) {
        if (fclose(handle->fp) != 0) {
            geLastError = mapErrnoToWinError();
            result = false;
        }
    } else if (handle->fd >= 0) {
        if (close(handle->fd) != 0) {
            geLastError = mapErrnoToWinError();
            result = false;
        }
    }
#endif
    
    // Free the handle structure
    free(handle);
    
    if (result) {
        geLastError = GE_ERROR_SUCCESS;
    }
    return result;
}

bool 
geWriteFile(
    void       *hFile, 
    const void *lpBuffer, 
    uint32      nNumberOfBytesToWrite,
    uint32     *lpNumberOfBytesWritten, 
    void       *lpOverlapped
)
{
    if (!hFile || hFile == GE_INVALID_HANDLE_VALUE || !lpBuffer || !nNumberOfBytesToWrite) {
        geLastError = GE_ERROR_INVALID_PARAMETER;
        return false;
    }
    
    GE_FILE_HANDLE* handle = (GE_FILE_HANDLE*)hFile;
    
    if (!handle->valid) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return false;
    }
    
    // Initialize bytes written to 0
    if (lpNumberOfBytesWritten) {
        *lpNumberOfBytesWritten = 0;
    }
    
#ifdef _WIN32
    // Windows implementation
    BOOL result = WriteFile(
        handle->winHandle,
        lpBuffer,
        nNumberOfBytesToWrite,
        (LPDWORD)lpNumberOfBytesWritten,
        (LPOVERLAPPED)lpOverlapped
    );
    
    if (!result) {
        geLastError = GetLastError();
        return false;
    }
#else
    // POSIX implementation
    ssize_t written;
    
    // Use the FILE* if available, otherwise use the file descriptor
    if (handle->fp) {
        written = fwrite(lpBuffer, 1, nNumberOfBytesToWrite, handle->fp);
        if (written < nNumberOfBytesToWrite && ferror(handle->fp)) {
            geLastError = mapErrnoToWinError();
            return false;
        }
    } else {
        written = write(handle->fd, lpBuffer, nNumberOfBytesToWrite);
        if (written < 0) {
            geLastError = mapErrnoToWinError();
            return false;
        }
    }
    
    if (lpNumberOfBytesWritten) {
        *lpNumberOfBytesWritten = (written > 0) ? written : 0;
    }
#endif

    geLastError = GE_ERROR_SUCCESS;
    return true;
}

uint32
geSetFilePointer(
    void   *hFile, 
    int32   lDistanceToMove,
    int32  *lpDistanceToMoveHigh, 
    uint32  dwMoveMethod
)
{
    if (!hFile || hFile == GE_INVALID_HANDLE_VALUE) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return (uint32)-1; // INVALID_SET_FILE_POINTER
    }
    
    GE_FILE_HANDLE* handle = (GE_FILE_HANDLE*)hFile;
    
    if (!handle->valid) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return (uint32)-1;
    }
    
#ifdef _WIN32
    // Windows implementation
    DWORD result = SetFilePointer(
        handle->winHandle,
        lDistanceToMove,
        (PLONG)lpDistanceToMoveHigh,
        dwMoveMethod
    );
    
    if (result == INVALID_SET_FILE_POINTER) {
        geLastError = GetLastError();
        return (uint32)-1;
    }
    
    return result;
#else
    // POSIX implementation
    int whence;
    switch (dwMoveMethod) {
        case GE_FILE_BEGIN:
            whence = SEEK_SET;
            break;
        case GE_FILE_CURRENT:
            whence = SEEK_CUR;
            break;
        case GE_FILE_END:
            whence = SEEK_END;
            break;
        default:
            geLastError = GE_ERROR_INVALID_PARAMETER;
            return (uint32)-1;
    }
    
    // Use high part if provided
    off_t distance = lDistanceToMove;
    if (lpDistanceToMoveHigh) {
        distance |= ((off_t)*lpDistanceToMoveHigh << 32);
    }
    
    off_t newPos;
    
    // Use FILE* if available, otherwise use file descriptor
    if (handle->fp) {
        if (fseeko(handle->fp, distance, whence) != 0) {
            geLastError = mapErrnoToWinError();
            return (uint32)-1;
        }
        newPos = ftello(handle->fp);
    } else {
        newPos = lseek(handle->fd, distance, whence);
        if (newPos == (off_t)-1) {
            geLastError = mapErrnoToWinError();
            return (uint32)-1;
        }
    }
    
    // Set high part if provided
    if (lpDistanceToMoveHigh) {
        *lpDistanceToMoveHigh = (int32)(newPos >> 32);
    }
    
    geLastError = GE_ERROR_SUCCESS;
    return (uint32)(newPos & 0xFFFFFFFF);
#endif
}

uint32
geGetLastError(void)
{
#ifdef _WIN32
    // On Windows, we can use the native GetLastError()
    return GetLastError();
#else
    // On other platforms, return our thread-local error code
    return geLastError;
#endif
}


bool geReadFile(void* hFile, void* lpBuffer, uint32 nNumberOfBytesToRead,
               uint32* lpNumberOfBytesRead, void* lpOverlapped) {
    if (!hFile || hFile == GE_INVALID_HANDLE_VALUE || !lpBuffer) {
        geLastError = GE_ERROR_INVALID_PARAMETER;
        return false;
    }
    
    GE_FILE_HANDLE* handle = (GE_FILE_HANDLE*)hFile;
    
    if (!handle->valid) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return false;
    }
    
    // Initialize bytes read to 0
    if (lpNumberOfBytesRead) {
        *lpNumberOfBytesRead = 0;
    }
    
#ifdef _WIN32
    // Windows implementation
    BOOL result = ReadFile(
        handle->winHandle,
        lpBuffer,
        nNumberOfBytesToRead,
        (LPDWORD)lpNumberOfBytesRead,
        (LPOVERLAPPED)lpOverlapped
    );
    
    if (!result) {
        geLastError = GetLastError();
        return false;
    }
#else
    // POSIX implementation
    ssize_t bytesRead;
    
    // Use the FILE* if available, otherwise use the file descriptor
    if (handle->fp) {
        bytesRead = fread(lpBuffer, 1, nNumberOfBytesToRead, handle->fp);
        if (bytesRead < nNumberOfBytesToRead && ferror(handle->fp)) {
            geLastError = mapErrnoToWinError();
            return false;
        }
    } else {
        bytesRead = read(handle->fd, lpBuffer, nNumberOfBytesToRead);
        if (bytesRead < 0) {
            geLastError = mapErrnoToWinError();
            return false;
        }
    }
    
    if (lpNumberOfBytesRead) {
        *lpNumberOfBytesRead = (bytesRead > 0) ? bytesRead : 0;
    }
#endif

    geLastError = GE_ERROR_SUCCESS;
    return true;
}

// ---- GetFileSize Implementation ----

bool geGetFileSize(void* hFile, uint32* lpFileSizeHigh) {
    if (!hFile || hFile == GE_INVALID_HANDLE_VALUE) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return 0;
    }
    
    GE_FILE_HANDLE* handle = (GE_FILE_HANDLE*)hFile;
    
    if (!handle->valid) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return 0;
    }
    
#ifdef _WIN32
    // Windows implementation
    DWORD highSize = 0;
    DWORD lowSize = GetFileSize(handle->winHandle, &highSize);
    
    if (lowSize == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
        geLastError = GetLastError();
        return 0;
    }
    
    if (lpFileSizeHigh) {
        *lpFileSizeHigh = highSize;
    }
    
    return lowSize;
#else
    // POSIX implementation
    off_t currentPos = 0;
    off_t size = 0;
    
    // Save current position
    if (handle->fp) {
        currentPos = ftello(handle->fp);
        if (currentPos < 0) {
            geLastError = mapErrnoToWinError();
            return 0;
        }
        
        // Seek to end to get size
        if (fseeko(handle->fp, 0, SEEK_END) != 0) {
            geLastError = mapErrnoToWinError();
            return 0;
        }
        
        size = ftello(handle->fp);
        
        // Restore original position
        if (fseeko(handle->fp, currentPos, SEEK_SET) != 0) {
            geLastError = mapErrnoToWinError();
            return 0;
        }
    } else {
        currentPos = lseek(handle->fd, 0, SEEK_CUR);
        if (currentPos < 0) {
            geLastError = mapErrnoToWinError();
            return 0;
        }
        
        // Seek to end to get size
        size = lseek(handle->fd, 0, SEEK_END);
        if (size < 0) {
            geLastError = mapErrnoToWinError();
            return 0;
        }
        
        // Restore original position
        if (lseek(handle->fd, currentPos, SEEK_SET) < 0) {
            geLastError = mapErrnoToWinError();
            return 0;
        }
    }
    
    if (lpFileSizeHigh) {
        *lpFileSizeHigh = (uint32)(size >> 32);
    }
    
    geLastError = GE_ERROR_SUCCESS;
    return (uint32)(size & 0xFFFFFFFF);
#endif
}

// ---- GetFileInformationByHandle Implementation ----

bool 
geGetFileInformationByHandle(void *hFile, GE_BY_HANDLE_FILE_INFORMATION *lpFileInformation)
{
    if (!hFile || hFile == GE_INVALID_HANDLE_VALUE || !lpFileInformation) {
        geLastError = GE_ERROR_INVALID_PARAMETER;
        return false;
    }
    
    GE_FILE_HANDLE* handle = (GE_FILE_HANDLE*)hFile;
    
    if (!handle->valid) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return false;
    }
    
    // Initialize structure
    memset(lpFileInformation, 0, sizeof(GE_BY_HANDLE_FILE_INFORMATION));
    
#ifdef _WIN32
    // Windows implementation
    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(handle->winHandle, &info)) {
        geLastError = GetLastError();
        return false;
    }
    
    lpFileInformation->dwFileAttributes = info.dwFileAttributes;
    lpFileInformation->ftCreationTime[GE_LOW] = info.ftCreationTime.dwLowDateTime;
    lpFileInformation->ftCreationTime[GE_HIGH] = info.ftCreationTime.dwHighDateTime;
    lpFileInformation->ftLastAccessTime[GE_LOW] = info.ftLastAccessTime.dwLowDateTime;
    lpFileInformation->ftLastAccessTime[GE_HIGH] = info.ftLastAccessTime.dwHighDateTime;
    lpFileInformation->ftLastWriteTime[GE_LOW] = info.ftLastWriteTime.dwLowDateTime;
    lpFileInformation->ftLastWriteTime[GE_HIGH] = info.ftLastWriteTime.dwHighDateTime;
    lpFileInformation->dwVolumeSerialNumber = info.dwVolumeSerialNumber;
    lpFileInformation->nFileSizeHigh = info.nFileSizeHigh;
    lpFileInformation->nFileSizeLow = info.nFileSizeLow;
    lpFileInformation->nNumberOfLinks = info.nNumberOfLinks;
    lpFileInformation->nFileIndexHigh = info.nFileIndexHigh;
    lpFileInformation->nFileIndexLow = info.nFileIndexLow;
#else
    // POSIX implementation
    struct stat st;
    int result;
    
    if (handle->fp) {
        result = fstat(fileno(handle->fp), &st);
    } else {
        result = fstat(handle->fd, &st);
    }
    
    if (result != 0) {
        geLastError = mapErrnoToWinError();
        return false;
    }
    
    // Fill in the structure with the available information
    // File attributes
    if (S_ISDIR(st.st_mode))
        lpFileInformation->dwFileAttributes |= GE_FILE_ATTRIBUTE_DIRECTORY;
    if (!(st.st_mode & S_IWUSR))
        lpFileInformation->dwFileAttributes |= GE_FILE_ATTRIBUTE_READONLY;
    
    // File times
    lpFileInformation->ftCreationTime[0] = (uint32)st.st_ctime;
    lpFileInformation->ftLastAccessTime[0] = (uint32)st.st_atime;
    lpFileInformation->ftLastWriteTime[0] = (uint32)st.st_mtime;
    
    // File size
    lpFileInformation->nFileSizeLow = (uint32)(st.st_size & 0xFFFFFFFF);
    lpFileInformation->nFileSizeHigh = (uint32)((st.st_size >> 32) & 0xFFFFFFFF);
    
    // Number of links
    lpFileInformation->nNumberOfLinks = st.st_nlink;
    
    // File index (inode on POSIX)
    lpFileInformation->nFileIndexLow = (uint32)(st.st_ino & 0xFFFFFFFF);
    lpFileInformation->nFileIndexHigh = (uint32)((st.st_ino >> 32) & 0xFFFFFFFF);
    
    // Volume serial number (device ID on POSIX)
    lpFileInformation->dwVolumeSerialNumber = st.st_dev;
#endif

    geLastError = GE_ERROR_SUCCESS;
    return true;
}

// ---- SetEndOfFile Implementation ----

bool 
geSetEndOfFile(void* hFile)
{
    if (!hFile || hFile == GE_INVALID_HANDLE_VALUE) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return false;
    }
    
    GE_FILE_HANDLE* handle = (GE_FILE_HANDLE*)hFile;
    
    if (!handle->valid) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return false;
    }
    
#ifdef _WIN32
    // Windows implementation
    if (!SetEndOfFile(handle->winHandle)) {
        geLastError = GetLastError();
        return false;
    }
#else
    // POSIX implementation
    off_t currentPos;
    
    if (handle->fp) {
        currentPos = ftello(handle->fp);
        if (currentPos < 0) {
            geLastError = mapErrnoToWinError();
            return false;
        }
        
        if (ftruncate(fileno(handle->fp), currentPos) != 0) {
            geLastError = mapErrnoToWinError();
            return false;
        }
    } else {
        currentPos = lseek(handle->fd, 0, SEEK_CUR);
        if (currentPos < 0) {
            geLastError = mapErrnoToWinError();
            return false;
        }
        
        if (ftruncate(handle->fd, currentPos) != 0) {
            geLastError = mapErrnoToWinError();
            return false;
        }
    }
#endif

    geLastError = GE_ERROR_SUCCESS;
    return true;
}

// ---- SetFileAttributes Implementation ----

bool 
geSetFileAttributes(const char *lpFileName, uint32 dwFileAttributes)
{
    if (!lpFileName) {
        geLastError = GE_ERROR_INVALID_PARAMETER;
        return false;
    }
    
#ifdef _WIN32
    // Windows implementation
    if (!SetFileAttributes(lpFileName, dwFileAttributes)) {
        geLastError = GetLastError();
        return false;
    }
#else
    // POSIX implementation
    struct stat st;
    if (stat(lpFileName, &st) != 0) {
        geLastError = mapErrnoToWinError();
        return false;
    }
    
    mode_t mode = st.st_mode;
    
    // Handle read-only attribute
    if (dwFileAttributes & GE_FILE_ATTRIBUTE_READONLY) {
        // Remove write permissions
        mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
    } else {
        // Add write permissions (for owner only)
        mode |= S_IWUSR;
    }
    
    // Apply the new permissions
    if (chmod(lpFileName, mode) != 0) {
        geLastError = mapErrnoToWinError();
        return false;
    }
    
    // Note: Many Windows file attributes don't have direct POSIX equivalents
    // (HIDDEN, SYSTEM, etc.) and are ignored in this implementation
#endif

    geLastError = GE_ERROR_SUCCESS;
    return true;
}

// ---- SetFileTime Implementation ----

bool 
geSetFileTime(
          void   *hFile, 
    const uint32 *lpCreationTime,
    const uint32 *lpLastAccessTime,
    const uint32 *lpLastWriteTime
) 
{
    if (!hFile || hFile == GE_INVALID_HANDLE_VALUE) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return false;
    }
    
    GE_FILE_HANDLE* handle = (GE_FILE_HANDLE*)hFile;
    
    if (!handle->valid) {
        geLastError = GE_ERROR_INVALID_HANDLE;
        return false;
    }
    
#ifdef _WIN32
    // Windows implementation
    FILETIME creationTime, lastAccessTime, lastWriteTime;
    LPFILETIME pCreationTime = NULL, pLastAccessTime = NULL, pLastWriteTime = NULL;
    
    if (lpCreationTime) {
        creationTime.dwLowDateTime = lpCreationTime[0];
        creationTime.dwHighDateTime = lpCreationTime[1];
        pCreationTime = &creationTime;
    }
    
    if (lpLastAccessTime) {
        lastAccessTime.dwLowDateTime = lpLastAccessTime[0];
        lastAccessTime.dwHighDateTime = lpLastAccessTime[1];
        pLastAccessTime = &lastAccessTime;
    }
    
    if (lpLastWriteTime) {
        lastWriteTime.dwLowDateTime = lpLastWriteTime[0];
        lastWriteTime.dwHighDateTime = lpLastWriteTime[1];
        pLastWriteTime = &lastWriteTime;
    }
    
    if (!SetFileTime(handle->winHandle, pCreationTime, pLastAccessTime, pLastWriteTime)) {
        geLastError = GetLastError();
        return false;
    }
#else
    // POSIX implementation
    struct utimbuf times;
    struct stat st;
    int fd;
    
    if (handle->fp) {
        fd = fileno(handle->fp);
    } else {
        fd = handle->fd;
    }
    
    if (fstat(fd, &st) != 0) {
        geLastError = mapErrnoToWinError();
        return false;
    }
    
    // Initialize with current times
    times.actime = st.st_atime;
    times.modtime = st.st_mtime;
    
    // Update access time if provided
    if (lpLastAccessTime) {
        times.actime = lpLastAccessTime[0];
    }
    
    // Update modification time if provided
    if (lpLastWriteTime) {
        times.modtime = lpLastWriteTime[0];
    }
    
    // Apply the new times - note that creation time is not supported in POSIX
    if (utime(handle->filename, &times) != 0) {
        geLastError = mapErrnoToWinError();
        return false;
    }
    
    // Note: CreationTime is ignored in POSIX as it doesn't have a direct equivalent
#endif

    geLastError = GE_ERROR_SUCCESS;
    return true;
}

bool 
geDeleteFile(const char* lpFileName)
{
    if (!lpFileName) {
        geLastError = GE_ERROR_INVALID_PARAMETER;
        return false;
    }
    
#ifdef _WIN32
    // Windows implementation
    if (!DeleteFile(lpFileName)) {
        geLastError = GetLastError();
        return false;
    }
#else
    // POSIX implementation
    if (unlink(lpFileName) != 0) {
        geLastError = mapErrnoToWinError();
        return false;
    }
#endif

    geLastError = GE_ERROR_SUCCESS;
    return true;
}

bool 
geMoveFile(const char* lpExistingFileName, const char* lpNewFileName)
{
    if (!lpExistingFileName || !lpNewFileName) {
        geLastError = GE_ERROR_INVALID_PARAMETER;
        return false;
    }
    
#ifdef _WIN32
    // Windows implementation
    if (!MoveFile(lpExistingFileName, lpNewFileName)) {
        geLastError = GetLastError();
        return false;
    }
#else
    // POSIX implementation
    if (rename(lpExistingFileName, lpNewFileName) != 0) {
        geLastError = mapErrnoToWinError();
        return false;
    }
#endif

    geLastError = GE_ERROR_SUCCESS;
    return true;
}

bool 
geCreateDirectory(const char* lpPathName, void* lpSecurityAttributes) 
{
    if (!lpPathName) {
        geLastError = GE_ERROR_INVALID_PARAMETER;
        return false;
    }
    
#ifdef _WIN32
    // Windows implementation
    bool result = CreateDirectory(lpPathName, (LPSECURITY_ATTRIBUTES)lpSecurityAttributes);
    
    if (!result) {
        geLastError = GetLastError();
        return false;
    }
#else
    // POSIX implementation
    // Default directory permissions (rwxr-xr-x)
    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    
    if (mkdir(lpPathName, mode) != 0) {
        geLastError = mapErrnoToWinError();
        return false;
    }
#endif

    geLastError = GE_ERROR_SUCCESS;
    return true;
}

uint32
geGetFileAttributes(const char *lpFileName)
{
    if (!lpFileName) {
        geLastError = GE_ERROR_INVALID_PARAMETER;
        return GE_INVALID_FILE_ATTRIBUTES;
    }
    
#ifdef _WIN32
    uint32 attrs = GetFileAttributes(lpFileName);
    if (attrs == GE_INVALID_FILE_ATTRIBUTES) {
        geLastError = GetLastError();
    } else {
        geLastError = GE_ERROR_SUCCESS;
    }
    return attrs;
#else
    struct stat statBuf;
    
    if (stat(lpFileName, &statBuf) != 0) {
        geLastError = mapErrnoToWinError();
        return GE_INVALID_FILE_ATTRIBUTES;
    }
    
    uint32 attributes = 0;
    
    /* Map POSIX mode flags to Windows attributes */
    if (S_ISDIR(statBuf.st_mode))
        attributes |= GE_FILE_ATTRIBUTE_DIRECTORY;
    
    if (!(statBuf.st_mode & S_IWUSR))
        attributes |= GE_FILE_ATTRIBUTE_READONLY;
    
    /* Hidden files in Unix-like systems start with '.' */
    const char* filename = strrchr(lpFileName, '/');
    if (filename) {
        filename++; /* Skip the slash */
    } else {
        filename = lpFileName;
    }
    
    if (filename[0] == '.')
        attributes |= GE_FILE_ATTRIBUTE_HIDDEN;
    
    /* If no specific attributes, use NORMAL */
    if (attributes == 0)
        attributes = GE_FILE_ATTRIBUTE_NORMAL;
    
    geLastError = GE_ERROR_SUCCESS;
    return attributes;
#endif
}


char * 
itoa(int n, char *str, int base) 
{
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    
    int i = 0;
    int isNegative = 0;
    
    if (n < 0 && base == 10) {
        isNegative = 1;
        n = -n;
    }
    
    if (n == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    
    unsigned int num = (unsigned int) n;
    while (num != 0) {
        int remainder = num % base;
        str[i++] = (remainder < 10) ? (remainder + '0') : (remainder - 10 + 'a');
        num = num / base;
    }
    
    if (isNegative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
    
    return str;
}

void 
splitpath(
    const char *path,
          char *drive, 
          char *dir, 
          char *fname, 
          char *ext
)
{
    const char *last_slash = NULL;
    const char *dot = NULL;
    const char *p = path;
    
    /* Initialize output buffers if provided */
    if (drive) *drive = '\0';
    if (dir) *dir = '\0';
    if (fname) *fname = '\0';
    if (ext) *ext = '\0';
    
    if (!path || !*path) return;
    
    /* Handle drive letter for Windows */
    #ifdef _WIN32
    if (path[1] == ':' && (path[2] == '/' || path[2] == '\\')) {
        if (drive) {
            drive[0] = path[0];
            drive[1] = ':';
            drive[2] = '\0';
        }
        p += 2; /* Skip drive letter and colon */
    }
    #endif
    
    /* Find last slash and last dot */
    const char *ptr = p;
    while (*ptr) {
        if (*ptr == '/' || *ptr == '\\')
            last_slash = ptr;
        ptr++;
    }
    
    /* Look for extension only in the filename part */
    if (last_slash) {
        ptr = last_slash + 1;
    } else {
        ptr = p;
    }
    
    dot = strrchr(ptr, '.');
    
    /* Set directory */
    if (dir && last_slash) {
        size_t dir_len = last_slash - p + 1;  /* +1 to include the slash */
        strncpy(dir, p, dir_len);
        dir[dir_len] = '\0';
    }
    
    /* Set filename */
    if (fname) {
        const char *fname_start = last_slash ? last_slash + 1 : p;
        size_t fname_len;
        
        if (dot && dot > fname_start) {
            fname_len = dot - fname_start;
        } else {
            fname_len = strlen(fname_start);
        }
        
        strncpy(fname, fname_start, fname_len);
        fname[fname_len] = '\0';
    }
    
    /* Set extension */
    if (ext && dot) {
        strcpy(ext, dot);  /* Copy including the dot */
    }
}
