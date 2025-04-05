/*
    FSDisk.C
    Author: Chris DeBoy
    Description: Platform-agnostic reimplementation of filesystem interface originally defined in FSDOS.h and FSDOS.c

    This code is released into the public domain or licensed under the terms of the 0BSD license
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#ifdef _WIN32
	#define	WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#define  PATH_MAX _MAX_PATH
    #define PLATFORM_PATH_SEPARATOR '\\'
#else
	#include <limits.h>
	#include <stdint.h>
    #include <unistd.h>
    #define INVALID_HANDLE_VALUE ((void*)-1)
    #define PLATFORM_PATH_SEPARATOR '/'
#endif

#include "GETypes.h"
#include "FSDisk.h"
#include "RAM.h"
#include "VFile.h"
#include "VFile_private.h"
#include "XPlatUtils.h"

/* "DF01" */
#define DISK_FILE_SIGNATURE 0x31304644
/* "DF02" */
#define DISK_FINDER_SIGNATURE 0x32304644
/*    */
#define CHECK_HANDLE(Handle) do{assert(Handle);assert(Handle->Signature == DISK_FILE_SIGNATURE);  }while(0)
#define CHECK_FINDER(Finder) do{assert(Finder);assert(Finder->Signature == DISK_FINDER_SIGNATURE);}while(0)


typedef struct
DiskFile
{
          uint32          Signature;
          GE_FIND_HANDLE *FileHandle;
          char           *FullPath;
    const char           *Name;
          bool            IsDirectory;
}
DiskFile;

typedef struct
DiskFinder
{
    uint32          Signature;
    GE_FIND_HANDLE *FindHandle;
    GE_FIND_DATA   *FindData;
    bool            FirstStillCached;
    int             OffsetToName;
}
DiskFinder;


static bool	
BuildFileName(
	const DiskFile  *File,
	const char      *Name,
	      char      *Buff,
	      char     **NamePtr,
	      int        MaxLen
)
{
	int		DirLength;
	int		NameLength;

	if ( ! Name )
		return false;

	if	(File)
	{
		if	(File->IsDirectory == false)
			return false;

		assert(File->FullPath);
		DirLength = strlen(File->FullPath);

		if	(DirLength > MaxLen)
			return false;

		memcpy(Buff, File->FullPath, DirLength);
	}
	else
	{
		DirLength = 0;
	}

	NameLength = strlen(Name);
	if	(DirLength + NameLength + 2 > MaxLen || ! Buff )
		return false;

	if	(DirLength != 0)
	{
		Buff[DirLength] = '\\';
		memcpy(Buff + DirLength + 1, Name, NameLength + 1);
		if	(NamePtr)
			*NamePtr = Buff + DirLength + 1;
	}
	else
	{
		memcpy(Buff, Name, NameLength + 1);
		if	(NamePtr)
			*NamePtr = Buff;

		// Special case: no directory, no file name.  We meant something like ".\"
		if	(!*Buff)
		{
			strcpy (Buff, ".\\");
		}
	}

	return true;
} /* BuildFileName */

static void *GENESISCC
FSDisk_FinderCreate(
          geVFile        *Disk_FS,
          GE_FIND_HANDLE *Handle,
    const char           *File_Spec
)
{
    DiskFinder *finder;
    DiskFile   *file;
    char       *name;
    char        buff[PATH_MAX];

    assert(File_Spec);

    file = (DiskFile*)Handle;

    CHECK_HANDLE(file);

    finder = geRam_Allocate(sizeof(*finder));
    if (!finder) {
        return NULL;
    }

    memset(finder, 0, sizeof(*finder));

    if (!BuildFileName(file, File_Spec, buff, &name, sizeof(buff))) {
        geRam_Free(finder);
        return NULL;
    }

    finder->OffsetToName     = name - buff;
    finder->FindHandle       = geFindFirstFile(buff, finder->FindData);
    finder->FirstStillCached = true;
    finder->Signature        = DISK_FINDER_SIGNATURE;
    
    return (void *)finder;
} /* FSDisk_FinderCreate */

static bool GENESISCC
FSDisk_FinderGetNextFile(void *handle)
{
    DiskFinder *finder;

    finder = (DiskFinder*)handle;

    CHECK_FINDER(finder);

    if (finder->FindHandle == GE_INVALID_HANDLE_VALUE) return false;

    if (finder->FirstStillCached == true) {
        finder->FirstStillCached = false;
        if (finder->FindData->fileName[0] != '.') return true;
    }

    while (geFindNextFile(finder->FindHandle, finder->FindData)) 
        if (finder->FindData->fileName[0] != '.') return true;
    
    return false;
} /* FSDisk_FinderGetNextFile */

