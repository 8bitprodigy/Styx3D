/****************************************************************************************/
/*  PCache.cpp                                                                          */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: D3D poly cache                                                         */
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
#include <stdio.h>

#include "D3DCache.h"
#include "D3D_Fx.h"

#include "d3d_PCache.h"
#include "D3DDrv.h"
#include "d3d_THandle.h"
#include "D3D_Err.h"
#include "d3d_scene.h" //for currentframe

#define D3D_MANAGE_TEXTURES

#define SUPER_FLUSH

#define MAX_BATCHES					32				// 32 max batches

// This has nothing to do with D3D's max vertex count.  
// D3D's vertex count is for execute buffers.  
// DrawPrimitive maxes out at 65535
#define PCACHE_MAX_TOTAL_POLYS			1024
#define PCACHE_MAX_TOTAL_POLY_VERTS		4096

#define PCACHE_MAX_POLYS				256
#define PCACHE_MAX_POLY_VERTS			1024

typedef struct
{
	float		u;
	float		v;
	uint32		Color;
} PCache_TVert;

#define MAX_TEXTURE_STAGES			2				// Up to 2 tmu's (stages)

typedef struct
{
	float			u,v;
} PCache_UVSet;

// Verts we defined in the D3D flexible vertex format (FVF)
// This is a transformed and lit vertex definition, with up to 8 sets of uvs
typedef struct
{
	float			x,y,z;							// Screen x, y, z
	float			rhw;							// homogenous w
	DWORD			color;							// color
	PCache_UVSet	uv[MAX_TEXTURE_STAGES];			// uv sets for each stage
} PCache_Vert;

typedef struct
{
	jeRDriver_Layer		Layers[MAX_TEXTURE_STAGES];
	void				*LMapCBContext;				// LMapCBContext (Layer 1)
	uint32				Flags;						// Flags for this poly
	int32				MipLevel;
	uint32				SortKey;
	int32				FirstVert;
	int32				NumVerts;
} PCache_Poly;

typedef struct PCache_PolyList
{
	PCache_Poly		Polys[PCACHE_MAX_TOTAL_POLYS];

	PCache_Vert		Verts[PCACHE_MAX_TOTAL_POLY_VERTS];
	PCache_TVert	TVerts[PCACHE_MAX_TOTAL_POLY_VERTS];		// Original uv

	PCache_Poly		*SortedPolys[PCACHE_MAX_TOTAL_POLYS];

	int32			NumPolys;									// Total number of polys
	int32			NumVerts;									// Total number of verts

	int32			FirstPolyInBatch[MAX_BATCHES];				// First poly in batch
	int32			NumPolysInBatch[MAX_BATCHES];				// Num polys in each batch
	int32			NumVertsInBatch[MAX_BATCHES];				// Num polys in each batch

	jeBoolean		ZSorted;

} PCache_PolyList;

typedef enum 
{
	TEXTURE_ROP_TEXTURE_NONE,
	TEXTURE_ROP_GOURAUD_PASS,
	TEXTURE_ROP_TEXTURE_PASS,
	TEXTURE_ROP_TEXTURE_PASS2,
	TEXTURE_ROP_LIGHTMAP_PASS,
	TEXTURE_ROP_FOG_PASS,
	TEXTURE_ROP_SIMULTANEOUS_PASS,
	TEXTURE_ROP_SIMULTANEOUS_FOG_PASS,
} PCache_TextureRopMode;

static PCache_PolyList					PolyList;
static PCache_PolyList					ZSortedPolyList;

static int32							BatchCounter;
static int32							CurrentBatch;

#define PREP_VERTS_TEXTURE_PASS			1				// Prep verts as normal
#define PREP_VERTS_LIGHTMAP_PASS		2				// Prep verts as lightmaps
#define PREP_VERTS_SIMULTANEOUS_PASS	3				// Prep verts for a simultaneous pass

//====================================================================================
//	Local *global* variables
//====================================================================================
extern THandle_MipData			SystemToVideo[]; // for LMap blits; from THandle.cpp

DRV_CacheInfo					PCache_CacheInfo;
float							PCache_MipBias = 1.0f;

static PCache_TextureRopMode	PCache_CurrentTextureRopMode = TEXTURE_ROP_TEXTURE_NONE;

//====================================================================================
//	Local static functions prototypes
//====================================================================================
static void FillLMapSurface(jeTexture *THandle, jeRDriver_LMapCBInfo *Info, int32 LNum);
static void FillLMapSurface2(jeTexture *THandle, jeRDriver_LMapCBInfo *Info, int32 LNum);
static void LoadLMapFromSystem(jeTexture *THandle, jeRDriver_LMapCBInfo *Info, int32 LNum);
static BOOL IsKeyDown(int KeyCode);
static jeBoolean SetupMipData(THandle_MipData *MipData);
static jeBoolean SetupLMap(jeTexture *THandle, int32 Stage, jeRDriver_LMapCBInfo *Info, int32 LNum);
static jeBoolean SetupTexture(int32 Stage, jeTexture *THandle, int32 MipLevel);
static jeBoolean PrepPolyVerts(PCache_PolyList *PolyList, PCache_Poly *pPoly, int32 PrepMode, int32 Stage1, int32 Stage2);
static int PolyComp1(const void *a, const void *b);
static int PolyComp2(const void *a, const void *b);
static void SortBatch(PCache_PolyList *PolyList);
static jeBoolean FlushBatchSerialPass(PCache_PolyList *PolyList);
static jeBoolean FlushZSortedBatchSerialPass(PCache_PolyList *PolyList);
static jeBoolean FlushBatchSimultaneousPass(PCache_PolyList *PolyList);
static jeBoolean RenderGouraudPoly(PCache_PolyList *PolyList, PCache_Poly *pPoly);
static jeBoolean RenderTexturePoly(PCache_PolyList *PolyList, PCache_Poly *pPoly);
static jeBoolean RenderLightmapPoly(PCache_PolyList *PolyList, PCache_Poly *pPoly);
static jeBoolean RenderSimultaneousPoly(PCache_PolyList *PolyList, PCache_Poly *pPoly);
static void SetTextureRopMode(PCache_TextureRopMode Mode);
static int32 ChooseMipLevel(jeTLVertex *Verts,int32 NumVerts,int32 Width,int32 Height,int32 NumMipLevels);

//====================================================================================
// EndScene calls FlushPolys for us
//====================================================================================
void PCache_BeginScene(void)
{
	// Make sure we clear the cache info structure...
	memset(&PCache_CacheInfo, 0, sizeof(DRV_CacheInfo));
}

//====================================================================================
//	Initialization
//====================================================================================
void PCache_InitStaticsAndGlobals(void)
{
	memset(&PCache_CacheInfo, 0, sizeof(PCache_CacheInfo));

	PCache_MipBias = 1.0f;
	PCache_CurrentTextureRopMode = TEXTURE_ROP_TEXTURE_NONE;

	CurrentBatch = 0;
	BatchCounter = 0;

	memset(&PolyList, 0, sizeof(PolyList));
	memset(&ZSortedPolyList, 0, sizeof(ZSortedPolyList));
	
	PolyList.ZSorted = JE_FALSE;
	ZSortedPolyList.ZSorted = JE_TRUE;
}

