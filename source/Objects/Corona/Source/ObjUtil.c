/****************************************************************************************/
/*  OBJUTIL.C                                                                           */
/*                                                                                      */
/*  Author:  Peter Siamidis                                                             */
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "vfile.h"
#include "bitmap.h"
#include "ram.h"
#include "errorlog.h"
#include "jeResource.h"
#include "ObjUtil.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Globals
////////////////////////////////////////////////////////////////////////////////////////
static char		*NoSelection = "< none >";



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_StrDup()
//
////////////////////////////////////////////////////////////////////////////////////////
char * ObjUtil_StrDup(
	const char	*const psz )	// string to copy
{

	// copy string
	char * p = (char *)jeRam_Allocate( strlen( psz ) + 1 );
	if ( p ) 
	{
		strcpy( p, psz );
	}
	else
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
	}

	// return string
	return p;

} // ObjUtil_StrDup()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_LoadLibraryString()
//
////////////////////////////////////////////////////////////////////////////////////////
char * ObjUtil_LoadLibraryString(
	HINSTANCE		hInstance,	// instance handle
	unsigned int	ID )		// message id
{

	// locals
	#define		MAX_STRING_SIZE	256
	static char	StringBuf[MAX_STRING_SIZE];
	char		*NewString;
	int			Size;

	// ensure valid data
	assert( hInstance != NULL );
	assert( ID >= 0 );

	// get resource string
	Size = LoadString( hInstance, ID, StringBuf, MAX_STRING_SIZE );
	if ( Size <= 0 )
	{
		jeErrorLog_Add( JE_ERR_WINDOWS_API_FAILURE, "Failed to get resource string" );
		return NULL;
	}

	// copy resource string
	NewString = jeRam_Allocate( Size + 1 );
	if ( NewString == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Failed to allocate string memory" );
		return NULL;
	}
	strcpy( NewString, StringBuf );

	// all done
	return NewString;

} // ObjUtil_LoadLibraryString()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_LogError()
//
////////////////////////////////////////////////////////////////////////////////////////
void ObjUtil_LogError(
	HINSTANCE	hInstance,	// instance to get strings from
	int			Type,		// error type
	int			Message )	// error message
{

	// locals
	char	StringMessage[256];

	// ensure valid data
	assert( hInstance != NULL );
	assert( Type >= 0 );
	assert( Message >= 0 );
	
	// log generic error message if we have no hInstance
	if ( hInstance == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "No class handle" );
		return;
	}
		
	// get error message
	if ( LoadString( hInstance, Message, StringMessage, 256 ) <= 0 )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Could not get error message string" );
		return;
	}

	// log error
	jeErrorLog_Add( Type, StringMessage );

} // ObjUtil_LogError()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_WriteString()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean ObjUtil_WriteString(
	jeVFile	*File,		// file to write to
	char	*String )	// string to write out
{

	// locals
	int			Size;
	jeBoolean	Result = JE_TRUE;

	// ensure valid data
	assert( File != NULL );
	assert( String != NULL );

	// write out complete
	Size = strlen( String ) + 1;
	assert( Size > 0 );
	Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	Result &= jeVFile_Write( File, String, Size );

	// all done
	return Result;

} // ObjUtil_WriteString()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_ReadString()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean ObjUtil_ReadString(
	jeVFile	*File,		// file to read from
	char	**String )	// where to save string pointer
{

	// locals
	int			Size;
	jeBoolean	Result = JE_TRUE;

	// ensure valid data
	assert( File != NULL );
	assert( String != NULL );

	// read string
	Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
	{
		*String = jeRam_Allocate( Size );
		if ( *String == NULL )
		{
			return JE_FALSE;
		}
		Result &= jeVFile_Read( File, *String, Size );
	}

	// all done
	return Result;

} // ObjUtil_ReadString()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_CreateBitmapFromFileName()
//
//	Create a bitmap from a file.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBitmap * ObjUtil_CreateBitmapFromFileName(
	jeVFile		*File,			// file system to use
	const char	*Name,			// name of the file
	const char	*AlphaName )	// name of the alpha file
{

	// locals
	jeVFile		*BmpFile;
	jeBitmap	*Bmp;
	jeBoolean	Result;

	// ensure valid data
	assert( Name != NULL );

	// open the bitmap
	if ( File == NULL )
	{
		BmpFile = jeVFile_OpenNewSystem( NULL, JE_VFILE_TYPE_DOS, Name, NULL, JE_VFILE_OPEN_READONLY );
	}
	else
	{
		BmpFile = jeVFile_Open( File, Name, JE_VFILE_OPEN_READONLY );
	}
	if ( BmpFile == NULL )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_OPEN, NULL );
		return NULL;
	}

	// create the bitmap
	Bmp = jeBitmap_CreateFromFile( BmpFile );
	jeVFile_Close( BmpFile );
	if ( Bmp == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		return NULL;
	}

	// add alpha if required...
	if ( AlphaName != NULL )
	{

		// locals
		jeBitmap	*AlphaBmp;
		jeVFile		*AlphaFile;

		// open alpha file
		if ( File == NULL )
		{
			AlphaFile = jeVFile_OpenNewSystem( NULL, JE_VFILE_TYPE_DOS, AlphaName, NULL, JE_VFILE_OPEN_READONLY );
		}
		else
		{
			AlphaFile = jeVFile_Open( File, AlphaName, JE_VFILE_OPEN_READONLY );
		}
		if( AlphaFile == NULL )
		{
			jeErrorLog_Add( JE_ERR_FILEIO_OPEN, NULL );
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// create alpha bitmap
		AlphaBmp = jeBitmap_CreateFromFile( AlphaFile );
		jeVFile_Close( AlphaFile );
		if ( AlphaBmp == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// fail if alpha isn't same size as main bitmap
		if (	( jeBitmap_Width( Bmp ) != jeBitmap_Width( AlphaBmp ) ) ||
				( jeBitmap_Height( Bmp ) != jeBitmap_Height( AlphaBmp ) ) )
		{
			jeErrorLog_Add( JE_ERR_BAD_PARAMETER, NULL );
			jeBitmap_Destroy( &AlphaBmp );
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// set its alpha
		Result = jeBitmap_SetAlpha( Bmp, AlphaBmp );
		if ( Result == JE_FALSE )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			jeBitmap_Destroy( &AlphaBmp );
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// don't need the alpha anymore
		jeBitmap_Destroy( &AlphaBmp );
	}
	// ...or just set the color key
	else
	{
		Result = jeBitmap_SetColorKey( Bmp, JE_TRUE, 255, JE_FALSE );
		assert( Result );
	}

	// all done
	return Bmp;

} // ObjUtil_CreateBitmapFromFileName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_DestroyBitmapList()
//
////////////////////////////////////////////////////////////////////////////////////////
void ObjUtil_DestroyBitmapList(
	BitmapList	**DeadList )	// list to destroy
{

	// locals
	BitmapList	*List;
	int			i;

	// ensure valid data
	assert( DeadList != NULL );
	assert( *DeadList != NULL );

	// get list pointer
	List = *DeadList;

	// destroy file list
	if ( List->Name != NULL )
	{
		assert( List->Total > 0 );
		for ( i = 0; i < List->Total; i++ )
		{
			if ( List->Name[i] != NULL )
			{
				jeRam_Free( List->Name[i] );
			}
		}
		jeRam_Free( List->Name );
	}

	// destroy width and height lists
	if ( List->Width != NULL )
	{
		jeRam_Free( List->Width );
	}
	if ( List->Height != NULL )
	{
		jeRam_Free( List->Height );
	}

	// destroy numeric sizes list
	if ( List->NumericSizes != NULL )
	{
		jeRam_Free( List->NumericSizes );
	}

	// destroy string sizes list
	if ( List->StringSizes != NULL )
	{
		for ( i = 0; i < List->SizesListSize; i++ )
		{
			assert( List->StringSizes[i] != NULL );
			jeRam_Free( List->StringSizes[i] );
		}
	}

	// destroy active list
	if ( List->ActiveList != NULL )
	{
		jeRam_Free( List->ActiveList );
	}

	// free bitmaplist struct
	jeRam_Free( List );

	// zap pointer
	*DeadList = NULL;

} // ObjUtil_DestroyBitmapList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_TextureGroupSetActiveList()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ObjUtil_TextureGroupSetActiveList(
	BitmapList	*AvailableArt,		// list of all available art
	int			NewActiveCurSize )	// new active size slot
{

	// locals
	int	BmpNum;

	// ensure valid data
	assert( AvailableArt != NULL );
	assert( NewActiveCurSize >= 0 );
	assert( NewActiveCurSize < AvailableArt->SizesListSize );

	// setup current list
	AvailableArt->ActiveCurSize = NewActiveCurSize;
	AvailableArt->ActiveList[0] = AvailableArt->Name[0];
	AvailableArt->ActiveCount = 1;
	for ( BmpNum = 1; BmpNum < AvailableArt->Total; BmpNum++ )
	{

		// zap old entry
		AvailableArt->ActiveList[BmpNum] = NULL;

		// add new entry if required
		if ( AvailableArt->Width[BmpNum] == AvailableArt->NumericSizes[AvailableArt->ActiveCurSize] )
		{
			AvailableArt->ActiveList[AvailableArt->ActiveCount] = AvailableArt->Name[BmpNum];
			AvailableArt->ActiveCount++;
		}
	}

} // ObjUtil_TextureGroupSetActiveList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_CreateBitmapList()
//
////////////////////////////////////////////////////////////////////////////////////////
BitmapList * ObjUtil_CreateBitmapList(
	jeResourceMgr	*ResourceMgr,	// resource manager to use
	char			*ResourceName,	// name of resource
	char			*FileFilter )	// file filter
{

	// locals
	BitmapList		*Bmps;
	jeVFile			*FileDir = NULL;
	jeVFile_Finder	*Finder = NULL;
	int				CurFile;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceName != NULL );
	assert( FileFilter != NULL );

	// allocate bitmaplist struct
	Bmps = jeRam_AllocateClear( sizeof( *Bmps ) );
	if ( Bmps == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return NULL;
	}

	// get vfile dir
	FileDir = jeResource_GetVFile( ResourceMgr, ResourceName );
	if ( FileDir == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		goto ERROR_ObjUtil_BuildBitmapList;
	}

	// create directory finder
	Finder = jeVFile_CreateFinder( FileDir, FileFilter );
	if ( Finder == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		goto ERROR_ObjUtil_BuildBitmapList;
	}

	// determine how many files there are
	Bmps->Total = 1;
	while ( jeVFile_FinderGetNextFile( Finder ) == JE_TRUE )
	{
		Bmps->Total++;
	}

	// destroy finder
	jeVFile_DestroyFinder( Finder );
	Finder = NULL;

	// allocate name list
	Bmps->Name = jeRam_AllocateClear( sizeof( char * ) * Bmps->Total );
	if ( Bmps->Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_ObjUtil_BuildBitmapList;
	}

	// allocate width list
	Bmps->Width = jeRam_AllocateClear( sizeof( int * ) * Bmps->Total );
	if ( Bmps->Width == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_ObjUtil_BuildBitmapList;
	}

	// allocate height list
	Bmps->Height = jeRam_AllocateClear( sizeof( int * ) * Bmps->Total );
	if ( Bmps->Height == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_ObjUtil_BuildBitmapList;
	}

	// allocate numeric sizes list
	Bmps->NumericSizes = jeRam_AllocateClear( sizeof( int * ) * Bmps->Total );
	if ( Bmps->NumericSizes == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_ObjUtil_BuildBitmapList;
	}

	// allocate string sizes list
	Bmps->StringSizes = jeRam_AllocateClear( sizeof( char * ) * Bmps->Total );
	if ( Bmps->StringSizes == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_ObjUtil_BuildBitmapList;
	}

	// create directory finder
	Finder = jeVFile_CreateFinder( FileDir, FileFilter );
	if ( Finder == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		goto ERROR_ObjUtil_BuildBitmapList;
	}

	// first entry is always the "no selection" slot
	CurFile = 0;
	Bmps->Name[CurFile++] = ObjUtil_StrDup( NoSelection );

	// build file list
	while ( jeVFile_FinderGetNextFile( Finder ) == JE_TRUE )
	{

		// locals
		jeVFile_Properties	Properties;
		jeBitmap			*Bitmap;

		// get properties of current file
		if( jeVFile_FinderGetProperties( Finder, &Properties ) == JE_FALSE )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			goto ERROR_ObjUtil_BuildBitmapList;
		}

		// save file name
		assert( CurFile < Bmps->Total );
		Bmps->Name[CurFile] = ObjUtil_StrDup( Properties.Name );
		if ( Bmps->Name[CurFile] == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			goto ERROR_ObjUtil_BuildBitmapList;
		}

		// save width and height
		Bitmap = ObjUtil_CreateBitmapFromFileName( FileDir, Bmps->Name[CurFile], NULL );
		if ( Bitmap == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			goto ERROR_ObjUtil_BuildBitmapList;
		}
		Bmps->Width[CurFile] = jeBitmap_Width( Bitmap );
		Bmps->Height[CurFile] = jeBitmap_Height( Bitmap );
		jeBitmap_Destroy( &Bitmap );

		// add size to numeric sizes list
		{

			// locals
			jeBoolean	AddIt;
			int			i;

			// check if it needs to be added to numeric sizes list
			AddIt = JE_TRUE;
			for ( i = 0; i < Bmps->Total; i++ )
			{
				if ( Bmps->Width[CurFile] == Bmps->NumericSizes[i] )
				{
					AddIt = JE_FALSE;
					break;
				}
			}

			// add it if required
			if ( AddIt == JE_TRUE )
			{
				for ( i = 0; i < Bmps->Total; i++ )
				{
					if (	( Bmps->Width[CurFile] < Bmps->NumericSizes[i] ) ||
							( Bmps->NumericSizes[i] == 0 ) )
					{
						int	Hold1, Hold2;
						Hold1 = Bmps->Width[CurFile];
						for ( ; i < Bmps->Total; i++ )
						{
							Hold2 = Bmps->NumericSizes[i];
							Bmps->NumericSizes[i] = Hold1;
							Hold1 = Hold2;
						}
						AddIt = JE_FALSE;
						break;
					}
				}
			}
			assert( AddIt == JE_FALSE );
		}

		// adjust file counter
		CurFile++;
	}

	// destroy finder
	jeVFile_DestroyFinder( Finder );

	// close vfile dir
	if ( jeResource_DeleteVFile( ResourceMgr, ResourceName ) == 0 )
	{
		jeVFile_Close( FileDir );
	}

	// create string sizes list
	{

		// locals
		int		i;
		char	Buf[256];

		// build list
		Bmps->SizesListSize = Bmps->Total;
		for ( i = 0; i < Bmps->Total; i++ )
		{
			if ( Bmps->NumericSizes[i] == 0 )
			{
				Bmps->SizesListSize = i;
				break;
			}
			itoa( Bmps->NumericSizes[i], Buf, 10 );
			Bmps->StringSizes[i] = ObjUtil_StrDup( Buf );
		}
	}

	// create active list
	{

		// allocate list
		Bmps->ActiveList = jeRam_AllocateClear( sizeof( char * ) * Bmps->Total );
		if ( Bmps->ActiveList == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			goto ERROR_ObjUtil_BuildBitmapList;
		}

		// setup current list
		ObjUtil_TextureGroupSetActiveList( Bmps, 0 );
	}

	// return bitmaplist struct
	return Bmps;


	//
	//	error handling
	//
	ERROR_ObjUtil_BuildBitmapList:

	// destroy bitmap list
	assert( Bmps != NULL );
	ObjUtil_DestroyBitmapList( &Bmps );

	// destroy finder
	if ( Finder != NULL )
	{
		jeVFile_DestroyFinder( Finder );
	}

	// close vfile dir
	if ( FileDir != NULL )
	{
		if ( jeResource_DeleteVFile( ResourceMgr, ResourceName ) == 0 )
		{
			jeVFile_Close( FileDir );
		}
	}

	// return failure
	return NULL;

} // ObjUtil_CreateBitmapList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_TextureGroupGetStringFromList()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * ObjUtil_TextureGroupGetStringFromList(
	BitmapList	*AvailableArt,		// list of all available art
	char		*CompareString )	// string we are looking for
{

	// locals
	int	i;

	// ensure valid data
	assert( AvailableArt != NULL );
	assert( CompareString != NULL );

	// locate string in available bitmaps list
	for ( i = 0; i < AvailableArt->Total; i++ )
	{
		assert( AvailableArt->Name[i] != NULL );
		if ( stricmp( AvailableArt->Name[i], CompareString ) == 0 )
		{
			return AvailableArt->Name[i];
		}
	}

	// if we got to here then no string was found
	return NULL;

} // ObjUtil_TextureGroupGetStringFromList()



