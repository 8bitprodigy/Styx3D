/****************************************************************************************/
/*  Bitmap.h                                                                            */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description:  Abstract Bitmap system                                                */
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
#ifndef BITMAP_H
#define BITMAP_H

#include "BaseType.h"
#include "PixelFormat.h"
#include "jePtrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

// from vfile.h
typedef	struct jeVFile jeVFile;

/***********************************************************************************/

typedef struct jeBitmap			jeBitmap;
typedef struct jeBitmap_Palette	jeBitmap_Palette;

typedef struct jeBitmap_Info
{
	int32					Width;
	int32					Height;
	int32					Stride;		// stride is in *pixels* ; it is the step to the next line : Stride >= Width
	jePixelFormat			Format;
	int32					MinimumMip;	//*including* minimumMip == 0 often
	int32					MaximumMip;	//*including* maximumMip == nummips-1
	jeBoolean				HasColorKey;
	uint32					ColorKey;	// meaningless unless HasColorKey ; the ColorKey is a Pixel in Format
	jeBitmap_Palette *		Palette;
} jeBitmap_Info;

/***********************************************************************************/		
// Bitmap methods

// see a big comment at the end of this file

/************************************************************************/

JETAPI jeBitmap *	JETCC	jeBitmap_Create(int32 Width, int32 Height, int32 MipCount, jePixelFormat Format ); 
JETAPI void			JETCC	jeBitmap_CreateRef(jeBitmap *Bmp);
JETAPI jeBitmap *	JETCC	jeBitmap_CreateCopy(const jeBitmap * Src);
JETAPI jeBitmap *	JETCC	jeBitmap_CreateFromInfo(const jeBitmap_Info * pInfo);

JETAPI jeBitmap *	JETCC	jeBitmap_CreateFromFile( jeVFile *F );
JETAPI jeBitmap *	JETCC	jeBitmap_CreateFromFileName(const jeVFile *BaseFS,const char *Name);
JETAPI jeBoolean 	JETCC	jeBitmap_WriteToFile( const jeBitmap *Bmp, jeVFile *F );
JETAPI jeBoolean	JETCC	jeBitmap_WriteToFileName(const jeBitmap * Bmp,const jeVFile *BaseFS,const char *Name);
										// BaseFS is not really const if it is a virtual file;
										//  it *is* const if it is a dos directory
JETAPI jeBitmap *	JETCC	jeBitmap_CreateFromFile2(jeVFile *VFile,jeVFile *ResourceBaseFS,jePtrMgr *PtrMgr);
JETAPI jeBoolean	JETCC	jeBitmap_WriteToFile2(const jeBitmap *Bmp,jeVFile *VFile,jePtrMgr *PtrMgr);
JETAPI jeBitmap *	JETCC	jeBitmap_CreateFromFileName2(const jeVFile *BaseFS,const char *Name,jePtrMgr *PtrMgr);
JETAPI jeBoolean	JETCC	jeBitmap_WriteToFileName2(const jeBitmap * Bmp,const jeVFile *BaseFS,const char *Name,jePtrMgr *PtrMgr);
										// the '2' functions try to write a minimal bitmap, either using the PtrMgr
										//	or writing a file name; they are NOT compatible with the original File IO funcs

JETAPI jeBoolean 	JETCC	jeBitmap_Destroy(jeBitmap **Bmp);
	// returns whether Bmp was actually destroyed : not success/failure

JETAPI jeBoolean	JETCC	jeBitmap_GetPersistableName(const jeBitmap *Bmp, jeVFile ** pBaseFS, char ** pName);

JETAPI jeBoolean 	JETCC	jeBitmap_GetInfo(const jeBitmap *Bmp, jeBitmap_Info *Info, jeBitmap_Info *SecondaryInfo);
	//LockForWrite returns data in Info's format

JETAPI jeBoolean 	JETCC	jeBitmap_Blit(const	jeBitmap *Src, int32 SrcPositionX, int32 SrcPositionY,
										jeBitmap *Dst, int32 DstPositionX, int32 DstPositionY,
										int32 SizeX, int32 SizeY );

JETAPI jeBoolean 	JETCC	jeBitmap_BlitMip(const jeBitmap * Src, int32 SrcMip, jeBitmap * Dst, int32 DstMip );
										// don't use this with Src == Dst, use UpdateMips instead !

JETAPI jeBoolean 	JETCC	jeBitmap_BlitBitmap(const jeBitmap * Src, jeBitmap * Dst);

