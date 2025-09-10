/****************************************************************************************/
/*  Bitmap.c                                                                            */
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
#define DONT_DEC_STREAMING	// <> doesn't decompress a streaming file
							// when it is attached to a driver

/**********
***
*
-----------------------------------------------
NOTEZ BIEN !

	If your name is not Charles, and you make a change to this file,
	please comment it!  write //Initials around changes!

-----------------------------------------------


see @@ for urgent todos !
see <> for todos
see {} for notes/long-term-todos

-------

{} when palettizing lots of mips, blit them together, so we only make
	one closestPal palInfo for all the blits (big speedup).
	(for UpdateMips too)
	perhaps the only way is to keep a cache of the last palInfo made and
	check the palette to see if its reusable.  Check on memory cost of the palInfo.

{} _Lock and _UnLock should percolate up the ->DataOwner chain , so you can't make
	a DataOwner Xerox and then lock the original for read & the copy for write
	(that's a no-no cuz you might get the same bits )

{} when we blit from one set of mips to another, we could copy the palette (or build the palette!)
	many times.  We must do this in general, because the mips could all have different palettes.
	The answer is to keep track of "palette was just copied from X"
	(actually, setting different palettes on two locked mips will cause bad data!!)

{} make a _BlitOnto which merges with alpha?

{} BTW FYI :
		1. glide 4444 surfaces do alpha and colorkey ; colorkey overrides alpha
		2. we're preferring 1555 over colorkey now

*
***
 ********/

#include	<math.h>
#include	<time.h>
#include	<stdio.h>
#include	<assert.h>
#include	<stdlib.h>
#include	<string.h>

#include	"BaseType.h"
#include	"jeTypes.h"
#include	"Ram.h"

#include	"VFile.h"
#include	"Errorlog.h"
#include	"Log.h"
#include	"ThreadLog.h"
#include	"MemPool.h"

#include	"Bitmap.h"
#include	"Bitmap._h"
#include	"Bitmap.__h"
#include	"bitmap_blitdata.h"
#include	"bitmap_gamma.h"

#include	"Wavelet.h"
#include	"palcreate.h"
#include	"palettize.h"
#include	"CodePal.h"
#include	"SortPal.h"
#include	"SortPalx.h"

#include	"ThreadQueue.h"

#ifdef BUILD_BE
#define strnicmp strncasecmp
#endif

#ifdef DO_TIMER
#include	"Timer.h"
#endif

#define allocate(ptr)	ptr = jeRam_Allocate(sizeof(*ptr))
#define clear(ptr)		memset(ptr,0,sizeof(*ptr))

#define SHIFT_R_ROUNDUP(val,shift)	(((val)+(1<<(shift)) - 1)>>(shift))

/*}{ ************* statics *****************/

//#define DO_TIMER

//#define DONT_USE_ASM

#ifdef _DEBUG
#define Debug(x)	x
static int _Bitmap_Debug_ActiveCount = 0;
static int _Bitmap_Debug_ActiveRefs = 0;
#else
#define Debug(x)
#endif

static int32 BitmapInit_RefCount = 0;
static MemPool * BitmapPool = NULL;
jeThreadQueue_Semaphore * Bitmap_Gamma_Lock = NULL;
jeThreadQueue_Semaphore * Bitmap_BlitData_Lock = NULL;

void jeBitmap_Start(void)
{
	if ( BitmapInit_RefCount == 0 )
	{
		BitmapPool = MemPool_Create(sizeof(jeBitmap),100,100);
		assert(BitmapPool);
		Palettize_Start();
		PalCreate_Start();
		Bitmap_Gamma_Lock = jeThreadQueue_Semaphore_Create();
		assert(Bitmap_Gamma_Lock);
		Bitmap_BlitData_Lock = jeThreadQueue_Semaphore_Create();
		assert(Bitmap_BlitData_Lock);
	}
	BitmapInit_RefCount ++;
}

void jeBitmap_Stop(void)
{
	assert(BitmapInit_RefCount > 0 );
	BitmapInit_RefCount --;
	if ( BitmapInit_RefCount == 0 )
	{
		assert(BitmapPool);
		MemPool_Destroy(&BitmapPool);
		Palettize_Stop();
		PalCreate_Stop();
		assert(Bitmap_Gamma_Lock);
		jeThreadQueue_Semaphore_Destroy(&Bitmap_Gamma_Lock);
		assert(Bitmap_BlitData_Lock);
		jeThreadQueue_Semaphore_Destroy(&Bitmap_BlitData_Lock);
	}
}

/*}{ ******** Creator Functions **********************/

jeBitmap * jeBitmap_Create_Base(void)
{
jeBitmap * Bmp;

	jeBitmap_Start();

	Bmp = (jeBitmap *)MemPool_GetHunk(BitmapPool);

	Bmp->RefCount = 1;

	Bmp->DriverGamma = Bmp->DriverGammaLast = 1.0f;

	Debug(_Bitmap_Debug_ActiveRefs ++);
	Debug(_Bitmap_Debug_ActiveCount ++);

return Bmp;
}

void jeBitmap_Destroy_Base(jeBitmap *Bmp)
{
	assert(Bmp);
	assert(Bmp->RefCount == 0);
	Debug(_Bitmap_Debug_ActiveCount --);

	MemPool_FreeHunk(BitmapPool,Bmp);

	jeBitmap_Stop();
}

JETAPI jeBitmap *	JETCC	jeBitmap_CreateCopy(const jeBitmap * Src)
{
jeBitmap_Info Info;
jeBitmap * Ret;

	assert( Src );
	if ( ! jeBitmap_GetInfo(Src,&Info,NULL) )
		return NULL;

	Info.MaximumMip = Info.MinimumMip = 0;

	Ret = jeBitmap_CreateFromInfo(&Info);
	if ( ! Ret )
		return NULL;

	if ( ! jeBitmap_BlitBitmap(Src,Ret) )
	{
		jeBitmap_Destroy(&Ret);
		return NULL;
	}

	jeBitmap_SetMipCount(Ret,Src->SeekMipCount);

return Ret;
}

JETAPI void JETCC	jeBitmap_CreateRef(jeBitmap *Bmp)
{
	assert(Bmp);
	Bmp->RefCount ++;
	Debug(_Bitmap_Debug_ActiveRefs ++);
}

JETAPI jeBitmap *	JETCC	jeBitmap_Create(
	int32					 Width,
	int32					 Height,
	int32					 MipCount,
	jePixelFormat Format)
{
jeBitmap * Bmp;

	Bmp = jeBitmap_Create_Base();
	if ( ! Bmp )
		return NULL;

	assert( Width > 0 );
	assert( Height > 0 );
	if ( MipCount == 0 )
		MipCount = 1;
	assert( MipCount > 0 );

	Bmp->Info.Width = Width;
	Bmp->Info.Stride = Width;
	Bmp->Info.Height = Height;
	Bmp->Info.Format = Format;

	Bmp->Info.MinimumMip = 0;
	Bmp->Info.MaximumMip = 0;
	Bmp->Info.HasColorKey = JE_FALSE;

	Bmp->SeekMipCount = MipCount;

	if ( Format == JE_PIXELFORMAT_WAVELET )
	{
		Bmp->Wavelet = jeWavelet_CreateEmpty(Width,Height);
	}

return Bmp;
}

JETAPI jeBitmap *	JETCC	jeBitmap_CreateFromInfo(const jeBitmap_Info * pInfo)
{
jeBitmap * Bmp;

	assert(pInfo);
	assert(jeBitmap_Info_IsValid(pInfo));

	Bmp = jeBitmap_Create_Base();
	if ( ! Bmp )
		return NULL;

	Bmp->Info = *pInfo;

	if ( Bmp->Info.Stride < Bmp->Info.Width )
		Bmp->Info.Stride = Bmp->Info.Width;

	if ( Bmp->Info.Palette )
		jeBitmap_Palette_CreateRef(Bmp->Info.Palette);

	if ( Bmp->Info.Format == JE_PIXELFORMAT_WAVELET )
	{
		Bmp->Wavelet = jeWavelet_CreateEmpty(Bmp->Info.Width,Bmp->Info.Height);
	}

return Bmp;
}

JETAPI jeBoolean	JETCC	 jeBitmap_Destroy(jeBitmap **Bmp)
{
int			i;
jeBitmap *	Bitmap;

	assert(Bmp);

	Bitmap = *Bmp;

	if ( Bitmap )
	{
		if ( Bitmap->LockOwner )
		{
			return jeBitmap_UnLock(Bitmap);
		}

		if ( Bitmap->RefCount <= 1 )
		{
			if ( Bitmap->DataOwner )
			{
				jeBitmap_Destroy(&(Bitmap->DataOwner));
				Bitmap->DataOwner = NULL;
			}
			else
			{
				if ( Bitmap->Driver )
				{
					jeBitmap_DetachDriver(Bitmap,JE_FALSE);
				}

				for	(i = Bitmap->Info.MinimumMip; i <= Bitmap->Info.MaximumMip; i++)
				{
					if	(Bitmap->Data[i])
						jeRam_Free(Bitmap->Data[i]);
				}

				if ( Bitmap->Wavelet )
				{
					jeWavelet_Destroy(&(Bitmap->Wavelet));
				}
			}
		}

		Debug(assert(_Bitmap_Debug_ActiveRefs > 0));
		Debug(_Bitmap_Debug_ActiveRefs --);

		Bitmap->RefCount --;

		if ( Bitmap->RefCount <= 0 )
		{
			if	(Bitmap->Alpha)
			{
				jeBitmap_Destroy(&Bitmap->Alpha);
			}

			if	(Bitmap->Info.Palette)
			{
				jeBitmap_Palette_Destroy(&(Bitmap->Info.Palette));
			}

			if	(Bitmap->DriverInfo.Palette)
			{
				jeBitmap_Palette_Destroy(&(Bitmap->DriverInfo.Palette));
			}

			jeBitmap_Destroy_Base(Bitmap);

			*Bmp = NULL;

			return JE_TRUE;
		}
	}

return JE_FALSE;
}

#if 0 // {} off limits until _Lock & _UnLock percolates up DataOwner
jeBitmap * jeBitmap_CreateXerox(jeBitmap *BmpSrc)
{
jeBitmap * Bmp;

	assert( jeBitmap_IsValid(BmpSrc) );
	if ( BmpSrc->LockOwner )
		return NULL;	//{} return jeBitmap_CreateXeroxFromLock()

	Bmp = jeBitmap_Create_Base();
	if ( ! Bmp )
		return NULL;
			
	memcpy(Bmp,BmpSrc);

	Bmp->LockCount = 0;
	Bmp->RefCount = 1;

	Bmp->DataOwner = BmpSrc;
	jeBitmap_CreateRef(BmpSrc);
return Bmp;
}
#endif

jeBoolean jeBitmap_AllocSystemMip(jeBitmap *Bmp,int32 mip)
{
	if ( ! Bmp )
	{
		return JE_FALSE;
	}

	if ( Bmp->LockOwner && mip != 0 ) return JE_FALSE;

	if ( ! Bmp->Data[mip] )
	{
	int32 bytes;
		bytes = jeBitmap_MipBytes(Bmp,mip);
		if ( bytes == 0 )
		{
			Bmp->Data[mip] = NULL;
			return JE_TRUE;
		}
		Bmp->Data[mip] = jeRam_Allocate( bytes );
	}

return (Bmp->Data[mip]) ? JE_TRUE : JE_FALSE;
}

jeBoolean jeBitmap_AllocPalette(jeBitmap *Bmp,jePixelFormat Format,DRV_Driver * Driver)
{
jeBitmap_Info * BmpInfo;
	assert(Bmp);

	if ( Driver )
		BmpInfo = &(Bmp->DriverInfo);
	else
		BmpInfo = &(Bmp->Info);

	if ( ! jePixelFormat_IsRaw(Format) )
		Format = JE_PIXELFORMAT_32BIT_XRGB;

	if ( ! BmpInfo->Palette )
	{
		assert( BmpInfo->Format == JE_PIXELFORMAT_8BIT_PAL );

		if ( Driver )
		{
		jeBoolean BmpHasAlpha;
			
			BmpHasAlpha = JE_FALSE;
			if ( jePixelFormat_HasGoodAlpha(Bmp->Info.Format) )
				BmpHasAlpha = JE_TRUE;
			else if ( Bmp->Info.Palette && jePixelFormat_HasGoodAlpha(Bmp->Info.Palette->Format) )
				BmpHasAlpha = JE_TRUE;

			if ( BmpHasAlpha || (Bmp->Info.HasColorKey && ! Bmp->DriverInfo.HasColorKey ) )
				Format = JE_PIXELFORMAT_32BIT_ARGB;

			BmpInfo->Palette = jeBitmap_Palette_CreateFromDriver(Driver,Format,256);
		}
		else
		{			
			BmpInfo->Palette = jeBitmap_Palette_Create(Format,256);
		}
	}

	if ( ! BmpInfo->Palette )
		return JE_FALSE;

	if ( BmpInfo->HasColorKey )
	{
		if ( ! BmpInfo->Palette->HasColorKey )
		{
			BmpInfo->Palette->HasColorKey = JE_TRUE;
			BmpInfo->Palette->ColorKey = 1; // <>
		}
		BmpInfo->Palette->ColorKeyIndex = BmpInfo->ColorKey;
	}

	if ( Driver )
	{
		assert( Bmp->DriverHandle );
		assert( BmpInfo->Palette->DriverHandle );
		if ( ! Driver->THandle_SetPalette(Bmp->DriverHandle,BmpInfo->Palette->DriverHandle) )
		{
			jeErrorLog_AddString(-1,"AllocPal : THandle_SetPalette", NULL);
			return JE_FALSE;
		}
	}

	if ( ! Bmp->Info.Palette )
	{
		Bmp->Info.Palette = jeBitmap_Palette_CreateCopy(BmpInfo->Palette);
	}

return JE_TRUE;
}

/*}{ *************** Thread & Streaming stuff *******************/

#if 0 	//<> expose this?
JETAPI jeBoolean JETCC jeBitmap_WaitForUnLock(const jeBitmap *Bmp)
{
	assert( jeBitmap_IsValid(Bmp) );
	
	if ( Bmp->LockOwner || Bmp->DataOwner )
		return JE_FALSE;

//	if ( jeThreadQueue_ActiveJobCount() <= 1 )
//		return JE_FALSE;

	while ( Bmp->LockCount )
	{
		jeThreadQueue_Sleep(1);
	}

	if ( Bmp->LockOwner || Bmp->DataOwner )
		return JE_FALSE;

return JE_TRUE;
}
#endif

/****

	just about every function should call either _WaitReady or _PeekReady

	the former waits for any streaming to finish
	the latter updates the bitmap based on the current status of the steaming.

****/

jeBoolean jeBitmap_WaitReady(const jeBitmap *Bmp)
{
	assert( jeBitmap_IsValid(Bmp) );
	
	if ( Bmp->StreamingStatus >= JE_BITMAP_STREAMING_STARTED )
	{
		assert(Bmp->Wavelet);
		ThreadLog_Printf("Wavelet : Waiting %08X Streaming\n",(uint32)(Bmp->Wavelet));
		jeWavelet_WaitStreaming(Bmp->Wavelet);
		((jeBitmap *)Bmp)->StreamingStatus = JE_BITMAP_STREAMING_DATADONE;
		((jeBitmap *)Bmp)->StreamingTHandle = JE_FALSE;
	}

return JE_TRUE;
}

jeBoolean jeBitmap_PeekReady(const jeBitmap *Bmp)
{
	assert( jeBitmap_IsValid(Bmp) );

	if ( Bmp->DataOwner )
		Bmp = Bmp->DataOwner;
	if ( Bmp->LockOwner || Bmp->LockCount < 0 )
		return JE_TRUE;
			
	//get streaming progress & time since last streaming update;
	// then call _Update_SystemToDriver

	// Warning : jeBitmap_Update_SystemToDriver calls PeekReady
	//	and PeekReady calls jeBitmap_Update_SystemToDriver !
	//	be carefull!

	if ( Bmp->StreamingStatus >= JE_BITMAP_STREAMING_STARTED )
	{
		assert(Bmp->Wavelet);
		if ( Bmp->DriverHandle )
		{
			if ( ! jeWavelet_StreamingJob(Bmp->Wavelet) )
			{
				((jeBitmap *)Bmp)->StreamingStatus = JE_BITMAP_STREAMING_DATADONE;
				((jeBitmap *)Bmp)->StreamingTHandle = JE_FALSE;
			}
			else
			{
				if ( jeWavelet_ShouldDecompressStreaming(Bmp->Wavelet) )
				{
					ThreadLog_Printf("Wavelet : Decompressing %08X Streaming\n",(uint32)(Bmp->Wavelet));
					jeBitmap_Update_SystemToDriver((jeBitmap *)Bmp);
					((jeBitmap *)Bmp)->StreamingStatus = JE_BITMAP_STREAMING_CHANGED;
					
					if ( ! jeWavelet_StreamingJob(Bmp->Wavelet) )
					{
						((jeBitmap *)Bmp)->StreamingStatus = JE_BITMAP_STREAMING_DATADONE;
						((jeBitmap *)Bmp)->StreamingTHandle = JE_FALSE;
					}
				}
				else
				{
					((jeBitmap *)Bmp)->StreamingStatus = JE_BITMAP_STREAMING_IDLE;
				}
			}
		}
	}

return JE_TRUE;
}

JETAPI jeBitmap_StreamingStatus JETCC jeBitmap_GetStreamingStatus(const jeBitmap *Bmp)
{
jeThreadQueue_JobStatus Status;
jeThreadQueue_Job * Job;
jeBitmap_StreamingStatus Ret;

	assert( jeBitmap_IsValid(Bmp) );

	if ( Bmp->DataOwner )
		Bmp = Bmp->DataOwner;
	if ( Bmp->LockOwner || Bmp->LockCount < 0 )
		return JE_BITMAP_STREAMING_ERROR;
			
	if ( Bmp->StreamingStatus < JE_BITMAP_STREAMING_STARTED )
		return JE_BITMAP_STREAMING_NOT;

	assert(Bmp->Wavelet);

	Job = jeWavelet_StreamingJob(Bmp->Wavelet);
	if ( ! Job )
	{
		//StreamingStatus could be DATADONE ; if so, return a real _DONE
		if ( Bmp->StreamingStatus == JE_BITMAP_STREAMING_DONE )
			Ret = JE_BITMAP_STREAMING_NOT;
		else
			Ret = JE_BITMAP_STREAMING_DONE;
	}
	else
	{
		if ( ! jeThreadQueue_WaitOnJob(Job,JE_THREADQUEUE_STATUS_RUNNING) )
		{
			jeErrorLog_AddString(-1,"Bitmap_GetStreamingStatus : WaitOnJob failed! Continuing anyway!",NULL);
		}

		Ret = JE_BITMAP_STREAMING_IDLE;
		if ( Bmp->Wavelet && jeWavelet_ShouldDecompressStreaming(Bmp->Wavelet) )
			Ret = JE_BITMAP_STREAMING_CHANGED;

		Job = jeWavelet_StreamingJob(Bmp->Wavelet);;
		if ( ! Job )
		{
			Ret = JE_BITMAP_STREAMING_DONE;
		}
		else
		{
			Status = jeThreadQueue_JobGetStatus(Job);
			if ( Status == JE_THREADQUEUE_STATUS_COMPLETED )
				Ret = JE_BITMAP_STREAMING_DONE;
		}
	}

	((jeBitmap *)Bmp)->StreamingStatus = Ret;

	if (Ret == JE_BITMAP_STREAMING_DONE ||
		Ret == JE_BITMAP_STREAMING_NOT )
		((jeBitmap *)Bmp)->StreamingTHandle = JE_FALSE;

return Ret;
}

/*}{ *************** Locks *******************/

JETAPI jeBoolean	JETCC	 jeBitmap_LockForWrite(
	jeBitmap *			Bmp,
	jeBitmap **			Target,
	int32				MinimumMip,
	int32				MaximumMip)
{
int32 mip;

	assert( jeBitmap_IsValid(Bmp) );
	assert( Target);
	assert(MaximumMip >= MinimumMip);
	assert( &Bmp != Target );

	jeBitmap_WaitReady(Bmp);

	if ( Bmp->LockCount || Bmp->LockOwner )
	{
		jeErrorLog_AddString(-1,"LockForWrite : already locked", NULL);
		return JE_FALSE;
	}

	if ( Bmp->DriverHandle )
	{
		if ( (MinimumMip < Bmp->DriverInfo.MinimumMip) ||
			 (MaximumMip > Bmp->DriverInfo.MaximumMip) )
		{
			jeErrorLog_AddString(-1,"LockForWrite : Driver : invalid mip", NULL);
			return JE_FALSE;
		}
	}
	else
	{
		if ( (MinimumMip < Bmp->Info.MinimumMip) ||
			 (MaximumMip >= MAXMIPLEVELS) )
		{
			jeErrorLog_AddString(-1,"LockForWrite : System : invalid mip", NULL);
			return JE_FALSE;
		}

		if ( MaximumMip > Bmp->Info.MaximumMip )
		{
			if ( ! jeBitmap_MakeSystemMips(Bmp,Bmp->Info.MaximumMip,MaximumMip) )
				return JE_FALSE;
			Bmp->Info.MaximumMip = MaximumMip;
		}
	}
	
	Bmp->Persistable = JE_FALSE;

	for(mip=MinimumMip;mip <= MaximumMip;mip ++)
	{
		if ( Bmp->DriverHandle )
		{
			Target[ mip - MinimumMip ] = jeBitmap_CreateLockFromMipOnDriver(Bmp,mip,-1);
		}
		else
		{
			Target[ mip - MinimumMip ] = jeBitmap_CreateLockFromMipSystem(Bmp,mip,-1);
		}
		if ( ! Target[ mip - MinimumMip ] )
		{
			jeErrorLog_AddString(-1,"LockForWrite : CreateLockFromMip failed", NULL);
			mip--;
			while(mip >= MinimumMip )
			{
				jeBitmap_Destroy( & Target[ mip - MinimumMip ] );
				mip--;
			}
			return JE_FALSE;
		}
	}

	assert( Bmp->LockCount == - (MaximumMip - MinimumMip + 1) );

	return JE_TRUE;
}

