/****************************************************************************************/
/*  FSLZ.C                                                                              */
/*                                                                                      */
/*  Author:  Charles Bloom                                                              */
/*  Description:                                                                        */
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

/***********

todos:

1. use the jeErrorLog * for threads

************/

// Threading the decompressor is probably pointless, cuz its so fast
#define FSLZ_ALWAYS_THREAD_READER 0
//#define FSLZ_ALWAYS_THREAD_READER 1

#ifndef _LOG
#define _LOG
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>

#include	"BaseType.h"
#include	"Ram.h"
#include	"ThreadQueue.h"

#include	"VFile.h"
#include	"VFile._h"

#include	"FSLZ.h"
#include	"lza.h"
#include	"Log.h"

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#define FSLZ_TAG_TYPE	uint32
#define FSLZ_TAG		((FSLZ_TAG_TYPE)0x5A4C5346)	// "FSLZ"
#define FSLZ_TAG_UNC	((FSLZ_TAG_TYPE)0x4E555A4C)	// "LZUN"

typedef struct	LZFile
{
	FSLZ_TAG_TYPE	Tag;

	uint32		Reading;
	uint32		Uncompressed;
	uint32		Pos;
	uint32		Size;
	int32		HintsSize;

	jeThreadQueue_Semaphore * Lock;	// the Lock applies to all stuff below:

	uint32		RefCount;
	jeVFile *	BaseFile;	// never touched by the main-thread accessor functions
	jeVFile *	HintsBaseFile;
	jeVFile *	MemFile;
	jeVFile *	HintsMemFile;
	uint32		MemFileLen;
	jeThreadQueue_Job * ReaderJob;

	// stuff only used by the ReaderJob:
	lzaDecoder * Decoder;
	uint32		CompLen,CompLenRead;
	uint8 *		CompArray;
} LZFile;

/*}{******************* Protos ******************************/

static void __inline FSLZ_Lock(LZFile * File);
static void __inline FSLZ_UnLock(LZFile * File);

static	jeBoolean	JETCC FSLZ_Close(void *Handle);
static	jeBoolean	JETCC FSLZ_Close2(void *Handle,jeBoolean Reader);
static	jeBoolean	JETCC FSLZ_BytesAvailable(void *Handle, int32 *pCount);

void FSLZReader_Func(jeThreadQueue_Job * Job,void * Context);
void FSLZReader_Peek(LZFile * File);
static jeBoolean jeVFile_CopyData(jeVFile * Fm,jeVFile *To,int Size);

/*}{******************* Implemented ******************************/

