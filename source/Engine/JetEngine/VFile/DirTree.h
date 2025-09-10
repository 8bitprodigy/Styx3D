/****************************************************************************************/
/*  DIRTREE.H                                                                           */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Directory tree interface                                               */
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
#ifndef	DIRTREE_H
#define	DIRTREE_H

#include	"VFile.h"

typedef struct DirTree			DirTree;
typedef struct DirTree_Finder	DirTree_Finder;

DirTree *DirTree_Create(void);

DirTree *DirTree_CreateFromFile(jeVFile *File);

jeBoolean DirTree_WriteToFile(const DirTree *Tree, jeVFile *File);

jeBoolean DirTree_GetSize(const DirTree *Tree, long *Size);
	// Gets the size of data that will be written to disk to persist
	// the tree.  This API is NOT efficient.

void DirTree_Destroy(DirTree *Tree);

DirTree *DirTree_FindExact(const DirTree *Tree, const char *Path);
DirTree *DirTree_FindPartial(
	const DirTree *	Tree,
	const char *	Path,
	const char **	LeftOvers);
//DirTree *DirTree_Find(const DirTree *Tree, const char *Path);

jeBoolean DirTree_OpenFile(DirTree * Tree,uint32 OpenFlags);

DirTree * DirTree_AddFile(DirTree *Tree, const char *Path, jeBoolean IsDirectory);

jeBoolean DirTree_Remove(DirTree *Tree, DirTree *SubTree);

void DirTree_SetFileAttributes(DirTree *Tree, jeVFile_Attributes Attributes);

void DirTree_GetFileAttributes(DirTree *Tree, jeVFile_Attributes *Attributes);

void DirTree_SetFileOffset(DirTree *Tree, long Offset);

void DirTree_GetFileOffset(DirTree *Tree, long *Offset);

void DirTree_SetFileTime(DirTree *Tree, const jeVFile_Time *Time);

void DirTree_GetFileTime(DirTree *Tree, jeVFile_Time *Time);

void DirTree_SetFileSize(DirTree *Tree, long Size);

void DirTree_GetFileSize(DirTree *Tree, long *Size);

jeBoolean DirTree_FileHasHints(DirTree *Tree);
jeVFile * DirTree_GetHintsFile(DirTree *Tree);

jeBoolean DirTree_GetFullName(const DirTree *Tree, char *Buff, int MaxLen);
jeBoolean DirTree_GetName(const DirTree *Tree, char *Buff, int MaxLen);

jeBoolean DirTree_FileExists(const DirTree *Tree, const char *Path);


DirTree_Finder * DirTree_CreateFinder(DirTree *Tree, const char *Path);

void DirTree_DestroyFinder(DirTree_Finder *Finder);

DirTree * DirTree_FinderGetNextFile(DirTree_Finder *Finder);


void DirTree_Dump(const DirTree *Tree);

#endif
