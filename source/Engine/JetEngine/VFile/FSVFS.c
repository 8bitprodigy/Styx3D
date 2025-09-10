/****************************************************************************************/
/*  FSVFS.C                                                                             */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Collection file system implementation                                  */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.jet3d.com                                                                */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Jet3D, released December 12, 1999.                             */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>

#include	"Ram.h"

#include	"FSVFS.h"
#include	"DirTree.h"
#include	"Errorlog.h"

#ifdef KROUERDEBUG
#include "windows.h"
#endif

#ifdef BUILD_BE
// These should be put under a new OS/Storage.h header file...
#include <StorageDefs.h>
#define _MAX_PATH MAXPATHLEN
#define min(a,b) (((a)<(b))?(a):(b))
void FastSplitPath(register char* path, char* drive, char* dir, char* fname, char* ext); // Function in DirTree.c
#define _splitpath FastSplitPath
#endif

//	"VF00"
#define	VFSFILEHEADER_SIGNATURE	0x30304656

//	"VF01"
#define	VFSFILE_SIGNATURE		0x31304656

//	"VF02"
#define	VFSFINDER_SIGNATURE		0x32304656

#define	HEADER_VERSION	0

typedef	struct	VFSFileHeader
{
	unsigned int	Signature;
	unsigned short	Version;			// Version number
	jeBoolean		Dispersed;			// Is this VFS dispersed?
//	long			DirectoryOffset;	// File offset to directory
	long			DataLength;			// Length of all file data, including VFS header
	long			Stuff;
}	VFSFileHeader;

typedef	struct	VFSFile
{
	unsigned int	Signature;

	jeVFile *		RWOps;				// Parent file for read/write ops
	struct VFSFile *System;				// If we're a child, we need a back pointer

	DirTree *		DirEntry;			// Directory entry for this file
	DirTree *		Directory;			// Directory information for the VFS

	long			RWOpsStartPos;		// Starting position in the RWOps file
	long			CurrentRelPos;		// Current file pointer (relative to our start)
	long			Length;				// Current file size

	unsigned int	OpenModeFlags;

	jeVFile *		DispersedFile;		// If the file is opened from a dispersed VFS
										//   this is the file handle to the file data.

	// Things that are specific to the Root node
	jeBoolean		IsSystem;			// Am I the owner of the Directory?
	long			RWOpsHintsStartPos;
	long			DataLength;			// Current size of the aggregate including VFS header
	jeBoolean		Dispersed;			// Is this VFS dispersed?
//	jeBoolean		CanSetHints;		// Used to keep parallel semantics with the other
										//   file systems.

}	VFSFile;

typedef	struct	VFSFinder
{
	unsigned int		Signature;
	VFSFile *			File;
	DirTree_Finder *	Finder;
	DirTree *			LastFind;
}	VFSFinder;

#define	CHECK_HANDLE(H)	assert(H);assert(H->Signature == VFSFILE_SIGNATURE);
#define	CHECK_FINDER(F)	assert(F);assert(F->Signature == VFSFINDER_SIGNATURE);

#pragma warning (disable:4100)
static	void *	JETCC FSVFS_FinderCreate(
	jeVFile *		FS,
	void *			Handle,
	const char *	FileSpec)
{
	VFSFinder *		Finder;
	VFSFile *		File;

	assert(FileSpec != NULL);

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if	(!File->Directory)
		return NULL;

	Finder = (VFSFinder *)jeRam_Allocate(sizeof(*Finder));
	if	(!Finder)
		return NULL;

	memset(Finder, 0, sizeof(*Finder));

	Finder->Signature = VFSFINDER_SIGNATURE;
	Finder->File	  = File;
	Finder->Finder	  = DirTree_CreateFinder(File->Directory, FileSpec);
	if	(!Finder->Finder)
	{
		jeRam_Free(Finder);
		return NULL;
	}

	return (void *)Finder;
}
#pragma warning (default:4100)

static	jeBoolean	JETCC FSVFS_FinderGetNextFile(void *Handle)
{
	VFSFinder *	Finder;

	Finder = (VFSFinder *)Handle;

	CHECK_FINDER(Finder);

	Finder->LastFind = DirTree_FinderGetNextFile(Finder->Finder);
	if	(Finder->LastFind)
		return JE_TRUE;

	return JE_FALSE;
}

static	jeBoolean	JETCC FSVFS_FinderGetProperties(void *Handle, jeVFile_Properties *Properties)
{
	VFSFinder *		Finder;

	assert(Properties);

	Finder = (VFSFinder*)Handle;

	CHECK_FINDER(Finder);

	if	(!Finder->LastFind)
		return JE_FALSE;

	DirTree_GetFileTime(Finder->LastFind, &Properties->Time);
	DirTree_GetFileAttributes(Finder->LastFind, &Properties->AttributeFlags);
	DirTree_GetFileSize(Finder->LastFind, &Properties->Size);
//	DirTree_GetFileHints(Finder->LastFind, &Properties->Hints);
	return DirTree_GetName(Finder->LastFind, &Properties->Name[0], sizeof(Properties->Name));
}

