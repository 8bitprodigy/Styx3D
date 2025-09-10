/****************************************************************************************/
/*  DIRTREE.C                                                                           */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Directory tree implementation                                          */
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

Jan/Feb 99 : cbloom :
	DirTree now does hints (correctly?)
	DirTree read/write through LZ packer

*******/

#define DO_LZ	// <> LZ turned off for debug

#include	<assert.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include 	<limits.h>

#include	"BaseType.h"
#include	"Ram.h"
#include	"Errorlog.h"

#include	"DirTree.h"

// Note: These file constants should be put into a OS/Storage.h.. header
#ifdef BUILD_BE
#include <StorageDefs.h>
#define _MAX_PATH MAXPATHLEN
#define _MAX_FNAME NAME_MAX
#define _MAX_EXT 256 //?
#define _MAX_DRIVE  3
#define _MAX_DIR    256 /* max. length of path component */

#define stricmp strcasecmp
void FastSplitPath(register char* path, char* drive, char* dir, char* fname, char * ext);
#define _splitpath FastSplitPath        
#endif

#ifndef	MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((unsigned long)(unsigned char)(ch0) | ((unsigned long)(unsigned char)(ch1) << 8) |   \
		((unsigned long)(unsigned char)(ch2) << 16) | ((unsigned long)(unsigned char)(ch3) << 24 ))
#endif

#define	DIRTREE_FILE_SIGNATURE		MAKEFOURCC('D', 'T', '0', '1')	//	0x31305444
#define	DIRTREE_LIST_TERMINATED		MAKEFOURCC('D', 'T', '0', '2')	//	0x32305444
#define DIRTREE_LIST_NOTTERMINATED	MAKEFOURCC('D', 'T', '0', '3')	//	0x33305444

typedef struct	DirTree
{
	char *				Name;
	jeVFile_Time		Time;
	jeVFile_Attributes	AttributeFlags;
	long				Size;
	jeVFile_Hints		Hints;
	jeVFile *			HintsFile;
	long				Offset;
	struct DirTree *	Parent;
	struct DirTree *	Children;
	struct DirTree *	Siblings;
}	DirTree;

typedef struct	DirTree_Finder
{
	char *		MatchName;
	char *		MatchExt;
	DirTree *	Current;
}	DirTree_Finder;

static	char *	DuplicateString(const char *String)
{
	int		Length;
	char *	NewString;

	Length = strlen(String) + 1;
	NewString = (char *)jeRam_Allocate(Length);
	if	(NewString)
		memcpy(NewString, String, Length);
	return NewString;
}

DirTree *DirTree_Create(void)
{
	DirTree *	Tree;

	Tree = (DirTree *)jeRam_Allocate(sizeof(*Tree));
	if	(!Tree)
		return Tree;

	memset(Tree, 0, sizeof(*Tree));
	Tree->Name = DuplicateString("");
	if	(!Tree->Name)
	{
		jeRam_Free(Tree);
		return NULL;
	}

	Tree->AttributeFlags |= JE_VFILE_ATTRIB_DIRECTORY;

	return Tree;
}

void	DirTree_Destroy(DirTree *Tree)
{
	if ( ! Tree )
		return;

	if	(Tree->Children)
		DirTree_Destroy(Tree->Children);

	if	(Tree->Siblings)
		DirTree_Destroy(Tree->Siblings);

	if	(Tree->HintsFile)
		jeVFile_Close(Tree->HintsFile);

	if ( Tree->Name )
		jeRam_Free(Tree->Name);

	if ( Tree->Hints.HintData != NULL)
		jeRam_Free(Tree->Hints.HintData);

	jeRam_Free(Tree);
}

typedef	struct	DirTree_Header
{
	unsigned long	Signature;
	uint32	ArchaicIgnored;
}	DirTree_Header;

