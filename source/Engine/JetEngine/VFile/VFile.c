/****************************************************************************************/
/*  VFILE.C                                                                             */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Virtual file implementation                                            */
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

#define LN_SEARCHLIST	// works? this lets us avoid a malloc
						// what exactly is the point of the searchlist?
//#define NO_INET

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include	<windows.h>

#endif

#include	<stdio.h>
#include	<assert.h>
#include	<stdarg.h>
#include	<string.h>

#include	"BaseType.h"
#include	"Ram.h"
#include	"ThreadQueue.h"
#include	"Log.h"

#include	"VFile.h"
#include	"VFile._h"

#ifdef WIN32
#include	"fsdos.h"
#endif

#ifdef BUILD_BE
#include <OS.h>
#include 	"FSBeos.h"
#endif

#include	"FSMemory.h"
#include	"FSVFS.h"
#include	"FSLZ.h"
#include	"fsfakenet.h"

#ifndef NO_INET
#include	"fsinet.h"
#endif

#ifdef LN_SEARCHLIST
#include	"List.h"
#endif

#ifndef LN_SEARCHLIST
typedef	struct	FSSearchList
{
	jeVFile *				FS;
	struct FSSearchList *	Next;
}	FSSearchList;
#endif

typedef	struct	jeVFile
{
#ifdef LN_SEARCHLIST
	LinkNode					LN;
	LinkNode					LN_Children;
#else
	FSSearchList *				SearchList;
#endif

	jeVFile_TypeIdentifier		SystemType;
	const jeVFile_SystemAPIs *	APIs;
	void *						FSData;
	jeVFile *					Context;
	jeThreadQueue_Semaphore *	Semaphore;	
	int							RefCount;
	jeBoolean					IsHintsFile;
}	jeVFile;

typedef struct	jeVFile_Finder
{
	const jeVFile_SystemAPIs *	APIs;
	void *						Data;
#ifdef WIN32
        CRITICAL_SECTION                        CriticalSection;
#endif // WIN32
#ifdef BUILD_BE
        sem_id                                  CriticalSection;
#endif // BUILD_BE  
}	jeVFile_Finder;

/*}{ ******* Statics *******/

#ifdef _DEBUG
static uint32 VFiles_Open = 0;
#endif

static uint32 						jeVFile_RefCount = 0;

static	jeVFile_SystemAPIs  **RegisteredAPIs = NULL;
static	int							SystemCount;

#define jeVFile_Lock(File)		jeThreadQueue_Semaphore_Lock(	((jeVFile *)(File))->Semaphore)
#define jeVFile_UnLock(File)	jeThreadQueue_Semaphore_UnLock(	((jeVFile *)(File))->Semaphore)

static jeVFile* JETCC	jeVFile_New(void);
static void		JETCC	jeVFile_Free(jeVFile * File);

static jeBoolean jeVFile_PathIsSane(const char * Path);
static jeBoolean JETCC CheckOpenFlags(unsigned int OpenModeFlags);

#ifdef WIN32
#define LOCK_CRITICALSECTION(a) EnterCriticalSection(a);
#define UNLOCK_CRITICALSECTION(a) LeaveCriticalSection(a);
#define DELETE_CRITICALSECTION(a) DeleteCriticalSection(a);
#endif

#ifdef BUILD_BE
#define LOCK_CRITICALSECTION(a) acquire_sem(*a); // passes a pointer, so convert
#define UNLOCK_CRITICALSECTION(a) release_sem(*a);
#define DELETE_CRITICALSECTION(a) delete_sem(*a);
#endif

/*}{ ******* File System Functions *******/

static	jeBoolean JETCC jeVFile_RegisterFileSystemInternal(const jeVFile_SystemAPIs *APIs, jeVFile_TypeIdentifier *Type)
{
	jeVFile_SystemAPIs **	NewList;

	NewList = (jeVFile_SystemAPIs **)jeRam_Realloc((void *)RegisteredAPIs, sizeof(*RegisteredAPIs) * (SystemCount + 1));
	if(!NewList)
	 return JE_FALSE;

	RegisteredAPIs = NewList;
	RegisteredAPIs[SystemCount++] = (jeVFile_SystemAPIs *)APIs;
	*Type = (jeVFile_TypeIdentifier)SystemCount;

	return JE_TRUE;
}