////////////////////////////////////////////////////////////////////////////////////////
//
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
	char			**SaveArtName )		// current art name and where to save new name
{

	// locals
	jeBoolean	SizeChangeFailed = JE_TRUE;
	int			i;

	// ensure valid data
	assert( Engine != NULL );
	assert( ResourceMgr != NULL );
	assert( AvailableArt != NULL );
	assert( ChosenSizeName != NULL );
	assert( SaveBitmapName != NULL );
	assert( SaveAlphaName != NULL );
	assert( SaveArt != NULL );
	assert( SaveArtName != NULL );

	// locate new active size
	for ( i = 0; i < AvailableArt->SizesListSize; i++ )
	{
		if ( stricmp( AvailableArt->StringSizes[i], ChosenSizeName ) == 0 )
		{

			// do nothing if same size was chosen
			if ( i == AvailableArt->ActiveCurSize )
			{
				return;
			}

			// make adjustments
			ObjUtil_TextureGroupSetActiveList( AvailableArt, i );
			SizeChangeFailed = JE_FALSE;
			i = AvailableArt->SizesListSize;
		}
	}
	assert( SizeChangeFailed == JE_FALSE );

	// reset bitmap pointers
	*SaveBitmapName = AvailableArt->ActiveList[0];
	*SaveAlphaName = AvailableArt->ActiveList[0];

	// destroy existing art
	if ( *SaveArt != NULL )
	{

		// free old art
		jeEngine_RemoveBitmap( Engine, *SaveArt );
		assert( *SaveArtName != NULL );
		if ( jeResource_Delete( ResourceMgr, *SaveArtName ) == 0 )
		{
			jeBitmap_Destroy( &( *SaveArt ) );
		}
		*SaveArt = NULL;

		// free old art name
		jeRam_Free( *SaveArtName );
		*SaveArtName = NULL;
	}

} // ObjUtil_TextureGroupSetSize()



