/****************************************************************************************/
/*  FSFAKENET.C                                                                         */
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

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>

#include	"BaseType.h"
#include	"Ram.h"
#include	"ThreadQueue.h"

#include	"VFile.h"
#include	"VFile._h"

#include	"fsfakenet.h"
#include	"Log.h"
#include	"Tsc.h"

double FakeNet_ByPerSec		= 100.0f;
double FakeNet_LagSec		= 0.5f;
double FakeNet_FirstLagSec	= 1.0f;

typedef struct	FakeNetFile
{
	uint32		Pos;
	double		OpenTime,AvailTime;

	uint32		RefCount;
	jeVFile *	BaseFile;
} FakeNetFile;

/*}{******************* Protos ******************************/

static	jeBoolean	JETCC FSFakeNet_Close(void *Handle);
static	jeBoolean	JETCC FSFakeNet_BytesAvailable(void *Handle, long *pCount);
static	jeBoolean	JETCC FSFakeNet_Seek(void *Handle, int Where, jeVFile_Whence Whence);

/*}{******************* Open/Close ******************************/

static	void *	JETCC FSFakeNet_OpenNewSystem(
	jeVFile *		FS,
	const char *	Name,
	void *			Context,
	unsigned int	OpenModeFlags)
{
FakeNetFile * File;

	if ( Name || Context || !FS )
		return NULL;

	File = (FakeNetFile *)jeRam_AllocateClear(sizeof(*File));
	if	(!File)
		return NULL;

	File->BaseFile = FS;
	jeVFile_CreateRef(File->BaseFile);

	File->OpenTime = timeTSC();
	File->AvailTime = File->OpenTime;

return File;
}

static	jeBoolean JETCC FSFakeNet_Close(void *Handle)
{
FakeNetFile * File;
	File = (FakeNetFile *)Handle;

	if ( File->RefCount > 0 )
	{
		File->RefCount--;
		return JE_TRUE;
	}

	if ( File->BaseFile )
		jeVFile_Close(File->BaseFile);

	jeRam_Free(File);

return JE_TRUE;
}

/*}{******************* The Fake Net ******************************/

static	jeBoolean	JETCC FSFakeNet_BytesAvailable(void *Handle, long *pCount)
{
FakeNetFile * File;
int32 AllowLen,CurLen;
double CurTime,LagTime;

	File = (FakeNetFile *)Handle;

	if ( ! jeVFile_BytesAvailable(File->BaseFile,pCount) )
		return JE_FALSE;

	CurTime = timeTSC();

	if ( File->Pos == 0 )
		LagTime = FakeNet_FirstLagSec;
	else
		LagTime = FakeNet_LagSec;

	if ( CurTime < (File->AvailTime + LagTime) )
	{
		*pCount = 0;
		return JE_TRUE;
	}

	if ( ! jeVFile_Tell(File->BaseFile,(long *)&CurLen) )
		return JE_FALSE;

	File->AvailTime = CurTime;

	AllowLen = (int)((CurTime - File->OpenTime) * FakeNet_ByPerSec) - CurLen;

	if ( *pCount > AllowLen )
		*pCount = AllowLen;

return JE_TRUE;
}

static	jeBoolean	JETCC FSFakeNet_Read(void *Handle, char *Buff, uint32 Count)
{
FakeNetFile * File;
uint32 Avail;

	File = (FakeNetFile *)Handle;
	
	do
	{
		if ( ! FSFakeNet_BytesAvailable(Handle,&Avail) )
			return JE_FALSE;

		if ( Avail == 0 )
			jeThreadQueue_Sleep(10);
		else if ( Avail < Count )
		{
			if ( ! jeVFile_Read(File->BaseFile,Buff,Avail) )
				return JE_FALSE;
			Buff 		+= Avail;
			Count 		-= Avail;

			Avail = 0;

			if ( jeVFile_EOF(File->BaseFile) )
				return JE_FALSE;
		}

	} while( Avail < Count );

return jeVFile_Read(File->BaseFile,Buff,Count);
}

static	int JETCC FSFakeNet_GetC(FakeNetFile * File)
{
 char C;
	if ( ! FSFakeNet_Read(File,&C,1) )
		return -1;
return C;
}

static	jeBoolean	JETCC FSFakeNet_GetS(void *Handle, char *Buff, int MaxLen)
{
FakeNetFile *	File;
int C;
char * Ptr;

	File = (FakeNetFile *)Handle;

	Ptr = Buff;
	while( MaxLen > 1 && (C = FSFakeNet_GetC(File)) != -1 )
	{	
		*Ptr++ = C;
		MaxLen--;
		if (C == '\n' || C == '\r' || C == 0 )
			break;		
	}
	
	while( (C = FSFakeNet_GetC(File)) != -1 )
	{
		if (C == '\n' || C == '\r' || C == 0 )
			continue;

		if ( ! FSFakeNet_Seek(File,-1,JE_VFILE_SEEKCUR) )
			return JE_FALSE;
		break;
	}

	*Ptr = 0;

return JE_TRUE;
}