static	jeBoolean	WriteTree(const DirTree *Tree, jeVFile *File)
{
	int		Length;
	int		Terminator;

	assert(Tree);
	assert(Tree->Name);

	Terminator = DIRTREE_LIST_NOTTERMINATED;
	if	(jeVFile_Write(File, &Terminator, sizeof(Terminator)) == JE_FALSE)
		return JE_FALSE;

	// Write out the name
	Length = strlen(Tree->Name) + 1;
	if	(jeVFile_Write(File, &Length, sizeof(Length)) == JE_FALSE)
		return JE_FALSE;
	if	(Length > 0)
	{
		if	(jeVFile_Write(File, Tree->Name, Length) == JE_FALSE)
			return JE_FALSE;
	}

	// Write out the attribute information
	if	(jeVFile_Write(File, &Tree->Time, sizeof(Tree->Time)) == JE_FALSE)
		return JE_FALSE;

	if	(jeVFile_Write(File, &Tree->AttributeFlags, sizeof(Tree->AttributeFlags)) == JE_FALSE)
		return JE_FALSE;

	if	(jeVFile_Write(File, &Tree->Size, sizeof(Tree->Size)) == JE_FALSE)
		return JE_FALSE;

	if	(jeVFile_Write(File, &Tree->Offset, sizeof(Tree->Offset)) == JE_FALSE)
		return JE_FALSE;
	
	if	(Tree->HintsFile)
	{
		jeVFile_MemoryContext	MemoryContext;

		jeVFile_UpdateContext(Tree->HintsFile, &MemoryContext, sizeof(MemoryContext));
		if	(jeVFile_Write(File, &MemoryContext.DataLength, sizeof(Tree->Hints.HintDataLength)) == JE_FALSE)
			return JE_FALSE;
		if ( MemoryContext.DataLength != 0 )
		{
			if	(jeVFile_Write(File, MemoryContext.Data, MemoryContext.DataLength) == JE_FALSE)
				return JE_FALSE;
		}
	}
	else
	{
		// <> CB 2/10
//		assert(Tree->Hints.HintDataLength == 0);
		assert(!Tree->HintsFile);
		if	(jeVFile_Write(File, &(Tree->Hints.HintDataLength), sizeof(Tree->Hints.HintDataLength)) == JE_FALSE)
			return JE_FALSE;
		if	(jeVFile_Write(File, Tree->Hints.HintData, Tree->Hints.HintDataLength) == JE_FALSE)
			return JE_FALSE;
	}
	
	// Write out the Children
	if	(Tree->Children)
	{
		WriteTree(Tree->Children, File);
	}
	else
	{
		Terminator = DIRTREE_LIST_TERMINATED;
		if	(jeVFile_Write(File, &Terminator, sizeof(Terminator)) == JE_FALSE)
			return JE_FALSE;
	}

	// Write out the Siblings
	if	(Tree->Siblings)
	{
		WriteTree(Tree->Siblings, File);
	}
	else
	{
		Terminator = DIRTREE_LIST_TERMINATED;
		if	(jeVFile_Write(File, &Terminator, sizeof(Terminator)) == JE_FALSE)
			return JE_FALSE;
	}
	
	return JE_TRUE;
}

static	jeBoolean DirTree_WriteToFile1(const DirTree *Tree, jeVFile *File, long *Size)
{
DirTree_Header	Header;
long			StartPosition;
long			EndPosition;
jeVFile * LZFS;
	
	Header.Signature = DIRTREE_FILE_SIGNATURE;

	jeVFile_Tell(File,&StartPosition);

#ifdef DO_LZ
	if ( ! (LZFS = jeVFile_OpenNewSystem(File,JE_VFILE_TYPE_LZ, NULL, NULL,JE_VFILE_OPEN_CREATE) ))
		return JE_FALSE;
#else
	LZFS = File;
#endif

	if	(jeVFile_Write(LZFS, &Header, sizeof(Header)) == JE_FALSE)
		return JE_FALSE;
			
	if	(WriteTree(Tree, LZFS) == JE_FALSE)
		return JE_FALSE;

#ifdef DO_LZ
	if ( ! jeVFile_Close(LZFS) )
		return JE_FALSE;
#endif

	jeVFile_Tell(File,&EndPosition);

	*Size = EndPosition - StartPosition;

	return JE_TRUE;
}

