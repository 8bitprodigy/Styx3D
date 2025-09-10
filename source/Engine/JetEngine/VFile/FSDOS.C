/****************************************************************************************/
/*  FSDOS.C                                                                             */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: DOS file system implementation                                         */
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

Jan/Feb 99 : cbloom : many hints-related bug-fixes

*******/

#include	<windows.h>

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>

#include	"basetype.h"
#include	"ram.h"

#include	"vfile.h"
#include	"vfile._h"

#include	"fsdos.h"

//	"DF01"
#define	DOSFILE_SIGNATURE	0x31304644

//	"DF02"
#define	DOSFINDER_SIGNATURE	0x32304644

#define	CHECK_HANDLE(H)	assert(H);assert(H->Signature == DOSFILE_SIGNATURE);
#define	CHECK_FINDER(F)	assert(F);assert(F->Signature == DOSFINDER_SIGNATURE);

typedef struct	DosFile
{
	unsigned int	Signature;
	HANDLE			FileHandle;
	char *			FullPath;
	const char *	Name;
	jeBoolean		IsDirectory;
	unsigned int	OpenFlags;
	jeBoolean		CanSetHints;
	jeVFile *		HintsFile;
	int				TrueFileBase;
}	DosFile;

typedef	struct	DosFinder
{
	unsigned int	Signature;
	HANDLE			FindHandle;
	WIN32_FIND_DATA	FindData;
	jeBoolean		FirstStillCached;
	int				OffsetToName;
}	DosFinder;

static	jeBoolean	BuildFileName(
	const DosFile *	File,
	const char *	Name,
	char *			Buff,
	char **			NamePtr,
	int 			MaxLen)
{
	int		DirLength;
	int		NameLength;

	if ( ! Name )
		return JE_FALSE;

	if	(File)
	{
		if	(File->IsDirectory == JE_FALSE)
			return JE_FALSE;

		assert(File->FullPath);
		DirLength = strlen(File->FullPath);

		if	(DirLength > MaxLen)
			return JE_FALSE;

		memcpy(Buff, File->FullPath, DirLength);
	}
	else
	{
		DirLength = 0;
	}

	NameLength = strlen(Name);
	if ( DirLength + NameLength + 2 > MaxLen || ! Buff )
		return JE_FALSE;

	if ( DirLength > 0 )
	{
		if ( Buff[DirLength-1] != '\\' )
		{
			Buff[DirLength] = '\\';
			DirLength++;
		}
		memcpy(Buff + DirLength, Name, NameLength + 1);
	}
	else
	{
//		Buff[0] = '.';
//		Buff[1] = '\\';
//		DirLength = 2;
		memcpy(Buff + DirLength, Name, NameLength + 1);

		// Special case: no directory, no file name.  We meant something like ".\"
		if	(!*Buff)
		{
			strcpy (Buff, ".\\");
			DirLength = 2;
		}
	}

	if	(NamePtr)
		*NamePtr = Buff + DirLength;

	NameLength = strlen(Buff);
	assert( NameLength > 0 );
	if ( Buff[NameLength-1] == '\\' )	
		Buff[NameLength-1] = 0;

	return JE_TRUE;
}

#pragma warning (disable:4100)
static	void *	JETCC FSDos_FinderCreate(
	jeVFile *			FS,
	void *			Handle,
	const char *	FileSpec)
{
	DosFinder *		Finder;
	DosFile *		File;
	char *			NamePtr;
	char			Buff[_MAX_PATH];

	assert(FileSpec != NULL);

	File = Handle;

	CHECK_HANDLE(File);

	Finder = jeRam_Allocate(sizeof(*Finder));
	if	(!Finder)
		return NULL;

	memset(Finder, 0, sizeof(*Finder));

	if	(BuildFileName(File, FileSpec, Buff, &NamePtr, sizeof(Buff)) == JE_FALSE)
	{
		jeRam_Free(Finder);
		return NULL;
	}

	Finder->OffsetToName = NamePtr - Buff;

	Finder->FindHandle = FindFirstFile(Buff, &Finder->FindData);

	Finder->FirstStillCached = JE_TRUE;

	Finder->Signature = DOSFINDER_SIGNATURE;
	return (void *)Finder;
}
#pragma warning (default:4100)