static	jeBoolean jeVFile_Enter(void)
{
	jeVFile_TypeIdentifier 	Type;

	if ( jeVFile_RefCount > 0 )
	{
		jeVFile_RefCount ++;
		return JE_TRUE;
	}

#ifdef WIN32
	if	(jeVFile_RegisterFileSystemInternal(FSDos_GetAPIs(), &Type) == JE_FALSE)
		return JE_FALSE;
	if	(Type != JE_VFILE_TYPE_DOS)
		return JE_FALSE;
#endif

#ifdef BUILD_BE
	if	(jeVFile_RegisterFileSystemInternal(FSBeOS_GetAPIs(), &Type) == JE_FALSE)
		return JE_FALSE;
	if	(Type != JE_VFILE_TYPE_DOS)
		return JE_FALSE;
#endif

	if	(jeVFile_RegisterFileSystemInternal(FSMemory_GetAPIs(), &Type) == JE_FALSE)
		return JE_FALSE;
	if	(Type != JE_VFILE_TYPE_MEMORY)
		return JE_FALSE;

	if	(jeVFile_RegisterFileSystemInternal(FSVFS_GetAPIs(), &Type) == JE_FALSE)
		return JE_FALSE;
	if	(Type != JE_VFILE_TYPE_VIRTUAL)
		return JE_FALSE;

	if	(jeVFile_RegisterFileSystemInternal(FSLZ_GetAPIs(), &Type) == JE_FALSE)
		return JE_FALSE;
	if	(Type != JE_VFILE_TYPE_LZ)
		return JE_FALSE;

	if	(jeVFile_RegisterFileSystemInternal(FSFakeNet_GetAPIs(), &Type) == JE_FALSE)
		return JE_FALSE;
	if	(Type != JE_VFILE_TYPE_FAKENET)
		return JE_FALSE;

	// INET must be last

#ifndef NO_INET
	if	(jeVFile_RegisterFileSystemInternal(FSINet_GetAPIs(), &Type) == JE_FALSE)
		return JE_FALSE;
	if	(Type != JE_VFILE_TYPE_INTERNET)
		return JE_FALSE;
#endif

	jeVFile_RefCount ++;

	return JE_TRUE;
}

static void jeVFile_Leave(void)
{
	jeVFile_RefCount --;
	if ( jeVFile_RefCount == 0 )
	{
		assert(RegisteredAPIs);
		#ifdef WIN32 // hack....
		jeRam_Free((void *)RegisteredAPIs);
		#endif
		#ifdef BUILD_BE
		jeRam_Free(RegisteredAPIs);
		#endif
		
		RegisteredAPIs = NULL;
	}
}

/*
//CB : this function is neither exposed or used, so ignore it
JETAPI jeBoolean JETCC jeVFile_RegisterFileSystem(const jeVFile_SystemAPIs *APIs, jeVFile_TypeIdentifier *Type)
{
	jeBoolean	Result;

	assert(APIs);
	assert(Type);

// @@ is this important ?
//	if	(RegisterBuiltInAPIs() == JE_FALSE)
//		return JE_FALSE;

	Result = jeVFile_RegisterFileSystemInternal(APIs, Type);
	return Result;
}
*/

/*}{ ******* Open Functions *******/

JETAPI jeVFile * JETCC jeVFile_OpenNewSystem(
	jeVFile *				FS,
	jeVFile_TypeIdentifier 	FileSystemType,
	const char *			Name,
	void *					Context,
	unsigned int 			OpenModeFlags)
{
	const jeVFile_SystemAPIs *	APIs;
	jeVFile *					File;
	void *						FSData;

	assert( ! Name || jeVFile_PathIsSane(Name) );

	if ( ! jeVFile_Enter() )
		return NULL;

	if	((FileSystemType == 0) || (FileSystemType > SystemCount))
		goto fail;

	if	(CheckOpenFlags(OpenModeFlags) == JE_FALSE)
		goto fail;

	// Sugarcoating support for a taste test
	if	(FS == NULL && FileSystemType == JE_VFILE_TYPE_VIRTUAL)
	{
		assert(Name);
		FS = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS, Name, NULL,
									 OpenModeFlags & ~JE_VFILE_OPEN_DIRECTORY);
		if	(! FS)
			goto fail;
		Name = NULL;
	}
	else if	(FS)
	{
		jeVFile_CreateRef(FS);
	}

	if	(FS)
		jeVFile_Lock(FS);

	APIs = RegisteredAPIs[FileSystemType - 1];
	assert(APIs);
	FSData = APIs->OpenNewSystem(FS, Name, Context, OpenModeFlags);
	if	(FS)
		jeVFile_UnLock(FS);

	if	(!FSData)
	{
		if	(FS)
			jeVFile_Close(FS);
		goto fail;
	}

	File = jeVFile_New();
	if	(!File)
	{
		if	(FS)
			jeVFile_Close(FS);
		APIs->Close(FSData);
		goto fail;
	}

	File->SystemType =	FileSystemType;
	File->APIs = 		APIs;
	File->FSData = 		FSData;
	