static	void *	JETCC FSLZ_OpenNewSystem(
	jeVFile *		FS,
	const char *	Name,
	void *			Context,
	unsigned int			OpenModeFlags)
{
LZFile * File;

	if ( Name || Context || !FS )
		return NULL;

	if	(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY)
		return NULL;

	File = (LZFile *)jeRam_AllocateClear(sizeof(*File));
	if	(!File)
		return NULL;

	File->BaseFile = FS;
	jeVFile_CreateRef(File->BaseFile);

	File->HintsBaseFile = jeVFile_GetHintsFile(File->BaseFile);
	if ( ! File->HintsBaseFile )
		File->HintsBaseFile = File->BaseFile;
	jeVFile_CreateRef(File->HintsBaseFile);

	File->Tag = FSLZ_TAG;

	if ( OpenModeFlags & JE_VFILE_OPEN_READONLY)
		File->Reading = JE_TRUE;

	if ( File->Reading )
	{
		jeVFile_Read(File->HintsBaseFile,&(File->Tag),sizeof(File->Tag));
		if ( File->Tag == FSLZ_TAG_UNC )
		{
		jeVFile_MemoryContext MemContext;
		jeVFile * NewBaseFile,*NewHintsBaseFile;

			if ( ! jeVFile_Read(File->HintsBaseFile,&(File->Size),sizeof(File->Size)) )
			{
				jeErrorLog_AddString(-1,"FSLZ : Read Hints failed!",NULL);
				FSLZ_Close(File);
				return NULL;
			}
			
			if ( ! jeVFile_Read(File->HintsBaseFile,&(File->HintsSize),sizeof(File->HintsSize)) )
			{
				jeErrorLog_AddString(-1,"FSLZ_OpenNew : Base Read failed!",NULL);
				assert(0);
				FSLZ_Close(File);
				return NULL;
			}

			MemContext.DataLength = File->HintsSize;
			MemContext.Data = jeRam_Allocate(MemContext.DataLength);
			if ( ! MemContext.Data )
			{
				jeErrorLog_AddString(-1,"FSLZ : Allocate failed!",NULL);
				FSLZ_Close(File);
				return NULL;
			}

			if ( ! jeVFile_Read(File->HintsBaseFile,MemContext.Data,MemContext.DataLength) )
			{
				jeErrorLog_AddString(-1,"FSLZ : Allocate failed!",NULL);
				FSLZ_Close(File);
				return NULL;
			}

			NewHintsBaseFile = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_MEMORY,NULL,&MemContext,JE_VFILE_OPEN_READONLY);
			if ( ! File->HintsBaseFile )
			{
				FSLZ_Close(File);
				return NULL;
			}

			MemContext.DataLength = File->Size;
			MemContext.Data = jeRam_Allocate(MemContext.DataLength);
			if ( ! MemContext.Data )
			{
				jeErrorLog_AddString(-1,"FSLZ : Allocate failed!",NULL);
				FSLZ_Close(File);
				return NULL;
			}

			if ( ! jeVFile_Read(FS,MemContext.Data,MemContext.DataLength) )
			{
				jeErrorLog_AddString(-1,"FSLZ : Allocate failed!",NULL);
				FSLZ_Close(File);
				return NULL;
			}
			
			NewBaseFile = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_MEMORY,NULL,&MemContext,JE_VFILE_OPEN_READONLY);
			if ( ! File->BaseFile )
			{
				FSLZ_Close(File);
				return NULL;
			}

			File->Uncompressed = JE_TRUE;

			jeVFile_Close(File->HintsBaseFile);
			jeVFile_Close(File->BaseFile);

			File->HintsBaseFile = NewHintsBaseFile;
			File->BaseFile = NewBaseFile;

			return File;
		}
		else if ( File->Tag != FSLZ_TAG )
		{
			jeVFile_Seek(File->HintsBaseFile,- (int)sizeof(File->Tag),JE_VFILE_SEEKCUR);
			jeErrorLog_AddString(-1,"FSLZ : Opening uncompressed without UNC header!",NULL);
			#pragma message("FSLZ : UNC NoHeader open may pass back hintsfile == file !")
			File->Uncompressed = JE_TRUE;

			return File;
		}
	}

	{
	jeVFile_MemoryContext MemContext;
		MemContext.Data = NULL;
		MemContext.DataLength = 0;

		File->MemFile = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_MEMORY,NULL,&MemContext,OpenModeFlags);
		if ( ! File->MemFile )
		{
			FSLZ_Close(File);
			return NULL;
		}
		MemContext.Data = NULL;
		MemContext.DataLength = 0;

		File->HintsMemFile = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_MEMORY,NULL,&MemContext,OpenModeFlags);
		if ( ! File->MemFile )
		{
			FSLZ_Close(File);
			return NULL;
		}
	}

	if ( File->Reading )
	{
		if ( ! jeVFile_Read(File->HintsBaseFile,&(File->Size),sizeof(File->Size)) )
		{
			jeErrorLog_AddString(-1,"FSLZ_OpenNew : Base Read failed!",NULL);
			assert(0);
			FSLZ_Close(File);
			return NULL;
		}

		if ( ! jeVFile_Read(File->HintsBaseFile,&(File->CompLen),sizeof(File->CompLen)) )
		{
			jeErrorLog_AddString(-1,"FSLZ_OpenNew : Base Read failed!",NULL);
			assert(0);
			FSLZ_Close(File);
			return NULL;
		}

		if ( ! jeVFile_Read(File->HintsBaseFile,&(File->HintsSize),sizeof(File->HintsSize)) )
		{
			jeErrorLog_AddString(-1,"FSLZ_OpenNew : Base Read failed!",NULL);
			assert(0);
			FSLZ_Close(File);
			return NULL;
		}

		if ( File->HintsSize > 0 )
		{
			if ( ! jeVFile_SetSize(File->HintsMemFile,File->HintsSize) )
			{
				jeErrorLog_AddString(-1,"FSLZ_OpenNew : Hints SetSize failed!",NULL);
				FSLZ_Close(File);
				return NULL;
			}
		}

		// read hints
		
		if ( ! jeVFile_CopyData(File->HintsBaseFile,File->HintsMemFile,File->HintsSize) )
		{
			jeErrorLog_AddString(-1,"FSLZ_OpenNew : copy hints!",NULL);
			FSLZ_Close(File);
			return NULL;
		}
		jeVFile_Rewind(File->HintsMemFile);

		if ( File->Size > 0 )
		{
			if ( ! jeVFile_SetSize(File->MemFile,File->Size) )
			{
				jeErrorLog_AddString(-1,"FSLZ_OpenNew : SetSize ailed!",NULL);
				FSLZ_Close(File);
				return NULL;
			}
		}

		if ( ! (File->CompArray = (uint8 *)jeRam_Allocate(File->CompLen) ) )
		{
			jeErrorLog_AddString(-1,"FSLZ_OpenNew : Allocate CompLen failed!",NULL);
			FSLZ_Close(File);
			return NULL;
		}

		{
		jeVFile_Properties Prop;
		memset(&Prop, 0 , sizeof(jeVFile_Properties) );
		
			File->RefCount++;
			if ( jeVFile_GetProperties(File->BaseFile,&Prop) && (Prop.AttributeFlags & JE_VFILE_ATTRIB_REMOTE) || ( FSLZ_ALWAYS_THREAD_READER == 1) )
			{
				File->Lock = jeThreadQueue_Semaphore_Create();
				if ( ! File->Lock )
				{
					jeErrorLog_AddString(-1,"FSLZ_OpenNew : Semaphore_Create failed!",NULL);
					FSLZ_Close(File);
					return NULL;
				}

				File->ReaderJob = jeThreadQueue_JobCreate(FSLZReader_Func,File,NULL,16384);
				if ( ! File->ReaderJob )
				{
					jeErrorLog_AddString(-1,"FSLZ_OpenNew : Thread_Create failed!",NULL);
					FSLZ_Close(File);
					return NULL;
				}
			}
			else
			{
				FSLZReader_Func(NULL,File);
			}
		}
	}