static	jeBoolean	JETCC FSDos_FinderGetNextFile(void *Handle)
{
	DosFinder *	Finder;

	Finder = Handle;

	CHECK_FINDER(Finder);

	if	(Finder->FindHandle == INVALID_HANDLE_VALUE)
		return JE_FALSE;

	if	(Finder->FirstStillCached == JE_TRUE)
	{
		Finder->FirstStillCached = JE_FALSE;

		if	(Finder->FindData.cFileName[0] != '.')
			return JE_TRUE;
	}
	
	while	(FindNextFile(Finder->FindHandle, &Finder->FindData) == TRUE)
	{
		if	(Finder->FindData.cFileName[0] != '.')
			return JE_TRUE;
	}

	return JE_FALSE;
}

static	jeBoolean	JETCC FSDos_FinderGetProperties(void *Handle, jeVFile_Properties *Props)
{
	DosFinder *			Finder;
	jeVFile_Attributes	Attribs;
	int					Length;

	assert(Props);

	Finder = Handle;

	CHECK_FINDER(Finder);

	if	(Finder->FindHandle == INVALID_HANDLE_VALUE)
		return JE_FALSE;

	Attribs = 0;
	if	(Finder->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		Attribs |= JE_VFILE_ATTRIB_DIRECTORY;
	if	(Finder->FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		Attribs |= JE_VFILE_ATTRIB_READONLY;

	Props->Time.Time1 = Finder->FindData.ftLastWriteTime.dwLowDateTime;
	Props->Time.Time2 = Finder->FindData.ftLastWriteTime.dwHighDateTime;

	Props->AttributeFlags = Attribs;
	Props->Size = Finder->FindData.nFileSizeLow;

	Length = strlen(Finder->FindData.cFileName);
	if	(Length > sizeof(Props->Name) - 1)
		return JE_FALSE;
	memcpy(Props->Name, Finder->FindData.cFileName, Length + 1);

	return JE_TRUE;
}

static	void JETCC FSDos_FinderDestroy(void *Handle)
{
	DosFinder *	Finder;

	Finder = Handle;

	CHECK_FINDER(Finder);

	if	(Finder->FindHandle != INVALID_HANDLE_VALUE)
		FindClose(Finder->FindHandle);

	Finder->Signature = 0;
	jeRam_Free(Finder);
}

static	jeBoolean	IsRootDirectory(char *Path)
{
	int		SlashCount;

	// Drive letter test
	if	(Path[1] == ':' && Path[2] == '\\' && Path[3] == '\0')
	{
		Path[2] = '\0';
		return JE_TRUE;
	}


// Added JH 20.3.2000	( e.g. "H:" is also root dir) 
	// Drive letter test
	if	(Path[1] == ':'  && Path[2] == '\0')
	{
		return JE_TRUE;
	}
// EOF JH	
	
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
		return JE_TRUE;

	return JE_FALSE;
}

#pragma warning (disable:4100)
static	void *	JETCC FSDos_Open(
	jeVFile *		FS,
	void *			Handle,
	const char *	Name,
	void *			Context,
	unsigned int 	OpenModeFlags)
{
	DosFile *	DosFS;
	DosFile *	NewFile;
	char		Buff[_MAX_PATH];
	int			Length;
	char *		NamePtr;

	DosFS = Handle;

	if	(DosFS && DosFS->IsDirectory != JE_TRUE)
		return NULL;

	NewFile = jeRam_Allocate(sizeof(*NewFile));
	if	(!NewFile)
		return NewFile;

	memset(NewFile, 0, sizeof(*NewFile));

	if	(BuildFileName(DosFS, Name, Buff, &NamePtr, sizeof(Buff)) == JE_FALSE)
		goto fail;

	Length = strlen(Buff);
	NewFile->FullPath = jeRam_Allocate(Length + 1);
	if	(!NewFile->FullPath)
		goto fail;

	NewFile->Name = NewFile->FullPath + (NamePtr - &Buff[0]);

	memcpy(NewFile->FullPath, Buff, Length + 1);

	if	(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY)
	{
		WIN32_FIND_DATA	FileInfo;
		HANDLE			FindHandle;
		jeBoolean		IsDirectory;

		assert(!DosFS || DosFS->IsDirectory == JE_TRUE);

		memset(&FileInfo, 0, sizeof(FileInfo));
		FindHandle = FindFirstFile(NewFile->FullPath, &FileInfo);
		if	(FindHandle != INVALID_HANDLE_VALUE &&
			 FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			IsDirectory = JE_TRUE;
		}
		else
		{
			IsDirectory = IsRootDirectory(NewFile->FullPath);
		}
		FindClose (FindHandle);

		if	(OpenModeFlags & JE_VFILE_OPEN_CREATE)
		{
			if	( ! IsDirectory )
				if	(CreateDirectory(NewFile->FullPath, NULL) != TRUE)
					goto fail;
		}
		else
		{
			if	(IsDirectory != JE_TRUE)
				goto fail;
		}

		NewFile->IsDirectory = JE_TRUE;
		NewFile->FileHandle = INVALID_HANDLE_VALUE;
	}
	else
	{
		DWORD	ShareMode = FILE_SHARE_READ;
		DWORD	CreationMode;
		DWORD	Access = GENERIC_READ;

		CreationMode = OPEN_EXISTING;

		switch	(OpenModeFlags & (JE_VFILE_OPEN_READONLY |
								  JE_VFILE_OPEN_UPDATE	 |
								  JE_VFILE_OPEN_CREATE))
		{
		case	JE_VFILE_OPEN_READONLY:
			Access = GENERIC_READ;
			ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
			break;

		case	JE_VFILE_OPEN_CREATE:
			CreationMode = CREATE_ALWAYS;
			// Fall through

		case	JE_VFILE_OPEN_UPDATE:
			Access = GENERIC_READ | GENERIC_WRITE;
			ShareMode = FILE_SHARE_READ;
			break;

		default:
			assert(!"Illegal open mode flags");
			break;
		}

		NewFile->FileHandle = CreateFile(NewFile->FullPath,
										 Access,
										 ShareMode,
										 NULL,
										 CreationMode,
										 0,
										 NULL);
		if	(NewFile->FileHandle == INVALID_HANDLE_VALUE)
		{
//			LastError = GetLastError();
			goto fail;
		}

		/*
			Now we have to go looking in the file to see if it has hint data
			that we need to encapsulate.
		*/
		if	( (OpenModeFlags & (JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_UPDATE)) &&
			 !(OpenModeFlags & (JE_VFILE_OPEN_RAW)))
		{
			DWORD					BytesRead;
			jeVFile_HintsFileHeader	HintsHeader;
			
			ReadFile(NewFile->FileHandle, &HintsHeader, sizeof(HintsHeader), &BytesRead, NULL);
			if	(BytesRead == sizeof(HintsHeader) &&
				 HintsHeader.Signature == JE_VFILE_HINTSFILEHEADER_SIGNATURE)
			{
				/*
					Allocate the hint data, read the data from disk, and set the true file
					base to the resulting file offset.  This will allow us to adjust file
					offsets that the client gets from the other APIs to ignore the hint
					data.
				*/
				jeVFile_MemoryContext	MemoryContext;

				MemoryContext.Data = NULL;
				MemoryContext.DataLength = 0;
				NewFile->HintsFile = jeVFile_OpenNewSystem(NULL,
														   JE_VFILE_TYPE_MEMORY,
														   NULL,
														   &MemoryContext,
														   JE_VFILE_OPEN_CREATE);
				if	(!NewFile->HintsFile)
					goto fail;

				if	(jeVFile_Seek(NewFile->HintsFile, HintsHeader.HintDataLength, JE_VFILE_SEEKSET) == JE_FALSE)
					goto fail;

				jeVFile_UpdateContext(NewFile->HintsFile, &MemoryContext, sizeof(MemoryContext));
				ReadFile(NewFile->FileHandle, MemoryContext.Data, HintsHeader.HintDataLength, &BytesRead, NULL);
				if	(BytesRead != HintsHeader.HintDataLength)
					goto fail;

				jeVFile_Seek(NewFile->HintsFile, 0, JE_VFILE_SEEKSET);
				NewFile->TrueFileBase = SetFilePointer(NewFile->FileHandle, 0, NULL, FILE_CURRENT);
			}
			else
			{
				if	(SetFilePointer(NewFile->FileHandle, 0, NULL, FILE_BEGIN) == (DWORD)-1)
					goto fail;
				NewFile->CanSetHints = JE_TRUE;
			}
		}
		else
		{
			NewFile->CanSetHints = JE_TRUE;
		}
	}

	NewFile->OpenFlags = OpenModeFlags;
	NewFile->Signature = DOSFILE_SIGNATURE;

	return (void *)NewFile;

fail:
	if	(NewFile->HintsFile)
		jeVFile_Close(NewFile->HintsFile);
	if	(NewFile->FullPath)
		jeRam_Free(NewFile->FullPath);
	jeRam_Free(NewFile);
	return NULL;
}
#pragma warning (default:4100)

static	void *	JETCC FSDos_OpenNewSystem(
	jeVFile *			FS,
	const char *	Name,
	void *			Context,
	unsigned int 	OpenModeFlags)
{
	return FSDos_Open(FS, NULL, Name, Context, OpenModeFlags);
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSDos_UpdateContext(
	jeVFile *		FS,
	void *			Handle,
	void *			Context,
	int 			ContextSize)
{
	return JE_FALSE;
}
#pragma warning (default:4100)

static	jeBoolean	JETCC FSDos_Close(void *Handle)
{
	jeBoolean	Result;

	DosFile *	File;
	
	File = Handle;
	
	CHECK_HANDLE(File);

	Result = JE_TRUE;
	if	(File->IsDirectory == JE_FALSE)
	{
		assert(File->FileHandle != INVALID_HANDLE_VALUE);

		if	(File->HintsFile && (File->CanSetHints == JE_TRUE))
		{
			void *	CopyBuff;
			DWORD	FileSize;
			/*
				Oh boy.  We have to shuffle all of the data in the file
				down and insert the hints data.  Lots of fun.  For now,
				we're going to be really really obtuse about this, and
				just allocate a big block and copy.
			*/

			SetFilePointer(File->FileHandle, 0, NULL, FILE_END);
			FileSize = SetFilePointer(File->FileHandle, 0, NULL, FILE_CURRENT);
			CopyBuff = jeRam_Allocate(FileSize);
			if	(CopyBuff)
			{
				DWORD	BytesRead;

				Result = JE_FALSE;
				SetFilePointer(File->FileHandle, 0, NULL, FILE_BEGIN);
				ReadFile(File->FileHandle, CopyBuff, FileSize, &BytesRead, NULL);
				if	(BytesRead == FileSize)
				{
					DWORD					BytesWritten;
					jeVFile_MemoryContext	MemoryContext;
					jeVFile_HintsFileHeader	HintsHeader;

		#ifdef COUNT_HEADER_SIZES
					Header_Sizes += sizeof(HintsHeader);
		#endif

					SetFilePointer(File->FileHandle, 0, NULL, FILE_BEGIN);
					jeVFile_UpdateContext(File->HintsFile, &MemoryContext, sizeof(MemoryContext));
					HintsHeader.Signature = JE_VFILE_HINTSFILEHEADER_SIGNATURE;
					HintsHeader.HintDataLength = MemoryContext.DataLength;
					WriteFile(File->FileHandle,
							  &HintsHeader, 
							  sizeof(HintsHeader),
							  &BytesWritten,
							  NULL);
					if	(BytesWritten == sizeof(HintsHeader))
					{
						WriteFile(File->FileHandle,
								  MemoryContext.Data, 
								  MemoryContext.DataLength,
								  &BytesWritten,
								  NULL);
						if	(BytesWritten == (uint32)MemoryContext.DataLength)
						{
							WriteFile(File->FileHandle, CopyBuff, FileSize, &BytesWritten, NULL);
							if	(BytesWritten == FileSize)
							{
								// That was it, we made it!
								Result = JE_TRUE;
							}
						}
					}
				}

				jeRam_Free(CopyBuff);
			}
		}

		if ( File->HintsFile ) // <> CB 2/10
			jeVFile_Close(File->HintsFile);

		CloseHandle(File->FileHandle);
	}

	assert(File->FullPath);
	File->Signature = 0;

	jeRam_Free(File->FullPath);
	jeRam_Free(File);

	return Result;
}

static	jeBoolean	JETCC FSDos_GetS(void *Handle, void *Buff, int MaxLen)
{
	DosFile *	File;
	DWORD		BytesRead;
//	BOOL		Result;
	char *		p;
	char *		End;

	assert(Buff);
	assert(MaxLen != 0);

	File = Handle;

	CHECK_HANDLE(File);

	assert(File->FileHandle != INVALID_HANDLE_VALUE);

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;

//	Result = ReadFile(File->FileHandle, Buff, MaxLen - 1, &BytesRead, NULL);
	ReadFile(File->FileHandle, Buff, MaxLen - 1, &BytesRead, NULL);
	if	(BytesRead == 0)
	{
#if 0
		if	(Result == FALSE)
			return JE_FALSE;
		
		// The Win32 API is vague about this, so we're being weird with the asserts
		assert(Result != TRUE);
#endif
		return JE_FALSE;
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
			SetFilePointer(File->FileHandle, -(int)(BytesRead - ((p + Skip) - (char *)Buff)), NULL, FILE_CURRENT); 
			assert(p - (char *)Buff <= MaxLen);
			return JE_TRUE;
		}
		else if	(*p == '\n')
		{
			// Set the file pointer back a bit since we probably overran
			p++;
			SetFilePointer(File->FileHandle, -(int)(BytesRead - (p - (char *)Buff)), NULL, FILE_CURRENT); 
			*p = '\0';
			assert(p - (char *)Buff <= MaxLen);
			return JE_TRUE;
		}
		p++;
	}

	return JE_FALSE;
}


static	jeBoolean	JETCC FSDos_Tell(const void *Handle, long *Position)
{
	const DosFile *	File;

	File = Handle;

	CHECK_HANDLE(File);

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;

	assert(File->FileHandle != INVALID_HANDLE_VALUE);

	*Position = SetFilePointer(File->FileHandle, 0, NULL, FILE_CURRENT);
	if	(*Position == -1L)
		return JE_FALSE;

	*Position -= File->TrueFileBase; 

	return JE_TRUE;
}

static	jeBoolean	JETCC FSDos_BytesAvailable(void *Handle, long *Count)
{
	DosFile *	File;
	long		CurrentPos;
	long		EndPos;

	File = Handle;

	CHECK_HANDLE(File);

	assert(Count);

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;

	assert(File->FileHandle != INVALID_HANDLE_VALUE);

	CurrentPos = SetFilePointer(File->FileHandle, 0, NULL, FILE_CURRENT);
	if	(CurrentPos == -1L)
		return JE_FALSE;

	EndPos = SetFilePointer(File->FileHandle, 0, NULL, FILE_END);
	if	(EndPos == -1L)
		return JE_FALSE;

	*Count = EndPos - CurrentPos;	

	CurrentPos = SetFilePointer(File->FileHandle, CurrentPos, NULL, FILE_BEGIN);
	if	(CurrentPos == -1L)
		return JE_FALSE;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSDos_Read(void *Handle, void *Buff, uint32 Count)
{
	DosFile *	File;
	DWORD		BytesRead;

	assert(Buff);
	assert(Count != 0);

	File = Handle;

	CHECK_HANDLE(File);

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;

#ifdef	KROUERDEBUG
{
	FILE *	fp;
	long	Position;

	Position = SetFilePointer(File->FileHandle, 0, NULL, FILE_CURRENT);
	fp = fopen("c:\\vfs.eli", "ab+");
	fprintf(fp, "FSDos_Read: %-8d bytes @ %-8d\r\n", Count, Position);
	fclose(fp);
}
#endif

	if	(ReadFile(File->FileHandle, Buff, Count, &BytesRead, NULL) == FALSE)
		return JE_FALSE;

	// CB added :
	if	(BytesRead == 0)
		return JE_FALSE;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSDos_Write(void *Handle, const void *Buff, int Count)
{
	DosFile *	File;
	DWORD		BytesWritten;

	assert(Buff);
	assert(Count != 0);

	File = Handle;

	CHECK_HANDLE(File);

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;

#ifdef	KROUERDEBUG
{
	FILE *	fp;
	long	Position;

	Position = SetFilePointer(File->FileHandle, 0, NULL, FILE_CURRENT);
	fp = fopen("c:\\vfs.eli", "ab+");
	fprintf(fp, "FSDos_Write: %-8d bytes @ %-8d\r\n", Count, Position);
	fclose(fp);
}
#endif

	if	(WriteFile(File->FileHandle, Buff, Count, &BytesWritten, NULL) == FALSE)
		return JE_FALSE;

	if ( (int)BytesWritten != Count )
		return JE_FALSE;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSDos_Seek(void *Handle, int Where, jeVFile_Whence Whence)
{
	int			RTLWhence = FILE_BEGIN;
	DosFile *	File;

	File = Handle;

	CHECK_HANDLE(File);

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;

	switch	(Whence)
	{
	case	JE_VFILE_SEEKCUR:
		RTLWhence = FILE_CURRENT;
		break;

	case	JE_VFILE_SEEKEND:
		RTLWhence = FILE_END;
		break;

	case	JE_VFILE_SEEKSET:
		RTLWhence = FILE_BEGIN;
		Where += File->TrueFileBase;
		break;
	default:
		assert(!"Unknown seek kind");
	}

	if	(SetFilePointer(File->FileHandle, Where, NULL, RTLWhence) == 0xffffffff)
		return JE_FALSE;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSDos_EOF(const void *Handle)
{
	const DosFile *	File;
	DWORD			CurPos;

	File = Handle;

	CHECK_HANDLE(File);

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;

	assert(File->FileHandle != INVALID_HANDLE_VALUE);

	CurPos = SetFilePointer(File->FileHandle, 0, NULL, FILE_CURRENT);
	assert(CurPos != 0xffffffff);

	if	(CurPos == GetFileSize(File->FileHandle, NULL))
		return JE_TRUE;

	return JE_FALSE;
}

static	jeBoolean	JETCC FSDos_Size(const void *Handle, long *Size)
{
	const DosFile *	File;

	File = Handle;

	CHECK_HANDLE(File);

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;

	assert(File->FileHandle != INVALID_HANDLE_VALUE);

	*Size = GetFileSize(File->FileHandle, NULL);
	if	(*Size == (uint32)0xffffffff)
		return JE_FALSE;

	*Size -= File->TrueFileBase; 

	return JE_TRUE;
}

static	jeBoolean	JETCC FSDos_GetProperties(const void *Handle, jeVFile_Properties *Properties)
{
	const DosFile *				File;
	jeVFile_Attributes			Attribs;
	BY_HANDLE_FILE_INFORMATION	Info;
	int							Length;

	assert(Properties);

	File = Handle;

	CHECK_HANDLE(File);

	if	(File->IsDirectory == JE_TRUE)
	{
		memset(Properties, 0, sizeof(*Properties));
		Properties->AttributeFlags = JE_VFILE_ATTRIB_DIRECTORY; // <> CB 2/10
#pragma message ("FSDos_GetProperties: Time support is not there for directories")
	}
	else
	{
		assert(File->FileHandle != INVALID_HANDLE_VALUE);
	
		if	(GetFileInformationByHandle(File->FileHandle, &Info) == FALSE)
			return JE_FALSE;
	
		Attribs = 0;
		if	(Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			Attribs |= JE_VFILE_ATTRIB_DIRECTORY;
		if	(Info.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			Attribs |= JE_VFILE_ATTRIB_READONLY;
	
		Properties->Time.Time1 = Info.ftLastWriteTime.dwLowDateTime;
		Properties->Time.Time2 = Info.ftLastWriteTime.dwHighDateTime;
	
		Properties->AttributeFlags 		 = Attribs;
		Properties->Size		  		 = Info.nFileSizeLow;
	}

	Length = strlen(File->Name) + 1;
	if	(Length > sizeof(Properties->Name))
		return JE_FALSE;
	memcpy(Properties->Name, File->Name, Length);

	return JE_TRUE;
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSDos_SetSize(void *Handle, long size)
{
	DosFile *	File;

	File = Handle;

	CHECK_HANDLE(File);

	if	(File->IsDirectory == JE_FALSE)
	{
		assert(File->FileHandle != INVALID_HANDLE_VALUE);
	
		if	(SetFilePointer(File->FileHandle, 0, NULL, FILE_END) == 0xffffffff)
			return JE_FALSE;
	
		if	(SetEndOfFile(File->FileHandle) == FALSE)
			return JE_FALSE;
	}

	return JE_FALSE;
}
#pragma warning (default:4100)

static	jeBoolean	JETCC FSDos_SetAttributes(void *Handle, jeVFile_Attributes Attributes)
{
	DosFile *	File;
	DWORD		Win32Attributes;

	File = Handle;

	CHECK_HANDLE(File);

	assert(File->FileHandle != INVALID_HANDLE_VALUE);

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;

	if	(Attributes & JE_VFILE_ATTRIB_READONLY)
		Win32Attributes = FILE_ATTRIBUTE_READONLY;
	else
		Win32Attributes = FILE_ATTRIBUTE_NORMAL;

	if	(SetFileAttributes(File->FullPath, Win32Attributes) == FALSE)
		return JE_FALSE;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSDos_SetTime(void *Handle, const jeVFile_Time *Time)
{
	DosFile *	File;
	FILETIME	Win32Time;

	File = Handle;

	CHECK_HANDLE(File);

	assert(File->FileHandle != INVALID_HANDLE_VALUE);

	Win32Time.dwLowDateTime  = Time->Time1;
	Win32Time.dwHighDateTime = Time->Time2;
	if	(SetFileTime(File->FileHandle, &Win32Time, &Win32Time, &Win32Time) == FALSE)
		return JE_FALSE;

	return JE_TRUE;
}

static	jeVFile *	JETCC FSDos_GetHintsFile(void *Handle)
{
	DosFile *	File;

	File = Handle;

	CHECK_HANDLE(File);

	if	(File->OpenFlags & JE_VFILE_OPEN_RAW)
		return NULL;

	if	(!File->HintsFile)
	{
		jeVFile_MemoryContext	Context;

		// Can't create hints on a readonly file
		if	(File->OpenFlags & JE_VFILE_OPEN_READONLY)
			return NULL;

		Context.Data = NULL;
		Context.DataLength = 0;
		File->HintsFile = jeVFile_OpenNewSystem(NULL,
												JE_VFILE_TYPE_MEMORY,
												NULL,
												&Context,
												JE_VFILE_OPEN_CREATE);
	}

	return File->HintsFile;
}

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSDos_FileExists(jeVFile *FS, void *Handle, const char *Name)
{
	DosFile *	File;
	char		Buff[_MAX_PATH];

	File = Handle;

	if	(File && File->IsDirectory == JE_FALSE)
		return JE_FALSE;

	if	(BuildFileName(File, Name, Buff, NULL, sizeof(Buff)) == JE_FALSE)
		return JE_FALSE;

	if	(GetFileAttributes(Buff) == 0xffffffff)
		return JE_FALSE;

	return JE_TRUE;
}
#pragma warning (default:4100)

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSDos_Disperse(
	jeVFile *	FS,
	void *		Handle,
	const char *Directory)
{
	return JE_FALSE;
}
#pragma warning (default:4100)

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSDos_DeleteFile(jeVFile *FS, void *Handle, const char *Name)
{
	DosFile *	File;
	char		Buff[_MAX_PATH];

	File = Handle;

	if	(File && File->IsDirectory == JE_FALSE)
		return JE_FALSE;

	if	(BuildFileName(File, Name, Buff, NULL, sizeof(Buff)) == JE_FALSE)
		return JE_FALSE;

	if	(DeleteFile(Buff) == FALSE)
		return JE_FALSE;

	return JE_TRUE;
}
#pragma warning (default:4100)

#pragma warning (disable:4100)
static	jeBoolean	JETCC FSDos_RenameFile(jeVFile *FS, void *Handle, const char *Name, const char *NewName)
{
	DosFile *	File;
	char		Old[_MAX_PATH];
	char		New[_MAX_PATH];

	File = Handle;

	if	(File && File->IsDirectory == JE_FALSE)
		return JE_FALSE;

	if	(BuildFileName(File, Name, Old, NULL, sizeof(Old)) == JE_FALSE)
		return JE_FALSE;

	if	(BuildFileName(File, NewName, New, NULL, sizeof(New)) == JE_FALSE)
		return JE_FALSE;

	if	(MoveFile(Old, New) == FALSE)
		return JE_FALSE;

	return JE_TRUE;
}
#pragma warning (default:4100)

static	jeVFile_SystemAPIs	FSDos_APIs =
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
	FSDos_BytesAvailable,
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

	FSDos_GetHintsFile,

};

const jeVFile_SystemAPIs *JETCC FSDos_GetAPIs(void)
{
	return &FSDos_APIs;
}