#ifndef LN_SEARCHLIST
	File->SearchList = 	jeRam_Allocate(sizeof(*File->SearchList));
#endif

	File->RefCount = 	0;
	File->Context =		FS;

#ifdef LN_SEARCHLIST
	LN_InitList(File);
	File->LN_Children.Next = (LinkNode *)File;
	File->LN_Children.Prev = (LinkNode *)File;
#endif

	File->Semaphore = jeThreadQueue_Semaphore_Create();
	if ( ! File->Semaphore )
	{
		jeVFile_Close(File);
		goto fail;	
	}

	assert( File->Context != File );

#ifndef LN_SEARCHLIST

	if	(!File->SearchList)
	{
		jeVFile_Close(File);
		goto fail;
	}

	File->SearchList->FS 	= File;
	File->SearchList->Next	= NULL;
#endif

#ifdef _DEBUG
	VFiles_Open ++;
#endif

	assert( jeVFile_IsValid(File) );

	return File;

fail:

	jeVFile_Leave();

	return NULL;
}

JETAPI jeVFile * JETCC jeVFile_Open(
	jeVFile *		FS,
	const char *	Name,
	unsigned int 	OpenModeFlags)
{
	jeVFile *		StartContext;
	jeVFile *		File;
	void *			FSData;
#ifndef LN_SEARCHLIST
	FSSearchList *	SearchList;
#endif

	assert( jeVFile_IsValid(FS) );
	assert( jeVFile_PathIsSane(Name) );

	if	(!FS)
		return NULL;

	if	(CheckOpenFlags(OpenModeFlags) == JE_FALSE)
		return NULL;

	StartContext = FS;

#ifdef KROUERDEBUG
	{
		char msg[80];
		sprintf(msg, "jeVFile::Open %s\n", Name);
		OutputDebugString(msg);
	}
#endif

	// you can't lock FS up here, cuz FS is
	//	about to change!

#ifdef LN_SEARCHLIST
	if	(!(OpenModeFlags & JE_VFILE_OPEN_CREATE))
	{
	LinkNode * Head;	
		Head = &(StartContext->LN_Children);
		for( FS = (jeVFile *)(Head->Next); FS != StartContext ; FS = (jeVFile *)(FS->LN.Next) ) 
		{
			if	(FS->APIs->FileExists(FS, FS->FSData, Name))
				break;
		}

		if ( ! FS )
			return NULL;
	}
#else
	SearchList = FS->SearchList;
	assert(SearchList);
	assert(SearchList->FS == FS);
	if	(!(OpenModeFlags & JE_VFILE_OPEN_CREATE))
	{
		while	(SearchList)
		{
			FS = SearchList->FS;
			if	(FS->APIs->FileExists(FS, FS->FSData, Name))
				break;
			SearchList = SearchList->Next;
		}
	}

	if	(!SearchList)
		return NULL;
#endif

#if 1 //@@ CB debug : 
	{
		if ( FS != StartContext )
			Log_Printf("Chose FS != StartContext\n");
		assert(jeVFile_IsValid(FS));
		assert(jeVFile_IsValid(StartContext));
	}
#endif

	jeVFile_Lock(FS);

	FSData = FS->APIs->Open(FS, FS->FSData, Name, NULL, OpenModeFlags);
	if	(!FSData)
	{
		jeVFile_UnLock(FS);
		return NULL;
	}

	File = jeVFile_New();
	if	(!File)
	{
		FS->APIs->Close(FSData);
		jeVFile_UnLock(FS);
		return NULL;
	}

	File->SystemType =	JE_VFILE_TYPE_INVALID;
//	File->SystemType  = FS->SystemType;
	File->APIs = 		FS->APIs;
	File->FSData = 		FSData;	
#ifndef LN_SEARCHLIST
	File->SearchList = 	jeRam_Allocate(sizeof(*File->SearchList));
#endif
	File->Context =		FS;
	File->RefCount = 	0;

	jeVFile_CreateRef(FS); 

	jeVFile_UnLock(FS);

#ifdef _DEBUG
	VFiles_Open ++;
#endif
		
#ifdef LN_SEARCHLIST
	LN_InitList(File);
	File->LN_Children.Next = (LinkNode *)File;
	File->LN_Children.Prev = (LinkNode *)File;
	LN_AddTail(&(StartContext->LN_Children),File);
#endif

	File->Semaphore = jeThreadQueue_Semaphore_Create();
	if ( ! File->Semaphore )
	{
		jeVFile_Close(File);
		return NULL;		
	}

#ifndef LN_SEARCHLIST
	if	(!File->SearchList)
	{
		jeVFile_Close(File);
		return NULL;
	}

	File->SearchList->FS 	= File;
	File->SearchList->Next	= StartContext->SearchList;
#endif

#ifdef KROUERDEBUG
	{
		char msg[80];
		sprintf(msg, "jeVFile::Open %s - open is ok %p\n", Name, File);
		OutputDebugString(msg);
	}
#endif

	assert( jeVFile_IsValid(File) );

	return File;
}