return File;
}

static	jeBoolean JETCC FSLZ_Close2(void *Handle,jeBoolean Reader)
{
LZFile * File;
jeBoolean Ret = JE_TRUE;
	
	File = (LZFile*)Handle;
	
	if ( File->Reading )
	{

		assert( ! Reader || File->RefCount > 0 );

		// must wait on Job BEFORE you Lock !

		if ( ! Reader && File->ReaderJob )
		{
			jeThreadQueue_WaitOnJob(File->ReaderJob,JE_THREADQUEUE_STATUS_COMPLETED);
			jeThreadQueue_JobDestroy(&(File->ReaderJob));
			File->ReaderJob = NULL;

			if ( File->Lock )
				jeThreadQueue_Semaphore_Destroy(&(File->Lock));
		}

		// reader job must be gone now

		if ( File->RefCount > 0 )
		{
			File->RefCount--;
			return JE_TRUE;
		}

		if ( File->CompArray )
		{
			jeRam_Free(File->CompArray);
			File->CompArray = NULL;
		}
		if ( File->Decoder )
		{
			lzaDecoder_Destroy(&(File->Decoder));
			File->Decoder = NULL;
		}

		// we're done
	}
	else
	{
		if ( File->RefCount > 0 )
		{
			File->RefCount--;
			return JE_TRUE;
		}

		{
		jeVFile_MemoryContext MemContext;
			// compresss & write to BaseFile !
			if ( jeVFile_UpdateContext(File->MemFile,&MemContext,sizeof(MemContext)) )
			{
			uint8 * OutBuf;
			uint32 OutLen;
				assert((int)File->Size == MemContext.DataLength);
				lzaEncode((uint8*)MemContext.Data,MemContext.DataLength,&OutBuf,&OutLen);
				if ( OutBuf && OutLen )
				{
					if ( OutLen < (uint32)MemContext.DataLength )
					{
						File->Tag = FSLZ_TAG;
						if (! jeVFile_Write(File->HintsBaseFile,&(File->Tag),sizeof(File->Tag)) ||
							! jeVFile_Write(File->HintsBaseFile,&(File->Size),sizeof(File->Size)) ||
							! jeVFile_Write(File->HintsBaseFile,&OutLen,sizeof(OutLen)) )
						{
							jeErrorLog_AddString(-1,"FSLZ_Close : VFile_WriteHints failed!",NULL);
							Ret = JE_FALSE;
						}
									// write hints
			
						if ( ! jeVFile_Size(File->HintsMemFile,&(File->HintsSize)) )
						{
							jeErrorLog_AddString(-1,"FSLZ_Close : VFile Hints Size failed!",NULL);
							Ret = JE_FALSE;
						}
						else
						{
							if (! jeVFile_Write(File->HintsBaseFile,&(File->HintsSize),sizeof(File->HintsSize)) )
							{
								jeErrorLog_AddString(-1,"FSLZ_Close : VFile_WriteHints failed!",NULL);
								Ret = JE_FALSE;
							}

							jeVFile_Rewind(File->HintsMemFile);
							if ( ! jeVFile_CopyData(File->HintsMemFile,File->HintsBaseFile,File->HintsSize) )
							{
								jeErrorLog_AddString(-1,"FSLZ_Close : copy hints!",NULL);
								Ret = JE_FALSE;
							}
						}

						if ( ! jeVFile_Write(File->BaseFile,OutBuf,OutLen) )						
						{
							jeErrorLog_AddString(-1,"FSLZ_Close : VFile_Write failed!",NULL);
							Ret = JE_FALSE;
						}

						Log_Printf("FSLZ : compressed %d -> %d = %1.3f bpc\n",MemContext.DataLength,OutLen,(OutLen*8.0f/MemContext.DataLength));

						#ifdef COUNT_HEADER_SIZES
						Header_Sizes += 12;
						#endif
						
						jeRam_Free(OutBuf);
					}
					else
					{
						jeRam_Free(OutBuf);

						// these hints cost 12 bytes :
						// 4 hints header
						// 4 hints len
						// 4 hints bytes
						// we could avoid this if we had VFile_UnWriteHints
						
						#pragma message("FSLZ : try passing through uncompressed without UNC header")

						File->Tag = FSLZ_TAG_UNC;
						if (	! jeVFile_Write(File->HintsBaseFile,&(File->Tag),sizeof(File->Tag))
							||	! jeVFile_Write(File->HintsBaseFile,&(MemContext.DataLength),sizeof(MemContext.DataLength)) )
						{
							jeErrorLog_AddString(-1,"FSLZ_Close : VFile_WriteHints failed!",NULL);
							Ret = JE_FALSE;
						}
						else if ( ! jeVFile_Size(File->HintsMemFile,&(File->HintsSize)) )
						{
							jeErrorLog_AddString(-1,"FSLZ_Close : VFile Hints Size failed!",NULL);
							Ret = JE_FALSE;
						}
						else if (! jeVFile_Write(File->HintsBaseFile,&(File->HintsSize),sizeof(File->HintsSize)) )
						{
							jeErrorLog_AddString(-1,"FSLZ_Close : VFile_WriteHints failed!",NULL);
							Ret = JE_FALSE;
						}
						else if ( ! jeVFile_Rewind(File->HintsMemFile) ||
								! jeVFile_CopyData(File->HintsMemFile,File->HintsBaseFile,File->HintsSize) )
						{
							jeErrorLog_AddString(-1,"FSLZ_Close : copy hints!",NULL);
							Ret = JE_FALSE;
						}
						else if ( ! jeVFile_Write(File->BaseFile,MemContext.Data,MemContext.DataLength) )
						{
							jeErrorLog_AddString(-1,"FSLZ_Close : VFile_Write failed!",NULL);
							Ret = JE_FALSE;
						}
					}
				}
				else
				{
					jeErrorLog_AddString(-1,"FSLZ_Close : lzaEncode failed!",NULL);
					Ret = JE_FALSE;
				}
			}

		}
	}

	if ( File->BaseFile )
		jeVFile_Close(File->BaseFile);
	if ( File->HintsBaseFile )
		jeVFile_Close(File->HintsBaseFile);
	if ( File->MemFile )
		jeVFile_Close(File->MemFile);
	if ( File->HintsMemFile )
		jeVFile_Close(File->HintsMemFile);

	jeRam_Free(File);

return Ret;
}

