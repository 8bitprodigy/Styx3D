/****************************************************************************************/
/*  FSDOS.C                                                                             */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: DOS file system implementation                                         */
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*Genesis3D Version 1.1 released November 15, 1999                            */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#ifdef _WIN32
	#define	WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#define  PATH_MAX _MAX_PATH
#else
	#include <limits.h>
	#include <stdint.h>
    #include <unistd.h>   // close(), read(), write()
#endif

#include "FSDOS.h"
#include "GETypes.h"
#include "RAM.h"
#include "VFile.h"
#include "VFile_private.h"
#include "XPlatUtils.h"


#define DBG_OUT( Text, ... ) _DBG_OUT("FSDOS.c:" Text, ##__VA_ARGS__ )


//	"DF01"
#define	DOSFILE_SIGNATURE	0x31304644

//	"DF02"
#define	DOSFINDER_SIGNATURE	0x32304644

#define	CHECK_GE_FILE_HANDLE(H)	assert(H);assert(H->Signature == DOSFILE_SIGNATURE);
#define	CHECK_FINDER(F)	assert(F);assert(F->Signature == DOSFINDER_SIGNATURE);

typedef struct	
DosFile
{
	      uint16          Signature;
	      void           *FileHandle;
	      char           *FullPath;
	const char           *Name;
	      bool            IsDirectory;
}	
DosFile;

typedef struct	
DosFinder
{
	uint16        Signature;
	void         *FindHandle;
	GE_FIND_DATA  FindData;
	bool          FirstStillCached;
	int           OffsetToName;
}	
DosFinder;

static	bool	
BuildFileName(
	const DosFile  *File,
	const char     *Name,
	char           *Buff,
	char          **NamePtr,
	int             MaxLen
)
{
	int		
		DirLength,
		NameLength;

	if ( !Name )
		return false;

	if (File) {
		if (File->IsDirectory == false) return false;

		assert(File->FullPath);
		DirLength = strlen(File->FullPath);

		if (DirLength > MaxLen) return false;

		memcpy(Buff, File->FullPath, DirLength);
	} else {
		DirLength = 0;
	}

	NameLength = strlen(Name);
	if (DirLength + NameLength + 2 > MaxLen || ! Buff ) return false;

	if (DirLength != 0) {
		Buff[DirLength] = PATH_SEPARATOR;
		memcpy(Buff + DirLength + 1, Name, NameLength + 1);
		if (NamePtr)
			*NamePtr = Buff + DirLength + 1;
	} else {
		memcpy(Buff, Name, NameLength + 1);
		if	(NamePtr)
			*NamePtr = Buff;

		// Special case: no directory, no file name.  We meant something like ".\"
		if (!*Buff) {
			strcpy (Buff, strcat(".",(char*)PATH_SEPARATOR));
		}
	}

	return true;
}

static void *GENESISCC 
FSDos_FinderCreate(
	      geVFile *FS,
	      void    *Handle,
	const char    *FileSpec
)
{
	DosFinder *		Finder;
	DosFile *		File;
	char *			NamePtr;
	char			Buff[PATH_MAX];

	assert(FileSpec != NULL);

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	Finder = geRam_Allocate(sizeof(*Finder));
	if	(!Finder)
		return NULL;

	memset(Finder, 0, sizeof(*Finder));

	if	(BuildFileName(File, FileSpec, Buff, &NamePtr, sizeof(Buff)) == false)
	{
		geRam_Free(Finder);
		return NULL;
	}

	Finder->OffsetToName = NamePtr - Buff;

	Finder->FindHandle = geFindFirstFile(Buff, &Finder->FindData);

	Finder->FirstStillCached = true;

	Finder->Signature = DOSFINDER_SIGNATURE;
	return (void *)Finder;
}