static	void JETCC FSVFS_FinderDestroy(void *Handle)
{
	VFSFinder *	Finder;

	Finder = (VFSFinder *)Handle;

	CHECK_FINDER(Finder);

	assert(Finder->Finder);

	Finder->Signature = 0;
	DirTree_DestroyFinder(Finder->Finder);
	jeRam_Free(Finder);
}

#pragma warning (disable:4100)
static	void *	JETCC FSVFS_Open(
	jeVFile *		FS,
	void *			Handle,
	const char *	Name,
	void *			Dummy,
	unsigned int 	OpenModeFlags)
{
	VFSFile *	Context;
	VFSFile *	NewFile;
	DirTree *	FileEntry;

	Context = (VFSFile *)Handle;

	CHECK_HANDLE(Context);

	assert(Name);

	if	(!Context->Directory)
		return NULL;

	/*
		If the file is being requested raw, we will fail from a VFS, because
		we might be getting a VFS that has hints in the file, and making the read
		operations coherent on a RAW request will be difficult.
	*/
#pragma message("FSVFS does not support JE_VFILE_OPEN_RAW")
	if	(OpenModeFlags & JE_VFILE_OPEN_RAW)
		return NULL;

	/*
		Right now, we only support update operations to a VFS which is being
		created.  We can do create operations to a VFS which is being created,
		or which already exists.  We can also support directory open operations
		at anytime.
	*/
	if	((OpenModeFlags & JE_VFILE_OPEN_UPDATE)   &&
		 !(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY) &&
		 !(OpenModeFlags & JE_VFILE_OPEN_CREATE) &&
		 !(Context->System->OpenModeFlags & JE_VFILE_OPEN_CREATE))
		return NULL;

	FileEntry = DirTree_FindExact(Context->Directory, Name);
	if	(OpenModeFlags & JE_VFILE_OPEN_CREATE)
	{
		if	(FileEntry)
			return NULL;

		FileEntry = DirTree_AddFile(Context->Directory,
									Name,
									(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY) ? JE_TRUE : JE_FALSE);
		if	(!FileEntry)
			return NULL;
	}
	else
	{
		if	(!FileEntry)
			return NULL;
			
#pragma message ("Clean up this DirTree_OpenFile thing")
		if ( ! DirTree_OpenFile(FileEntry,OpenModeFlags) )	
			return NULL;
	}

	NewFile = (VFSFile *)jeRam_Allocate(sizeof(*NewFile));
	if	(!NewFile)
		return NewFile;

	memset(NewFile, 0, sizeof(*NewFile));

	NewFile->Signature 	  = VFSFILE_SIGNATURE;
	NewFile->DirEntry  	  = FileEntry;
	NewFile->RWOps	   	  = Context->RWOps;
	NewFile->Dispersed	  = JE_FALSE;
	NewFile->System		  = Context->System;
//	NewFile->CanSetHints  = JE_FALSE;

	// If we're a directory, make us a first class operator with the child
	if	(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY)
	{
		NewFile->Directory = FileEntry;
	}
	else
	{
//		if	(Context->System->OpenModeFlags & JE_VFILE_OPEN_CREATE)
		if	(OpenModeFlags & JE_VFILE_OPEN_CREATE)
		{
			NewFile->RWOpsStartPos = Context->System->DataLength +
									 Context->System->RWOpsStartPos;
		}
		else
		{
			// Here's where we open an existing file
			DirTree_GetFileOffset(FileEntry, &(NewFile->RWOpsStartPos));
			if	(NewFile->System->Dispersed == JE_TRUE)
			{
			jeVFile * DispersedDir;	// <> CB 2/10
			char	Buff[_MAX_PATH];
				// We have to get the file data from elsewhere
				if	(DirTree_GetFullName(FileEntry, Buff, sizeof(Buff)) == JE_FALSE)
				{
					jeErrorLog_AddString(-1,"DirTree GetFullName failed !",NULL);	
					jeRam_Free(NewFile);
					return NULL;
				}
				DispersedDir = jeVFile_GetContext(NewFile->RWOps);
				if ( ! DispersedDir )
				{
					jeErrorLog_AddString(-1,"No Context on Dispersed VFS !",NULL);	
					jeRam_Free(NewFile);
					return NULL;
				}
				NewFile->DispersedFile = jeVFile_Open(DispersedDir,
													  Buff,
													  JE_VFILE_OPEN_READONLY);
				if	(!NewFile->DispersedFile)
				{
					jeErrorLog_AddString(-1,"File note found in sispersed VFS !",Buff);	
					jeRam_Free(NewFile);
					return NULL;	// <> CB 2/10
				}
			}
		}
	}

	NewFile->OpenModeFlags = OpenModeFlags;

	if	(!(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY))
	{
		if	(OpenModeFlags & JE_VFILE_OPEN_CREATE)
		{
			DirTree_SetFileOffset(FileEntry, NewFile->RWOpsStartPos);
//			NewFile->CanSetHints = JE_TRUE;
		}
		else
		{
			assert(!(OpenModeFlags & JE_VFILE_OPEN_UPDATE));
			DirTree_GetFileSize(FileEntry, &NewFile->Length);
		}
	}

	// Only a VFS opened with OpenNewSystem gets to be the owner
	NewFile->IsSystem = JE_FALSE;

	return (void *)NewFile;
}
#pragma warning (default:4100)