static bool GENESISCC
FSDisk_FinderGetproperties(void *Handle, geVFile_Properties *Props)
{
    DiskFinder         *finder;
    geVFile_Attributes  attributes = 0;
    int                 length;

    assert(Props);

    finder = (DiskFinder*)Handle;

    CHECK_FINDER(finder);

    if (finder->FindHandle == GE_INVALID_HANDLE_VALUE) return false; GE_ATTR_DIRECTORY

    if (finder->FindData.attributes & GE_ATTR_DIRECTORY) attributes |= GE_VFILE_ATTRIB_DIRECTORY;
    if (finder->FindData.attributes & GE_ATTR_READONLY)  attributes |= GE_VFILE_ATTRIB_READONLY;

    Props->Time.Time1 = finder->FindData.ftLastWriteTime.dwLowDateTime;
	Props->Time.Time2 = finder->FindData.ftLastWriteTime.dwHighDateTime;

	Props->AttributeFlags       = attributes;
	Props->Size                 = finder->FindData.fileSize;
	Props->Hints.HintData       = NULL;
	Props->Hints.HintDataLength = 0;

	length = strlen(finder->FindData.fileName);
	if (length > sizeof(Props->Name) -1) return false;
	memcpy(Props->Name, finder->FindData.fileName, length+1);
    
    return true;
} /* FSDisk_FinderGetproperties */

static void GENESISCC
FSDisk_FinderDestroy(void *Handle)
{
    DiskFinder *finder = (DiskFinder*)Handle;

    CHECK_FINDER(finder);

    if (finder->FindHandle != GE_INVALID_HANDLE_VALUE) geFindClose(finder->FindHandle);
    
    finder->Signature = 0;
    geRam_Free(finder);
} /* FSDisk_FinderDestroy */

static 
#ifndef _WIN32
    inline
#endif
bool
IsRootDirectory(char *Path)
{
#ifdef _WIN32
	int		SlashCount;

	// Drive letter test
	if	(Path[1] == ':' && Path[2] == '\\' && Path[3] == '\0')
	{
		Path[2] = '\0';
		return true;
	}

	// Now UNC path test
	SlashCount = 0;
	if	(Path[0] == '\\' && Path[1] == '\\')
	{
		Path += 2;
		while	(*Path)
		{
			if	(*Path++ == '\\')
				SlashCount++;
		}
	}

	if	(SlashCount == 1)
		return true;

	return false;
#else 
    return strcmp(path, "/");
#endif
}