//====================================================================================
//	PCache_InsertGouraudPoly
//====================================================================================
jeBoolean PCache_InsertGouraudPoly(jeTLVertex *Verts, int32 NumVerts, uint32 Flags)
{
	PCache_Poly* pCachePoly{};
	jeTLVertex* pVerts{};
	PCache_Vert* pD3DVerts{};
	int32				i{}, SAlpha{};
	PCache_PolyList* pPolyList{};
	uint32				SortKey{};

	assert(Verts);
	assert(NumVerts >= 3);

	if (Flags & JE_RENDER_FLAG_ALPHA)
	{
		float		CenterZ{};

		pVerts = Verts;

		// Find the center of the poly to use for sorting
		CenterZ = 0.0f;

		for (i=0; i< NumVerts; i++, pVerts++)
			CenterZ += pVerts->z;

		CenterZ /= NumVerts;
		
		SortKey = (uint32)(CenterZ*(1<<15));

		pPolyList = &ZSortedPolyList;
	}
	else
	{
	#ifdef ALTERNATE_SORTKEY
		if (Scene_CurrentFrame & 1) // @@ reverse order every other frame
			SortKey = 0xFFFFFFFF;
		else
	#endif
			SortKey = 0;

		pPolyList = &PolyList;
	}
	
	if (pPolyList->NumVerts+NumVerts >= PCACHE_MAX_POLY_VERTS)
	{
		// If the cache is full, we must flush it before going on...
		if (!PCache_FlushBatch(pPolyList))
			return JE_FALSE;
	}
	else if (PolyList.NumPolys+1 >= PCACHE_MAX_POLYS)
	{
		// If the cache is full, we must flush it before going on...
		if (!PCache_FlushBatch(pPolyList))
			return JE_FALSE;
	}

	// Store info about this poly in the cache
	pCachePoly = &pPolyList->Polys[pPolyList->NumPolys];

	pCachePoly->Layers->THandle = NULL;
	pCachePoly->LMapCBContext = NULL;
	pCachePoly->Flags = Flags;
	pCachePoly->FirstVert = pPolyList->NumVerts;
	pCachePoly->NumVerts = NumVerts;
	pCachePoly->MipLevel = -1;

	// Precompute the alpha value...
	SAlpha = ((int32)Verts->a)<<24;

	// Get a pointer to the original polys verts
	pVerts = Verts;

	// Get a pointer into the world verts
	pD3DVerts = &pPolyList->Verts[pPolyList->NumVerts];

	for (i=0; i< NumVerts; i++)
	{
		float	ZRecip{};

		ZRecip = 1/(pVerts->z);

		pD3DVerts->x = pVerts->x;
		pD3DVerts->y = pVerts->y;

		pD3DVerts->z = (1.0f - ZRecip);		// ZBUFFER
		pD3DVerts->rhw = ZRecip;

		pD3DVerts->color = SAlpha | ((int32)pVerts->r<<16) | ((int32)pVerts->g<<8) | (int32)pVerts->b;

		pVerts++;
		pD3DVerts++;
	}
	
	pPolyList->NumVertsInBatch[CurrentBatch] += NumVerts;
	pPolyList->NumPolysInBatch[CurrentBatch]++;

	pPolyList->NumVerts += NumVerts;
	pPolyList->NumPolys++;

	return JE_TRUE;
}

//====================================================================================
//	PCache_InsertWorldPoly
//====================================================================================
jeBoolean PCache_InsertWorldPoly(jeTLVertex *Verts, int32 NumVerts, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags)
{
	int32			Mip{};
	float			ZRecip{}, DrawScaleU{}, DrawScaleV{};
	PCache_Poly* pCachePoly{};
	jeTLVertex* pVerts{};
	PCache_TVert* pTVerts{};
	PCache_Vert* pD3DVerts{};
	int32			i{};
	uint32			Alpha{};
	PCache_PolyList* pPolyList{};
	uint32			SortKey{};

	assert(Verts);
	assert(NumVerts >= 3);
	assert(Layers);
	assert(NumLayers > 0);
	assert(NumLayers <= MAX_TEXTURE_STAGES);

	Mip = ChooseMipLevel(Verts, NumVerts, 1, 1, Layers->THandle->NumMipLevels);

	if (Layers->THandle->PixelFormat.Flags & RDRIVER_PF_ALPHA)
		Flags |= JE_RENDER_FLAG_COLORKEY;
		
	if (Flags & (JE_RENDER_FLAG_ALPHA|JE_RENDER_FLAG_COLORKEY))
	{
		float		CenterZ{};

		pVerts = Verts;

		// Find the center of the poly to use for sorting
		CenterZ = 0.0f;

		for (i=0; i< NumVerts; i++, pVerts++)
			CenterZ += pVerts->z;

		CenterZ /= NumVerts;
		
		SortKey = (uint32)(CenterZ*(1<<15));

		pPolyList = &ZSortedPolyList;
	}
	else
	{
		SortKey = ((uint32)Layers->THandle<<2) + Mip;

	#ifdef ALTERNATE_SORTKEY
		if ( Scene_CurrentFrame & 1 ) // @@ reverse order every other frame
			SortKey = 0xFFFFFFFF - SortKey;
	#endif

		pPolyList = &PolyList;
	}
	
	if (pPolyList->NumVerts + NumVerts >= PCACHE_MAX_POLY_VERTS)
	{
		// If the cache is full, we must flush it before going on...
		if (!PCache_FlushBatch(pPolyList))
			return JE_FALSE;
	}
	else if (PolyList.NumPolys+1 >= PCACHE_MAX_POLYS)
	{
		// If the cache is full, we must flush it before going on...
		if (!PCache_FlushBatch(pPolyList))
			return JE_FALSE;
	}

	DrawScaleU = 1.0f / Layers->ScaleU;
	DrawScaleV = 1.0f / Layers->ScaleV;

	// Stuff the poly in the cache for later use
	pCachePoly = &pPolyList->Polys[pPolyList->NumPolys];

	pCachePoly->Layers[0] = Layers[0];

	if (NumLayers > 1)
		pCachePoly->Layers[1] = Layers[1];

	pCachePoly->LMapCBContext = LMapCBContext;
	pCachePoly->Flags = Flags;
	pCachePoly->FirstVert = pPolyList->NumVerts;
	pCachePoly->NumVerts = NumVerts;
	pCachePoly->MipLevel = Mip;

	pCachePoly->SortKey = SortKey;

	// Get a pointer into the world verts
	pD3DVerts = &pPolyList->Verts[pPolyList->NumVerts];
	pTVerts = &pPolyList->TVerts[pPolyList->NumVerts];

	// Get a pointer to the verts
	pVerts = Verts;

	// Setup alpha
	if (Flags & JE_RENDER_FLAG_ALPHA)
		Alpha = (uint32)pVerts->a<<24;
	else
		Alpha = (uint32)(255<<24);

	for (i=0; i< NumVerts; i++)
	{
		float ZVal = (pVerts->z);
		ZRecip = 1.0f/ZVal;

		pD3DVerts->x = pVerts->x;
		pD3DVerts->y = pVerts->y;

		pD3DVerts->z = (1.0f - ZRecip);	// ZBUFFER
		pD3DVerts->rhw = ZRecip;
		
		// Store the uv's so the prep pass can use them...
		pTVerts->u = pVerts->u;
		pTVerts->v = pVerts->v;

		pTVerts->Color = Alpha | ((uint32)pVerts->r<<16) | ((uint32)pVerts->g<<8) | (uint32)pVerts->b;

		pTVerts++;
		pVerts++;	 
		pD3DVerts++;

	}

	if (!LMapCBContext)		// Prep the verts now so the actualy flush code can assume this
		PrepPolyVerts(pPolyList, pCachePoly, PREP_VERTS_TEXTURE_PASS, 0, 0);
	
	pPolyList->NumVertsInBatch[CurrentBatch] += NumVerts;
	pPolyList->NumPolysInBatch[CurrentBatch]++;

	pPolyList->NumVerts += NumVerts;
	pPolyList->NumPolys++;

	return JE_TRUE;
}

