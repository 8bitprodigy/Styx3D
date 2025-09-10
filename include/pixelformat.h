/****************************************************************************************/
/*  PixelFormat.h                                                                       */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description:  The abstract Pixel primitives                                         */
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
#ifndef	PIXELFORMAT_H
#define	PIXELFORMAT_H

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum		// all supported formats (including shifts)
{
	JE_PIXELFORMAT_NO_DATA = 0,
	JE_PIXELFORMAT_8BIT,				// PAL
	JE_PIXELFORMAT_8BIT_GRAY,		// no palette (intensity from bit value)
	JE_PIXELFORMAT_16BIT_555_RGB,
	JE_PIXELFORMAT_16BIT_555_BGR,
	JE_PIXELFORMAT_16BIT_565_RGB,	// #5
	JE_PIXELFORMAT_16BIT_565_BGR, 
	JE_PIXELFORMAT_16BIT_4444_ARGB, // #7
	JE_PIXELFORMAT_16BIT_1555_ARGB, 
	JE_PIXELFORMAT_24BIT_RGB,		// #9
	JE_PIXELFORMAT_24BIT_BGR,
	JE_PIXELFORMAT_24BIT_YUV,		// * see note below
	JE_PIXELFORMAT_32BIT_RGBX, 
	JE_PIXELFORMAT_32BIT_XRGB, 
	JE_PIXELFORMAT_32BIT_BGRX, 
	JE_PIXELFORMAT_32BIT_XBGR,
	JE_PIXELFORMAT_32BIT_RGBA, 
	JE_PIXELFORMAT_32BIT_ARGB,		// #17
	JE_PIXELFORMAT_32BIT_BGRA, 
	JE_PIXELFORMAT_32BIT_ABGR,
	
	JE_PIXELFORMAT_WAVELET,			// #20 , Wavelet Compression

	JE_PIXELFORMAT_COUNT
} jePixelFormat;
	
/******

there's something wacked out about these format names :

	for 16 bit & 32 bit , the _RGB or _BGR refers to their order
		*in the word or dword* ; since we're on intel, this means
		the bytes in the data file have the *opposite* order !!
		(for example the 32 bit _ARGB is actually B,G,R,A in raw bytes)
	for 24 bit , the _RGB or _BGR refers to their order in the
		actual bytes, so that windows bitmaps actually have
		_RGB order in a dword !!

* YUV : the pixelformat ops here are identical to those of 24bit_RGB ;
		this is just a place-keeper to notify you that you should to a YUV_to_RGB conversion

*********/

#define JE_PIXELFORMAT_8BIT_PAL JE_PIXELFORMAT_8BIT

typedef uint32	(*jePixelFormat_Composer   )(int R,int G,int B,int A);
typedef void	(*jePixelFormat_Decomposer )(uint32 Pixel,int *R,int *G,int *B,int *A);

typedef void	(*jePixelFormat_ColorGetter)(uint8 **ppData,int *R,int *G,int *B,int *A);
typedef void	(*jePixelFormat_ColorPutter)(uint8 **ppData,int  R,int  G,int  B,int  A);

typedef uint32	(*jePixelFormat_PixelGetter)(uint8 **ppData);
typedef void	(*jePixelFormat_PixelPutter)(uint8 **ppData,uint32 Pixel);

typedef struct jePixelFormat_Operations
{
	uint32	RMask;
	uint32	GMask;
	uint32	BMask;
	uint32	AMask;

	int		RShift;
	int		GShift;
	int		BShift;
	int		AShift;

	int		RAdd;
	int		GAdd;
	int		BAdd;
	int		AAdd;

	int			BytesPerPel;
	jeBoolean	HasPalette;
	char *		Description;
	
	jePixelFormat_Composer		ComposePixel;
	jePixelFormat_Decomposer	DecomposePixel;

	jePixelFormat_ColorGetter	GetColor;
	jePixelFormat_ColorPutter	PutColor;

	jePixelFormat_PixelGetter	GetPixel;
	jePixelFormat_PixelPutter	PutPixel;
} jePixelFormat_Operations;

	// the Masks double as boolean "HaveAlpha" .. etc..

JETAPI const jePixelFormat_Operations * JETCC jePixelFormat_GetOperations( jePixelFormat Format );

	// quick accessors to _GetOps
JETAPI jeBoolean	JETCC jePixelFormat_IsValid(		jePixelFormat Format);
JETAPI unsigned int JETCC jePixelFormat_BytesPerPel(	jePixelFormat Format );
JETAPI jeBoolean	JETCC jePixelFormat_HasPalette(		jePixelFormat Format );
JETAPI jeBoolean	JETCC jePixelFormat_HasAlpha(		jePixelFormat Format );
JETAPI jeBoolean	JETCC jePixelFormat_HasGoodAlpha(	jePixelFormat Format ); // more than 1 bit of alpha
JETAPI const char * JETCC jePixelFormat_Description(	jePixelFormat Format );
JETAPI jeBoolean	JETCC jePixelFormat_IsRaw(			jePixelFormat Format );
									// 'Raw' means pixels can be made with the Compose operations

JETAPI uint32		JETCC jePixelFormat_ComposePixel(	jePixelFormat Format,int R,int G,int B,int A);
JETAPI void			JETCC jePixelFormat_DecomposePixel(	jePixelFormat Format,uint32 Pixel,int *R,int *G,int *B,int *A);
			
															// these four functions move ppData to the next pixel

JETAPI void			JETCC jePixelFormat_GetColor(jePixelFormat Format,uint8 **ppData,int *R,int *G,int *B,int *A);
JETAPI void			JETCC jePixelFormat_PutColor(jePixelFormat Format,uint8 **ppData,int R,int G,int B,int A);

JETAPI uint32		JETCC jePixelFormat_GetPixel(jePixelFormat Format,uint8 **ppData);
JETAPI void			JETCC jePixelFormat_PutPixel(jePixelFormat Format,uint8 **ppData,uint32 Pixel);
	
JETAPI uint32		JETCC jePixelFormat_ConvertPixel(jePixelFormat Format,uint32 Pixel,jePixelFormat ToFormat);


#ifdef __cplusplus
}
#endif

#endif