JETAPI jeBoolean 	JETCC	jeBitmap_BlitBestMip(const jeBitmap * Src, jeBitmap * Dst);
										// blits the largest mip from Src that fits in Dst

JETAPI jeBoolean 	JETCC	jeBitmap_LockForRead(		// a non-exclusive lock
	const jeBitmap *	Bmp,
	jeBitmap **			Target,
	int32				MinimumMip,
	int32				MaximumMip,
	jePixelFormat 		Format,
	jeBoolean			RespectColorKey,
	uint32				ColorKey);
									// not really const, stores lock-count, but *data* is const
									// will do a format conversion!

JETAPI jeBoolean	JETCC	jeBitmap_LockForReadNative(
	const jeBitmap *	Bmp,
	jeBitmap **			Target,
	int32				MinimumMip,
	int32				MaximumMip);
									// lock for read in a format that gaurantee no conversions
									// then do GetInfo on the locks to see what you have!

JETAPI jeBoolean 	JETCC	jeBitmap_LockForWrite(	// an exclusive lock
	jeBitmap *			Bmp,
	jeBitmap **			Target,
	int32				MinimumMip,
	int32				MaximumMip);

JETAPI jeBoolean 	JETCC	jeBitmap_LockForWriteFormat(
	jeBitmap *			Bmp,
	jeBitmap **			Target,
	int32				MinimumMip,
	int32				MaximumMip,
	jePixelFormat 		Format);
									// Format must be one of the two returned in GetInfo !!

JETAPI jeBoolean 	JETCC	jeBitmap_UnLock(jeBitmap *Bmp);	// must be done on All locked mips
JETAPI jeBoolean 	JETCC	jeBitmap_UnLockArray(jeBitmap **Locks, int32 Size);

JETAPI jeBoolean 	JETCC	jeBitmap_SetFormat(jeBitmap *Bmp, 
							jePixelFormat NewFormat, 
							jeBoolean RespectColorKey, uint32 ColorKey,
							const jeBitmap_Palette * Palette);
	// _SetFormat may cause you to lose color information!
	// SetFormat does a conversion!
	// if NewFormat is palettized and Palette is NULL, we create a palette for the bitmap!

JETAPI jeBoolean 	JETCC	jeBitmap_SetFormatMin(jeBitmap *Bmp,jePixelFormat NewFormat);
								// the Min version keeps colorkey & palette from the old format

JETAPI jeBoolean	JETCC	jeBitmap_SetColorKey(jeBitmap *Bmp, jeBoolean HasColorKey, uint32 ColorKey, jeBoolean Smart);
	// SetColorKey discards old colorkey information!
	//	does not do a conversion (changes the colorkey in the current data
	// if 'Smart' is on, we don't set HasColorKey to true unless it is actually used!

JETAPI jeBoolean	JETCC	jeBitmap_GetAverageColor(const jeBitmap *Bmp,int32 *pR, int32 *pG, int32 *pB);
	// tells you the average color; computes it and caches it out

JETAPI jeBoolean	JETCC	jeBitmap_SetCompressionOptions(jeBitmap * Bmp, int32 CompressionLevel, jeBoolean NeedMips, jeFloat ratio);
	// CompressionOptions to be used in any later compression (not written to file)
	// level is 0-10	for 0 = fastest , 10 = most compression
	// NeedMips tells the coder whether to provide for fast mip creation in decompression
	// ratio is the *minimum* compression ratio (In_Len/Out_Len); higher ratios create loss
JETAPI jeBoolean	JETCC	jeBitmap_SetCompressionOptionsExpert(jeBitmap * Bmp,jeFloat Ratio,int32 TransformN,int32 CoderN,jeBoolean TransposeLHs,jeBoolean Block);

JETAPI jeBitmap_Palette * 	JETCC	jeBitmap_GetPalette(const jeBitmap *Bmp);
JETAPI jeBoolean			JETCC	jeBitmap_SetPalette(jeBitmap *Bmp, const jeBitmap_Palette *Palette);
	// _SetPal tries to _CreateRef your Palette, so no copy occurs & palettes may be shared
	// you may _Destroy() palette after using it to set (though its bits may not be freed)
	//	(hence Palette is *not* const)
	// Warning : SetPalette on any mip changes the palette of ALL mips !
	// see Palette note at _UnLock
	// _SetPal destroys the bitmap's original palette and refs the new one, 
	//		so if you setpal with the bitmap's palette, there is no net change in ref counts (good!)

