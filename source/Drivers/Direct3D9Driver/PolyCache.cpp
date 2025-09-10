#include <assert.h>
#include <algorithm>
#include <vector>
#include "PolyCache.h"
#include "D3D9Log.h"

struct TextureSortFunctor
{
	bool operator ()(PolyCacheEntry &e1, PolyCacheEntry &e2)
	{
		if (e1.NumLayers != e2.NumLayers)
			return false;

		if (e1.Layers[0]->id == e2.Layers[0]->id)
		{
			if (e1.NumLayers > 1)
			{
				if (e1.Layers[1]->id == e2.Layers[1]->id)
					return true;
			}
			else
				return true;
		}

		return false;
	}
};

//=====================================================================================
//	Log2
//	Return the log of a size
//=====================================================================================
static uint32 Log2(uint32 P2)
{
	uint32		p = 0;
	int32		i = 0;
	
	for (i = P2; i > 0; i>>=1)
		p++;

	return (p-1);
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
static void FillLMapSurface(jeTLVertex *Pnts,int32 NumPoints,jeTexture *THandle, jeRDriver_LMapCBInfo *Info, int32 LNum)
{
	U16					*pTempBits,*pTempBits2;
	int32				w, h, Width, Height, Size;
	U8					*pBitPtr;
	RGB_LUT				*Lut;
	int32				Extra;
	//U8					PrevR,PrevG,PrevB;

	pBitPtr = (U8*)Info->RGBLight[LNum];

	Width = THandle->Width;
	Height = THandle->Height;
	Size = THandle->Log;

	Lut = &Lut1;

	D3D9_THandle_Lock(THandle, 0, (void**)&pTempBits);
	pTempBits2=pTempBits;

	Extra = (THandle->stride-THandle->Width);

	for (h=0; h< Height; h++)
	{
		for (w=0; w< Width; w++)
		{
			U8	R, G, B;
			U16	Color;
	
			R = *pBitPtr++;
			G = *pBitPtr++;
			B =  *pBitPtr++;
			
			Color = (U16)((Lut->R[R] | Lut->G[G] | Lut->B[B])&0xFFFF);

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
static jeBoolean PrepPolyVerts(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers,void *LMapCBContext)
{
	float			InvScale, u, v;
	float			ShiftU, ShiftV, ScaleU, ScaleV;
	jeTLVertex		*TempVerts,*pTVerts, *pVerts;
	//float			du, dv, dx, dy, MipScale;
	int32			j;
//	D3DSURFACE_DESC				desc;

	float				InvScale2, ShiftU2, ShiftV2;
	jeRDriver_Layer		*LMapLayer;
	jeRDriver_Layer		*TexLayer;

	TempVerts = (jeTLVertex*)malloc(sizeof(jeTLVertex)*NumPoints);
	memcpy(TempVerts,Pnts,sizeof(jeTLVertex)*NumPoints);

	pTVerts = &TempVerts[0];

	// Set up shifts and scaled for texture uv's
	TexLayer = &Layers[0];
	LMapLayer = &Layers[1];

	// Normalize UV's using Texture Size
	InvScale = 1.0f / (float)((1<<TexLayer->THandle->Log));

	ShiftU = TexLayer->ShiftU;
	ShiftV = TexLayer->ShiftV;
	ScaleU = (1.0f/(TexLayer->ScaleU));
	ScaleV = (1.0f/(TexLayer->ScaleV));

	if(NumLayers>1)
	{
		// Set up shifts and scaled for lightmap uv's
		ShiftU2 = (float)-LMapLayer->ShiftU + 8.0f;
		ShiftV2 = (float)-LMapLayer->ShiftV + 8.0f;
		
		InvScale2 = 1.0f/(float)((1<<LMapLayer->THandle->Log)<<4);
	}

	pVerts = &Pnts[0];

	for (j=0; j<NumPoints; j++)
	{

		u = pTVerts->u*ScaleU+ShiftU;
		v = pTVerts->v*ScaleV+ShiftV;

		pVerts->u = u * InvScale;
		pVerts->v = v * InvScale;

		if(NumLayers>1)
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
	m_Cache.clear();
	m_NumVerts = 0;

	m_StaticBuffers.clear();
}

PolyCache::~PolyCache()
{
	Shutdown();
}

jeBoolean PolyCache::Initialize(IDirect3DDevice9 *pDevice)
{
	HRESULT								hres;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  PolyCache::Initialize()");
	else
		REPORT("Function Call:  PolyCache::Initialize()");

	m_pDevice = pDevice;
	m_pDevice->AddRef();

	hres = m_pDevice->CreateVertexBuffer(sizeof(PolyVert) * 25000, D3DUSAGE_DYNAMIC, J3D_FVF, D3DPOOL_DEFAULT, &m_pVB, NULL);
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:   Could not create vertex buffer!!");
		return JE_FALSE;
	}

	m_NumVerts = 0;

	return JE_TRUE;
}

void PolyCache::Shutdown()
{
	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  PolyCache::Shutdown()");
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
	m_Cache.clear();
}

jeBoolean PolyCache::AddMiscTexturePoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags)
{
	HRESULT							hres;
	PolyVert						*verts = NULL, *data = NULL;
	PolyCacheEntry					*p = NULL;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  PolyCache::AddMiscTexturePoly()");
	else
		REPORT("Function Call:  PolyCache::AddMiscTexturePoly()");

	m_Cache.resize(m_Cache.size() + 1);
	p = &m_Cache[m_Cache.size() - 1];

	memset(p, 0, sizeof(PolyCacheEntry));

	p->StartVertex = m_NumVerts;

	verts = new PolyVert[NumPoints];
	if (!verts)
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Out of memory!!");
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

		verts[i].diffuse = D3DCOLOR_ARGB((DWORD)Pnts[i].a, (DWORD)Pnts[i].r, (DWORD)Pnts[i].g, (DWORD)Pnts[i].b);
	}

	if (m_NumVerts == 0)
		hres = m_pVB->Lock(0, sizeof(PolyVert) * NumPoints, (void**)&data, D3DLOCK_DISCARD);
	else
		hres = m_pVB->Lock(m_NumVerts * sizeof(PolyVert), sizeof(PolyVert) * NumPoints, (void**)&data, D3DLOCK_NOOVERWRITE);

	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not lock vertex buffer!!");
		SAFE_DELETE_ARRAY(verts);
		return JE_FALSE;
	}

	memcpy(data, verts, sizeof(PolyVert) * NumPoints);

	hres = m_pVB->Unlock();
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not unlock vertex buffer!!");
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

jeBoolean PolyCache::AddGouraudPoly(jeTLVertex *Pnts, int32 NumPoints, uint32 Flags)
{	
	HRESULT							hres;
	PolyVert						*verts = NULL, *data = NULL;
	PolyCacheEntry					*p = NULL;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  PolyCache::AddGouraudPoly()");
	else
		REPORT("Function Call:  PolyCache::AddGouraudPoly()");

	m_Cache.resize(m_Cache.size() + 1);
	p = &m_Cache[m_Cache.size() - 1];

	memset(p, 0, sizeof(PolyCacheEntry));

	p->StartVertex = m_NumVerts;

	verts = new PolyVert[NumPoints];
	if (!verts)
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Out of memory!!");
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

		verts[i].diffuse = D3DCOLOR_ARGB((DWORD)Pnts[i].a, (DWORD)Pnts[i].r, (DWORD)Pnts[i].g, (DWORD)Pnts[i].b);
	}

	if (m_NumVerts == 0)
		hres = m_pVB->Lock(0, sizeof(PolyVert) * NumPoints, (void**)&data, D3DLOCK_DISCARD);
	else
		hres = m_pVB->Lock(m_NumVerts * sizeof(PolyVert), sizeof(PolyVert) * NumPoints, (void**)&data, D3DLOCK_NOOVERWRITE);

	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not lock vertex buffer!!");
		SAFE_DELETE_ARRAY(verts);
		return JE_FALSE;
	}

	memcpy(data, verts, sizeof(PolyVert) * NumPoints);

	hres = m_pVB->Unlock();
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not unlock vertex buffer!!");
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

jeBoolean PolyCache::AddWorldPoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags)
{
	HRESULT							hres;
	PolyCacheEntry					*p = NULL;
	PolyVert						*verts = NULL, *data = NULL;
	jeRDriver_LMapCBInfo			LMapInfo;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  PolyCache::AddWorldPoly()");
	else
		REPORT("Function Call:  PolyCache::AddWorldPoly()");

	m_Cache.resize(m_Cache.size() + 1);
	p = &m_Cache[m_Cache.size() - 1];

	memset(p, 0, sizeof(PolyCacheEntry));

	p->StartVertex = m_NumVerts;

	if (LMapCBContext && NumLayers > 1)
	{
		g_D3D9Drv.SetupLightmap(&LMapInfo, LMapCBContext);
		if (!Layers[1].THandle)
			D3D9Log::GetPtr()->Printf("WARNING:  No lightmap!!");

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

	verts = new PolyVert[NumPoints];
	if (!verts)
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Out of memory!!");
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

		verts[j].diffuse = D3DCOLOR_ARGB((DWORD)Pnts[j].a, (DWORD)Pnts[j].r, (DWORD)Pnts[j].g, (DWORD)Pnts[j].b);
	}

	if (m_NumVerts == 0)
		hres = m_pVB->Lock(0, sizeof(PolyVert) * NumPoints, (void**)&data, D3DLOCK_DISCARD);
	else
		hres = m_pVB->Lock(m_NumVerts * sizeof(PolyVert), sizeof(PolyVert) * NumPoints, (void**)&data, D3DLOCK_NOOVERWRITE);

	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not lock vertex buffer!!");
		SAFE_DELETE_ARRAY(verts);
		return JE_FALSE;
	}

	memcpy(data, verts, sizeof(PolyVert) * NumPoints);

	hres = m_pVB->Unlock();
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not unlock vertex buffer!!");
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

