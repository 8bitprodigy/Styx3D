/*
    FSOps.C
    Author: Chris DeBoy
    Description: Platform-agnostic reimplementation of filesystem interface originally defined in FSDOS.h and FSDOS.c

    This code is released into the public domain or licensed under the terms of the 0BSD license
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "BaseType.h"
#include "FSOps.h"
#include "RAM.h"
#include "VFile.h"
#include "VFile_private.h"

/*    */
#define CHECK_HANDLE(Handle)
#define CHECK_FINDER(Finder)


typedef struct
FSFile
{
          unsigned int        Signature;
                   void      *FileHandle;
                   char      *FullPath;
    const          char      *Name;
                   geBoolean  IsDirectory;
}
FSFile;

typedef struct
FSFinder
{
    unsigned int        Signature;
             void      *FindHandle;
             void      *FindData;
             geBoolean  FirstStillCached;
             int        OffsetToName;
}
FSFinder;


static void *GENESISCC
FS_FinderCreate(
          geVFile *FS,
          void    *Handle,
    const char    *FileSpec
)
{
    /*
        TODO: everything
    */
    return NULL;
} /* FS_FinderCreate */

static geBoolean GENESISCC
FS_FinderGetNextFile(void *Handle)
{
    /*
        TODO: everything
    */
    return GE_FALSE;
} /* FS_FinderGetNextFile */

static geBoolean GENESISCC
FS_FinderGetproperties(void *Handle, geVFile_Properties *Props)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_FinderGetproperties */

static void GENESISCC
FS_FinderDestroy(void *Handle)
{
    return;
} /* FS_FinderDestroy */

static void *GENESISCC
FS_Open(
    geVFile *FS,
                   void *Handle,
    const          char *Name,
                   void *Context,
          unsigned int   OpenModeFlags
)
{
    return NULL;
} /* FS_Open */

static void *GENESISCC
FS_OpenNewSystem(
          geVFile      *FS,
    const char         *Name,
          void         *Context,
          unsigned int  OpenModeFlags
)
{
    return FS_Open(FS, NULL, Name, Context, OpenModeFlags);
} /* FS_OpenNewSystem */

static geBoolean GENESISCC
FS_UpdateContext(
    geVFile *FS,
    void    *Handle,
    void    *Context,
    int      ContextSize
)
{
    return GE_FALSE;
}

static void GENESISCC
FS_Close(void *Handle)
{
    /*
        TODO
    */
    return;
} /* FS_CLOSE */

static geBoolean GENESISCC
FS_GetS(void *Handle, void *Buff, int MaxLen)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FS_GetS */

static geBoolean GENESISCC
FS_Read(void *Handle, void *Buff, int Count)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_Read */

static geBoolean GENESISCC
FS_Write(void *Handle, const void *Buff, int Count)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_Write */

static geBoolean GENESISCC
FS_Seek(void *Handle, int Where, geVFile_Whence Whence)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_Seek */

static geBoolean GENESISCC
FS_EOF(const void *Handle)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_EOF */

static geBoolean GENESISCC
FS_Tell(const void *Handle, long *Position)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_Tell */

static geBoolean GENESISCC
FS_Size(const void *Handle, long *Size)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FS_Size */

static geBoolean GENESISCC
FS_GetProperties(const void *Handle, geVFile_Properties *Properties)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_GetProperties */

static geBoolean GENESISCC
FS_SetSize(void *Handle, long size)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FS_SetSize */

static geBoolean GENESISCC
FS_SetAttributes(void *Handle, geVFile_Attributes Attributes)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_SetAttributes */

static geBoolean GENESISCC
FS_SetTime(void *Handle, const geVFile_Time *Time)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_SetTime */

static geBoolean GENESISCC
FS_SetHints(void *Handle, const geVFile_Hints *Hints)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FS_SetHints */

static geBoolean GENESISCC
FS_FileExists(geVFile *FS, void *Handle, const char *Name)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FS_FileExists */

static geBoolean GENESISCC
FS_Disperse(
          geVFile   *FS,
          void      *Handle,
    const char      *Directory,
          geBoolean  Recursive
)
{
    return GE_FALSE;
} /* FS_Disperse */

static geBoolean GENESISCC
FS_DeleteFile(geVFile *FS, void *Handle, const char *Name)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_DeleteFile */

static geBoolean GENESISCC
FS_RenameFile(geVFile *FS, void *Handle, const char *Name, const char *NewName)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FS_RenameFile */


static geVFile_SystemAPIs
FS_APIs =
{
    FS_FinderCreate,
    FS_FinderGetNextFile,
    FS_FinderGetproperties,
    FS_FinderDestroy,

    FS_OpenNewSystem,
    FS_UpdateContext,
    FS_Open,
    FS_DeleteFile,
    FS_RenameFile,
    FS_FileExists,
    FS_Disperse,
    FS_Close,

    FS_GetS,
    FS_Read,
    FS_Write,
    FS_Seek,
    FS_EOF,
    FS_Tell,
    FS_Size,

    FS_GetProperties,

    FS_SetSize,
    FS_SetAttributes,
    FS_SetTime,
    FS_SetHints,
};

const geVFile_SystemAPIs *GENESISCC
FSDos_GetAPIs(void)
{
    return &FS_APIs;
}