JETAPI jeBoolean	JETCC	jeBitmap_HasAlpha(const jeBitmap * Bmp);
	// returns true if bitmap has *any* type of alpha

JETAPI jeBitmap *	JETCC	jeBitmap_GetAlpha(const jeBitmap *Bmp);
JETAPI jeBoolean 	JETCC	jeBitmap_SetAlpha(jeBitmap *Bmp, const jeBitmap *AlphaBmp);
	// we Ref the AlphaBmp, so you may destroy it after calling Set()
	// it may be NULL
	// there's only one Alpha per bitmap (for the top Mip) right now

JETAPI jeBoolean	JETCC	jeBitmap_SetGammaCorrection(jeBitmap *Bmp,jeFloat Gamma,jeBoolean Apply);
	// this Gamma does not change the *original* (system/secondary) bits
	//	it only affects the appearance when drawn
	// note : if you write to the gamma corrected bits, you must gamma correct manually if you
	//	wish to fit in smoothly with the previous data
	// warning : if you use this function with many different gammas, performance will suffer!
	//	use one global gamma for all bitmaps!  try to let the engine manage gamma for you,
	//	via jeEngine_SetGamma !

JETAPI jeBoolean		JETCC	jeBitmap_SetPreferredFormat(jeBitmap *Bmp,jePixelFormat Format);
JETAPI jePixelFormat	JETCC	jeBitmap_GetPreferredFormat(const jeBitmap *Bmp);

JETAPI void *		JETCC	jeBitmap_GetBits(jeBitmap *Bmp);	// works only on a Lock()

JETAPI jeBoolean 	JETCC	jeBitmap_RefreshMips(jeBitmap *Bmp);	// rebuilds mips; *tries* to be smart & not overwrite manually-fixed mips
												// RefreshMips does *not* build mips that don't exist
JETAPI jeBoolean 	JETCC	jeBitmap_UpdateMips(jeBitmap *Bmp,int32 SourceMip,int32 TargetMip);	
												// will create the target if it doesn't exist;
												// will overwrite manually-fixed mips!
JETAPI jeBoolean 	JETCC	jeBitmap_SetMipCount(jeBitmap *Bmp,int32 Count);
												// creates or destroys to match the new count

JETAPI jeBoolean 	JETCC	jeBitmap_ClearMips(jeBitmap *Bmp);	// Destroy all mips (except the first) !
												// use with care! this is not polite!

// Shortcuts
JETAPI int32		JETCC	jeBitmap_Width(const jeBitmap *Bitmap);
JETAPI int32		JETCC	jeBitmap_Height(const jeBitmap *Bitmap);
JETAPI uint32		JETCC	jeBitmap_MipBytes(const jeBitmap * Bitmap, int32 mip);

/**
*
* if Bitmap is a lock for read, functions that modify it return failure
* if Bitmap is a lock for write, functions that modify it attempt to
*	modify the owner of the lock
*
* warning : if you lock multiple mips for write, and then modify one of the mips
*		(such as via SetPalette) it may affect the owner and all sibling mips!
*		doing different SetPalettes with different palettes on different locked mips 
*		has undefined behavior!
*
**/

#ifdef _DEBUG
JETAPI uint32		JETCC	jeBitmap_Debug_GetCount(void);

	// assert this is zero before you shutdown !

#endif

/***********************************************************************************/

typedef enum
{
	JE_BITMAP_STREAMING_ERROR=0,
	JE_BITMAP_STREAMING_NOT,
	JE_BITMAP_STREAMING_STARTED,
	JE_BITMAP_STREAMING_IDLE,
	JE_BITMAP_STREAMING_CHANGED,
	JE_BITMAP_STREAMING_DATADONE,
	JE_BITMAP_STREAMING_DONE,
} jeBitmap_StreamingStatus;

JETAPI jeBitmap_StreamingStatus JETCC jeBitmap_GetStreamingStatus(const jeBitmap *Bmp);

		/** on a file which is streaming, the sequence of returns looks like :

			JE_BITMAP_STREAMING_IDLE
			JE_BITMAP_STREAMING_CHANGED
			JE_BITMAP_STREAMING_IDLE
			JE_BITMAP_STREAMING_IDLE
			JE_BITMAP_STREAMING_CHANGED
			...
			JE_BITMAP_STREAMING_DONE
			JE_BITMAP_STREAMING_NOT
			JE_BITMAP_STREAMING_NOT
			JE_BITMAP_STREAMING_NOT
			...

		Status >= JE_BITMAP_STREAMING_STARTED means streaming has started & is in progress

		the user should never see _STARTED or _DATADONE

		***/

