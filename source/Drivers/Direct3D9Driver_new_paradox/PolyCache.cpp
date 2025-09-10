#include <assert.h>
#include <algorithm>
#include <vector>
#include "PolyCache.h"
#include "D3D9Log.h"
/*		commenting out ref to D3DRender -- not using.
#include "D3D9Render.h"
*/

struct TextureSortFunctor
{
	bool operator ()(PolyCacheEntry& e1, PolyCacheEntry& e2)
	{
		if (e1.Layers[0] == NULL || e2.Layers[0] == NULL)
			return (e1.Layers[0] != NULL);

		if (e1.Layers[0]->id >= e2.Layers[0]->id)
		{
			if (e1.Layers[0] == e2.Layers[0])
			{
				if (e1.NumLayers != e2.NumLayers)
					return e1.NumLayers < e2.NumLayers;

				if (e1.NumLayers > 1)
					return (e1.Layers[1]->id < e2.Layers[1]->id);
			}

			return false;
		}

		return true;
	}
};

__inline DWORD FtoDW(FLOAT f) { return *((DWORD*)& f); }

//=====================================================================================
//	Log2
//	Return the log of a size
//=====================================================================================
/*static uint32 Log2(uint32 P2)
{
	uint32		p = 0;
	int32		i = 0;

	for (i = P2; i > 0; i>>=1)
		p++;
	return (p-1);
}*/

static int Log2(uint32 n) {

	int pos = 0;

	if (n >= 1 << 16) { n >>= 16; pos += 16; }
	if (n >= 1 << 8) { n >>= 8; pos += 8; }
	if (n >= 1 << 4) { n >>= 4; pos += 4; }
	if (n >= 1 << 2) { n >>= 2; pos += 2; }
	if (n >= 1 << 1) { pos += 1; }

	return ((n == 0) ? (-1) : pos);
}

//=====================================================================================
//	SnapToPower2
//	Snaps a number to a power of 2
//=====================================================================================
static int32 SnapToPower2(int32 Width)
{
	if (Width <= 1) return 1;
	else if (Width <= 2) return 2;
	else if (Width <= 4) return 4;
	else if (Width <= 8) return 8;
	else if (Width <= 16) return 16;
	else if (Width <= 32) return 32;
	else if (Width <= 64) return 64;
	else if (Width <= 128) return 128;
	else if (Width <= 256) return 256;
	else if (Width <= 512) return 512;
	else if (Width <= 1024) return 1024;
	else if (Width <= 2048) return 2048;
	else
		return -1;
}

//=====================================================================================
//	Return the max log of a (power of 2) width and height
//=====================================================================================
static int32 GetLog(int32 Width, int32 Height)
{
	int32	LWidth = SnapToPower2(max(Width, Height));

	return Log2(LWidth);
}

//=====================================================================================
//	FillLMapSurface
//=====================================================================================
static void FillLMapSurface(jeTLVertex* Pnts, int32 NumPoints, jeTexture* THandle, jeRDriver_LMapCBInfo* Info, int32 LNum)
{
	U16* pTempBits, * pTempBits2;
	int32				w, h, Width, Height, Size;
	U8* pBitPtr;
	RGB_LUT* Lut;
	int32				Extra;
	//U8					PrevR,PrevG,PrevB;

	pBitPtr = (U8*)Info->RGBLight[LNum];

	Width = THandle->Width;
	Height = THandle->Height;
	Size = THandle->Log;

	Lut = &Lut1;
	/*commenting out D3D9Render refs... not using
	Lut = D3D9Render::getSingletonPtr()->GetRGBTable();
	*/

	D3D9_THandle_Lock(THandle, 0, (void**)& pTempBits);
	pTempBits2 = pTempBits;

	Extra = (THandle->stride - THandle->Width);

	for (h = 0; h < Height; h++)
	{
		for (w = 0; w < Width; w++)
		{
			U8	R, G, B;
			U16	Color;

			R = *pBitPtr++;
			G = *pBitPtr++;
			B = *pBitPtr++;

			Color = (U16)((Lut->R[R] | Lut->G[G] | Lut->B[B]) & 0xFFFF);

			*((U16*)pTempBits) = Color;
			((U16*)pTempBits)++;
		}
		pTempBits += Extra;
	}
	D3D9_THandle_Unlock(THandle, 0);


}