void DirTree_SetFileSize(DirTree *Tree, long Size)
{
	assert(Tree);
	Tree->Size = Size;
}

void DirTree_GetFileSize(DirTree *Tree, long *Size)
{
	assert(Tree);
	*Size = Tree->Size;
}

jeBoolean DirTree_WriteToFile(const DirTree *Tree, jeVFile *File)
{
long Size;

return DirTree_WriteToFile1(Tree, File, &Size);
}

jeBoolean DirTree_GetSize(const DirTree *Tree, long *Size)
{
	jeVFile *				FS;
	jeVFile_MemoryContext	Context;

	/*
		This function is implemented via a write to a memory file for
		a few reasons.  First, it makes it easier to maintain this code.  We
		don't have to track format information in Write, Read and Size functions,
		just in Write and Read.  Second, it gets us testing of the memory
		file system for free.  Third, it was cute.  The last one doesn't count,
		of course, but the other two are compelling.  This API ends up being
		inefficient, but the assumption is that it will be called rarely.
	*/

	Context.Data	   = NULL;
	Context.DataLength = 0;

	FS = jeVFile_OpenNewSystem(NULL,
							 JE_VFILE_TYPE_MEMORY,
							 NULL,
							 &Context,
							 JE_VFILE_OPEN_CREATE);
	if	(!FS)
		return JE_FALSE;

	if	(DirTree_WriteToFile1(Tree, FS, Size) == JE_FALSE)
		return JE_FALSE;

	if	(jeVFile_Size(FS, Size) == JE_FALSE)
		return JE_FALSE;

	jeVFile_Close(FS);

	return JE_TRUE;
}

jeBoolean DirTree_OpenFile(DirTree * Tree,uint32 OpenFlags)
{
	assert(Tree);

	if ( (Tree->AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY) )
	{
		if ( ! (OpenFlags & JE_VFILE_OPEN_DIRECTORY ) )
			return JE_FALSE;
	}
	else
	{
		if ( (OpenFlags & JE_VFILE_OPEN_DIRECTORY ) )
			return JE_FALSE;
	}

	if ( Tree->HintsFile ) // <> CB 2/10
	{
		if ( ! jeVFile_Seek(Tree->HintsFile,0,JE_VFILE_SEEKSET) )
			return JE_FALSE;
	}

return JE_TRUE;
}