static	void *	JETCC FSVFS_OpenNewSystem(
	jeVFile *		RWOps,
	const char *	Name,
	void *			Context,
	unsigned int 	OpenModeFlags)
{
VFSFile *	NewFS;
long		RWOpsStartPos;
VFSFileHeader	Header;
jeVFile *		HintsFile;

	assert(RWOps != NULL);
	assert(Name == NULL);
	assert(Context == NULL);

	// All VFS are directories
	if	(!(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY))
		return NULL;

	/*
		If the file is being requested raw, we will fail from a VFS, because
		we might be getting a VFS that has hints in the file, and making the read
		operations coherent on a RAW request will be difficult.
	*/
#pragma message("FSVFS does not support JE_VFILE_OPEN_RAW")
#ifdef KROUERDEBUG
	{
		char msg[80];
		sprintf(msg, "FSVFS_OpenNewSystem %s\n", Name);
		OutputDebugString(msg);
	}
#endif

	if	(OpenModeFlags & JE_VFILE_OPEN_RAW)
		return NULL;

	if	(jeVFile_Tell(RWOps, &RWOpsStartPos) == JE_FALSE)
	{
		jeErrorLog_AddString(-1,"FSVFS : 1",NULL);
		return NULL;
	}

	HintsFile = jeVFile_GetHintsFile(RWOps);
	if	(!HintsFile)
	{
		jeErrorLog_AddString(-1,"FSVFS : Get Hints failed!",NULL);
		return NULL;
	}

	NewFS = (VFSFile *)jeRam_AllocateClear(sizeof(*NewFS));
	if	(!NewFS)
		return NewFS;

	jeVFile_Tell(HintsFile,&(NewFS->RWOpsHintsStartPos));

	if	(!(OpenModeFlags & JE_VFILE_OPEN_CREATE))
	{

		if	(jeVFile_Read(RWOps, &Header, sizeof(Header)) == JE_FALSE)
		{
			jeErrorLog_AddString(-1,"FSVFS : Read Header failed!",NULL);
			return NULL;
		}

		if( 	(Header.Signature != VFSFILEHEADER_SIGNATURE) || 
				(Header.Version != HEADER_VERSION) )
		{
			jeVFile_Seek(RWOps, RWOpsStartPos, JE_VFILE_SEEKSET);
			jeErrorLog_AddString(-1,"FSVFS : Bad Signature",NULL);
			return NULL;
		}

		NewFS->RWOps = RWOps;
		NewFS->RWOpsStartPos = RWOpsStartPos;
		NewFS->DataLength = Header.DataLength;
		NewFS->Dispersed = Header.Dispersed;

		// Read the directory

		NewFS->Directory = DirTree_CreateFromFile(HintsFile);
		if	(!NewFS->Directory)
		{
			jeRam_Free(NewFS);
			jeVFile_Seek(RWOps, RWOpsStartPos, JE_VFILE_SEEKSET);
			jeErrorLog_AddString(-1,"FSVFS : DirTree_Create failed!",NULL);
			return NULL;
		}

		// Get the end position for paranoia checking
//DirTree_Dump(NewFS->Directory);
	}
	else
	{
		NewFS->RWOps = RWOps;
		NewFS->RWOpsStartPos = RWOpsStartPos;
		NewFS->Directory = DirTree_Create();
		NewFS->DataLength = sizeof(VFSFileHeader);

		memset(&Header,0,sizeof(Header));
		jeVFile_Write(RWOps,&Header,sizeof(Header));
	}

	NewFS->Signature	 = VFSFILE_SIGNATURE;
	NewFS->IsSystem		 = JE_TRUE;
	NewFS->System		 = NewFS;
	NewFS->OpenModeFlags = OpenModeFlags;

	return NewFS;
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSVFS_UpdateContext(
	jeVFile *		FS,
	void *			Handle,
	void *			Context,
	int 			ContextSize)
{
	return JE_FALSE;
}
#pragma warning (default:4100)

static	jeBoolean	JETCC FSVFS_Close(void *Handle)
{
	VFSFile *	File;
	jeBoolean	Result;

	File = (VFSFile *)Handle;
	
	CHECK_HANDLE(File);

#ifdef KROUERDEBUG
	{
		char msg[80];
		sprintf(msg, "FSVFS_Close\n");
		OutputDebugString(msg);
	}
#endif

	Result = JE_TRUE;
	if	(File->Directory)
	{
		if	(File->IsSystem)
		{
			// Hmmm.  We're the top level
			assert(File == File->System);

			if	(File->OpenModeFlags & (JE_VFILE_OPEN_CREATE | JE_VFILE_OPEN_UPDATE))
			{
				VFSFileHeader	Header;
				jeVFile *		HintsFile;

				Result = JE_FALSE;
				// Have to update the directory
				HintsFile = jeVFile_GetHintsFile(File->RWOps);
				if	(HintsFile)
				{

				/*
				int HintsPos;

					// @@
#pragma message ("Seeking in the directory hints file disallows embedded (not nested) VFS files")
					jeVFile_Tell(HintsFile,&HintsPos);
					if ( HintsPos != File->RWOpsHintsStartPos )
					{
						jeErrorLog_AddString(-1,"FSVFS_Close : Hints position changed between Open & Close!",NULL);
						jeRam_Free(File);
						return JE_FALSE;
					}

//					jeVFile_Seek(HintsFile, 0, JE_VFILE_SEEKSET);
					*/

					if	(DirTree_WriteToFile(File->Directory, HintsFile) == JE_TRUE)
					{
						Header.Signature = VFSFILEHEADER_SIGNATURE;
						Header.Version = HEADER_VERSION;
						Header.Dispersed = JE_FALSE;
						Header.DataLength = File->DataLength;
						Header.Stuff = 0;

						if	(jeVFile_Seek(File->RWOps, File->RWOpsStartPos, JE_VFILE_SEEKSET) == JE_TRUE)
						{
							if	(jeVFile_Write(File->RWOps, &Header, sizeof(Header)) == JE_TRUE)
							{
								Result = JE_TRUE;
							}
							else
							{
								jeErrorLog_AddString(-1,"FSVFS_Close : VFile_Write failed!",NULL);
							}
						}
					}
				}
			}

			DirTree_Destroy(File->Directory);

			// Have to make sure that we leave the file pointer at the end
			// of our data in the RWOps file that we come from.

			// @@ CB changed
			jeVFile_Seek(File->RWOps, File->RWOpsStartPos + File->DataLength, JE_VFILE_SEEKSET);
		}
	}
	else
	{	
		if	(File->OpenModeFlags & (JE_VFILE_OPEN_CREATE | JE_VFILE_OPEN_UPDATE)) //@@ CB
		{
			// Update the system with the length of this file.  Subsequent
			// file operations will follow this file.
			assert(File->System);
			File->System->DataLength += File->Length;
			DirTree_SetFileSize(File->DirEntry, File->Length);
		}
	}

	if	(File->DispersedFile)
		jeVFile_Close(File->DispersedFile);

	jeRam_Free(File);

	return Result;
}

static	uint32			ClampOperationSize(const VFSFile *File, int Size)
{
	assert(!File->Directory);
	assert(File->CurrentRelPos >= 0);
	return min(File->Length - File->CurrentRelPos, Size);
}

static	void		JETCC UpdateFilePos(VFSFile *File)
{
	long	RWOpsPos;

	assert(!File->Directory);
	assert(File->CurrentRelPos >= 0);

	if ( ! jeVFile_Tell(File->RWOps, &RWOpsPos) )
		assert(0);

	File->CurrentRelPos = RWOpsPos - File->RWOpsStartPos;
	if	(File->CurrentRelPos > File->Length)
		File->Length = File->CurrentRelPos;

	assert(File->CurrentRelPos >= 0);
}

static	jeBoolean	JETCC ForceFilePos(VFSFile *File)
{
#ifdef _DEBUG
long CurRelPos;

	assert(File);
	assert(File->RWOps);
	assert(!File->Directory);
	
	assert(File->CurrentRelPos >= 0);
	assert(File->CurrentRelPos <= File->Length);

	CurRelPos = File->CurrentRelPos;

	if ( ! jeVFile_Seek(File->RWOps,
					  File->RWOpsStartPos + CurRelPos,
					  JE_VFILE_SEEKSET) )
		return JE_FALSE;

	UpdateFilePos(File);

	assert(CurRelPos == File->CurrentRelPos);

	return JE_TRUE;
#else

	return jeVFile_Seek(File->RWOps,
					  File->RWOpsStartPos + File->CurrentRelPos,
					  JE_VFILE_SEEKSET);

#endif
}

static	jeBoolean	JETCC FSVFS_GetS(void *Handle, void *Buff, int MaxLen)
{
	VFSFile *	File;
	jeBoolean	Res;

	assert(Buff);
	assert(MaxLen != 0);

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->Directory)
		return JE_FALSE;

	assert(File->CurrentRelPos >= 0);
	assert(File->CurrentRelPos <= File->Length);

	if	(!ForceFilePos(File))
		return JE_FALSE;

	MaxLen = ClampOperationSize(File, MaxLen);

	Res = jeVFile_GetS(File->RWOps, Buff, MaxLen);

	UpdateFilePos(File);

	return Res;
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSVFS_BytesAvailable(void *Handle, long *Count)
{
	VFSFile *	File;
//	jeBoolean	Res;
#ifndef	NDEBUG
//	int			CurRelPos;
#endif

	assert(Count);

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->Directory)
		return JE_FALSE;

	assert(File->CurrentRelPos >= 0);
	assert(File->CurrentRelPos <= File->Length);

	if ( File->DispersedFile )
		return jeVFile_BytesAvailable(File->DispersedFile,Count);

	*Count = File->Length - File->CurrentRelPos;

	return JE_TRUE;
}
#pragma warning (default:4100)