//====================================================================================
//	PrepPolyVerts
//====================================================================================
static jeBoolean PrepPolyVerts(jeTLVertex* Pnts, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, void* LMapCBContext)
{
	float			InvScale, u, v;
	float			ShiftU, ShiftV, ScaleU, ScaleV;
	jeTLVertex* TempVerts, * pTVerts, * pVerts;
	//float			du, dv, dx, dy, MipScale;
	int32			j;
	//	D3DSURFACE_DESC				desc;

	float				InvScale2, ShiftU2, ShiftV2;
	jeRDriver_Layer* LMapLayer;
	jeRDriver_Layer* TexLayer;

	TempVerts = (jeTLVertex*)malloc(sizeof(jeTLVertex) * NumPoints);
	memcpy(TempVerts, Pnts, sizeof(jeTLVertex) * NumPoints);

	pTVerts = &TempVerts[0];

	// Set up shifts and scaled for texture uv's
	TexLayer = &Layers[0];
	LMapLayer = &Layers[1];

	// Normalize UV's using Texture Size
	InvScale = 1.0f / (float)((1 << TexLayer->THandle->Log));

	ShiftU = TexLayer->ShiftU;
	ShiftV = TexLayer->ShiftV;
	ScaleU = (1.0f / (TexLayer->ScaleU));
	ScaleV = (1.0f / (TexLayer->ScaleV));

	if (NumLayers > 1)
	{
		// Set up shifts and scaled for lightmap uv's
		ShiftU2 = (float)-LMapLayer->ShiftU + 8.0f;
		ShiftV2 = (float)-LMapLayer->ShiftV + 8.0f;

		InvScale2 = 1.0f / (float)((1 << LMapLayer->THandle->Log) << 3);
	}

	pVerts = &Pnts[0];

	for (j = 0; j < NumPoints; j++)
	{

		u = pTVerts->u * ScaleU + ShiftU;
		v = pTVerts->v * ScaleV + ShiftV;

		pVerts->u = u * InvScale;
		pVerts->v = v * InvScale;

		if (NumLayers > 1)
		{
			u = pTVerts->u + ShiftU2;
			v = pTVerts->v + ShiftV2;

			pVerts->pad1 = u * InvScale2;
			pVerts->pad2 = v * InvScale2;
		}


		//	pVerts->color = pTVerts->Color;

		pTVerts++;
		pVerts++;
	}

	free(TempVerts);
	return TRUE;
}

PolyCache::PolyCache()
{
	m_pVB = NULL;
	m_pDevice = NULL;

	m_WorldCache.clear();
	m_MiscCache.clear();
	m_GouraudCache.clear();
	m_StencilCache.clear();

	m_NumVerts = 0;
	m_pRS = NULL;
}

PolyCache::~PolyCache()
{
	Shutdown();
}

jeBoolean PolyCache::Initialize(IDirect3DDevice9* pDevice)
{
	HRESULT								hres;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  PolyCache::Initialize()");
	else
		REPORT("Function Call:  PolyCache::Initialize()");

	m_pDevice = pDevice;
	m_pDevice->AddRef();

	hres = m_pDevice->CreateVertexBuffer(sizeof(PolyVert) * 25000, D3DUSAGE_DYNAMIC, J3D_FVF, D3DPOOL_DEFAULT, &m_pVB, NULL);
	if (FAILED(hres))
	{
		LOG("ERROR:   Could not create vertex buffer!!");
		return JE_FALSE;
	}

	m_NumVerts = 0;
	for (int i = 0; i < MAX_LAYERS; i++)
	{
		m_TextureStages[i] = NULL;
		m_bOldTexWrap[i] = JE_FALSE;
	}

	m_bOldAlphaEnable = JE_FALSE;
	m_OldSFunc = m_OldDFunc = D3DBLEND_ONE;

	m_WorldCache.clear();
	m_MiscCache.clear();
	m_GouraudCache.clear();
	m_StencilCache.clear();

	m_pRS = new RenderStateManager(pDevice);

	int i;
	for (i = 0; i < MAX_LAYERS; i++)
	{
		m_TextureStages[i] = NULL;
		m_bOldTexWrap[i] = JE_FALSE;
	}

	m_OldSFunc = m_OldDFunc = D3DBLEND_ONE;

	return JE_TRUE;
}

