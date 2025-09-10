/****************************************************************************************/
/*  jeVertexCache.cpp                                                              */
/*                                                                                      */
/*  Author:			Nicholas Anton							                            */
/*	Version:		1.0	, 11/20/2003													*/
/*  Description:	Class to maintain a dynamic vertex buffer							*/
/****************************************************************************************/
#include "jeVertexBuffer.h"

jeVertexCache	*jeVertexCache_Create(const void* lpD3D,const DWORD& FVF,
						const unsigned int& VertexCount, unsigned int Size, JVB_BUFFER_TYPE bufType)
{
	return (jeVertexCache*)new jeVertexCache((LPDIRECT3DDEVICE9)lpD3D, FVF,VertexCount,Size,bufType);
}

/****************************************************************************************/
/* jeVertexCache(	const LPDIRECT3DDEVICE9 lpD3D, const DWORD& FVF,				*/
/*					const unsigned int& VertexCount, unsigned int Size );				*/
/*																						*/
/* Description:		Create the jeVertexCache object, and initialize all member			*/
/*					varialbles															*/
/*																						*/
/* Parameters:		LPDIRECT3DDEVICE9	lpD3D			(the direct3d device object)	*/
/*					DWORD&				FVF				(FVF flags to be passed to d3d)	*/
/*					unsigned int		VertexCount		(number of vertices )			*/
/*					unsigned int		Size			(size of vertex struct in bytes)*/
/****************************************************************************************/
jeVertexCache::jeVertexCache( const LPDIRECT3DDEVICE9 lpD3D, const DWORD& FVF, 
								const unsigned int& VertexCount, unsigned int Size, 
								JVB_BUFFER_TYPE bufType )
{
	m_pVB											=0;
	m_iPosition										=0;
	m_bFlush										=true;
	m_bLocked										=false;
	m_iVertexCount									=VertexCount;
	HRESULT		hr									=NULL;
	m_iSize											=Size;
	m_FVF											=FVF;
	m_Type											=bufType;

	if(VertexCount!=0)
	{
	hr= lpD3D->CreateVertexBuffer(	m_iVertexCount * m_iSize,
									D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, FVF,
									D3DPOOL_DEFAULT, &m_pVB, NULL );
	}

	assert(hr == D3D_OK);
//	assert(m_pVB);
}

void jeVertexCache::SetVertexBuffer(void* vb)
{
	this->m_pVB=(LPDIRECT3DVERTEXBUFFER9)vb;
}

/****************************************************************************************/
/* GetVertexCount();																	*/
/*																						*/
/* Description:		Gets the number of vertices in the buffer							*/
/*																						*/
/* Parameters:		none																*/
/*																						*/
/* Return Type:		unsigned int (the # of vertices)									*/
/****************************************************************************************/
unsigned int jeVertexCache::GetVertexCount()
{
	return this->m_iVertexCount;
}

/****************************************************************************************/
/* GetType();																			*/
/*																						*/
/* Description:		Gets the type of data in buffer										*/
/*																						*/
/* Parameters:		none																*/
/*																						*/
/* Return Type:		JVB_BUFFER_TYPE	(type of buffer)									*/
/****************************************************************************************/
JVB_BUFFER_TYPE jeVertexCache::GetType()
{
	return this->m_Type;
}

/****************************************************************************************/
/* GetSize();																			*/
/*																						*/
/* Description:		SIZE DOES MATTER! this returns the size of the FVF structure		*/
/*																						*/
/* Parameters:		none																*/
/*																						*/
/* Return Type:		unsigned int	(size of the FVF)									*/
/****************************************************************************************/
unsigned int jeVertexCache::GetSize()
{
	return this->m_iSize;
}

/****************************************************************************************/
/* GetFVF();																			*/
/*																						*/
/* Description:		return the FVF flags passed on creation								*/
/*																						*/
/* Parameters:		none																*/
/*																						*/
/* Return Type:		DWORD (the FVF flags)												*/
/****************************************************************************************/
DWORD jeVertexCache::GetFVF()
{
	return this->m_FVF;
}

/****************************************************************************************/
/* GetInterface();																		*/
/*																						*/
/* Description:		return a pointer to the vertex buffer								*/
/*																						*/
/* Parameters:		none																*/
/*																						*/
/* Return Type:		LPDIRECT3DVERTEXBUFFER9	(a pointer to the vertex buffer)			*/
/****************************************************************************************/
void* jeVertexCache::GetInterface()
{
	return (void*)m_pVB;
}

/****************************************************************************************/
/* FlushAtFrameStart();																	*/
/*																						*/
/* Description:		Use at beginning of frame to force a flush of VB contents			*/
/*					on first draw														*/
/*																						*/
/* Parameters:		none																*/
/*																						*/
/* Return Type:		void																*/
/****************************************************************************************/
void jeVertexCache::FlushAtFrameStart()
{
	m_bFlush = true;
}

/****************************************************************************************/
/* Lock( const unsigned int& LockCount, unsigned int& StartVertex );					*/
/*																						*/
/* Description:		Lock the vertex buffer and pass a pointer to the data back			*/
/*																						*/
/* Parameters:		unsigned int		LockCount		(number of vertices to lock)	*/
/*					unsigned int		StartVertex		(first vertex to lock)			*/
/*																						*/
/* Return Type:		void** (a pointer to the vertex data)								*/
/****************************************************************************************/
void* jeVertexCache::Lock( const unsigned int LockCount, unsigned int *StartVertex )
{
	*StartVertex										=0;
	pLockedData										=0;
	DWORD			dwFlags							=JVB_LOCKFLAGS_APPEND;
	DWORD			dwSize							=0;
	HRESULT			hr								=NULL;

	if (LockCount > m_iVertexCount) 
	{ 
		assert( false ); 
		return 0; 
	}
	if (m_pVB)
	{
		if (m_bFlush || ( (LockCount + m_iPosition) > m_iVertexCount) )
		{
			m_bFlush								=false;
			m_iPosition								=0;
			dwFlags									=JVB_LOCKFLAGS_FLUSH;
		}
		m_pVB->Lock(	m_iPosition  * m_iSize,LockCount * m_iSize, 
						&pLockedData,dwFlags);

		assert( hr == D3D_OK );

		if ( hr == D3D_OK )
		{
			assert( pLockedData != 0 );
			m_bLocked = true;
			*StartVertex = m_iPosition;
			m_iPosition += LockCount;
		}
	}
	return pLockedData;
}

UINT jeVertexCache::GetPosition()
{
	return this->m_iPosition;
}

/****************************************************************************************/
/* Unlock();																			*/
/*																						*/
/* Description:		Unlocks the vertex buffer, we can now render using it (hopefully)	*/
/*																						*/
/* Parameters:		none																*/
/*																						*/
/* Return Type:		void																*/
/****************************************************************************************/
void jeVertexCache::Unlock()
{
	HRESULT			hr								=NULL;

	if ( ( m_bLocked ) && ( m_pVB ) )
	{
		hr = m_pVB->Unlock();				
		assert( hr == D3D_OK );
		m_bLocked = false;
	}
}

/****************************************************************************************/
/* ~jeVertexCache();																	*/
/*																						*/
/* Description:		Destroy's the vertex buffer object									*/
/*																						*/
/* Parameters:		none																*/
/****************************************************************************************/
jeVertexCache::~jeVertexCache()
{
	this->Unlock();
	if ( m_pVB )
	{
		m_pVB->Release();
	}
}