static	jeBoolean JETCC FSLZ_Close(void *Handle)
{
return FSLZ_Close2(Handle,JE_FALSE);
}

/*}{******************* The Reader Job ******************************/

void FSLZReader_Func(jeThreadQueue_Job * Job,void * Context)
{
LZFile * File;
uint32 CurLen;
uint32 NewMemFileLen;

	File = (LZFile*)Context;

	while( File->CompLenRead < File->CompLen )
	{

		jeVFile_BytesAvailable(File->BaseFile,(int32 *)&CurLen);
		while ( CurLen < 10 && (CurLen + File->CompLenRead != File->CompLen) )
		{
			if ( jeVFile_EOF(File->BaseFile) )
				goto fail;
			jeThreadQueue_Sleep(10);
			jeVFile_BytesAvailable(File->BaseFile,(int32 *)&CurLen);
		}

		CurLen = min(CurLen, File->CompLen - File->CompLenRead );

		FSLZ_Lock(File);
		jeVFile_BytesAvailable(File->BaseFile,(int32 *)&CurLen);
		
		if ( jeVFile_Read(File->BaseFile,File->CompArray + File->CompLenRead, CurLen ) )
		{
			if ( File->CompLenRead == 0 )
			{
			jeVFile_MemoryContext MemContext;
				if ( ! jeVFile_UpdateContext(File->MemFile,&MemContext,sizeof(MemContext)) )
				{
					FSLZ_UnLock(File);
					goto fail;
				}

				File->Decoder = lzaDecoder_Create(File->CompArray,File->CompLen,CurLen,(uint8*)MemContext.Data,MemContext.DataLength);
				if ( ! File->Decoder )
				{
					FSLZ_UnLock(File);
					goto fail;
				}
					
				lzaDecoder_Extend(File->Decoder,0,&NewMemFileLen);
				assert( NewMemFileLen >= File->MemFileLen );
				File->MemFileLen = NewMemFileLen;
					
			}
			else
			{
				lzaDecoder_Extend(File->Decoder,CurLen,&NewMemFileLen);
				assert( NewMemFileLen >= File->MemFileLen );
				File->MemFileLen = NewMemFileLen;
			}
			File->CompLenRead += CurLen;
			assert( File->CompLenRead <= File->CompLen );

			Log_Printf("FSLZ : decompressed %d -> %d\n",File->CompLenRead,File->MemFileLen);
		}

		// we don't have to tell the MemFile that we've added to it!

#if 0 // for debugging ; should be a "do nothing"
		if ( File->CompLenRead == File->CompLen )
		{
			lzaDecoder_Extend(File->Decoder,0,&NewMemFileLen);
		}
#endif

		FSLZ_UnLock(File);
	}

fail:

	FSLZ_Lock(File);

	if ( File->CompArray )
	{
		jeRam_Free(File->CompArray);
		File->CompArray = NULL;
	}
	if ( File->Decoder )
	{
		lzaDecoder_Destroy(&(File->Decoder));
		File->Decoder = NULL;
	}
	
	FSLZ_UnLock(File);

	FSLZ_Close2(File,JE_TRUE);

return;
}

