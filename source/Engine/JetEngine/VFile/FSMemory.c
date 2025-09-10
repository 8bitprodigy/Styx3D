/****************************************************************************************/
/*  FSMEMORY.C                                                                          */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Memory file system implementation                                      */
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
/******

Jan/Feb 99 : cbloom : hints-related bug-fix

*******/

#ifdef WIN32
#include	<windows.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>

#include	"BaseType.h"
#include	"Ram.h"

#include	"VFile.h"
#include	"VFile._h"

#include	"FSMemory.h"

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

//	"MF01"
#define	MEMORYFILE_SIGNATURE	0x3130464D

//	"MF02"
#define	MEMORYFINDER_SIGNATURE	0x3230464D

#define	CHECK_HANDLE(H)	assert(H);assert(H->Signature == MEMORYFILE_SIGNATURE);
#define	CHECK_FINDER(F)	assert(F);assert(F->Signature == MEMORYFINDER_SIGNATURE);

#define	MEMORY_FILE_GROW	0x2000

typedef struct	MemoryFile
{
	unsigned int	Signature;
	char *			Memory;
	int				Size;
	int				AllocatedSize;
	int				Position;
	int				PositionAdjust;				// Adjustment for old hints data
//	jeVFile_Hints	Hints;
	jeBoolean		WeOwnMemory;
	jeBoolean		ReadOnly;
	jeVFile *		HintsFile;
	unsigned int	OpenModeFlags;
}	MemoryFile;

static char * JETCC DataPtr(const MemoryFile *File)
{
	return File->Memory + File->Position + File->PositionAdjust;
}

static	jeBoolean	JETCC TestForExpansion(MemoryFile *File, int Size)
{
	assert(File);
	assert(File->ReadOnly == JE_FALSE);
	assert(File->WeOwnMemory == JE_TRUE);

	assert(File->AllocatedSize >= File->Size);
	assert(File->AllocatedSize >= File->Position);

	if	(File->AllocatedSize - File->Position < Size)
	{
		int		NewSize;
		char *	NewBlock;

		NewSize = ((File->AllocatedSize + Size + (MEMORY_FILE_GROW - 1)) / MEMORY_FILE_GROW) * MEMORY_FILE_GROW;
		NewBlock = (char *)jeRam_Realloc(File->Memory, NewSize);
		if	(!NewBlock)
			return JE_FALSE;
		File->Memory = NewBlock;
		File->AllocatedSize = NewSize;
//printf("FSMemory: Expanded file to %d bytes\n", NewSize);
	}

	return JE_TRUE;
}

#pragma warning (disable:4100)
static	void *	JETCC FSMemory_FinderCreate(
	jeVFile *		FS,
	void *			Handle,
	const char *	FileSpec)
{
	return NULL;
}
#pragma warning (default:4100)

