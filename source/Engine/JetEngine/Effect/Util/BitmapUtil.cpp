/****************************************************************************************/
/*  BITMAPUTIL.C                                                                        */
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
#include <assert.h>
#include "BitmapUtil.h"



////////////////////////////////////////////////////////////////////////////////////////
//
//	BitmapUtil_CreateFromFileName()
//
//	Create a bitmap from a file.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBitmap * BitmapUtil_CreateFromFileName(
	jeVFile		*File,		// file system to use
	const char	*Name,		// name of the file
	const char	*AlphaName,	// name of the alpha file
	jeBoolean	MipIt,		// whether or not to create mips for it
	jeBoolean	Sync )		// load it synchronously
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
		return NULL;
	}

	// create the bitmap
	Bmp = jeBitmap_CreateFromFile( BmpFile );
	jeVFile_Close( BmpFile );
	if ( Bmp == NULL )
	{
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
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// create alpha bitmap
		AlphaBmp = jeBitmap_CreateFromFile( AlphaFile );
		jeVFile_Close( AlphaFile );
		if ( AlphaBmp == NULL )
		{
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// set its alpha
		Result = jeBitmap_SetAlpha( Bmp, AlphaBmp );
		if ( Result == JE_FALSE )
		{
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

	// create mipmaps for it
	if ( MipIt == JE_TRUE )
	{
		//jeBitmap_SetMipCount( Bmp, 4 ); undone
	}

	// all done
	return Bmp;

} // BitmapUtil_CreateFromFileName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	BitmapUtil_LockBitmap()
//
//	Lock a bitmap for writing.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean BitmapUtil_LockBitmap(
	jeBitmap		*Bmp,			// bitmap to lock
	jePixelFormat	PixelFormat,	// pixel format to use
	jeBitmap		**LockedBitmap,	// where to store locked bitmap pointer
	uint8			**Bits,			// where to store bits pointer
	jeBitmap_Info	*BmpInfo )		// where to store bitmap info
{

	// locals
	jeBoolean	Success = JE_TRUE;
	jeBitmap	*DestLocked;

	// ensure valid data
	assert( Bmp != NULL );
	assert( LockedBitmap != NULL );
	assert( Bits != NULL );
	assert( BmpInfo != NULL );

	// lock the hud bitmap for writing
	if ( jeBitmap_LockForWriteFormat( Bmp, &DestLocked, 0, 0, PixelFormat ) == JE_FALSE )
	{
		Success = JE_FALSE;
		goto ALLDONE;
	}
	*LockedBitmap = DestLocked;
	if ( jeBitmap_GetInfo( DestLocked, BmpInfo, NULL ) == JE_FALSE )
	{
		Success = JE_FALSE;
		goto ALLDONE;
	}
	*Bits = jeBitmap_GetBits( DestLocked );
	if ( *Bits == NULL )
	{
		Success = JE_FALSE;
		goto ALLDONE;
	}

	// all done
	ALLDONE:
	return Success;

} // BitmapUtil_LockBitmap()



////////////////////////////////////////////////////////////////////////////////////////
//
//	BitmapUtil_UnLockBitmap()
//
//	Unlock a bitmap.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean BitmapUtil_UnLockBitmap(
	jeBitmap	*Bmp )	// bitmap to unlock
{

	// ensure valid data
	assert( Bmp != NULL );

	// unlock and return
	return jeBitmap_UnLock( Bmp );

} // BitmapUtil_UnLockBitmap()