static bool GENESISCC 
FSDos_FinderGetNextFile(void *Handle)
{
	DosFinder *Finder;

	Finder = Handle;

	CHECK_FINDER(Finder);

	if	(Finder->FindHandle == GE_INVALID_HANDLE_VALUE)
		return false;

	if	(Finder->FirstStillCached == true)
	{
		Finder->FirstStillCached = false;

		if	(Finder->FindData.fileName[0] != '.')
			return true;
	}
	
	while	(geFindNextFile(Finder->FindHandle, &Finder->FindData) == true)
	{
		if	(Finder->FindData.fileName[0] != '.')
			return true;
	}

	return false;
}

static bool GENESISCC 
FSDos_FinderGetProperties(void *Handle, geVFile_Properties *Props)
{
	DosFinder *			Finder;
	geVFile_Attributes	Attribs;
	int					Length;

	assert(Props);

	Finder = Handle;

	CHECK_FINDER(Finder);

	if	(Finder->FindHandle == GE_INVALID_HANDLE_VALUE)
		return false;

	Attribs = 0;
	if	(Finder->FindData.fileAttributes & GE_FILE_ATTRIBUTE_DIRECTORY)
		Attribs |= GE_VFILE_ATTRIB_DIRECTORY;
	if	(Finder->FindData.fileAttributes & GE_FILE_ATTRIBUTE_READONLY)
		Attribs |= GE_VFILE_ATTRIB_READONLY;

	Props->Time.Time1 = Finder->FindData.ftLastWriteTime[GE_LOW];
	Props->Time.Time2 = Finder->FindData.ftLastWriteTime[GE_HIGH];

	Props->AttributeFlags = Attribs;
	Props->Size = Finder->FindData.fileSizeLow;
	Props->Hints.HintData = NULL;
	Props->Hints.HintDataLength = 0;

	Length = strlen(Finder->FindData.fileName);
	if	(Length > sizeof(Props->Name) - 1)
		return false;
	memcpy(Props->Name, Finder->FindData.fileName, Length + 1);

	return true;
}

static void GENESISCC 
FSDos_FinderDestroy(void *Handle)
{
	DosFinder *	Finder;

	Finder = Handle;

	CHECK_FINDER(Finder);

	if	(Finder->FindHandle != GE_INVALID_HANDLE_VALUE)
		geFindClose(Finder->FindHandle);

	Finder->Signature = 0;
	geRam_Free(Finder);
}

// Terrible function.  It mutated, and now it modifies its argument.
static bool
IsRootDirectory(char *Path)
{
	int SlashCount = 0;

#ifdef _WIN32
	// Drive letter test
	if	(Path[1] == ':' && Path[2] == '\\' && Path[3] == '\0')
	{
		Path[2] = '\0';
		return true;
	}

	// Now UNC path test
	if	(Path[0] == PATH_SEPARATOR && Path[1] == PATH_SEPARATOR)
	{
		Path += 2;
#endif
		while	(*Path)
		{
			if	(*Path++ == PATH_SEPARATOR)
				SlashCount++;
		}
#ifdef _WIN32
	}
#endif

	if (SlashCount == 1) return true;

	return false;
}

