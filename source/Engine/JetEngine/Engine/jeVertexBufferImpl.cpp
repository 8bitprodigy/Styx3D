#include "jeVertexBufferImpl.h"

jeVertexBufferImpl::jeVertexBufferImpl()
{
	m_Buffer.clear();
	m_pMaterial = NULL;
	m_RefCount = 1;
}

jeVertexBufferImpl::~jeVertexBufferImpl()
{
	m_Buffer.clear();
	m_pMaterial = NULL;
}

uint32 jeVertexBufferImpl::AddRef()
{
	m_RefCount++;
	return m_RefCount;
}

uint32 jeVertexBufferImpl::Release()
{
	m_RefCount--;
	if (m_RefCount == 0)
	{
		delete this;
		return 0;
	}

	return m_RefCount;
}

int16 jeVertexBufferImpl::AddVertices(jeTLVertex *Pnts, int32 NumPoints)
{
	int16						start_vertex = 0;

	start_vertex = (int16)m_Buffer.size();
	m_Buffer.resize((int32)start_vertex + NumPoints);

	for (int16 i = start_vertex, j = 0; i < (start_vertex + (int16)NumPoints); i++, j++)
	{
		m_Buffer[i].x = Pnts[j].x;
		m_Buffer[i].y = Pnts[j].y;
		m_Buffer[i].z = Pnts[j].z;

		m_Buffer[i].u = Pnts[j].u;
		m_Buffer[i].v = Pnts[j].v;

		m_Buffer[i].r = Pnts[j].r;
		m_Buffer[i].g = Pnts[j].g;
		m_Buffer[i].b = Pnts[j].b;
		m_Buffer[i].a = Pnts[j].a;

		m_Buffer[i].sr = Pnts[j].sr;
		m_Buffer[i].sb = Pnts[j].sb;
		m_Buffer[i].sg = Pnts[j].sg;

		m_Buffer[i].pad = Pnts[j].pad;
		m_Buffer[i].pad1 = Pnts[j].pad1;
		m_Buffer[i].pad2 = Pnts[j].pad2;
		m_Buffer[i].pad3 = Pnts[j].pad3;
	}

	return start_vertex;
}

jeBoolean jeVertexBufferImpl::SetMaterial(jeMaterial *Material)
{
	m_pMaterial = Material;
	return JE_TRUE;
}

void jeVertexBufferImpl::ClearBuffer()
{
	m_Buffer.clear();
	m_pMaterial = NULL;
}
