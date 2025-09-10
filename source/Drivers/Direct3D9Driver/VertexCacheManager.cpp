#include "VertexCacheManager.h"

VertexCacheManager::VertexCacheManager(IDirect3DDevice9 *pDevice)
{
	m_pDevice = pDevice;
	m_pDevice->AddRef();

	m_StaticBuffers.clear();

	for (DWORD i = 0; i < MAX_VCACHES; i++)
	{
		m_WorldVertexCache[i] = new VertexCache(m_pDevice, i, D3DFVF_WORLDVERTEX, sizeof(D3DFVF_WORLDVERTEX));
		m_RegularVertexCache[i] = new VertexCache(m_pDevice, i, D3DFVF_REGULARVERTEX, sizeof(D3DFVF_REGULARVERTEX));
	}

	m_ShadedVertexCache = new VertexCache(m_pDevice, i, D3DFVF_SHADEDVERTEX, sizeof(D3DFVF_SHADEDVERTEX));
	m_ActiveCache = 0;
}

VertexCacheManager::~VertexCacheManager()
{
	std::list<StaticBuffer*>::iterator			i;

	// Release the static buffers
	i = m_StaticBuffers.begin();
	while (i != m_StaticBuffers.end())
	{
		for (int j = 0; j < MAX_LAYERS; j++)
			(*i)->Texture[j] = NULL;

		(*i)->pBuffer->Release();
		(*i)->pBuffer = NULL;

		delete (*i);
		i++;
	}

	m_StaticBuffers.clear();

	// Release dynamic cache
	for (DWORD j = 0; j < MAX_VCACHES; j++)
	{
		delete m_WorldVertexCache[j];
		delete m_RegularVertexCache[j];
		
		m_WorldVertexCache[j] = NULL;
		m_RegularVertexCache[j] = NULL;
	}

	delete m_ShadedVertexCache;
	m_ShadedVertexCache = NULL;

	m_pDevice->Release();
	m_pDevice = NULL;
}

jeBoolean VertexCacheManager::CreateStaticBuffer(jeTexture *Texture, int numVerts, const void *pVerts, DWORD *id)
{
	HRESULT						hres;
	void						*pData = NULL;
	StaticBuffer				*sb = NULL;

	sb = new StaticBuffer;
	if (!sb)
		return JE_FALSE;

	sb->NumVerts = numVerts;
	sb->NumTris = numVerts / 3;
	sb->Stride = sizeof(D3DFVF_REGULARVERTEX);

	hres = m_pDevice->CreateVertexBuffer(numVerts * sb->Stride, D3DUSAGE_WRITEONLY, D3DFVF_REGULARVERTEX, D3DPOOL_DEFAULT, &sb->pBuffer, NULL);
	if (FAILED(hres))
	{
		delete sb;
		sb = NULL;

		return JE_FALSE;
	}

	hres = sb->pBuffer->Lock(0, 0, &pData, 0);
	if (FAILED(hres))
	{
		sb->pBuffer->Release();
		sb->pBuffer = NULL;

		delete sb;
		sb = NULL;

		return JE_FALSE;
	}

	memcpy(pData, pVerts, numVerts * sb->Stride);

	hres = sb->pBuffer->Unlock();
	if (FAILED(hres))
	{
		sb->pBuffer->Release();
		sb->pBuffer = NULL;

		delete sb;
		sb = NULL;

		return JE_FALSE;
	}

	*id = (DWORD)m_StaticBuffers.size();
	m_StaticBuffers.push_back(sb);

	return JE_TRUE;
}

jeBoolean VertexCacheManager::Flush(DWORD id)
{
	if (id >= MAX_VCACHES)
		return JE_FALSE;

	return m_WorldVertexCache[id]->Flush();
}

jeBoolean VertexCacheManager::FlushAll()
{
	for (DWORD i = 0; i < MAX_VCACHES; i++)
	{
		if (!m_WorldVertexCache[i]->Flush())
			return JE_FALSE;

		if (!m_RegularVertexCache[i]->Flush())
			return JE_FALSE;
	}

	if (!m_ShadedVertexCache->Flush())
		return JE_FALSE;

	return JE_TRUE;
}

jeBoolean VertexCacheManager::RenderStaticBuffer(DWORD id)
{
	HRESULT											hres;
	std::list<StaticBuffer*>::iterator				i;

	// If there are no static buffers, there's nothing to render
	if (m_StaticBuffers.empty())
		return JE_FALSE;

	// Find the buffer with the given id
	i = m_StaticBuffers.begin();
	for (DWORD j = 0; j < id; j++)
		i++;

	if (i == m_StaticBuffers.end())
		return JE_FALSE;

	m_pDevice->SetFVF(D3DFVF_REGULARVERTEX);
	m_pDevice->SetVertexShader(NULL);

	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	hres = m_pDevice->SetStreamSource(0, (*i)->pBuffer, 0, (*i)->Stride);
	if (FAILED(hres))
		return JE_FALSE;

	hres = m_pDevice->SetTexture(0, (*i)->Texture[0]->pTexture);
	if (FAILED(hres))
		return JE_FALSE;

	m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    
	m_pDevice->SetRenderState(D3DRS_ALPHAREF, 50);
    m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    
	hres = m_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, (*i)->NumTris);
	if (FAILED(hres))
		return JE_FALSE;

	return JE_TRUE;
}