static void * GENESISCC 
FSDos_Open(
	      geVFile *FS,
	      void    *Handle,
	const char    *Name,
	      void    *Context,
	      unsigned int   OpenModeFlags
)
{
	DosFile *DosFS;
	DosFile *NewFile;
	char     Buff[PATH_MAX];
	int      Length;
	char    *NamePtr;

	DosFS = (DosFile*)Handle;
	
/*
	if (!(DosFS && DosFS->IsDirectory)) {
		DBG_OUT("FSDos_Open()\tFailed due to DosFS is not a directory...");
		return NULL;
	}
*/

	NewFile = geRam_Allocate(sizeof(*NewFile));
	if (!NewFile) { 
		DBG_OUT("FSDos_Open()\tFailed due to no NewFile...");
		return NewFile;
	}

	memset(NewFile, 0, sizeof(*NewFile));

	if (!BuildFileName(DosFS, Name, Buff, &NamePtr, sizeof(Buff))) {
		DBG_OUT("FSDos_Open()\tFailing due to failue to build filename...");
		goto fail;
	}

	Length = strlen(Buff);
	NewFile->FullPath = geRam_Allocate(Length + 1);
	if (!NewFile->FullPath) { 
		DBG_OUT("FSDos_Open()\tFailing due to NewFile->FullPath empty...");
		goto fail;
	}

	NewFile->Name = NewFile->FullPath + (NamePtr - &Buff[0]);

	memcpy(NewFile->FullPath, Buff, Length + 1);

	if (OpenModeFlags & GE_VFILE_OPEN_DIRECTORY) {
		GE_FIND_DATA *FileInfo;
		GE_FIND_FILE *FindHandle;
		bool          IsDirectory;

		assert(!DosFS || DosFS->IsDirectory == true);

		memset(&FileInfo, 0, sizeof(FileInfo));
		FindHandle = (GE_FIND_FILE*)geFindFirstFile(NewFile->FullPath, FileInfo);
		if (FindHandle != GE_INVALID_HANDLE_VALUE 
			&& FileInfo->fileAttributes & GE_FILE_ATTRIBUTE_DIRECTORY
		) {
			IsDirectory = true;
			DBG_OUT("FSDos_Open()\tIsDirectory is true");
		} else {
			IsDirectory = IsRootDirectory(NewFile->FullPath);
			DBG_OUT("FSDos_Open()\tIsDirectory was assigned IsRootDirectory");
		}
		DBG_OUT("FSDos_Open()\tIsDirectory:\t%b", IsDirectory);

		geFindClose(FindHandle);

		if (OpenModeFlags & GE_VFILE_OPEN_CREATE) {
			if (IsDirectory == true) {
				DBG_OUT("FSDos_Open()\tFailing due to IsDirectory == false...");
				goto fail;
			}
			if (!geCreateDirectory(NewFile->FullPath, NULL)) {
				DBG_OUT("FSDos_Open()\tFailing due to not being able to create a directory for NewFile->FullPath...");
				goto fail;
			}
		} else if (!IsDirectory) {
			DBG_OUT("FSDos_Open()\tFailing due to !IsDirectory...");
			goto fail;
		}

		NewFile->IsDirectory = true;
		NewFile->FileHandle = GE_INVALID_HANDLE_VALUE;
		
	} else {
		uint32			ShareMode;
		uint32			CreationMode;
		uint32			Access;
		uint32			LastError;

		CreationMode = GE_OPEN_EXISTING;

		switch (
			OpenModeFlags 
			& (
				GE_VFILE_OPEN_READONLY 
				| GE_VFILE_OPEN_UPDATE
				| GE_VFILE_OPEN_CREATE
			)
		)
		{
		case	GE_VFILE_OPEN_READONLY:
			Access    = GE_GENERIC_READ;
			ShareMode = GE_FILE_SHARE_READ | GE_FILE_SHARE_WRITE;
			break;

		case GE_VFILE_OPEN_CREATE:
			CreationMode = GE_CREATE_ALWAYS; /* FALLTHROUGH */
		case	GE_VFILE_OPEN_UPDATE:
			Access       = GE_GENERIC_READ | GE_GENERIC_WRITE;
			ShareMode    = GE_FILE_SHARE_READ;
			break;

		default:
			assert(!"Illegal open mode flags");
			break;
		}

		NewFile->FileHandle = geCreateFile(
			NewFile->FullPath,
			Access,
			ShareMode,
			NULL,
			CreationMode,
			0,
			NULL
		);
		if (NewFile->FileHandle == GE_INVALID_HANDLE_VALUE) {
			LastError = geGetLastError();
			DBG_OUT("FSDos_Open()\tFailing due to NewFile->FileHandle being invalid...");
			goto fail;
		}
	}

	NewFile->Signature = DOSFILE_SIGNATURE;
	DBG_OUT("FSDos_Open()\Succeeded");
	return (void *)NewFile;

fail:
	if	(NewFile->FullPath)
		geRam_Free(NewFile->FullPath);
	geRam_Free(NewFile);
	DBG_OUT("FSDos_Open()\tFailed :'(");
	return NULL;
}

static	void *	GENESISCC 
FSDos_OpenNewSystem(
	geVFile *			FS,
	const char *	Name,
	void *			Context,
	unsigned int 	OpenModeFlags
)
{
	return FSDos_Open(FS, NULL, Name, Context, OpenModeFlags);
}

