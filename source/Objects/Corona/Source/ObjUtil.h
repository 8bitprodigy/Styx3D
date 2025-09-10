/****************************************************************************************/
/*  OBJUTIL.H                                                                           */
/*                                                                                      */
/*  Author:                                                                             */
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
#ifndef OBJUTIL_H
#define OBJUTIL_H

#pragma warning ( disable : 4115 )
#include <windows.h>
#pragma warning ( default : 4115 )
#include "jeWorld.h"


#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Bitmaplist struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct BitmapList
{

	// bitmap info
	int		Total;			// total number of bitmaps available
	char	**Name;			// name of each bitmap
	int		*Width;			// width of each bitmap
	int		*Height;		// height of each bitmap

	// bitmap sizes
	int		*NumericSizes;	// available bitmap sizes, numerical list
	char	**StringSizes;	// available bitmap sizes, string list
	int		SizesListSize;	// size of available bitmap sizes lists

	// current active bitmap list
	char	**ActiveList;	// list of available bitmaps based on current size
	int		ActiveCount;	// members in the active list (changes)
	int		ActiveCurSize;	// currently active size

} BitmapList;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	ObjUtil_StrDup()
//
////////////////////////////////////////////////////////////////////////////////////////
char * ObjUtil_StrDup(
	const char	*const psz );	// string to copy

//	ObjUtil_LoadLibraryString()
//
////////////////////////////////////////////////////////////////////////////////////////
char * ObjUtil_LoadLibraryString(
	HINSTANCE		hInstance,	// instance handle
	unsigned int	ID );		// message id

//	ObjUtil_LogError()
//
////////////////////////////////////////////////////////////////////////////////////////
void ObjUtil_LogError(
	HINSTANCE	hInstance,	// instance to get strings from
	int			Type,		// error type
	int			Message );	// error message

//	ObjUtil_WriteString()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean ObjUtil_WriteString(
	jeVFile	*File,		// file to write to
	char	*String );	// string to write out

//	ObjUtil_ReadString()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean ObjUtil_ReadString(
	jeVFile	*File,		// file to read from
	char	**String );	// where to save string pointer

//	ObjUtil_DestroyBitmapList()
//
////////////////////////////////////////////////////////////////////////////////////////
void ObjUtil_DestroyBitmapList(
	BitmapList	**DeadList );	// list to destroy

//	ObjUtil_CreateBitmapList()
//
////////////////////////////////////////////////////////////////////////////////////////
BitmapList * ObjUtil_CreateBitmapList(
	jeResourceMgr	*ResourceMgr,	// resource manager to use
	char			*ResourceName,	// name of resource
	char			*FileFilter );	// file filter

//	ObjUtil_TextureGroupSetSize()
//
////////////////////////////////////////////////////////////////////////////////////////
void ObjUtil_TextureGroupSetSize(
	jeEngine		*Engine,			// engine to use
	jeResourceMgr	*ResourceMgr,		// resource manager to use
	BitmapList		*AvailableArt,		// list of all available art
	char			*ChosenSizeName,	// name of size that was chosen
	char			**SaveBitmapName,	// current bitmap name and where to save new name
	char			**SaveAlphaName,	// current alpha name and where to save new name
	jeBitmap		**SaveArt,			// current art and where to save new art
	char			**SaveArtName );	// current art name and where to save new name

//	ObjUtil_TextureGroupSetArt()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean ObjUtil_TextureGroupSetArt(
	jeEngine		*Engine,			// engine to use
	jeResourceMgr	*ResourceMgr,		// resource manager to use
	BitmapList		*AvailableArt,		// list of all available art
	char			*ChosenBitmapName,	// name of bitmap that was chosen
	char			*ChosenAlphaName,	// name of alpha that was chosen
	char			**SaveBitmapName,	// current bitmap name and where to save new name
	char			**SaveAlphaName,	// current alpha name and where to save new name
	jeBitmap		**SaveArt,			// current art and where to save new art
	char			**ArtName );		// current art name and where to save new name


#ifdef __cplusplus
	}
#endif

#endif