static	jeBoolean	ReadTree(jeVFile *File, DirTree **TreePtr)
{
int			Terminator;
int			Length;
DirTree *	Tree = NULL;

	if	(jeVFile_Read(File, &Terminator, sizeof(Terminator)) == JE_FALSE)
	{
		jeErrorLog_AddString(-1,"ReadTree : VF_Read Name",NULL);
		goto fail;
	}

	if	(Terminator == DIRTREE_LIST_TERMINATED)
	{
		*TreePtr = NULL;
		return JE_TRUE;
	}

	if	(Terminator != DIRTREE_LIST_NOTTERMINATED)
	{
		jeErrorLog_AddString(-1,"ReadTree : DirTree : garbled Terminator!",NULL);
		goto fail;
	}

	Tree = (DirTree *)jeRam_AllocateClear(sizeof(*Tree));
	if	(!Tree)
	{
		jeErrorLog_AddString(-1,"ReadTree : Ram",NULL);
		goto fail;
	}

	// Read the name
	if	(jeVFile_Read(File, &Length, sizeof(Length)) == JE_FALSE)
	{
		jeErrorLog_AddString(-1,"ReadTree : VF_Read Name",NULL);
		goto fail;
	}

	assert(Length > 0 && Length <= (_MAX_PATH + _MAX_PATH));
	Tree->Name = (char *)jeRam_Allocate(Length+1);
	if	(!Tree->Name)
	{
		jeErrorLog_AddString(-1,"ReadTree : Ram",NULL);
		goto fail;
	}
	
	if	(jeVFile_Read(File, Tree->Name, Length) == JE_FALSE)
	{
		jeErrorLog_AddString(-1,"ReadTree : VF_Read Name",NULL);
		goto fail;
	}

	Tree->Name[Length] = 0;

//printf("Reading '%s'\n", Tree->Name);

	// Read out the attribute information
	if	(jeVFile_Read(File, &Tree->Time, sizeof(Tree->Time)) == JE_FALSE)
	{
		jeErrorLog_AddString(-1,"ReadTree : VF_Read",Tree->Name);
		goto fail;
	}

	if	(jeVFile_Read(File, &Tree->AttributeFlags, sizeof(Tree->AttributeFlags)) == JE_FALSE)	
	{
		jeErrorLog_AddString(-1,"ReadTree : VF_Read",Tree->Name);
		goto fail;
	}

	if	(jeVFile_Read(File, &Tree->Size, sizeof(Tree->Size)) == JE_FALSE)	
	{
		jeErrorLog_AddString(-1,"ReadTree : VF_Read",Tree->Name);
		goto fail;
	}

	if	(jeVFile_Read(File, &Tree->Offset, sizeof(Tree->Offset)) == JE_FALSE)
	{
		jeErrorLog_AddString(-1,"ReadTree : VF_Read",Tree->Name);
		goto fail;
	}

	if	(jeVFile_Read(File, &Tree->Hints.HintDataLength, sizeof(Tree->Hints.HintDataLength)) == JE_FALSE)
	{
		jeErrorLog_AddString(-1,"ReadTree : VF_Read",Tree->Name);
		goto fail;
	}

	if	(Tree->Hints.HintDataLength != 0)
	{
		Tree->Hints.HintData = jeRam_Allocate(Tree->Hints.HintDataLength);
		if	(!Tree->Hints.HintData)
		{
			jeErrorLog_AddString(-1,"ReadTree : Ram",Tree->Name);
			goto fail;
		}
		if	(jeVFile_Read(File, Tree->Hints.HintData, Tree->Hints.HintDataLength) == JE_FALSE)
		{
			jeErrorLog_AddString(-1,"ReadTree : VF_Read",Tree->Name);
			goto fail;
		}
	}

	assert(Tree->HintsFile == NULL);

//printf("Reading children of '%s'\n", Tree->Name);
	// Read the children
	if	(ReadTree(File, &Tree->Children) == JE_FALSE)
		goto fail;

//printf("Reading siblings of '%s'\n", Tree->Name);
	// Read the Siblings
	if	(ReadTree(File, &Tree->Siblings) == JE_FALSE)
		goto fail;

//DirTree_Dump(Tree);

	*TreePtr = Tree;

	assert(Tree->HintsFile == NULL);

	return JE_TRUE;

fail:
	if ( Tree )
	DirTree_Destroy(Tree);
	return JE_FALSE;
}

DirTree *DirTree_CreateFromFile(jeVFile *File)
{
DirTree *		Res;
DirTree_Header	Header;
jeVFile * LZFS;
	
#ifdef	KROUERDEBUG
{
	FILE *	fp;
	fp = fopen("c:\\vfs.eli", "ab+");
	fprintf(fp, "DirTree_CreateFromFile: Begins...\r\n");
	fclose(fp);
}
#endif

	if ( ! (LZFS = jeVFile_OpenNewSystem(File,JE_VFILE_TYPE_LZ, NULL, NULL,JE_VFILE_OPEN_READONLY) ))
		return NULL;

	if ( ! jeVFile_Read(LZFS, &Header, sizeof(Header)) )
		return NULL;

	if	(Header.Signature != DIRTREE_FILE_SIGNATURE)
	{
		jeErrorLog_AddString(-1,"DirTree : didn't get signature!",NULL);
		return NULL;
	}

	if	(ReadTree(LZFS, &Res) == JE_FALSE)
		return NULL;

	if ( ! jeVFile_Close(LZFS) )
		return NULL;

#ifdef	KROUERDEBUG
{
	FILE *	fp;
	fp = fopen("c:\\vfs.eli", "ab+");
	fprintf(fp, "DirTree_CreateFromFile: Ends...\r\n");
	fclose(fp);
}
#endif

	return Res;
}