static	bool	GENESISCC 
FSDos_UpdateContext(
	geVFile *		FS,
	void *			Handle,
	void *			Context,
	int 			ContextSize)
{
	return false;
}

static	void	GENESISCC 
FSDos_Close(void *Handle)
{
	DosFile *	File;
	
	File = Handle;
	
	CHECK_GE_FILE_HANDLE(File);

	if	(File->IsDirectory == false)
	{
		assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);

		geCloseHandle(File->FileHandle);
	}
	
	assert(File->FullPath);
	File->Signature = 0;
	geRam_Free(File->FullPath);
	geRam_Free(File);
}

static	bool	GENESISCC 
FSDos_GetS(void *Handle, void *Buff, int MaxLen)
{
	DosFile   *File;
	uint32     BytesRead;
	bool       Result;
	char      *p;
	char      *End;

	assert(Buff);
	assert(MaxLen != 0);

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);

	if	(File->IsDirectory == true)
		return false;

	Result = geReadFile(File->FileHandle, Buff, MaxLen - 1, &BytesRead, NULL);
	if	(BytesRead == 0)
	{
#if 0
		if	(Result == false)
			return false;
		
		// The Win32 API is vague about this, so we're being weird with the asserts
		assert(Result != true);
#endif
		return false;
	}

	End = (char *)Buff + BytesRead;
	p = Buff;
	while	(p < End)
	{
		/*
		  This code will terminate a line on one of three conditions:
			\r	Character changed to \n, next char set to 0
			\n	Next char set to 0
			\r\n	First \r changed to \n.  \n changed to 0.
		*/
		if	(*p == '\r')
		{
			int Skip = 0;
			
			*p = '\n';		// set end of line
			p++;			// and skip to next char
			// If the next char is a newline, then skip it too  (\r\n case)
			if (*p == '\n')
			{
				Skip = 1;
			}
			*p = '\0';
			// Set the file pointer back a bit since we probably overran
			geSetFilePointer(File->FileHandle, -(int)(BytesRead - ((p + Skip) - (char *)Buff)), NULL, GE_FILE_CURRENT); 
			assert(p - (char *)Buff <= MaxLen);
			return true;
		}
		else if	(*p == '\n')
		{
			// Set the file pointer back a bit since we probably overran
			p++;
			geSetFilePointer(File->FileHandle, -(int)(BytesRead - (p - (char *)Buff)), NULL, GE_FILE_CURRENT); 
			*p = '\0';
			assert(p - (char *)Buff <= MaxLen);
			return true;
		}
		p++;
	}

	return false;
}


static bool GENESISCC 
FSDos_Read(void *Handle, void *Buff, int Count)
{
	DosFile *File;
	uint32   BytesRead;

	assert(Buff);
	assert(Count != 0);

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	if	(File->IsDirectory == true)
		return false;

#ifdef	ELIDEBUG
{
	FILE  *fp;
	int32  Position;

	Position = geSetFilePointer(File->FileHandle, 0, NULL, GE_FILE_CURRENT);
	fp = fopen("c:\\vfs.eli", "ab+");
	fprintf(fp, "FSDos_Read: %-8d bytes @ %-8d\r\n", Count, Position);
	fclose(fp);
}
#endif

	if	(geReadFile(File->FileHandle, Buff, Count, &BytesRead, NULL) == false)
		return false;

	if	(BytesRead == 0)
		return false;

	return true;
}

static bool GENESISCC 
FSDos_Write(void *Handle, const void *Buff, int Count)
{
	DosFile *File;
	int32    BytesWritten;

	assert(Buff);
	assert(Count != 0);

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	if	(File->IsDirectory == true)
		return false;

#ifdef	ELIDEBUG
{
	FILE  *fp;
	int32  Position;

	Position = geSetFilePointer(File->FileHandle, 0, NULL, GE_FILE_CURRENT);
	fp = fopen("c:\\vfs.eli", "ab+");
	fprintf(fp, "FSDos_Write: %-8d bytes @ %-8d\r\n", Count, Position);
	fclose(fp);
}
#endif

	if (geWriteFile(File->FileHandle, Buff, Count, &BytesWritten, NULL) == false) return false;

	if ((int)BytesWritten != Count) return false;

	return true;
}