void FSLZReader_Peek(LZFile * File)
{
jeThreadQueue_JobStatus Status;

	if ( ! File->ReaderJob )
		return;

	FSLZ_Lock(File);
	
	Status = jeThreadQueue_JobGetStatus(File->ReaderJob);

	FSLZ_UnLock(File);

	if ( Status == JE_THREADQUEUE_STATUS_WAITINGFORTHREAD )
	{
		jeThreadQueue_PollJobs();
	}
	else if ( Status == JE_THREADQUEUE_STATUS_COMPLETED )
	{
		FSLZ_Lock(File);
		jeThreadQueue_JobDestroy(&(File->ReaderJob));
		File->ReaderJob = NULL;
		FSLZ_UnLock(File);
	}
}

/*}{******************* Utilities ******************************/

static jeBoolean jeVFile_CopyData(jeVFile * Fm,jeVFile *To,int CurSize)
{
char CopyBuff[1024];
int CurLen;
	while(CurSize)
	{
		CurLen = min(1024,CurSize);
		if ( ! jeVFile_Read( Fm,CopyBuff,CurLen) )
			return JE_FALSE;
		if ( ! jeVFile_Write(To,CopyBuff,CurLen) )
			return JE_FALSE;
		CurSize -= CurLen;
	}
return JE_TRUE;
}

