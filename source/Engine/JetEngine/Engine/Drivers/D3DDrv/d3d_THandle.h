/****************************************************************************************/
/*  THandle.h                                                                           */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: THandle manager for D3DDrv                                             */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
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
#ifndef D3D_THANDLE_H
#define D3D_THANDLE_H

#include <Windows.h>

#include "BaseType.h"
#include "DCommon.h"
#include "D3DCache.h"
#include "VFile.h"
#include "d3d_TPage.h"

//============================================================================================
//============================================================================================
#define THANDLE_MAX_MIP_LEVELS		(16)
//#define	MAX_LMAP_LOG_SIZE			(8)			// Max lightmap size in pixels will be 128x128
//#define	MAX_LMAP_LOG_SIZE			(7)			// Max lightmap size in pixels will be 64x64
#define	MAX_LMAP_LOG_SIZE			(6)			// Max lightmap size in pixels will be 32x32

typedef struct THandle_MipData
{
	LPDIRECTDRAWSURFACE7	Surface;			// The DD surface
	D3DCache_Type			*CacheType;
	D3DCache_Slot			*Slot;

	uint8					Flags;
	uint8					Pad[3];

	uint32					UsedFrame;
	uint32					KickedFrame;
} THandle_MipData;

// THandle flags
#define THANDLE_LOCKED					(1<<0)
#define THANDLE_UPDATE					(1<<1)

typedef struct jeTexture
{
	uint8					Active;
	uint16					Width;
	uint16					Height;
	uint16					Stride;
	uint8					NumMipLevels;
	uint8					Log;
	jeRDriver_PixelFormat	PixelFormat;

	THandle_MipData			*MipData;				// A mipdata per miplevel

#ifdef USE_TPAGES
	TPage_Block				*Block;
#endif

} jeTexture;

//extern D3DCache				*TextureCache;
//extern D3DCache				*LMapCache;

extern TPage_Mgr			*TPageMgr;

//============================================================================================
//============================================================================================
void FreeAllCaches(void);
jeTexture *FindTextureHandle(void);
jeBoolean FreeAllTextureHandles(void);
jeBoolean THandle_Startup(void);
void THandle_Shutdown(void);
jeTexture *Create3DTHandle(jeTexture *THandle, int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat);
jeTexture *CreateLightmapTHandle(jeTexture *THandle, int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat);
jeTexture *Create2DTHandle(jeTexture *THandle, int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat);
jeTexture *DRIVERCC THandle_Create(int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat);
jeTexture *DRIVERCC THandle_CreateFromFile(jeVFile *File);
jeBoolean DRIVERCC THandle_Destroy(jeTexture *THandle);
jeBoolean DRIVERCC THandle_Lock(jeTexture *THandle, int32 MipLevel, void **Bits);
jeBoolean DRIVERCC THandle_UnLock(jeTexture *THandle, int32 MipLevel);
jeBoolean DRIVERCC THandle_GetInfo(jeTexture *THandle, int32 MipLevel, jeTexture_Info *Info);
jeBoolean CreateSystemToVideoSurfaces(void);
void DestroySystemToVideoSurfaces(void);
jeBoolean THandle_CreateSurfaces(THandle_MipData *MipData, int32 Width, int32 Height, DDSURFACEDESC2 *SurfDesc, jeBoolean ColorKey, int32 Stage , jeBoolean autoResize);
void THandle_DestroySurfaces(THandle_MipData *MipData);
jeBoolean THandle_CheckCache(void);
jeBoolean THandle_UpdateCaches(void);

int32 THandle_GetCacheTypeUse(D3DCache_Type *Type);
void THandle_GetCacheTypeUses(void);

BOOL THandle_EvictAll(void);

D3DCache_Slot * THandle_MipDataGetSlot(THandle_MipData * MipData);

#endif