//====================================================================================
//	PCache_InsertMiscPoly
//====================================================================================
jeBoolean PCache_InsertMiscPoly(jeTLVertex *Verts, int32 NumVerts, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags)
{
	int32				Mip{};
	float				ZRecip{}, u{}, v{}, ScaleU{}, ScaleV{}, InvScale{};
	PCache_Poly* pCachePoly{};
	jeTLVertex* pVerts{};
	PCache_Vert* pD3DVerts{};
	int32				i{}, SAlpha{};
	jeTexture* THandle{};
	PCache_PolyList* pPolyList{};
	uint32				SortKey{};

	assert(Verts);
	assert(NumVerts >= 3);
	assert(Layers);
	assert(NumLayers == 1);

	THandle = Layers->THandle;
	assert(THandle);

	//	Get the mip level
	Mip = ChooseMipLevel(Verts,NumVerts, THandle->Width,THandle->Height,THandle->NumMipLevels);

	if (Layers->THandle->PixelFormat.Flags & RDRIVER_PF_ALPHA)
		Flags |= JE_RENDER_FLAG_COLORKEY;

	if (Flags & (JE_RENDER_FLAG_ALPHA|JE_RENDER_FLAG_COLORKEY))
	{
		float		CenterZ{};

		pVerts = Verts;

		// Find the center of the poly to use for sorting
		CenterZ = 0.0f;

		for (i=0; i< NumVerts; i++, pVerts++)
			CenterZ += pVerts->z;

		CenterZ /= NumVerts;
		
		SortKey = (uint32)(CenterZ*(1<<15));

		pPolyList = &ZSortedPolyList;
	}
	else
	{
		SortKey = ((uint32)THandle<<2) + Mip;

	#ifdef ALTERNATE_SORTKEY
		if ( Scene_CurrentFrame & 1 ) // @@ reverse order every other frame
			SortKey = 0xFFFFFFFF - SortKey;
	#endif

		pPolyList = &PolyList;
	}
	
	if (pPolyList->NumVerts+NumVerts >= PCACHE_MAX_POLY_VERTS)
	{
		// If the cache is full, we must flush it before going on...
		if (!PCache_FlushBatch(pPolyList))
			return JE_FALSE;
	}
	else if (PolyList.NumPolys+1 >= PCACHE_MAX_POLYS)
	{
		// If the cache is full, we must flush it before going on...
		if (!PCache_FlushBatch(pPolyList))
			return JE_FALSE;
	}

	// Store info about this poly in the cache
	pCachePoly = &pPolyList->Polys[pPolyList->NumPolys];

	pCachePoly->Layers->THandle = THandle;
	pCachePoly->LMapCBContext = NULL;
	pCachePoly->Flags = Flags;
	pCachePoly->FirstVert = pPolyList->NumVerts;
	pCachePoly->NumVerts = NumVerts;
	pCachePoly->MipLevel = Mip;
	pCachePoly->SortKey = SortKey;

	// Get scale value for vertices
	InvScale = 1.0f / (float)((1<<THandle->Log));

	// Convert them to take account that the vertices are allready from 0 to 1
	ScaleU = (float)THandle->Width * InvScale;
	ScaleV = (float)THandle->Height * InvScale;

	// Precompute the alpha value...
	SAlpha = ((int32)Verts->a)<<24;

	// Get a pointer to the original polys verts
	pVerts = Verts;

	// Get a pointer into the world verts
	pD3DVerts = &pPolyList->Verts[pPolyList->NumVerts];

	for (i=0; i< NumVerts; i++)
	{
		ZRecip = 1/(pVerts->z);

		pD3DVerts->x = pVerts->x;
		pD3DVerts->y = pVerts->y;

		pD3DVerts->z = (1.0f - ZRecip);		// ZBUFFER
		pD3DVerts->rhw = ZRecip;
		
		u = pVerts->u * ScaleU;
		v = pVerts->v * ScaleV;

		pD3DVerts->uv[0].u = u;
		pD3DVerts->uv[0].v = v;

		pD3DVerts->color = SAlpha | ((int32)pVerts->r<<16) | ((int32)pVerts->g<<8) | (int32)pVerts->b;

		pVerts++;
		pD3DVerts++;
	}
	
	pPolyList->NumVertsInBatch[CurrentBatch] += NumVerts;
	pPolyList->NumPolysInBatch[CurrentBatch]++;

	pPolyList->NumVerts += NumVerts;
	pPolyList->NumPolys++;

	return JE_TRUE;
}

//====================================================================================
//	PrepBatchBegin
//====================================================================================
static jeBoolean PrepBatchBegin(PCache_PolyList *PolyList)
{
	if (!CurrentBatch)		// Special case the first batch
		PolyList->FirstPolyInBatch[CurrentBatch] = 0;
	else
		PolyList->FirstPolyInBatch[CurrentBatch] = PolyList->NumPolysInBatch[CurrentBatch-1];

	PolyList->NumPolysInBatch[CurrentBatch] = 0;
	PolyList->NumVertsInBatch[CurrentBatch] = 0;

	return JE_TRUE;
}

//====================================================================================
//	PCache_BeginBatch
//====================================================================================
jeBoolean DRIVERCC PCache_BeginBatch(void)
{
	assert(BatchCounter >= 0);

	if (BatchCounter >= MAX_BATCHES)
		return JE_FALSE;

	BatchCounter++;
	CurrentBatch = BatchCounter - 1;

	if (!PrepBatchBegin(&PolyList))
		return JE_FALSE;

	if (!PrepBatchBegin(&ZSortedPolyList))
		return JE_FALSE;

	return JE_TRUE;
}

//====================================================================================
//	PCache_EndBatch
//====================================================================================
jeBoolean DRIVERCC PCache_EndBatch(void)
{
	assert(BatchCounter > 0);

	// Flush all polys for the current batch
	if (!PCache_FlushALLBatches())
		return JE_FALSE;

	BatchCounter--;
	CurrentBatch = BatchCounter - 1;

	return JE_TRUE;
}

//====================================================================================
//	PCache_FlushALLBatches
//====================================================================================
jeBoolean PCache_FlushALLBatches(void)
{
	if (!PCache_FlushBatch(&PolyList))
		return JE_FALSE;

	if (!PCache_FlushBatch(&ZSortedPolyList))
		return JE_FALSE;

	return JE_TRUE;
}