static void __inline FSLZ_Lock(LZFile * File)
{
	if ( File->ReaderJob )
		jeThreadQueue_Semaphore_Lock(File->Lock);
}

static void __inline FSLZ_UnLock(LZFile * File)
{
	if ( File->ReaderJob )
		jeThreadQueue_Semaphore_UnLock(File->Lock);
}


/*}{******************* Pass-Throughs ******************************/

static	jeBoolean	JETCC FSLZ_BytesAvailable(void *Handle, int32 *pCount)
{
	LZFile *	File;

	File = (LZFile*)Handle;

	if ( ! File->Reading )
		return JE_FALSE;

	if ( File->Uncompressed )
		return jeVFile_BytesAvailable(File->BaseFile,pCount);

	FSLZReader_Peek(File);

	FSLZ_Lock(File);

	*pCount = (File->MemFileLen - File->Pos);

	FSLZ_UnLock(File);
	
return JE_TRUE;
}

static	jeBoolean	JETCC FSLZ_Read(void *Handle, void *Buff, uint32 Count)
{
LZFile * File;
uint32 Avail;

	File = (LZFile*)Handle;

	if ( ! File->Reading )
		return JE_FALSE;

	if ( File->Uncompressed )
	{
		return jeVFile_Read(File->BaseFile,Buff,Count);
	}
	
	if ( Count > (File->Size - File->Pos) )
		return JE_FALSE;
	if ( Count <= 0 )
		return JE_FALSE;

	FSLZ_BytesAvailable(Handle,(int32 *)&Avail);
	
	if ( Avail < Count )
	{
		if ( ! File->ReaderJob )
			return JE_FALSE;

		if ( ! jeThreadQueue_WaitOnJob(File->ReaderJob,JE_THREADQUEUE_STATUS_RUNNING) )
			return JE_FALSE;
			
		do
		{
			jeThreadQueue_Sleep(1);
			FSLZ_BytesAvailable(Handle,(int32 *)&Avail);
		} while( Avail < Count );
	}

	if ( File->ReaderJob )
	{
	jeThreadQueue_JobStatus Status;	
		Status = jeThreadQueue_JobGetStatus(File->ReaderJob);
		if ( Status == JE_THREADQUEUE_STATUS_COMPLETED )
		{
			FSLZ_Lock(File);
			jeThreadQueue_JobDestroy(&File->ReaderJob);
			File->ReaderJob = NULL;
			FSLZ_UnLock(File);
		}
	}
	
	FSLZ_Lock(File);

	if ( ! jeVFile_Read(File->MemFile,Buff,Count) )
		return JE_FALSE;

	FSLZ_UnLock(File);

	File->Pos += Count;
	return JE_TRUE;
}