static	const char *GetNextDir(const char *Path, char *Buff)
{
	while	(*Path && *Path != '\\')
		*Buff++ = *Path++;
	*Buff = '\0';

	if	(*Path == '\\')
		Path++;

	return Path;
}

DirTree *DirTree_FindExact(const DirTree *Tree, const char *Path)
{
	static char	Buff[_MAX_PATH];
	DirTree *	Siblings;

	assert(Tree);
	assert(Path);

	if	(*Path == '\\')
		return NULL;

	if	(*Path == '\0')
		return (DirTree *)Tree;

	Path = GetNextDir(Path, Buff);

	Siblings = Tree->Children;
	while	(Siblings)
	{
		if	(!stricmp(Siblings->Name, Buff))
		{
			if	(!*Path)
				return Siblings;
			return DirTree_FindExact(Siblings, Path);
		}
		Siblings = Siblings->Siblings;
	}

	return NULL;
}

DirTree *DirTree_FindPartial(
	const DirTree *	Tree,
	const char *	Path,
	const char **	LeftOvers)
{
	static char	Buff[_MAX_PATH];
	DirTree *	Siblings;

	assert(Tree);
	assert(Path);

	if	(*Path == '\\')
		return NULL;

	*LeftOvers = Path;

	if	(*Path == '\0')
		return (DirTree *)Tree;

	Path = GetNextDir(Path, Buff);

	Siblings = Tree->Children;
	while	(Siblings)
	{
		if	(!stricmp(Siblings->Name, Buff))
		{
			*LeftOvers = Path;
			if	(!*Path)
				return Siblings;
			return DirTree_FindPartial(Siblings, Path, LeftOvers);
		}
		Siblings = Siblings->Siblings;
	}

	return (DirTree *)Tree;
}

static	jeBoolean	PathHasDir(const char *Path)
{
	if	(strchr(Path, '\\'))
		return JE_TRUE;

	return JE_FALSE;
}

DirTree * DirTree_AddFile(DirTree *Tree, const char *Path, jeBoolean IsDirectory)
{
	DirTree *		NewEntry;
	const char *	LeftOvers;

	assert(Tree);
	assert(Path);
	assert(IsDirectory == JE_TRUE || IsDirectory == JE_FALSE);

	assert(strlen(Path) > 0);

	if	(PathHasDir(Path))
	{
		Tree = DirTree_FindPartial(Tree, Path, &LeftOvers);
		if	(!Tree)
			return NULL;
	
		if	(PathHasDir(LeftOvers))
			return NULL;
	
		Path = LeftOvers;
	}

	NewEntry = (DirTree *)jeRam_Allocate(sizeof(*NewEntry));
	if	(!NewEntry)
		return NULL;

	memset(NewEntry, 0, sizeof(*NewEntry));
	NewEntry->Name = DuplicateString(Path);
	if	(!NewEntry->Name)
	{
		jeRam_Free(NewEntry->Name);
		jeRam_Free(NewEntry);
		return NULL;
	}

	NewEntry->Siblings = Tree->Children;
						 Tree->Children = NewEntry;

	if	(IsDirectory == JE_TRUE)
		NewEntry->AttributeFlags |= JE_VFILE_ATTRIB_DIRECTORY;

	return NewEntry;
}

jeBoolean DirTree_Remove(DirTree *Tree, DirTree *SubTree)
{
	DirTree 	Siblings;
	DirTree * 	pSiblings;
	DirTree *	Parent;
	DirTree *	ParanoiaCheck;

	assert(Tree);
	assert(SubTree);

	Parent = SubTree->Parent;
	assert(Parent);

	ParanoiaCheck = Parent;
	while	(ParanoiaCheck && ParanoiaCheck != Tree)
		ParanoiaCheck = ParanoiaCheck->Parent;
	if	(!ParanoiaCheck)
		return JE_FALSE;

	Siblings.Siblings = Parent->Children;
	assert(Siblings.Siblings);
	pSiblings = &Siblings;
	while	(pSiblings->Siblings)
	{
		if	(pSiblings->Siblings == SubTree)
		{
			pSiblings->Siblings = SubTree->Siblings;
			if	(SubTree == Parent->Children)
				Parent->Children = SubTree->Siblings;
			SubTree->Siblings = NULL;
			DirTree_Destroy(SubTree);
			return JE_TRUE;
		}
		pSiblings = pSiblings->Siblings;
	}

	assert(!"Shouldn't be a way to get here");
	return JE_FALSE;
}