JETAPI jeBoolean JETCC jeVFile_Close(jeVFile *File)
{
jeBoolean	Result = JE_TRUE;
jeVFile * Context;
int RefCount;

	assert( jeVFile_IsValid(File) );

#ifdef _DEBUG
	VFiles_Open --;
#endif

	RefCount = File->RefCount;
	Context = File->Context;

#ifdef KROUERDEBUG
	{
		char msg[80];
		strcpy(msg, "jeVFile::Close      ");
		jeVFile_GetName(File, msg+16, 60);
		strcat(msg, "\n");
		OutputDebugString(msg);
	}
#endif

	if ( ! RefCount )
	{

		if ( File->Semaphore )
			jeVFile_Lock(File);

#ifdef LN_SEARCHLIST
		LN_Cut(File);
		LN_Cut(&(File->LN_Children));
#endif

		if ( File->FSData )
			if ( ! File->APIs->Close(File->FSData) )
				Result = JE_FALSE;
			
		if ( File->Semaphore )
		{
			jeVFile_UnLock(File);
			jeThreadQueue_Semaphore_Destroy(&(File->Semaphore));
		}

#ifndef LN_SEARCHLIST
		// <> never cuts self from parents' list !?
		if ( File->SearchList )
			jeRam_Free(File->SearchList);
#endif

		jeVFile_Free(File);
		File = NULL;
	}
	else
	{
		File->RefCount --;
	}

	if	( Context)
	{
		assert(Context->RefCount >= RefCount);
		if ( ! jeVFile_Close(Context) )
			Result = JE_FALSE;
	}

return Result;
}

JETAPI jeBoolean JETCC jeVFile_Destroy(jeVFile **pFile)
{
jeVFile * File;
	assert(pFile);
	File = *pFile;
	if ( ! File )
		return JE_TRUE;
	*pFile = NULL;
return jeVFile_Close(File);
}

#ifdef _DEBUG
JETAPI uint32 JETCC jeVFile_OpenCount(void)
{
return VFiles_Open;
}
#endif

JETAPI void		 JETCC jeVFile_CreateRef(jeVFile *File)
{
	assert( jeVFile_IsValid(File) );

	if	( File->Context)
	{
		jeVFile_CreateRef(File->Context);
	}

#ifdef _DEBUG
	VFiles_Open ++;
#endif

	File->RefCount++;
}

JETAPI jeBoolean JETCC jeVFile_UpdateContext(jeVFile *FS, void *Context, int ContextSize)
{
	jeBoolean	Result;

	assert(Context);

	assert( jeVFile_IsValid(FS) );

	jeVFile_Lock(FS);
	Result = FS->APIs->UpdateContext(FS, FS->FSData, Context, ContextSize);
	jeVFile_UnLock(FS);
	return Result;
}

JETAPI jeVFile * JETCC jeVFile_GetContext(const jeVFile *File)
{
	assert( jeVFile_IsValid(File) );

	return File->Context;
}

#ifndef LN_SEARCHLIST //{
static	void			DestroySearchList(FSSearchList *SearchList)
{
	while	(SearchList)
	{
		FSSearchList *	Temp;

		Temp = SearchList;
		SearchList = SearchList->Next;
		jeRam_Free(Temp);
	}
}

