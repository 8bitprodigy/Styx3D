/****************************************************************************************/
/*  THandle.cpp                                                                         */
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
#include <Windows.h>
#include <DDraw.h>
#include <D3D.h>

#include "d3d_THandle.h"
#include "BaseType.h"
#include "D3DDrv.h"
#include "DCommon.h"
#include "D3DCache.h"
#include "D3D_Main.h"
#include "d3d_PCache.h"
#include "D3d_FX.h"

#include "d3d_TPage.h"
#include "d3d_scene.h"

#define D3D_MANAGE_TEXTURES

#define USE_ONE_CACHE	//@@

//#define MAX_TEXTURE_HANDLES					(8192)	//@@ sweet god that's alot of static mem!
// BEGIN - Whitescreen issue - Jeff
#define MAX_TEXTURE_HANDLES					(20000)	//@@ sweet god even more static mem than before!
// END - Whitescreen issue - Jeff

	/*
		we need to dynamically realloc this!
		use MemPool for MipDatas ?
	*/		

#define TSTAJE_TEX			(0)
#define TSTAJE_LMAP			(1)

#ifndef USE_ONE_CACHE
#define TEXTURE_CACHE_PERCENT				(0.75f)
#define LMAP_CACHE_PERCENT					(0.25f)
#endif

//============================================================================================
// statics & globals
//============================================================================================

static jeTexture	TextureHandles[MAX_TEXTURE_HANDLES];
static int					NumTextureHandles = 0;

static DDMemMgr				*MemMgr = nullptr;
static DDMemMgr_Partition	*Partition0 = nullptr;

static D3DCache				*TextureCache = nullptr;

#ifndef USE_ONE_CACHE
static D3DCache				*LMapCache = nullptr;
static DDMemMgr_Partition	*Partition1 = nullptr;
#endif

TPage_Mgr					*TPageMgr = nullptr;		// externed

static DDSURFACEDESC2		CurrentSurfDesc;

THandle_MipData				SystemToVideo[MAX_LMAP_LOG_SIZE];	//externed by pcache

//============================================================================================
//	FreeAllcaches
//============================================================================================
void FreeAllCaches(void)
{

#ifndef USE_ONE_CACHE
	if (LMapCache)
		D3DCache_Destroy(LMapCache);
	LMapCache = nullptr;

	if (Partition1)
		DDMemMgr_PartitionDestroy(Partition1);
		
	Partition1 = nullptr;
#endif

	if (TextureCache)
		D3DCache_Destroy(TextureCache);

	TextureCache = nullptr;

	if (Partition0)
		DDMemMgr_PartitionDestroy(Partition0);
	Partition0 = nullptr;

	if (MemMgr)
		DDMemMgr_Destroy(MemMgr);
	MemMgr = nullptr;
		
#ifdef USE_TPAGES
	if (TPageMgr)
	{
		TPage_MgrDestroy(&TPageMgr);
		TPageMgr = nullptr;
	}
#endif

}

//============================================================================================
//	FindTextureHandle
//============================================================================================
jeTexture *FindTextureHandle(void)
{
	int32				i{};
	jeTexture* pHandle{};

	pHandle = TextureHandles;

	for (i=0; i< MAX_TEXTURE_HANDLES; i++, pHandle++)
	{
		if (!pHandle->Active)
		{
			memset(pHandle, 0, sizeof(jeTexture));
			pHandle->Active = 1;
			NumTextureHandles = max(NumTextureHandles, i+1);
			return pHandle;
		}
	}

	SetLastDrvError(DRV_ERROR_GENERIC, "D3D_FindTextureHandle:  No more handles left.\n");

	return nullptr;
}