//====================================================================================
//	PCache_FlushBatch
//====================================================================================
jeBoolean PCache_FlushBatch(PCache_PolyList *PolyList)
{
	assert(BatchCounter > 0);

	if (!PolyList->NumPolysInBatch[CurrentBatch])
		return JE_TRUE;

	if (!THandle_CheckCache())
		return JE_FALSE;
	
	if (D3DInfo.CanDoMultiTexture)
	{
		FlushBatchSimultaneousPass(PolyList);
	}
	else
	{
		#if 1
			FlushBatchSerialPass(PolyList);
		#else
			if (PolyList->ZSorted)
				FlushZSortedBatchSerialPass(PolyList);
			else
				FlushBatchSerialPass(PolyList);
		#endif
	}

	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, 0);
	D3DInfo.lpD3DDevice->EndScene();
	D3DInfo.lpD3DDevice->BeginScene();

	return JE_TRUE;
}

//====================================================================================
//	PCache_Reset
//====================================================================================
void PCache_Reset(void)
{
	BatchCounter = 0;
	CurrentBatch = 0;

	PolyList.NumPolys = 0;
	PolyList.NumVerts = 0;
	ZSortedPolyList.NumPolys = 0;
	ZSortedPolyList.NumVerts = 0;
}

//====================================================================================
//	**** LOCAL STATIC FUNCTIONS *****
//====================================================================================

//=====================================================================================
//	FillLMapSurface
//=====================================================================================
static void FillLMapSurface(jeTexture *THandle, jeRDriver_LMapCBInfo *Info, int32 LNum)
{
	U16* pTempBits{};
	int32				w{}, h{}, Width{}, Height{}, Size{};
	U8* pBitPtr{};
	RGB_LUT* Lut{};
	int32				Extra{};

	pBitPtr = (U8*)Info->RGBLight[LNum];

	Width = THandle->Width;
	Height = THandle->Height;
	Size = 1<<THandle->Log;

	Lut = &D3DInfo.Lut1;

	THandle_Lock(THandle, 0, (void**)&pTempBits);

	Extra = Size - Width;

	for (h=0; h< Height; h++)
	{
		for (w=0; w< Width; w++)
		{
			U8	R, G, B;
			U16	Color;
			R = *pBitPtr++;
			G = *pBitPtr++;
			B =  *pBitPtr++;
			
			Color = (U16)(Lut->R[R] | Lut->G[G] | Lut->B[B]);

			*pTempBits++ = Color;
		}
		pTempBits += Extra;
	}

	THandle_UnLock(THandle, 0);
}

#ifdef USE_TPAGES
//=====================================================================================
//	FillLMapSurface
//=====================================================================================
static void FillLMapSurface2(jeTexture *THandle, jeRDriver_LMapCBInfo *Info, int32 LNum)
{
	U16					*pTempBits;
	int32				w, h, Width, Height, Stride;
	U8					*pBitPtr;
	RGB_LUT				*Lut;
	HRESULT				Result;
	const RECT			*pRect;
    DDSURFACEDESC2		SurfDesc;
	LPDIRECTDRAWSURFACE7	Surface;
	int32				Extra;

	pBitPtr = (U8*)Info->RGBLight[LNum];

	Width = THandle->Width;
	Height = THandle->>Height;

	Lut = &D3DInfo.Lut1;

    pRect = TPage_BlockGetRect(THandle->Block);
	Surface = TPage_BlockGetSurface(THandle->Block);

    memset(&SurfDesc, 0, sizeof(DDSURFACEDESC2));
    SurfDesc.dwSize = sizeof(DDSURFACEDESC2);

	Result = Surface->Lock((RECT*)pRect, &SurfDesc, DDLOCK_WAIT, NULL);

	assert(Result == DD_OK);

	Stride = SurfDesc.dwWidth;

	pTempBits = (U16*)SurfDesc.lpSurface;

	Extra = Stride - Width; 

	for (h=0; h< Height; h++)
	{
		for (w=0; w< Width; w++)
		{
			U8	R, G, B;
			U16	Color;
			R = *pBitPtr++;
			G = *pBitPtr++;
			B = *pBitPtr++;
			
			Color = (U16)(Lut->R[R] | Lut->G[G] | Lut->B[B]);

			*pTempBits++ = Color;
		}
		pTempBits += Extra;
	}

    Result = Surface->Unlock((RECT*)pRect);

	assert(Result == DD_OK);
}
#endif

//=====================================================================================
//	LoadLMapFromSystem
//=====================================================================================
static void LoadLMapFromSystem(jeTexture *THandle, jeRDriver_LMapCBInfo *Info, int32 LNum)
{
	U16* pTempBits{};
	int32				w{}, h{}, Width{}, Height{}, Size{}, Extra{};
	U8* pBitPtr{};
	LPDIRECTDRAWSURFACE7 Surface{};
	RGB_LUT* Lut{};
	DDSURFACEDESC2		ddsd{};
	HRESULT				ddrval{};

	pBitPtr = (U8*)Info->RGBLight[LNum];

	Width = THandle->Width;
	Height = THandle->Height;
	Size = 1<<THandle->Log;

	Extra = Size - Width;

	Lut = &D3DInfo.Lut1;

	Surface = SystemToVideo[THandle->Log].Surface;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddrval = Surface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

	assert(ddrval == DD_OK);

	pTempBits = (USHORT*)ddsd.lpSurface;

	for (h=0; h< Height; h++)
	{
		for (w=0; w< Width; w++)
		{
			U8	R, G, B;
			U16	Color;
			R = *pBitPtr++;
			G = *pBitPtr++;
			B =  *pBitPtr++;
			
			Color = (U16)(Lut->R[R] | Lut->G[G] | Lut->B[B]);
			
			*pTempBits++ = Color;
		}
		pTempBits += Extra;
	}

    ddrval = Surface->Unlock(NULL);
	assert(ddrval == DD_OK);
}

//=====================================================================================
//	IsKeyDown
//=====================================================================================
static BOOL IsKeyDown(int KeyCode)
{
	if (GetAsyncKeyState(KeyCode) & 0x8000)
		return TRUE;

	return FALSE;
}

extern uint32 Scene_CurrentFrame;

//=====================================================================================
//	SetupMipData
//=====================================================================================
static jeBoolean SetupMipData(THandle_MipData *MipData)
{
	if (!THandle_MipDataGetSlot(MipData) )
	{
		D3DCache_TypeMissed(MipData->CacheType,MipData->KickedFrame);

		MipData->Slot = D3DCache_TypeFindSlot(MipData->CacheType);
		assert(MipData->Slot);

		D3DCache_SlotSetMipData(MipData->Slot, MipData);

	#ifdef SUPER_FLUSH
		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, 0);
		
		// <> must flush before you upload new data into a slot
		//	(since some pending polys might ref that slot for a different texture)
		// @@ log to PCache Flushes
	#endif

		return JE_FALSE;
	}

	return JE_TRUE;
}