static	FSSearchList *	CopySearchList(const FSSearchList *SearchList)
{
	FSSearchList *	NewList;
	FSSearchList *	Tail;

	NewList = Tail = NULL;
	while	(SearchList)
	{
		FSSearchList *	Temp;

		Temp = jeRam_Allocate(sizeof(*Tail));
		if	(!Temp)
		{
			DestroySearchList(NewList);
			return NULL;
		}
		if	(Tail)
			Tail->Next = Temp;
		else
		{
			assert(!NewList);
			NewList = Temp;
		}
		Tail = Temp;
		Tail->FS = SearchList->FS;
		Tail->Next = NULL;
		SearchList = SearchList->Next;
	}
	return NewList;
}

JETAPI jeBoolean JETCC jeVFile_AddPath(jeVFile *FS1, const jeVFile *FS2, jeBoolean Append)
{
	FSSearchList *	SearchList;

	assert(FS1);
	assert(FS2);
	assert( jeVFile_IsValid(FS1) );
	assert( jeVFile_IsValid(FS2) );

	jeVFile_Lock(FS1);
	jeVFile_Lock(FS2);

	SearchList = CopySearchList(FS2->SearchList);
	if	(!SearchList)
		goto fail;

	if	(Append == JE_FALSE)
	{
		SearchList->Next = FS1->SearchList;
		FS1->SearchList = SearchList;
	}
	else
	{
		FSSearchList	Temp;
		FSSearchList *	pTemp;

		Temp.Next = FS1->SearchList;
		pTemp = &Temp;
		while	(pTemp->Next)
		{
			pTemp = pTemp->Next;
		}

		pTemp->Next = SearchList;
//		SearchList->Next = NULL;
	}

	jeVFile_UnLock(FS2);
	jeVFile_UnLock(FS1);
	return JE_TRUE;

fail:
	jeVFile_UnLock(FS2);
	jeVFile_UnLock(FS1);
	return JE_FALSE;
}
#endif //}

JETAPI jeBoolean JETCC jeVFile_DeleteFile(jeVFile *FS, const char *FileName)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(FS) );
	assert( jeVFile_PathIsSane(FileName) );

	jeVFile_Lock(FS);
	Result = FS->APIs->DeleteFile(FS, FS->FSData, FileName);
	jeVFile_UnLock(FS);
	return Result;
}

JETAPI jeBoolean JETCC jeVFile_RenameFile(jeVFile *FS, const char *FileName, const char *NewName)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(FS) );
	assert( jeVFile_PathIsSane(FileName) );
	assert( jeVFile_PathIsSane(NewName) );

	jeVFile_Lock(FS);
	Result = FS->APIs->RenameFile(FS, FS->FSData, FileName, NewName);
	jeVFile_UnLock(FS);
	return Result;
}

JETAPI jeBoolean JETCC jeVFile_FileExists(jeVFile *FS, const char *FileName)
{
	assert( jeVFile_PathIsSane(FileName) );
	assert( jeVFile_IsValid(FS) );
	return FS->APIs->FileExists(FS, FS->FSData, FileName);
}

JETAPI jeBoolean JETCC jeVFile_Disperse(jeVFile *FS, const char *Directory)
{
	jeBoolean	Result;

	assert(Directory);
	assert( jeVFile_IsValid(FS) );
	assert( jeVFile_PathIsSane(Directory) );

	jeVFile_Lock(FS);
	Result = FS->APIs->Disperse(FS, FS->FSData, Directory);
	jeVFile_UnLock(FS);
	return Result;
}

JETAPI jeBoolean JETCC jeVFile_GetS(jeVFile *File, void *Buff, int MaxLen)
{
	jeBoolean	Result;

	assert(Buff);
	assert( jeVFile_IsValid(File) );

	if	(MaxLen == 0)
		return JE_FALSE;

	jeVFile_Lock(File);
	Result = File->APIs->GetS(File->FSData, Buff, MaxLen);
	jeVFile_UnLock(File);
	
	assert( jeVFile_IsValid(File) );

	return Result;
}

JETAPI jeBoolean JETCC jeVFile_BytesAvailable(jeVFile *File, long *Count)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );
	
	jeVFile_Lock(File);
	Result = File->APIs->BytesAvailable(File->FSData, Count);
	jeVFile_UnLock(File);
	
	assert( jeVFile_IsValid(File) );

	return Result;
}

jeVFile * Hack_File;
jeBoolean Hack_Used = 0;

