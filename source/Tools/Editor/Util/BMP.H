/****************************************************************************************/
/*  BMP.H                                                                               */
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
#pragma once

#ifndef BMP_H
#define BMP_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(disable : 4201 4214 4115)
#include <Windows.h>
#include <Windowsx.h>
#pragma warning(default : 4201 4214 4115; disable : 4514)

#include "bitmap.h"
#include "jeMaterial.h"

HBITMAP		CreateHBitmapFromgeBitmap (jeBitmap *Bitmap, HDC hdc);
HBITMAP		Bmp_CreateEmpty24BitDIB( HDC hDC, int32 nWidth, int32 nHeight, void * pBits ) ;

// helper function prototypes
BITMAPINFO *PrepareRGBBitmapInfo(WORD wWidth, WORD wHeight);

void ShrinkDataInt(BYTE *pInBuff, 
                          WORD wWidth, 
                          WORD wHeight,
                          BYTE *pOutBuff, 
                          WORD wNewWidth, 
                          WORD wNewHeight);

void EnlargeDataInt(BYTE *pInBuff, 
                           WORD wWidth, 
                           WORD wHeight,
                           BYTE *pOutBuff, 
                           WORD wNewWidth, 
                           WORD wNewHeight);

jeBoolean CreateThumbnails(jeBitmap* pBmps, jeMaterialSpec_Thumbnail* pThumb);

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: bmp.h */