jeBoolean VertexCacheManager::RenderWorldVertex(int numVerts, const void *pVerts, jeTexture **Textures, int numTextures)
{
	// Make sure there is at least one texture
	if (numTextures == 0 || Textures == NULL)
		return JE_FALSE;

	// Check if a buffer uses the same texture
	for (DWORD i = 0; i < MAX_VCACHES; i++)
	{
		if (!m_WorldVertexCache[i]->IsEmpty())
		{
			if (m_WorldVertexCache[i]->GetTexture(0) == Textures[0])
				return m_WorldVertexCache[i]->Add(numVerts, pVerts);
		}
	}

	// Check for an empty cache
	for (i = 0; i < MAX_VCACHES; i++)
	{
		if (m_WorldVertexCache[i]->IsEmpty())
		{
			for (int j = 0; j < numTextures; j++)
				m_WorldVertexCache[i]->SetTexture(j, Textures[j]);

			return m_WorldVertexCache[i]->Add(numVerts, pVerts);
		}
	}

	// Check for the fullest cache and flush it
	VertexCache				*pCache = m_WorldVertexCache[0];

	for (i = 0; i < MAX_VCACHES; i++)
	{
		if (m_WorldVertexCache[i]->GetNumVerts() > pCache->GetNumVerts())
			pCache = m_WorldVertexCache[i];
	}

	if (!pCache->Flush())
		return JE_FALSE;

	for (int j = 0; j < numTextures; j++)
		pCache->SetTexture(j, Textures[j]);

	return pCache->Add(numVerts, pVerts);
}

jeBoolean VertexCacheManager::RenderRegularVertex(int numVerts, const void *pVerts, jeTexture *Texture)
{
	// Find a cache
	for (DWORD i = 0; i < MAX_VCACHES; i++)
	{
		if (!m_RegularVertexCache[i]->IsEmpty())
		{
			if (m_RegularVertexCache[i]->GetTexture(0) == Texture)
				return m_RegularVertexCache[i]->Add(numVerts, pVerts);
		}
	}

	// Find an empty cache
	for (i = 0; i < MAX_VCACHES; i++)
	{
		if (m_RegularVertexCache[i]->IsEmpty())
		{
			m_RegularVertexCache[i]->SetTexture(0, Texture);
			return m_RegularVertexCache[i]->Add(numVerts, pVerts);
		}
	}

	// Find the fullest buffer and flush it
	VertexCache				*pCache = m_RegularVertexCache[0];

	for (i = 0; i < MAX_VCACHES; i++)
	{
		if (m_RegularVertexCache[i]->GetNumVerts() > pCache->GetNumVerts())
			pCache = m_RegularVertexCache[i];
	}

	if (!pCache->Flush())
		return JE_FALSE;

	pCache->SetTexture(0, Texture);
	return pCache->Add(numVerts, pVerts);
}

jeBoolean VertexCacheManager::RenderShadedVertex(int numVerts, const void *pVerts)
{
	if (m_ShadedVertexCache->GetNumVerts() + numVerts >= MAX_VERTS)
		m_ShadedVertexCache->Flush();

	return m_ShadedVertexCache->Add(numVerts, pVerts);
}

VertexCache::VertexCache(IDirect3DDevice9 *pDevice, DWORD id, DWORD fvf, DWORD stride)
{
	HRESULT					hres;

	m_ID = id;
	m_FVF = fvf;
	m_Stride = stride;

	m_NumVerts = 0;

	m_pDevice = pDevice;
	m_pDevice->AddRef();

	hres = pDevice->CreateVertexBuffer(MAX_VERTS * m_Stride, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVB, NULL);
	if (FAILED(hres))
		m_pVB = NULL;

	for (int i = 0; i < MAX_LAYERS; i++)
		m_pTexture[i] = NULL;
}

VertexCache::~VertexCache()
{
	if (m_pVB)
		m_pVB->Release();

	if (m_pDevice)
		m_pDevice->Release();

	m_pVB = NULL;
	m_pDevice = NULL;
}

jeBoolean VertexCache::Add(DWORD numVerts, const void *pVerts)
{
	HRESULT						hres;
	DWORD						flags = 0;
	int							pos = 0;
	void						*pData = NULL;

	// Check if buffer is full
	if (numVerts + m_NumVerts > MAX_VERTS)
		Flush();

	// Calculate flags
	if (GetNumVerts() == 0)
	{
		pos = 0;
		flags = D3DLOCK_DISCARD;
	}
	else
	{
		pos = m_Stride * (int)GetNumVerts();
		flags = D3DLOCK_NOOVERWRITE;
	}

	// Lock the buffer
	hres = m_pVB->Lock(pos, numVerts * m_Stride, &pData, flags);
	if (FAILED(hres))
		return JE_FALSE;

	memcpy(pData, pVerts, numVerts * m_Stride);

	hres = m_pVB->Unlock();
	if (FAILED(hres))
		return JE_FALSE;

	m_NumVerts += numVerts;

	return JE_TRUE;
}

jeBoolean VertexCache::Flush()
{
	HRESULT							hres;

	if (m_NumVerts <= 0)
		return JE_TRUE;

	m_pDevice->SetFVF(m_FVF);
	m_pDevice->SetVertexShader(NULL);

	for (int j = 0; j < MAX_LAYERS; j++)
	{
		if (m_pTexture[j] != NULL)
		{
			hres = m_pDevice->SetTexture(j, m_pTexture[j]->pTexture);
			if (FAILED(hres))
				return JE_FALSE;
		}
	}

	hres = m_pDevice->SetStreamSource(0, m_pVB, 0, m_NumVerts * m_Stride);
	if (FAILED(hres))
		return JE_FALSE;

	m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	m_pDevice->SetRenderState(D3DRS_ALPHAREF, 50);
    m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    m_pDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
    m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    
	hres = m_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_NumVerts / 3);
	if (FAILED(hres))
		return JE_FALSE;

	m_NumVerts = 0;

	return JE_TRUE;
}