jeVFile * Hack_VFS_File = NULL;

static	jeBoolean	JETCC FSVFS_Read(void *Handle, void *Buff, uint32 Count)
{
	VFSFile *	File;
	jeBoolean	Res;
	long		CurPos;
#ifndef	NDEBUG
	int			CurRelPos;
	uint32 A,B;
#endif

	assert(Buff);
	assert(Count != 0);

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->Directory)
		return JE_FALSE;

	if	(File->DispersedFile)
	{
	jeBoolean Ret;
	jeVFile * MyFile;
	extern jeVFile * Hack_File;
		Hack_VFS_File = MyFile = Hack_File;
		assert(jeVFile_IsValid(Hack_VFS_File));
		Ret = jeVFile_Read(File->DispersedFile, Buff, Count);
		assert( MyFile != Hack_File );
		assert(jeVFile_IsValid(MyFile));
		Hack_VFS_File = NULL;
		return Ret;
	}

	assert(File->CurrentRelPos >= 0);
	assert(File->CurrentRelPos <= File->Length);

	jeVFile_Tell(File->RWOps,&CurPos);

#ifdef _DEBUG
{
	long	RWOpsPos,AbsPos;

	AbsPos = File->RWOpsStartPos + File->CurrentRelPos;

	jeVFile_Seek(File->RWOps, AbsPos, JE_VFILE_SEEKSET);

	CurRelPos = File->CurrentRelPos;
	
	assert(!File->Directory);
	assert(File->CurrentRelPos >= 0);

	jeVFile_Tell(File->RWOps, &RWOpsPos);

	assert( AbsPos == RWOpsPos );

	File->CurrentRelPos = RWOpsPos - File->RWOpsStartPos;
	if	(File->CurrentRelPos > File->Length)
		File->Length = File->CurrentRelPos;

	assert(CurRelPos == File->CurrentRelPos);
}
#endif

	if	(!ForceFilePos(File))
		return JE_FALSE;

	if	(ClampOperationSize(File, Count) != Count)
		return JE_FALSE;

