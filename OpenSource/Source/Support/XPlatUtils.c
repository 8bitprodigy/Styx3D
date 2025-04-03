#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>
#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <limits.h>
#else
    #include <dlfcn.h>
    #include <limits.h>
    #include <pwd.h>
    #include <unistd.h>
#endif

#include "GETypes.h"
#include "XPlatUtils.h"


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
    /* Windows implementation */
    path = malloc(MAX_PATH);
    if (!path) return NULL;
    
    DWORD result = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (result == 0 || result == MAX_PATH) {
        free(path);
        return NULL;
    }
#elif defined(__APPLE__)
    /* macOS implementation */
    path = malloc(PATH_MAX);
    if (!path) return NULL;
    
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(path, &size) != 0) {
        free(path);
        return NULL;
    }
#else
    /* Linux implementation */
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


bool 
geFindData_populate(const char *path, geFindData *data)
 {
    memset(data, 0, sizeof(geFindData));
    
    /* Get file information using SDL_RWops */
    SDL_RWops* file = SDL_RWFromFile(path, "rb");
    if (!file) return false;
    
    /* Get file size */
    data->fileSize = SDL_RWsize(file);
    
    /* Get current time for timestamps */
    Uint64 currentTime = SDL_GetTicks64();
    data->creationTime = currentTime;
    data->lastAccessTime = currentTime;
    data->lastWriteTime = currentTime;
    
    /* Get filename from path */
    const char *lastSlash = strrchr(path, '/');
    if (lastSlash) {
        strncpy(data->filename, lastSlash + 1, sizeof(data->filename) - 1);
    } else {
        strncpy(data->filename, path, sizeof(data->filename) - 1);
    }
    
    /* Store full path */
    strncpy(data->fullpath, path, sizeof(data->fullpath) - 1);
    
    /* Set basic attributes */
    data->attributes = GE_ATTR_NORMAL;
    
    SDL_RWclose(file);
    return true;
}

void * 
geFindFirstFile(const char *file_name, geFindData *find_data) 
{
/*
    geFile *file = malloc(sizeof(geFile));
    if (!file) return NULL;
    
    memset(find_data, 0, sizeof(geFindData));
    
    SDL_RWops* dirHandle = SDL_RWFromFile(file_name, "rb");
    if (!dirHandle) {
        free(file);
        return NULL;
    }
    
    file->currentFile = dirHandle;
    file->platformData = NULL;
    
    if (geFindData_populate(file_name, find_data)) {
        return file;
    }
    
    SDL_RWclose(dirHandle);
    free(file);
*/
    return NULL;
}

bool 
geFindNextFile(void *handle, geFindData *find_data)
{
/*
    DirectorySearchState* searchState = (DirectorySearchState*)handle;
    if (!searchState || !searchState->currentFile) return false;
    
    int64 fileSize = SDL_RWsize(searchState->currentFile);
    if (fileSize < 0) return false;
    
    int64 currentPosition = SDL_RWtell(searchState->currentFile);
    if (currentPosition < 0) return false;
    
    char filename[PATH_MAX];
    uint64 bytesRead = SDL_RWread(searchState->currentFile, filename, 1, 256);
    if (bytesRead <= 0) return false;
    
    // Populate find_data structure
    memset(find_data, 0, sizeof(geFindData));
    strncpy(find_data->filename, filename, sizeof(find_data->filename) - 1);
    find_data->fileSize = (uint64)fileSize;
*/
    return true;
}

bool 
geFindClose(void *handle) 
{
/*
    DirectorySearchState *searchState = (DirectorySearchState*)handle;
    if (!searchState) return false;
    
    if (searchState->currentFile) {
        SDL_RWclose(searchState->currentFile);
    }
    free(searchState);
*/
    return true;
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