jeBoolean PolyCache::Flush()
{
	HRESULT							hres;
	int32							previd = -1;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  PolyCache::Flush()");
	else
		REPORT("Function Call:  PolyCache::Flush()");

	std::sort(m_Cache.begin(), m_Cache.end(), TextureSortFunctor());

	m_pDevice->SetFVF(J3D_FVF);
	m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(PolyVert));

	for (size_t i = 0; i < m_Cache.size(); i++)
	{
		if (m_Cache[i].NumLayers > 0)
		{
			if (m_Cache[i].NumLayers == 0 && previd != -1)
			{
				m_pDevice->SetTexture(0, NULL);
				m_pDevice->SetTexture(1, NULL);
				previd = -1;
			}
			else if (previd != m_Cache[i].Layers[0]->id)
			{
				previd = m_Cache[i].Layers[0]->id;
				m_pDevice->SetTexture(0, m_Cache[i].Layers[0]->pTexture);
				m_pDevice->SetTexture(1, NULL);
			}

			m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
			m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
			m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			if (m_Cache[i].Flags & JE_RENDER_FLAG_BILINEAR_FILTER)
			{
				m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
				m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
				m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

				m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
				m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
				m_pDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			}
			else
			{
				m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
				m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
				m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

				m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
				m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
				m_pDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
			}

			if (m_Cache[i].Flags & JE_RENDER_FLAG_CLAMP_UV)
			{
				m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
				m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			}
			else
			{
				m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			}

			if (m_Cache[i].NumLayers > 1)
			{
				m_pDevice->SetTexture(1, m_Cache[i].Layers[1]->pTexture);

				m_pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
				m_pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				m_pDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
				m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
				
				m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
				m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

				m_pDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
				m_pDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
								
				m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			}
			else
			{
				if (m_Cache[i].Flags & JE_RENDER_FLAG_ALPHA)
					m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				else
					m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			}

			if (m_Cache[i].Flags & JE_RENDER_FLAG_ALPHA)
				m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			else
				m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

			if (m_Cache[i].Flags & JE_RENDER_FLAG_WIREFRAME)
				m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			else
				m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

			m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
			
			//if (m_Cache[i].Flags & JE_RENDER_FLAG_COUNTER_CLOCKWISE)
			//	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			//else
				m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

			if (m_Cache[i].Flags & JE_RENDER_FLAG_NO_ZTEST)
				m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
			else
				m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

			if (m_Cache[i].Flags & JE_RENDER_FLAG_NO_ZWRITE)
				m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			else
				m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

			hres = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, m_Cache[i].StartVertex, m_Cache[i].NumVertices - 2);
			if (FAILED(hres))
			{
				D3D9Log::GetPtr()->Printf("ERROR:  DrawPrimitive() failed!!");
				return JE_FALSE;
			}
		}
	}

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("DEBUG:  Poly cache rendered %d vertices", m_NumVerts);

	m_Cache.clear();
	m_NumVerts = 0;

	return JE_TRUE;
}