/***********************************************************************************/

// palette methods :

JETAPI jeBitmap_Palette * 	JETCC	jeBitmap_Palette_Create(jePixelFormat Format, int32 Size);

JETAPI jeBitmap_Palette * 	JETCC	jeBitmap_Palette_CreateCopy(const jeBitmap_Palette *Palette);

JETAPI jeBitmap_Palette *	JETCC	jeBitmap_Palette_CreateFromFile(jeVFile *F);

JETAPI jeBitmap_Palette *	JETCC	jeBitmap_Palette_CreateFromBitmap(jeBitmap * Bmp,jeBoolean Slow);
												// does GetPalette, and if NULL, then
												// it create an optimal palette for a
												//	non-palettized bitmap
												//	(this is a create, you must destroy later!)
												// put Slow == TRUE for higher quality & slower

JETAPI jeBoolean		JETCC	jeBitmap_Palette_SortColors(jeBitmap_Palette * P,jeBoolean Slower);

JETAPI jeBoolean		JETCC	jeBitmap_Palette_CreateRef(jeBitmap_Palette *Palette);

JETAPI jeBoolean		JETCC	jeBitmap_Palette_Destroy(jeBitmap_Palette ** ppPalette);

JETAPI jeBoolean		JETCC	jeBitmap_Palette_WriteToFile(const jeBitmap_Palette *Palette,jeVFile *F);

JETAPI jeBoolean		JETCC	jeBitmap_Palette_SetFormat(jeBitmap_Palette * Palette,jePixelFormat Format);

JETAPI jeBoolean		JETCC	jeBitmap_Palette_Copy(const jeBitmap_Palette * Src,jeBitmap_Palette * Target);

JETAPI jeBoolean		JETCC	jeBitmap_Palette_GetInfo(const	jeBitmap_Palette *P,jeBitmap_Info *Into);
												// get the info as if it were a bitmap; Into->Height == 1

JETAPI jeBoolean		JETCC	jeBitmap_Palette_Lock(jeBitmap_Palette *Palette, void **pBits, jePixelFormat *pFormat, int32 *pSize);
												// pFormat & pSize are optional

JETAPI jeBoolean		JETCC	jeBitmap_Palette_UnLock(jeBitmap_Palette *Palette);
											// palette unlock does NOT notify the bitmap that the palette has changed.
											// call Bitmap_SetPalette() with the same palette pointer 
											// 	to tell the bitmap that it must to some processing
											// (don't worry, it won't duplicate it or copy it onto itself)

JETAPI jeBoolean		JETCC	jeBitmap_Palette_GetData(const jeBitmap_Palette *P,      void *Into,jePixelFormat Format, int32 Colors);
JETAPI jeBoolean		JETCC	jeBitmap_Palette_SetData(      jeBitmap_Palette *P,const void *From,jePixelFormat Format, int32 Colors);
											// does Lock/UnLock for you
											// From and Into are arrays of Colors*jePixelFormat_BytesPerPel bytes

JETAPI jeBoolean		JETCC	jeBitmap_Palette_SetEntryColor(      jeBitmap_Palette *P, int32 Color, int32 R, int32 G, int32 B, int32 A);
JETAPI jeBoolean		JETCC	jeBitmap_Palette_GetEntryColor(const jeBitmap_Palette *P, int32 Color, int32 *R, int32 *G, int32 *B, int32 *A);
											// Set/Get does Lock/Unlock for you ; these are slow! do not use these to work on all the colors!

JETAPI jeBoolean		JETCC	jeBitmap_Palette_SetEntry(      jeBitmap_Palette *P, int32 Color, uint32 Pixel);
JETAPI jeBoolean		JETCC	jeBitmap_Palette_GetEntry(const jeBitmap_Palette *P, int32 Color, uint32 *Pixel);

/***********************************************************************************/

/************************************************************************

A brief tutorial on the Bitmap system, by Charles Bloom, cbloom@eclipsegames.com

The Bitmap is a smart wrapper for complex functionality.  You give it hints to
the opaque Bitmap object, and it tries its best to follow those hints, but it
may not always do so.  The Bitmap is the owner of its bits; you must Lock the
bitmap to get permission to touch those bits, and UnLock to tell the bitmap
you are done.  The format may change between two Locks.  Bitmaps can also be
multiply owned, so you should account for the fact that others may touch your
bitmap between your uses.

The Bitmap contains one or two pixel-sets representing an image.  The "primary" is
a fast-blitting version of the image, and the "secondary" is a storage version
(eventually wavelet compressed) which can be used to rebuild the primary if it is
freed or damaged.  Both cary a generalized format.

Let's do an example.  I want to load a bitmap, set it up for drawing with the
Jet3D Engine, and then blit some interactive stuff into it.

************************************************************************/

