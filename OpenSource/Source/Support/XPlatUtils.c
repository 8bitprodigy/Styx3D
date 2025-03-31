#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#else
    #include <dlfcn.h>
    #include <pwd.h>
    #include <unistd.h>
#endif

#include "XPlatUtils.h"


/* [NOTE] Move these to the header later as inline */
geModule 
geLoadLibrary(const char* path) 
{
#ifdef _WIN32
    return LoadLibraryExA(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
    return dlopen(path, RTLD_LAZY);
#endif
}

void * 
geGetProcAddress(geModule lib, const char* symbol) 
{
#ifdef _WIN32
    return GetProcAddress((HMODULE)lib, symbol);
#else
    return dlsym(lib, symbol);
#endif
}

int 
geFreeLibrary(geModule lib) 
{
#ifdef _WIN32
    return FreeLibrary((HMODULE)lib) != 0;
#else
    return dlclose(lib) == 0;
#endif
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
/* [/NOTE] Move these to the header later as inline */

char * 
itoa(int n, char *str, int base) 
{
    // Check if the base is valid
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    
    // Handle negative numbers
    int i = 0;
    int isNegative = 0;
    
    if (n < 0 && base == 10) {
        isNegative = 1;
        n = -n;
    }
    
    // Special case for n = 0
    if (n == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    
    // Process individual digits
    unsigned int num = (unsigned int) n;
    while (num != 0) {
        int remainder = num % base;
        str[i++] = (remainder < 10) ? (remainder + '0') : (remainder - 10 + 'a');
        num = num / base;
    }
    
    // If the number is negative, append '-'
    if (isNegative) {
        str[i++] = '-';
    }
    
    // Null-terminate the string
    str[i] = '\0';
    
    // Reverse the string
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
