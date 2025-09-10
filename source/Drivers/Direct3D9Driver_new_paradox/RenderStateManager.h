#ifndef RENDER_STATE_MANAGER_H
#define RENDER_STATE_MANAGER_H

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "D3D9TextureMgr.h"
#include "jeSingleton.h"

#define MAX_TEXTURE_STAGES						8

enum MaterialType
{
	MATERIAL_TYPE_NO_TEXTURES = 0,
	MATERIAL_TYPE_BASE_TEXTURE_ONLY,
	MATERIAL_TYPE_LIGHTMAP_POLY,

	NUM_MATERIAL_TYPES
};

class RenderStateManager : public jet3d::jeSingleton<RenderStateManager>
{
public:
	RenderStateManager(IDirect3DDevice9* pDevice);
	virtual ~RenderStateManager();

protected:
	IDirect3DDevice9* m_pDevice;

	D3DTEXTUREFILTERTYPE						m_MinFilter[MAX_TEXTURE_STAGES], m_MagFilter[MAX_TEXTURE_STAGES];
	D3DFILLMODE									m_FillMode;
	D3DSHADEMODE								m_ShadeMode;
	D3DCULL										m_CullMode;

	BOOL										m_ZEnable, m_ZWriteEnable;
	D3DCMPFUNC									m_ZFunc;

	BOOL										m_StencilEnable;
	D3DCMPFUNC									m_StencilFunc;

	BOOL										m_AlphaEnable;
	D3DBLEND									m_SrcBlend, m_DestBlend;

	BOOL										m_FogEnable;
	D3DCOLOR									m_FogColor;

	BOOL										m_FSAAEnable;
	BOOL										m_LightingEnable;

	BOOL										m_TexWrapEnable[MAX_TEXTURE_STAGES];

public:
	void										SetTextureFilter(DWORD Stage, D3DTEXTUREFILTERTYPE min, D3DTEXTUREFILTERTYPE mag);
	void										SetFillMode(D3DFILLMODE fill);
	void										SetShadeMode(D3DSHADEMODE shade);
	void										SetCullMode(D3DCULL cull);

	void										EnableZBuffer(BOOL Enable);
	void										EnableZWriteBuffer(BOOL Enable);
	void										SetZCmp(D3DCMPFUNC func);

	void										EnableStencil(BOOL Enable);
	void										SetStencilFunc(D3DCMPFUNC func);

	void										EnableAlpha(BOOL Enable);
	void										SetAlphaBlendMode(D3DBLEND src, D3DBLEND dest);

	void										EnableFog(BOOL Enable);
	void										SetFogColor(D3DCOLOR color);

	void										EnableFSAA(BOOL Enable);
	void										EnableLighting(BOOL Enable);

	void										EnableTexWrap(DWORD Stage, BOOL Enable);
	BOOL										SetMaterialType(DWORD type, jeTexture** Layers, int32 NumLayers);
};

#endif