#if 0
// {
//-----------------------------------------------------------------------------

void Init(jeEngine * Engine);
void Shutdown(void);
void Draw(void);
void DrawPolite(void);

static jeBitmap * myBM = NULL;
static jeEngine * myEngine = NULL;

void Init(jeEngine * Engine)
{
jeBoolean success;
jeBitmap_Info Info;

	myEngine = Engine;	// this is not looked well upon; for ease of demonstration only!
	assert(Engine);

	myBM = jeBitmap_CreateFromFileName(NULL,"mybitmap.bmp");

	// CreateFromFile can load windows BMP files, or custom GeBm files.

	assert(myBM);

	// get the main info; I don't care about the secondary, so leave it NULL

	success = jeBitmap_GetInfo(myBM,&Info,NULL);
	assert(success);

	// make sure I loaded a bitmap in the format I understand !

	if ( Info.Format == JE_PIXELFORMAT_8BIT_PAL )
	{
		// I want palette index 255 to act as transparency, so I must use SetColorKey

		success = jeBitmap_SetColorKey(myBM,JE_TRUE,255);
		assert(success);

		// just for fun, let's modify the palette:
		if (1)
		{
		jeBitmap_Palette * Pal;

			// get the palette ; I don't care if its primary or secondary, so
			/// I don't use the Info.Palette field

			Pal = jeBitmap_GetPalette(myBM);
			assert(Pal);

			// I'm only fiddling one entry, so don't bother with a full Lock() UnLock()
			//  sequence on the palette

			// make palette index zero bright red; we use alpha = 255 for opaque

			success = jeBitmap_Palette_SetEntryColor(Pal,0,255,0,0,255);
			assert(success);

			// tell the bitmap system you've changed the palette; this function
			//  is smart enough to not do unecessary copies or whatever.

			success = jeBitmap_SetPalette(myBM,Pal);
			assert(success);
		}

	}
	else
	{
		// otherwise, treat black as transparent, in whatever format I have

		success = jeBitmap_SetColorKey(myBM,JE_TRUE,jePixelFormat_ComposePixel(Info.Format,0,0,0,0));
		assert(success);
	}	

	// note that I did NOT use SetFormat.  SetFormat may do a conversion, and since the original
	//	bitmap was created without colorkey, it would have been converted to a new format but
	//	kept its property of having no colorkey!
	// (SetFormat will fiddle the bits and whatever way necessary to keep bitmaps as visually similar
	//		as possible)

	// I want to fiddle the fast format in 565 later, so cue the bitmap to try to give me that format.

	success = jeBitmap_SetPreferredFormat(myBM,JE_PIXELFORMAT_16BIT_565_RGB);
	assert(success);

	// Add it to the engine so it can be used for drawing.

	success = jeEngine_AddBitmap(myEngine,myBM);
	assert(success);
}

void Shutdown(void)
{
jeBoolean WasDestroyed;

	assert(myBM);
	
	// clean up

	jeEngine_RemoveBitmap(myEngine,myBM);

	WasDestroyed = jeBitmap_Destroy(&myBM);

	// someone else might have done _CreateRef on our bitmap,
	//  so we can't be sure it's actually destroyed.
	// this code is still ready to be run again with a new call to Init()

	//assert(WasDestroyed);

	myBM = NULL;
	myEngine = NULL;
}