static	jeBoolean	JETCC FSFakeNet_GetProperties(const void *Handle, jeVFile_Properties *Properties)
{
const FakeNetFile * File;

	File = (FakeNetFile *)Handle;

	if ( ! jeVFile_GetProperties(File->BaseFile,Properties) )
		return JE_FALSE;
	
	Properties->AttributeFlags |= JE_VFILE_ATTRIB_REMOTE;

return JE_TRUE;
}

/*}{******************* Pass-Throughs ******************************/

static	jeBoolean	JETCC FSFakeNet_Write(void *Handle, const void *Buff, int Count)
{
FakeNetFile * File;
	File = (FakeNetFile *)Handle;

return jeVFile_Write(File->BaseFile,Buff,Count);
}

static	jeBoolean	JETCC FSFakeNet_Seek(void *Handle, int Where, jeVFile_Whence Whence)
{
FakeNetFile * File;
	File = (FakeNetFile *)Handle;

return jeVFile_Seek(File->BaseFile,Where,Whence);
}

static	jeBoolean	JETCC FSFakeNet_EOF(const void *Handle)
{
const FakeNetFile *	File;
	File = (FakeNetFile *)Handle;

return jeVFile_EOF(File->BaseFile);
}

static	jeBoolean	JETCC FSFakeNet_Tell(const void *Handle, long *pPosition)
{
const FakeNetFile *	File;
	File = (FakeNetFile *)Handle;

return jeVFile_Tell(File->BaseFile,pPosition);
}

static	jeBoolean	JETCC FSFakeNet_Size(const void *Handle, long *pSize)
{
const FakeNetFile *	File;
	File = (FakeNetFile *)Handle;

return jeVFile_Size(File->BaseFile,pSize);
}

static	jeVFile *	JETCC FSFakeNet_GetHintsFile(void *Handle)
{
FakeNetFile *	File;
	File = (FakeNetFile *)Handle;

return jeVFile_GetHintsFile(File->BaseFile);
}

/*}{******************* UnImplemented Bullshit ******************************/

static	void *	JETCC FSFakeNet_FinderCreate(
	jeVFile *			FS,
	void *			Handle,
	const char *	FileSpec)
{
	return NULL;
}

static	jeBoolean	JETCC FSFakeNet_FinderGetNextFile(void *Handle)
{
	assert(!Handle);
	return JE_FALSE;
}

static	jeBoolean	JETCC FSFakeNet_FinderGetProperties(void *Handle, jeVFile_Properties *Props)
{
	assert(!Handle);
	return JE_FALSE;
}

static	void JETCC FSFakeNet_FinderDestroy(void *Handle)
{
	assert(!Handle);
}

static	void *	JETCC FSFakeNet_Open(
	jeVFile *		FS,
	void *			Handle,
	const char *	Name,
	void *			Context,
	unsigned int	OpenModeFlags)
{
	return NULL;
}


static	jeBoolean	JETCC FSFakeNet_SetSize(void *Handle, long Size)
{
	assert(!"Not implemented");
	return JE_FALSE;
}

static	jeBoolean	JETCC FSFakeNet_SetAttributes(void *Handle, jeVFile_Attributes Attributes)
{
	assert(!"Not implemented");
	return JE_FALSE;
}

static	jeBoolean	JETCC FSFakeNet_SetTime(void *Handle, const jeVFile_Time *Time)
{
	assert(!"Not implemented");
	return JE_FALSE;
}

static	jeBoolean	JETCC FSFakeNet_FileExists(jeVFile *FS, void *Handle, const char *Name)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSFakeNet_Disperse(
	jeVFile *	FS,
	void *		Handle,
	const char *Directory)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSFakeNet_DeleteFile(jeVFile *FS, void *Handle, const char *Name)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSFakeNet_RenameFile(jeVFile *FS, void *Handle, const char *Name, const char *NewName)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSFakeNet_UpdateContext(
	jeVFile *		FS,
	void *			Handle,
	void *			Context,
	int 			ContextSize)
{
	return JE_FALSE;
}

/*}{******************* The FSFakeNet Struct ******************************/

#pragma warning (disable : 4113 4028)

static	jeVFile_SystemAPIs	FSFakeNet_APIs =
{
	FSFakeNet_FinderCreate,
	FSFakeNet_FinderGetNextFile,
	FSFakeNet_FinderGetProperties,
	FSFakeNet_FinderDestroy,

	FSFakeNet_OpenNewSystem,
	FSFakeNet_UpdateContext,
	FSFakeNet_Open,
	FSFakeNet_DeleteFile,
	FSFakeNet_RenameFile,
	FSFakeNet_FileExists,
	FSFakeNet_Disperse,
	FSFakeNet_Close,

	FSFakeNet_GetS,
	FSFakeNet_BytesAvailable,
	FSFakeNet_Read,
	FSFakeNet_Write,
	FSFakeNet_Seek,
	FSFakeNet_EOF,
	FSFakeNet_Tell,
	FSFakeNet_Size,

	FSFakeNet_GetProperties,

	FSFakeNet_SetSize,
	FSFakeNet_SetAttributes,
	FSFakeNet_SetTime,

	FSFakeNet_GetHintsFile,
};

const jeVFile_SystemAPIs * JETCC FSFakeNet_GetAPIs(void)
{
	return &FSFakeNet_APIs;
}

/*}{******************* EOF ******************************/