#ifndef	NDEBUG
	CurRelPos = File->CurrentRelPos;
#endif
#ifdef _DEBUG
	jeVFile_Tell(File->RWOps,&A);
#endif
	Res = jeVFile_Read(File->RWOps, Buff, Count);
#ifdef _DEBUG
	jeVFile_Tell(File->RWOps,&B);
#endif

	assert( B == (A + Count) );

	UpdateFilePos(File);
	assert(File->CurrentRelPos - CurRelPos == (int32)Count);

	jeVFile_Seek(File->RWOps,CurPos,JE_VFILE_SEEKSET);

	return Res;
}

static	jeBoolean	JETCC FSVFS_Write(void *Handle, const void *Buff, int Count)
{
	VFSFile *	File;
	jeBoolean	Res;
	long		CurPos;
#ifndef	NDEBUG
	int			CurRelPos;
#endif

	assert(Buff);
	assert(Count != 0);

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->Directory)
		return JE_FALSE;

	if	(File->OpenModeFlags & JE_VFILE_OPEN_READONLY)
		return JE_FALSE;

	assert(!File->DispersedFile);

	assert(File->CurrentRelPos >= 0);
	assert(File->CurrentRelPos <= File->Length);

	jeVFile_Tell(File->RWOps,&CurPos);

	if	(!ForceFilePos(File))
		return JE_FALSE;

#ifndef	NDEBUG
	CurRelPos = File->CurrentRelPos;
#endif
	Res = jeVFile_Write(File->RWOps, Buff, Count);

	UpdateFilePos(File);
	assert(File->CurrentRelPos - CurRelPos == Count);

	jeVFile_Seek(File->RWOps,CurPos,JE_VFILE_SEEKSET);

//	File->CanSetHints = JE_FALSE;

	return Res;
}

static	jeBoolean	JETCC FSVFS_Seek(void *Handle, int Where, jeVFile_Whence Whence)
{
	VFSFile *	File;
	jeBoolean	Res;
	long		AbsolutePos = 0;

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->Directory)
		return JE_FALSE;

	if	(File->DispersedFile)
		return jeVFile_Seek(File->DispersedFile, Where, Whence);

	assert(File->CurrentRelPos >= 0);
	assert(File->CurrentRelPos <= File->Length);

	switch	(Whence)
	{
	case	JE_VFILE_SEEKSET:
		AbsolutePos = File->RWOpsStartPos + Where;
		break;

	case	JE_VFILE_SEEKEND:
		AbsolutePos = File->RWOpsStartPos + File->Length - Where;
		break;

	case	JE_VFILE_SEEKCUR:
		AbsolutePos = File->RWOpsStartPos + File->CurrentRelPos + Where;
		break;

	default:
		assert(!"Illegal seek case");
	}

	if	(AbsolutePos < File->RWOpsStartPos)
		return JE_FALSE;

	Res = jeVFile_Seek(File->RWOps, AbsolutePos, JE_VFILE_SEEKSET);

	UpdateFilePos(File);

//	File->CanSetHints = JE_FALSE;

	return Res;
}

static	jeBoolean	JETCC FSVFS_EOF(const void *Handle)
{
	const VFSFile *	File;

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->Directory)
		return JE_FALSE;

	if	(File->DispersedFile)
		return jeVFile_EOF(File->DispersedFile);

	assert(File->CurrentRelPos >= 0);
	assert(File->CurrentRelPos <= File->Length);

	if	(File->CurrentRelPos == File->Length)
		return JE_TRUE;

	return JE_FALSE;
}