JETAPI jeBoolean JETCC jeVFile_Read(jeVFile *File, void *Buff, uint32 Count)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );

	if	(Count == 0) // <> CB 2/10
		return JE_TRUE;

	assert(Buff);

	jeVFile_Lock(File);
//	assert( ! Hack_Used );
	Hack_File = File;
	Hack_Used ++;
	Result = File->APIs->Read(File->FSData, Buff, Count);
	Hack_Used --;
	jeVFile_UnLock(File);
	
	assert( jeVFile_IsValid(File) );

	return Result;
}

JETAPI jeBoolean JETCC jeVFile_Write(jeVFile *File, const void *Buff, int Count)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );

	if	(Count == 0) // <> CB 2/10
		return JE_TRUE;

	assert(Buff);

#ifdef KROUERDEBUG
	{
		char msg[80];
		sprintf(msg, "jeVFile::Write %d bytes\n", Count);
		OutputDebugString(msg);
	}
#endif

	jeVFile_Lock(File);
	Result = File->APIs->Write(File->FSData, Buff, Count);
	jeVFile_UnLock(File);
	
#ifdef KROUERDEBUG
	{
		char msg[80];
		sprintf(msg, "jeVFile::Write result %d\n", Result);
		OutputDebugString(msg);
	}
#endif
	assert( jeVFile_IsValid(File) );

	return Result;
}

JETAPI jeBoolean JETCC jeVFile_Rewind(jeVFile *File)
{
jeVFile * HintsFile;

	assert( jeVFile_IsValid(File) );

	if ( ! jeVFile_Seek(File,0,JE_VFILE_SEEKSET) )
		return JE_FALSE;

	HintsFile = jeVFile_GetHintsFile(File);
	if ( HintsFile )
	{
		if ( ! jeVFile_Seek(HintsFile,0,JE_VFILE_SEEKSET) )
			return JE_FALSE;
	}

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeVFile_Seek(jeVFile *File, int Where, jeVFile_Whence Whence)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );
	
	jeVFile_Lock(File);
	Result = File->APIs->Seek(File->FSData, Where, Whence);
	jeVFile_UnLock(File);
	
	assert( jeVFile_IsValid(File) );

	return Result;
}

JETAPI jeBoolean JETCC jeVFile_Printf(jeVFile *File, const char *Format, ...)
{
	jeBoolean	Result;
	char		Temp[8096];
	va_list		ArgPtr;

	assert( jeVFile_IsValid(File) );
	assert(Format);

	va_start(ArgPtr, Format);
	vsprintf(Temp, Format, ArgPtr);
	va_end(ArgPtr);

	jeVFile_Lock(File);
	Result = File->APIs->Write(File->FSData, &Temp[0], strlen(Temp));
	jeVFile_UnLock(File);
	return Result;
}

JETAPI jeBoolean JETCC jeVFile_EOF   (const jeVFile *File)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );
	
	jeVFile_Lock(File);
	Result = File->APIs->Eof(File->FSData);
	jeVFile_UnLock(File);
	return Result;
}

JETAPI jeBoolean JETCC jeVFile_Tell  (const jeVFile *File, long *Position)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );
	
	jeVFile_Lock(File);
	Result = File->APIs->Tell(File->FSData, Position);
	jeVFile_UnLock(File);
	return Result;
}

JETAPI jeBoolean JETCC jeVFile_Size  (const jeVFile *File, long *Size)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );
	
	jeVFile_Lock(File);
	Result = File->APIs->Size(File->FSData, Size);
	jeVFile_UnLock(File);

	assert( jeVFile_IsValid(File) );

	return Result;
}

JETAPI jeBoolean JETCC jeVFile_GetProperties(const jeVFile *File, jeVFile_Properties *Properties)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );
	
	jeVFile_Lock(File);
	Result = File->APIs->GetProperties(File->FSData, Properties);
	jeVFile_UnLock(File);

	assert( jeVFile_IsValid(File) );

	return Result;
}

JETAPI jeBoolean JETCC jeVFile_GetName(const jeVFile *File, char *Buff, int MaxBuffLen)
{
jeVFile_Properties Properties;
	if ( ! jeVFile_GetProperties(File,&Properties) )
		return JE_FALSE;

	strncpy(Buff,Properties.Name,MaxBuffLen);

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeVFile_SetSize(jeVFile *File, long Size)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );
	
	jeVFile_Lock(File);
	Result = File->APIs->SetSize(File->FSData, Size);
	jeVFile_UnLock(File);

	assert( jeVFile_IsValid(File) );

	return Result;
}

