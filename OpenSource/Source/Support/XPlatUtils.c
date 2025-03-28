#include "XPlatUtils.h"

#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#else
    #include <dlfcn.h>
    #include <pwd.h>
    #include <unistd.h>
#endif


geModule geLoadLibrary(const char* path) {
#ifdef _WIN32
    return LoadLibraryExA(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
    return dlopen(path, RTLD_LAZY);
#endif
}

void* geGetProcAddress(geModule lib, const char* symbol) {
#ifdef _WIN32
    return GetProcAddress((HMODULE)lib, symbol);
#else
    return dlsym(lib, symbol);
#endif
}

int geFreeLibrary(geModule lib) {
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

inline uint32
geGetCurrentDir(uint32 size, char *path)
{
    if (!path || size == 0) return false;
    
#ifdef _WIN32
    return GetCurrentDirectory((DWORD)size, path);
#else
    return getcwd(path, size);
#endif
}
