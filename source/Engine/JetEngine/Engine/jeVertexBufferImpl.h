/*!
	@file jeVertexBufferImpl.h
	@author Anthony Rufrano (paradoxnj)
	@brief The vertex buffer implementation
*/
#ifndef JE_VERTEXBUFFER_IMPL_H
#define JE_VERTEXBUFFER_IMPL_H

#include <vector>
#include "jeVertexBuffer.h"

class jeVertexBufferImpl : public jeVertexBuffer
{
public:
	jeVertexBufferImpl();
	virtual ~jeVertexBufferImpl();

private:
	std::vector<jeTLVertex>					m_Buffer;
	jeMaterial								*m_pMaterial;
	uint32									m_RefCount;

public:
	uint32									AddRef();
	uint32									Release();

	int16									AddVertices(jeTLVertex *Pnts, int32 NumPoints);
	jeBoolean								SetMaterial(jeMaterial *Material);
	void									ClearBuffer();
};

#endif