JETAPI jeBoolean JETCC jeVFile_SetAttributes(jeVFile *File, jeVFile_Attributes Attributes)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );
	
	jeVFile_Lock(File);
	Result = File->APIs->SetAttributes(File->FSData, Attributes);
	jeVFile_UnLock(File);
	return Result;
}

JETAPI jeBoolean JETCC jeVFile_SetTime(jeVFile *File, const jeVFile_Time *Time)
{
	jeBoolean	Result;

	assert( jeVFile_IsValid(File) );
	
	jeVFile_Lock(File);
	Result = File->APIs->SetTime(File->FSData, Time);
	jeVFile_UnLock(File);
	return Result;
}

JETAPI jeVFile * JETCC jeVFile_GetHintsFile(jeVFile *File)
{
	jeVFile *	Result;

#pragma message("jeVFile_GetHintsFile : remove me for CreateHintsFile")

	assert( jeVFile_IsValid(File) );

	if	(File->IsHintsFile == JE_TRUE)
		return NULL;

	jeVFile_Lock(File);
	Result = File->APIs->GetHintsFile(File->FSData);
	if	(Result)
		Result->IsHintsFile = JE_TRUE;
	jeVFile_UnLock(File);
	
	return Result;
}

JETAPI jeVFile * JETCC jeVFile_CreateHintsFile(jeVFile *File)
{
	jeVFile *	Result;

	assert( jeVFile_IsValid(File) );

	if	(File->IsHintsFile == JE_TRUE)
		return NULL;

	jeVFile_Lock(File);
	Result = File->APIs->GetHintsFile(File->FSData);
	if	(Result)
		Result->IsHintsFile = JE_TRUE;
	jeVFile_UnLock(File);
	
	if ( Result )
	{
	/*
	// no; you can't do this: 
	// Context will call Close on the HintsFile
	// and then HintsFile will Close its context, which will ...
		assert( Result->Context == NULL || Result->Context == File );
		if ( Result->Context != File )
		{
		int r;
			for(r = File->RefCount;r>=0;r--)
				jeVFile_CreateRef(File);
		}
		Result->Context = File;
	*/
		jeVFile_CreateRef(Result);
	}

	return Result;
}

JETAPI jeVFile_Finder * JETCC jeVFile_CreateFinder(
	jeVFile *FileSystem,
	const char *FileSpec)
{
	jeVFile_Finder *	Finder;

	assert(FileSystem);
	assert(FileSpec);
	assert( jeVFile_IsValid(FileSystem) );
	assert( jeVFile_PathIsSane(FileSpec) );

	// CB : I don't think we use Finders enough to justify a MemPool

	Finder = (jeVFile_Finder *)jeRam_Allocate(sizeof(jeVFile_Finder));
	if	(!Finder)
		return Finder;

	jeVFile_Lock(FileSystem);
	Finder->Data = FileSystem->APIs->FinderCreate(FileSystem, FileSystem->FSData, FileSpec);
	jeVFile_UnLock(FileSystem);
	if	(!Finder->Data)
	{
		jeRam_Free(Finder);
		return NULL;
	}

	Finder->APIs = FileSystem->APIs;
       
    #ifdef WIN32
    InitializeCriticalSection(&Finder->CriticalSection);
    #endif
        
    #ifdef BUILD_BE
    Finder->CriticalSection = create_sem(1,NULL);
    #endif
        
	return Finder;
}

JETAPI void JETCC jeVFile_DestroyFinder(jeVFile_Finder *Finder)
{
	assert(Finder);
	assert(Finder->APIs);

	LOCK_CRITICALSECTION(&Finder->CriticalSection);
	Finder->APIs->FinderDestroy(Finder->Data);
	UNLOCK_CRITICALSECTION(&Finder->CriticalSection);
	DELETE_CRITICALSECTION(&Finder->CriticalSection);
	jeRam_Free(Finder);
}

JETAPI jeBoolean JETCC jeVFile_FinderGetNextFile(jeVFile_Finder *Finder)
{
	jeBoolean	Result;

	assert(Finder);
	assert(Finder->APIs);
	assert(Finder->Data);
	
	LOCK_CRITICALSECTION(&Finder->CriticalSection);
	Result = Finder->APIs->FinderGetNextFile(Finder->Data);
	UNLOCK_CRITICALSECTION(&Finder->CriticalSection);
	return Result;
}

