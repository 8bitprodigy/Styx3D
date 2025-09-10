#include "RenderStateManager.h"

RenderStateManager::RenderStateManager(IDirect3DDevice9* pDevice)
{
	m_pDevice = pDevice;
	m_pDevice->AddRef();

	for (DWORD i = 0; i < MAX_TEXTURE_STAGES; i++)
	{
		m_MinFilter[i] = m_MagFilter[i] = D3DTEXF_LINEAR;
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, m_MinFilter[i]);
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, m_MagFilter[i]);

		m_TexWrapEnable[i] = TRUE;
		m_pDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		m_pDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}

	m_FillMode = D3DFILL_SOLID;
	m_ShadeMode = D3DSHADE_GOURAUD;
	m_CullMode = D3DCULL_NONE;

	m_ZEnable = FALSE;
	m_ZWriteEnable = FALSE;
	m_ZFunc = D3DCMP_LESSEQUAL;

	m_StencilEnable = FALSE;
	m_StencilFunc = D3DCMP_ALWAYS;

	m_AlphaEnable = FALSE;
	m_SrcBlend = D3DBLEND_SRCALPHA;
	m_DestBlend = D3DBLEND_INVSRCALPHA;

	m_FogEnable = FALSE;
	m_FogColor = 0;

	m_FSAAEnable = FALSE;
	m_LightingEnable = FALSE;

	m_pDevice->SetRenderState(D3DRS_FILLMODE, m_FillMode);
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, m_ShadeMode);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, m_CullMode);
	m_pDevice->SetRenderState(D3DRS_ZENABLE, m_ZEnable);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, m_ZWriteEnable);
	m_pDevice->SetRenderState(D3DRS_ZFUNC, m_ZFunc);
	m_pDevice->SetRenderState(D3DRS_STENCILENABLE, m_StencilEnable);
	m_pDevice->SetRenderState(D3DRS_STENCILFUNC, m_StencilFunc);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, m_AlphaEnable);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, m_SrcBlend);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, m_DestBlend);
	m_pDevice->SetRenderState(D3DRS_FOGENABLE, m_FogEnable);
	m_pDevice->SetRenderState(D3DRS_FOGCOLOR, m_FogColor);
	m_pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, m_FSAAEnable);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, m_LightingEnable);
}

RenderStateManager::~RenderStateManager()
{
	m_pDevice->Release();
	m_pDevice = NULL;
}

void RenderStateManager::SetTextureFilter(DWORD Stage, D3DTEXTUREFILTERTYPE min, D3DTEXTUREFILTERTYPE mag)
{
	if (m_MinFilter[Stage] != min)
	{
		m_pDevice->SetSamplerState(Stage, D3DSAMP_MINFILTER, min);
		m_MinFilter[Stage] = min;
	}

	if (m_MagFilter[Stage] != mag)
	{
		m_pDevice->SetSamplerState(Stage, D3DSAMP_MAGFILTER, mag);
		m_MagFilter[Stage] = mag;
	}
}

void RenderStateManager::SetFillMode(D3DFILLMODE fill)
{
	if (m_FillMode != fill)
	{
		m_pDevice->SetRenderState(D3DRS_FILLMODE, fill);
		m_FillMode = fill;
	}
}

void RenderStateManager::SetShadeMode(D3DSHADEMODE shade)
{
	if (m_ShadeMode != shade)
	{
		m_ShadeMode = shade;
		m_pDevice->SetRenderState(D3DRS_SHADEMODE, shade);
	}
}

void RenderStateManager::SetCullMode(D3DCULL cull)
{
	if (m_CullMode != cull)
	{
		m_CullMode = cull;
		m_pDevice->SetRenderState(D3DRS_CULLMODE, cull);
	}
}

void RenderStateManager::EnableZBuffer(BOOL Enable)
{
	if (m_ZEnable != Enable)
	{
		m_pDevice->SetRenderState(D3DRS_ZENABLE, Enable);
		m_ZEnable = Enable;
	}
}

void RenderStateManager::EnableZWriteBuffer(BOOL Enable)
{
	if (m_ZWriteEnable != Enable)
	{
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, Enable);
		m_ZWriteEnable = Enable;
	}
}