void PolyCache::Shutdown()
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  PolyCache::Shutdown()");
	else
		REPORT("Function Call:  PolyCache::Shutdown()");

	for (size_t i = 0; i < m_StaticBuffers.size(); i++)
	{
		SAFE_RELEASE(m_StaticBuffers[i].pVB);
		m_StaticBuffers[i].Active = JE_FALSE;
		m_StaticBuffers[i].NumLayers = 0;
		m_StaticBuffers[i].NumVerts = 0;
		m_StaticBuffers[i].Layers = NULL;
		m_StaticBuffers[i].Flags = NULL;
	}

	m_StaticBuffers.clear();

	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pDevice);

	m_NumVerts = 0;

	m_WorldCache.clear();
	m_MiscCache.clear();
	m_GouraudCache.clear();
	m_StencilCache.clear();
}

jeBoolean PolyCache::AddMiscTexturePoly(jeTLVertex* Pnts, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, uint32 Flags)
{
	HRESULT							hres;
	PolyVert* verts = NULL, * data = NULL;
	PolyCacheEntry* p = NULL;
	DWORD							dwAlpha = 0;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  PolyCache::AddMiscTexturePoly()");
	else
		REPORT("Function Call:  PolyCache::AddMiscTexturePoly()");

	m_MiscCache.resize(m_MiscCache.size() + 1);
	p = &m_MiscCache[m_MiscCache.size() - 1];

	memset(p, 0, sizeof(PolyCacheEntry));

	p->StartVertex = m_NumVerts;

	if (Flags & JE_RENDER_FLAG_ALPHA)
		dwAlpha = FtoDW(Pnts->a);
	else
		dwAlpha = 255;

	verts = new PolyVert[NumPoints];
	if (!verts)
	{
		LOG("ERROR:  Out of memory!!");
		return JE_FALSE;
	}

	for (int32 i = 0; i < NumPoints; i++)
	{
		float					zrecip;

		zrecip = 1.0f / Pnts[i].z;

		verts[i].x = Pnts[i].x;
		verts[i].y = Pnts[i].y;
		verts[i].z = 1.0f - zrecip;
		verts[i].rhw = zrecip;

		verts[i].u = Pnts[i].u;
		verts[i].v = Pnts[i].v;

		verts[i].lu = Pnts[i].pad1;
		verts[i].lv = Pnts[i].pad2;

		verts[i].diffuse = D3DCOLOR_ARGB(dwAlpha, (DWORD)Pnts[i].r, (DWORD)Pnts[i].g, (DWORD)Pnts[i].b);
	}

	if (m_NumVerts == 0)
		hres = m_pVB->Lock(0, sizeof(PolyVert) * NumPoints, (void**)& data, D3DLOCK_DISCARD);
	else
		hres = m_pVB->Lock(m_NumVerts * sizeof(PolyVert), sizeof(PolyVert) * NumPoints, (void**)& data, D3DLOCK_NOOVERWRITE);

	if (FAILED(hres))
	{
		LOG("ERROR:  Could not lock vertex buffer!!");
		SAFE_DELETE_ARRAY(verts);
		return JE_FALSE;
	}

	memcpy(data, verts, sizeof(PolyVert) * NumPoints);

	hres = m_pVB->Unlock();
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not unlock vertex buffer!!");
		SAFE_DELETE_ARRAY(verts);
		return JE_FALSE;
	}

	p->NumVertices = NumPoints;
	p->NumLayers = NumLayers;
	p->Layers[0] = Layers[0].THandle;

	if (NumLayers > 1)
		p->Layers[1] = Layers[1].THandle;
	else
		p->Layers[1] = NULL;

	p->Flags = Flags;

	m_NumVerts += NumPoints;

	SAFE_DELETE_ARRAY(verts);
	return JE_TRUE;
}

