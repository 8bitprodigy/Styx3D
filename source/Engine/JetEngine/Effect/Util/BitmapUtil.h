/****************************************************************************************/
/*  BITMAPUTIL.H                                                                        */
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
#ifndef BITMAPUTIL_H
#define BITMAPUTIL_H

#include "Jet.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Create a bitmap from a file.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBitmap * BitmapUtil_CreateFromFileName(
	jeVFile		*File,		// file system to use
	const char	*Name,		// name of the file
	const char	*AlphaName,	// name of the alpha file
	jeBoolean	MipIt,		// whether or not to create mips for it
	jeBoolean	Sync );		// load it synchronously

//	Lock a bitmap for writing.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean BitmapUtil_LockBitmap(
	jeBitmap		*Bmp,			// bitmap to lock
	jePixelFormat	PixelFormat,	// pixel format to use
	jeBitmap		**LockedBitmap,	// where to store locked bitmap pointer
	uint8			**Bits,			// where to store bits pointer
	jeBitmap_Info	*BmpInfo );		// where to store bitmap info

//	Unlock a bitmap.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean BitmapUtil_UnLockBitmap(
	jeBitmap	*Bmp );	// bitmap to unlock


#ifdef __cplusplus
	}
#endif

#endif