static	jeBoolean	JETCC FSMemory_FinderGetNextFile(void *Handle)
{
	assert(!Handle);
	return JE_FALSE;
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSMemory_FinderGetProperties(void *Handle, jeVFile_Properties *Props)
{
	assert(!Handle);
	return JE_FALSE;
}
#pragma warning (default:4100)

static	void JETCC FSMemory_FinderDestroy(void *Handle)
{
	assert(!Handle);
}

#pragma warning (disable:4100)
static	void *	JETCC FSMemory_Open(
	jeVFile *		FS,
	void *			Handle,
	const char *	Name,
	void *			Context,
	unsigned int 	OpenModeFlags)
{
	return NULL;
}
#pragma warning (default:4100)

static	void *	JETCC FSMemory_OpenNewSystem(
	jeVFile *		FS,
	const char *	Name,
	void *			Context,
	unsigned int 	OpenModeFlags)
{
	MemoryFile *			NewFS;
	jeVFile_MemoryContext *	MemContext;

	if	(FS || Name || !Context)
		return NULL;

	MemContext = (jeVFile_MemoryContext *)Context;

	// Don't allow the user to pass in memory pointer if we're updating or creating, because
	// we don't know what allocation functions we should use to resize their block if
	// necessary.  If you want to create a new file, you have to pass in NULL and let
	// us manage the allocations.
	if	(MemContext->Data && (OpenModeFlags & (JE_VFILE_OPEN_UPDATE | JE_VFILE_OPEN_CREATE)))
		return NULL;

	if	(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY)
		return NULL;

	NewFS = (MemoryFile *)jeRam_Allocate(sizeof(*NewFS));
	if	(!NewFS)
		return NewFS;
	memset(NewFS, 0, sizeof(*NewFS));

	NewFS->Memory = (char *)MemContext->Data;
	NewFS->Size = MemContext->DataLength;
	NewFS->AllocatedSize = NewFS->Size;
	NewFS->OpenModeFlags = OpenModeFlags;

	if	(NewFS->Memory)
	{
		jeVFile_HintsFileHeader *	HintsHeader;

		NewFS->ReadOnly = JE_TRUE;
		NewFS->WeOwnMemory = JE_FALSE;

		HintsHeader = (jeVFile_HintsFileHeader *)NewFS->Memory;
		if	(HintsHeader->Signature == JE_VFILE_HINTSFILEHEADER_SIGNATURE &&
			 !(OpenModeFlags & JE_VFILE_OPEN_RAW))
		{
#if 1
			jeVFile_MemoryContext	MemoryContext;
			if	((uint32)HintsHeader->HintDataLength + sizeof(*HintsHeader) > (uint32)NewFS->Size)
			{
				//  Oops.  Something is wrong with this file
				jeRam_Free(NewFS);
				return NULL;
			}
			MemoryContext.Data = (void *)(HintsHeader + 1);
			MemoryContext.DataLength = HintsHeader->HintDataLength;
			NewFS->HintsFile = jeVFile_OpenNewSystem(NULL,
													 JE_VFILE_TYPE_MEMORY,
													 NULL, 
													 &MemoryContext,
													 JE_VFILE_OPEN_READONLY);
			NewFS->PositionAdjust = HintsHeader->HintDataLength + sizeof(*HintsHeader);

			NewFS->Size -= NewFS->PositionAdjust; 
#else
			NewFS->Hints.HintData = (void *)(HintsHeader + 1);
			NewFS->Hints.HintDataLength = HintsHeader->HintDataLength;
#endif
		}
	}
	else
	{
		NewFS->ReadOnly = JE_FALSE;
		NewFS->WeOwnMemory = JE_TRUE;
	}

	NewFS->Signature = MEMORYFILE_SIGNATURE;

	return NewFS;
}

static	jeBoolean	JETCC FSMemory_UpdateContext(
	jeVFile *		FS,
	void *			Handle,
	void *			Context,
	int 			ContextSize)
{
MemoryFile *			File;
jeVFile_MemoryContext *	MemoryContext;
int HintsLength;

	assert(FS);
	assert(Context);
	
	File = (MemoryFile *)Handle;
	
	CHECK_HANDLE(File);

	if	(ContextSize != sizeof(jeVFile_MemoryContext))
		return JE_FALSE;

	HintsLength = 0;	
	if	(File->HintsFile)
	{
		jeVFile_MemoryContext	HintsContext;
		jeVFile_HintsFileHeader	HintsHeader;

		jeVFile_UpdateContext(File->HintsFile, &HintsContext, sizeof(HintsContext));
		if	(TestForExpansion(File, HintsContext.DataLength) == JE_FALSE)
			return JE_FALSE;

		HintsLength = sizeof(HintsHeader) + HintsContext.DataLength;
		memmove(File->Memory + HintsLength, File->Memory + File->PositionAdjust, File->Size); 
		HintsHeader.Signature = JE_VFILE_HINTSFILEHEADER_SIGNATURE;
		HintsHeader.HintDataLength = HintsContext.DataLength;
		memcpy(File->Memory, &HintsHeader, sizeof(HintsHeader));
		memcpy(File->Memory + sizeof(HintsHeader), HintsContext.Data, HintsContext.DataLength);
		File->PositionAdjust = HintsLength;
	}

	MemoryContext = (jeVFile_MemoryContext *)Context;
	
	MemoryContext->Data		  = File->Memory;
	MemoryContext->DataLength = File->Size + HintsLength;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSMemory_Close(void *Handle)
{
	MemoryFile *	File;
	
	File = (MemoryFile *)Handle;
	
	CHECK_HANDLE(File);

	if	(File->WeOwnMemory == JE_TRUE && File->Memory)
		jeRam_Free(File->Memory);

	if	(File->HintsFile)
		jeVFile_Close(File->HintsFile);

	jeRam_Free(File);

	return JE_TRUE;
}

static uint32 JETCC ClampOperationSize(const MemoryFile *File, int Size)
{
	return min(File->Size - File->Position, Size);
}

static	jeBoolean	JETCC FSMemory_GetS(void *Handle, void *Buff, int MaxLen)
{
	MemoryFile *	File;
	char *			p;
	char *			Start;
	char *			pBuff;

	assert(Buff);
	assert(MaxLen != 0);

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);

	MaxLen = ClampOperationSize(File, MaxLen);
	if	(MaxLen == 0)
		return JE_FALSE;

	p = DataPtr(File);
	pBuff = (char *)Buff;
	Start = p;
	while	(*p != '\n' && MaxLen > 0)
	{
		*pBuff++ = *p++;
		MaxLen--;
	}

	File->Position += p - Start + 1;
	assert(File->Position <= File->Size);
	assert(File->Size <= File->AllocatedSize);

	if	(MaxLen != 0)
	{
		*pBuff = *p;
		return JE_TRUE;
	}

	return JE_FALSE;
}

static	jeBoolean	JETCC FSMemory_BytesAvailable(void *Handle, long *Count)
{
	MemoryFile *	File;

	assert(Count);

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);

	*Count = File->Size - File->Position;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSMemory_Read(void *Handle, void *Buff, uint32 Count)
{
	MemoryFile *	File;

	assert(Buff);
	assert(Count != 0);

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);

	if	(ClampOperationSize(File, Count) != Count)
		return JE_FALSE;

	memcpy(Buff, DataPtr(File), Count);

	File->Position += Count;
	assert(File->Position <= File->Size);
	assert(File->Size <= File->AllocatedSize);

	return JE_TRUE;
}