static	jeBoolean	JETCC FSLZ_Write(void *Handle, const void *Buff, int Count)
{
LZFile * File;

	File = (LZFile*)Handle;

	if ( File->Reading )
		return JE_FALSE;

	if ( ! jeVFile_Write(File->MemFile,Buff,Count) )
		return JE_FALSE;

	File->Pos += Count;
	if ( File->Pos > File->Size )
		File->Size = File->Pos;
	File->MemFileLen = File->Size;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSLZ_Seek(void *Handle, int Where, jeVFile_Whence Whence)
{
LZFile * File;
uint32 NewPos;

	File = (LZFile*)Handle;

	if ( File->Uncompressed )
		return jeVFile_Seek(File->BaseFile,Where,Whence);
	
	switch(Whence)
	{
		case JE_VFILE_SEEKCUR:
			NewPos = File->Pos + Where;
			break;

		case JE_VFILE_SEEKEND:
			NewPos = File->Size + Where;
			break;

		case JE_VFILE_SEEKSET:
			NewPos = Where;
			break;
	}

	if ( File->Reading )
	{
	uint32 Len;
		FSLZ_Lock(File);
		Len = File->MemFileLen;
		FSLZ_UnLock(File);
		if ( NewPos > Len )
			 return JE_FALSE;
	}

	File->Pos = NewPos;

	return jeVFile_Seek(File->MemFile,Where,Whence);
}

static	jeBoolean	JETCC FSLZ_EOF(const void *Handle)
{
	const LZFile *	File;

	File = (LZFile*)Handle;

	if ( File->Uncompressed )
		return jeVFile_EOF(File->BaseFile);

	if ( File->Pos == File->Size )
		return JE_TRUE;

	return JE_FALSE;
}

static	jeBoolean	JETCC FSLZ_Tell(const void *Handle, int32 *pPosition)
{
	const LZFile *	File;

	File = (LZFile*)Handle;

	if ( File->Uncompressed )
		return jeVFile_Tell(File->BaseFile,pPosition);

	*pPosition = File->Pos;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSLZ_Size(const void *Handle, int32 *pSize)
{
	const LZFile *	File;

	File = (LZFile*)Handle;

	if ( File->Uncompressed )
		return jeVFile_Size(File->BaseFile,pSize);

	*pSize = File->Size;

	return JE_TRUE;
}


static	jeVFile *	JETCC FSLZ_GetHintsFile(void *Handle)
{
LZFile *	File;

	File = (LZFile*)Handle;

	if ( File->Uncompressed )
		return File->HintsBaseFile;

	if ( File->Reading )
	{
	int32 Size;
		if ( ! jeVFile_Size(File->HintsMemFile,&Size) )
			return NULL;
		if ( ! Size )
			return NULL;
	}

	return File->HintsMemFile;
}

static	int JETCC FSLZ_GetC(LZFile * File)
{
unsigned char C;
	if ( ! FSLZ_Read(File,&C,1) )
		return -1;
return C;
}

static	jeBoolean	JETCC FSLZ_GetS(void *Handle, char *Buff, int MaxLen)
{
LZFile *	File;
int C;
char * Ptr;

	File = (LZFile*)Handle;

	if ( ! File->Reading )
		return JE_FALSE;

	Ptr = Buff;
	while( MaxLen > 1 && (C = FSLZ_GetC(File)) != -1 )
	{	
		*Ptr++ = C;
		MaxLen--;
		if (C == '\n' || C == '\r' || C == 0 )
			break;		
	}
	
	while( (C = FSLZ_GetC(File)) != -1 )
	{
		if (C == '\n' || C == '\r' || C == 0 )
			continue;

		if ( ! FSLZ_Seek(File,-1,JE_VFILE_SEEKCUR) )
			return JE_FALSE;
		break;
	}

	*Ptr = 0;

return JE_TRUE;
}

static	jeBoolean	JETCC FSLZ_GetProperties(const void *Handle, jeVFile_Properties *Properties)
{
const LZFile * File;

	File = (LZFile*)Handle;

	if ( ! jeVFile_GetProperties(File->BaseFile,Properties) )
		return JE_FALSE;
	
	Properties->Size = File->Size;
return JE_TRUE;
}

/*}{******************* UnImplemented Bullshit ******************************/

static	void *	JETCC FSLZ_FinderCreate(
	jeVFile *			FS,
	void *			Handle,
	const char *	FileSpec)
{
	return NULL;
}

static	jeBoolean	JETCC FSLZ_FinderGetNextFile(void *Handle)
{
	assert(!Handle);
	return JE_FALSE;
}

static	jeBoolean	JETCC FSLZ_FinderGetProperties(void *Handle, jeVFile_Properties *Props)
{
	assert(!Handle);
	return JE_FALSE;
}

static	void JETCC FSLZ_FinderDestroy(void *Handle)
{
	assert(!Handle);
}

static	void *	JETCC FSLZ_Open(
	jeVFile *		FS,
	void *			Handle,
	const char *	Name,
	void *			Context,
	unsigned int			OpenModeFlags)
{
	return NULL;
}


static	jeBoolean	JETCC FSLZ_SetSize(void *Handle, int32 Size)
{
	assert(!"Not implemented");
	return JE_FALSE;
}

static	jeBoolean	JETCC FSLZ_SetAttributes(void *Handle, jeVFile_Attributes Attributes)
{
	assert(!"Not implemented");
	return JE_FALSE;
}

static	jeBoolean	JETCC FSLZ_SetTime(void *Handle, const jeVFile_Time *Time)
{
	assert(!"Not implemented");
	return JE_FALSE;
}

static	jeBoolean	JETCC FSLZ_FileExists(jeVFile *FS, void *Handle, const char *Name)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSLZ_Disperse(
	jeVFile *	FS,
	void *		Handle,
	const char *Directory)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSLZ_DeleteFile(jeVFile *FS, void *Handle, const char *Name)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSLZ_RenameFile(jeVFile *FS, void *Handle, const char *Name, const char *NewName)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSLZ_UpdateContext(
	jeVFile *		FS,
	void *			Handle,
	void *			Context,
	int 			ContextSize)
{
	return JE_FALSE;
}

/*}{******************* The FSLZ Struct ******************************/

#pragma warning (disable : 4113 4028)

static	jeVFile_SystemAPIs	FSLZ_APIs =
{
	FSLZ_FinderCreate,
	FSLZ_FinderGetNextFile,
	FSLZ_FinderGetProperties,
	FSLZ_FinderDestroy,

	FSLZ_OpenNewSystem,
	FSLZ_UpdateContext,
	FSLZ_Open,
	FSLZ_DeleteFile,
	FSLZ_RenameFile,
	FSLZ_FileExists,
	FSLZ_Disperse,
	FSLZ_Close,

	FSLZ_GetS,
	FSLZ_BytesAvailable,
	FSLZ_Read,
	FSLZ_Write,
	FSLZ_Seek,
	FSLZ_EOF,
	FSLZ_Tell,
	FSLZ_Size,

	FSLZ_GetProperties,

	FSLZ_SetSize,
	FSLZ_SetAttributes,
	FSLZ_SetTime,

	FSLZ_GetHintsFile,
};

const jeVFile_SystemAPIs * JETCC FSLZ_GetAPIs(void)
{
	return &FSLZ_APIs;
}

/*}{******************* EOF ******************************/
