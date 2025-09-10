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
//#include "VertexCacheManager.h"

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
	
	jeTexture					*Layers[MAX_LAYERS];
	int32						NumLayers;

	uint32						Flags;
} PolyCacheEntry;

typedef struct StaticBuffer
{
	jeBoolean					Active;

	IDirect3DVertexBuffer9		*pVB;
	int32						NumVerts;

	jeRDriver_Layer				*Layers;
	int32						NumLayers;

	uint32						Flags;
} StaticBuffer;

class PolyCache
{
public:
	PolyCache();
	virtual ~PolyCache();

private:
	std::vector<PolyCacheEntry>				m_Cache;

	int32									m_NumVerts;
	IDirect3DVertexBuffer9					*m_pVB;

	std::vector<StaticBuffer>				m_StaticBuffers;

	IDirect3DDevice9						*m_pDevice;

public:
	jeBoolean					Initialize(IDirect3DDevice9 *pDevice);
	void						Shutdown();

	uint32						AddStaticBuffer(jeHWVertex *Points, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags);
	jeBoolean					RemoveStaticBuffer(uint32 id);
	jeBoolean					RenderStaticBuffer(uint32 id, int32 StartVertex, int32 NumPolys, jeXForm3d *XForm);

	jeBoolean					AddGouraudPoly(jeTLVertex *Pnts, int32 NumPoints, uint32 Flags);
	jeBoolean					AddMiscTexturePoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags);
	jeBoolean					AddWorldPoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags);

	jeBoolean					Flush();

private:
	void						EnableAlpha(jeBoolean Enable);
};

#endif