jeBoolean PolyCache::AddGouraudPoly(jeTLVertex* Pnts, int32 NumPoints, uint32 Flags)
{
	HRESULT							hres;
	PolyVert* verts = NULL, * data = NULL;
	PolyCacheEntry* p = NULL;
	DWORD							dwAlpha = 0;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  PolyCache::AddGouraudPoly()");
	else
		REPORT("Function Call:  PolyCache::AddGouraudPoly()");

	m_GouraudCache.resize(m_GouraudCache.size() + 1);
	p = &m_GouraudCache[m_GouraudCache.size() - 1];

	memset(p, 0, sizeof(PolyCacheEntry));

	p->StartVertex = m_NumVerts;

	if (Flags & JE_RENDER_FLAG_ALPHA)
		dwAlpha = FtoDW(Pnts->a);
	else
		dwAlpha = 255;

	verts = new PolyVert[NumPoints];
	if (!verts)
	{
		LOG("ERROR:  Out of memory!!");
		return JE_FALSE;
	}

	for (int32 i = 0; i < NumPoints; i++)
	{
		float					zrecip;

		zrecip = 1.0f / Pnts[i].z;

		verts[i].x = Pnts[i].x;
		verts[i].y = Pnts[i].y;
		verts[i].z = 1.0f - zrecip;
		verts[i].rhw = zrecip;

		verts[i].u = 0.0f;
		verts[i].v = 0.0f;

		verts[i].lu = 0.0f;
		verts[i].lv = 0.0f;

		verts[i].diffuse = D3DCOLOR_ARGB(dwAlpha, (DWORD)Pnts[i].r, (DWORD)Pnts[i].g, (DWORD)Pnts[i].b);
	}

	if (m_NumVerts == 0)
		hres = m_pVB->Lock(0, sizeof(PolyVert) * NumPoints, (void**)& data, D3DLOCK_DISCARD);
	else
		hres = m_pVB->Lock(m_NumVerts * sizeof(PolyVert), sizeof(PolyVert) * NumPoints, (void**)& data, D3DLOCK_NOOVERWRITE);

	if (FAILED(hres))
	{
		LOG("ERROR:  Could not lock vertex buffer!!");
		SAFE_DELETE_ARRAY(verts);
		return JE_FALSE;
	}

	memcpy(data, verts, sizeof(PolyVert) * NumPoints);

	hres = m_pVB->Unlock();
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not unlock vertex buffer!!");
		SAFE_DELETE_ARRAY(verts);
		return JE_FALSE;
	}

	p->NumVertices = NumPoints;
	p->NumLayers = 0;
	p->Layers[0] = NULL;
	p->Layers[1] = NULL;
	p->Flags = Flags;

	m_NumVerts += NumPoints;

	SAFE_DELETE_ARRAY(verts);
	return JE_TRUE;
}