//=====================================================================================
//	SetupLMap
//=====================================================================================
static jeBoolean SetupLMap(jeTexture *THandle, int32 Stage, jeRDriver_LMapCBInfo *Info, int32 LNum)
{
#ifdef D3D_MANAGE_TEXTURES
	#ifdef USE_TPAGES
	{
		if (Info->Dynamic)
			THandle->Flags |= THANDLE_UPDATE;

		if (!THandle->Block)
		{
			THandle->Block = TPage_MgrFindOptimalBlock(TPageMgr, Scene_CurrentFrame);
			THandle->Flags |= THANDLE_UPDATE;
			TPage_BlockSetUserData(THandle->Block, THandle);
			assert(THandle->Block);
		}
		else if (TPage_BlockGetUserData(THandle->Block) != THandle)
		{
			// Find another block
			THandle->Block = TPage_MgrFindOptimalBlock(TPageMgr, Scene_CurrentFrame);
			assert(THandle->Block);

			THandle->Flags |= THANDLE_UPDATE;
			TPage_BlockSetUserData(THandle->Block, THandle);
		}

		if (THandle->Flags & THANDLE_UPDATE)
			FillLMapSurface2(Info, LNum);

		TPage_BlockSetLRU(THandle->Block, Scene_CurrentFrame);
		D3DSetTexture(Stage, TPage_BlockGetSurface(THandle->Block));
	
		if (Dynamic)
			THandle->Flags |= THANDLE_UPDATE;
		else
			THandle->Flags &= ~THANDLE_UPDATE;

		return JE_TRUE;
	}
	#else
	{
		if (Info->Dynamic)
			THandle->MipData[0].Flags |= THANDLE_UPDATE;

		if (THandle->MipData[0].Flags & THANDLE_UPDATE)
			FillLMapSurface(THandle, Info, LNum);

		D3DSetTexture(Stage, THandle->MipData[0].Surface);
	
		if (Info->Dynamic)
			THandle->MipData[0].Flags |= THANDLE_UPDATE;
		else
			THandle->MipData[0].Flags &= ~THANDLE_UPDATE;

		return JE_TRUE;
	}
	#endif

#else
	THandle_MipData		*MipData;

	MipData = &THandle->MipData[0];

	if (Info->Dynamic)
		MipData->Flags |= THANDLE_UPDATE;

	if (!SetupMipData(MipData))
	{
		MipData->Flags |= THANDLE_UPDATE;		// Force an upload
		//PCache_CacheInfo.LMapMisses++;
		//PCache_CacheInfo.LMapMissBytes += MipData->CacheType->Size;
	}

	MipData->UsedFrame = Scene_CurrentFrame;

	if (MipData->Flags & THANDLE_UPDATE)
	{
		HRESULT					Error;
		LPDIRECTDRAWSURFACE7	Surface;

		assert(MipData->Slot);
		
		Surface = D3DCache_SlotGetSurface(MipData->Slot);

		assert(Surface);
		assert(THandle->Log < MAX_LMAP_LOG_SIZE);
		assert(SystemToVideo[THandle->Log].Surface);

		LoadLMapFromSystem(THandle, Info, LNum);

		Error = Surface->BltFast(0, 0, SystemToVideo[THandle->Log].Surface, NULL, DDBLTFAST_WAIT);
		//Error = Surface->BltFast(0, 0, SystemToVideo[THandle->Log].Surface, NULL, 0);
		//Error = Surface->Blt(NULL, SystemToVideo[THandle->Log].Surface, NULL, DDBLT_WAIT, NULL);
		//Error = Surface->Blt(NULL, SystemToVideo[THandle->Log].Surface, NULL, 0, NULL);
		
		if (Error != DD_OK)
		{
			if(Error==DDERR_SURFACELOST)
			{
				if (!D3DMain_RestoreAllSurfaces())
					return JE_FALSE;
			}
			else
			{
				D3DMain_Log("SetupLMap: System to Video cache Blt failed.\n %s", D3DErrorToString(Error));
				return JE_FALSE;
			}
		}
	}

	if (Info->Dynamic)		// If it was dynmamic, force an update for one more frame
		MipData->Flags |= THANDLE_UPDATE;
	else
		MipData->Flags &= ~THANDLE_UPDATE;

	D3DCache_SlotSetLRU(MipData->Slot, Scene_CurrentFrame);
	D3DSetTexture(Stage, D3DCache_SlotGetSurface(MipData->Slot));

	return JE_TRUE;
#endif
}

//=====================================================================================
//	SetupTexture
//=====================================================================================
static jeBoolean SetupTexture(int32 Stage, jeTexture *THandle, int32 MipLevel)
{
#ifdef D3D_MANAGE_TEXTURES
	D3DSetTexture(Stage, THandle->MipData[MipLevel].Surface);
	return JE_TRUE;
#else
	THandle_MipData		*MipData;

	MipData = &THandle->MipData[MipLevel];

	if (!SetupMipData(MipData))
	{
		MipData->Flags |= THANDLE_UPDATE;		// Force an upload
		//PCache_CacheInfo.TexMisses++;
	}

	MipData->UsedFrame = Scene_CurrentFrame;

	if (MipData->Flags & THANDLE_UPDATE)
	{
		HRESULT					Error;
		LPDIRECTDRAWSURFACE7	Surface;

		Surface = D3DCache_SlotGetSurface(MipData->Slot);

		Error = Surface->BltFast(0, 0, MipData->Surface, NULL, DDBLTFAST_WAIT);

		if (Error != DD_OK)
		{
			if(Error==DDERR_SURFACELOST)
			{
				if (!D3DMain_RestoreAllSurfaces())
					return FALSE;
			}
			else
			{
				D3DMain_Log("SetupTexture: System to Video cache Blt failed.\n %s", D3DErrorToString(Error));
				return JE_FALSE;
			}
		}
	}

	MipData->Flags &= ~THANDLE_UPDATE;

	D3DCache_SlotSetLRU(MipData->Slot, Scene_CurrentFrame);
	D3DSetTexture(Stage, D3DCache_SlotGetSurface(MipData->Slot));

	return JE_TRUE;
#endif
}