void DirTree_SetFileAttributes(DirTree *Tree, jeVFile_Attributes Attributes)
{
	assert(Tree);
	assert(Attributes);

	// Only support the read only flag
	assert(!(Attributes & ~JE_VFILE_ATTRIB_READONLY));
	assert(!(Tree->AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY));

	Tree->AttributeFlags = (Tree->AttributeFlags & ~JE_VFILE_ATTRIB_READONLY)  | Attributes;
}

void DirTree_GetFileAttributes(DirTree *Tree, jeVFile_Attributes *Attributes)
{
	assert(Tree);
	assert(Attributes);

	*Attributes = Tree->AttributeFlags;
}

void DirTree_SetFileOffset(DirTree *Leaf, long Offset)
{
	assert(Leaf);
	assert(!(Leaf->AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY));

	Leaf->Offset = Offset;
}

void DirTree_GetFileOffset(DirTree *Leaf, long *Offset)
{
	assert(Leaf);
	assert(!(Leaf->AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY));

	*Offset = Leaf->Offset;
}

void DirTree_SetFileTime(DirTree *Tree, const jeVFile_Time *Time)
{
	assert(Tree);

	Tree->Time = *Time;
}

void DirTree_GetFileTime(DirTree *Tree, jeVFile_Time *Time)
{
	assert(Tree);

	*Time = Tree->Time;
}

jeBoolean DirTree_FileHasHints(DirTree *Tree)
{
	if	(!Tree->HintsFile && Tree->Hints.HintDataLength == 0)	// <> CB 2/10
		return JE_FALSE;

	return JE_TRUE;
}

jeVFile * DirTree_GetHintsFile(DirTree *Tree)
{
	if	(!Tree->HintsFile)
	{
		jeVFile_MemoryContext	MemoryContext;

		if	(Tree->Hints.HintDataLength != 0)
		{
			MemoryContext.Data = Tree->Hints.HintData;
			MemoryContext.DataLength = Tree->Hints.HintDataLength;
	
			Tree->HintsFile = jeVFile_OpenNewSystem(NULL,
													JE_VFILE_TYPE_MEMORY,
													NULL,
													&MemoryContext,
													JE_VFILE_OPEN_READONLY);
		}
		else
		{
			MemoryContext.Data = NULL;
			MemoryContext.DataLength = 0;
	
			Tree->HintsFile = jeVFile_OpenNewSystem(NULL,
													JE_VFILE_TYPE_MEMORY,
													NULL,
													&MemoryContext,
													JE_VFILE_OPEN_CREATE);
		}
	}

	return Tree->HintsFile;
}

jeBoolean DirTree_GetName(const DirTree *Tree, char *Buff, int MaxLen)
{
	int	Length;

	assert(Tree);
	assert(Buff);
	assert(MaxLen > 0);

	Length = strlen(Tree->Name);
	if	(Length > MaxLen)
		return JE_FALSE;

	memcpy(Buff, Tree->Name, Length + 1);

	return JE_TRUE;
}

jeBoolean DirTree_GetFullName(const DirTree *Tree, char *Buff, int MaxLen)
{
	int	Length;

	Length = strlen(Tree->Name) + 1;
	if	(Length > MaxLen)
		return JE_FALSE;

	*Buff = '\0';
	if	(Tree->Parent)
	{
		if	(DirTree_GetFullName(Tree->Parent, Buff, MaxLen - Length) == JE_FALSE)
			return JE_FALSE;
	}

	strcat(Buff, Tree->Name);

	return JE_TRUE;
}

jeBoolean DirTree_FileExists(const DirTree *Tree, const char *Path)
{
	if	(DirTree_FindExact(Tree, Path) == NULL)
		return JE_FALSE;

	return JE_TRUE;
}