static	bool	GENESISCC 
FSDos_Seek(void *Handle, int Where, geVFile_Whence Whence)
{
	int      RTLWhence;
	DosFile *File;

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	if (File->IsDirectory == true)
		return false;

	switch (Whence)
	{
	case	GE_VFILE_SEEKCUR:
		RTLWhence = GE_FILE_CURRENT;
		break;

	case	GE_VFILE_SEEKEND:
		RTLWhence = GE_FILE_END;
		break;

	case	GE_VFILE_SEEKSET:
		RTLWhence = GE_FILE_BEGIN;
		break;
	default:
		assert(!"Unknown seek kind");
	}

	if (geSetFilePointer(File->FileHandle, Where, NULL, RTLWhence) == 0xffffffff) return false;

	return true;
}

static bool GENESISCC 
FSDos_EOF(const void *Handle)
{
	const DosFile *File;
	      uint32   CurPos;

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	if	(File->IsDirectory == true)
		return false;

	assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);

	CurPos = geSetFilePointer(File->FileHandle, 0, NULL, GE_FILE_CURRENT);
	assert(CurPos != 0xffffffff);

	if	(CurPos == geGetFileSize(File->FileHandle, NULL))
		return true;

	return false;
}

static bool GENESISCC 
FSDos_Tell(const void *Handle, int32 *Position)
{
	const DosFile *File;

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	if (File->IsDirectory == true) return false;

	assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);

	*Position = geSetFilePointer(File->FileHandle, 0, NULL, GE_FILE_CURRENT);
	if (*Position == -1L) return false;

	return true;
}

static	bool	GENESISCC 
FSDos_Size(const void *Handle, int32 *Size)
{
	const DosFile *File;

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	if (File->IsDirectory == true) return false;

	assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);

	*Size = geGetFileSize(File->FileHandle, NULL);
	if (*Size != (long)0xffffffff) return true;

	return false;
}

static	bool	GENESISCC 
FSDos_GetProperties(const void *Handle, geVFile_Properties *Properties)
{
	const DosFile                       *File;
	      geVFile_Attributes		     Attribs;
	      GE_BY_HANDLE_FILE_INFORMATION  Info;
	      int						    Length;

	assert(Properties);

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	if	(File->IsDirectory == true)
	{
		memset(Properties, 0, sizeof(*Properties));
		Properties->AttributeFlags = GE_FILE_ATTRIBUTE_DIRECTORY;
#pragma message ("FSDos_GetProperties: Time support is not there for directories")
	}
	else
	{
		assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);
	
		if (geGetFileInformationByHandle(File->FileHandle, &Info) == false) return false;
	
		Attribs = 0;
		if (Info.dwFileAttributes & GE_FILE_ATTRIBUTE_DIRECTORY) Attribs |= GE_VFILE_ATTRIB_DIRECTORY;
		if (Info.dwFileAttributes & GE_FILE_ATTRIBUTE_READONLY)  Attribs |= GE_VFILE_ATTRIB_READONLY;
	
		Properties->Time.Time1 = Info.ftLastWriteTime[GE_LOW];
		Properties->Time.Time2 = Info.ftLastWriteTime[GE_HIGH];
	
		Properties->AttributeFlags 		 = Attribs;
		Properties->Size		  		 = Info.nFileSizeLow;
		Properties->Hints.HintData		 = NULL;
		Properties->Hints.HintDataLength = 0;
	}

	Length = strlen(File->Name) + 1;
	if (Length > sizeof(Properties->Name)) return false;
	memcpy(Properties->Name, File->Name, Length);

	return true;
}