static	jeBoolean	JETCC FSMemory_Write(void *Handle, const void *Buff, int Count)
{
	MemoryFile *	File;

	assert(Buff);
	assert(Count != 0);

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->ReadOnly == JE_TRUE)
		return JE_FALSE;

	if	(TestForExpansion(File, Count) == JE_FALSE)
		return JE_FALSE;

	memcpy(DataPtr(File), Buff, Count);
	
	File->Position += Count;
	if	(File->Size < File->Position)
		File->Size = File->Position;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSMemory_Seek(void *Handle, int Where, jeVFile_Whence Whence)
{
	MemoryFile *	File;

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);

	switch	(Whence)
	{
	int		NewPos;

	case	JE_VFILE_SEEKCUR:
		NewPos = File->Position + Where;
		if	(NewPos > File->AllocatedSize)
		{
			if	(File->ReadOnly == JE_TRUE)
				return JE_FALSE;
			if	(TestForExpansion(File, Where) == JE_FALSE)
				return JE_FALSE;
		}
		File->Position = NewPos;
		break;

	case	JE_VFILE_SEEKEND:
		if	(File->Size < Where)
			return JE_FALSE;
		File->Position = File->Size - Where;
		break;

	case	JE_VFILE_SEEKSET:
		if	(Where > File->AllocatedSize)
		{
			if	(File->ReadOnly == JE_TRUE)
				return JE_FALSE;
			if	(TestForExpansion(File, Where - File->Position) == JE_FALSE)
				return JE_FALSE;
		}
		File->Position = Where;
		break;

	default:
		assert(!"Unknown seek kind");
	}

	if	(File->Position > File->Size)
		File->Size = File->Position;
	
	assert(File->Size <= File->AllocatedSize);
	assert(File->Position <= File->AllocatedSize);

	return JE_TRUE;
}

static	jeBoolean	JETCC FSMemory_EOF(const void *Handle)
{
	const MemoryFile *	File;

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->Position == File->Size)
		return JE_TRUE;

	return JE_FALSE;
}