jeBoolean PolyCache::AddWorldPoly(jeTLVertex* Pnts, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, void* LMapCBContext, uint32 Flags)
{
	HRESULT							hres;
	PolyCacheEntry* p = NULL;
	PolyVert* verts = NULL, * data = NULL;
	jeRDriver_LMapCBInfo			LMapInfo;
	DWORD							dwAlpha = 0;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  PolyCache::AddWorldPoly()");
	else
		REPORT("Function Call:  PolyCache::AddWorldPoly()");

	m_WorldCache.resize(m_WorldCache.size() + 1);
	p = &m_WorldCache[m_WorldCache.size() - 1];

	memset(p, 0, sizeof(PolyCacheEntry));

	p->StartVertex = m_NumVerts;

	if (LMapCBContext && NumLayers > 1)
	{
		g_D3D9Drv.SetupLightmap(&LMapInfo, LMapCBContext);
		if (!Layers[1].THandle)
			LOG("WARNING:  No lightmap!!");

		if (LMapInfo.Dynamic || !Layers[1].THandle->Lightmap)
		{
			FillLMapSurface(Pnts, NumPoints, Layers[1].THandle, &LMapInfo, 0);
			Layers[1].THandle->Lightmap = JE_TRUE;
		}

		PrepPolyVerts(Pnts, NumPoints, Layers, NumLayers, LMapCBContext);
	}
	else
	{
		PrepPolyVerts(Pnts, NumPoints, Layers, NumLayers, NULL);
	}

	if (Flags & JE_RENDER_FLAG_ALPHA)
		dwAlpha = FtoDW(Pnts->a);
	else
		dwAlpha = 255;

	verts = new PolyVert[NumPoints];
	if (!verts)
	{
		LOG("ERROR:  Out of memory!!");
		return JE_FALSE;
	}

	for (int32 j = 0; j < NumPoints; j++)
	{
		float						zrecip;

		zrecip = 1.0f / Pnts[j].z;

		verts[j].x = Pnts[j].x;
		verts[j].y = Pnts[j].y;
		verts[j].z = 1.0f - zrecip;
		verts[j].rhw = zrecip;

		verts[j].u = Pnts[j].u;
		verts[j].v = Pnts[j].v;

		verts[j].lu = Pnts[j].pad1;
		verts[j].lv = Pnts[j].pad2;

		verts[j].diffuse = D3DCOLOR_ARGB(dwAlpha, (DWORD)Pnts[j].r, (DWORD)Pnts[j].g, (DWORD)Pnts[j].b);
	}

	if (m_NumVerts == 0)
		hres = m_pVB->Lock(0, sizeof(PolyVert) * NumPoints, (void**)& data, D3DLOCK_DISCARD);
	else
		hres = m_pVB->Lock(m_NumVerts * sizeof(PolyVert), sizeof(PolyVert) * NumPoints, (void**)& data, D3DLOCK_NOOVERWRITE);

	if (FAILED(hres))
	{
		LOG("ERROR:  Could not lock vertex buffer!!");
		SAFE_DELETE_ARRAY(verts);
		return JE_FALSE;
	}

	memcpy(data, verts, sizeof(PolyVert) * NumPoints);

	hres = m_pVB->Unlock();
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not unlock vertex buffer!!");
		SAFE_DELETE_ARRAY(verts);
		return JE_FALSE;
	}

	p->NumVertices = NumPoints;
	p->NumLayers = NumLayers;
	p->Layers[0] = Layers[0].THandle;
	p->Layers[1] = Layers[1].THandle;
	p->Flags = Flags;

	m_NumVerts += NumPoints;
	//m_Cache.push_back(p);

	SAFE_DELETE_ARRAY(verts);

	return JE_TRUE;
}

jeBoolean PolyCache::FlushWorldCache()
{
	HRESULT hres = S_OK;
	int32 previd = -1;
	D3DMATERIAL9 mat;

	mat.Diffuse.r = 1.0f;
	mat.Diffuse.g = 1.0f;
	mat.Diffuse.b = 1.0f;
	mat.Diffuse.a = 1.0f;

	mat.Ambient = mat.Diffuse;

	m_pDevice->SetMaterial(&mat);

	for (size_t i = 0; i < m_WorldCache.size(); i++)
	{
		PolyCacheEntry* p = &m_WorldCache[i];

		if (p->NumLayers == 0 && previd != -1)
		{
			SetTexture(0, NULL);
			SetTexture(1, NULL);

			previd = -1;
		}
		else if (previd != p->Layers[0]->id)
		{
			previd = p->Layers[0]->id;
			SetTexture(0, p->Layers[0]);
			SetTexture(1, NULL);
		}

		if (p->NumLayers == 1)
		{
			m_pRS->SetMaterialType(MATERIAL_TYPE_BASE_TEXTURE_ONLY, p->Layers, 1);
		}
		else if (p->NumLayers == 2)
		{
			m_pRS->SetMaterialType(MATERIAL_TYPE_LIGHTMAP_POLY, p->Layers, p->NumLayers);
		}

		if (p->Flags & JE_RENDER_FLAG_BILINEAR_FILTER)
			SetFilterType(FILTER_TYPE_BILINEAR);
		else
			SetFilterType(FILTER_TYPE_ANISOTROPIC);

		if (p->Flags & JE_RENDER_FLAG_CLAMP_UV)
			SetTexWrap(0, JE_FALSE);
		else
			SetTexWrap(0, JE_TRUE);

		if (p->Flags & JE_RENDER_FLAG_WIREFRAME)
			m_pRS->SetFillMode(D3DFILL_WIREFRAME);
		else
			m_pRS->SetFillMode(D3DFILL_SOLID);

		m_pRS->EnableLighting(FALSE);
		m_pRS->SetCullMode(D3DCULL_CCW);

		if (p->Flags & JE_RENDER_FLAG_NO_ZTEST)
			m_pRS->EnableZBuffer(FALSE);
		else
			m_pRS->EnableZBuffer(TRUE);

		if (p->Flags & JE_RENDER_FLAG_NO_ZWRITE)
			m_pRS->EnableZWriteBuffer(FALSE);
		else
			m_pRS->EnableZWriteBuffer(TRUE);

		hres = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, p->StartVertex, p->NumVertices - 2);
		if (FAILED(hres))
		{
			return JE_FALSE;
		}
	}

	m_WorldCache.clear();
	return JE_TRUE;
}