static void *GENESISCC
FSDisk_Open(
          geVFile *disk_fs,
          void    *Handle,
    const char    *Name,
          void    *Context,
          uint32   Open_Mode_Flags
)
{
    DiskFile 
        *disk_fs,
        *new_file;

    char  buff[PATH_MAX];
    int   length;
    char *name_ptr;

    disk_fs = (DiskFile*)Handle;

    if (disk_fs && disk_fs->IsDirectory) return NULL;

    new_file = geRam_Allocate(sizeof(*new_file));
    if (!new_file) return new_file;

    memset(new_file, 0, sizeof(*new_file);

    if (!BuildFileName(disk_fs, Name, buff, &name_ptr, sizeof(Buff))) goto FAIL;

    length = strlen(buff);
    new_file->FullPath = geRam_Allocate(length+1);
    if (!new_file->FullPath) goto FAIL;

    new_file->Name = new_file->FullPath + (name_ptr - &buff[0]);

    memcpy(new_file->FullPath, buff, length + 1);

    if (Open_Mode_Flags & GE_VFILE_OPEN_DIRECTORY) {
        GE_FIND_DATA   *file_info;
        GE_FILE_HANDLE *find_handle;
        bool            is_directory;

        assert(!disk_fs || disk_fs->IsDirectory == true);

        memset(&file_info, 0, sizeof(file_info));
        find_handle = geFindFirstFile(new_file->FullPath, &file_info);
        if (find_handle != GE_INVALID_HANDLE_VALUE
            && file_info.attributes & GE_ATTR_DIRECTORY
        ) {
            is_directory = true;
        } else {
            is_directory = IsRootDirectory(new_file->FullPath);
        }

        geFindClose(find_handle);

        if (Open_Mode_Flags & GE_VFILE_OPEN_CREATE) {
            if (is_directory                                == true)  goto FAIL;
            if (geCreateDirectory(new_file->fullPath, NULL) == false) goto FAIL;
        } else if (!is_directory) goto FAIL;
        
        new_file->IsDirectory = true;
        new_file->file_handle = GE_INVALID_HANDLE_VALUE;
    } else {
        uint32
            share_mode,
            creation_mode,
            access,
            last_error;

        creation_mode = GE_OPEN_EXISTING;

        switch (Open_Mode_Flags
            & (GE_VFILE_OPEN_READONLY
            |  GE_VFILE_OPEN_UPDATE
            |  GE_VFILE_OPEN_CREATE
            )
        )
        {
        case GE_VFILE_OPEN_READONLY:
            access     = GE_READ;
            share_mode = GE_FILE_SHARE_READ | GE_FILE_SHARE_WRITE;
            break;
            
        case GE_VFILE_OPEN_CREATE:
            creation_mode = GE_CREATE_ALWAYS; /* FALLTHROUGH */
        case GEVFILE_OPEN_UPDATE:
            access     = GE_READ | GE_WRITE;
            share_mode = GE_FILE_SHARE_READ;
            break;

        default:
            assert(!"Illegal open mode flags");
            break;
        }

        new_file->FileHandle == geCreateFile(
    }

    /* FSDOS.c:332 */
    
    return NULL;
} /* FSDisk_Open */

static void *GENESISCC
FSDisk_OpenNewSystem(
          geVFile      *FS,
    const char         *Name,
          void         *Context,
          unsigned int  OpenModeFlags
)
{
    return FSDisk_Open(FS, NULL, Name, Context, OpenModeFlags);
} /* FSDisk_OpenNewSystem */

static geBoolean GENESISCC
FSDisk_UpdateContext(
    geVFile *FS,
    void    *Handle,
    void    *Context,
    int      ContextSize
)
{
    return GE_FALSE;
}

static void GENESISCC
FSDisk_Close(void *Handle)
{
    /*
        TODO
    */
    return;
} /* FSDisk_CLOSE */

static geBoolean GENESISCC
FSDisk_GetS(void *Handle, void *Buff, int MaxLen)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FSDisk_GetS */

static geBoolean GENESISCC
FSDisk_Read(void *Handle, void *Buff, int Count)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_Read */

static geBoolean GENESISCC
FSDisk_Write(void *Handle, const void *Buff, int Count)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_Write */

static geBoolean GENESISCC
FSDisk_Seek(void *Handle, int Where, geVFile_Whence Whence)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_Seek */

static geBoolean GENESISCC
FSDisk_EOF(const void *Handle)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_EOF */

static geBoolean GENESISCC
FSDisk_Tell(const void *Handle, int32 *Position)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_Tell */

static geBoolean GENESISCC
FSDisk_Size(const void *Handle, int32 *Size)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FSDisk_Size */

static geBoolean GENESISCC
FSDisk_GetProperties(const void *Handle, geVFile_Properties *Properties)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_GetProperties */

static geBoolean GENESISCC
FSDisk_SetSize(void *Handle, int32 size)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FSDisk_SetSize */

static geBoolean GENESISCC
FSDisk_SetAttributes(void *Handle, geVFile_Attributes Attributes)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_SetAttributes */

static geBoolean GENESISCC
FSDisk_SetTime(void *Handle, const geVFile_Time *Time)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_SetTime */

static geBoolean GENESISCC
FSDisk_SetHints(void *Handle, const geVFile_Hints *Hints)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FSDisk_SetHints */

static geBoolean GENESISCC
FSDisk_FileExists(geVFile *FS, void *Handle, const char *Name)
{
    /*
        TODO
    */
    return GE_FALSE;
} /* FSDisk_FileExists */

static geBoolean GENESISCC
FSDisk_Disperse(
          geVFile   *FS,
          void      *Handle,
    const char      *Directory,
          geBoolean  Recursive
)
{
    return GE_FALSE;
} /* FSDisk_Disperse */

static geBoolean GENESISCC
FSDisk_DeleteFile(geVFile *FS, void *Handle, const char *Name)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_DeleteFile */

static geBoolean GENESISCC
FSDisk_RenameFile(geVFile *FS, void *Handle, const char *Name, const char *NewName)
{
    /*
        TODO
    */
    return GE_TRUE;
} /* FSDisk_RenameFile */


static geVFile_SystemAPIs
FSDisk_APIs =
{
    FSDisk_FinderCreate,
    FSDisk_FinderGetNextFile,
    FSDisk_FinderGetproperties,
    FSDisk_FinderDestroy,

    FSDisk_OpenNewSystem,
    FSDisk_UpdateContext,
    FSDisk_Open,
    FSDisk_DeleteFile,
    FSDisk_RenameFile,
    FSDisk_FileExists,
    FSDisk_Disperse,
    FSDisk_Close,

    FSDisk_GetS,
    FSDisk_Read,
    FSDisk_Write,
    FSDisk_Seek,
    FSDisk_EOF,
    FSDisk_Tell,
    FSDisk_Size,

    FSDisk_GetProperties,

    FSDisk_SetSize,
    FSDisk_SetAttributes,
    FSDisk_SetTime,
    FSDisk_SetHints,
};

const geVFile_SystemAPIs *GENESISCC
FSDisk_GetAPIs(void)
{
    return &FSDisk_APIs;
}