DirTree_Finder * DirTree_CreateFinder(DirTree *Tree, const char *Path)
{
	DirTree_Finder *	Finder;
	DirTree *			SubTree;
	char				Directory[_MAX_PATH];
	char				Name[_MAX_FNAME];
	char				Ext[_MAX_EXT];

	assert(Tree);
	assert(Path);

	_splitpath((char *)Path, NULL, Directory, Name, Ext);

	if	(strlen(Ext) == 0)
		strcpy(Ext, "*");

#pragma message("DirTree_CreateFinder : separate handling of Name and Ext is archaic!")
	// CB : Win9x no longer has the 8.3 paradigm!
	// filenames like "gurbu.splat.foo" are VALID !
	// as are names like "dukubatu" !

	SubTree = DirTree_FindExact(Tree, Directory);
	if	(!SubTree)
		return NULL;

	Finder = (DirTree_Finder *)jeRam_Allocate(sizeof(*Finder));
	if	(!Finder)
		return Finder;

	Finder->MatchName = DuplicateString(Name);
	if	(!Finder->MatchName)
	{
		jeRam_Free(Finder);
		return NULL;
	}

	/*// The RTL leaves the '.' on there.  That won't do.
	if	(*Ext == '.')
		Finder->MatchExt = DuplicateString(&Ext[1]);
	else*/
		Finder->MatchExt = DuplicateString(&Ext[0]);

	if	(!Finder->MatchExt)
	{
		jeRam_Free(Finder->MatchName);
		jeRam_Free(Finder);
		return NULL;
	}

	Finder->Current = SubTree->Children;

	return Finder;
}

void DirTree_DestroyFinder(DirTree_Finder *Finder)
{
	assert(Finder);
	assert(Finder->MatchName);
	assert(Finder->MatchExt);

	jeRam_Free(Finder->MatchName);
	jeRam_Free(Finder->MatchExt);
	jeRam_Free(Finder);
}

static jeBoolean	MatchPattern(const char *Source, const char *Pattern)
{
	assert(Source);
	assert(Pattern);

	switch	(*Pattern)
	{
	case	'\0':
		if	(*Source)
			return JE_FALSE;
		break;

	case	'*':
		if	(*(Pattern + 1) != '\0')
		{
			Pattern++;
			while	(*Source)
			{
				if	(MatchPattern(Source, Pattern) == JE_TRUE)
					return JE_TRUE;
				Source++;
			}
			return JE_FALSE;
		}
		break;

	case	'?':
		return MatchPattern(Source + 1, Pattern + 1);

	default:
		if	(*Source == *Pattern)
			return MatchPattern(Source + 1, Pattern + 1);
		else
			return JE_FALSE;
	}

	return JE_TRUE;
}

DirTree * DirTree_FinderGetNextFile(DirTree_Finder *Finder)
{
	DirTree *	Res;
	char		Name[_MAX_FNAME];
	char		Ext[_MAX_EXT];

	assert(Finder);

	Res = Finder->Current;

	if	(!Res)
		return Res;

	do
	{
		_splitpath(Res->Name, NULL, NULL, Name, Ext);
		if	(MatchPattern(Name, Finder->MatchName) == JE_TRUE &&
			 MatchPattern(Ext,  Finder->MatchExt) == JE_TRUE)
		{
			break;
		}

		Res = Res->Siblings;

	}	while	(Res);

	if	(Res)
		Finder->Current = Res->Siblings;

	return Res;
}

#define DEBUG

#ifdef	DEBUG
static	void	indent(int i)
{
	while	(i--)
		printf(" ");
}

static	void DirTree_Dump1(const DirTree *Tree, int i)
{
	DirTree *	Temp;

	indent(i);
	if	(Tree->AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY)
		printf("\\%s\n", Tree->Name);
	else
		printf("%-*s  %08x  %08x\n", 40 - i, Tree->Name, Tree->Offset, Tree->Size);
	Temp = Tree->Children;
	while	(Temp)
	{
		DirTree_Dump1(Temp, i + 2);
		Temp = Temp->Siblings;
	}
}