uint32 PolyCache::AddStaticBuffer(jeHWVertex *Points, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags)
{
	HRESULT								hres;
	uint32								currIndex;
	void								*data = NULL;
	StaticBuffer						*buffer = NULL;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  PolyCache::AddStaticBuffer()");
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
		D3D9Log::GetPtr()->Printf("ERROR:  Could not create static vertex buffer!!");
		return 0;
	}

	buffer->Active = JE_TRUE;
	buffer->Layers = Layers;
	buffer->NumLayers = NumLayers;
	buffer->NumVerts = NumPoints;

	hres = buffer->pVB->Lock(0, sizeof(jeHWVertex) * NumPoints, &data, 0);
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not lock static vertex buffer!!");
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
		D3D9Log::GetPtr()->Printf("ERROR:  Could not unlock static vertex buffer!!");
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
		D3D9Log::GetPtr()->Printf("Function Call:  PolyCache::RemoveStaticBuffer()");
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

jeBoolean PolyCache::RenderStaticBuffer(uint32 id, int32 StartVertex, int32 NumPolys, jeXForm3d *XForm)
{
	uint32							actual_id = id - 1;
	HRESULT							hres;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  PolyCache::RenderStaticBuffer()");
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
		D3D9Log::GetPtr()->Printf("ERROR:  Could not draw static buffer!!");
		return JE_FALSE;
	}

	return JE_TRUE;
}