//====================================================================================
//	PrepPolyVerts
//====================================================================================
static jeBoolean PrepPolyVerts(PCache_PolyList *PolyList, PCache_Poly *pPoly, int32 PrepMode, int32 Stage1, int32 Stage2)
{
	float			InvScale{}, u{}, v{};
	PCache_TVert* pTVerts{};
	PCache_Vert* pVerts{};
	float			ShiftU{}, ShiftV{}, ScaleU{}, ScaleV{};
	int32			j{};

	switch (PrepMode)
	{
		case PREP_VERTS_TEXTURE_PASS:
		{
			jeRDriver_Layer		*TexLayer;

			pTVerts = &PolyList->TVerts[pPoly->FirstVert];

			TexLayer = &pPoly->Layers[0];

			// Normalize UV's using Texture Size
			InvScale = 1.0f / (float)((1<<TexLayer->THandle->Log));

			ShiftU = TexLayer->ShiftU;
			ShiftV = TexLayer->ShiftV;

		 	ScaleU = (1.0f/TexLayer->ScaleU);
			ScaleV = (1.0f/TexLayer->ScaleV);

			pVerts = &PolyList->Verts[pPoly->FirstVert];
			
			for (j=0; j< pPoly->NumVerts; j++)
			{
				u = pTVerts->u*ScaleU + ShiftU;
				v = pTVerts->v*ScaleV + ShiftV;

				pVerts->uv[Stage1].u = u * InvScale;
				pVerts->uv[Stage1].v = v * InvScale;

				pVerts->color = pTVerts->Color;

				pTVerts++;
				pVerts++;
			}

			break;
		}

		case PREP_VERTS_LIGHTMAP_PASS:
		{
			jeRDriver_Layer* LMapLayer{};

			if (!pPoly->LMapCBContext)
				return JE_TRUE;

			LMapLayer = &pPoly->Layers[1];

			ShiftU = (float)-LMapLayer->ShiftU + 8.0f;
			ShiftV = (float)-LMapLayer->ShiftV + 8.0f;

			// 
			InvScale = 1.0f/(float)((1<<LMapLayer->THandle->Log)<<4);
				
			pTVerts = &PolyList->TVerts[pPoly->FirstVert];
			pVerts = &PolyList->Verts[pPoly->FirstVert];

			for (j=0; j< pPoly->NumVerts; j++)
			{
				u = pTVerts->u + ShiftU;
				v = pTVerts->v + ShiftV;

				pVerts->uv[Stage1].u = u * InvScale;
				pVerts->uv[Stage1].v = v * InvScale;

				pVerts->color = 0xffffffff;

				pTVerts++;
				pVerts++;
			}
			break;
		}

		case PREP_VERTS_SIMULTANEOUS_PASS:
		{
			float				InvScale2{}, ShiftU2{}, ShiftV2{};
			jeRDriver_Layer* LMapLayer{};
			jeRDriver_Layer* TexLayer{};

			assert(pPoly->LMapCBContext);

			pTVerts = &PolyList->TVerts[pPoly->FirstVert];

			// Set up shifts and scaled for texture uv's
			TexLayer = &pPoly->Layers[0];
			LMapLayer = &pPoly->Layers[1];

			// Normalize UV's using Texture Size
			InvScale = 1.0f / (float)((1<<TexLayer->THandle->Log));

			ShiftU = TexLayer->ShiftU;
			ShiftV = TexLayer->ShiftV;
		 	ScaleU = (1.0f/TexLayer->ScaleU);
			ScaleV = (1.0f/TexLayer->ScaleV);

			// Set up shifts and scaled for lightmap uv's
			ShiftU2 = (float)-LMapLayer->ShiftU + 8.0f;
			ShiftV2 = (float)-LMapLayer->ShiftV + 8.0f;

			InvScale2 = 1.0f/(float)((1<<LMapLayer->THandle->Log)<<4);

			pVerts = &PolyList->Verts[pPoly->FirstVert];

			for (j=0; j< pPoly->NumVerts; j++)
			{
				u = pTVerts->u*ScaleU+ShiftU;
				v = pTVerts->v*ScaleV+ShiftV;

				pVerts->uv[Stage1].u = u * InvScale;
				pVerts->uv[Stage1].v = v * InvScale;
			
				u = pTVerts->u + ShiftU2;
				v = pTVerts->v + ShiftV2;

				pVerts->uv[Stage2].u = u * InvScale2;
				pVerts->uv[Stage2].v = v * InvScale2;

				pVerts->color = pTVerts->Color;

				pTVerts++;
				pVerts++;
			}

			break;
		}

		default:
			return FALSE;
	}

	return TRUE;
}

//====================================================================================
//	PolyComp
//====================================================================================
static int PolyComp(const void *a, const void *b)
{
	int32			Id1{}, Id2{};

#if 0
	// Push all the lightmaps to the front of the list 
	if ((*(PCache_Poly**)a)->LMapCBContext && !(*(PCache_Poly**)b)->LMapCBContext)
		return 0;		// Lightmap already in front, leave it
	if (!(*(PCache_Poly**)a)->LMapCBContext && (*(PCache_Poly**)b)->LMapCBContext)
		return 1;		// Put Lightmap in front of non lightmap
#endif

	// Both are same type, so sort by key
	Id1 = (*(PCache_Poly**)a)->SortKey;
	Id2 = (*(PCache_Poly**)b)->SortKey;

	if ( Id1 == Id2)
		return 0;

	if (Id1 > Id2)
		return -1;

	return 1;
}

//====================================================================================
//	SortBatch
//====================================================================================
static void SortBatch(PCache_PolyList *PolyList)
{
	PCache_Poly* pPoly{};
	int32		i{}, NumPolys{};

	NumPolys = PolyList->NumPolysInBatch[CurrentBatch];

	pPoly = &PolyList->Polys[PolyList->FirstPolyInBatch[CurrentBatch]];

	for (i=0; i<NumPolys; i++, pPoly++)
		PolyList->SortedPolys[i] = pPoly;
	
	// Sort the polys
	qsort(&PolyList->SortedPolys, NumPolys, sizeof(PolyList->SortedPolys[0]), PolyComp);
}

#define TSTAJE_0			0
#define TSTAJE_1			1

//====================================================================================
//	FlushBatchSerialPass
//====================================================================================
static jeBoolean FlushBatchSerialPass(PCache_PolyList *PolyList)
{
	PCache_Poly* pPoly{}, * LightmapPolys[PCACHE_MAX_POLYS];
	int32			i{}, NumLightmapPolys{}, NumPolys{};

	NumPolys = PolyList->NumPolysInBatch[CurrentBatch];

	if (NumPolys <= 0)
	{
		assert(PolyList->NumVertsInBatch[CurrentBatch] == 0);
		return JE_TRUE;
	}

	NumLightmapPolys = 0;

	// Sort them
	SortBatch(PolyList);
	
	for (i=0; i< NumPolys; i++)
	{
		pPoly = PolyList->SortedPolys[i];
		
		if (pPoly->MipLevel == -1)
		{
			RenderGouraudPoly(PolyList, pPoly);
			continue;
		}

		RenderTexturePoly(PolyList, pPoly);

		if (pPoly->LMapCBContext)
			LightmapPolys[NumLightmapPolys++] = pPoly;
	}

	// We know tex wrapping is always going to be off for Layer 1 (Lightmap)
	D3DTexWrap(0, JE_FALSE);

	for (i=0; i< NumLightmapPolys; i++)
	{
		pPoly = LightmapPolys[i];
		
		RenderLightmapPoly(PolyList, pPoly);
	}

	PolyList->NumPolys -= NumPolys;
	PolyList->NumVerts -= PolyList->NumVertsInBatch[CurrentBatch];

	PolyList->NumPolysInBatch[CurrentBatch] = 0;
	PolyList->NumVertsInBatch[CurrentBatch] = 0;

	assert(!(PolyList->NumPolys > 0 && CurrentBatch == 0));
	assert(!(PolyList->NumVerts > 0 && CurrentBatch == 0));

	return JE_TRUE;
}

//====================================================================================
//	FlushZSortedBatchSerialPass
//====================================================================================
static jeBoolean FlushZSortedBatchSerialPass(PCache_PolyList *PolyList)
{
	PCache_Poly* pPoly{};
	int32			i{}, NumPolys{};

	NumPolys = PolyList->NumPolysInBatch[CurrentBatch];

	if (NumPolys <= 0)
	{
		assert(PolyList->NumVertsInBatch[CurrentBatch] == 0);
		return JE_TRUE;
	}

	// Sort them
	SortBatch(PolyList);
	
	for (i=0; i< NumPolys; i++)
	{
		pPoly = PolyList->SortedPolys[i];
		
		if (pPoly->MipLevel == -1)
		{
			RenderGouraudPoly(PolyList, pPoly);
			continue;
		}

		RenderTexturePoly(PolyList, pPoly);
		
		if (pPoly->LMapCBContext)
		{
			D3DTexWrap(0, JE_FALSE);
			RenderLightmapPoly(PolyList, pPoly);
		}
	}

	PolyList->NumPolys -= NumPolys;
	PolyList->NumVerts -= PolyList->NumVertsInBatch[CurrentBatch];

	PolyList->NumPolysInBatch[CurrentBatch] = 0;
	PolyList->NumVertsInBatch[CurrentBatch] = 0;

	assert(!(PolyList->NumPolys > 0 && CurrentBatch == 0));
	assert(!(PolyList->NumVerts > 0 && CurrentBatch == 0));

	return JE_TRUE;
}