jeBoolean PolyCache::FlushMiscCache()
{
	HRESULT hres = S_OK;
	int32 previd = -1;
	D3DMATERIAL9 mat;

	mat.Diffuse.r = 1.0f;
	mat.Diffuse.g = 1.0f;
	mat.Diffuse.b = 1.0f;
	mat.Diffuse.a = 1.0f;

	mat.Ambient = mat.Diffuse;

	m_pDevice->SetMaterial(&mat);

	for (size_t i = 0; i < m_MiscCache.size(); i++)
	{
		PolyCacheEntry* p = &m_MiscCache[i];

		if (p->NumLayers == 0 && previd != -1)
		{
			m_pRS->SetMaterialType(MATERIAL_TYPE_NO_TEXTURES, NULL, 0);
			previd = -1;
		}
		else if (previd != p->Layers[0]->id)
		{
			previd = p->Layers[0]->id;
			SetTexture(0, p->Layers[0]);
			SetTexture(1, NULL);
		}

		if (p->NumLayers == 1)
			m_pRS->SetMaterialType(MATERIAL_TYPE_BASE_TEXTURE_ONLY, p->Layers, p->NumLayers);

		if (p->Flags & JE_RENDER_FLAG_BILINEAR_FILTER)
			SetFilterType(FILTER_TYPE_BILINEAR);
		else
			SetFilterType(FILTER_TYPE_ANISOTROPIC);

		if (p->Flags & JE_RENDER_FLAG_CLAMP_UV)
			SetTexWrap(0, JE_FALSE);
		else
			SetTexWrap(0, JE_TRUE);

		if (p->Flags & JE_RENDER_FLAG_WIREFRAME)
			m_pRS->SetFillMode(D3DFILL_WIREFRAME);
		else
			m_pRS->SetFillMode(D3DFILL_SOLID);

		m_pRS->EnableLighting(FALSE);
		m_pRS->SetCullMode(D3DCULL_NONE);

		if (p->Flags & JE_RENDER_FLAG_NO_ZTEST)
			m_pRS->EnableZBuffer(FALSE);
		else
			m_pRS->EnableZBuffer(TRUE);

		if (p->Flags & JE_RENDER_FLAG_NO_ZWRITE)
			m_pRS->EnableZWriteBuffer(FALSE);
		else
			m_pRS->EnableZWriteBuffer(TRUE);

		hres = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, p->StartVertex, p->NumVertices - 2);
		if (FAILED(hres))
		{
			return JE_FALSE;
		}
	}

	m_MiscCache.clear();
	return JE_TRUE;
}

