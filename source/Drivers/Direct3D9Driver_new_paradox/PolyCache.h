/*!
	@file PolyCache.h
	@author Anthony Rufrano (paradoxnj)
	@brief Polygon Caching
*/
#ifndef POLY_CACHE_H
#define POLY_CACHE_H

#include <vector>
#include <list>

#include "Direct3D9Driver.h"
#include "D3D9TextureMgr.h"
#include "RenderStateManager.h"

typedef struct PolyVert
{
	float						x, y, z, rhw;
	D3DCOLOR					diffuse;
	float						u, v;
	float						lu, lv;
} PolyVert;

#define J3D_FVF					(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2)
#define J3D_HW_FVF				(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2)

typedef struct PolyCacheEntry
{
	int32						StartVertex;
	int32						NumVertices;

	jeTexture* Layers[MAX_LAYERS];
	int32						NumLayers;

	uint32						Flags;
} PolyCacheEntry;

typedef struct StaticBuffer
{
	jeBoolean					Active;

	IDirect3DVertexBuffer9* pVB;
	int32						NumVerts;

	jeRDriver_Layer* Layers;
	int32						NumLayers;

	uint32						Flags;
} StaticBuffer;

typedef std::vector<PolyCacheEntry>			CacheVector;

static const uint32 FILTER_TYPE_BILINEAR = 0;
static const uint32 FILTER_TYPE_ANISOTROPIC = 1;

class PolyCache
{
public:
	PolyCache();
	virtual ~PolyCache();

private:
	CacheVector								m_WorldCache;
	CacheVector								m_MiscCache;
	CacheVector								m_GouraudCache;
	CacheVector								m_StencilCache;

	int32									m_NumVerts;
	IDirect3DVertexBuffer9* m_pVB;

	std::vector<StaticBuffer>				m_StaticBuffers;

	IDirect3DDevice9* m_pDevice;

	jeTexture* m_TextureStages[MAX_LAYERS];
	jeBoolean								m_bOldAlphaEnable;
	D3DBLEND								m_OldSFunc, m_OldDFunc;
	jeBoolean								m_bOldTexWrap[MAX_LAYERS];
	uint32									m_OldFilterType;

	RenderStateManager* m_pRS;

public:
	jeBoolean					Initialize(IDirect3DDevice9* pDevice);
	void						Shutdown();

	uint32						AddStaticBuffer(jeHWVertex* Points, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, uint32 Flags);
	jeBoolean					RemoveStaticBuffer(uint32 id);
	jeBoolean					RenderStaticBuffer(uint32 id, int32 StartVertex, int32 NumPolys, jeXForm3d* XForm);

	jeBoolean					AddGouraudPoly(jeTLVertex* Pnts, int32 NumPoints, uint32 Flags);
	jeBoolean					AddMiscTexturePoly(jeTLVertex* Pnts, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, uint32 Flags);
	jeBoolean					AddWorldPoly(jeTLVertex* Pnts, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, void* LMapCBContext, uint32 Flags);

	jeBoolean					Flush();
	jeBoolean					FlushWorldCache();
	jeBoolean					FlushMiscCache();
	jeBoolean					FlushGouraudCache();
	jeBoolean					FlushStencilCache();

private:
	void						EnableAlpha(jeBoolean Enable);
	jeBoolean					SetTexture(int32 Stage, jeTexture* Texture);
	void						SetBlendFunc(D3DBLEND SFunc, D3DBLEND DFunc);
	void						SetTexWrap(int32 Stage, jeBoolean bWrap);
	void						SetFilterType(uint32 FilterType);
};

#endif