void DirTree_Dump(const DirTree *Tree)
{
	printf("%-*s  %-8s  %-8s\n", 40, "Name", "Offset", "Size");
	printf("------------------------------------------------------------\n");
	DirTree_Dump1(Tree, 0);
}

#endif

#ifdef BUILD_BE

void FastSplitPath(register char* path, char* drive, char* dir, char* fname, char* ext)
{
        register char *p;
        char *last_slash = NULL, *dot = NULL;
        unsigned len;

        /* we assume that the path argument has the following form, where any
         * or all of the components may be missing.
         *
         *  <drive><dir><fname><ext>
         *
         * and each of the components has the following expected form(s)
         *
         *  drive:
         *  0 to _MAX_DRIVE-1 characters, the last of which, if any, is a
         *  ':'
         *  dir:
         *  0 to _MAX_DIR-1 characters in the form of an absolute path
         *  (leading '/' or '\') or relative path, the last of which, if
         *  any, must be a '/' or '\'.  E.g -
         *  absolute path:
         *      \top\next\last\     ; or
         *      /top/next/last/
         *  relative path:
         *      top\next\last\  ; or
         *      top/next/last/
         *  Mixed use of '/' and '\' within a path is also tolerated
         *  fname:
         *  0 to _MAX_FNAME-1 characters not including the '.' character
         *  ext:
         *  0 to _MAX_EXT-1 characters where, if any, the first must be a
         *  '.'
         *
         */
        /* extract drive letter and :, if any */
        if ((strlen(path) >= (_MAX_DRIVE - 2)) && (*(path + _MAX_DRIVE - 2) == ':')) {
            if (drive) {
                strncpy(drive, path, _MAX_DRIVE - 1);
                *(drive + _MAX_DRIVE-1) = '\0';
            }
            path += _MAX_DRIVE - 1;
        }
        else if (drive) {
            *drive = '\0';
        }

        /* extract path string, if any.  Path now points to the first character
         * of the path, if any, or the filename or extension, if no path was
         * specified.  Scan ahead for the last occurence, if any, of a '/' or
         * '\' path separator character.  If none is found, there is no path.
         * We will also note the last '.' character found, if any, to aid in
         * handling the extension.
         */
        for (last_slash = NULL, p = (char *)path; *p; p++) {
            if (*p == '/' || *p == '\\')
                /* point to one beyond for later copy */
                last_slash = p + 1;

            else if (*p == '.')
                dot = p;
        }

        if (last_slash) {

            /* found a path - copy up through last_slash or max. characters
             * allowed, whichever is smaller
             */

            if (dir) {
                len = MIN(((char *)last_slash - (char *)path) / sizeof(char),
                    (_MAX_DIR - 1));
                strncpy(dir, path, len);
                *(dir + len) = '\0';
            }
            path = last_slash;
        }
        else if (dir) {
            /* no path found */
            *dir = '\0';
        }

        /* extract file name and extension, if any.  Path now points to the
         * first character of the file name, if any, or the extension if no
         * file name was given.  Dot points to the '.' beginning the extension,
         * if any.
         */

        if (dot && (dot >= path)) {
            /* found the marker for an extension - copy the file name up to
             * the '.'.
             */
            if (fname) {
                len = MIN(((char *)dot - (char *)path) / sizeof(char),
                    (_MAX_FNAME - 1));
                strncpy(fname, path, len);
                *(fname + len) = '\0';
            }
            /* now we can get the extension - remember that p still points
             * to the terminating nul character of path.
             */
            if (ext) {
                len = MIN(((char *)p - (char *)dot) / sizeof(char),
                    (_MAX_EXT - 1));
                strncpy(ext, dot, len);
                *(ext + len) = '\0';
            }
        }
        else {
            /* found no extension, give empty extension and copy rest of
             * string into fname.
             */
            if (fname) {
                len = MIN(((char *)p - (char *)path) / sizeof(char),
                    (_MAX_FNAME - 1));
                strncpy(fname, path, len);
                *(fname + len) = '\0';
            }
            if (ext) {
                *ext = '\0';
            }
        }
}

#endif