jeBoolean PolyCache::FlushGouraudCache()
{
	HRESULT hres = S_OK;
	D3DMATERIAL9 mat;

	mat.Diffuse.r = 1.0f;
	mat.Diffuse.g = 1.0f;
	mat.Diffuse.b = 1.0f;
	mat.Diffuse.a = 1.0f;

	mat.Ambient = mat.Diffuse;

	m_pDevice->SetMaterial(&mat);

	for (size_t i = 0; i < m_GouraudCache.size(); i++)
	{
		PolyCacheEntry* p = &m_GouraudCache[i];

		if (p->NumLayers == 0)
		{
			m_pRS->SetMaterialType(MATERIAL_TYPE_NO_TEXTURES, NULL, 0);

			if (p->Flags & JE_RENDER_FLAG_WIREFRAME)
				m_pRS->SetFillMode(D3DFILL_WIREFRAME);
			else
				m_pRS->SetFillMode(D3DFILL_SOLID);

			m_pRS->EnableLighting(FALSE);
			m_pRS->SetCullMode(D3DCULL_NONE);

			if (p->Flags & JE_RENDER_FLAG_NO_ZTEST)
				m_pRS->EnableZBuffer(FALSE);
			else
				m_pRS->EnableZBuffer(TRUE);

			if (p->Flags & JE_RENDER_FLAG_NO_ZWRITE)
				m_pRS->EnableZWriteBuffer(FALSE);
			else
				m_pRS->EnableZWriteBuffer(TRUE);

			hres = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, p->StartVertex, p->NumVertices - 2);
			if (FAILED(hres))
			{
				return JE_FALSE;
			}
		}
	}

	m_GouraudCache.clear();
	return JE_TRUE;
}

jeBoolean PolyCache::FlushStencilCache()
{
	return JE_TRUE;
}

jeBoolean PolyCache::Flush()
{
	std::sort(m_MiscCache.begin(), m_MiscCache.end(), TextureSortFunctor());
	std::sort(m_WorldCache.begin(), m_WorldCache.end(), TextureSortFunctor());

	m_pDevice->SetFVF(J3D_FVF);
	m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(PolyVert));

	if (!FlushWorldCache())
		return JE_FALSE;

	if (!FlushMiscCache())
		return JE_FALSE;

	if (!FlushGouraudCache())
		return JE_FALSE;

	if (!FlushStencilCache())
		return JE_FALSE;

	m_NumVerts = 0;

	return JE_TRUE;
}

uint32 PolyCache::AddStaticBuffer(jeHWVertex* Points, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, uint32 Flags)
{
	HRESULT								hres;
	uint32								currIndex;
	void* data = NULL;
	StaticBuffer* buffer = NULL;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  PolyCache::AddStaticBuffer()");
	else
		REPORT("Function Call:  PolyCache::AddStaticBuffer()");

	if (!m_pDevice)
		return 0;

	if (m_StaticBuffers.size() > 0)
	{
		size_t i = 0;
		for (i = 0; i < m_StaticBuffers.size(); i++)
		{
			if (m_StaticBuffers[i].Active == JE_FALSE)
			{
				buffer = &m_StaticBuffers[i];
				break;
			}
		}

		if (i >= m_StaticBuffers.size())
		{
			currIndex = (uint32)m_StaticBuffers.size();
			m_StaticBuffers.resize(m_StaticBuffers.size() + 1);
			buffer = &m_StaticBuffers[currIndex];
		}
	}
	else
	{
		currIndex = (uint32)m_StaticBuffers.size();
		m_StaticBuffers.resize(m_StaticBuffers.size() + 1);
		buffer = &m_StaticBuffers[currIndex];
	}

	hres = m_pDevice->CreateVertexBuffer(sizeof(jeHWVertex) * NumPoints, 0, J3D_HW_FVF, D3DPOOL_DEFAULT, &buffer->pVB, NULL);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not create static vertex buffer!!");
		return 0;
	}

	buffer->Active = JE_TRUE;
	buffer->Layers = Layers;
	buffer->NumLayers = NumLayers;
	buffer->NumVerts = NumPoints;

	hres = buffer->pVB->Lock(0, sizeof(jeHWVertex) * NumPoints, &data, 0);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not lock static vertex buffer!!");
		buffer->Active = JE_FALSE;
		buffer->Layers = NULL;
		buffer->NumVerts = 0;
		buffer->NumLayers = 0;
		SAFE_RELEASE(buffer->pVB);
		return 0;
	}

	memcpy(data, (const void*)Points, sizeof(jeHWVertex) * NumPoints);

	hres = buffer->pVB->Unlock();
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not unlock static vertex buffer!!");
		buffer->Active = JE_FALSE;
		buffer->Layers = NULL;
		buffer->NumVerts = 0;
		buffer->NumLayers = 0;
		SAFE_RELEASE(buffer->pVB);
		return 0;
	}

	buffer->Flags = Flags;
	return (uint32)m_StaticBuffers.size();
}

