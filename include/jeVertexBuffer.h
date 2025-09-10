/*!
	@file jeVertexBuffer.h
	@author Anthony Rufrano (paradoxnj)
	@brief A vertex buffer
*/
#ifndef JE_VERTEXBUFFER_H
#define JE_VERTEXBUFFER_H

#include "BaseType.h"
#include "jeMaterial.h"

class jeVertexBuffer : virtual public jeUnknown
{
protected:
	virtual ~jeVertexBuffer()						{}

public:
	/*!
		@fn int16 jeVertexBuffer::AddVertices(jeTLVertex *Pnts, int32 NumPoints)
		@brief Adds vertices to the vertex buffer
		@param[in] Pnts Vertices to add
		@param[in] NumPoints The number of vertices to add
		@return The start index in the array
	*/
	virtual int16					AddVertices(jeTLVertex *Pnts, int32 NumPoints) = 0;

	/*!
		@fn jeBoolean jeVertexBuffer::SetMaterial(jeMaterial *Material)
		@brief Sets the material
		@param[in] Material The material to use
		@return JE_TRUE on success, JE_FALSE on failure
	*/
	virtual jeBoolean				SetMaterial(jeMaterial *Material) = 0;

	/*!
		@fn void jeVertexBuffer::ClearBuffer()
		@brief Empties the vertex buffer
	*/
	virtual void					ClearBuffer() = 0;
};

// End of header
#endif