static	jeBoolean	JETCC FSVFS_Tell(const void *Handle, long *Position)
{
	const VFSFile *	File;

	File = (const VFSFile *) Handle;

	CHECK_HANDLE(File);

	if	(File->Directory)
		return JE_FALSE;

	if	(File->DispersedFile)
		return jeVFile_Tell(File->DispersedFile, Position);

	assert(File->CurrentRelPos >= 0);
	assert(File->CurrentRelPos <= File->Length);

	*Position = File->CurrentRelPos;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSVFS_Size(const void *Handle, long *Size)
{
	const VFSFile *	File;

	File = (VFSFile *) Handle;

	CHECK_HANDLE(File);

	if	(File->Directory != NULL)
		return JE_FALSE;

	assert(File->CurrentRelPos >= 0);
	assert(File->CurrentRelPos <= File->Length);

	if ( File->DispersedFile )
		return jeVFile_Size(File->DispersedFile, Size);
		
	*Size = File->Length;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSVFS_GetProperties(const void *Handle, jeVFile_Properties *Properties)
{
const VFSFile *	File;
jeVFile_Attributes AttribRemote;

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if ( ! File->DirEntry ) // <> CB 2/10
	{
		if ( jeVFile_GetProperties(File->RWOps,Properties) )
		{
			Properties->AttributeFlags |= JE_VFILE_ATTRIB_DIRECTORY;
			return JE_TRUE;
		}
		return JE_FALSE;
	}
	else
	{
		if ( ! jeVFile_GetProperties(File->RWOps,Properties) )
			return JE_FALSE;

		AttribRemote = Properties->AttributeFlags &	JE_VFILE_ATTRIB_REMOTE; 
	}

	DirTree_GetFileTime(File->DirEntry, &Properties->Time);
	DirTree_GetFileAttributes(File->DirEntry, &Properties->AttributeFlags);
	DirTree_GetFileSize(File->DirEntry, &Properties->Size);
//	DirTree_GetFileHints(File->DirEntry, &Properties->Hints);

	Properties->AttributeFlags |= AttribRemote;

	return DirTree_GetName(File->DirEntry, &Properties->Name[0], sizeof(Properties->Name));
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSVFS_SetSize(void *Handle, long Size)
{
	assert(!"Not implemented");
	return JE_FALSE;
}
#pragma warning (default:4100)

static	jeBoolean	JETCC FSVFS_SetAttributes(void *Handle, jeVFile_Attributes Attributes)
{
	const VFSFile *	File;

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	assert(File->DirEntry);

	if	(Attributes & ~JE_VFILE_ATTRIB_READONLY)
		return JE_FALSE;

	DirTree_SetFileAttributes(File->DirEntry, Attributes);

	return JE_TRUE;
}

static	jeBoolean	JETCC FSVFS_SetTime(void *Handle, const jeVFile_Time *Time)
{
	const VFSFile *	File;

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	assert(File->DirEntry);

	DirTree_SetFileTime(File->DirEntry, Time);

	return JE_FALSE;
}

#if 0

static	jeBoolean	JETCC FSVFS_HintsSize(void *Handle, long * Size)
{
	const VFSFile *	File;

	File = Handle;

	CHECK_HANDLE(File);

	assert(File->DirEntry);

	return DirTree_FileHintsSize(File->DirEntry, Size);
}


static	jeBoolean	JETCC FSVFS_ReadHints(void *Handle, void *Buff, int Count)
{
	VFSFile *	File;

	File = Handle;

	CHECK_HANDLE(File);

	assert(File->DirEntry);

//	if	(File->CanSetHints == JE_FALSE || DirTree_HasFileHints(File->DirEntry) == JE_TRUE)
//		return JE_FALSE;

//	File->CanSetHints = JE_FALSE;

//	return DirTree_SetFileHints(File->DirEntry, Hints);
	return DirTree_ReadFileHints(File->DirEntry, Buff, Count);
}

static	jeBoolean	JETCC FSVFS_WriteHints(void *Handle, const void *Buff, int Count)
{
	VFSFile *	File;

	File = Handle;

	CHECK_HANDLE(File);

	assert(File->DirEntry);

	return DirTree_WriteFileHints(File->DirEntry, Buff, Count);
}
#endif

static	jeVFile *	JETCC FSVFS_GetHintsFile(void *Handle)
{
	VFSFile *	File;

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	assert(File->DirEntry);

	if	(File->OpenModeFlags & JE_VFILE_OPEN_READONLY)
	{
		if	(DirTree_FileHasHints(File->DirEntry) == JE_FALSE)
			return NULL;

	}

	return DirTree_GetHintsFile(File->DirEntry);
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSVFS_FileExists(jeVFile *FS, void *Handle, const char *Name)
{
	VFSFile *	File;

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if	(!File->Directory)
		return JE_FALSE;

	return DirTree_FileExists(File->Directory, Name);
}
#pragma warning (default:4100)

static	void		GetDirectoryName(const char *Path, char *Buff)
{
#if 0	
//	char	Drive[_MAX_DRIVE];
	char	Directory[_MAX_DRIVE];

	_splitpath(Path, NULL, Directory, NULL, NULL);
	_makepath(Buff, NULL, Directory, NULL, NULL);
#else
	const char *	p;

	*Buff = '\0';

	p = Path + strlen(Path) - 1;
	if	(p > Path)
	{
		while	(*p && *p != '\\')
			p--;
		if	(p > Path)
		{
			memcpy(Buff, Path, p - Path + 1);
			Buff[p - Path + 1] = '\0';
		}
	}
#endif
}

static	jeBoolean	CopyVFile(jeVFile *Src, jeVFile *Dest)
{
	long	Length;
	char	Buff[4096];

	if	(jeVFile_Size(Src, &Length) == JE_FALSE)
		return JE_FALSE;

// <> CB 2/14	NO ! That's valid!  Small Bitmaps will be pure Hints
//	if	(Length == 0)
//		return JE_FALSE;

	while	(Length != 0)
	{
		long	Count;

		Count = min((long)sizeof(Buff), Length);
		if	(jeVFile_Read(Src, Buff, Count) == JE_FALSE)
			return JE_FALSE;
		if	(jeVFile_Write(Dest, Buff, Count) == JE_FALSE)
			return JE_FALSE;
		Length -= Count;
	}

	return JE_TRUE;
}

static	jeBoolean	CopyOneFile(jeVFile *FSSrc, jeVFile *FSDest, const char *src, const char *dest)
{
	jeVFile *	SrcFile;
	jeVFile *   DestFile;
	jeBoolean	Result;

	SrcFile = jeVFile_Open(FSSrc, src, JE_VFILE_OPEN_READONLY);
	if	(!SrcFile)
	{
		jeErrorLog_AddString(-1,"CopyOneFile : VFile_Open Src failed!",NULL);
		return JE_FALSE;
	}
	
	DestFile = jeVFile_Open(FSDest, dest, JE_VFILE_OPEN_CREATE);
	if	(!DestFile)
	{
		jeVFile_Close(SrcFile);
		jeErrorLog_AddString(-1,"CopyOneFile : VFile_Open Dest failed!",NULL);
		return JE_FALSE;
	}

	Result = CopyVFile(SrcFile, DestFile);

	jeVFile_Close(DestFile);
	jeVFile_Close(SrcFile);

	if ( ! Result )
		jeErrorLog_AddString(-1,"CopyOneFile : CopyVFile failed!",NULL);

	return Result;
}

static	jeBoolean	CopyDir(jeVFile *FSSrcDir, jeVFile *FSDestDir)
{
	jeVFile_Finder *	Finder;

	Finder = jeVFile_CreateFinder(FSSrcDir, "*.*");
	if	(!Finder)
	{
		jeErrorLog_AddString(-1,"CopyDir : CreateFinder failed!",NULL);
		return JE_FALSE;
	}

	while	(jeVFile_FinderGetNextFile(Finder) != JE_FALSE)
	{
		jeVFile_Properties	Properties;

		if	(jeVFile_FinderGetProperties(Finder, &Properties) == JE_FALSE)
		{
			jeErrorLog_AddString(-1,"CopyDir : FinderGetProps failed!",NULL);
			jeVFile_DestroyFinder(Finder);
			return JE_FALSE;
		}
		
		if	(Properties.AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY)
		{
			jeVFile *		SrcSubDir;
			jeVFile *		DestSubDir;
			jeBoolean	Result;

			SrcSubDir = jeVFile_Open(FSSrcDir,
									 Properties.Name,
									 JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY);
			if	(!SrcSubDir)
			{
				jeErrorLog_AddString(-1,"CopyDir : VF Open Dir failed!",NULL);
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			DestSubDir = jeVFile_Open(FSDestDir,
									  Properties.Name,
									  JE_VFILE_OPEN_CREATE | JE_VFILE_OPEN_DIRECTORY);
			if	(!DestSubDir)
			{
				jeErrorLog_AddString(-1,"CopyDir : VF Open Dir failed!",NULL);
				jeVFile_Close(SrcSubDir);
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			Result = CopyDir(SrcSubDir, DestSubDir);
			jeVFile_Close(DestSubDir);
			jeVFile_Close(SrcSubDir);
			if	(Result == JE_FALSE)
			{
				jeErrorLog_AddString(-1,"CopyDir : CopyDir failed!",NULL);
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}
		}
		else
		{
			if	(CopyOneFile(FSSrcDir, FSDestDir, Properties.Name, Properties.Name) == JE_FALSE)
			{
				jeErrorLog_AddString(-1,"CopyDir : CopyOneFile failed!",NULL);
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}
		}
	}

	jeVFile_DestroyFinder(Finder);
return JE_TRUE;
}

static	jeBoolean	ExplodeData(jeVFile *VFS, const char *FileSpec, jeVFile *DosFS)
{
#if 0
	jeVFile_Finder *	Finder;

	Finder = jeVFile_CreateFinder(VFS, "*.*");
	if	(!Finder)
		return JE_FALSE;

	while	(jeVFile_FinderGetNextFile(Finder))
	{
		jeVFile_Properties	Properties;
		jeBoolean	Result;

		Result = JE_FALSE;
		jeVFile_FinderGetProperties(Finder, &Properties);
		if	(Properties.AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY)
		{
			char		Buff[_MAX_PATH];
			jeVFile *	SubDir;

			Result = JE_FALSE;
			SubDir = jeVFile_Open(DosFS, Properties.Name, JE_VFILE_OPEN_CREATE | JE_VFILE_OPEN_DIRECTORY);
			if	(SubDir)
			{
				GetDirectoryName(FileSpec, Buff);
				strcat(Buff, Properties.Name);
				strcat(Buff, "\\*.*");
//				Result = ExplodeData(VFS, FileSpec, SubDir);
				jeVFile_Close(SubDir);
			}
		}
		else
		{
			jeVFile *	DosFile;
			jeBoolean	Result;

			Result = JE_FALSE;
			DosFile = jeVFile_Open(DosFS, Properties.Name, JE_VFILE_OPEN_CREATE);
			if	(DosFile)
			{
				Result = CopyVFile
			}
		}

		if	(Result == JE_FALSE)
		{
			jeVFile_DestroyFinder(Finder);
			return JE_FALSE;
		}
	}

	jeVFile_DestroyFinder(Finder);
	return JE_TRUE;
#endif
}

static	jeBoolean	JETCC FSVFS_Disperse(
	jeVFile *		FS,
	void *			Handle,
	const char *	Directory)
{
	VFSFile *			File;
	VFSFileHeader		Header;
	jeVFile *			DosFS;
	jeVFile_Properties	Properties;
	jeVFile *			DispersedFile;
	jeVFile *			HintsFile;
	char				VFSName[_MAX_PATH];
	jeBoolean			Result;

	File = (VFSFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->IsSystem == JE_FALSE)
		return JE_FALSE;

	assert(File == File->System);

	DosFS = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_DOS, Directory, NULL, JE_VFILE_OPEN_CREATE | JE_VFILE_OPEN_DIRECTORY);
	if	(!DosFS)
		return JE_FALSE;

	jeVFile_GetProperties(File->RWOps, &Properties);
	_splitpath(Properties.Name, NULL, NULL, VFSName, NULL);
	strcat(VFSName, ".Dispersed");
	
	DispersedFile = jeVFile_Open(DosFS, VFSName, JE_VFILE_OPEN_CREATE);
	if	(!DispersedFile)
	{
		jeVFile_Close(DosFS);
		return JE_FALSE;
	}

	HintsFile = jeVFile_GetHintsFile(DispersedFile);
	if	(!HintsFile)
	{
		jeVFile_Close(DispersedFile);
		jeVFile_Close(DosFS);
		return JE_FALSE;
	}

	Result = JE_FALSE;
	Header.Signature = VFSFILEHEADER_SIGNATURE;
	Header.Version = HEADER_VERSION;
	Header.Dispersed = JE_TRUE;
	Header.DataLength = File->DataLength;
	Header.Stuff = 0;
	if	(jeVFile_Write(DispersedFile, &Header, sizeof(Header)) == JE_TRUE)
	{
//		if	(DirTree_WriteToFile(File->Directory, DispersedFile) == JE_TRUE)
		if	(DirTree_WriteToFile(File->Directory, HintsFile) == JE_TRUE)
		{
			Result = CopyDir(FS, DosFS);
		}
	}
	jeVFile_Close(DispersedFile);
	jeVFile_Close(DosFS);

#pragma message("FSVFS : Dispersed bitmaps could be 0 bytes; do we handle this well?")

	return Result;
}
#pragma warning (default:4100)

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSVFS_DeleteFile(jeVFile *FS, void *Handle, const char *Name)
{
assert(!"Not implemented");
	return JE_FALSE;
}
#pragma warning (default:4100)

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSVFS_RenameFile(jeVFile *FS, void *Handle, const char *Name, const char *NewName)
{
assert(!"Not implemented");
	return JE_FALSE;
}
#pragma warning (default:4100)

static	jeVFile_SystemAPIs	FSVFS_APIs =
{
	FSVFS_FinderCreate,
	FSVFS_FinderGetNextFile,
	FSVFS_FinderGetProperties,
	FSVFS_FinderDestroy,

	FSVFS_OpenNewSystem,
	FSVFS_UpdateContext,
	FSVFS_Open,
	FSVFS_DeleteFile,
	FSVFS_RenameFile,
	FSVFS_FileExists,
	FSVFS_Disperse,
	FSVFS_Close,

	FSVFS_GetS,
	FSVFS_BytesAvailable,
	FSVFS_Read,
	FSVFS_Write,
	FSVFS_Seek,
	FSVFS_EOF,
	FSVFS_Tell,
	FSVFS_Size,

	FSVFS_GetProperties,

	FSVFS_SetSize,
	FSVFS_SetAttributes,
	FSVFS_SetTime,

#if 0
	FSVFS_ReadHints,
	FSVFS_WriteHints,
	FSVFS_HintsSize,
#endif
	FSVFS_GetHintsFile,
};

const jeVFile_SystemAPIs * JETCC FSVFS_GetAPIs(void)
{
	return &FSVFS_APIs;
}