JETAPI jeBoolean JETCC jeVFile_FinderGetProperties(const jeVFile_Finder *Finder, jeVFile_Properties *Properties)
{
	jeBoolean	Result;

	assert(Finder);
	assert(Finder->APIs);
	assert(Finder->Data);

	LOCK_CRITICALSECTION(&((jeVFile_Finder *)Finder)->CriticalSection);
	Result = Finder->APIs->FinderGetProperties(Finder->Data, Properties);
	UNLOCK_CRITICALSECTION(&((jeVFile_Finder *)Finder)->CriticalSection);

	return Result;
}

#ifdef WIN32
JETAPI void JETCC jeVFile_TimeToWin32FileTime(const jeVFile_Time *Time, LPFILETIME Win32FileTime)
{
        *Win32FileTime = *(LPFILETIME)Time;
}
#endif

#ifdef BUILD_BE
JETAPI void JETCC jeVFile_TimeToTime_TFileTime(const jeVFile_Time *Time, bigtime_t* fileTime)
{
        // not sure how we do this yet..
        memcpy(fileTime,Time,sizeof(int64));
}
#endif

static	jeBoolean	JETCC	CheckOpenFlags(unsigned int OpenModeFlags)
{
	int 			FlagCount;
	unsigned int	AccessFlags;

	// Test to see that the open mode for this thing is mutually exclusive in
	// the proper flags.
	FlagCount = 0;
	AccessFlags = OpenModeFlags & (JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_UPDATE | JE_VFILE_OPEN_CREATE);
	if	(AccessFlags & JE_VFILE_OPEN_READONLY)
		FlagCount++;
	if	(AccessFlags & JE_VFILE_OPEN_UPDATE)
		FlagCount++;
	if	(AccessFlags & JE_VFILE_OPEN_CREATE)
		FlagCount++;

	if	(FlagCount != 1 && !(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY))
		return JE_FALSE;

	return JE_TRUE;
}

static jeBoolean jeVFile_PathIsSane(const char * Path)
{
char * SubStr; 

	// look for paths that will crash your computer

	assert(Path);

	if ( (SubStr = strstr(Path,"//")) != NULL )
	{
		assert( SubStr[0] == '/' && SubStr[1] == '/' );
		if ( SubStr[-1] != ':' )
			return JE_FALSE;
	}
	if ( strstr(Path,"///") != NULL )
		return JE_FALSE;
	if ( (SubStr = strstr(Path,"\\\\")) != NULL )
	{
		if ( SubStr != Path )
			return JE_FALSE;
		if ( Path[2] == 0 )
			return JE_FALSE;
	}
return JE_TRUE;
}

#ifndef NDEBUG
JETAPI jeBoolean JETCC jeVFile_IsValid(const jeVFile * F)
{
	assert( F );
	assert( F->RefCount >= 0 );
//	assert( jeRam_IsValidPtr(F) );
	
#ifndef LN_SEARCHLIST
	assert( jeRam_IsValidPtr(F->SearchList) );
#else
	assert( F->LN.Next );
#endif
	assert( F->Semaphore );
	assert( F->IsHintsFile == JE_TRUE || F->IsHintsFile == JE_FALSE );
//	assert( F->SystemType != JE_VFILE_TYPE_INVALID ); // why is this commented out?
	assert( F->SystemType < JE_VFILE_TYPE_COUNT );
	if ( F->Context )
	{
		assert(F->Context->RefCount >= F->RefCount);
		if ( ! jeVFile_IsValid(F->Context) )
			return JE_FALSE;
	}
return JE_TRUE;
}
#endif

/*}{**** VFile Ram *********/

#include	"MemPool.h"

static MemPool * VFilePool = NULL;
static int VFilesAllocated = 0;

static jeVFile * JETCC jeVFile_New(void)
{
jeVFile * File;

	if ( ! VFilesAllocated )
	{
		assert( ! VFilePool );
		VFilePool = MemPool_Create(sizeof(jeVFile),32,32);
		if ( ! VFilePool )
			return NULL;
	}

	File = (jeVFile *)MemPool_GetHunk(VFilePool);
	assert(File);
	VFilesAllocated++;

return File;
}

static void JETCC jeVFile_Free(jeVFile * File)
{
	assert(File);
	assert(VFilePool);
	MemPool_FreeHunk(VFilePool,File);
	VFilesAllocated--;
	if ( ! VFilesAllocated )
	{
		MemPool_Destroy(&VFilePool);
		VFilePool = NULL;
	} 
}