//====================================================================================
//	FlushBatchSimultaneousPass
//====================================================================================
static jeBoolean FlushBatchSimultaneousPass(PCache_PolyList *PolyList)
{
	PCache_Poly* pPoly{};
	int32			i{}, NumPolys{};

	NumPolys = PolyList->NumPolysInBatch[CurrentBatch];

	if (NumPolys <= 0)
	{
		assert(PolyList->NumVertsInBatch[CurrentBatch] == 0);
		return JE_TRUE;
	}
	
	// Sort them
	SortBatch(PolyList);
	
	// We know tex wrapping is always going to be off for Layer 1 (Lightmap)
	D3DTexWrap(TSTAJE_1, JE_FALSE);

	for (i=0; i< NumPolys; i++)
	{
		pPoly = PolyList->SortedPolys[i];

		if (pPoly->MipLevel == -1)
		{
			RenderGouraudPoly(PolyList, pPoly);
			continue;
		}

		if (pPoly->LMapCBContext)
			RenderSimultaneousPoly(PolyList, pPoly);
		else
			RenderTexturePoly(PolyList, pPoly);
	}

	PolyList->NumPolys -= NumPolys;
	PolyList->NumVerts -= PolyList->NumVertsInBatch[CurrentBatch];

	PolyList->NumPolysInBatch[CurrentBatch] = 0;
	PolyList->NumVertsInBatch[CurrentBatch] = 0;

	assert(!(PolyList->NumPolys > 0 && CurrentBatch == 0));
	assert(!(PolyList->NumVerts > 0 && CurrentBatch == 0));

	return JE_TRUE;
}

//====================================================================================
//	RenderGouraudPoly
//====================================================================================
static jeBoolean RenderGouraudPoly(PCache_PolyList *PolyList, PCache_Poly *pPoly)
{
	SetTextureRopMode(TEXTURE_ROP_GOURAUD_PASS);

	if (pPoly->Flags & JE_RENDER_FLAG_NO_ZTEST)		// We are assuming that this is not going to change all that much
		D3DZEnable(FALSE);
	else
		D3DZEnable(TRUE);

	if (pPoly->Flags & JE_RENDER_FLAG_NO_ZWRITE)	// We are assuming that this is not going to change all that much
		D3DZWriteEnable(FALSE);	
	else
		D3DZWriteEnable(TRUE);

	// Draw the texture
	D3DTexturedPoly(&PolyList->Verts[pPoly->FirstVert], pPoly->NumVerts);

	return JE_TRUE;
}

//====================================================================================
//	RenderTexturePoly
//====================================================================================
static jeBoolean RenderTexturePoly(PCache_PolyList *PolyList, PCache_Poly *pPoly)
{
	if (pPoly->Flags & JE_RENDER_FLAG_COLORKEY)
		SetTextureRopMode(TEXTURE_ROP_TEXTURE_PASS2);
	else
		SetTextureRopMode(TEXTURE_ROP_TEXTURE_PASS);

	if (pPoly->Flags & JE_RENDER_FLAG_NO_ZTEST)		// We are assuming that this is not going to change all that much
		D3DZEnable(FALSE);
	else
		D3DZEnable(TRUE);

	if (pPoly->Flags & JE_RENDER_FLAG_NO_ZWRITE)	// We are assuming that this is not going to change all that much
		D3DZWriteEnable(FALSE);	
	else
		D3DZWriteEnable(TRUE);
									  
	if (pPoly->Flags & JE_RENDER_FLAG_CLAMP_UV)
		D3DTexWrap(0, FALSE);
	else
		D3DTexWrap(0, TRUE);

	// Prep the verts (only if it has a lightmap, other wise it's already preped...)
	if (pPoly->LMapCBContext)
		PrepPolyVerts(PolyList, pPoly, PREP_VERTS_TEXTURE_PASS, TSTAJE_0, TSTAJE_1);

	if (!SetupTexture(0, pPoly->Layers[0].THandle, pPoly->MipLevel))
		return JE_FALSE;				

	// Draw the texture
	D3DTexturedPoly(&PolyList->Verts[pPoly->FirstVert], pPoly->NumVerts);

	return JE_TRUE;
}

//====================================================================================
//	RenderLightmapPoly
//====================================================================================
static jeBoolean RenderLightmapPoly(PCache_PolyList *PolyList, PCache_Poly *pPoly)
{
	jeRDriver_LMapCBInfo	LMapCBInfo{};

	SetTextureRopMode(TEXTURE_ROP_LIGHTMAP_PASS);

	assert(pPoly->LMapCBContext);

	// Call the engine to set this sucker up, because it's visible...
	D3DDRV.SetupLightmap(&LMapCBInfo, pPoly->LMapCBContext);

	if (!SetupLMap(pPoly->Layers[1].THandle, 0, &LMapCBInfo, 0))
		return JE_FALSE;

	PrepPolyVerts(PolyList, pPoly, PREP_VERTS_LIGHTMAP_PASS, 0, 0);

	D3DTexturedPoly(&PolyList->Verts[pPoly->FirstVert], pPoly->NumVerts);
				
	if (LMapCBInfo.RGBLight[1])
	{
		D3DBlendFunc (D3DBLEND_ONE, D3DBLEND_ONE);				// Change to a fog state

		// For some reason, some cards can't upload data to the same texture twice, and have it take.
		// So we force Fog maps to use a different slot than the lightmap was using...
		pPoly->Layers[1].THandle->MipData[0].Slot = NULL;

		// Dynamic is 1, because fog is always dynamic
		LMapCBInfo.Dynamic = JE_TRUE;

		if (!SetupLMap(pPoly->Layers[1].THandle, 0, &LMapCBInfo, 1))
			return JE_FALSE;

		D3DTexturedPoly(&PolyList->Verts[pPoly->FirstVert], pPoly->NumVerts);
		
		D3DBlendFunc (D3DBLEND_DESTCOLOR, D3DBLEND_ZERO);		// Restore state
	}

	return JE_TRUE;
}