void RenderStateManager::SetZCmp(D3DCMPFUNC func)
{
	if (m_ZFunc != func)
	{
		m_pDevice->SetRenderState(D3DRS_ZFUNC, func);
		m_ZFunc = func;
	}
}

void RenderStateManager::EnableStencil(BOOL Enable)
{
	if (m_StencilEnable != Enable)
	{
		m_pDevice->SetRenderState(D3DRS_STENCILENABLE, Enable);
		m_StencilEnable = Enable;
	}
}

void RenderStateManager::SetStencilFunc(D3DCMPFUNC func)
{
	if (m_StencilFunc != func)
	{
		m_pDevice->SetRenderState(D3DRS_STENCILFUNC, func);
		m_StencilFunc = func;
	}
}

void RenderStateManager::EnableAlpha(BOOL Enable)
{
	if (m_AlphaEnable != Enable)
	{
		m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, Enable);
		m_AlphaEnable = Enable;
	}
}

void RenderStateManager::SetAlphaBlendMode(D3DBLEND src, D3DBLEND dest)
{
	if (m_SrcBlend != src)
	{
		m_pDevice->SetRenderState(D3DRS_SRCBLEND, src);
		m_SrcBlend = src;
	}

	if (m_DestBlend != dest)
	{
		m_pDevice->SetRenderState(D3DRS_DESTBLEND, dest);
		m_DestBlend = dest;
	}
}

void RenderStateManager::EnableFog(BOOL Enable)
{
	if (m_FogEnable != Enable)
	{
		m_pDevice->SetRenderState(D3DRS_FOGENABLE, Enable);
		m_FogEnable = Enable;
	}
}

void RenderStateManager::SetFogColor(D3DCOLOR color)
{
	if (m_FogColor != color)
	{
		m_pDevice->SetRenderState(D3DRS_FOGCOLOR, color);
		m_FogColor = color;
	}
}

void RenderStateManager::EnableFSAA(BOOL Enable)
{
	if (m_FSAAEnable != Enable)
	{
		m_FSAAEnable = Enable;
		m_pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, Enable);
	}
}

void RenderStateManager::EnableLighting(BOOL Enable)
{
	if (m_LightingEnable != Enable)
	{
		m_LightingEnable = Enable;
		m_pDevice->SetRenderState(D3DRS_LIGHTING, Enable);
	}
}

void RenderStateManager::EnableTexWrap(DWORD Stage, BOOL Enable)
{
	if (m_TexWrapEnable[Stage] != Enable)
	{
		if (Enable)
		{
			m_pDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			m_pDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		}
		else
		{
			m_pDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			m_pDevice->SetSamplerState(Stage, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		}
	}
}

BOOL RenderStateManager::SetMaterialType(uint32 type, jeTexture** Layers, int32 NumLayers)
{
	for (DWORD i = 0; i < MAX_TEXTURE_STAGES; i++)
	{
		m_pDevice->SetTexture(i, NULL);
		m_pDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pDevice->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}

	switch (type)
	{
	case MATERIAL_TYPE_NO_TEXTURES:
	{
		m_pDevice->SetTexture(0, NULL);
		m_pDevice->SetTexture(1, NULL);

		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		break;
	}
	case MATERIAL_TYPE_BASE_TEXTURE_ONLY:
	{
		m_pDevice->SetTexture(0, D3D9_THandle_GetInterface(Layers[0]));
		m_pDevice->SetTexture(1, NULL);

		m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		SetAlphaBlendMode(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
		EnableAlpha(TRUE);

		break;
	}
	case MATERIAL_TYPE_LIGHTMAP_POLY:
	{
		m_pDevice->SetTexture(0, D3D9_THandle_GetInterface(Layers[0]));
		m_pDevice->SetTexture(1, D3D9_THandle_GetInterface(Layers[1]));

		m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		m_pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
		m_pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		EnableAlpha(TRUE);
		SetAlphaBlendMode(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		EnableTexWrap(0, TRUE);
		EnableTexWrap(1, FALSE);

		break;
	}
	}

	return JE_TRUE;
}