//============================================================================================
//	FreeAllTextureHandles
//============================================================================================
jeBoolean FreeAllTextureHandles(void)
{
	int32				i{};
	jeTexture* pHandle{};

	pHandle = TextureHandles;

	for (i=0; i< NumTextureHandles; i++, pHandle++)
	{
		if (!pHandle->Active)
			continue;

		if (!THandle_Destroy(pHandle))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//============================================================================================
//============================================================================================
jeBoolean THandle_Startup(void)
{
	NumTextureHandles = 0;

	// Create the main memory manager
	MemMgr = DDMemMgr_Create(D3DInfo.VidMemFree);

	if (!MemMgr)
		goto ExitWithError;

#ifdef USE_ONE_CACHE

	// Create partition 0
	Partition0 = DDMemMgr_PartitionCreate(MemMgr, DDMemMgr_GetFreeMem(MemMgr) );

#else

	// Create partition 0
	Partition0 = DDMemMgr_PartitionCreate(MemMgr, (uint32)((float)DDMemMgr_GetFreeMem(MemMgr)*TEXTURE_CACHE_PERCENT));

#endif

	if (!Partition0)
		goto ExitWithError;

	// Create the texture cache from partition 0
	TextureCache = D3DCache_Create("Main Texture Cache", D3DInfo.lpDD, Partition0, D3DInfo.CanDoMultiTexture);

	if (!TextureCache)
		goto ExitWithError;

#ifndef USE_ONE_CACHE
	// Create partition 1
	Partition1 = DDMemMgr_PartitionCreate(MemMgr, DDMemMgr_GetFreeMem(MemMgr));

	if (!Partition1)
		goto ExitWithError;

	// Create the lmap cache from partition 1
	LMapCache = D3DCache_Create("Lightmap Cache", D3DInfo.lpDD, Partition1, D3DInfo.CanDoMultiTexture);

	if (!LMapCache)
		goto ExitWithError;
#endif

	// Create all the system to video surfaces (for lmaps)
	if (!CreateSystemToVideoSurfaces())
		goto ExitWithError;

	#ifdef USE_TPAGES
		TPageMgr = TPage_MgrCreate(D3DInfo.lpDD, &D3DInfo.ddTexFormat, 512);
		if (!TPageMgr)
			goto ExitWithError;
	#endif

	return JE_TRUE;

	ExitWithError:
	{
		THandle_Shutdown();
		return JE_FALSE;
	}
}

//============================================================================================
//============================================================================================
void THandle_Shutdown(void)
{
	FreeAllTextureHandles();
	FreeAllCaches();
	DestroySystemToVideoSurfaces();
}

jeBoolean THandle_UpdateCaches(void)
{
	D3DCache_Update(TextureCache);

#ifndef USE_ONE_CACHE
	D3DCache_Update(LMapCache);
#endif

	return JE_TRUE;
}

BOOL THandle_EvictAll(void) // called by Main_RestoreAll
{
	// Force an update in the cache system
	if (TextureCache)
		if (!D3DCache_EvictAllSurfaces(TextureCache))
			return FALSE;

#ifndef USE_ONE_CACHE
	if (LMapCache)
		if (!D3DCache_EvictAllSurfaces(LMapCache))
			return FALSE;
#endif

	return TRUE;
}

//============================================================================================
//	Create3DTHandle
//============================================================================================
jeTexture *Create3DTHandle(jeTexture *THandle, int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat)
{
	int32				Size{}, i{};

	assert(NumMipLevels < THANDLE_MAX_MIP_LEVELS);

	// Store width/height info
	THandle->Width = (uint16)Width;
	THandle->Height = (uint16)Height;
	THandle->NumMipLevels = (uint8)NumMipLevels;
	THandle->Log = (uint8)GetLog(Width, Height);
	THandle->Stride = (1<<THandle->Log);

	// Create the surfaces to hold all the mips
	THandle->MipData = (THandle_MipData*)malloc(sizeof(THandle_MipData)*NumMipLevels);
	memset(THandle->MipData, 0, sizeof(THandle_MipData)*NumMipLevels);
	
	if (!THandle->MipData)
	{
		THandle_Destroy(THandle);
		return nullptr;
	}

	Size = 1<<THandle->Log;

	// Create all the surfaces for each mip level
	for (i=0; i< NumMipLevels; i++)
	{
		int32	Stage{};

		if (!THandle_CreateSurfaces(&THandle->MipData[i], Size, Size, &CurrentSurfDesc, JE_FALSE, 0 , true))
		{
			THandle_Destroy(THandle);
			return nullptr;
		}

		// get a cache type for this surface since it is a 3d surface, and will need to be cached on the video card
		//THandle->MipData[i].CacheType = D3DCache_TypeCreate(TextureCache, Size, Size, NumMipLevels, &CurrentSurfDesc);
		// We can use 1 miplevel for the type, since we are createing types for each miplevel...
		if (D3DInfo.CanDoMultiTexture)
			Stage = TSTAJE_TEX;
		else
			Stage = 0;

		THandle->MipData[i].CacheType = D3DCache_TypeCreate(TextureCache, Size, Size, 1, Stage, &CurrentSurfDesc);

		if (!THandle->MipData[i].CacheType)
		{
			THandle_Destroy(THandle);
			return nullptr;
		}

		Size>>=1;
	}

	return THandle;
}

//============================================================================================
//	CreateLightmapTHandle
//============================================================================================
jeTexture *CreateLightmapTHandle(jeTexture *THandle, int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat)
{
	int32				Size{}, Stage{};

	assert(NumMipLevels < THANDLE_MAX_MIP_LEVELS);

	if (NumMipLevels != 1)
	{
		SetLastDrvError(DRV_ERROR_GENERIC, "D3D_CreateLightmapTHandle:  Invalid number of miplevels.\n");
		return nullptr;
	}
	
	// Save some info about the lightmap
	THandle->Width = (uint16)Width;
	THandle->Height = (uint16)Height;
	THandle->NumMipLevels = (uint8)NumMipLevels;
	THandle->Log = (uint8)GetLog(Width, Height);
	THandle->Stride = 1<<THandle->Log;

	assert(THandle->Log < MAX_LMAP_LOG_SIZE);

	Size = 1<<THandle->Log;

	THandle->MipData = (THandle_MipData*)malloc(sizeof(THandle_MipData)*NumMipLevels);
	memset(THandle->MipData, 0, sizeof(THandle_MipData)*NumMipLevels);

	// Force an update the first time used
	THandle->MipData[0].Flags = THANDLE_UPDATE;

#ifdef D3D_MANAGE_TEXTURES
	#ifndef USE_TPAGES
	{
		int32		Stage{};

		if (D3DInfo.CanDoMultiTexture)
			Stage = 1;
		else
			Stage = 0;

		if (!THandle_CreateSurfaces(&THandle->MipData[0], Size, Size, &CurrentSurfDesc, JE_FALSE, Stage , true))
		{
			THandle_Destroy(THandle);
			return nullptr;
		}
	
		//D3DSetTexture(0, THandle->MipData[0].Surface);
	}
	#endif
#endif

	if (D3DInfo.CanDoMultiTexture)
		Stage = TSTAJE_LMAP;
	else
		Stage = 0;

#ifdef USE_ONE_CACHE
	THandle->MipData[0].CacheType = D3DCache_TypeCreate(TextureCache, Size, Size, NumMipLevels, Stage, &CurrentSurfDesc);
#else
	THandle->MipData[0].CacheType = D3DCache_TypeCreate(LMapCache, Size, Size, NumMipLevels, Stage, &CurrentSurfDesc);
#endif

	if (!THandle->MipData[0].CacheType)
	{
		THandle_Destroy(THandle);
		return nullptr;
	}

	return THandle;
}

//============================================================================================
//	Create2DTHandle
//============================================================================================
jeTexture *Create2DTHandle(jeTexture *THandle, int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat)
{
	assert(NumMipLevels < THANDLE_MAX_MIP_LEVELS);

	if (NumMipLevels != 1)
	{
		SetLastDrvError(DRV_ERROR_GENERIC, "D3D_Create2DTHandle:  Invalid number of miplevels.\n");
		return nullptr;
	}

	// Save some info about the lightmap
	THandle->Width = (uint16)Width;
	THandle->Height = (uint16)Height;
	THandle->NumMipLevels = (uint8)NumMipLevels;
	THandle->Log = (uint8)GetLog(Width, Height);
	//THandle->Stride = (uint16)Width;
	THandle->Stride = (uint16)( ( Width + 3 ) & (~3) ); // Fix by Stu, added by Incarnadine

	// Create the surfaces to hold all the mips
	THandle->MipData = (THandle_MipData*)malloc(sizeof(THandle_MipData)*NumMipLevels);
	memset(THandle->MipData, 0, sizeof(THandle_MipData)*NumMipLevels);
	
	if (!THandle->MipData)
	{
		THandle_Destroy(THandle);
		return nullptr;
	}
	
	if (!THandle_CreateSurfaces(&THandle->MipData[0], Width, Height, &CurrentSurfDesc, JE_TRUE, 0 , false))
	{
		THandle_Destroy(THandle);
		return nullptr;
	}

	return THandle;
}

//============================================================================================
//	SetupCurrent3dDesc
//============================================================================================
jeBoolean SetupCurrent3dDesc(jePixelFormat PixelFormat)
{
	switch (PixelFormat)
	{
		case JE_PIXELFORMAT_16BIT_555_RGB:
		case JE_PIXELFORMAT_16BIT_565_RGB:
		{
			memcpy(&CurrentSurfDesc, &D3DInfo.ddTexFormat, sizeof(DDSURFACEDESC2));
			break;
		}
		case JE_PIXELFORMAT_16BIT_4444_ARGB:
		{
			memcpy(&CurrentSurfDesc, &D3DInfo.ddFourBitAlphaSurfFormat, sizeof(DDSURFACEDESC2));
			break;
		}
		case JE_PIXELFORMAT_16BIT_1555_ARGB:
		{
			memcpy(&CurrentSurfDesc, &D3DInfo.ddOneBitAlphaSurfFormat, sizeof(DDSURFACEDESC2));
			break;
		}

		default:
		{
			SetLastDrvError(DRV_ERROR_GENERIC, "SetupCurrent3dDesc:  Invalid pixel format.\n");
			return JE_FALSE;
		}
	}

	return JE_TRUE;
}
//============================================================================================
//	THandle_Create
//============================================================================================
jeTexture *DRIVERCC THandle_Create(int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat)
{
	jeTexture* THandle{};

	if (PixelFormat->Flags & RDRIVER_PF_3D)
	{
		// <> how to respond to creations that are too big
	#if 0 //{
		if ( Width > D3DInfo.MaxTextureWidth ||
			 Height > D3DInfo.MaxTextureHeight )
		{
			SetLastDrvError(DRV_ERROR_GENERIC, "D3DDRV:THandle_Create:  Size too big!\n");
			return nullptr;
		}
	#else //}{
		// <> just cap it!?
		// @@ we need to fix the World poly UV's now!
		//	they'll give us 0 -> 512, and we're 0 ->256 (for example)

		if ( Width  > D3DInfo.MaxTextureWidth  ) Width  = D3DInfo.MaxTextureWidth;
		if ( Height > D3DInfo.MaxTextureHeight ) Height = D3DInfo.MaxTextureHeight;
	#endif //}
	}

	THandle = FindTextureHandle();

	if (!THandle)
	{
		SetLastDrvError(DRV_ERROR_GENERIC, "D3DDRV:THandle_Create:  Out of texture handles.\n");
		return nullptr;
	}

	THandle->PixelFormat = *PixelFormat;

	if (PixelFormat->Flags & RDRIVER_PF_3D)
	{
		// Get the pixel format desc for this thandle
		if (!SetupCurrent3dDesc(PixelFormat->PixelFormat))
			return nullptr;

		if (!Create3DTHandle(THandle, Width, Height, NumMipLevels, PixelFormat))
			return nullptr;
	}
	else if (PixelFormat->Flags & RDRIVER_PF_LIGHTMAP)
	{
		// Get the pixel format desc for this thandle
		if (!SetupCurrent3dDesc(PixelFormat->PixelFormat))
			return nullptr;

		if (!CreateLightmapTHandle(THandle, Width, Height, NumMipLevels, PixelFormat))
			return nullptr;
	}
	else if (PixelFormat->Flags & RDRIVER_PF_2D)
	{
		// 2d surfaces are always this format for now
		memcpy(&CurrentSurfDesc, &D3DInfo.ddSurfFormat, sizeof(DDSURFACEDESC2));

		if (!Create2DTHandle(THandle, Width, Height, NumMipLevels, PixelFormat))
			return nullptr;
	}

	return THandle;
}

//============================================================================================
//	THandle_MipDataGetSlot
//============================================================================================
D3DCache_Slot * THandle_MipDataGetSlot(THandle_MipData * MipData)
{
	if ( ! MipData->Slot )
		return nullptr;

	if ( D3DCache_SlotGetMipData(MipData->Slot) != MipData )
		return nullptr;

return MipData->Slot;
}

//============================================================================================
//	THandle_Destroy
//============================================================================================
jeBoolean DRIVERCC THandle_Destroy(jeTexture *THandle)
{
	int32		i{};

	assert(THandle);
//	assert(THandle->Active);
	if ( ! THandle->Active ) // <> !! Lightmaps call _Destroy on THandles that are trashed!
		return JE_TRUE;

	for (i=0; i< THandle->NumMipLevels; i++)
	{
		D3DCache_Slot* pSlot{};

		assert(THandle->MipData);

		if ( pSlot = THandle_MipDataGetSlot(&(THandle->MipData[i])) )
		{
			D3DCache_SlotSetMipData(pSlot,nullptr);
		}
		THandle->MipData[i].Slot = nullptr;

		if (THandle->MipData[i].CacheType)
		{
			D3DCache_TypeDestroy(THandle->MipData[i].CacheType);
//			CacheNeedsUpdate = JE_TRUE; // @@ no need for update when you destroy
//				Cache_TypeDestroy does it for you
			THandle->MipData[i].CacheType = nullptr;
		}
		
		if (THandle->MipData[i].Surface)
		{
			THandle_DestroySurfaces(&THandle->MipData[i]);
			THandle->MipData[i].Surface = nullptr;
		}
	}

	if (THandle->MipData)
		free(THandle->MipData);

	memset(THandle, 0, sizeof(jeTexture));

	return JE_TRUE;
}

//=====================================================================================
//=====================================================================================
jeBoolean DRIVERCC THandle_Lock(jeTexture *THandle, int32 MipLevel, void **Bits)
{
	DDSURFACEDESC2		SurfDesc{};
	HRESULT				Result{};

	if ( MipLevel >= THandle->NumMipLevels )
		return JE_FALSE;
    if ( THandle->MipData[MipLevel].Flags & THANDLE_LOCKED )
		return JE_FALSE;

	// Lock the surface so it can be filled with the data
    memset(&SurfDesc, 0, sizeof(DDSURFACEDESC2));
    SurfDesc.dwSize = sizeof(DDSURFACEDESC2);

    Result = THandle->MipData[MipLevel].Surface->Lock(nullptr, &SurfDesc, DDLOCK_WAIT, nullptr);

    if (Result != DD_OK) 
	{
        return JE_FALSE;
    }

	THandle->MipData[MipLevel].Flags |= THANDLE_LOCKED;

	*Bits = (void*)SurfDesc.lpSurface;

	return JE_TRUE;
}

//=====================================================================================
//	THandle_UnLock
//=====================================================================================
jeBoolean DRIVERCC THandle_UnLock(jeTexture *THandle, int32 MipLevel)
{
	HRESULT				Result{};

    assert(MipLevel <= THandle->NumMipLevels);
    assert(THandle->MipData[MipLevel].Flags & THANDLE_LOCKED);

	// Unlock the surface
    Result = THandle->MipData[MipLevel].Surface->Unlock(nullptr);

    if (Result != DD_OK) 
	{
        return JE_FALSE;
    }

	THandle->MipData[MipLevel].Flags |= THANDLE_UPDATE;
	THandle->MipData[MipLevel].Flags &= ~THANDLE_LOCKED;

	return JE_TRUE;
}

#ifndef NDEBUG
#define DebugIf(a, b) if (a) b
#else
#define DebugIf(a, b)
#endif

//=====================================================================================
//	THandle_GetInfo
//=====================================================================================
jeBoolean DRIVERCC THandle_GetInfo(jeTexture *THandle, int32 MipLevel, jeTexture_Info *Info)
{
	assert(THandle);

	DebugIf (MipLevel > THandle->Log, return JE_FALSE);

	Info->Width = THandle->Width>>MipLevel;
	Info->Height = THandle->Height>>MipLevel;
	Info->Stride = THandle->Stride>>MipLevel;

	if (THandle->PixelFormat.Flags & RDRIVER_PF_CAN_DO_COLORKEY)
	{
		Info->Flags = RDRIVER_THANDLE_HAS_COLORKEY;
		Info->ColorKey = 1;
	}
	else
	{
		Info->Flags = 0;
		Info->ColorKey = 0;
	}

	Info->PixelFormat = THandle->PixelFormat;

	return JE_TRUE;
}

//=====================================================================================
//	CreateSystemToVideoSurfaces
//	System surfaces to copy from system to video
//=====================================================================================
jeBoolean CreateSystemToVideoSurfaces(void)
{
	int32				i{};
	DDSURFACEDESC2		SurfDesc{};

	memcpy(&SurfDesc, &D3DInfo.ddTexFormat, sizeof(DDSURFACEDESC2));

	for (i=0; i<MAX_LMAP_LOG_SIZE; i++)
	{
		int32		Size{};

		Size = 1<<i;
		
		if (!THandle_CreateSurfaces(&SystemToVideo[i], Size, Size, &SurfDesc, JE_FALSE, 1 , true))
		{
			DestroySystemToVideoSurfaces();
			return JE_FALSE;
		}
	}

	return JE_TRUE;
}

//=====================================================================================
//	DestroySystemToVideoSurfaces
//=====================================================================================
void DestroySystemToVideoSurfaces(void)
{
	int32				i{};

	for (i=0; i<MAX_LMAP_LOG_SIZE; i++)
		THandle_DestroySurfaces(&SystemToVideo[i]);
}

//=====================================================================================
//	THandle_CreateSurfaces
//=====================================================================================
jeBoolean THandle_CreateSurfaces(THandle_MipData *Surf, int32 Width, int32 Height, DDSURFACEDESC2 *SurfDesc, jeBoolean ColorKey, int32 Stage , jeBoolean autoResize)
{
	LPDIRECTDRAWSURFACE7 Surface{};
	DDSURFACEDESC2		ddsd{};
	HRESULT				Hr{};
	D3DDEVICEDESC7* pddDesc = &D3DInfo.Drivers[D3DInfo.CurrentDriver].Desc;

	memcpy(&ddsd, SurfDesc, sizeof(DDSURFACEDESC2));

	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;

	// Why do we use D3D_MANAGE_TEXTURES?  This can be determined at runtime with:
/*
    // Turn on texture management for hardware devices
    if(pddDesc->deviceGUID == IID_IDirect3DHALDevice )
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    else if(pddDesc->deviceGUID == IID_IDirect3DTnLHalDevice )
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    else
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
*/
#ifdef D3D_MANAGE_TEXTURES
	//ColorKey = JE_TRUE;			// Force a colorkey on system surfaces since we are letting D3D do our cacheing...

	ddsd.ddsCaps.dwCaps = 0;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_HINTDYNAMIC;
#else
	ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
	ddsd.ddsCaps.dwCaps2 = 0;
#endif

	ddsd.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;
	ddsd.ddsCaps.dwCaps2 |= DDSCAPS2_HINTDYNAMIC;
	ddsd.ddsCaps.dwCaps3 = 0;
	ddsd.ddsCaps.dwCaps4 = 0;

	ddsd.dwWidth = Width;
	ddsd.dwHeight = Height;

if( autoResize )
{
    // Adjust width and height, if the driver requires it
    if (pddDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2) {
        ddsd.dwWidth = 1;
		while ((DWORD)Width > ddsd.dwWidth)
			ddsd.dwWidth <<= 1;
		ddsd.dwHeight = 1;
		while ((DWORD)Height > ddsd.dwHeight)
			ddsd.dwHeight <<= 1;
    }
    if (pddDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
        if (ddsd.dwWidth > ddsd.dwHeight)
			ddsd.dwHeight = ddsd.dwWidth;
        else
			ddsd.dwWidth = ddsd.dwHeight;
    }
}

	ddsd.dwTextureStage = Stage;

if( autoResize )
{
	// Fix by Stu, added by Incarnadine
	if ((ddsd.dwWidth > 4) && (ddsd.dwWidth & 0x3)) {
		ddsd.dwFlags |= DDSD_PITCH;
		ddsd.lPitch = (ddsd.dwWidth + 3) & (~3);
	} // end add
}

	Hr = D3DInfo.lpDD->CreateSurface(&ddsd, &Surface, nullptr);
	if (Hr != DD_OK) {
		return FALSE;
	}

	Surf->Surface = Surface;
	
	if (ColorKey)
	{
		DDCOLORKEY			CKey{};
		
		// Create the color key for this surface
		CKey.dwColorSpaceLowValue = 1;
		CKey.dwColorSpaceHighValue = 1;
		
		if (Surf->Surface->SetColorKey(DDCKEY_SRCBLT , &CKey) != DD_OK)
		{
			SetLastDrvError(DRV_ERROR_GENERIC, "THandle_CreateSurfaces: SetColorKey failed for texture.");
			Surf->Surface->Release();
			Surf->Surface = nullptr;
			return FALSE;
		}
	}
	return JE_TRUE;		// All good dude
}

//=====================================================================================
//	DestroySystemSurface
//=====================================================================================
void THandle_DestroySurfaces(THandle_MipData *Surf)
{
	if (Surf->Surface)
		Surf->Surface->Release();

	memset(Surf, 0, sizeof (THandle_MipData));
}

//=====================================================================================
//	THandle_CheckCache
//=====================================================================================
jeBoolean THandle_CheckCache(void)
{

#ifndef D3D_MANAGE_TEXTURES

	#ifndef USE_ONE_CACHE
		if (!D3DCache_CheckSlots(LMapCache))
		{
			D3DMain_Log("THandle_CheckCache:  D3DCache_CheckSlots failed for LMapCache.\n");
			return JE_FALSE;
		}
	#endif

	if (!D3DCache_CheckSlots(TextureCache))
	{
		D3DMain_Log("THandle_CheckCache:  D3DCache_CheckSlots failed for TextureCache.\n");
		return JE_FALSE;
	}

#endif

/*
	// this is maintained for us

	// Make sure no THandles reference any slots, because they mave have been moved around, or gotten destroyed...
	//  (Evict all textures in the cache)
	pTHandle = TextureHandles;

	for (i=0; i< NumTextureHandles; i++, pTHandle++)
	{
		int32		m;

		for (m=0; m< pTHandle->NumMipLevels; m++)
		{
			pTHandle->MipData[m].Slot = nullptr;
			pTHandle->MipData[m].UsedFrame = 0;
		}
	}
*/

return JE_TRUE;
}


//============================================================================================
// THandle_GetCacheTypeUse : 
//	go through the handles and see how many mips point to this Type
//	this is done for each CacheType in a D3DCache_Update
//	@@ this could be done much faster by walking the list in one pass to get all usages!
//============================================================================================
int32 THandle_GetCacheTypeUse(D3DCache_Type *Type)
{
	int32				i{};
	jeTexture* pHandle{};
int32			Uses = 0;

	pHandle = TextureHandles;

	for (i=0; i< NumTextureHandles; i++, pHandle++)
	{
		if ( pHandle->Active )
		{
		int m;
			for(m=0;m<pHandle->NumMipLevels;m++)
			{
				THandle_MipData* MipData{};
				MipData = pHandle->MipData + m;
				if ( MipData->CacheType == Type &&
					MipData->UsedFrame == Scene_CurrentFrame )
				{
					Uses ++;
				}	
			}
		}
	}
return Uses;
}

void THandle_GetCacheTypeUses(void)
{
	int32				i{};
	jeTexture* pHandle{};

	pHandle = TextureHandles;

	for (i=0; i< NumTextureHandles; i++, pHandle++) // @@ slow! walks a huge list!
	{
		if ( pHandle->Active )
		{
			int m{};
			for(m=0;m<pHandle->NumMipLevels;m++)
			{
			THandle_MipData *MipData;
				MipData = pHandle->MipData + m;
				if ( MipData->UsedFrame == Scene_CurrentFrame )
				{
					D3DCache_TypeUsed(MipData->CacheType);
				}
			}
		}
	}
}