static	bool	GENESISCC 
FSDos_SetSize(void *Handle, int32 size)
{
	DosFile *File;

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	if (File->IsDirectory == false) {
		assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);
	
		if (geSetFilePointer(File->FileHandle, 0, NULL, GE_FILE_END) == 0xffffffff) return false;
		if (geSetEndOfFile(File->FileHandle) == false)                              return false;
	}

	return false;
}

static	bool	GENESISCC 
FSDos_SetAttributes(void *Handle, geVFile_Attributes Attributes)
{
	DosFile *File;
	uint32   Win32Attributes;

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);

	if (File->IsDirectory == true) return false;

	if (Attributes & GE_VFILE_ATTRIB_READONLY) Win32Attributes = GE_FILE_ATTRIBUTE_READONLY;
	else                                       Win32Attributes = GE_FILE_ATTRIBUTE_NORMAL;

	if (geSetFileAttributes(File->FullPath, Win32Attributes) == false) return false;

	return true;
}

static	bool	GENESISCC 
FSDos_SetTime(void *Handle, const geVFile_Time *Time)
{
	DosFile  *File;

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);

	if (geSetFileTime(File->FileHandle, &Time->Time1, &Time->Time1, &Time->Time1) == false) return false;

	return true;
}

static	bool	GENESISCC 
FSDos_SetHints(void *Handle, const geVFile_Hints *Hints)
{
	DosFile *	File;

	File = Handle;

	CHECK_GE_FILE_HANDLE(File);

	assert(File->FileHandle != GE_INVALID_HANDLE_VALUE);

	return false;
}

static	bool	GENESISCC 
FSDos_FileExists(geVFile *FS, void *Handle, const char *Name)
{
	DosFile *	File;
	char		Buff[PATH_MAX];

	File = Handle;

	if (File && File->IsDirectory                           == false) return false;
	if (BuildFileName(File, Name, Buff, NULL, sizeof(Buff)) == false) return false;
	if (geGetFileAttributes(Buff)                           == -1)    return false;

	return true;
}

static	bool	GENESISCC 
FSDos_Disperse(
	geVFile *		FS,
	void *		Handle,
	const char *Directory,
	bool	Recursive)
{
	return false;
}

static	bool	GENESISCC 
FSDos_DeleteFile(geVFile *FS, void *Handle, const char *Name)
{
	DosFile *	File;
	char		Buff[PATH_MAX];

	File = Handle;

	if	(File && File->IsDirectory == false)
		return false;

	if	(BuildFileName(File, Name, Buff, NULL, sizeof(Buff)) == false)
		return false;

	if	(geDeleteFile(Buff) == false)
		return false;

	return true;
}

static	bool	GENESISCC 
FSDos_RenameFile(geVFile *FS, void *Handle, const char *Name, const char *NewName)
{
	DosFile *	File;
	char		Old[PATH_MAX];
	char		New[PATH_MAX];

	File = Handle;

	if	(File && File->IsDirectory == false)
		return false;

	if	(BuildFileName(File, Name, Old, NULL, sizeof(Old)) == false)
		return false;

	if	(BuildFileName(File, NewName, New, NULL, sizeof(New)) == false)
		return false;

	if	(geMoveFile(Old, New) == false)
		return false;

	return true;
}

static	geVFile_SystemAPIs	
FSDos_APIs =
{
	FSDos_FinderCreate,
	FSDos_FinderGetNextFile,
	FSDos_FinderGetProperties,
	FSDos_FinderDestroy,

	FSDos_OpenNewSystem,
	FSDos_UpdateContext,
	FSDos_Open,
	FSDos_DeleteFile,
	FSDos_RenameFile,
	FSDos_FileExists,
	FSDos_Disperse,
	FSDos_Close,

	FSDos_GetS,
	FSDos_Read,
	FSDos_Write,
	FSDos_Seek,
	FSDos_EOF,
	FSDos_Tell,
	FSDos_Size,

	FSDos_GetProperties,

	FSDos_SetSize,
	FSDos_SetAttributes,
	FSDos_SetTime,
	FSDos_SetHints,
};

const geVFile_SystemAPIs *GENESISCC 
FSDos_GetAPIs(void)
{
	return &FSDos_APIs;
}