////////////////////////////////////////////////////////////////////////////////////////
//
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
	char			**SaveArtName )		// current art name and where to save new name
{

	// ensure valid data
	assert( Engine != NULL );
	assert( ResourceMgr != NULL );
	assert( AvailableArt != NULL );
	assert( ( ( ChosenBitmapName == NULL ) && ( ChosenAlphaName == NULL ) ) == JE_FALSE );
	assert( SaveBitmapName != NULL );
	assert( SaveAlphaName != NULL );
	assert( SaveArt != NULL );
	assert( SaveArtName != NULL );

	// destroy existing art
	if ( *SaveArt != NULL )
	{

		// free old art
		jeEngine_RemoveBitmap( Engine, *SaveArt );
		assert( *SaveArtName != NULL );
		if ( jeResource_Delete( ResourceMgr, *SaveArtName ) == 0 )
		{
			jeBitmap_Destroy( &( *SaveArt ) );
		}
		*SaveArt = NULL;

		// free old art name
		jeRam_Free( *SaveArtName );
		*SaveArtName = NULL;
	}

	// get chosen string from available art list
	if ( ChosenBitmapName != NULL )
	{
		*SaveBitmapName = ObjUtil_TextureGroupGetStringFromList( AvailableArt, ChosenBitmapName );
	}
	else
	{
		*SaveAlphaName = ObjUtil_TextureGroupGetStringFromList( AvailableArt, ChosenAlphaName );
	}
	assert( *SaveBitmapName != NULL );
	assert( *SaveAlphaName != NULL );

	// do nothing further if there is no main bitmap
	if ( stricmp( *SaveBitmapName, NoSelection ) == 0 )
	{
		return JE_TRUE;
	}

	// build art name
	{

		// locals
		int	Size;

		// determine length of full art name
		Size = strlen( *SaveBitmapName ) + 1;
		if ( stricmp( *SaveAlphaName, NoSelection ) != 0 )
		{
			Size += strlen( *SaveAlphaName );
		}

		// create full artname
		*SaveArtName = jeRam_Allocate( Size );
		if ( *SaveArtName == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Failed to allocate full art name" );
			goto ERROR_ObjUtil_TextureGroupSetArt;
		}
		strcpy( *SaveArtName, *SaveBitmapName );
		if ( stricmp( *SaveAlphaName, NoSelection ) != 0 )
		{
			strcat( *SaveArtName, *SaveAlphaName );
		}
	}

	// get new art
	*SaveArt = jeResource_Get( ResourceMgr, *SaveArtName );

	// if it doesn't exist then create it
	if ( *SaveArt == NULL )
	{

		// locals
		jeVFile	*FileDir;

		// get vfile dir
		FileDir = jeResource_GetVFile( ResourceMgr, "GlobalMaterials" );
		if ( FileDir == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Failed to get a vfile resource" );
			goto ERROR_ObjUtil_TextureGroupSetArt;
		}

		// create new art
		if ( stricmp( *SaveAlphaName, NoSelection ) != 0 )
		{
			*SaveArt = ObjUtil_CreateBitmapFromFileName( FileDir, *SaveBitmapName, *SaveAlphaName );
		}
		else
		{
			*SaveArt = ObjUtil_CreateBitmapFromFileName( FileDir, *SaveBitmapName, NULL );
		}

		// close vfile dir
		if ( jeResource_DeleteVFile( ResourceMgr, "GlobalMaterials" ) == 0 )
		{
			jeVFile_Close( FileDir );
		}

		// fail if art wasnt created
		if ( *SaveArt == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Failed to create new art" );
			goto ERROR_ObjUtil_TextureGroupSetArt;
		}

		// add it to the resource manager
		jeResource_Add( ResourceMgr, *SaveArtName, JE_RESOURCE_BITMAP, *SaveArt );
	}

	// add it to the engine
	jeEngine_AddBitmap( Engine, *SaveArt, JE_ENGINE_BITMAP_TYPE_3D );

	// all done
	return JE_TRUE;


	//
	//	error handling
	//
	ERROR_ObjUtil_TextureGroupSetArt:

	// free full art name
	if ( *SaveArtName != NULL )
	{
		jeRam_Free( *SaveArtName );
		*SaveArtName = NULL;
	}

	// return failure
	return JE_FALSE;

} // ObjUtil_TextureGroupSetArt()