void Draw(void)
{
jeBitmap * Lock;
jeBoolean success;
jeBitmap_Info Info;
uint16 *bits,*bptr;
int x,y;

	// lets fiddle the bits.
	// we need to lock the bitmap for write.
	//	LockForWrite is an exclusive lock, unlike LockForRead which is non-blocking
	// request our favorite format, and only lock Mip 0 (the full size bitmap)

	success = jeBitmap_LockForWriteFormat(myBM,&Lock,0,0,JE_PIXELFORMAT_16BIT_565_RGB);
	if ( ! success )
	{
		// well, we tried to be nice; if we were very polite, we would do a LockForWrite
		// here, and try to fiddle the bits in whatever format we got; However, we aren't
		// that polite, so we just do a _SetFormat
		//
		// note that we are destroying the original bitmap by changing its format
		// we should only do this if we are going to draw into the bitmap

		success = jeBitmap_SetFormat(myBM,JE_PIXELFORMAT_16BIT_565_RGB,JE_TRUE,0,NULL);
		assert(success);

		// now we should be able to get the bits we want, *but* they may not be the
		// primary (fast) format; oh well, it's the best we can do...
		// (if you must have the fastest bits, then use only _LockForWrite, never LockForWriteFormat,
		// which might have to do a conversion)

		success = jeBitmap_LockForWriteFormat(myBM,&Lock,0,0,JE_PIXELFORMAT_16BIT_565_RGB);
		assert(success);
	}

	// now Lock is our bitmap in 565
	// we do a GetInfo because the Lock's info could be different than
	//	the original bitmap's (particularly the Palette & the Stride)

	success = jeBitmap_GetInfo(Lock,&Info,NULL);
	assert(success);

	// you can only call _GetBits on a locked bitmap

	bits = jeBitmap_GetBits(Lock);
	assert( bits );

	bptr = bits;
	for(y=0; y < Info.Height; y++)
	{
		for(x=0; x < Info.Width; x++)
		{
		uint16 R,G,B;
			// make a silly 565 gradient
			R = x & 0x1F;
			G = x & 0x3F;
			B = y & 0x1F;

			*bptr++ = (R<<11) + (G<<5) + B;
		}

		// note that bptr is a word pointer, and Stride is in pixels :

		bptr += Info.Stride -  Info.Width;
	}
	bits = bptr = NULL;

	// you call Unlock on all the mips you locked - not on the original bitmap!

	success = jeBitmap_UnLock(Lock);
	assert(success);

	// now, we only fiddled the full-size Mip, and there might be more,
	//  so lets percolate the changes into the smaller mips:

	success = jeBitmap_RefreshMips(myBM);
	assert(success);

	// a null rect means use the whole bitmap;
	// Engine_DrawBitmap blits a 2d decal to the framebuffer (fast)

	success = jeEngine_DrawBitmap(myEngine,myBM,NULL,0,0);
	assert(success);

}

void DrawPolite(void)
{
jeBitmap * Lock;
jeBoolean success;
jeBitmap_Info Info;
void *bits;
int x,y;

	// this function does the same thing as Draw() , but is more polite
	// lock in the fastest format (whatever it is)
	// because we did SetPreferred, this should be 565_RGB, but might not be

	success = jeBitmap_LockForWrite(myBM,&Lock,0,0);
	assert(success);

	success = jeBitmap_GetInfo(Lock,&Info,NULL);
	assert(success);

	bits = jeBitmap_GetBits(Lock);
	assert( bits );

	if ( Info.Format == JE_PIXELFORMAT_16BIT_565_RGB )
	{
	uint16 *wptr;

		// our favorite format

		wptr = bits;
		for(y=0; y < Info.Height; y++)
		{
			for(x=0; x < Info.Width; x++)
			{
			uint16 R,G,B;
				// make a silly 565 gradient
				R = x & 0x1F;
				G = x & 0x3F;
				B = y & 0x1F;

				*wptr++ = (R<<11) + (G<<5) + B;
			}
			wptr += Info.Stride -  Info.Width;
		}
	}
	else
	{
	uint8 * bptr;

		// oh well, do our best
		// bitmaps must have had a good reason to not give us the format we preferred,

		bptr = bits;
		for(y=0; y < Info.Height; y++)
		{
			for(x=0; x < Info.Width; x++)
			{
			uint32 R,G,B;

				// put a color in any format

				R = (x & 0x1F)<<3;
				G = (x & 0x3F)<<2;
				B = (y & 0x1F)<<3;

				// we use alpha of 255 for opaque

				jePixelFormat_PutColor(Info.Format,&bptr,R,G,B,255);
			}

			bptr += (Info.Stride -  Info.Width) * jePixelFormat_BytesPerPel(Info.Format);
		}
	}
	bits = NULL;

	// same as before:

	success = jeBitmap_UnLock(Lock);
	assert(success);

	success = jeBitmap_RefreshMips(myBM);
	assert(success);

	success = jeEngine_DrawBitmap(myEngine,myBM,NULL,0,0);
	assert(success);

}

// end tutorial on the Bitmap system
//-----------------------------------------------------------------------------
// }

/***********************************************************************************/

#endif
#ifdef __cplusplus
}
#endif


#endif
