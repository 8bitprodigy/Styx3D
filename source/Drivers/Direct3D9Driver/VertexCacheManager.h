#ifndef VERTEX_CACHE_MANAGER_H
#define VERTEX_CACHE_MANAGER_H

#include <windows.h>
#include <d3d9.h>
#include <list>
#include "DCommon.h"
#include "D3D9TextureMgr.h"

class VertexCacheManager;
class VertexCache;

#define MAX_VCACHES						10
#define MAX_VERTS						8192

#define D3DFVF_WORLDVERTEX		(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2)
#define D3DFVF_REGULARVERTEX	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define D3DFVF_SHADEDVERTEX		(D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

typedef struct RegularVertex
{
	float								x, y, z, rhw;
	D3DCOLOR							diffuse;
	float								u, v;
} RegularVertex;

typedef struct WorldVertex
{
	float								x, y, z, rhw;
	D3DCOLOR							diffuse;
	float								u1, v1;
	float								u2, v2;
} WorldVertex;

typedef struct ShadedVertex
{
	float								x, y, z, rhw;
	D3DCOLOR							diffuse;
} ShadedVertex;

typedef struct StaticBuffer
{
	DWORD					id;

	int						Stride;
	jeTexture				*Texture[MAX_LAYERS];

	int						NumVerts;
	int						NumTris;

	IDirect3DVertexBuffer9	*pBuffer;
} StaticBuffer;


class VertexCacheManager
{
public:
	VertexCacheManager(IDirect3DDevice9 *pDevice);
	virtual ~VertexCacheManager();

private:
	std::list<StaticBuffer*>			m_StaticBuffers;
	
	VertexCache							*m_WorldVertexCache[MAX_VCACHES];
	VertexCache							*m_RegularVertexCache[MAX_VCACHES];
	VertexCache							*m_ShadedVertexCache;

	DWORD								m_ActiveCache;

	IDirect3DDevice9					*m_pDevice;

public:
	jeBoolean							CreateStaticBuffer(jeTexture *Texture, int numVerts, const void *pVerts, DWORD *id);

	jeBoolean							RenderStaticBuffer(DWORD id);
	jeBoolean							RenderWorldVertex(int numVerts, const void *pVerts, jeTexture **Textures, int numTextures);
	jeBoolean							RenderRegularVertex(int numVerts, const void *pVerts, jeTexture *Texture);
	jeBoolean							RenderShadedVertex(int numVerts, const void *pVerts);

	jeBoolean							FlushAll();
	jeBoolean							Flush(DWORD id);

	DWORD								GetActiveCache()			{	return m_ActiveCache;	}
	void								SetActiveCache(DWORD id)	{	m_ActiveCache = id;		}
};

class VertexCache
{
public:
	VertexCache(IDirect3DDevice9 *pDevice, DWORD id, DWORD fvf, DWORD stride);
	virtual ~VertexCache();

private:
	DWORD								m_ID;

	IDirect3DVertexBuffer9				*m_pVB;
	IDirect3DDevice9					*m_pDevice;

	jeTexture							*m_pTexture[MAX_LAYERS];

	DWORD								m_NumVerts;

	DWORD								m_FVF;
	DWORD								m_Stride;

public:
	jeBoolean							Add(DWORD numVerts, const void *pVerts);
	
	jeBoolean							Flush();

	void								SetTexture(int stage, jeTexture *Texture)		{	m_pTexture[stage] = Texture;	}
	jeTexture							*GetTexture(int stage)							{	return m_pTexture[stage];	}

	DWORD								GetNumVerts()									{	return m_NumVerts;	}
	jeBoolean							IsEmpty()										{	return m_NumVerts == 0 ? JE_TRUE : JE_FALSE;	}
};

#endif