jeBoolean PolyCache::RemoveStaticBuffer(uint32 id)
{
	uint32							actual_id = id - 1;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  PolyCache::RemoveStaticBuffer()");
	else
		REPORT("Function Call:  PolyCache::RemoveStaticBuffer()");

	if (m_StaticBuffers[actual_id].Active == JE_FALSE)
		return JE_TRUE;

	m_StaticBuffers[actual_id].Active = JE_FALSE;
	m_StaticBuffers[actual_id].NumVerts = 0;
	m_StaticBuffers[actual_id].NumLayers = 0;
	m_StaticBuffers[actual_id].Layers = NULL;
	m_StaticBuffers[actual_id].Flags = 0;
	SAFE_RELEASE(m_StaticBuffers[actual_id].pVB);

	return JE_TRUE;
}

jeBoolean PolyCache::RenderStaticBuffer(uint32 id, int32 StartVertex, int32 NumPolys, jeXForm3d* XForm)
{
	uint32							actual_id = id - 1;
	HRESULT							hres;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  PolyCache::RenderStaticBuffer()");
	else
		REPORT("Function Call:  PolyCache::RenderStaticBuffer()");

	if (!m_pDevice)
		return JE_FALSE;

	if (XForm != NULL)
	{
		D3DXMATRIX					mat;

		jeXForm3d_ToD3DMatrix(XForm, &mat);
		m_pDevice->SetTransform(D3DTS_WORLD, &mat);
	}
	else
	{
		m_pDevice->SetTransform(D3DTS_WORLD, NULL);
	}

	m_pDevice->SetFVF(J3D_HW_FVF);
	m_pDevice->SetStreamSource(0, m_StaticBuffers[actual_id].pVB, 0, sizeof(jeHWVertex));

	if (m_StaticBuffers[actual_id].NumLayers > 0)
	{
		m_pDevice->SetTexture(0, m_StaticBuffers[actual_id].Layers[0].THandle->pTexture);

		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

		m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		if (m_StaticBuffers[actual_id].NumLayers == 2)
		{
			m_pDevice->SetTexture(1, m_StaticBuffers[actual_id].Layers[1].THandle->pTexture);

			m_pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
			m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
			m_pDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);

			m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
			m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			m_pDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
			m_pDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			m_pDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
			m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0);
		}
	}
	else
	{
		m_pDevice->SetTexture(0, NULL);
		m_pDevice->SetTexture(1, NULL);
	}

	hres = m_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, StartVertex, m_StaticBuffers[actual_id].NumVerts / 3);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not draw static buffer!!");
		return JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean PolyCache::SetTexture(int32 Stage, jeTexture* Texture)
{
	if (m_TextureStages[Stage] == Texture)
		return JE_TRUE;

	if (FAILED(m_pDevice->SetTexture(Stage, Texture->pTexture)))
		return JE_FALSE;

	m_TextureStages[Stage] = Texture;
	return JE_TRUE;
}

void PolyCache::SetFilterType(uint32 FilterType)
{
	if (m_OldFilterType == FilterType)
		return;

	switch (FilterType)
	{
	case FILTER_TYPE_BILINEAR:
	{
		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		break;
	}

	case FILTER_TYPE_ANISOTROPIC:
	{
		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

		m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		m_pDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

		break;
	}
	};

	m_OldFilterType = FilterType;
}

void PolyCache::SetBlendFunc(D3DBLEND SFunc, D3DBLEND DFunc)
{
	if (m_OldSFunc != SFunc)
	{
		m_pDevice->SetRenderState(D3DRS_SRCBLEND, SFunc);
		m_OldSFunc = SFunc;
	}

	if (m_OldDFunc != DFunc)
	{
		m_pDevice->SetRenderState(D3DRS_DESTBLEND, DFunc);
		m_OldDFunc = DFunc;
	}
}

void PolyCache::EnableAlpha(jeBoolean Enable)
{
	if (m_bOldAlphaEnable == Enable)
		return;

	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, Enable);
	m_bOldAlphaEnable = Enable;
}

void PolyCache::SetTexWrap(int32 Stage, jeBoolean bWrap)
{
	if (m_bOldTexWrap[Stage] == bWrap)
		return;

	if (bWrap)
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