static	jeBoolean	JETCC FSMemory_Tell(const void *Handle, long *Position)
{
	const MemoryFile *	File;

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);

	*Position = File->Position;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSMemory_Size(const void *Handle, long *Size)
{
	const MemoryFile *	File;

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);
	
	*Size = File->Size;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSMemory_GetProperties(const void *Handle, jeVFile_Properties *Properties)
{
	const MemoryFile *	File;

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);
	
	assert(Properties);

	Properties->Time.Time1 = Properties->Time.Time2 = 0;

	Properties->Size = File->Size;
	Properties->Name[0] = 0;
		
	Properties->AttributeFlags = 0;
	if ( File->ReadOnly )
		Properties->AttributeFlags |= JE_VFILE_ATTRIB_READONLY;

return JE_TRUE;
}

static	jeBoolean	JETCC FSMemory_SetSize(void *Handle, long Size)
{
	MemoryFile *	File;

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);

	if	(Size < File->Size)
		return JE_FALSE;

	if	(!TestForExpansion(File, (Size - File->Position)))
		return JE_FALSE;

	File->Size = Size;

	return JE_TRUE;
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSMemory_SetAttributes(void *Handle, jeVFile_Attributes Attributes)
{
	assert(!"Not implemented");
	return JE_FALSE;
}
#pragma warning (default:4100)

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSMemory_SetTime(void *Handle, const jeVFile_Time *Time)
{
	assert(!"Not implemented");
	return JE_FALSE;
}
#pragma warning (default:4100)

static	jeVFile *	JETCC FSMemory_GetHintsFile(void *Handle)
{
	MemoryFile *	File;

	File = (MemoryFile *)Handle;

	CHECK_HANDLE(File);

	if	(!File->HintsFile && !(File->OpenModeFlags & JE_VFILE_OPEN_RAW))
	{
		jeVFile_MemoryContext	MemoryContext;

		// Can't create a hints file for a readonly memory file
		if	(File->ReadOnly )
			return NULL;

		MemoryContext.Data = NULL;
		MemoryContext.DataLength = 0;

		File->HintsFile = jeVFile_OpenNewSystem(NULL,
												JE_VFILE_TYPE_MEMORY,
												NULL,
												&MemoryContext,
												JE_VFILE_OPEN_CREATE);

	}

	return File->HintsFile;
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSMemory_FileExists(jeVFile *FS, void *Handle, const char *Name)
{
	return JE_FALSE;
}
#pragma warning (default:4100)

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSMemory_Disperse(
	jeVFile *	FS,
	void *		Handle,
	const char *Directory)
{
	return JE_FALSE;
}
#pragma warning (default:4100)

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSMemory_DeleteFile(jeVFile *FS, void *Handle, const char *Name)
{
	return JE_FALSE;
}
#pragma warning (default:4100)

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSMemory_RenameFile(jeVFile *FS, void *Handle, const char *Name, const char *NewName)
{
	return JE_FALSE;
}
#pragma warning (default:4100)

static	jeVFile_SystemAPIs	FSMemory_APIs =
{
	FSMemory_FinderCreate,
	FSMemory_FinderGetNextFile,
	FSMemory_FinderGetProperties,
	FSMemory_FinderDestroy,

	FSMemory_OpenNewSystem,
	FSMemory_UpdateContext,
	FSMemory_Open,
	FSMemory_DeleteFile,
	FSMemory_RenameFile,
	FSMemory_FileExists,
	FSMemory_Disperse,
	FSMemory_Close,

	FSMemory_GetS,
	FSMemory_BytesAvailable,
	FSMemory_Read,
	FSMemory_Write,
	FSMemory_Seek,
	FSMemory_EOF,
	FSMemory_Tell,
	FSMemory_Size,

	FSMemory_GetProperties,

	FSMemory_SetSize,
	FSMemory_SetAttributes,
	FSMemory_SetTime,

#if 0
	FSMemory_ReadHints,
	FSMemory_WriteHints,
	FSMemory_HintsSize,
#endif
	FSMemory_GetHintsFile,
};

const jeVFile_SystemAPIs * JETCC FSMemory_GetAPIs(void)
{
	return &FSMemory_APIs;
}
