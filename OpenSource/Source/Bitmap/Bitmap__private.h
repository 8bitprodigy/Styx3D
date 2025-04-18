#ifndef GE_BITMAP____H
#define GE_BITMAP____H

/****************************************************************************************/
/*  Bitmap.__h                                                                          */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description:  Bitmap*.c Internal Header (contains the Bitmap Struct)                */
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/

#include "Bitmap.h"
#include "Bitmap_private.h"

// Hey ! 
// this is bitmap.__h : for inclusion by bitmap friends ONLY!
//	included by bitmap.c , bitmap_blitdata.c

/*}{ *********** the bitmap type *******************/

// the Version Major indicates an incompatibility

#define GEBM_VERSION_MAJOR		(0x0004)
#define GEBM_VERSION_MINOR		(0x0000)

#define	MAXMIPLEVELS			(8)

#define PALETTE_FORMAT_DEFAULT	(GE_PIXELFORMAT_24BIT_RGB)

#define ALPHA_TO_TRANSPARENCY_THRESHOLD		(80)

struct geBitmap_Palette
{
	int				LockCount,RefCount;
	gePixelFormat	Format;
	int				Size;
	geBoolean		HasColorKey;
	uint32			ColorKey;		// the color which is the colorkey
	int				ColorKeyIndex;	// the palette index which is the color;
									// Bitmap->CK == Bitmap->Pal->CK_Index

	// we have EITHER Data or DriverHandle
	void			*Data;		//Size * BytesPerPixel(Format)
	DRV_Driver		*Driver;
	geRDriver_THandle*DriverHandle;
	void			*DriverBits;	//only non-null inside a Lock/UnLock
};

struct geBitmap
{
	int 				RefCount;
	geBitmap_Info		Info;
	void *				Data[MAXMIPLEVELS];
	geBoolean			Modified[MAXMIPLEVELS];
							// modified tells whether a mip != a scaledown of mip 0
							// modified[0] is ignored
	geBitmap *			Alpha;

	int					LockCount;	// -Nmips for 'write' , > 0 for 'read's
	geBitmap *			LockOwner;	// this points to our owner and doubles as boolean 'islocked'
	geBitmap *			DataOwner;	// if this is set, then my Data is not mine to free

	gePixelFormat		PreferredFormat;	// user's favorite
	int					SeekMipCount;		// when we attach to driver, ask for this many mips

		// must support any number of locks for read
		// a lock for read can be a pointer to my raw bits, or a whole different bitmap

	geBitmap_Info		DriverInfo;			// all the driver mess..
	uint32				DriverFlags;
	DRV_Driver *		Driver;
	geRDriver_THandle *	DriverHandle;
	int					DriverMipLock;		// which mip to lock on GetBits
	geBoolean			DriverBitsLocked;
	geBoolean			DriverDataChanged;	// relative to system copy
	geFloat				DriverGamma;
	geFloat				DriverGammaLast;
	geBoolean			DriverGammaSet;
};


/*}{ ************* internal protos *****************/

//geBitmap * geBitmap_CreateXerox(geBitmap *BmpSrc);

geBoolean geBitmap_IsValid(const geBitmap *Bmp);
geBoolean geBitmap_Info_IsValid(const geBitmap_Info *Info);
geBoolean geBitmap_Palette_IsValid(const geBitmap_Palette *Pal);

geBoolean geBitmap_BlitMipRect(const geBitmap * Src, int SrcMip, int SrcX,int SrcY,
									 geBitmap * Dst, int DstMip, int DstX,int DstY,
							int SizeX,int SizeY);

geBitmap * geBitmap_CreateLock_CopyInfo(geBitmap *BmpSrc,int LockCnt,int mip);
geBitmap * geBitmap_CreateLockFromMip(geBitmap *Src,int mip,
	gePixelFormat Format,geBoolean HasColorKey,uint32 ColorKey,int LockCnt);
geBitmap * geBitmap_CreateLockFromMipSystem(geBitmap *Src,int mip,int LockCnt);
geBitmap * geBitmap_CreateLockFromMipOnDriver(geBitmap *Src,int mip,int LockCnt);

geBoolean geBitmap_UnLock_NoChange(geBitmap *Bmp);
geBoolean geBitmap_UnLockArray_NoChange(geBitmap **Locks,int Size);

geBoolean geBitmap_Update_SystemToDriver(geBitmap *Bmp);
geBoolean geBitmap_Update_DriverToSystem(geBitmap *Bmp);

geBoolean geBitmap_MakeSystemMips(geBitmap *Bmp,int low,int high);
geBoolean geBitmap_UpdateMips_Data(	geBitmap_Info * FmInfo,void * FmBits,
									geBitmap_Info * ToInfo,void * ToBits);
geBoolean geBitmap_UpdateMips_System(geBitmap *Bmp,int fm,int to);

geBoolean geBitmap_UsesColorKey(const geBitmap * Bmp);

void	  geBitmap_MakeMipInfo(  geBitmap_Info *Src,int mip,geBitmap_Info *Into);
geBoolean geBitmap_MakeDriverLockInfo(geBitmap *Bmp,int mip,geBitmap_Info *Into);
					// MakeDriverLockInfo also doesn't full out the full info, so it must be a valid info first!
					// Bmp also gets some crap written into him.

geBoolean geBitmap_AllocSystemMip(geBitmap *Bmp,int mip);
geBoolean geBitmap_AllocPalette(geBitmap *Bmp,gePixelFormat Format,DRV_Driver * Driver);

geBoolean geBitmap_ReadInfo( geBitmap *Bmp,geVFile * F);
geBoolean geBitmap_WriteInfo(const geBitmap *Bmp,geVFile * F);

geBoolean geBitmap_FixDriverFlags(uint32 *pFlags);

/*}{ ************* end *****************/

#endif