//====================================================================================
//	RenderSimultaneousPoly
//====================================================================================
static jeBoolean RenderSimultaneousPoly(PCache_PolyList *PolyList, PCache_Poly *pPoly)
{
	jeRDriver_LMapCBInfo	LMapCBInfo{};

	SetTextureRopMode(TEXTURE_ROP_SIMULTANEOUS_PASS);

	// Render Layer 0 (Texture Layer), and Layer 1(Lightmap) in one pass
	if (pPoly->Flags & JE_RENDER_FLAG_NO_ZTEST)		// We are assuming that this is not going to change all that much
		D3DZEnable(FALSE);
	else
		D3DZEnable(TRUE);

	if (pPoly->Flags & JE_RENDER_FLAG_NO_ZWRITE)	// We are assuming that this is not going to change all that much
		D3DZWriteEnable(FALSE);	
	else
		D3DZWriteEnable(TRUE);
									  
	if (pPoly->Flags & JE_RENDER_FLAG_CLAMP_UV)
		D3DTexWrap(0, FALSE);
	else
		D3DTexWrap(0, TRUE);

	if (!SetupTexture(TSTAJE_0, pPoly->Layers[0].THandle, pPoly->MipLevel))
		return JE_FALSE;				

	// Call the engine to set this sucker up, because it's visible...
	D3DDRV.SetupLightmap(&LMapCBInfo, pPoly->LMapCBContext);

	if (!SetupLMap(pPoly->Layers[1].THandle, TSTAJE_1, &LMapCBInfo, 0))
		return JE_FALSE;
			
	// Prep the verts for a lightmap and texture map
	PrepPolyVerts(PolyList, pPoly, PREP_VERTS_SIMULTANEOUS_PASS, TSTAJE_0, TSTAJE_1);

	// Draw the texture
	D3DTexturedPoly(&PolyList->Verts[pPoly->FirstVert], pPoly->NumVerts);
		
	// Render any fog maps
	if (LMapCBInfo.RGBLight[1])
	{
		SetTextureRopMode(TEXTURE_ROP_SIMULTANEOUS_FOG_PASS);

		// For some reason, some cards can't upload data to the same texture twice, and have it take.
		// So we force Fog maps to use a different slot other than what the lightmap was using...
		pPoly->Layers[1].THandle->MipData[0].Slot = NULL;

		LMapCBInfo.Dynamic = JE_TRUE;	// Dynamic is 1, because fog is always dynamic
			
		if (!SetupLMap(pPoly->Layers[1].THandle, TSTAJE_1, &LMapCBInfo,1))
			return JE_FALSE;

		D3DTexturedPoly(&PolyList->Verts[pPoly->FirstVert], pPoly->NumVerts);

		// Restore states to the last state before fog map
		SetTextureRopMode(TEXTURE_ROP_SIMULTANEOUS_PASS);
	}

	return JE_TRUE;
}
//====================================================================================
//	SetTextureRopMode
//====================================================================================
static void SetTextureRopMode(PCache_TextureRopMode Mode)
{
	if (PCache_CurrentTextureRopMode == Mode)
		return;		// Nothing changed

	switch (Mode)
	{
		case TEXTURE_ROP_GOURAUD_PASS:
		{
			D3DBlendFunc (D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
			
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			break;
		}

		case TEXTURE_ROP_TEXTURE_PASS:
		{
		#if 1
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

			if (D3DInfo.CanDoMultiTexture)
			{
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			}
		#else
			if (PCache_CurrentTextureRopMode != TEXTURE_ROP_SIMULTANEOUS_PASS)
			{
				// If we just came out of a TEXTURE_ROP_SIMULTANEOUS_PASS, then we can ignore these as they are already set
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
			}

			// Turn Stage 1 off
			if (D3DInfo.CanDoMultiTexture)
			{
			#if 0
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
			#else
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			#endif
			}
		#endif
			D3DBlendEnable(TRUE);
			D3DBlendFunc (D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
	
			break;
		}
	
		case TEXTURE_ROP_TEXTURE_PASS2:
		{
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

			if (D3DInfo.CanDoMultiTexture)
			{
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			}

			D3DBlendEnable(TRUE);
			D3DBlendFunc (D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
	
			break;
		}

		case TEXTURE_ROP_LIGHTMAP_PASS:
		{
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			
			D3DBlendEnable(TRUE);
			D3DBlendFunc (D3DBLEND_DESTCOLOR, D3DBLEND_ZERO);
		
			break;
		}
	
		case TEXTURE_ROP_SIMULTANEOUS_PASS:
		{
			assert(D3DInfo.CanDoMultiTexture == JE_TRUE);
				
			if (PCache_CurrentTextureRopMode == TEXTURE_ROP_SIMULTANEOUS_FOG_PASS)
			{
				// If we just came out of a TEXTURE_ROP_SIMULTANEOUS_FOG_PASS, then we only need to restore some settings, then bail
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	
				D3DBlendFunc (D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
	
				break;
			}
			
			//if (PCache_CurrentTextureRopMode != TEXTURE_ROP_TEXTURE_PASS)
			{
				// If we just came out of a TEXTURE_ROP_TEXTURE_PASS, then we can ignore these as they are already set
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
				D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
			}
										 
			D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
			D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
				
		#ifdef USE_2X_MODULATION
			if (D3DInfo.dwTextureOpCaps & D3DTOP_MODULATE2X)
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE2X);
			else
				D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		#else
			D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		#endif
		
			D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
			D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
			D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	
			D3DBlendEnable(TRUE);
			D3DBlendFunc (D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
			
			break;
		}
	
		case TEXTURE_ROP_SIMULTANEOUS_FOG_PASS:
		{
			assert(PCache_CurrentTextureRopMode == TEXTURE_ROP_SIMULTANEOUS_PASS);
	
			D3DBlendFunc (D3DBLEND_ONE, D3DBLEND_ONE);				// Change to a fog state
	
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
			D3DInfo.lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	
			D3DInfo.lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	
			break;
		}
	}

	PCache_CurrentTextureRopMode = Mode;
}	
	
//====================================================================================
//	ChooseMipLevel
//====================================================================================
static int32 ChooseMipLevel(jeTLVertex *Verts,int32 NumVerts,int32 Width,int32 Height,int32 NumMipLevels)
{
	int32		Mip{};
	float		du{}, dv{}, dx{}, dy{}, MipScale{};
	jeTLVertex* pVert0{}, * pVert1{};

	if (NumMipLevels == 1)
		return 0;				// Short-curcuit the whole process

#if 0 // accurate
	MipScale = 0.0f;

	for(int i=1;i<NumVerts;i++)
	{
		float	r;

		pVert0 = Verts[i-1];
		pVert1 = Verts[i];

		du = (pVert1.u - pVerts->u) * Width;
		dv = (pVert1.v - pVerts->v) * Height;
		dx =  pVert1.x - pVerts->x;
		dy =  pVert1.y - pVerts->y;

		r = ((du*du)+(dv*dv)) / ((dx*dx)+(dy*dy));

		if ( r > MipScale )
			MipScale = r;
	}
#else // faster
	pVert0 = Verts;
	pVert1 = Verts+1;

	du = (pVert1->u - pVert0->u) * Width;
	dv = (pVert1->v - pVert0->v) * Height;
	dx =  pVert1->x - pVert0->x;
	dy =  pVert1->y - pVert0->y;

	MipScale = ((du*du)+(dv*dv)) / ((dx*dx)+(dy*dy));
#endif 

	// MipScale is the (Texels / Pixels) squared

	// 1 - 4 - 16 - 64
	//	 4	 12	  40 

	// if mip bias is larger, we use smaller mips

	MipScale *= PCache_MipBias;

	if (MipScale <= 4.0f)	
		Mip = 0;
	else if (MipScale <= 12.0f )
		Mip = 1;
	else if (MipScale <= 48.0f )
		Mip = 2;
	else
		Mip = 3;

	if (Mip >= NumMipLevels)
		Mip = NumMipLevels-1;

	return Mip;
}