JETAPI jeBoolean	JETCC jeBitmap_LockForWriteFormat(
	jeBitmap *			Bmp,
	jeBitmap **			Target,
	int32				MinimumMip,
	int32				MaximumMip,
	jePixelFormat 		Format)
{
int32 mip;

	assert( jeBitmap_IsValid(Bmp) );
	assert( Target);
	assert(MaximumMip >= MinimumMip);
	assert( &Bmp != Target );
	
	jeBitmap_WaitReady(Bmp);

	if ( Bmp->LockCount || Bmp->LockOwner )
	{
		jeErrorLog_AddString(-1,"LockForWrite : already locked", NULL);
		return JE_FALSE;
	}

	if ( Format != Bmp->Info.Format && Format != Bmp->DriverInfo.Format )
	{
		jeErrorLog_AddString(-1,"LockForWriteFormat : must be System or Driver Format !", NULL);
		return JE_FALSE;
	}

	if ( Format == Bmp->DriverInfo.Format )
	{
		if ( MinimumMip < Bmp->DriverInfo.MinimumMip || MaximumMip > Bmp->DriverInfo.MaximumMip )
		{
			jeErrorLog_AddString(-1,"LockForWrite : invalid Driver mip", NULL);
			return JE_FALSE;
		}
	}
	else
	{
		assert( Format == Bmp->Info.Format );

		if ( Bmp->DriverHandle )
		{
			if ( ! jeBitmap_Update_DriverToSystem(Bmp) )
			{
				jeErrorLog_AddString(-1,"LockForWrite : Update_DriverToSystem", NULL);
				return JE_FALSE;
			}
		}

		// create mips?

		if ( MinimumMip < Bmp->Info.MinimumMip || MaximumMip >= MAXMIPLEVELS )
		{
			jeErrorLog_AddString(-1,"LockForWrite : invalid System mip", NULL);
			return JE_FALSE;
		}
		
		if ( ! jeBitmap_MakeSystemMips(Bmp,Bmp->Info.MaximumMip,MaximumMip) )
			return JE_FALSE;
		Bmp->Info.MaximumMip = MaximumMip;
	}

	Bmp->Persistable = JE_FALSE;

	for(mip=MinimumMip;mip <= MaximumMip;mip ++)
	{
		if ( Bmp->DriverHandle && Format == Bmp->DriverInfo.Format )
		{
			Target[ mip - MinimumMip ] = jeBitmap_CreateLockFromMipOnDriver(Bmp,mip,-1);
		}
		else
		{
			Target[ mip - MinimumMip ] = jeBitmap_CreateLockFromMipSystem(Bmp,mip,-1);
		}

		if ( ! Target[ mip - MinimumMip ] )
		{
			jeErrorLog_AddString(-1,"LockForWrite : CreateLockFromMip failed", NULL);
			mip--;
			while(mip >= MinimumMip )
			{
				jeBitmap_Destroy( & Target[ mip - MinimumMip ] );
				mip--;
			}
			return JE_FALSE;
		}
	}

	assert( Bmp->LockCount == - (MaximumMip - MinimumMip + 1) );

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_LockForReadNative(
	const jeBitmap *	iBmp,
	jeBitmap **			Target,
	int32				MinimumMip,
	int32				MaximumMip)
{
int32 mip;
jeBitmap * Bmp = (jeBitmap *)iBmp;

	assert( jeBitmap_IsValid(Bmp) );
	assert( Target);
	assert(MaximumMip >= MinimumMip);
	assert( &Bmp != Target );

// <> lock-for-read : don't do peekready ? if it's a wavelet, 
//	jeBitmap_PeekReady(Bmp);

	if ( (MinimumMip < Bmp->Info.MinimumMip && MinimumMip < Bmp->DriverInfo.MinimumMip) ||
		 (MaximumMip >= MAXMIPLEVELS) )
	{
		jeErrorLog_AddString(-1,"LockForRead : invalid mip", NULL);
		return JE_FALSE;
	}

	if ( Bmp->LockCount < 0 || Bmp->LockOwner )
	{
		jeErrorLog_AddString(-1,"LockForRead : already locked", NULL);
		return JE_FALSE;
	}

	for(mip=MinimumMip;mip <= MaximumMip;mip ++)
	{
		// err on the side of *not* choosing the driver data to read from !
		if ( Bmp->DriverHandle && Bmp->DriverDataChanged
			&& mip <= Bmp->DriverInfo.MaximumMip && mip >= Bmp->DriverInfo.MinimumMip)
		{
			Target[ mip - MinimumMip ] = jeBitmap_CreateLockFromMipOnDriver(Bmp,mip,1);
		}
		else
		{
			Target[ mip - MinimumMip ] = jeBitmap_CreateLockFromMipSystem(Bmp,mip,1);
		}
		if ( ! Target[ mip - MinimumMip ] )
		{
			jeErrorLog_AddString(-1,"LockForRead : CreateLockFromMip failed", NULL);
			mip--;
			while(mip >= MinimumMip )
			{
				jeBitmap_Destroy( & Target[ mip - MinimumMip ] );
				mip--;
			}
			return JE_FALSE;
		}
	}

return JE_TRUE;
}

JETAPI jeBoolean	JETCC jeBitmap_LockForRead(
	const jeBitmap *	iBmp,
	jeBitmap **			Target,
	int32				MinimumMip,
	int32				MaximumMip,
	jePixelFormat		Format,
	jeBoolean			HasColorKey,
	uint32				ColorKey)
{
int32 mip;
jeBitmap * Bmp = (jeBitmap *)iBmp;

	assert( jeBitmap_IsValid(Bmp) );
	assert( Target);
	assert(MaximumMip >= MinimumMip);
	assert( &Bmp != Target );

// <> lock-for-read : don't do peekready ?
//	jeBitmap_PeekReady(Bmp);

	if ( MinimumMip < Bmp->Info.MinimumMip ||
//		 MaximumMip > Bmp->Info.MaximumMip
		MaximumMip >= MAXMIPLEVELS )
	{
		jeErrorLog_AddString(-1,"LockForRead : invalid mip", NULL);
		return JE_FALSE;
	}

	if ( Bmp->LockCount < 0 || Bmp->LockOwner )
	{
		jeErrorLog_AddString(-1,"LockForRead : already locked", NULL);
		return JE_FALSE;
	}
	
	//LockForRead must special case wavelet for making many mips at once
	if ( Bmp->Info.Format == JE_PIXELFORMAT_WAVELET )
	{
	jeBitmap * Lock;
	jeBitmap_Info * Infos[MAXMIPLEVELS];
	void * Bits[MAXMIPLEVELS];
	int32 i,MipCount;;

		assert(Bmp->Wavelet);
		assert(jePixelFormat_BytesPerPel(Format) > 0 );

		MipCount = MaximumMip - MinimumMip + 1;

		for(mip=MinimumMip;mip <= MaximumMip;mip ++)
		{
			i = mip - MinimumMip;
			Lock = jeBitmap_CreateLock_CopyInfo(Bmp,1,mip);
			if ( ! Lock )
			{
				jeBitmap_UnLockArray(Target,mip - MinimumMip);
				return JE_FALSE;
			}

			Lock->Info.Format = Format;
			Lock->Info.ColorKey = ColorKey;
			Lock->Info.HasColorKey = HasColorKey;
			if ( ! jeBitmap_AllocSystemMip(Lock,0) )
			{
				jeBitmap_UnLockArray(Target,mip - MinimumMip + 1);
				return JE_FALSE;
			}
			Target[i] = Lock;
			Infos[i] = &(Lock->Info);
			Bits[i] = Lock->Data[0];
		}

		if ( jeWavelet_CanDecompressMips(Bmp->Wavelet,Infos[0]) )
		{
			if ( ! jeWavelet_DecompressMips(Bmp->Wavelet,(const jeBitmap_Info **)Infos,(const void **)Bits,MinimumMip,MaximumMip) )
			{
				jeErrorLog_AddString(-1,"LockForRead : Wavelet_DecompressMips failed!", NULL);
				jeBitmap_UnLockArray(Target,MipCount);
				return JE_FALSE;
			}
		}
		else
		{
			if ( MinimumMip != 0 )
			{
				jeErrorLog_AddString(-1,"sorry, can't lock wavelet with minMip != 0", NULL);
				jeBitmap_UnLockArray(Target,MipCount);
				return JE_FALSE;
			}

			if ( ! jeWavelet_Decompress(Bmp->Wavelet,Infos[0],Bits[0]) )
			{
				jeErrorLog_AddString(-1,"LockForRead : Wavelet_Decompress failed!", NULL);
				jeBitmap_UnLockArray(Target,MipCount);
				return JE_FALSE;
			}

			// now make the mips

			for(i=1;i<MipCount;i++)
			{
				if ( ! jeBitmap_UpdateMips_Data(Infos[i-1],Bits[i-1],Infos[i],Bits[i]) )
				{
					jeErrorLog_AddString(-1,"LockForRead : UpdateMips_Data failed!", NULL);
					jeBitmap_UnLockArray(Target,MipCount);
					return JE_FALSE;
				}
			}
		}
	}
	else
	{
		for(mip=MinimumMip;mip <= MaximumMip;mip ++)
		{
			Target[ mip - MinimumMip ] = jeBitmap_CreateLockFromMip(Bmp,mip, Format,HasColorKey,ColorKey,1);
			if ( ! Target[ mip - MinimumMip ] )
			{
				jeErrorLog_AddString(-1,"LockForRead : CreateLockFromMip failed", NULL);
				mip--;
				while(mip >= MinimumMip )
				{
					jeBitmap_Destroy( & Target[ mip - MinimumMip ] );
					mip--;
				}
				return JE_FALSE;
			}
		}
	}

return JE_TRUE;
}

jeBoolean jeBitmap_UnLockArray_NoChange(jeBitmap **Locks,int32 Size)
{
int i;
jeBoolean Ret = JE_TRUE;
	assert(Locks);
	for(i=0;i<Size;i++)
	{
		if ( ! jeBitmap_UnLock_NoChange(Locks[i]) )
			Ret = JE_FALSE;
	}
return Ret;
}

JETAPI jeBoolean	JETCC jeBitmap_UnLockArray(jeBitmap **Locks,int32 Size)
{
int i;
jeBoolean Ret = JE_TRUE;
	assert(Locks);
	for(i=0;i<Size;i++)
	{
		if ( ! jeBitmap_UnLock(Locks[i]) )
			Ret = JE_FALSE;
	}
return Ret;
}

jeBoolean jeBitmap_UnLock_Internal(jeBitmap *Bmp,jeBoolean Apply)
{
jeBoolean Ret = JE_TRUE;

	if ( ! Bmp )
	{
		jeErrorLog_AddString(-1,"UnLock : bad bmp", NULL);
		return JE_FALSE;
	}

	assert( Bmp->LockCount == 0 );

	if ( Bmp->LockOwner )
	{
	int DoUpdate = 0;

		assert(Bmp->LockOwner->LockCount != 0);
		if ( Bmp->LockOwner->LockCount > 0 )
		{
			Bmp->LockOwner->LockCount --;
		}
		else if ( Bmp->LockOwner->LockCount < 0 )
		{
			Bmp->LockOwner->LockCount ++;

			if ( Apply )
			{
			jeBitmap_Palette * Pal;

				Bmp->LockOwner->Modified[Bmp->Info.MinimumMip] = JE_TRUE;
			
				Pal = Bmp->DriverInfo.Palette ? Bmp->DriverInfo.Palette : Bmp->Info.Palette;
				if ( Pal )
					jeBitmap_SetPalette(Bmp->LockOwner,Pal);

				// this palette will be destroyed later on

				Bmp->HasAverageColor = JE_FALSE;
			}

			if ( Bmp->LockOwner->LockCount == 0 && Apply )
			{
				// last unlock for write
				// if Bmp is on hardware, flag the Data[] as needing update
				if ( Bmp->DriverBitsLocked )
				{
					assert( Bmp->DriverHandle );
					Bmp->LockOwner->DriverDataChanged = JE_TRUE;
					DoUpdate = 1;
				}
				else
				{
					DoUpdate = -1;
				}
			}
		}
		
		if ( Bmp->DriverHandle && Bmp->DriverBitsLocked )
		{
			assert(Bmp->Driver);
			if ( ! Bmp->Driver->THandle_UnLock(Bmp->DriverHandle, Bmp->DriverMipLock - Bmp->DriverMipBase) )
			{
				jeErrorLog_AddString(-1,"UnLock : thandle_unlock", NULL);
				Ret = JE_FALSE;
			}
			Bmp->DriverBitsLocked = JE_FALSE;
			Bmp->DriverMipLock = 0;
		}

		if ( Bmp->Alpha )
		{
			if ( ! jeBitmap_UnLock(Bmp->Alpha) )
				Ret = JE_FALSE;

			Bmp->Alpha = NULL;
		}

		if ( DoUpdate )
		{
			assert(Bmp->LockOwner->LockCount == 0 );
			// we just finished unlocking a lock-for-write
			if ( DoUpdate > 0 )
			{
			//	don't update from driver -> system, leaved the changed data on the driver
			//	we've got DriverDataChanged
			//	if ( ! jeBitmap_Update_DriverToSystem(Bmp) )
			//		Ret = JE_FALSE;
			}
			else
			{
				if ( Bmp->LockOwner->DriverHandle )
					if ( ! jeBitmap_Update_SystemToDriver(Bmp->LockOwner) )
						Ret = JE_FALSE;
			}
		}

		// we did a CreateRef on the lockowner
		jeBitmap_Destroy(&(Bmp->LockOwner));
		Bmp->LockOwner = NULL;

	}
	// else fail ?

	assert(Bmp->RefCount == 1);

	jeBitmap_Destroy(&Bmp);

	assert(Bmp == NULL);

return Ret;
}

JETAPI jeBoolean	JETCC jeBitmap_UnLock(jeBitmap *Bmp)
{
return jeBitmap_UnLock_Internal(Bmp,JE_TRUE);
}

jeBoolean jeBitmap_UnLock_NoChange(jeBitmap *Bmp)
{
return jeBitmap_UnLock_Internal(Bmp,JE_FALSE);
}

JETAPI void *	JETCC jeBitmap_GetBits(jeBitmap *Bmp)
{
void * bits;

	assert( jeBitmap_IsValid(Bmp) );

	if ( ! Bmp )
	{
		jeErrorLog_AddString(-1,"GetBits : bad bmp", NULL);
		return NULL;
	}

	if ( ! Bmp->LockOwner )	// must be a lock!
	{
		jeErrorLog_AddString(-1,"GetBits : not a lock", NULL);
		return NULL;
	}

	if ( Bmp->DriverHandle )
	{
		assert(Bmp->Driver);
		if ( ! Bmp->Driver->THandle_Lock(Bmp->DriverHandle,Bmp->DriverMipLock - Bmp->DriverMipBase,&bits) )
		{
			jeErrorLog_AddString(-1,"GetBits : THandle_Lock", NULL);
			return NULL;
		}

		Bmp->DriverBitsLocked = JE_TRUE;
	}
	else if ( Bmp->Wavelet )
	{
		//{} with WaveletMipLock
		bits = Bmp->Wavelet;
	}
	else
	{
		bits = Bmp->Data[0];
	}

return bits;
}

/*}{ ************* _CreateLock_#? *********************/

jeBitmap * jeBitmap_CreateLock_CopyInfo(jeBitmap *BmpSrc,int32 LockCnt,int32 mip)
{
jeBitmap * Bmp;

	assert( jeBitmap_IsValid(BmpSrc) );

	// all _CreateLocks go through here

	Bmp = jeBitmap_Create_Base();
	if ( ! Bmp )
		return NULL;

	jeBitmap_MakeMipInfo(&(BmpSrc->Info),mip,&(Bmp->Info));
	jeBitmap_MakeMipInfo(&(BmpSrc->DriverInfo),mip,&(Bmp->DriverInfo));
	Bmp->DriverFlags = BmpSrc->DriverFlags;
	Bmp->DriverGamma = BmpSrc->DriverGamma;
	Bmp->DriverGammaLast = BmpSrc->DriverGammaLast;
		
	Bmp->Info.Palette = NULL;
	Bmp->DriverInfo.Palette = NULL;

	Bmp->Driver	= BmpSrc->Driver;
	Bmp->PreferredFormat= BmpSrc->PreferredFormat;

	Bmp->LockOwner = BmpSrc;
	jeBitmap_CreateRef(BmpSrc); // we do a _Destroy() in UnLock()

	Bmp->HasWaveletOptions = BmpSrc->HasWaveletOptions;
	Bmp->WaveletOptions = BmpSrc->WaveletOptions;

	BmpSrc->LockCount += LockCnt;

return Bmp;
}

void jeBitmap_MakeMipInfo(jeBitmap_Info *Src,int32 mip,jeBitmap_Info * Target)
{
	assert( Src && Target );
	assert( mip >= 0 && mip < MAXMIPLEVELS );
	*Target = *Src;

	Target->Width  = SHIFT_R_ROUNDUP(Target->Width,mip);
	Target->Height = SHIFT_R_ROUNDUP(Target->Height,mip);
	Target->Stride = SHIFT_R_ROUNDUP(Target->Stride,mip);
	Target->MinimumMip = Target->MaximumMip = mip;
}

jeBitmap * jeBitmap_CreateLockFromMip(jeBitmap *Src,int32 mip,
	jePixelFormat Format,
	jeBoolean	HasColorKey,
	uint32		ColorKey,
	int32			LockCnt)
{
jeBitmap * Ret;

	assert( jeBitmap_IsValid(Src) );
	if ( mip < 0 || mip >= MAXMIPLEVELS )
		return NULL;

	// LockForRead always goes through here

	// you can never lock Wavelet data (or can you?)
//	if ( jePixelFormat_BytesPerPel(Format) < 1 )
//		return NULL;

	if ( Src->DriverInfo.Format == Format &&
		 JE_BOOLSAME(Src->DriverInfo.HasColorKey,HasColorKey) &&
		 (!HasColorKey || Src->DriverInfo.ColorKey == ColorKey) &&
		 mip >= Src->DriverInfo.MinimumMip && mip <= Src->DriverInfo.MaximumMip )
	{
		return jeBitmap_CreateLockFromMipOnDriver(Src,mip,LockCnt);
	}

	if ( Src->DriverHandle )
	{
		if ( ! jeBitmap_Update_DriverToSystem(Src) )
		{
			return NULL;
		}
	}
		
	if ( ! Src->Data[mip] )
	{
		if ( ! jeBitmap_MakeSystemMips(Src,mip,mip) )
			return NULL;
	}

	if ( Src->Info.Format == Format &&
		 JE_BOOLSAME(Src->Info.HasColorKey,HasColorKey) &&
		 (!HasColorKey || Src->Info.ColorKey == ColorKey) )
	{
		return jeBitmap_CreateLockFromMipSystem(Src,mip,LockCnt);
	}

	Ret = jeBitmap_CreateLock_CopyInfo(Src,LockCnt,mip);

	if ( ! Ret )
		return NULL;

	Ret->Info.Stride = Ret->Info.Width;	// {} ?

	Ret->Info.Format = Format;
	Ret->Info.ColorKey = ColorKey;
	Ret->Info.HasColorKey = HasColorKey;

	if ( jePixelFormat_HasPalette(Format) && Src->Info.Palette )
	{
		Ret->Info.Palette = Src->Info.Palette;
		jeBitmap_Palette_CreateRef(Ret->Info.Palette);
	}

	assert( Ret->Alpha == NULL );
	if ( ! jePixelFormat_HasGoodAlpha(Format) && Src->Alpha )
	{
		if ( ! jeBitmap_LockForRead(Src->Alpha,&(Ret->Alpha),mip,mip,JE_PIXELFORMAT_8BIT_GRAY,0,0) )
		{
			jeErrorLog_AddString(-1,"CreateLockFromMip : LockForRead failed", NULL);
			jeBitmap_Destroy(&Ret);
			return NULL;
		}
		assert( Ret->Alpha );
	}

	assert( jeBitmap_IsValid(Ret) );

	if (	Src->Info.Format == JE_PIXELFORMAT_WAVELET &&
			Ret->Info.Format == JE_PIXELFORMAT_WAVELET )
	{
		assert(Src->Wavelet);
		Ret->Wavelet = Src->Wavelet;
		jeWavelet_CreateRef(Ret->Wavelet);

		Ret->WaveletMipLock = mip;
	}
	else
	{
		assert( Ret->Info.Format != JE_PIXELFORMAT_WAVELET);

		if ( ! jeBitmap_AllocSystemMip(Ret,0) )
		{
			jeBitmap_Destroy(&Ret);
			return NULL;
		}

		if ( ! jeBitmap_BlitMip( Src, mip, Ret, 0 ) )
		{
			jeErrorLog_AddString(-1,"CreateLockFromMip : BlitMip failed", NULL);
			jeBitmap_Destroy(&Ret);
			return NULL;
		}
	}

return Ret;
}

jeBitmap * jeBitmap_CreateLockFromMipSystem(jeBitmap *Src,int32 mip,int32 LockCnt)
{
jeBitmap * Ret;

	assert( jeBitmap_IsValid(Src) );
	if ( mip < Src->Info.MinimumMip || mip >= MAXMIPLEVELS )
		return NULL;

	// you can never lock Wavelet data {}
	//if ( jePixelFormat_BytesPerPel(Src->Info.Format) < 1 )
	//	return NULL;

	if ( ! Src->Data[mip] )
	{
		if ( ! jeBitmap_MakeSystemMips(Src,mip,mip) )
			return NULL;
	}

	Ret = jeBitmap_CreateLock_CopyInfo(Src,LockCnt,mip);

	if ( ! Ret ) return NULL;

	Ret->Data[0] = Src->Data[mip];

	if ( Src->Info.Format == JE_PIXELFORMAT_WAVELET )
	{
		assert(Src->Wavelet);
		Ret->Wavelet = Src->Wavelet;
		Ret->WaveletMipLock = mip;
	}

	Ret->Info.Palette = Src->Info.Palette;
	if ( Ret->Info.Palette )
		jeBitmap_Palette_CreateRef(Ret->Info.Palette);

	Ret->DataOwner = Src;
	jeBitmap_CreateRef(Src);

	assert( Ret->Alpha == NULL );
	if ( ! jePixelFormat_HasGoodAlpha(Src->Info.Format) && Src->Alpha )
	{
		if ( ! jeBitmap_LockForRead(Src->Alpha,&(Ret->Alpha),mip,mip,JE_PIXELFORMAT_8BIT_GRAY,0,0) )
		{
			jeErrorLog_AddString(-1,"CreateLockFromMipSystem : LockForRead failed", NULL);
			jeBitmap_Destroy(&Ret);
			return NULL;
		}
		assert( Ret->Alpha );
	}

	assert( jeBitmap_IsValid(Ret) );

return Ret;
}

jeBitmap * jeBitmap_CreateLockFromMipOnDriver(jeBitmap *Src,int32 mip,int32 LockCnt)
{
jeBitmap * Ret;

	assert( jeBitmap_IsValid(Src) );
	if ( ! Src->DriverHandle || ! Src->Driver || Src->DriverMipLock || mip < Src->DriverInfo.MinimumMip || mip > Src->DriverInfo.MaximumMip )
		return NULL;

	// the driver can never have Wavelet data
	// {} it could have S3TC data, though..
	assert( jePixelFormat_BytesPerPel(Src->DriverInfo.Format) > 0);

	Ret = jeBitmap_CreateLock_CopyInfo(Src,LockCnt,mip);

	if ( ! Ret ) return NULL;

	Ret->DriverMipLock = mip;
	Ret->DriverHandle = Src->DriverHandle;
	Ret->DriverMipBase = Src->DriverMipBase;

	Ret->DataOwner = Src;
	jeBitmap_CreateRef(Src);

	Ret->DriverInfo.Palette = Src->DriverInfo.Palette;

	if ( ! jeBitmap_MakeDriverLockInfo(Ret,mip,&(Ret->DriverInfo)) )
	{
		jeErrorLog_AddString(-1,"CreateLockFromMipOnDriver : UpdateInfo failed", NULL);
		jeBitmap_Destroy(&Ret);
		return NULL;
	}

	assert(Ret->DriverInfo.Palette == Src->DriverInfo.Palette);

	Ret->Info = Ret->DriverInfo;	//{} shouldn't be necessary
	
	if ( Ret->DriverInfo.Palette )
		jeBitmap_Palette_CreateRef(Ret->DriverInfo.Palette);
	if ( Ret->Info.Palette )
		jeBitmap_Palette_CreateRef(Ret->Info.Palette);

	assert( jeBitmap_IsValid(Ret) );

return Ret;
}

/*}{ ************* Driver Attachment *********************/

#define MAX_DRIVER_FORMATS (100)

static jeBoolean EnumPFCB(jeRDriver_PixelFormat *pFormat,void *Context)
{
jeRDriver_PixelFormat **pDriverFormatsPtr;
	pDriverFormatsPtr = (jeRDriver_PixelFormat **)Context;
	**pDriverFormatsPtr = *pFormat;
	(*pDriverFormatsPtr) += 1;
return JE_TRUE;
}

static int32 NumBitsOn(uint32 val)
{
uint32 count = 0;
	while(val)
	{
		count += val&1;
		val >>= 1;
	}
return count;
}

static jeBoolean IsInArray(uint32 Val,uint32 *Array,int32 Len)
{
	while(Len--)
	{
		if ( Val == *Array )
			return JE_TRUE;
		Array--;
	}
return JE_FALSE;
}

jeBoolean jeBitmap_ChooseDriverFormat(
	jePixelFormat	SeekFormat1,
	jePixelFormat	SeekFormat2,
	jeBoolean		SeekCK,
	jeBoolean		SeekAlpha,
	jeBoolean		SeekSeparates,
	uint32			SeekFlags,
	jeRDriver_PixelFormat *DriverFormatsArray,int32 ArrayLen,
	jeRDriver_PixelFormat *pTarget)
{
int32 i,rating;
int32 FormatRating[MAX_DRIVER_FORMATS];
jeRDriver_PixelFormat * DriverPalFormat;
jeRDriver_PixelFormat * pf;
jeBoolean FoundAlpha;
uint32 SeekMajor,SeekMinor;
const jePixelFormat_Operations *seekops,*pfops;

	assert(pTarget && DriverFormatsArray && ArrayLen > 0);

	if ( SeekAlpha )
		SeekCK = JE_FALSE;	// you can't have both

	if ( SeekFlags & RDRIVER_PF_ALPHA_SURFACE )
		SeekFlags = RDRIVER_PF_ALPHA_SURFACE;
	else if ( SeekFlags & RDRIVER_PF_PALETTE )
		SeekFlags = RDRIVER_PF_PALETTE;

	SeekMajor = SeekFlags & RDRIVER_PF_MAJOR_MASK;
	SeekMinor = SeekFlags - SeekMajor;

	pf = DriverFormatsArray;
	DriverPalFormat = NULL;
	for(i=0;i<ArrayLen;i++)
	{
		if ( pf->Flags & RDRIVER_PF_PALETTE )
		{
			if ( ! DriverPalFormat || jePixelFormat_HasGoodAlpha(pf->PixelFormat) )
				DriverPalFormat = pf;
		}
		pf++;
	}

	seekops = jePixelFormat_GetOperations(SeekFormat1);

	for(i=0;i<ArrayLen;i++)
	{
		pf = DriverFormatsArray + i;
		rating = 0;

		/***
		{}

		this is the code to try to pick the closest format the driver has to offer
		the choosing precedence is :

			1. if want alpha, get alpha
			2. match the _3D_ type flags exactly (PF_MAJOR)
			3. match PreferredFormat
			4. match the bitmap's system format
			5. match up the pixel formats masks

		we use fuzzy logic : we apply rating to all the matches and then choose
		the one with the best rating.

		possible todos :
			1. be aware of memory use and format-conversion times, and penalize
				or favor formats accordingly

		note : we currently try to use 1555 for colorkey.

		***/

		if ( (pf->Flags & RDRIVER_PF_MAJOR_MASK) == SeekMajor )
		{
			rating += 1<<23;
		}
		else if ( (pf->Flags & SeekMajor) != SeekMajor )
		{
		    FormatRating[i] = 0;
			continue;
		}
		else
		{
			// advantage to as few different major flags as possible
			// higher priority than similarity of pixelformats
			rating += (32 - NumBitsOn( (pf->Flags ^ SeekFlags) & RDRIVER_PF_MAJOR_MASK ))<<6;
		}

		pfops = jePixelFormat_GetOperations(pf->PixelFormat);

		if ( pf->PixelFormat == SeekFormat1 )
		{
			rating += 1<<16;
		}
		else if ( pf->PixelFormat == SeekFormat2 )
		{
			rating += 1<<15;
		}
		else if ( jePixelFormat_IsRaw(SeekFormat1) && jePixelFormat_IsRaw(pf->PixelFormat) )
		{
		int32 R,G,B,A;
			// measure similarity
			R = (seekops->RMask >> seekops->RShift) ^ (pfops->RMask >> pfops->RShift);
			G = (seekops->GMask >> seekops->GShift) ^ (pfops->GMask >> pfops->GShift);
			B = (seekops->BMask >> seekops->BShift) ^ (pfops->BMask >> pfops->BShift);
			A = (seekops->AMask >> seekops->AShift) ^ (pfops->AMask >> pfops->AShift);
			R = 8 - NumBitsOn(R);
			G = 8 - NumBitsOn(G);
			B = 8 - NumBitsOn(B);
			A = 4*(8 - NumBitsOn(A)); // right number of A bits is molto importante
			rating += (R + G + B + A);
		}
		else
		{
			// one of the formats is not raw
			// palettized ? compressed ?
			rating += 16;
		}

		FoundAlpha = JE_FALSE;

		if ( NumBitsOn(pfops->AMask) > 2 )
			FoundAlpha = JE_TRUE;

		if ( jePixelFormat_HasPalette(pf->PixelFormat) )
		{
            // if Pixelformat is 8BIT_PAL , look at the palette's format to see if it
			//		had alpha!
			assert(DriverPalFormat);
			if ( jePixelFormat_HasGoodAlpha(DriverPalFormat->PixelFormat) )
				FoundAlpha = JE_TRUE;
		}

		if ( SeekAlpha && FoundAlpha )
			rating += 1<<24; // HIGHEST ! (except separates)
		else if ( (!SeekAlpha) && (!FoundAlpha) )
			rating += 1<<10; // LOWEST
		else if ( SeekAlpha )
		{
			if ( NumBitsOn(pfops->AMask) || pf->Flags & RDRIVER_PF_CAN_DO_COLORKEY )
			{
				// sought Alpha, found CK
				rating += 1<<19;
			}
		}

		if ( (pf->Flags & RDRIVER_PF_HAS_ALPHA_SURFACE) && SeekSeparates )
		{
			// separates doesn't count as alpha unless we asked for it !
			rating += 1<<25; // VERY HIGHEST !
		}
	
		if ( SeekCK ) 
		{
			if ( pf->PixelFormat == JE_PIXELFORMAT_16BIT_1555_ARGB )
			{
				rating += 1<<23; // just lower than alpha
			}
			else if ( FoundAlpha ) //better than nothing!
			{
				rating += 1<<18;	// higher than !FoundAlpha
			}
		}

		if ( JE_BOOLSAME((pf->Flags & RDRIVER_PF_CAN_DO_COLORKEY),SeekCK) )
		{
			rating += 1<<17;
		}

		FormatRating[i] = rating;
	}

	rating = 0;

	for(i=0;i<ArrayLen;i++)
	{
		if ( FormatRating[i] > rating )
		{
			rating = FormatRating[i];
			*pTarget = DriverFormatsArray[i];
		}
	}

	if ( rating == 0)
	{
		jeErrorLog_AddString(-1,"ChooseDriverFormat : no valid formats found!", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}
#if 0
jeBoolean jeBitmap_ChooseDriverFormat(
	jePixelFormat	SeekFormat1,
	jePixelFormat	SeekFormat2,
	jeBoolean		SeekCK,
	jeBoolean		SeekAlpha,
	jeBoolean		SeekSeparates,
	uint32			SeekFlags,
	jeRDriver_PixelFormat *DriverFormatsArray,int32 ArrayLen,
	jeRDriver_PixelFormat *pTarget)
{
int32 i,rating;
int32 FormatRating[MAX_DRIVER_FORMATS];
jeRDriver_PixelFormat * DriverPalFormat;
jeRDriver_PixelFormat * pf;
jeBoolean FoundAlpha;
uint32 SeekMajor,SeekMinor;
const jePixelFormat_Operations *seekops,*pfops;

	assert(pTarget && DriverFormatsArray && ArrayLen > 0);

#if 0 // @@
	if ( SeekAlpha )
		SeekCK = JE_FALSE;	// you can't have both, you bastard!
#endif

	if ( SeekFlags & RDRIVER_PF_ALPHA_SURFACE )
		SeekFlags = RDRIVER_PF_ALPHA_SURFACE;
	else if ( SeekFlags & RDRIVER_PF_PALETTE )
		SeekFlags = RDRIVER_PF_PALETTE;

	if ( ! SeekFormat1 )
		SeekFormat1 = SeekFormat2;

	SeekMajor = SeekFlags & RDRIVER_PF_MAJOR_MASK;
	SeekMinor = SeekFlags - SeekMajor;

	pf = DriverFormatsArray;
	DriverPalFormat = NULL;
	for(i=0;i<ArrayLen;i++)
	{
		if ( pf->Flags & RDRIVER_PF_PALETTE )
		{
			assert( jePixelFormat_IsRaw(pf->PixelFormat) );
			if ( ! DriverPalFormat || jePixelFormat_HasGoodAlpha(pf->PixelFormat) )
				DriverPalFormat = pf;
		}
		pf++;
	}

	seekops = jePixelFormat_GetOperations(SeekFormat1);

	for(i=0;i<ArrayLen;i++)
	{
		pf = DriverFormatsArray + i;
		rating = 0;

		/***
		{}

		this is the code to try to pick the closest format the driver has to offer
		the choosing precedence is :

			1. if want alpha, get alpha
			2. match the _3D_ type flags exactly
			3. match PreferredFormat
			4. match the bitmap's system format
			5. match up the formats masks

		***/

		if ( (pf->Flags & RDRIVER_PF_MAJOR_MASK) == SeekMajor )
		{
			rating += 1<<23;
		}
		else if ( (pf->Flags & SeekMajor) != SeekMajor )
		{
		    FormatRating[i] = 0;
			continue;
		}
		else
		{
			// advantage to as few different major flags as possible
			// higher priority than similarity of pixelformats
			rating += (32 - NumBitsOn( (pf->Flags ^ SeekFlags) & RDRIVER_PF_MAJOR_MASK )) <<6;
		}

		pfops = jePixelFormat_GetOperations(pf->PixelFormat);

		if ( pf->PixelFormat == SeekFormat1 )
		{
			rating += 1<<22;
		}
		else if ( pf->PixelFormat == SeekFormat2 )
		{
			rating += 1<<21;
		}
		else if ( jePixelFormat_IsRaw(SeekFormat1) && jePixelFormat_IsRaw(pf->PixelFormat) )
		{
		int32 R,G,B,A;
			// measure similarity
			R = (seekops->RMask >> seekops->RShift) ^ (pfops->RMask >> pfops->RShift);
			G = (seekops->GMask >> seekops->GShift) ^ (pfops->GMask >> pfops->GShift);
			B = (seekops->BMask >> seekops->BShift) ^ (pfops->BMask >> pfops->BShift);
			A = (seekops->AMask >> seekops->AShift) ^ (pfops->AMask >> pfops->AShift);
			R = 8 - NumBitsOn(R);
			G = 8 - NumBitsOn(G);
			B = 8 - NumBitsOn(B);
			A = 4*(8 - NumBitsOn(A)); // right number of A bits is molto importante
			rating += R + G + B + A;
		}

		FoundAlpha = JE_FALSE;

		if ( NumBitsOn(pfops->AMask) > 2 )
			FoundAlpha = JE_TRUE;

		if ( jePixelFormat_HasPalette(pf->PixelFormat) )
		{
                        // if Pixelformat is 8BIT_PAL , look at the palette's format to see if it
			//		had alpha!
			assert(DriverPalFormat);
			if ( jePixelFormat_HasGoodAlpha(DriverPalFormat->PixelFormat) )
				FoundAlpha = JE_TRUE;
		}

		if ( SeekAlpha && FoundAlpha )
			rating += 1<<24; // HIGHEST ! (except separates)
		else if ( (!SeekAlpha) && (!FoundAlpha) )
			rating += 1<<16; // LOWEST

		if ( (pf->Flags & RDRIVER_PF_HAS_ALPHA_SURFACE) && SeekSeparates )
		{
			// separates doesn't count as alpha unless we asked for it !
			rating += 1<<25; // VERY HIGHEST !
		}
	
		if ( (pf->PixelFormat == JE_PIXELFORMAT_16BIT_1555_ARGB) && SeekCK )
		{
			rating += 1<<23; // just lower than alpha
		}
		else if ( JE_BOOLSAME((pf->Flags & RDRIVER_PF_CAN_DO_COLORKEY),SeekCK) )
		{
			rating += 1<<20;
		}

		if ( SeekCK && FoundAlpha ) //better than nothing!
		{
			rating += 1<<17;	// higher than !FoundAlpha
		}

		FormatRating[i] = rating;
	}

	rating = 0;

	for(i=0;i<ArrayLen;i++)
	{
		if ( FormatRating[i] > rating )
		{
			rating = FormatRating[i];
			*pTarget = DriverFormatsArray[i];
		}
	}

	if ( rating == 0)
	{
		jeErrorLog_AddString(-1,"ChooseDriverFormat : no valid formats found!", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}
#endif

jeTexture * jeBitmap_CreateTHandle(DRV_Driver *Driver,int32 Width,int32 Height,int32 NumMipLevels,
			jePixelFormat SeekFormat1,jePixelFormat SeekFormat2,jeBoolean SeekCK,jeBoolean SeekAlpha,jeBoolean SeekSeparates,uint32 DriverFlags)
{
jeRDriver_PixelFormat DriverFormats[MAX_DRIVER_FORMATS];
jeRDriver_PixelFormat *DriverFormatsPtr;
int32 DriverFormatsCount;
jeRDriver_PixelFormat DriverFormat;
jeTexture * Ret;

	DriverFormatsPtr = DriverFormats;
	Driver->EnumPixelFormats(EnumPFCB,&DriverFormatsPtr);
	DriverFormatsCount = ((uint32)DriverFormatsPtr - (uint32)DriverFormats)/sizeof(*DriverFormatsPtr);
	assert(DriverFormatsCount < MAX_DRIVER_FORMATS && DriverFormatsCount >= 0);

	if ( DriverFormatsCount == 0 )
	{
		jeErrorLog_AddString(-1,"Bitmap_CreateTHandle : no formats found!", NULL);
		return NULL;
	}

	if ( ! SeekFormat1 )
		SeekFormat1 = SeekFormat2;
	else if ( ! SeekFormat2 )
		SeekFormat2 = SeekFormat1;

	assert( jePixelFormat_IsValid(SeekFormat1) );
	assert( jePixelFormat_IsValid(SeekFormat2) );

	// now choose DriverFormat
	if ( ! jeBitmap_ChooseDriverFormat(SeekFormat1,SeekFormat2,SeekCK,SeekAlpha,SeekSeparates,DriverFlags,
										DriverFormats,DriverFormatsCount,&DriverFormat) )
		return NULL;

	assert( jePixelFormat_IsValid(DriverFormat.PixelFormat) );

#if 1 //{
	Log_Printf("Bitmap : Chose %s for %s",
		jePixelFormat_Description(DriverFormat.PixelFormat),
		jePixelFormat_Description(SeekFormat1));

	if ( SeekFormat1 != SeekFormat2 )
		Log_Printf(" (%s)",jePixelFormat_Description(SeekFormat2));
	if ( SeekCK )
		Log_Printf(" (sought CK)");
	if ( SeekAlpha )
		Log_Printf(" (sought Alpha)");
	if ( DriverFormat.Flags & RDRIVER_PF_2D )
		Log_Printf(" (2D)");
	if ( DriverFormat.Flags & RDRIVER_PF_3D )
		Log_Printf(" (3D)");
	if ( DriverFormat.Flags & RDRIVER_PF_CAN_DO_COLORKEY )
		Log_Printf(" (can CK)");
	if ( DriverFormat.Flags & RDRIVER_PF_PALETTE )
		Log_Printf(" (Palette)");
	if ( DriverFormat.Flags & RDRIVER_PF_ALPHA_SURFACE )
		Log_Printf(" (Alpha)");
	if ( DriverFormat.Flags & RDRIVER_PF_LIGHTMAP )
		Log_Printf(" (Lightmap)");

	Log_Printf("\n");
#endif //}

	Ret = Driver->THandle_Create(
		Width,Height,
		NumMipLevels,
		&DriverFormat);

	if ( ! Ret )
	{
		jeErrorLog_AddString(-1, Driver->LastErrorStr, NULL);
		jeErrorLog_AddString(-1,"Bitmap_CreateTHandle : Driver->THandle_Create failed", NULL);
	}

return Ret;
}

JETAPI	jeBoolean	JETCC	jeBitmap_HasAlpha(const jeBitmap * Bmp)
{
	assert( jeBitmap_IsValid(Bmp) );
	
	if ( Bmp->Alpha )
		return JE_TRUE;

	if ( Bmp->Wavelet )
		return jeWavelet_HasAlpha(Bmp->Wavelet);

	if ( jePixelFormat_HasGoodAlpha(Bmp->Info.Format) )
		return JE_TRUE;

	if ( jePixelFormat_HasPalette(Bmp->Info.Format) && Bmp->Info.Palette )
	{
		if ( jePixelFormat_HasGoodAlpha(Bmp->Info.Palette->Format) )
			return JE_TRUE;
	}	

return JE_FALSE;
}

jeBoolean	BITMAP_JET_INTERNAL jeBitmap_AttachToDriver(jeBitmap *Bmp, 
	DRV_Driver * Driver, uint32 DriverFlags)
{

	/**************
	* When you want to change the Driver,
	* I still need a copy of the old one to get the bits out
	* of the old THandles.  That is:
	* 
	* 	AttachDriver(Bmp,Driver)
	* 	<do stuff>
	* 	Change Driver Entries
	* 	AttachDriver(Bmp,Driver)
	* 
	* is forbidden!  The two different options are :
	* 
	* 	1.
	* 
	* 	AttachDriver(Bmp,Driver)
	* 	<do stuff>
	* 	DetachDriver(Bmp)
	* 	Change Driver Entries
	* 	AttachDriver(Bmp,Driver)
	* 
	* 	2.
	* 
	* 	AttachDriver(Bmp,Driver1)
	* 	<do stuff>
	* 	Driver2 = copy of Driver1
	* 	Change Driver2 Entries
	* 	AttachDriver(Bmp,Driver2)
	* 	Free Driver1
	* 
	* This isn't so critical when just changing modes,
	* but is critical when changing drivers.
	* 
	****************/

	assert( jeBitmap_IsValid(Bmp) );

	if ( Bmp->LockOwner || Bmp->DataOwner || Bmp->LockCount )
	{
		jeErrorLog_AddString(-1,"AttachToDriver : not an isolated bitmap", NULL);
		return JE_FALSE;
	}

	if ( Bmp->DriverHandle && Bmp->Driver == Driver )
	{
		assert( DriverFlags == 0 || DriverFlags == Bmp->DriverFlags );
		return JE_TRUE;
	}

	if ( ! jeBitmap_DetachDriver(Bmp,JE_TRUE) )
	{
		jeErrorLog_AddString(-1,"AttachToDriver : detach failed", NULL);
		return JE_FALSE;
	}

	if ( DriverFlags == 0 )
	{
		DriverFlags = Bmp->DriverFlags;
		if ( ! DriverFlags )
		{
			//	return JE_FALSE;
			// ? {}
			DriverFlags = RDRIVER_PF_3D;
		}
	}

	if ( Driver )
	{
	int32 NumMipLevels;
	int32 Width,Height;
	jeBoolean WantAlpha;
	jeTexture * DriverHandle;

//		jeBitmap_PeekReady(Bmp); // this is about to be done in UpdateSystem anyway..

//		if ( Bmp->DriverFlags & RDRIVER_PF_COMBINE_LIGHTMAP )
//			Bmp->SeekMipCount = max(Bmp->SeekMipCount,4);

		NumMipLevels = max(Bmp->SeekMipCount,(Bmp->Info.MaximumMip + 1));
		if ( NumMipLevels > 4 ) NumMipLevels = 4; // {} kind of a hack, our drivers ignore mips > 4

		// make sizes power-of-two and square
		// {} note : must let drivers do this to correctly scale UV's 
		Width	= Bmp->Info.Width;
		Height	= Bmp->Info.Height;

		WantAlpha = jeBitmap_HasAlpha(Bmp);
		if ( jePixelFormat_HasGoodAlpha(Bmp->PreferredFormat) )
			WantAlpha = JE_TRUE;

		assert( jeBitmap_IsValid(Bmp) );

		DriverHandle = jeBitmap_CreateTHandle(Driver,Width,Height,NumMipLevels,
			Bmp->PreferredFormat,Bmp->Info.Format,Bmp->Info.HasColorKey,
			WantAlpha, (Bmp->Alpha) ? JE_TRUE : JE_FALSE,
			DriverFlags);

		assert( jeBitmap_IsValid(Bmp) );

		if ( ! DriverHandle )
			return JE_FALSE;

		Bmp->DriverHandle = DriverHandle;
		Bmp->Driver = Driver;
		Bmp->DriverFlags = DriverFlags;

#ifdef _DEBUG
		Bmp->DriverInfo = Bmp->Info;
		assert( jeBitmap_IsValid(Bmp) );
#endif
		clear(&(Bmp->DriverInfo));

		Bmp->DriverMipBase = 0;
		if ( ! jeBitmap_MakeDriverLockInfo(Bmp,0,&(Bmp->DriverInfo)) )
		{
			jeErrorLog_AddString(-1,"AttachToDriver : updateinfo", NULL);
			return JE_FALSE;
		}

		Bmp->DriverInfo.MinimumMip = 0;

		Width = Bmp->Info.Width / Bmp->DriverInfo.Width;
		while( Width > 1 )
		{
			Bmp->DriverInfo.MinimumMip ++;
			Width >>= 1;
		}

		Bmp->DriverMipBase = Bmp->DriverInfo.MinimumMip;

		Bmp->DriverInfo.MaximumMip = Bmp->DriverInfo.MinimumMip + NumMipLevels - 1;
		
		assert( jeBitmap_IsValid(Bmp) );

/*******
		if ( jePixelFormat_HasPalette(Bmp->DriverInfo.Format) )
		{
			if ( ! Bmp->Info.Palette )
			{
				Bmp->Info.Palette = createPaletteFromBitmapNoLock(Bmp, JE_FALSE);
				if ( ! Bmp->Info.Palette )
				{
					jeErrorLog_AddString(-1,"AttachToDriver : createPalette failed!", NULL);
					jeBitmap_Palette_Destroy(&(Bmp->DriverInfo.Palette));
					Driver->THandle_Destroy(Bmp->DriverHandle);
					Bmp->DriverHandle = NULL;
					return JE_FALSE;
				}
			}

			if ( ! jeBitmap_AllocPalette(&(Bmp->DriverInfo), Bmp->Info.Palette->Format,Bmp->Driver) )
			{
				jeErrorLog_AddString(-1,"AttachToDriver : Palette_Create", NULL);
				Driver->THandle_Destroy(Bmp->DriverHandle);
				Bmp->DriverHandle = NULL;
				return JE_FALSE;
			}
			assert( Bmp->DriverInfo.Palette->DriverHandle );

			if ( ! Bmp->DriverInfo.HasColorKey )
			{
				Bmp->DriverInfo.HasColorKey = Bmp->DriverInfo.Palette->HasColorKey;
				Bmp->DriverInfo.ColorKey = Bmp->DriverInfo.Palette->ColorKey;
			}

			jeBitmap_Palette_Copy(Bmp->Info.Palette,Bmp->DriverInfo.Palette);

			if ( ! Driver->THandle_SetPalette(Bmp->DriverHandle,Bmp->DriverInfo.Palette->DriverHandle) )
			{
				jeErrorLog_AddString(-1,"AttachToDriver : THandle_SetPalette", NULL);
				jeBitmap_Palette_Destroy(&(Bmp->DriverInfo.Palette));
				Driver->THandle_Destroy(Bmp->DriverHandle);
				Bmp->DriverHandle = NULL;
				return JE_FALSE;
			}
		}
*******/

		assert( jeBitmap_IsValid(Bmp) );

#ifdef DONT_DEC_STREAMING
		if (	Bmp->StreamingStatus >= JE_BITMAP_STREAMING_STARTED && 
				Bmp->StreamingStatus < JE_BITMAP_STREAMING_DATADONE )
		{
			assert( Bmp->Wavelet );
			assert( jeWavelet_StreamingJob(Bmp->Wavelet) );
			Bmp->StreamingTHandle = JE_TRUE;
			return JE_TRUE;
		}
#endif

		if ( ! jeBitmap_Update_SystemToDriver(Bmp) )
		{
			jeErrorLog_AddString(-1,"AttachToDriver : Update_SystemToDriver", NULL);
			Driver->THandle_Destroy(Bmp->DriverHandle);
			Bmp->DriverHandle = NULL;
			return JE_FALSE;
		}

		// {} Palette : Update_System calls Blit_Data, which should build it for us 
		//		if Driver is pal & System isn't
	}

return JE_TRUE;
}

jeBoolean jeBitmap_FixDriverFlags(uint32 *pFlags)
{
uint32 DriverFlags;
	assert(pFlags);
	DriverFlags = *pFlags;
	
	if ( DriverFlags & RDRIVER_PF_COMBINE_LIGHTMAP )
		DriverFlags |= RDRIVER_PF_3D;
	if ( DriverFlags & RDRIVER_PF_CAN_DO_COLORKEY )
	{
		// <> someone is doing this!
		// bad!
		DriverFlags ^= RDRIVER_PF_CAN_DO_COLORKEY;
		//	return JE_FALSE;
	}
	if ( (DriverFlags & RDRIVER_PF_COMBINE_LIGHTMAP) &&
		(DriverFlags & (RDRIVER_PF_LIGHTMAP | RDRIVER_PF_PALETTE) ) )
		return JE_FALSE;
	if ( NumBitsOn(DriverFlags & RDRIVER_PF_MAJOR_MASK) == 0 )
		return JE_FALSE;
	*pFlags = DriverFlags;
return JE_TRUE;
}

jeBoolean BITMAP_JET_INTERNAL jeBitmap_SetDriverFlags(jeBitmap *Bmp,uint32 Flags)
{
	assert( jeBitmap_IsValid(Bmp) );
	assert(Flags);
	if ( ! jeBitmap_FixDriverFlags(&Flags) )
	{
		Bmp->DriverFlags = 0;
		return JE_FALSE;
	}
	Bmp->DriverFlags = Flags;
return JE_TRUE;
}

jeBoolean BITMAP_JET_INTERNAL jeBitmap_DetachDriver(jeBitmap *Bmp,jeBoolean DoUpdate)
{
jeBoolean Ret = JE_TRUE;

	assert(jeBitmap_IsValid(Bmp) );

	if ( Bmp->LockOwner || Bmp->DataOwner || Bmp->LockCount )
	{
		jeErrorLog_AddString(-1,"DetachDriver : not an isolated bitmap!", NULL);
		return JE_FALSE;
	}

	if ( Bmp->RefCount > 1 )
		DoUpdate = JE_TRUE;

	if ( Bmp->Driver && Bmp->DriverHandle )
	{
		if ( DoUpdate )
		{
			if ( ! jeBitmap_Update_DriverToSystem(Bmp) )
			{
				jeErrorLog_AddString(-1,"DetachDriver : Update_DriverToSystem", NULL);
				Ret = JE_FALSE;
			}
			assert(Bmp->DriverDataChanged == JE_FALSE);
		}
			Bmp->Driver->THandle_Destroy(Bmp->DriverHandle);
		Bmp->DriverHandle = NULL;
	}

	if ( Bmp->DriverInfo.Palette )
	{
		// save it for later in case we re-attach
		if ( ! Bmp->Info.Palette )
		{
		jeBitmap_Palette * NewPal;
		jePixelFormat Format;
			Format = Bmp->DriverInfo.Palette->Format;
			NewPal = jeBitmap_Palette_Create(Format,256);
			if ( NewPal )
			{
				if ( jeBitmap_Palette_Copy(Bmp->DriverInfo.Palette,NewPal) )
				{
					Bmp->Info.Palette = NewPal;
				}
				else
				{
					jeBitmap_Palette_Destroy(&NewPal);
				}
			}
		}

		jeBitmap_Palette_Destroy(&(Bmp->DriverInfo.Palette));
	}

	if ( Bmp->Alpha )
	{
		if ( ! jeBitmap_DetachDriver(Bmp->Alpha,DoUpdate) )
		{
			jeErrorLog_AddString(-1,"DetachDriver : detach alpha", NULL);
			Ret = JE_FALSE;
		}
	}

	Bmp->DriverInfo.Width = Bmp->DriverInfo.Height = Bmp->DriverInfo.Stride = 0;
	Bmp->DriverInfo.MinimumMip = Bmp->DriverInfo.MaximumMip = 0;
	Bmp->DriverInfo.ColorKey = Bmp->DriverInfo.HasColorKey = 0;
	Bmp->DriverInfo.Format = JE_PIXELFORMAT_NO_DATA;
	Bmp->DriverInfo.Palette = NULL;
	Bmp->DriverMipLock = 0;
	Bmp->DriverBitsLocked = JE_FALSE;
	Bmp->DriverDataChanged = JE_FALSE;
	Bmp->DriverHandle = NULL;
	Bmp->Driver = NULL;

	//Bmp->DriverFlags left intentionally !

return Ret;
}

jeBoolean JETCC jeBitmap_SetGammaCorrection_DontChange(jeBitmap *Bmp,jeFloat Gamma)
{
	assert(jeBitmap_IsValid(Bmp));
	assert( Gamma > 0.0f );

	if ( ! Bmp->DriverGammaSet )
	{
		Bmp->DriverGammaLast = Bmp->DriverGamma;
		Bmp->DriverGamma = Gamma;
	}

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_SetGammaCorrection(jeBitmap *Bmp,jeFloat Gamma,jeBoolean Apply)
{
	assert(jeBitmap_IsValid(Bmp));
	assert( Gamma > 0.0f );

	/***

	there are actually some anomalies involved in exposing this to the user:
		if the driver's gamma correction is turned on, then this Gamma will be applied *in addition* to the driver gamma.
	There's no easy way to avoid that problem.

	we like to expose this to the user so that they can disable software gamma correction on some bitmaps
		(eg. procedurals)
	perhaps provide a Bitmap_DisableGamma(Bmp) instead of SetGamma ?

	**/

	if ( Apply && Bmp->DriverHandle )
	{
		jeBitmap_PeekReady(Bmp);
		if ( fabs(Bmp->DriverGamma - Gamma) > 0.1f )
		{
			if ( jePixelFormat_BytesPerPel(Bmp->Info.Format) == 0 && Bmp->DriverHandle )
			{
				// system format is compressed, and Bmp is on the card

				if ( (Bmp->DriverGammaLast >= Bmp->DriverGamma && Bmp->DriverGamma >= Gamma) ||
					 (Bmp->DriverGammaLast <= Bmp->DriverGamma && Bmp->DriverGamma <= Gamma) )
				{
					// moving in the same direction

					// invert the old
					if ( ! jeBitmap_Gamma_Apply(Bmp,JE_TRUE) )
						return JE_FALSE;

					Bmp->DriverGammaLast = Bmp->DriverGamma;
					Bmp->DriverGamma = Gamma;

					// apply the new
					if ( ! jeBitmap_Gamma_Apply(Bmp,JE_FALSE) )
						return JE_FALSE;
				}
				else
				{
					// changed direction so must do an update

					if ( ! jeBitmap_Update_DriverToSystem(Bmp) )
						return JE_FALSE;

					Bmp->DriverGammaLast = Bmp->DriverGamma = Gamma;
					
					if ( ! jeBitmap_Update_SystemToDriver(Bmp) )
						return JE_FALSE;
				}
			}
			else
			{
				if ( ! jeBitmap_Update_DriverToSystem(Bmp) )
					return JE_FALSE;

				Bmp->DriverGammaLast = Bmp->DriverGamma = Gamma;
				
				if ( ! jeBitmap_Update_SystemToDriver(Bmp) )
					return JE_FALSE;
			}
		}
	}
	else
	{
		Bmp->DriverGamma = Gamma;
	}
	Bmp->DriverGammaSet = JE_TRUE;

return JE_TRUE;
}

jeTexture * BITMAP_JET_INTERNAL jeBitmap_GetTHandle(const jeBitmap *Bmp)
{
//	assert( jeBitmap_IsValid(Bmp) );

	// <> make this an assert?
	//if ( ! Bmp->DriverHandle )
	//	return NULL;

	if ( Bmp->StreamingTHandle )
		jeBitmap_PeekReady(Bmp);

	return Bmp->DriverHandle;
}

jeBoolean jeBitmap_Update_SystemToDriver(jeBitmap *Bmp)
{
jeBitmap * SrcLocks[MAXMIPLEVELS];
jeBoolean Ret,MipsChanged;
int32 mip,mipMin,mipMax;
jeTexture * SaveDriverHandle;
jeBitmap * SaveAlpha;
int32 SaveMaxMip;
	
	assert( jeBitmap_IsValid(Bmp) );

	/**

		this function is totally hacked out, because what we really need to do
		is lock Bmp for Read (system) & Write (driver) , but that's illegal!

	**/

	if ( Bmp->LockOwner )
		Bmp = Bmp->LockOwner;

	if ( Bmp->LockCount > 0 || Bmp->DataOwner )
	{
		jeErrorLog_AddString(-1,"Update_SystemToDriver : not an original bitmap", NULL);
		return JE_FALSE;
	}

	if ( ! Bmp->DriverHandle )
	{
		jeErrorLog_AddString(-1,"Update_SystemToDriver : no driver data", NULL);
		return JE_FALSE;
	}

#if 0 // <> NO! YOU CANNOT CALL PEEKREADY!	PeekReady calls us !
	jeBitmap_PeekReady(Bmp);
#endif

	//if Bmp->Format == Wavelet && Wavelet_CanDoMips , 
	//	then just do a LockForWrite & direct decompress!

	// <> thread the wavelet decompressor

	if ( Bmp->Info.Format == JE_PIXELFORMAT_WAVELET &&
		jeWavelet_CanDecompressMips(Bmp->Wavelet,&(Bmp->DriverInfo)) )
	{
	jeBitmap * DstLocks[MAXMIPLEVELS];
	jeBitmap_Info Infos[MAXMIPLEVELS];
	jeBitmap_Info * InfoPtrs[MAXMIPLEVELS];
	void * Bits[MAXMIPLEVELS];
	int32 i;

		mipMax = Bmp->DriverInfo.MaximumMip;
		if ( ! jeBitmap_LockForWrite(Bmp,DstLocks,0,mipMax) )
			return JE_FALSE;

		for(i=0;i<=mipMax;i++)
		{
			InfoPtrs[i] = &Infos[i];
			jeBitmap_GetInfo(DstLocks[i],InfoPtrs[i],NULL);
			Bits[i] = jeBitmap_GetBits(DstLocks[i]);
			assert(Bits[i]);
		}

		if ( ! jeWavelet_DecompressMips(Bmp->Wavelet,(const jeBitmap_Info **)InfoPtrs,(const void **)Bits,0,mipMax) )
		{
			jeErrorLog_AddString(-1,"Update_SystemToDriver : Wavelet_DecompressMips failed!", NULL);
			jeBitmap_UnLockArray_NoChange(DstLocks,mipMax+1);
			return JE_FALSE;
		}

		jeBitmap_UnLockArray_NoChange(DstLocks,mipMax+1);
		
		if ( ! jeBitmap_Gamma_Apply(Bmp,JE_FALSE) )
		{
			jeErrorLog_AddString(-1,"AttachToDriver : Gamma_Apply failed!", NULL);
			return JE_FALSE;
		}

	return JE_TRUE;
	}

	MipsChanged = JE_FALSE;
	for(mip=Bmp->DriverInfo.MinimumMip;mip<=Bmp->DriverInfo.MaximumMip;mip++)
	{
		if ( Bmp->Modified[mip] && mip != Bmp->Info.MinimumMip )
		{
			assert(Bmp->Data[mip]);
			MipsChanged = JE_TRUE;
		}
	}

	//make mips after driver blit
	
	mipMin = Bmp->DriverInfo.MinimumMip;

	if ( MipsChanged )
		mipMax = Bmp->DriverInfo.MaximumMip;
	else
		mipMax = mipMin;


	SaveDriverHandle = Bmp->DriverHandle;
	Bmp->DriverHandle = NULL;	// so Lock() won't use the driver data

	SaveAlpha = Bmp->Alpha;

	if ( Bmp->Alpha && ! jePixelFormat_HasGoodAlpha(Bmp->DriverInfo.Format) && 
			(Bmp->DriverFlags & RDRIVER_PF_HAS_ALPHA_SURFACE) )
	{
		// hide the alpha so that it won't be used to make a colorkey in the target
		// we'll blit it independently later
		Bmp->Alpha = NULL;
	}

	// note : LockForReadNative calls PeekReady, but DriverHandle has been
	//	set to NULL so we don't get called again!

	if ( ! jeBitmap_LockForReadNative(Bmp,SrcLocks,mipMin,mipMax) )
	{
		jeErrorLog_AddString(-1,"Update_SystemToDriver : LockForReadNative", NULL);
		return JE_FALSE;
	}

	Ret = JE_TRUE;
	Bmp->DriverHandle = SaveDriverHandle;
	Bmp->Alpha = NULL;

	// we should have always updated the driver to system before fiddling the system
	assert( ! Bmp->DriverDataChanged );

	/**

		Bmp is a driver BMP

		the SrcLocks are locks of the system bits.

	**/

	for(mip=mipMin;mip <=mipMax;mip++)
	{
	jeBitmap *SrcMip;
	void * SrcBits,*DstBits;
	jeBitmap_Info DstInfo;

		SrcMip = SrcLocks[mip - mipMin];
		SrcBits = jeBitmap_GetBits(SrcMip);

		DstInfo = Bmp->DriverInfo;

		if ( ! jeBitmap_MakeDriverLockInfo(Bmp,mip,&DstInfo) )
		{
			jeErrorLog_AddString(-1,"Update_SystemToDriver : MakeInfo", NULL);
			Ret = JE_FALSE;
			continue;
		}

		// Zooma! THandle_Lock might lock the Win16 Lock !
		//	this is really bad when _BlitData is a wavelet decompress !
		// {} try this : decompress to a buffer in memory (on a thread)
		//	then THandle_Lock and just do a (prefetching) memcpy
//<>		#pragma message("Bitmap : minimize time spent in a THandle_Lock!")

		if ( ! Bmp->Driver->THandle_Lock(SaveDriverHandle,mip - Bmp->DriverMipBase,&DstBits) )
		{
			jeErrorLog_AddString(-1,"Update_SystemToDriver : THandle_Lock", NULL);
			Ret = JE_FALSE;
			continue;
		}

		if ( ! SrcBits || ! DstBits )
		{
			jeErrorLog_AddString(-1,"Update_SystemToDriver : No Bits", NULL);
			Ret = JE_FALSE;
			continue;
		}

		assert( DstInfo.Palette == Bmp->DriverInfo.Palette );

		if ( ! jeBitmap_BlitData(	&(SrcMip->Info),SrcBits,SrcMip,
									&DstInfo,		DstBits,Bmp,
									SrcMip->Info.Width,SrcMip->Info.Height) )
		{
			jeErrorLog_AddString(-1,"Update_SystemToDriver : BlitData", NULL);
			assert(0);
			Ret = JE_FALSE;
			continue;
		}

		if ( ! Bmp->Driver->THandle_UnLock(SaveDriverHandle,mip - Bmp->DriverMipBase) )
		{
			jeErrorLog_AddString(-1,"Update_SystemToDriver : THandle_UnLock", NULL);
			Ret = JE_FALSE;
			continue;
		}

		// normally this would be done by the Bitmap_UnLock ,
		//  but since we don't lock ..
		if ( DstInfo.Palette != Bmp->DriverInfo.Palette )
		{
			//assert( OldDstPal == NULL );
			jeBitmap_SetPalette(Bmp,DstInfo.Palette);
			jeBitmap_Palette_Destroy(&(DstInfo.Palette));
			// must destroy here, since DstInfo is on the stack!
		}
	}

	Bmp->Alpha = SaveAlpha;
	Bmp->DriverBitsLocked = JE_FALSE;
	Bmp->DriverMipLock = 0;
	Bmp->DriverDataChanged = JE_FALSE;

	jeBitmap_UnLockArray(SrcLocks, mipMax - mipMin + 1 );

	if ( ! Ret )
	{
		jeErrorLog_AddString(-1,"Update_SystemToDriver : Locking and Blitting error", NULL);
	}

	if ( Bmp->Alpha && ! jePixelFormat_HasGoodAlpha(Bmp->DriverInfo.Format) && 
			(Bmp->DriverFlags & RDRIVER_PF_HAS_ALPHA_SURFACE) )
	{
	jeTexture * AlphaTH;

		// blit the alpha surface to the separate alpha

		AlphaTH = Bmp->Driver->THandle_GetAlpha(Bmp->DriverHandle);
		if ( !AlphaTH || AlphaTH != Bmp->Alpha->DriverHandle)
		{
			if ( ! jeBitmap_AttachToDriver(Bmp->Alpha,Bmp->Driver,Bmp->Alpha->DriverFlags | RDRIVER_PF_ALPHA_SURFACE) )
			{
				jeErrorLog_AddString(-1,"AttachToDriver : attach Alpha", NULL);
				return JE_FALSE;
			}

			assert(Bmp->Alpha->DriverHandle);
			if ( ! Bmp->Driver->THandle_SetAlpha(Bmp->DriverHandle,Bmp->Alpha->DriverHandle) )
			{
				jeErrorLog_AddString(-1,"AttachToDriver : THandle_SetAlpha", NULL);
				jeBitmap_DetachDriver(Bmp->Alpha,JE_FALSE);
				jeBitmap_DetachDriver(Bmp,JE_FALSE);
				return JE_FALSE;
			}
			
			AlphaTH = Bmp->Driver->THandle_GetAlpha(Bmp->DriverHandle);
			assert(AlphaTH == Bmp->Alpha->DriverHandle);
		}
	}

	// now bits are on driver , gamma correct them

	//for gamma : just Gamma up to mipMax then make mips from it
	//	seems to work

	SaveMaxMip = Bmp->DriverInfo.MaximumMip;
	Bmp->DriverInfo.MaximumMip = mipMax;
	if ( ! jeBitmap_Gamma_Apply(Bmp,JE_FALSE) )
	{
		jeErrorLog_AddString(-1,"AttachToDriver : Gamma_Apply failed!", NULL);
		Ret = JE_FALSE;
	}
	Bmp->DriverInfo.MaximumMip = SaveMaxMip;

	if ( ! MipsChanged && mipMax < Bmp->DriverInfo.MaximumMip )
	{
		for(mip=mipMax+1;mip<= Bmp->DriverInfo.MaximumMip; mip++)
		{
			if ( ! jeBitmap_UpdateMips(Bmp,mip-1,mip) )
			{
				jeErrorLog_AddString(-1,"AttachToDriver : UpdateMips on driver failed!", NULL);
				return JE_FALSE;
			}
		}
	}

	Bmp->DriverDataChanged = JE_FALSE; // in case _SetPal freaks us out

return Ret;
}

jeBoolean jeBitmap_Update_DriverToSystem(jeBitmap *Bmp)
{
jeBitmap *DriverLocks[MAXMIPLEVELS];
jeBoolean Ret;
int32 mip;
	
	assert( jeBitmap_IsValid(Bmp) );
	jeBitmap_WaitReady(Bmp);

	if ( Bmp->LockOwner )
		Bmp = Bmp->LockOwner;

	if ( Bmp->LockCount > 0 || Bmp->DataOwner )
	{
		jeErrorLog_AddString(-1,"Update_DriverToSystem : not an original bitmap", NULL);
		return JE_FALSE;
	}

	if ( ! Bmp->DriverHandle )
	{
		jeErrorLog_AddString(-1,"Update_DriverToSystem : no driver data", NULL);
		return JE_FALSE;
	}

	if ( ! Bmp->DriverDataChanged )
		return JE_TRUE;

	// bits are on driver; undo the gamma to copy them home

	Log_Puts("Bitmap : Doing Update_DriverToSystem");

	if ( ! jeBitmap_Gamma_Apply(Bmp,JE_TRUE) ) // undo the gamma!
		return JE_FALSE;

	if ( Bmp->Info.Palette && Bmp->DriverInfo.Palette )
	{
		if ( ! jeBitmap_Palette_Copy(Bmp->DriverInfo.Palette,Bmp->Info.Palette) )
		{
			jeErrorLog_AddString(-1,"Update_DriverToSystem : Palette_Copy", NULL);
		}
	}

	if ( jeBitmap_LockForReadNative(Bmp,DriverLocks,
			Bmp->DriverInfo.MinimumMip,Bmp->DriverInfo.MaximumMip) )
	{
		Ret = JE_TRUE;

		for(mip=Bmp->DriverInfo.MinimumMip;mip <=Bmp->DriverInfo.MaximumMip;mip++)
		{	
		jeBitmap *MipBmp;
		jeBitmap_Info SystemInfo;

			MipBmp = DriverLocks[mip];

			if ( Bmp->Modified[mip] )
			{
			void *DriverBits,*SystemBits;
				DriverBits = jeBitmap_GetBits(MipBmp);
				assert( MipBmp->DriverBitsLocked );

				if ( ! jeBitmap_AllocSystemMip(Bmp,mip) )
					Ret = JE_FALSE;

				SystemBits = Bmp->Data[mip];

				jeBitmap_MakeMipInfo(&(Bmp->Info),mip,&SystemInfo);

				if ( DriverBits && SystemBits )
				{
					// _Update_DriverToSystem
					// {} palette (not) made in AttachToDriver; must be made in here->
					if ( ! jeBitmap_BlitData(	&(MipBmp->Info), DriverBits, MipBmp,
												&SystemInfo,	SystemBits, Bmp,
												SystemInfo.Width,SystemInfo.Height) )
						Ret = JE_FALSE;
				}
				else
				{
					Ret = JE_FALSE;
				}
			}
			
			jeBitmap_UnLock(DriverLocks[mip]);
		}

		Bmp->DriverDataChanged = JE_FALSE;
	}
	else
	{
		Ret = JE_FALSE;
	}

	if ( ! Ret )
	{
		jeErrorLog_AddString(-1,"Update_DriverToSystem : Locking and Blitting error", NULL);
	}

	if ( ! jeBitmap_Gamma_Apply(Bmp,JE_FALSE) ) // redo the gamma!
		return JE_FALSE;

return Ret;
}

/*}{ ************* Mip Control *****************/

// Note : all the Mip control 

JETAPI jeBoolean JETCC jeBitmap_RefreshMips(jeBitmap *Bmp)
{
int32 mip;

	assert( jeBitmap_IsValid(Bmp) );

	if ( Bmp->LockOwner || Bmp->LockCount || Bmp->DataOwner )
		return JE_FALSE;

	for(mip = (Bmp->Info.MinimumMip + 1);mip <= Bmp->Info.MaximumMip;mip++)
	{
		if ( Bmp->Data[mip] && !(Bmp->Modified[mip]) )
		{
		int32 src;
			src = mip-1;
			while( ! Bmp->Data[src] )
			{
				src--;
				if ( src < Bmp->Info.MinimumMip )
					return JE_FALSE;
			}
			if ( ! jeBitmap_UpdateMips(Bmp,src,mip) )
				return JE_FALSE;
		}
	}

#if 0	// never turn off a modified flag
	for(mip=0;mip<MAXMIPLEVELS;mip++)
		Bmp->Modified[mip] = JE_FALSE;
#endif

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_UpdateMips(jeBitmap *Bmp,int32 fm,int32 to)
{
jeBitmap * Locks[MAXMIPLEVELS];
void *FmBits,*ToBits;
jeBitmap_Info FmInfo,ToInfo;
jeBoolean Ret = JE_FALSE;

	assert( jeBitmap_IsValid(Bmp) );
	jeBitmap_WaitReady(Bmp);

	if ( Bmp->LockOwner || Bmp->LockCount > 0 || Bmp->DataOwner )
		return JE_FALSE;

	if ( fm >= to )
		return JE_FALSE;

	if ( Bmp->DriverHandle ) 
	{
		//{} this version does *NOT* make new mips if to > Bmp->DriverInfo.MaximumMip

		if ( ! jeBitmap_LockForWrite(Bmp,Locks,fm,to) )
			return JE_FALSE;

		if ( jeBitmap_GetInfo(Locks[0],&FmInfo,NULL) && jeBitmap_GetInfo(Locks[to - fm],&ToInfo,NULL) )
		{
			FmBits = jeBitmap_GetBits(Locks[0]);
			ToBits = jeBitmap_GetBits(Locks[to - fm]);
		
			if ( FmBits && ToBits )
			{
				Ret = jeBitmap_UpdateMips_Data(	&FmInfo, FmBits, 
												&ToInfo, ToBits );
			}
		}

		jeBitmap_UnLockArray_NoChange(Locks,to - fm + 1);
	}
	else
	{
		Ret = jeBitmap_UpdateMips_System(Bmp,fm,to);
	}

return Ret;
}

jeBoolean jeBitmap_UpdateMips_System(jeBitmap *Bmp,int32 fm,int32 to)
{
jeBitmap_Info FmInfo,ToInfo;
jeBoolean Ret;

	assert( jeBitmap_IsValid(Bmp) );

	// this is called to create new mips in LockFor* -> CreateLockFrom* (through MakeSystemMips)

	if ( Bmp->LockOwner )
		Bmp = Bmp->LockOwner;
//	if ( Bmp->LockCount > 0 )
//		return JE_FALSE;
	if ( Bmp->DataOwner )
		return JE_FALSE;

	// {} for compressed data, just don't make mips and say we did!
	if ( jePixelFormat_BytesPerPel(Bmp->Info.Format) < 1 )
		return JE_TRUE;

	while(Bmp->Data[fm] == NULL || fm == to )
	{
		fm--;
		if ( fm < 0 )
			return JE_FALSE;
	}

	if ( fm < Bmp->Info.MinimumMip || fm > Bmp->Info.MaximumMip ||
	     to < fm || to >= MAXMIPLEVELS )
		return JE_FALSE;

	if ( ! Bmp->Data[to] )
	{
		if ( ! jeBitmap_AllocSystemMip(Bmp,to) )
			return JE_FALSE;
	}

	assert( to > fm && fm >= 0 );

	FmInfo = ToInfo = Bmp->Info;

	FmInfo.Width = SHIFT_R_ROUNDUP(Bmp->Info.Width ,fm);
	FmInfo.Height= SHIFT_R_ROUNDUP(Bmp->Info.Height,fm);
	FmInfo.Stride= SHIFT_R_ROUNDUP(Bmp->Info.Stride,fm);
	ToInfo.Width = SHIFT_R_ROUNDUP(Bmp->Info.Width ,to);
	ToInfo.Height= SHIFT_R_ROUNDUP(Bmp->Info.Height,to);
	ToInfo.Stride= SHIFT_R_ROUNDUP(Bmp->Info.Stride,to);

	Ret = jeBitmap_UpdateMips_Data(	&FmInfo, Bmp->Data[fm],
									&ToInfo, Bmp->Data[to]);

	Bmp->Info.MaximumMip = max(Bmp->Info.MaximumMip,to);

return Ret;
}

jeBoolean jeBitmap_UpdateMips_Data(	jeBitmap_Info * FmInfo,void * FmBits,
									jeBitmap_Info * ToInfo,void * ToBits)
{
int32 fmxtra,tow,toh,toxtra,fmw,fmh,fmstep,x,y,bpp;

	assert( FmInfo && ToInfo && FmBits && ToBits );
	assert( FmInfo->Format == ToInfo->Format && FmInfo->HasColorKey == ToInfo->HasColorKey );

	tow = ToInfo->Width;
	toh = ToInfo->Height;
	toxtra = ToInfo->Stride - ToInfo->Width;
	
	x = ToInfo->Width;
	fmstep = 1;
	while( x < FmInfo->Width )
	{
		fmstep += fmstep;
		x += x;
	}

	fmw = FmInfo->Width;
	fmh = FmInfo->Height;
	fmxtra = (FmInfo->Stride - tow) * fmstep; // amazingly simple and correct! think about it!

	// fmh == 15
	// toh == 8
	// fmstep == 2
	// 7*2 <= 14 -> Ok
	if ( (toh-1)*fmstep > (fmh - 1) )
	{
		jeErrorLog_AddString(-1,"UpdateMips_Data : Vertical mip scaling doesn't match horizontal!", NULL);
		return JE_FALSE;
	}

	// {} todo : average for some special cases (16rgb,24rgb,32rgb)

	bpp = jePixelFormat_BytesPerPel(FmInfo->Format);

	if ( fmstep == 2 && bpp > 1 )
	{
	int32 R1,G1,B1,A1,R2,G2,B2,A2,R3,G3,B3,A3,R4,G4,B4,A4;
	jePixelFormat_ColorGetter GetColor;
	jePixelFormat_ColorPutter PutColor;
	const jePixelFormat_Operations *ops;
	uint8 *fmp,*fmp2,*top;

		fmp = (uint8*)FmBits;
		top = (uint8*)ToBits;

		ops = jePixelFormat_GetOperations(FmInfo->Format);
		GetColor = ops->GetColor;
		PutColor = ops->PutColor;

		fmxtra *= bpp;
		toxtra *= bpp;

		if ( FmInfo->HasColorKey )
		{
		uint32 ck,p1,p2,p3,p4;
		jePixelFormat_PixelGetter GetPixel;
		jePixelFormat_PixelPutter PutPixel;
		jePixelFormat_Decomposer DecomposePixel;

			assert( FmInfo->ColorKey == ToInfo->ColorKey );
			ck = FmInfo->ColorKey;
			GetPixel = ops->GetPixel;
			PutPixel = ops->PutPixel;
			DecomposePixel = ops->DecomposePixel;
		
			// {} the colorkey mip-subsampler
			// slow as hell; yet another reason to not use CK !
			
			for(y=toh;y--;)
			{
				//y = 7, fmh = 15; y*2+1 == fmh : last line is not a double line
				if ( (y+y + 1) == fmh )	fmp2 = fmp;
				else					fmp2 = fmp + (FmInfo->Stride*bpp);
				for(x=tow;x--;)
				{
					p1 = GetPixel(&fmp);
					p2 = GetPixel(&fmp);
					p3 = GetPixel(&fmp2);
					p4 = GetPixel(&fmp2);
					if ( p1 == ck || p4 == ck )
					{
						PutPixel(&top,ck);
					}
					else
					{
						// p1 and p4 are not ck;
						if ( p2 == ck ) p2 = p1;
						if ( p3 == ck ) p3 = p4;
						DecomposePixel(p1,&R1,&G1,&B1,&A1);
						DecomposePixel(p2,&R2,&G2,&B2,&A2);
						DecomposePixel(p3,&R3,&G3,&B3,&A3);
						DecomposePixel(p4,&R4,&G4,&B4,&A4);
						PutColor(&top,(R1+R2+R3+R4+2)>>2,(G1+G2+G3+G4+2)>>2,(B1+B2+B3+B4+2)>>2,(A1+A2+A3+A4+2)>>2);
					}
				}
				fmp += fmxtra;
				top += toxtra;
			}
		}
		else
		{
			for(y=toh;y--;)
			{
				//y = 7, fmh = 15; y*2+1 == fmh : last line is not a double line
				if ( (y+y + 1) == fmh )	fmp2 = fmp;
				else					fmp2 = fmp + (FmInfo->Stride*bpp);
				for(x=tow;x--;)
				{
					GetColor(&fmp ,&R1,&G1,&B1,&A1);
					GetColor(&fmp ,&R2,&G2,&B2,&A2);
					GetColor(&fmp2,&R3,&G3,&B3,&A3);
					GetColor(&fmp2,&R4,&G4,&B4,&A4);
					PutColor(&top,(R1+R2+R3+R4+2)>>2,(G1+G2+G3+G4+2)>>2,(B1+B2+B3+B4+2)>>2,(A1+A2+A3+A4+2)>>2);
				}
				fmp += fmxtra;
				top += toxtra;
			}
		}

		assert( top == (((uint8 *)ToBits) + ToInfo->Stride * ToInfo->Height * bpp ) );
		assert( fmp == (((uint8 *)FmBits) + FmInfo->Stride * ToInfo->Height * 2 * bpp ) );
	}
	else if ( fmstep == 2 && jePixelFormat_HasPalette(FmInfo->Format) )
	{
	int32 R,G,B;
	uint8 *fmp,*fmp2,*top;
	uint8 paldata[768],*palptr;
	int32 p;
	palInfo * PalInfo;

		assert(bpp == 1);
		assert(FmInfo->Palette);

		if ( ! jeBitmap_Palette_GetData(FmInfo->Palette,paldata,JE_PIXELFORMAT_24BIT_RGB,256) )
			return JE_FALSE;

		if ( ! (PalInfo = closestPalInit(paldata)) )
			return JE_FALSE;

		fmp = (uint8*)FmBits;
		top = (uint8*)ToBits;

		// @@ colorkey?

		for(y=toh;y--;)
		{
			//y = 7, fmh = 15; y*2+1 == fmh : last line is not a double line
			if ( (y*2 + 1) == fmh )	fmp2 = fmp;
			else					fmp2 = fmp + (FmInfo->Stride*bpp);

			for(x=tow;x--;)
			{
				p = *fmp++;
				palptr = paldata + p*3;
				R  = palptr[0]; G  = palptr[1]; B  = palptr[2]; 
				p = *fmp++;
				palptr = paldata + p*3;
				R += palptr[0]; G += palptr[1]; B += palptr[2]; 
				p = *fmp2++;
				palptr = paldata + p*3;
				R += palptr[0]; G += palptr[1]; B += palptr[2]; 
				p = *fmp2++;
				palptr = paldata + p*3;
				R += palptr[0]; G += palptr[1]; B += palptr[2]; 

				R = (R+2)>>2;
				G = (G+2)>>2;
				B = (B+2)>>2;

				p = closestPal(R,G,B,PalInfo);
				*top++ = (uint8)p;
			}
			fmp += fmxtra;
			top += toxtra;
		}

		closestPalFree(PalInfo);

		assert( top == (((uint8 *)ToBits) + ToInfo->Stride * ToInfo->Height * bpp ) );
		assert( fmp == (((uint8 *)FmBits) + FmInfo->Stride * ToInfo->Height * 2 * bpp ) );
	}
	else
	{
		// we just sub-sample to make mips, so we don't have to
		//	know anything about pixelformat.
		// (btw this spoils the whole point of mips, so we might as well kill the mip!)

		//{} Blend correctly !?

		switch( bpp )
		{
			default:
			{
				return JE_FALSE;
			}
			case 1:
			{
				uint8 *fmp,*top;
				fmp = (uint8*)FmBits;
				top = (uint8*)ToBits;
				for(y=toh;y--;)
				{
					for(x=tow;x--;)
					{
						*top++ = *fmp;
						fmp += fmstep;
					}
					fmp += fmxtra;
					top += toxtra;
				}
				break;
			}
			case 2:
			{
				uint16 *fmp,*top;
				fmp = (uint16*)FmBits;
				top = (uint16*)ToBits;
				for(y=toh;y--;)
				{
					for(x=tow;x--;)
					{
						*top++ = *fmp;
						fmp += fmstep;
					}
					fmp += fmxtra;
					top += toxtra;
				}
				break;
			}
			case 4:
			{
				uint32 *fmp,*top;
				fmp = (uint32*)FmBits;
				top = (uint32*)ToBits;
				for(y=toh;y--;)
				{
					for(x=tow;x--;)
					{
						*top++ = *fmp;
						fmp += fmstep;
					}
					fmp += fmxtra;
					top += toxtra;
				}
				break;
			}
			case 3:
			{
				uint8 *fmp,*top;
				fmp = (uint8*)FmBits;
				top = (uint8*)ToBits;
				fmstep = (fmstep - 1) * 3;
				fmxtra *= 3;
				toxtra *= 3;
				for(y=toh;y--;)
				{
					for(x=tow;x--;)
					{
						*top++ = *fmp++;
						*top++ = *fmp++;
						*top++ = *fmp++;
						fmp += fmstep;
					}
					fmp += fmxtra;
					top += toxtra;
				}
				break;
			}
		}
	}

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_ClearMips(jeBitmap *Bmp)
{
int32 mip;
DRV_Driver * Driver;

	// WARNING ! This destroys any mips!

	assert( jeBitmap_IsValid(Bmp) );
	jeBitmap_WaitReady(Bmp);

	if ( Bmp->LockOwner || Bmp->LockCount || Bmp->DataOwner )
		return JE_FALSE;

	if ( Bmp->SeekMipCount == 0 && Bmp->Info.MaximumMip == 0 )
		return JE_TRUE;

	Driver = Bmp->Driver;
	if ( Driver )
	{
		if ( ! jeBitmap_DetachDriver(Bmp,JE_TRUE) )
			return JE_FALSE;
	}
	assert(Bmp->Driver == NULL);

	mip = Bmp->Info.MinimumMip;
	if ( mip == 0 ) 
		mip++;

	Bmp->SeekMipCount = mip;

	for( ; mip <= Bmp->Info.MaximumMip ; mip++)
	{
		if ( Bmp->Data[mip] )
		{
			jeRam_Free( Bmp->Data[mip] );
			Bmp->Data[mip] = NULL;
		}
	}

	Bmp->Info.MaximumMip = Bmp->Info.MinimumMip;

	if ( Driver )
	{
		if ( ! jeBitmap_AttachToDriver(Bmp,Driver,0) )
			return JE_FALSE;
	}

return JE_TRUE;
}

JETAPI jeBoolean 	JETCC	jeBitmap_SetMipCount(jeBitmap *Bmp,int32 Count)
{
DRV_Driver * Driver;

	assert( jeBitmap_IsValid(Bmp) );
	jeBitmap_WaitReady(Bmp);

	if ( Bmp->LockOwner || Bmp->LockCount || Bmp->DataOwner )
		return JE_FALSE;

// @@ don't do this ?
//	if ( Bmp->Info.MaximumMip < (Count-1) )
//		jeBitmap_MakeSystemMips(Bmp,0,Count-1);

	if ( Bmp->SeekMipCount == Count )
		return JE_TRUE;

	Driver = Bmp->Driver;
	if ( Driver )
	{
		if ( ! jeBitmap_DetachDriver(Bmp,JE_TRUE) )
			return JE_FALSE;
	}
	assert(Bmp->Driver == NULL);

	Bmp->SeekMipCount = Count;

	if ( Driver )
	{
		if ( ! jeBitmap_AttachToDriver(Bmp,Driver,0) )
			return JE_FALSE;
	}

return JE_TRUE;
}

jeBoolean jeBitmap_MakeSystemMips(jeBitmap *Bmp,int32 low,int32 high)
{
int32 mip;

	assert( jeBitmap_IsValid(Bmp) );

	// this is that CreateLockFromMip uses to make its new data

	if ( Bmp->LockOwner )
		Bmp = Bmp->LockOwner;
//	if ( Bmp->LockCount > 0 )
//		return JE_FALSE;
	if ( Bmp->DataOwner )
		return JE_FALSE;

	// {} for compressed data, just don't make mips and say we did!
	if ( jePixelFormat_BytesPerPel(Bmp->Info.Format) < 1 )
		return JE_TRUE;

	if ( low < 0 || high >= MAXMIPLEVELS || low > high )
		return JE_FALSE;

	for( mip = low; mip <= high; mip++)
	{
		if ( ! Bmp->Data[mip] )
		{
			if ( ! jeBitmap_AllocSystemMip(Bmp,mip) )
				return JE_FALSE;
	
			if ( mip != 0 )
			{
				if ( ! jeBitmap_UpdateMips_System(Bmp,mip-1,mip) )
					return JE_FALSE;
			}
		}
	}

	Bmp->Info.MinimumMip = min(Bmp->Info.MinimumMip,low);
	Bmp->Info.MaximumMip = max(Bmp->Info.MaximumMip,high);

return JE_TRUE;
}

/*}{ ******* Miscellany ***********/

JETAPI uint32 JETCC jeBitmap_MipBytes(const jeBitmap *Bmp,int32 mip)
{
uint32 bytes;
	if ( ! Bmp )
		return 0;
	bytes = jePixelFormat_BytesPerPel(Bmp->Info.Format) * 
						SHIFT_R_ROUNDUP(Bmp->Info.Stride,mip) *
						SHIFT_R_ROUNDUP(Bmp->Info.Height,mip);
return bytes;
}

JETAPI jeBoolean JETCC jeBitmap_GetInfo(const jeBitmap *Bmp, jeBitmap_Info *Info, jeBitmap_Info *SecondaryInfo)
{
	assert( jeBitmap_IsValid(Bmp) );

	assert(Info);

	if ( Bmp->DriverHandle )
	{
		*Info = Bmp->DriverInfo;
	}
	else
	{
		*Info = Bmp->Info;
	}

	if ( SecondaryInfo )
		*SecondaryInfo = Bmp->Info;

	return JE_TRUE;
}

jeBoolean jeBitmap_MakeDriverLockInfo(jeBitmap *Bmp,int32 mip,jeBitmap_Info *Into)
{
jeTexture_Info TInfo;

	// MakeDriverLockInfo also doesn't full out the full info, so it must be a valid info first!
	// Bmp also gets some crap written into him.

	assert(Bmp && Into); // not necessarily valid

	if ( ! Bmp->DriverHandle || ! Bmp->Driver || mip < Bmp->DriverInfo.MinimumMip || mip > Bmp->DriverInfo.MaximumMip )
		return JE_FALSE;

	if ( ! Bmp->Driver->THandle_GetInfo(Bmp->DriverHandle,mip - Bmp->DriverMipBase,&TInfo) )
	{
		jeErrorLog_AddString(-1,"MakeDriverLockInfo : THandle_GetInfo", NULL);
		return JE_FALSE;
	}

	Bmp->DriverMipLock	= mip;
	Bmp->DriverFlags	= TInfo.PixelFormat.Flags;

	Into->Width			= TInfo.Width;
	Into->Height		= TInfo.Height;
	Into->Stride		= TInfo.Stride;
	Into->Format		= TInfo.PixelFormat.PixelFormat;
	Into->ColorKey		= TInfo.ColorKey;

	if ( TInfo.Flags & RDRIVER_THANDLE_HAS_COLORKEY )
		Into->HasColorKey = JE_TRUE;
	else
		Into->HasColorKey = JE_FALSE;

	Into->MinimumMip = Into->MaximumMip = mip;

	if ( jePixelFormat_HasPalette(Into->Format) && Into->Palette && Into->Palette->HasColorKey )
	{
		Into->HasColorKey = JE_TRUE;
		Into->ColorKey = Into->Palette->ColorKeyIndex;
	}

return JE_TRUE;
}

JETAPI int32 JETCC	jeBitmap_Width(const jeBitmap *Bmp)
{
	assert(Bmp);
return(Bmp->Info.Width);
}

JETAPI int32 JETCC	jeBitmap_Height(const jeBitmap *Bmp)
{
	assert(Bmp);
return(Bmp->Info.Height);
}

JETAPI jeBoolean JETCC jeBitmap_Blit(const jeBitmap *Src, int32 SrcPositionX, int32 SrcPositionY,
						jeBitmap *Dst, int32 DstPositionX, int32 DstPositionY,
						int32 SizeX, int32 SizeY )
{
	assert( jeBitmap_IsValid(Src) );
	assert( jeBitmap_IsValid(Dst) );
	return jeBitmap_BlitMipRect(Src,0,SrcPositionX,SrcPositionY,
								Dst,0,DstPositionX,DstPositionY,
								SizeX,SizeY);
}

JETAPI jeBoolean JETCC jeBitmap_BlitBitmap(const jeBitmap * Src, jeBitmap * Dst )
{
	assert( jeBitmap_IsValid(Src) );
	assert( jeBitmap_IsValid(Dst) );
	assert( Src != Dst );
	return jeBitmap_BlitMipRect(Src,0,0,0,Dst,0,0,0,-1,-1);
}

JETAPI jeBoolean JETCC jeBitmap_BlitBestMip(const jeBitmap * Src, jeBitmap * Dst )
{
int32 Width,Mip;
	assert( jeBitmap_IsValid(Src) );
	assert( jeBitmap_IsValid(Dst) );
	assert( Src != Dst );
	for(Mip=0;	(Width = SHIFT_R_ROUNDUP(Src->Info.Width,Mip)) > Dst->Info.Width ; Mip++) ;
	return jeBitmap_BlitMipRect(Src,Mip,0,0,Dst,0,0,0,-1,-1);
}

JETAPI jeBoolean JETCC jeBitmap_BlitMip(const jeBitmap * Src, int32 SrcMip, jeBitmap * Dst, int32 DstMip )
{
	assert( jeBitmap_IsValid(Src) );
	assert( jeBitmap_IsValid(Dst) );
	return jeBitmap_BlitMipRect(Src,SrcMip,0,0,Dst,DstMip,0,0,-1,-1);
}

jeBoolean jeBitmap_BlitMipRect(const jeBitmap * Src, int32 SrcMip, int32 SrcX,int32 SrcY,
									 jeBitmap * Dst, int32 DstMip, int32 DstX,int32 DstY,
							int32 SizeX,int32 SizeY)
{
jeBitmap * SrcLock,* DstLock;
jeBoolean SrcUnLock,DstUnLock;
jeBitmap_Info *SrcLockInfo,*DstLockInfo;
uint8 *SrcBits,*DstBits;
	
	assert(Src && Dst);
	jeBitmap_PeekReady(Src);
	jeBitmap_WaitReady(Dst);

	assert( Src != Dst );
	// <> if Src == Dst we could still do this, but we assert SrcMip != DstMip & be smart

	SrcUnLock = DstUnLock = JE_FALSE;

	if ( Src->LockOwner )
	{
		assert( Src->LockOwner->LockCount );
		if ( SrcMip != 0 )
		{
			jeErrorLog_AddString(-1,"BlitMipRect : Src is a lock and mip != 0", NULL);
			goto fail;
		}

		SrcLock = (jeBitmap *)Src;
	}
	else
	{
		if ( ! jeBitmap_LockForReadNative((jeBitmap *)Src,&SrcLock,SrcMip,SrcMip) )
		{
			jeErrorLog_AddString(-1,"BlitMipRect : LockForReadNative", NULL);
			goto fail;
		}
		SrcUnLock = JE_TRUE;
	}

	if ( Dst->LockOwner )
	{
		if ( DstMip != 0 )
			goto fail;
//		if ( Dst->LockOwner->LockCount >= 0 )
//			goto fail;
//		{} can't check this, cuz we use _BlitMip to create locks for read
		DstLock = Dst;
	}
	else
	{
		if ( ! jeBitmap_LockForWrite(Dst,&DstLock,DstMip,DstMip) )
		{
			jeErrorLog_AddString(-1,"BlitMipRect : LockForWrite", NULL);
			goto fail;
		}
		DstUnLock = JE_TRUE;
	}

	Src = Dst = NULL;

	if ( SrcLock->DriverHandle ) 
		SrcLockInfo = &(SrcLock->DriverInfo);
	else
		SrcLockInfo = &(SrcLock->Info);

	if ( DstLock->DriverHandle ) 
		DstLockInfo = &(DstLock->DriverInfo);
	else
		DstLockInfo = &(DstLock->Info);

	if ( ! (SrcBits = (uint8*)jeBitmap_GetBits(SrcLock)) || 
		 ! (DstBits = (uint8*)jeBitmap_GetBits(DstLock)) )
	{
		jeErrorLog_AddString(-1,"BlitMipRect : GetBits", NULL);
		goto fail;
	}

	if ( SizeX < 0 )
		SizeX = min(SrcLockInfo->Width,DstLockInfo->Width);
	if ( SizeY < 0 )
		SizeY = min(SrcLockInfo->Height,DstLockInfo->Height);

	if (( (SrcX + SizeX) > SrcLockInfo->Width ) ||
		( (SrcY + SizeY) > SrcLockInfo->Height) ||
		( (DstX + SizeX) > DstLockInfo->Width ) ||
		( (DstY + SizeY) > DstLockInfo->Height))
	{
		jeErrorLog_AddString(-1,"BlitMipRect : dimensions bad", NULL);
		goto fail;
	}

	SrcBits += jePixelFormat_BytesPerPel(SrcLockInfo->Format) * ( SrcY * SrcLockInfo->Stride + SrcX );
	DstBits += jePixelFormat_BytesPerPel(DstLockInfo->Format) * ( DstY * DstLockInfo->Stride + DstX );

	// _BlitMipRect : made palette
	if ( ! jeBitmap_BlitData(	SrcLockInfo,SrcBits,SrcLock,
								DstLockInfo,DstBits,DstLock,
								SizeX,SizeY) )
	{
		goto fail;
	}

	if ( SrcUnLock ) jeBitmap_UnLock(SrcLock);
	if ( DstUnLock ) jeBitmap_UnLock(DstLock);

	return JE_TRUE;

	fail:

	if ( SrcUnLock ) jeBitmap_UnLock(SrcLock);
	if ( DstUnLock ) jeBitmap_UnLock(DstLock);

	return JE_FALSE;
}

JETAPI jeBoolean 	JETCC	jeBitmap_SetFormatMin(jeBitmap *Bmp,jePixelFormat NewFormat)
{
jeBitmap_Palette * Pal;

	assert(jeBitmap_IsValid(Bmp));

	Pal = jeBitmap_GetPalette(Bmp);
	if ( Bmp->Info.HasColorKey )
	{
	uint32 CK;
		if ( jePixelFormat_IsRaw(NewFormat) )
		{
			if ( jePixelFormat_IsRaw(Bmp->Info.Format) )
			{
				CK = jePixelFormat_ConvertPixel(Bmp->Info.Format,Bmp->Info.ColorKey,NewFormat);
			}
			else if ( jePixelFormat_HasPalette(Bmp->Info.Format) )
			{
				assert(Pal);
				jeBitmap_Palette_GetEntry(Pal,Bmp->Info.ColorKey,&CK);
				CK = jePixelFormat_ConvertPixel(Pal->Format,CK,NewFormat);
				if ( ! CK ) CK = 1;
			}
		}
		else
		{
			if ( jePixelFormat_HasPalette(NewFormat) )
			{
				CK = 255;
			}
			else
			{
				CK = 1;
			}
		}
		
		return jeBitmap_SetFormat(Bmp,NewFormat,JE_TRUE,CK,Pal);
	}
	else
	{
		return jeBitmap_SetFormat(Bmp,NewFormat,JE_FALSE,0,Pal);
	}
}

JETAPI jeBoolean JETCC jeBitmap_SetCompressionOptions(jeBitmap * Bmp,int32 clevel,jeBoolean NeedMips,jeFloat ratio)
{
	assert(jeBitmap_IsValid(Bmp));

	Bmp->HasWaveletOptions = jeWavelet_SetOptions(&(Bmp->WaveletOptions),clevel,NeedMips,ratio);

return Bmp->HasWaveletOptions;
}

JETAPI jeBoolean JETCC jeBitmap_SetCompressionOptionsExpert(jeBitmap * Bmp,jeFloat Ratio,int32 TransformN,int32 CoderN,jeBoolean TransposeLHs,jeBoolean Block)
{
	assert(jeBitmap_IsValid(Bmp));

	Bmp->HasWaveletOptions = jeWavelet_SetExpertOptions(&(Bmp->WaveletOptions),Ratio,TransformN,CoderN,TransposeLHs,Block);

return Bmp->HasWaveletOptions;
}

JETAPI jeBoolean JETCC jeBitmap_SetFormat(jeBitmap *Bmp, 
							jePixelFormat NewFormat, 
							jeBoolean HasColorKey, uint32 ColorKey,
							const jeBitmap_Palette *Palette )
{
	assert( jeBitmap_IsValid(Bmp) );
	jeBitmap_WaitReady(Bmp);

	if ( Bmp->LockOwner || Bmp->LockCount || Bmp->DataOwner )
	{
		jeErrorLog_AddString(-1,"SetFormat : not an original bitmap", NULL);
		return JE_FALSE;	
	}
	// can't do _SetFormat on a locked mip, cuz it would change the size of all the locked mips = no good

	// always affects the non-Driver copy

	if ( NewFormat == JE_PIXELFORMAT_WAVELET )
	{
		jeBitmap_ClearMips(Bmp);

		if ( Bmp->Wavelet )
		{
			assert(Bmp->Info.Format == JE_PIXELFORMAT_WAVELET);
			return JE_TRUE;
		}
			
		if ( Bmp->Info.HasColorKey )
			Bmp->Info.HasColorKey = jeBitmap_UsesColorKey(Bmp);

		if ( Bmp->HasWaveletOptions )
			Bmp->Wavelet = jeWavelet_CreateFromBitmap(Bmp,&(Bmp->WaveletOptions));
		else
			Bmp->Wavelet = jeWavelet_CreateFromBitmap(Bmp,NULL);
			
		if ( ! Bmp->Wavelet )
			return JE_FALSE;

		Bmp->Info.Format = JE_PIXELFORMAT_WAVELET;

		if ( Bmp->Data[0] )
		{
			jeRam_Free(Bmp->Data[0]);
			Bmp->Data[0] = NULL;
		}
		
		if ( Bmp->Alpha )
		{
			jeBitmap_Destroy(&(Bmp->Alpha));
			Bmp->Alpha = NULL;
		}

		return JE_TRUE;
	}

	if ( NewFormat == Bmp->Info.Format )
	{
		// but not wavelet

		if ( jePixelFormat_HasPalette(NewFormat) && Palette )
		{
			if ( ! jeBitmap_SetPalette(Bmp,(jeBitmap_Palette *)Palette) )
				return JE_FALSE;
		}

		if ( (! HasColorKey )
			|| ( HasColorKey && Bmp->Info.HasColorKey && ColorKey == Bmp->Info.ColorKey ) )
		{
			Bmp->Info.HasColorKey = HasColorKey;
			Bmp->Info.ColorKey = ColorKey;
			return JE_TRUE;
		}
		else
		{
		jeBitmap_Info OldInfo;

			OldInfo = Bmp->Info;

			assert(HasColorKey);

			// just change the colorkey

			Bmp->Info.HasColorKey = HasColorKey;
			Bmp->Info.ColorKey = ColorKey;

			if ( Bmp->Data[Bmp->Info.MinimumMip] == NULL )
				return JE_TRUE;
		
			assert(Bmp->Info.MinimumMip == 0); //{} this is just out of laziness

			// _SetFormat : same format
			if ( ! jeBitmap_BlitData(	&OldInfo,		Bmp->Data[Bmp->Info.MinimumMip], NULL,
										&(Bmp->Info),	Bmp->Data[Bmp->Info.MinimumMip], NULL,
										Bmp->Info.Width, Bmp->Info.Height) )
			{
				return JE_FALSE;
			}

			return JE_TRUE;
		}
	}
	else
	{
	jeBitmap_Info OldInfo;
	int OldBPP,NewBPP;
	int OldMaxMips;
	DRV_Driver * Driver;

		if ( jePixelFormat_HasPalette(NewFormat) )
		{
			if ( Palette )
			{
				if ( ! jeBitmap_SetPalette(Bmp,(jeBitmap_Palette *)Palette) )
					return JE_FALSE;
			}
			else
			{
				if ( ! jeBitmap_GetPalette(Bmp) && ! jePixelFormat_HasPalette(Bmp->Info.Format) )
				{
				jeBitmap_Palette *NewPal;
					NewPal = jeBitmap_Palette_CreateFromBitmap(Bmp,JE_FALSE);
					if ( ! NewPal )
					{
						jeErrorLog_AddString(-1,"_SetFormat : createPaletteFromBitmap failed", NULL);
						return JE_FALSE;
					}
					if ( ! jeBitmap_SetPalette(Bmp,NewPal) )
						return JE_FALSE;
					jeBitmap_Palette_Destroy(&NewPal);
				}
			}
		}

		Driver = Bmp->Driver;
		if ( Driver )
			if ( ! jeBitmap_DetachDriver(Bmp,JE_TRUE) )
				return JE_FALSE;

		OldBPP = jePixelFormat_BytesPerPel(Bmp->Info.Format);
		NewBPP = jePixelFormat_BytesPerPel(NewFormat);

		OldInfo = Bmp->Info;
		Bmp->Info.Format = NewFormat;
		Bmp->Info.HasColorKey = HasColorKey;
		Bmp->Info.ColorKey = ColorKey;

		// {} this is not very polite; we do restore them later, though...
		OldMaxMips = max(Bmp->Info.MaximumMip,Bmp->DriverInfo.MaximumMip);
		jeBitmap_ClearMips(Bmp);		

		if ( ! Bmp->Wavelet && Bmp->Data[Bmp->Info.MinimumMip] == NULL && 
				Bmp->DriverHandle == NULL )
			return JE_TRUE;

		if ( OldBPP == NewBPP )
		{
		jeBitmap * Lock;
		void * Bits;
			// can work in place
			if ( ! jeBitmap_LockForWrite(Bmp,&Lock,0,0) )
				return JE_FALSE;

			if ( ! (Bits = jeBitmap_GetBits(Lock)) )
			{
				jeBitmap_UnLock(Lock);
				return JE_FALSE;
			}

			// _SetFormat : new format
			if ( ! jeBitmap_BlitData(	&OldInfo,		Bits, Lock,
										&(Lock->Info),	Bits, Lock,
										Lock->Info.Width, Lock->Info.Height) )
			{
				jeBitmap_UnLock(Lock);
				return JE_FALSE;
			}

			jeBitmap_UnLock(Lock);
		}
		else // NewFormat is raw && != OldFormat
		{
		jeBitmap OldBmp;
		jeBitmap *Lock,*SrcLock;
		void *Bits,*OldBits;

			OldBmp = *Bmp;
			OldBmp.Info = OldInfo;

			// clear out the Bmp for putting the new format in
			Bmp->Info.Stride = Bmp->Info.Width;
			Bmp->Data[0] = NULL;
			Bmp->Alpha = NULL;
			Bmp->Wavelet = NULL;
			Bmp->WaveletMipLock = 0;

			if ( ! jeBitmap_AllocSystemMip(Bmp,0) )
				return JE_FALSE;

			if ( ! jeBitmap_LockForReadNative(&OldBmp,&SrcLock,0,0) )
				return JE_FALSE;

			if ( ! jeBitmap_LockForWrite(Bmp,&Lock,0,0) )
				return JE_FALSE;

			if ( ! (Bits = jeBitmap_GetBits(Lock)) )
			{
				jeBitmap_UnLock(Lock);
				return JE_FALSE;
			}
			if ( ! (OldBits = jeBitmap_GetBits(SrcLock)) )
			{
				jeBitmap_UnLock(Lock);
				return JE_FALSE;
			}

			// _SetFormat : new format
			if ( ! jeBitmap_BlitData(	&OldInfo,		OldBits,		SrcLock,
										&(Lock->Info),	Bits,			Lock,
										Lock->Info.Width, Lock->Info.Height) )
			{
				// try to undo as well as possible
				return JE_FALSE;
			}
		
			jeBitmap_UnLock(Lock);
			jeBitmap_UnLock(SrcLock);

			if ( OldBmp.Data[0] )
			{
				jeRam_Free(OldBmp.Data[0]);
				OldBmp.Data[0] = NULL;
			}
			// ok, now delete wavelet
			if ( OldBmp.Wavelet )
			{
				jeWavelet_Destroy(&(OldBmp.Wavelet));
			}

			if ( jePixelFormat_HasGoodAlpha(NewFormat) )
			{
				jeBitmap_Destroy(&(OldBmp.Alpha));
			}
			else
			{
				Bmp->Alpha = OldBmp.Alpha;
			}
		}

		{
		int32 mip;
			mip = Bmp->Info.MinimumMip;
			while( mip < OldMaxMips )
			{
				jeBitmap_UpdateMips(Bmp,mip,mip+1);
				mip++;
			}
		}

		if ( Driver )
		{		
			if ( ! jeBitmap_AttachToDriver(Bmp,Driver,0) )
				return JE_FALSE;
		}
	}

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_SetColorKey(jeBitmap *Bmp, jeBoolean HasColorKey, uint32 ColorKey , jeBoolean Smart)
{
	assert( jeBitmap_IsValid(Bmp) );

	if ( Bmp->LockOwner || Bmp->LockCount || Bmp->DataOwner )
	{
		jeErrorLog_AddString(-1,"SetColorKey : not an original bitmap", NULL);
		return JE_FALSE;	
	}

	// see comments in SetFormat

	if ( Bmp->DriverHandle )
		jeBitmap_Update_DriverToSystem(Bmp);

	if ( HasColorKey && 
			((uint32)ColorKey>>1) >= ((uint32)1<<(jePixelFormat_BytesPerPel(Bmp->Info.Format)*8 - 1)) )
	{
		jeErrorLog_AddString(-1,"jeBitmap_SetColorKey : invalid ColorKey pixel!", NULL);
		return JE_FALSE;
	}
	if ( HasColorKey && jePixelFormat_HasAlpha(Bmp->Info.Format) )
	{
		jeErrorLog_AddString(-1,"jeBitmap_SetColorKey : non-fatal : Alpha and ColorKey together won't work right", NULL);
	}

	if ( HasColorKey && Smart && Bmp->Data[0] )
	{
		Bmp->Info.HasColorKey = JE_TRUE;
		Bmp->Info.ColorKey = ColorKey;
		if ( ! jeBitmap_UsesColorKey(Bmp) )
		{
			Bmp->Info.HasColorKey = JE_FALSE;
			Bmp->Info.ColorKey = 1;
		}
	}
	else
	{
		Bmp->Info.HasColorKey = HasColorKey;
		Bmp->Info.ColorKey = ColorKey;
	}

	if ( Bmp->DriverHandle )
		jeBitmap_Update_SystemToDriver(Bmp);

return JE_TRUE;
}

jeBoolean jeBitmap_UsesColorKey(const jeBitmap * Bmp)
{
void * Bits;
const jePixelFormat_Operations * ops;
int32 x,y,w,h,s;
uint32 pel,ColorKey;

	jeBitmap_WaitReady(Bmp);

	if ( ! Bmp->Info.HasColorKey )
		return JE_FALSE;

	if ( ! Bmp->Data[0] )
	{
		jeErrorLog_AddString(-1,"UsesColorKey : no data!", NULL);
		return JE_TRUE;
	}

	assert( Bmp->Info.MinimumMip == 0 );

	Bits = Bmp->Data[0];
	ops = jePixelFormat_GetOperations(Bmp->Info.Format);
	assert(ops);

	w = Bmp->Info.Width;
	h = Bmp->Info.Height;
	s = Bmp->Info.Stride;

	ColorKey = Bmp->Info.ColorKey;

	switch(ops->BytesPerPel)
	{
		case 0:
			jeErrorLog_AddString(-1,"UsesColorKey : invalid format", NULL);
			return JE_TRUE;
		case 3:
			#pragma message("Bitmap : UsesColorKey : no 24bit Smart ColorKey")
			jeErrorLog_AddString(-1,"UsesColorKey : no 24bit Smart ColorKey", NULL);
			return JE_TRUE;	
		case 1:
		{
		uint8 * ptr;
			ptr = (uint8*)Bits;
			for(y=h;y--;)
			{
				for(x=w;x--;)
				{
					pel = *ptr++;
					if ( pel == ColorKey )
					{
						Log_Printf("UsesColorKey : Yes\n");
						return JE_TRUE;	
					}
				}
				ptr += (s-w);
			}
			break;
		}
		case 2:
		{
		uint16 * ptr;
			ptr = (uint16*)Bits;
			for(y=h;y--;)
			{
				for(x=w;x--;)
				{
					pel = *ptr++;
					if ( pel == ColorKey )
					{
						Log_Printf("UsesColorKey : Yes\n");
						return JE_TRUE;	
					}
				}
				ptr += (s-w);
			}
			break;
		}
		case 4:
		{
		uint32 * ptr;
			ptr = (uint32*)Bits;
			for(y=h;y--;)
			{
				for(x=w;x--;)
				{
					pel = *ptr++;
					if ( pel == ColorKey )
					{
						Log_Printf("UsesColorKey : Yes\n");
						return JE_TRUE;	
					}
				}
				ptr += (s-w);
			}
			break;
		}
	}
return JE_FALSE;
}


JETAPI jeBoolean JETCC jeBitmap_SetPalette(jeBitmap *Bmp, const jeBitmap_Palette *Palette)
{
	assert(Bmp); // not nec. valid
	assert( jeBitmap_Palette_IsValid(Palette) );
	jeBitmap_WaitReady(Bmp);

	if ( Bmp->LockOwner )
		Bmp = Bmp->LockOwner;

/* //{} breaks PalCreate
	if ( Bmp->LockCount > 0 || Bmp->DataOwner )
	{
		jeErrorLog_AddString(-1,"SetPalette : not an original bitmap", NULL);
		return JE_FALSE;
	}
*/

	// warning : Bitmap_Blitdata calls us when it auto-creates a palette!

	// note that when we _SetPalette on a bitmap, all its write-locked children
	//	also get new palettes

	if ( Bmp->Info.Palette != Palette )
	{
		// save the palette even if we're not palettized, for later use
		if ( Palette->Driver )
		{
			if ( ! jeBitmap_AllocPalette(Bmp,Palette->Format,NULL) )
				return JE_FALSE;
			
			if ( ! jeBitmap_Palette_Copy(Palette,Bmp->Info.Palette) )
				return JE_FALSE;
		}
		else
		{
			if ( Bmp->Info.Palette )
				jeBitmap_Palette_Destroy(&(Bmp->Info.Palette));

			Bmp->Info.Palette = (jeBitmap_Palette *)Palette;
			jeBitmap_Palette_CreateRef(Bmp->Info.Palette);
		}
	}

	if ( jePixelFormat_HasPalette(Bmp->DriverInfo.Format) &&
		Bmp->DriverInfo.Palette != Palette )
	{
		if ( Palette->Driver == Bmp->Driver && 
			( ! Palette->HasColorKey || ! Bmp->DriverInfo.ColorKey ||
				(uint32)Palette->ColorKeyIndex == Bmp->DriverInfo.ColorKey ) )
		{
			if ( Bmp->DriverInfo.Palette )
				jeBitmap_Palette_Destroy(&(Bmp->DriverInfo.Palette));
			Bmp->DriverInfo.Palette = (jeBitmap_Palette *)Palette;
			jeBitmap_Palette_CreateRef(Bmp->DriverInfo.Palette);
		}
		else if ( Bmp->DriverInfo.Palette )
		{
			if ( ! jeBitmap_Palette_Copy(Palette,Bmp->DriverInfo.Palette) )
				return JE_FALSE;
		}
		else
		{
			// IS JE_PIXELFORMAT_NO_DATA a safe replacement for 0 here?
			if ( ! jeBitmap_AllocPalette(Bmp,JE_PIXELFORMAT_NO_DATA,Bmp->Driver) )
				return JE_FALSE;

			if ( ! jeBitmap_Palette_Copy(Palette,Bmp->DriverInfo.Palette) )
				return JE_FALSE;
		}
	}

	if ( Bmp->DriverHandle )
	{
		// if one has pal and other doesn't this is real change!
		if (	jePixelFormat_HasPalette(Bmp->Info.Format) &&
			  ! jePixelFormat_HasPalette(Bmp->DriverInfo.Format) )
		{
			// this over-rides any driver changes!
			Bmp->DriverDataChanged = JE_FALSE;
			if ( ! jeBitmap_Update_SystemToDriver(Bmp) )
				return JE_FALSE;
		}
		else if ( ! jePixelFormat_HasPalette(Bmp->Info.Format) &&
				jePixelFormat_HasPalette(Bmp->DriverInfo.Format) )
		{
			Bmp->DriverDataChanged = JE_TRUE;
		}
	}

	assert( jeBitmap_IsValid(Bmp) );

return JE_TRUE;
}

JETAPI jeBitmap_Palette * JETCC jeBitmap_GetPalette(const jeBitmap *Bmp)
{
	if ( ! Bmp ) return NULL;

	if ( Bmp->Driver && Bmp->DriverInfo.Palette )
	{
		assert(Bmp->Info.Palette);
		return Bmp->DriverInfo.Palette;
	}

	return Bmp->Info.Palette;
}


JETAPI jeBitmap * JETCC jeBitmap_GetAlpha(const jeBitmap *Bmp)
{
	if ( ! Bmp ) return NULL;
	return Bmp->Alpha;
}

JETAPI jeBoolean JETCC jeBitmap_SetAlpha(jeBitmap *Bmp, const jeBitmap *AlphaBmp)
{
	assert( jeBitmap_IsValid(Bmp) );
	
	if ( Bmp->LockOwner )
		Bmp = Bmp->LockOwner;
	if ( Bmp->LockCount > 0 || Bmp->DataOwner )
	{
		jeErrorLog_AddString(-1,"SetAlpha : not an original bitmap", NULL);
		return JE_FALSE;
	}

	if ( AlphaBmp == Bmp->Alpha )
		return JE_TRUE;

	if ( Bmp->DriverHandle )
	{
		jeBitmap_Update_DriverToSystem(Bmp);
	}

	if ( Bmp->Alpha )
	{
		jeBitmap_Destroy(&(Bmp->Alpha));
	}

	Bmp->Alpha = (jeBitmap *)AlphaBmp;
	if ( AlphaBmp )
	{
		assert( jeBitmap_IsValid(AlphaBmp) );
		jeBitmap_CreateRef(Bmp->Alpha);
	}

	if ( Bmp->DriverHandle )
	{
		// upload the new alpha to the driver bitmap
		jeBitmap_Update_SystemToDriver(Bmp);
	}

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_SetPreferredFormat(jeBitmap *Bmp,jePixelFormat Format)
{

	if ( Bmp->LockOwner )
		Bmp = Bmp->LockOwner;
	if ( Bmp->LockCount > 0 || Bmp->DataOwner )
	{
		jeErrorLog_AddString(-1,"SetPrefferedFormat : not an original bitmap", NULL);
		return JE_FALSE;
	}

	if ( Bmp->PreferredFormat != Format )
	{
	DRV_Driver * Driver;
		Bmp->PreferredFormat = Format;
		Driver = Bmp->Driver;
		if ( Driver )
		{
			if ( ! jeBitmap_DetachDriver(Bmp,JE_TRUE) )
				return JE_FALSE;
			if ( ! jeBitmap_AttachToDriver(Bmp,Driver,0) )
				return JE_FALSE;
		}
	}

return JE_TRUE;
}

JETAPI jePixelFormat JETCC jeBitmap_GetPreferredFormat(const jeBitmap *Bmp)
{
	if ( ! Bmp ) return JE_PIXELFORMAT_NO_DATA;
return Bmp->PreferredFormat;
}

/*}{ ************** FILE I/O ************************/


JETAPI jeBoolean  JETCC jeBitmap_GetPersistableName(const jeBitmap *Bmp, jeVFile ** pBaseFS, char ** pName)
{
	if ( Bmp->Persistable )
	{
		*pBaseFS = (jeVFile *)Bmp->PersistBaseFS;
		*pName = (char *)Bmp->PersistName;
		return JE_TRUE;
	}
	else
	{
		*pBaseFS = NULL;
		*pName = (char *)Bmp->PersistName;
		return JE_FALSE;
	}
}

JETAPI jeBitmap * JETCC jeBitmap_CreateFromFileName(const jeVFile *BaseFS,const char *Name)
{
	jeVFile * File;
	jeBitmap * Bitmap;

	if ( BaseFS )
	{
		File = jeVFile_Open((jeVFile *)BaseFS, Name, JE_VFILE_OPEN_READONLY);
	}
	else
	{
		if ( strnicmp(Name,"http:",5) == 0 || strnicmp(Name,"ftp:",4) == 0 || strnicmp(Name,"www.",4) == 0 )
		{
		jeVFile * inet;
			inet = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_INTERNET,NULL,NULL,JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY);
			assert(inet);
			File = jeVFile_Open(inet,Name,JE_VFILE_OPEN_READONLY);
			jeVFile_Close(inet);
		}
		else
		{
			File = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,Name,NULL,JE_VFILE_OPEN_READONLY);
		}
	}
	if ( ! File )
		return NULL;

	Bitmap = jeBitmap_CreateFromFile(File);
	jeVFile_Close(File);

	if ( ! Bitmap->Persistable )
	{
		Bitmap->Persistable = JE_TRUE;
		Bitmap->PersistBaseFS = BaseFS;
		strcpy(Bitmap->PersistName,Name);
	}

	return Bitmap;
}

JETAPI jeBoolean JETCC jeBitmap_WriteToFileName(const jeBitmap * Bmp,const jeVFile *BaseFS,const char *Name)
{
	jeVFile * File;
	jeBoolean Ret;

	if ( BaseFS )
	{
		File = jeVFile_Open((jeVFile *)BaseFS, Name, JE_VFILE_OPEN_CREATE);
	}
	else
	{
		File = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,Name,NULL,JE_VFILE_OPEN_CREATE);
	}

	if ( ! File )
		return JE_FALSE;

	Ret = jeBitmap_WriteToFile(Bmp,File);

	jeVFile_Close(File);

	if ( ! Bmp->Persistable )
	{
		((jeBitmap *)Bmp)->Persistable = JE_TRUE;
		((jeBitmap *)Bmp)->PersistBaseFS = BaseFS;
		strcpy(((jeBitmap *)Bmp)->PersistName,Name);
	}

	return Ret;
}

JETAPI jeBitmap * JETCC jeBitmap_CreateFromFileName2(const jeVFile *BaseFS,const char *Name,jePtrMgr *PtrMgr)
{
	jeVFile * File;
	jeBitmap * Bitmap;

	if ( BaseFS )
	{
		File = jeVFile_Open((jeVFile *)BaseFS, Name, JE_VFILE_OPEN_READONLY);
	}
	else
	{
		if ( strnicmp(Name,"http:",5) == 0 || strnicmp(Name,"ftp:",4) == 0 || strnicmp(Name,"www.",4) == 0 )
		{
		jeVFile * inet;
			inet = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_INTERNET,NULL,NULL,JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY);
			assert(inet);
			File = jeVFile_Open(inet,Name,JE_VFILE_OPEN_READONLY);
			jeVFile_Close(inet);
		}
		else
		{
			File = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,Name,NULL,JE_VFILE_OPEN_READONLY);
		}
	}
	if ( ! File )
		return NULL;
	Bitmap = jeBitmap_CreateFromFile2(File,(jeVFile *)BaseFS,PtrMgr);
	jeVFile_Close(File);

	if ( ! Bitmap->Persistable )
	{
		Bitmap->Persistable = JE_TRUE;
		Bitmap->PersistBaseFS = BaseFS;
		strcpy(Bitmap->PersistName,Name);
	}

	return Bitmap;
}

JETAPI jeBoolean JETCC jeBitmap_WriteToFileName2(const jeBitmap * Bmp,const jeVFile *BaseFS,const char *Name,jePtrMgr *PtrMgr)
{
	jeVFile * File;
	jeBoolean Ret;

	if ( BaseFS )
	{
		File = jeVFile_Open((jeVFile *)BaseFS, Name, JE_VFILE_OPEN_CREATE);
	}
	else
	{
		File = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,Name,NULL,JE_VFILE_OPEN_CREATE);
	}

	if ( ! File )
		return JE_FALSE;

	Ret = jeBitmap_WriteToFile2(Bmp,File,PtrMgr);

	jeVFile_Close(File);

	if ( ! Bmp->Persistable )
	{
		((jeBitmap *)Bmp)->Persistable = JE_TRUE;
		((jeBitmap *)Bmp)->PersistBaseFS = BaseFS;
		strcpy(((jeBitmap *)Bmp)->PersistName,Name);
	}

	return Ret;
}

JETAPI jeBitmap * JETCC jeBitmap_CreateFromFile2(jeVFile *VFile,jeVFile *ResourceBaseFS,jePtrMgr *PtrMgr)
{
	jeBitmap * Bmp;
	uint8 NameStrLen;

	if ( PtrMgr )
	{
		if (!jePtrMgr_ReadPtr(PtrMgr, VFile, (void **)&Bmp))
			return NULL;

		if ( Bmp )
		{
			jeBitmap_CreateRef(Bmp);
			return Bmp;
		}
	}
	
	jeVFile_Read(VFile,&NameStrLen,1);

	if ( NameStrLen > 0 )
	{
	char Name[1024];
		jeVFile_Read(VFile,Name,NameStrLen);
		Name[NameStrLen] = 0;

		Bmp = jeBitmap_CreateFromFileName(ResourceBaseFS,Name);
	}
	else
	{
		Bmp = jeBitmap_CreateFromFile(VFile);
	}

	if ( ! Bmp )
		return NULL;

	if ( PtrMgr )
		jePtrMgr_PushPtr(PtrMgr,Bmp);

return Bmp;
}

JETAPI jeBoolean JETCC jeBitmap_WriteToFile2(const jeBitmap *Bmp,jeVFile *VFile,jePtrMgr *PtrMgr)
{
uint8 NameStrLen;

	if ( PtrMgr )
	{
	uint32 Count;

		if (!jePtrMgr_WritePtr(PtrMgr, VFile, (void *)Bmp, &Count))
			return JE_FALSE;

		if (Count)		// Already loaded
			return JE_TRUE;
	}
	
	if ( ! Bmp->Persistable )	NameStrLen = 0;
	else						NameStrLen = strlen(Bmp->PersistName);

	jeVFile_Write(VFile,&NameStrLen,1);

	if ( NameStrLen > 0 )
	{
		jeVFile_Write(VFile,Bmp->PersistName,NameStrLen);
	}
	else
	{
		jeBitmap_WriteToFile(Bmp,VFile);
	}

	if ( PtrMgr )
		jePtrMgr_PushPtr(PtrMgr,(void *)Bmp);

return JE_TRUE;
}

// GeBm Tag in 4 bytes {}
typedef uint32			jeBmTag_t;
#define JEBM_TAG		((jeBmTag_t)0x6D426547)	// "GeBm"

// version in a byte
#define JEBM_VERSION			(((uint32)JEBM_VERSION_MAJOR<<4) + (uint32)JEBM_VERSION_MINOR)
#define VERSION_MAJOR(Version)	(((Version)>>4)&0x0F)
#define VERSION_MINOR(Version)	((Version)&0x0F)

#define MIP_MASK				(0xF)
#define MIP_FLAG_COMPRESSED		(1<<4)
#define MIP_FLAG_PAETH_FILTERED	(1<<5)

static jeBoolean jeBitmap_ReadFromBMP(jeBitmap * Bmp,jeVFile * F);

JETAPI jeBitmap * JETCC jeBitmap_CreateFromFile(jeVFile *F)
{
jeBitmap *	Bmp;
jeBmTag_t Tag;
jeVFile * HF;

	assert(F);

	//{} we don't thread these reads, because we need to block on them!
	// the bitmap is not valid until these reads finish, and when we
	// return, we gaurantee a valid bitmap.

	Bmp = jeBitmap_Create_Base();
	if ( ! Bmp )
		return NULL;

	Tag = 0;
	if ( (HF = jeVFile_GetHintsFile(F)) != NULL )
	{
		if ( jeVFile_Read(HF, &Tag, sizeof(Tag)) )
		{
			if ( Tag != JEBM_TAG )
			{
				jeVFile_Seek(HF, - (int)sizeof(Tag), JE_VFILE_SEEKSET);
			}
		}
	}

	Bmp->StreamingStatus = JE_BITMAP_STREAMING_NOT;
		// we'll set it to CHANGED later

	if ( Tag == JEBM_TAG )
	{
	uint8 flags;
	uint8 Version;
	int32 mip;
	
		// see WriteToFile for comments on the file format
		assert( HF );

		if ( ! jeVFile_Read(HF, &Version, sizeof(Version)) )
			goto fail;

		if ( VERSION_MAJOR(Version) != VERSION_MAJOR(JEBM_VERSION) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : incompatible GeBm version", NULL);	
			goto fail;
		}

		if ( ! jeBitmap_ReadInfo(Bmp,HF) )
			goto fail;

		if ( Bmp->Info.Palette )
		{
			Bmp->Info.Palette = NULL;
			if ( Version <= (4<<4) )
			{
				if ( ! ( Bmp->Info.Palette = jeBitmap_Palette_CreateFromFile(F)) )
					goto fail;
			}
			else
			{
				if ( ! ( Bmp->Info.Palette = jeBitmap_Palette_CreateFromFile(HF)) )
					goto fail;
			}
		}

		if ( Bmp->Info.Format == JE_PIXELFORMAT_WAVELET )
		{

			Bmp->Wavelet = jeWavelet_CreateFromFile(Bmp,F);

			if ( ! Bmp->Wavelet )
			{
				jeErrorLog_AddString(-1,"jeWavelet_CreateFromFile failed!",NULL);
				goto fail;
			}

			if ( jeWavelet_StreamingJob(Bmp->Wavelet) )
				Bmp->StreamingStatus = JE_BITMAP_STREAMING_STARTED;
			// else already set to STREAMING_NOT
		}
		else
		{
			for(;;)
			{
				if ( ! jeVFile_Read(HF, &flags, sizeof(flags)) )
					goto fail;

				mip = flags & MIP_MASK;

				if ( mip > Bmp->Info.MaximumMip )
					break;

				assert(mip >= Bmp->Info.MinimumMip );
				assert( Bmp->Info.Stride == Bmp->Info.Width );

				if ( ! jeBitmap_AllocSystemMip(Bmp,mip) )
					goto fail;

				if ( flags & MIP_FLAG_COMPRESSED )
				{
				jeVFile * LzF;

					LzF = jeVFile_OpenNewSystem(F,JE_VFILE_TYPE_LZ,NULL,NULL,JE_VFILE_OPEN_READONLY);
					if ( ! LzF )
					{
						jeErrorLog_AddString(-1,"Bitmap_CreateFromFile : LZ File Open failed",NULL);
						return NULL;
					}

					if ( ! jeVFile_Read(LzF, Bmp->Data[mip], jeBitmap_MipBytes(Bmp,mip) ) )
					{
						jeVFile_Close(LzF);
						jeErrorLog_AddString(-1,"Bitmap_CreateFromFile : LZ File Read failed",NULL);
						return NULL;
					}

					if ( ! jeVFile_Close(LzF) )
					{
						jeErrorLog_AddString(-1,"Bitmap_CreateFromFile : LZ File Close failed",NULL);
						return NULL;
					}
				}
				else
				{
					if ( ! jeVFile_Read(F, Bmp->Data[mip], jeBitmap_MipBytes(Bmp,mip) ) )
						goto fail;
				}

				if ( flags & MIP_FLAG_PAETH_FILTERED )
				{
					jeErrorLog_AddString(-1,"Bitmap_CreateFromFile : Paeth Filter not supported in this version!",NULL);
					return NULL;
				}

				Bmp->Modified[mip] = JE_TRUE;
			}
		}

		if( Bmp->Alpha )
		{
			if ( ! (Bmp->Alpha = jeBitmap_CreateFromFile(F)) )
				goto fail;
		}
	}	// end jeBitmap reader
	else 
	{
		if ( ! jeVFile_Read(F, &Tag, sizeof(Tag)) )
			goto fail;

		if ( ! jeVFile_Seek(F, - (int)sizeof(Tag), JE_VFILE_SEEKCUR) )
			goto fail;

		if ( (Tag&0xFFFF) == 0x4D42 )	// 'BM'
		{
		
			if ( ! jeBitmap_ReadFromBMP(Bmp,F) )
				goto fail;
		}
		else
		{
			// jeErrorLog_AddString(-1,"CreateFromFile : unknown format", NULL);
			goto fail;
		}
	}

	if ( ! Bmp->Persistable )
	{
		Bmp->Persistable = JE_TRUE;
		Bmp->PersistBaseFS = NULL;
		jeVFile_GetName(F,Bmp->PersistName,sizeof(Bmp->PersistName));
	}

	return Bmp;

fail:
	assert(Bmp);

	jeBitmap_Destroy(&Bmp);
	return NULL;
}

JETAPI jeBoolean JETCC jeBitmap_WriteToFile(const jeBitmap *Bmp, jeVFile *F)
{
jeBmTag_t jeBM_Tag;
uint8  jeBM_Version;
uint8 flags;
int32 mip;
jeVFile * HF;
	
	assert(Bmp && F);
	assert( jeBitmap_IsValid(Bmp) );
	jeBitmap_WaitReady(Bmp);

	jeBM_Tag = JEBM_TAG;
	jeBM_Version = JEBM_VERSION;

	if ( Bmp->DriverHandle )
	{
		if ( ! jeBitmap_Update_DriverToSystem((jeBitmap *)Bmp) )
		{
			jeErrorLog_AddString(-1,"WriteToFile : Update_DriverToSystem", NULL);	
			return JE_FALSE;
		}
	}

	HF = jeVFile_GetHintsFile(F);
	if ( ! HF )
		return JE_FALSE;

	if ( ! jeVFile_Write(HF, &jeBM_Tag, sizeof(jeBM_Tag)) )
		return JE_FALSE;

	if ( ! jeVFile_Write(HF, &jeBM_Version, sizeof(jeBM_Version)) )
		return JE_FALSE;

	if ( ! jeBitmap_WriteInfo(Bmp,HF) )
		return JE_FALSE;

	#ifdef COUNT_HEADER_SIZES
		Header_Sizes += 15;
	#endif

	// the pointer Bmp->Info.Palette serves as boolean : HasPalette
	if ( Bmp->Info.Palette )
	{
		if ( ! jeBitmap_Palette_WriteToFile(Bmp->Info.Palette,HF) )
			return JE_FALSE;
	}

	if ( Bmp->Info.Format == JE_PIXELFORMAT_WAVELET )
	{
		if ( ! jeWavelet_WriteToFile(Bmp->Wavelet,F) )
			return JE_FALSE;
	}
	else
	{
		for( mip = Bmp->Info.MinimumMip; mip <= Bmp->Info.MaximumMip; mip++ )
		{

			// write out all the interesting mips :
			//	the first one, and then mips which are not just
			//	sub-samples of the first (eg. that have been user-set)

			if ( (mip == Bmp->Info.MinimumMip || Bmp->Modified[mip]) && Bmp->Data[mip] )
			{
			uint8 * MipData;
			jeBoolean MipDataAlloced;
			uint32 MipDataLen;
			jeVFile * LzF;

				MipDataLen = SHIFT_R_ROUNDUP(Bmp->Info.Width,mip) * SHIFT_R_ROUNDUP(Bmp->Info.Height,mip) *
								jePixelFormat_BytesPerPel(Bmp->Info.Format);

				if ( Bmp->Info.Stride == Bmp->Info.Width )
				{
					MipData = (uint8*)Bmp->Data[mip];
					MipDataAlloced = JE_FALSE;
				}
				else
				{
				int32 w,h,s,y;
				uint8 * fptr,*tptr;
				
					if ( ! (MipData = (uint8*)jeRam_Allocate(MipDataLen) ) )
					{
						jeErrorLog_AddString(-1,"Bitmap_WriteToFile : Ram_Alloc failed!",NULL);
						return JE_FALSE;
					}

					MipDataAlloced = JE_TRUE;

					s = SHIFT_R_ROUNDUP(Bmp->Info.Stride,mip)* jePixelFormat_BytesPerPel(Bmp->Info.Format);
					w = SHIFT_R_ROUNDUP(Bmp->Info.Width,mip) * jePixelFormat_BytesPerPel(Bmp->Info.Format);
					h = SHIFT_R_ROUNDUP(Bmp->Info.Height,mip);

					fptr = (uint8*)Bmp->Data[mip];
					tptr = MipData;
					for(y=h;y--;)
					{
						memcpy(tptr,fptr,w);
						fptr += s;
						tptr += w;
					}
				}

				assert( mip <= MIP_MASK );
				flags = (uint8)mip;
				//flags |= MIP_FLAG_COMPRESSED;

				if ( ! jeVFile_Write(HF, &flags, sizeof(flags)) )
					return JE_FALSE;

				if ( flags & MIP_FLAG_COMPRESSED )
					LzF = jeVFile_OpenNewSystem(F,JE_VFILE_TYPE_LZ,NULL,NULL,JE_VFILE_OPEN_CREATE);
				else
					LzF = F;
				
				if ( ! LzF )
				{
					if ( MipDataAlloced )
						jeRam_Free(MipData);
					jeErrorLog_AddString(-1,"Bitmap_WriteToFile : LZ File Open failed",NULL);
					return JE_FALSE;
				}

				if ( ! jeVFile_Write(LzF, MipData, MipDataLen ) )
					return JE_FALSE;

				if ( flags & MIP_FLAG_COMPRESSED )
				{
					if ( ! jeVFile_Close(LzF) )
					{
						if ( MipDataAlloced )
							jeRam_Free(MipData);
						jeErrorLog_AddString(-1,"Bitmap_WriteToFile : LZ File Close failed",NULL);
						return JE_FALSE;
					}
				}

				if ( MipDataAlloced )
					jeRam_Free(MipData);
			}
		}
		
		// mip > MaximumMip signals End-Of-Mips

		flags = MIP_MASK;
		if ( ! jeVFile_Write(HF, &flags, sizeof(flags)) )
			return JE_FALSE;
	}

	// the pointer Bmp->Alpha serves as boolean : HasAlpha

	if( Bmp->Alpha )
	{
		if ( ! jeBitmap_WriteToFile(Bmp->Alpha,F) )
			return JE_FALSE;
	}

	if ( ! Bmp->Persistable )
	{
		((jeBitmap *)Bmp)->Persistable = JE_TRUE;
		((jeBitmap *)Bmp)->PersistBaseFS = NULL;
		jeVFile_GetName(F,((jeBitmap *)Bmp)->PersistName,sizeof(((jeBitmap *)Bmp)->PersistName));
	}

return JE_TRUE;
}

/*}{********** Windows BMP Crap *******/

#pragma pack(1)
typedef struct 
{
	uint32      biSize;
	long		biWidth;
	long		biHeight;
	uint16      biPlanes;
	uint16      biBitCount;
	uint32      biCompression;
	uint32      biSizeImage;
	long		biXPelsPerMeter;
	long		biYPelsPerMeter;
	uint32      biClrUsed;
	uint32      biClrImportant;
} BITMAPINFOHEADER;

typedef struct 
{
	uint16   bfType;
	uint32   bfSize;
	uint16   bfReserved1;
	uint16   bfReserved2;
	uint32   bfOffBits;
} BITMAPFILEHEADER;

typedef struct 
{
    uint8    B;
    uint8    G;
    uint8    R;
    uint8    rgbReserved;
} RGBQUAD;
#pragma pack()

static jeBoolean jeBitmap_ReadFromBMP(jeBitmap * Bmp,jeVFile * F)
{
BITMAPFILEHEADER 	bmfh;
BITMAPINFOHEADER	bmih;
int32 bPad,myRowWidth,bmpRowWidth,pelBytes;

	// Windows Bitmap

	if ( ! jeVFile_Read(F, &bmfh, sizeof(bmfh)) )
		return JE_FALSE;

	assert(bmfh.bfType == 0x4D42);

	bPad = bmfh.bfOffBits;

	if ( ! jeVFile_Read(F, &bmih, sizeof(bmih)) )
		return JE_FALSE;

	if ( bmih.biSize > sizeof(bmih) )
	{
		jeVFile_Seek(F, bmih.biSize - sizeof(bmih), JE_VFILE_SEEKCUR);
	}
	else if ( bmih.biSize < sizeof(bmih) )
	{
		jeErrorLog_AddString(-1,"CreateFromFile : bmih size bad", NULL);	
		return JE_FALSE;
	}

	if ( bmih.biCompression )
	{
		jeErrorLog_AddString(-1,"CreateFromFile : only BI_RGB BMP compression supported", NULL);
		return JE_FALSE;
	}

	bPad -= sizeof(bmih) + sizeof(bmfh);

	switch (bmih.biBitCount) 
	{
		case 8:			/* colormapped image */
			if ( bmih.biClrUsed == 0 ) bmih.biClrUsed = 256;

			if ( ! (Bmp->Info.Palette = jeBitmap_Palette_Create(JE_PIXELFORMAT_32BIT_XRGB,bmih.biClrUsed)) )
				return JE_FALSE;

			if ( ! jeVFile_Read(F, Bmp->Info.Palette->Data, bmih.biClrUsed * 4) )
				return JE_FALSE;

			bPad -= bmih.biClrUsed * 4;

			Bmp->Info.Format = JE_PIXELFORMAT_8BIT_PAL;
			pelBytes = 1;
			break;
		case 16:			
			Bmp->Info.Format = JE_PIXELFORMAT_16BIT_555_RGB;
			// tried 555,565_BGR & RGB, seems to have too much green
			pelBytes = 2;
			break;
		case 24:			
			Bmp->Info.Format = JE_PIXELFORMAT_24BIT_BGR;
			pelBytes = 3;
			break;
		case 32:			
			Bmp->Info.Format = JE_PIXELFORMAT_32BIT_XRGB; // surprisingly sane !?
			pelBytes = 4;
			break;
		default:
			return JE_FALSE;
	}

	if ( bPad < 0 )
	{
		jeErrorLog_AddString(-1,"CreateFromFile : bPad bad", NULL);
		return JE_FALSE;
	}

	jeVFile_Seek(F, bPad, JE_VFILE_SEEKCUR);
	
	Bmp->Info.Width = bmih.biWidth;
	Bmp->Info.Height = abs(bmih.biHeight);
	Bmp->Info.Stride = ((bmih.biWidth+3)&(~3));

	Bmp->Info.HasColorKey = JE_FALSE;

	myRowWidth	= Bmp->Info.Stride * pelBytes;
	bmpRowWidth = (((bmih.biWidth * pelBytes) + 3)&(~3));

	assert( bmpRowWidth <= myRowWidth );

	if ( ! jeBitmap_AllocSystemMip(Bmp,0) )
		return JE_FALSE;

	if ( bmih.biHeight > 0 )
	{
	int32 y;
	char * row;
		row = (char *)Bmp->Data[0];
		row += (Bmp->Info.Height - 1) * myRowWidth;
		for(y= Bmp->Info.Height;y--;)
		{
			if ( ! jeVFile_Read(F, row, bmpRowWidth) )
				return JE_FALSE;				
			row -= myRowWidth;
		}
	}
	else
	{
	int32 y;
	char * row;
		row = (char *)Bmp->Data[0];
		for(y= Bmp->Info.Height;y--;)
		{
			if ( ! jeVFile_Read(F, row, bmpRowWidth) )
				return JE_FALSE;				
			row += myRowWidth;
		}
	}

return JE_TRUE;
}	// end BMP reader

/*
KROUER:
First try to save jeBitmap as windows bmps
Function cancelled for the moment
static jeBoolean jeBitmap_WriteToBMP(jeBitmap * Bmp,jeVFile * F)
{
	BITMAPFILEHEADER bfh = 
	{
		((unsigned short)'B' | ((unsigned short)'M' << 8)),
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + Bmp->Info.Width * Bmp->Info.Height * 2,
		0,
		0,
		sizeof(BITMAPINFOHEADER)
	};

	BITMAPINFOHEADER bi =
	{
		sizeof(BITMAPINFOHEADER),
		Bmp->Info.Width,
		Bmp->Info.Height,
		1,
		24,
		0,
		0,
		0,
		0,
		0,
		0
	};
	return JE_TRUE;
}
*/

/*}{ *** Packed Info IO ***/

#define INFO_FLAG_WH_ARE_LOG2	(1<<0)
#define INFO_FLAG_HAS_CK     	(1<<1)
#define INFO_FLAG_HAS_ALPHA  	(1<<2)
#define INFO_FLAG_HAS_PAL    	(1<<3)
#define INFO_FLAG_HAS_AVERAGE  	(1<<4)
#define INFO_FLAG_IF_NOT_LOG2_ARE_BYTE	(1<<5)

jeBoolean jeBitmap_ReadInfo(jeBitmap *Bmp,jeVFile * F)
{
uint8 data[4];
uint8 flags;
uint8 b;
uint16 w;
jeBitmap_Info * pi;

	pi = &(Bmp->Info);

	if ( ! jeVFile_Read(F,data,3) )
		return JE_FALSE;

	flags = data[0];

	pi->Format = (jePixelFormat)data[1]; // could go in 5 bits
	if ( ! jePixelFormat_IsValid(pi->Format) )
		return JE_FALSE;

	b = data[2];

	pi->MaximumMip = (b>>4)&0xF;
	Bmp->SeekMipCount = (b)&0xF;

	if ( flags & INFO_FLAG_HAS_PAL )
		pi->Palette  = (jeBitmap_Palette *)1;
	if ( flags & INFO_FLAG_HAS_ALPHA )
		Bmp->Alpha = (jeBitmap *)1;

	if ( flags & INFO_FLAG_WH_ARE_LOG2 )
	{
	int logw,logh;

		if ( ! jeVFile_Read(F,&b,1) )
			return JE_FALSE;

		logw = (b>>4)&0xF;
		logh = (b   )&0xF;

		pi->Width = 1<<logw;
		pi->Height= 1<<logh;
	}
	else if ( flags & INFO_FLAG_IF_NOT_LOG2_ARE_BYTE )
	{
		if ( ! jeVFile_Read(F,&b,1) )
			return JE_FALSE;
		pi->Width = b;
		if ( ! jeVFile_Read(F,&b,1) )
			return JE_FALSE;
		pi->Height = b;
	}
	else
	{
		if ( ! jeVFile_Read(F,&w,2) )
			return JE_FALSE;
		pi->Width = w;
		if ( ! jeVFile_Read(F,&w,2) )
			return JE_FALSE;
		pi->Height = w;
	}

	if ( (flags & INFO_FLAG_HAS_CK) && jePixelFormat_BytesPerPel(pi->Format) > 0 )
	{
	uint8 * ptr;
		pi->HasColorKey = JE_TRUE;

		if ( ! jeVFile_Read(F,data,jePixelFormat_BytesPerPel(pi->Format)) )
			return JE_FALSE;
		
		ptr = data;
		pi->ColorKey = jePixelFormat_GetPixel(pi->Format,&ptr);
	}

	if ( flags & INFO_FLAG_HAS_AVERAGE )
	{
		if ( ! jeVFile_Read(F,data,3) )
			return JE_FALSE;

		Bmp->HasAverageColor = JE_TRUE;
		Bmp->AverageR = data[0];
		Bmp->AverageG = data[1];
		Bmp->AverageB = data[2];
	}

	pi->Stride = pi->Width;

	return JE_TRUE;
}

jeBoolean jeBitmap_WriteInfo(const jeBitmap *Bmp,jeVFile * F)
{
uint8 data[64];
uint8 * ptr;
uint8 flags;
uint8 b;
int len,logw,logh;
const jeBitmap_Info * pi;
int R,G,B;

/*
	bit flags :
		W&H are log2
		HasCK
		HasAlpha
		HasPal

		W&H logs in 1 byte, or W & H each in 2 bytes

		Format in 5 bits
		MaxMip in 3 bits
		Bmp->SeekMipCount in 3 bits

		CK in bpp bytes
*/

	pi = &(Bmp->Info);
	ptr = data;

	assert( pi->Width < 65536 && pi->Height < 65536 );
	assert( pi->MinimumMip == 0 );
	assert( jePixelFormat_IsValid(pi->Format) );

	flags = 0;
	*ptr++ = 0; // flags will go there

	*ptr++ = pi->Format; // could go in 5 bits

	b = (uint8)((pi->MaximumMip << 4) + Bmp->SeekMipCount); // could go in 6 bits
	*ptr++ = b;

	if ( pi->Palette )
		flags |= INFO_FLAG_HAS_PAL;
	if ( Bmp->Alpha )
		flags |= INFO_FLAG_HAS_ALPHA;

	for(logw=0;(1<<logw) < pi->Width;logw++);
	for(logh=0;(1<<logh) < pi->Height;logh++);

	if ( (1<<logw) == pi->Width && (1<<logh) == pi->Height )
	{
		flags |= INFO_FLAG_WH_ARE_LOG2;
		assert( logw <= 0xF && logh <= 0xF );
		b = (logw<<4) + logh;
		*ptr++ = b;
	}
	else
	{
		if ( pi->Width < 256 && pi->Height < 256 )
		{
			flags |= INFO_FLAG_IF_NOT_LOG2_ARE_BYTE;
			*ptr++ = (uint8)pi->Width;
			*ptr++ = (uint8)pi->Height;
		}
		else
		{
			*((uint16 *)ptr) = (uint16)pi->Width;  ptr += 2;
			*((uint16 *)ptr) = (uint16)pi->Height; ptr += 2;
		}
	}

	if ( pi->HasColorKey && jePixelFormat_BytesPerPel(pi->Format) > 0 )
	{
		flags |= INFO_FLAG_HAS_CK;

		jePixelFormat_PutPixel(pi->Format,&ptr,pi->ColorKey);
	}

	if ( jeBitmap_GetAverageColor(Bmp,&R,&G,&B) )
	{
		flags |= INFO_FLAG_HAS_AVERAGE;
		*ptr++ = JE_CLAMP8(R);
		*ptr++ = JE_CLAMP8(G);
		*ptr++ = JE_CLAMP8(B);
	}

	*data = flags;
	len = (int)(ptr - data);

	if ( ! jeVFile_Write(F,data,len) )
		return JE_FALSE;

	return JE_TRUE;
}

/*}{ ***************** Palette Functions *******************/

jeBoolean jeBitmap_Palette_BlitData(jePixelFormat SrcFormat,const void *SrcData,const jeBitmap_Palette * SrcPal,
									jePixelFormat DstFormat,	  void *DstData,const jeBitmap_Palette * DstPal,
									int32 Pixels)
{
char *SrcPtr,*DstPtr;
jeBoolean SrcHasCK,DstHasCK;
uint32 SrcCK,DstCK;
int SrcCKi,DstCKi;

	assert( SrcData && DstData );

	assert( jePixelFormat_IsRaw(SrcFormat) );
	assert( jePixelFormat_IsRaw(DstFormat) );

	SrcPtr = (char *)SrcData;
	DstPtr = (char *)DstData;

	if ( SrcPal && SrcPal->HasColorKey )
	{
		SrcHasCK = JE_TRUE;
		SrcCK = SrcPal->ColorKey;
		SrcCKi = SrcPal->ColorKeyIndex;
	}
	else
	{
		SrcHasCK = JE_FALSE;
	}

	if ( DstPal && DstPal->HasColorKey )
	{
		DstHasCK = JE_TRUE;
		DstCK = DstPal->ColorKey;
		DstCKi = DstPal->ColorKeyIndex;
	}
	else
	{
		DstHasCK = JE_FALSE;
	}

#if 0 // {} ?
	if ( SrcHasCK && DstHasCK )
	{
		if ( DstCKi == -1 )
			DstCKi = SrcCKi;
	}
#endif

	// no, can't do this, and if SrcCKi < 0 then it's just ignored, which is correct
	//assert( SrcCKi >= 0 );
	//assert( DstCKi >= 0 );

	// CK -> no CK : do nothing
	// no CK -> CK : avoid CK
	// CK -> CK    : assert the CKI's are the same; change color at CKI

	{
	uint32 Pixel;
	int p,R,G,B,A;
	const jePixelFormat_Operations *SrcOps,*DstOps;
	jePixelFormat_Composer		ComposePixel;
	jePixelFormat_Decomposer	DecomposePixel;
	jePixelFormat_PixelPutter	PutPixel;
	jePixelFormat_PixelGetter	GetPixel;

		SrcOps = jePixelFormat_GetOperations(SrcFormat);
		DstOps = jePixelFormat_GetOperations(DstFormat);
		assert(SrcOps && DstOps);

		GetPixel = SrcOps->GetPixel;
		DecomposePixel = SrcOps->DecomposePixel;
		ComposePixel = DstOps->ComposePixel;
		PutPixel = DstOps->PutPixel;

		if ( SrcOps->AMask && ! DstOps->AMask )
		{
			// alpha -> CK in the palette
			for(p=0;p<Pixels;p++)
			{
				Pixel = GetPixel((uint8**)&SrcPtr);
				DecomposePixel(Pixel,&R,&G,&B,&A);
				if ( SrcHasCK && ( p == SrcCKi || Pixel == SrcCK ) ) 
					A = 0;
				Pixel = ComposePixel(R,G,B,A);

				if ( DstHasCK )
				{
					if ( p == DstCKi || A < 128 )
						Pixel = DstCK;
					else if ( Pixel == DstCK )
						Pixel ^= 1;

					// BTW this makes dark blue into dark purple on glide
				}
				PutPixel((uint8**)&DstPtr,Pixel);
			}
		}
		else if ( ! SrcOps->AMask && DstOps->AMask )
		{
			// CK -> alpha in the palette
			for(p=0;p<Pixels;p++)
			{
				Pixel = GetPixel((uint8**)&SrcPtr);
				DecomposePixel(Pixel,&R,&G,&B,&A);
				if ( SrcHasCK && ( p == SrcCKi || Pixel == SrcCK ) ) 
					A = 0;

				Pixel = ComposePixel(R,G,B,A);
				if ( DstHasCK )
				{
					if ( p == DstCKi )
						Pixel = DstCK;
					else if ( Pixel == DstCK )
						Pixel ^= 1;
				}
				PutPixel((uint8**)&DstPtr,Pixel);
			}
		}
		else
		{
			// both have alpha or both don't
			for(p=0;p<Pixels;p++)
			{
				Pixel = GetPixel((uint8**)&SrcPtr);
				DecomposePixel(Pixel,&R,&G,&B,&A);
				if ( (SrcHasCK && ( p == SrcCKi || Pixel == SrcCK )) ||
					 DstHasCK && p == DstCKi ) 
				{
					Pixel = DstCK;
				}
				else
				{
					Pixel = ComposePixel(R,G,B,A);
					if ( DstHasCK && Pixel == DstCK )
						Pixel ^= 1;
				}
				PutPixel((uint8**)&DstPtr,Pixel);
			}
		}
	}

return JE_TRUE;
}

JETAPI jeBitmap_Palette * JETCC jeBitmap_Palette_Create(jePixelFormat Format,int32 Size)
{
jeBitmap_Palette * P;
int DataBytes;
const jePixelFormat_Operations * ops;

	ops = jePixelFormat_GetOperations(Format);
	if ( ! ops->RMask )
	{
		jeErrorLog_AddString(-1,"jeBitmap_Palette_Create : Invalid format for a palette!", NULL);
		return NULL;
	}

	DataBytes = jePixelFormat_BytesPerPel(Format) * Size;
	if ( DataBytes == 0 )
	{
		jeErrorLog_AddString(-1,"jeBitmap_Palette_Create : Invalid format for a palette!", NULL);
		return NULL;
	}

	P = (jeBitmap_Palette *)jeRam_Allocate(sizeof(jeBitmap_Palette));
	if ( ! P ) return NULL;
	clear(P);

	P->Size = Size;
	P->Format = Format;
	if ( ! (P->Data = jeRam_Allocate(DataBytes)) )
	{
		jeRam_Free(P);
		return NULL;
	}

	P->RefCount = 1;
	P->LockCount = 0;

	P->HasColorKey = JE_FALSE;

return P;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_CreateRef(jeBitmap_Palette *P)
{
	if ( ! P || P->RefCount < 1 )
		return JE_FALSE;
	P->RefCount ++;
return JE_TRUE;
}

JETAPI jeBitmap_Palette * JETCC jeBitmap_Palette_CreateFromBitmap(jeBitmap * Bmp,jeBoolean Slow)
{
jeBitmap_Palette * Pal;
	Pal = jeBitmap_GetPalette(Bmp);
	if ( Pal )
	{
		jeBitmap_Palette_CreateRef(Pal);
		return Pal;
	}
	else
	{
		return createPaletteFromBitmap(Bmp, Slow);
	}
}

jeBitmap_Palette * BITMAP_JET_INTERNAL jeBitmap_Palette_CreateFromDriver(DRV_Driver * Driver,jePixelFormat Format,int32 Size)
{
jeBitmap_Palette * P;
jeTexture_Info TInfo;

	assert(Driver);

	P = (jeBitmap_Palette *)jeRam_Allocate(sizeof(jeBitmap_Palette));

	if ( ! P ) return NULL;
	clear(P);

	P->Size = Size;
	P->Driver = Driver;	

	// {} the pixelformat passed in here has non-trivial implications when the
	//		driver provides more than one possible palette type

	assert( jePixelFormat_IsRaw(Format) );

	P->DriverHandle = jeBitmap_CreateTHandle(Driver,Size,1,1,
			Format,JE_PIXELFORMAT_NO_DATA,0,jePixelFormat_HasAlpha(Format),0,RDRIVER_PF_PALETTE);
	if ( ! P->DriverHandle )
	{
		jeErrorLog_AddString(-1,"Palette_CreateFromDriver : CreateTHandle", NULL);	
		jeRam_Free(P);
		return NULL;
	}

	Driver->THandle_GetInfo(P->DriverHandle,0,&TInfo);
	P->Format = TInfo.PixelFormat.PixelFormat;

	P->HasColorKey = (TInfo.Flags & RDRIVER_THANDLE_HAS_COLORKEY) ? JE_TRUE : JE_FALSE;
	P->ColorKey = TInfo.ColorKey;
	P->ColorKeyIndex = -1;

	P->RefCount = 1;

return P;
}

JETAPI jeBitmap_Palette * JETCC jeBitmap_Palette_CreateCopy(const jeBitmap_Palette *Palette)
{
jeBitmap_Palette * P;

	if ( ! Palette )
		return NULL;

	if ( Palette->Driver )
	{
		P = jeBitmap_Palette_CreateFromDriver(Palette->Driver,Palette->Format,Palette->Size);
	}
	else
	{
		P = jeBitmap_Palette_Create(Palette->Format,Palette->Size);
	}

	if ( ! P ) return NULL;

	if ( ! jeBitmap_Palette_Copy(Palette,P) )
	{
		jeBitmap_Palette_Destroy(&P);
		return NULL;
	}

return P;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_Destroy(jeBitmap_Palette ** ppPalette)
{
jeBitmap_Palette * Palette;
	assert(ppPalette);
	if ( Palette = *ppPalette )
	{
		if ( Palette->LockCount )
			return JE_FALSE;
		Palette->RefCount --;
		if ( Palette->RefCount <= 0 )
		{
			if ( Palette->Data )
				jeRam_Free(Palette->Data);
			if ( Palette->DriverHandle )
			{
				Palette->Driver->THandle_Destroy(Palette->DriverHandle);
				Palette->DriverHandle = NULL;
			}
			jeRam_Free(Palette);
		}
	}
	*ppPalette = NULL;
return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_Lock(jeBitmap_Palette *P, void **pBits, jePixelFormat *pFormat,int32 *pSize)
{
	assert(P);
	assert(pBits);

	if ( P->LockCount )
		return JE_FALSE;
	P->LockCount++;

	*pBits = NULL;

	if ( P->Data )
	{
		*pBits		= P->Data;
		if ( pFormat )
			*pFormat= P->Format;
		if ( pSize )
			*pSize	= P->Size;
	}
	else if ( P->DriverHandle )
	{
	jeTexture_Info TInfo;

		if ( ! P->Driver->THandle_GetInfo(P->DriverHandle,0,&TInfo) )
			return JE_FALSE;

		if ( TInfo.Height != 1 )
			return JE_FALSE;

		if ( ! (P->Driver->THandle_Lock(P->DriverHandle,0,pBits)) )
			*pBits = NULL;

		P->DriverBits = *pBits;

		if ( pFormat )
			*pFormat = TInfo.PixelFormat.PixelFormat;
		if ( pSize )
			*pSize = TInfo.Width;
	}

	return (*pBits) ? JE_TRUE : JE_FALSE;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_UnLock(jeBitmap_Palette *P)
{
	assert(P);
	if ( P->LockCount <= 0 )
		return JE_FALSE;
	P->LockCount--;
	if ( P->LockCount == 0 )
	{
		if ( P->HasColorKey )
		{
			if ( P->ColorKeyIndex >= 0 && P->ColorKeyIndex < P->Size )
			{
			uint8 *Bits,*pBits;
			uint32 Pixel;
			int p;
			const jePixelFormat_Operations *ops;
			jePixelFormat_PixelPutter	PutPixel;
			jePixelFormat_PixelGetter	GetPixel;

				if ( P->Data )
				{
					Bits = (uint8*)P->Data;
				}
				else if ( P->DriverBits )
				{
					Bits = (uint8*)P->DriverBits;
				}

				ops = jePixelFormat_GetOperations(P->Format);
				assert(ops);

				GetPixel = ops->GetPixel;
				PutPixel = ops->PutPixel;

				for(p=0;p<P->Size;p++)
				{
					pBits = Bits;
					Pixel = GetPixel(&Bits);
					if ( p == P->ColorKeyIndex )
					{
						PutPixel(&pBits,P->ColorKey);
					}
					else if ( Pixel == P->ColorKey )
					{
						Pixel ^= 1;
						PutPixel(&pBits,Pixel);
					}
				}
			}
		}
		if ( P->DriverHandle )
		{
			if ( ! P->Driver->THandle_UnLock(P->DriverHandle,0) )
				return JE_FALSE;
			P->DriverBits = NULL;
		}
	}
return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_SetFormat(jeBitmap_Palette * P,jePixelFormat Format)
{
void * NewData;
	
	assert(P);

	if ( P->DriverHandle ) // can't change format on card!
		return JE_FALSE;

	assert( ! P->HasColorKey ); // can't have colorkey accept on crappy Glide

	if ( Format == P->Format )
		return JE_TRUE;

	NewData = jeRam_Allocate( jePixelFormat_BytesPerPel(Format) * P->Size );
	if ( ! NewData )
		return JE_FALSE;

	if ( ! jeBitmap_Palette_BlitData(P->Format,P->Data,NULL,Format,NewData,NULL,P->Size) )
	{
		jeRam_Free(NewData);
		return JE_FALSE;
	}

	jeRam_Free(P->Data);
	P->Data = NewData;
	P->Format = Format;

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_GetData(const jeBitmap_Palette *P,void *Into,jePixelFormat Format,int32 Size)
{
jePixelFormat FmFormat;
const void *FmData;
int32 FmSize;
jeBoolean Ret;

	assert(P);
	assert(Into);

	if ( ! jeBitmap_Palette_Lock((jeBitmap_Palette *)P,(void **)&FmData,&FmFormat,&FmSize) )
		return JE_FALSE;

	if ( FmSize < Size )
		Size = FmSize;

	Ret = jeBitmap_Palette_BlitData(FmFormat,FmData,P,Format,Into,NULL,Size);
	
	jeBitmap_Palette_UnLock((jeBitmap_Palette *)P);

return Ret;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_GetInfo(const jeBitmap_Palette *P,jeBitmap_Info *pInfo)
{
	assert(P && pInfo);

	pInfo->Width = pInfo->Stride = P->Size;
	pInfo->Height = 1;

	pInfo->Format = P->Format;
	pInfo->HasColorKey = P->HasColorKey;
	pInfo->ColorKey = P->ColorKey;
	pInfo->MaximumMip = pInfo->MinimumMip = 0;
	pInfo->Palette = NULL;

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_SetData(jeBitmap_Palette *P,const void *From,jePixelFormat Format,int32 Colors)
{
jePixelFormat PalFormat;
void *PalData;
int32 PalSize;
jeBoolean Ret;

	assert(P);
	assert(From);

	if ( ! jeBitmap_Palette_Lock(P,&PalData,&PalFormat,&PalSize) )
		return JE_FALSE;

	if ( PalSize < Colors )
		Colors = PalSize;

	Ret = jeBitmap_Palette_BlitData(Format,From,NULL,PalFormat,PalData,P,Colors);
	
	if ( ! jeBitmap_Palette_UnLock(P) )
		return JE_FALSE;

return Ret;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_Copy(const jeBitmap_Palette * Fm,jeBitmap_Palette * To)
{
jePixelFormat FmFormat,ToFormat;
void *FmData,*ToData;
int32 FmSize,ToSize;
jeBoolean Ret;

	assert(Fm);
	assert(To);
	if ( Fm == To )
		return JE_TRUE;

	if ( ! jeBitmap_Palette_Lock((jeBitmap_Palette *)Fm,&FmData,&FmFormat,&FmSize) )
		return JE_FALSE;

	if ( ! jeBitmap_Palette_Lock(To,&ToData,&ToFormat,&ToSize) )
	{
		jeBitmap_Palette_UnLock((jeBitmap_Palette *)Fm);
		return JE_FALSE;
	}

	if ( FmSize > ToSize )
	{
		Ret = JE_FALSE;
	}
	else
	{
		Ret = jeBitmap_Palette_BlitData(FmFormat,FmData,Fm,ToFormat,ToData,To,FmSize);
	}
	
	jeBitmap_Palette_UnLock((jeBitmap_Palette *)Fm);
	jeBitmap_Palette_UnLock(To);

return Ret;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_SetEntryColor(jeBitmap_Palette *P,int32 Color,int32 R,int32 G,int32 B,int32 A)
{
	assert(P);
	
	if ( A < 80 && ! jePixelFormat_HasAlpha(P->Format) && P->HasColorKey )
	{
		return jeBitmap_Palette_SetEntry(P,Color,P->ColorKey);
	}
	else if ( P->HasColorKey )
	{
	uint32 Pixel;

		// might have alpha AND colorkey !

		if ( Color == P->ColorKeyIndex ) // and A > 80 because of the above
			return JE_FALSE;

		Pixel = jePixelFormat_ComposePixel(P->Format,R,G,B,A);
		if ( Pixel == P->ColorKey )
			Pixel ^= 1;
			
		return jeBitmap_Palette_SetEntry(P,Color,Pixel);
	}
	else
	{
		return jeBitmap_Palette_SetEntry(P,Color,jePixelFormat_ComposePixel(P->Format,R,G,B,A));
	}
}

JETAPI jeBoolean JETCC jeBitmap_Palette_GetEntryColor(const jeBitmap_Palette *P,int32 Color,int32 *R,int32 *G,int32 *B,int32 *A)
{
uint32 Pixel;
	assert(P);
	if ( P->HasColorKey )
	{
		if ( Color == P->ColorKeyIndex )
		{
			*R = *G = *B = *A = 0;
			return JE_TRUE;
		}
		else
		{
			if ( ! jeBitmap_Palette_GetEntry(P,Color,&Pixel) )
				return JE_FALSE;
			if ( Pixel == P->ColorKey )
			{
				*R = *G = *B = *A = 0;
			}
			else
			{
				jePixelFormat_DecomposePixel(P->Format,Pixel,R,G,B,A);
			}
		}
	}
	else
	{
		if ( ! jeBitmap_Palette_GetEntry(P,Color,&Pixel) )
			return JE_FALSE;
		jePixelFormat_DecomposePixel(P->Format,Pixel,R,G,B,A);
	}
	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_SetEntry(jeBitmap_Palette *P,int32 Color,uint32 Pixel)
{
	assert(P);

	if ( P->HasColorKey )
	{
		if ( Color == P->ColorKeyIndex )
			return JE_FALSE;
	}

	if ( P->Data )
	{
	char *Data;

		if ( Color >= P->Size )
			return JE_FALSE;

		Data = (char *)(P->Data) + Color * jePixelFormat_BytesPerPel(P->Format);
		jePixelFormat_PutPixel(P->Format,(uint8 **)&Data,Pixel);
	}
	else
	{
	char *Data;
	jePixelFormat Format;
	int Size;

		if ( ! jeBitmap_Palette_Lock(P,(void **)&Data,&Format,&Size) )
			return JE_FALSE;

		if ( Color >= Size )
		{
			jeBitmap_Palette_UnLock(P);
			return JE_FALSE;
		}

		Data += Color * jePixelFormat_BytesPerPel(Format);
		jePixelFormat_PutPixel(Format,(uint8**)&Data,Pixel);

		jeBitmap_Palette_UnLock(P);
	}
return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_GetEntry(const jeBitmap_Palette *P,int32 Color,uint32 *Pixel)
{

	assert(P);

	if ( P->Data )
	{
	char *Data;

		if ( Color >= P->Size )
			return JE_FALSE;

		Data = (char *)(P->Data) + Color * jePixelFormat_BytesPerPel(P->Format);
		*Pixel = jePixelFormat_GetPixel(P->Format,(uint8 **)&Data);
	}
	else
	{
	char *Data;
	jePixelFormat Format;
	int Size;

		// must cast away const cuz we don't have a lockforread/write on palettes

		if ( ! jeBitmap_Palette_Lock((jeBitmap_Palette *)P,(void **)&Data,&Format,&Size) )
			return JE_FALSE;

		if ( Color >= Size )
		{
			jeBitmap_Palette_UnLock((jeBitmap_Palette *)P);
			return JE_FALSE;
		}

		Data += Color * jePixelFormat_BytesPerPel(Format);
		*Pixel = jePixelFormat_GetPixel(Format,(uint8 **)&Data);

		jeBitmap_Palette_UnLock((jeBitmap_Palette *)P);
	}
return JE_TRUE;
}

#define PALETTE_INFO_FORMAT_MASK	(0x1F)
#define PALETTE_INFO_FLAG_SIZE256	(1<<5)	// 5 is the low
#define PALETTE_INFO_FLAG_COMPRESS	(1<<6)

JETAPI jeBitmap_Palette * JETCC jeBitmap_Palette_CreateFromFile(jeVFile *F)
{
jeBitmap_Palette * P;
int Size;
jePixelFormat Format;
uint8 flags,b;
jeVFile * HF;

	// for old version compatibility :
	// in new versions, F is already a hints file
	if ( ( HF = jeVFile_GetHintsFile(F) ) == NULL )
		HF = F;

	if ( ! jeVFile_Read(HF, &flags, sizeof(flags)) )
		return NULL;

	Format = (jePixelFormat)(flags & PALETTE_INFO_FORMAT_MASK);

	if ( flags & PALETTE_INFO_FLAG_SIZE256 )
	{
		Size = 256;
	}
	else
	{
		if ( ! jeVFile_Read(HF, &b, sizeof(b)) )
			return NULL;
		Size = b;
	}

	P = jeBitmap_Palette_Create(Format,Size);
	if ( ! P )
		return NULL;

	if ( flags & PALETTE_INFO_FLAG_COMPRESS )
	{
		if ( ! codePal_Read(P, F) )
		{
			jeErrorLog_AddString(-1,"Bitmap_Palette_CreateFromFile : codePal failed!",NULL);
			return NULL;
		}
	}
	else
	{
		if ( ! jeVFile_Read(F, P->Data, jePixelFormat_BytesPerPel(P->Format) * P->Size) )
		{
			jeRam_Free(P);
			return NULL;
		}
	}

return P;
}

	// we usually write the palette's header in one byte :^)

jeBoolean JETCC jeBitmap_Palette_WriteHeaderToFile(int32 Size,jePixelFormat Format,jeBoolean Compressed,jeVFile *F)
{
uint8 b;

	b = Format;
	assert( b < 32 );
	if ( Size == 256 )
		b |= PALETTE_INFO_FLAG_SIZE256;

	// to compress :

	if ( Compressed )
		b |= PALETTE_INFO_FLAG_COMPRESS;

	if ( ! jeVFile_Write(F, &b, sizeof(b)) )
		return JE_FALSE;

	if ( Size != 256 )
	{
		assert(Size < 256);
		b = (uint8)Size;
		
		if ( ! jeVFile_Write(F, &b, sizeof(b)) )
			return JE_FALSE;
	}

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_WriteToFile(const jeBitmap_Palette *P,jeVFile *F)
{
jePixelFormat Format;
void *Data;
int Size,codedLen,StartPos;

	assert(P);

	// assert(F->IsHintsFile); // F is a hints file!

	assert( P->HasColorKey == JE_FALSE ); // system palettes can't have color key!

	if ( ! jeBitmap_Palette_Lock((jeBitmap_Palette *)P,&Data,&Format,&Size) )
		return JE_FALSE;

	jeBitmap_Palette_UnLock((jeBitmap_Palette *)P);

	jeVFile_Tell(F,(int32 *)&StartPos);

	jeBitmap_Palette_WriteHeaderToFile(Size,Format,JE_TRUE,F);

	if ( ! codePal_Write(P, F, &codedLen) )
	{
		jeErrorLog_AddString(-1,"Bitmap_Palette_WriteToFile : codePal failed!",NULL);
		return JE_FALSE;
	}
	
	if ( (uint32)codedLen > (jePixelFormat_BytesPerPel(Format) * Size) )
	{
		jeVFile_Seek(F, StartPos, JE_VFILE_SEEKSET);

		jeBitmap_Palette_WriteHeaderToFile(Size,Format,JE_FALSE,F);

		if ( ! jeBitmap_Palette_Lock((jeBitmap_Palette *)P,&Data,&Format,&Size) )
			return JE_FALSE;

		if ( ! jeVFile_Write(F, Data, jePixelFormat_BytesPerPel(Format) * Size) )
		{
			jeBitmap_Palette_UnLock((jeBitmap_Palette *)P);
			return JE_FALSE;
		}
		
		jeBitmap_Palette_UnLock((jeBitmap_Palette *)P);
	}

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBitmap_Palette_SortColors(jeBitmap_Palette * P,jeBoolean Slower)
{
int permutation[256],usage[256];
uint8 paldata[768];
int flags;
int i;

	assert(P);
	
	for(i=0;i<256;i++)
	{
		usage[i] = 1;
		permutation[i] = i;
	}

	// <> usePal / reducePal

	if ( Slower )
		flags = SORTPAL_OPTIMIZE;
	else
		flags = SORTPAL_FAST;

	if ( jePixelFormat_HasAlpha(P->Format) )
	{
	uint8 rgba_in[1024];
	uint8 rgba_out[1024];

		// if the palette has alpha, use the permutation to shuffle  
		//	the alphas and thereby retain them!

		if ( ! jeBitmap_Palette_GetData(P,rgba_in,JE_PIXELFORMAT_32BIT_RGBA,P->Size) )
			return JE_FALSE;
		if ( ! jeBitmap_Palette_GetData(P,paldata,JE_PIXELFORMAT_24BIT_RGB,P->Size) )
			return JE_FALSE;

		if ( ! sortPal(P->Size,paldata,permutation,usage,flags) )
			return JE_FALSE;

		for(i=0;i<256;i++)
		{
		int j,R,G,B,A;
			j = permutation[i];
			R = paldata[3*i + 0];
			G = paldata[3*i + 1];
			B = paldata[3*i + 2];
			A = rgba_in[4*j];
			rgba_out[4*i + 0] = A;
			rgba_out[4*i + 1] = B;
			rgba_out[4*i + 2] = G;
			rgba_out[4*i + 3] = R;
		}

		if ( ! jeBitmap_Palette_SetData(P,rgba_out,JE_PIXELFORMAT_32BIT_RGBA,P->Size) )
			return JE_FALSE;
	}
	else
	{
		if ( ! jeBitmap_Palette_GetData(P,paldata,JE_PIXELFORMAT_24BIT_RGB,P->Size) )
			return JE_FALSE;

		if ( ! sortPal(P->Size,paldata,permutation,usage,flags) )
			return JE_FALSE;
			
		if ( ! jeBitmap_Palette_SetData(P,paldata,JE_PIXELFORMAT_24BIT_RGB,P->Size) )
			return JE_FALSE;
	}

return JE_TRUE;
}

/*}{ ******************** EOF **************************/

// {} put ErrorLogs indicating where we failed in _IsValid

jeBoolean jeBitmap_IsValid(const jeBitmap *Bmp)
{
	if ( ! Bmp ) return JE_FALSE;

	assert( Bmp->RefCount >= 1 );

	assert( ! (Bmp->LockCount && Bmp->LockOwner) );

	assert( !( (Bmp->DriverDataChanged || Bmp->DriverBitsLocked) &&
			! Bmp->DriverHandle ) );
	assert( ! (Bmp->DriverHandle && ! Bmp->Driver) );

	if ( ! jeBitmap_Info_IsValid(&(Bmp->Info)) )
		return JE_FALSE;

	if ( Bmp->DriverHandle && ! jeBitmap_Info_IsValid(&(Bmp->DriverInfo)) )
		return JE_FALSE;

	if ( Bmp->LockOwner && Bmp->Alpha )
		assert( Bmp->Alpha->LockOwner );

	if ( Bmp->LockOwner )
	{
		assert(Bmp->LockOwner != Bmp);
		assert( Bmp->LockOwner->LockCount );
	}

	if ( Bmp->DataOwner )
	{
		assert(Bmp->DataOwner != Bmp);
		assert( Bmp->DataOwner->RefCount >= 2 );
	}

	if ( Bmp->Alpha )
	{
		assert(Bmp->Alpha != Bmp);
		if ( ! jeBitmap_IsValid(Bmp->Alpha) )
			return JE_FALSE;
	}

return JE_TRUE;
}

jeBoolean jeBitmap_Info_IsValid(const jeBitmap_Info *Info)
{
	if ( ! Info ) return JE_FALSE;

	assert( Info->Width > 0 && Info->Height > 0 && Info->Stride >= Info->Width );

	assert( Info->MinimumMip >= 0 && Info->MaximumMip < MAXMIPLEVELS && Info->MinimumMip <= Info->MaximumMip );

	assert( Info->Format > JE_PIXELFORMAT_NO_DATA && Info->Format < JE_PIXELFORMAT_COUNT );

//	ok to have palette on non-palettized
//	if ( ! jePixelFormat_HasPalette(Info->Format) && Info->Palette )
//		return JE_FALSE;

	if ( Info->Palette )
		if ( ! jeBitmap_Palette_IsValid(Info->Palette) )
			return JE_FALSE;

return JE_TRUE;
}

jeBoolean jeBitmap_Palette_IsValid(const jeBitmap_Palette *Pal)
{
	if ( ! Pal ) return JE_FALSE;

	assert(  Pal->Data ||  Pal->DriverHandle );
	assert( !Pal->Data || !Pal->DriverHandle );

	assert( (Pal->Driver && Pal->DriverHandle) ||
		(! Pal->Driver && ! Pal->DriverHandle) );

	assert( Pal->RefCount >= 1 && Pal->Size >= 1 );
	assert( Pal->Format > JE_PIXELFORMAT_NO_DATA && Pal->Format < JE_PIXELFORMAT_COUNT );

return JE_TRUE;
}

#ifdef _DEBUG
JETAPI uint32 JETCC jeBitmap_Debug_GetCount(void)
{
	assert(  _Bitmap_Debug_ActiveRefs >=  _Bitmap_Debug_ActiveCount );

//	Log_Printf("jeBitmap_Debug_GetCount : Refs = %d\n",_Bitmap_Debug_ActiveRefs);

//	jeBitmap_Gamma_Debug_Report();

	if (  _Bitmap_Debug_ActiveCount == 0 )
		assert(_Bitmap_Debug_ActiveRefs == 0 );

	return _Bitmap_Debug_ActiveCount;
}
#endif

/*}{ ******************** EOF **************************/

JETAPI jeBoolean JETCC jeBitmap_GetAverageColor(const jeBitmap *Bmp,int32 *pR,int32 *pG,int32 *pB)
{

	if ( ! Bmp->HasAverageColor )
	{
	int32 bpp,x,y,w,h,xtra,dock;
	jePixelFormat Format;
	uint8 * ptr;
	uint32 R,G,B,A,Rt,Gt,Bt,cnt,ck;

		//{} Rt == Rtotal , probably won't overflow; we can handle a 4096x4095 solid-white image

		if ( Bmp->DriverHandle && Bmp->DriverDataChanged )
		{
			// must use the driver bits
			if ( ! jeBitmap_Update_DriverToSystem((jeBitmap *)Bmp) )
			{
				jeErrorLog_AddString(-1,"Bitmap_AverageColor : DriverToSystem failed!",NULL);
				return JE_FALSE;
			}
		}

		Format = Bmp->Info.Format;
		bpp = jePixelFormat_BytesPerPel(Format);
		ptr = (uint8*)Bmp->Data[0];	

		if ( ! ptr || bpp < 1 )
		{
			jeErrorLog_AddString(-1,"Bitmap_AverageColor : no data!",NULL);
			return JE_FALSE;
		}

		w = Bmp->Info.Width;
		h = Bmp->Info.Height;
		xtra = (Bmp->Info.Stride - w)*bpp;
		ck = Bmp->Info.ColorKey;
		dock = Bmp->Info.HasColorKey;

		Rt = Gt = Bt = cnt = 0;

		if ( jePixelFormat_HasPalette(Format) )
		{
			// <> Blech!
			jeErrorLog_AddString(-1,"Bitmap_AverageColor : doesn't support palettized yet!",NULL);
			#pragma message("Bitmap_AverageColor : doesn't support palettized yet!")
			return JE_FALSE;
		}
		else
		{
		const jePixelFormat_Operations * ops;
		jePixelFormat_ColorGetter GetColor;
		jePixelFormat_PixelGetter GetPixel;
		jePixelFormat_Decomposer Decomposer;

			assert( jePixelFormat_IsRaw(Format) );

			ops = jePixelFormat_GetOperations(Format);
			GetColor = ops->GetColor;
			GetPixel = ops->GetPixel;
			Decomposer = ops->DecomposePixel;

			if ( dock )
			{
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
					uint32 Pixel;
						Pixel = GetPixel(&ptr);
						if ( Pixel != ck )
						{
							Decomposer(Pixel,(int32 *)&R,(int32 *)&G,(int32 *)&B,(int32 *)&A);
							Rt += R; Gt += G; Bt += B;
							cnt ++;
						}
					}
					ptr += xtra;
				}
			}
			else
			{
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						GetColor(&ptr,(int32 *)&R,(int32 *)&G,(int32 *)&B,(int32 *)&A);
						if ( A > 80 )
						{
							Rt += R; Gt += G; Bt += B;
							cnt ++;
						}
					}
					ptr += xtra;
				}
			}
		}

		((jeBitmap *)Bmp)->AverageR = (Rt + (cnt>>1)) / cnt;
		((jeBitmap *)Bmp)->AverageG = (Gt + (cnt>>1)) / cnt;
		((jeBitmap *)Bmp)->AverageB = (Bt + (cnt>>1)) / cnt;
		((jeBitmap *)Bmp)->HasAverageColor = JE_TRUE;
	}

	if ( pR ) *pR = Bmp->AverageR;
	if ( pG ) *pG = Bmp->AverageG;
	if ( pB ) *pB = Bmp->AverageB;

return JE_TRUE;
}
