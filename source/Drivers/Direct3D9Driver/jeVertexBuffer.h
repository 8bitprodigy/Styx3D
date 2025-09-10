/****************************************************************************************/
/*  jeVertexBuffer.h	                                                            */
/*                                                                                      */
/*  Author:			Nicholas Anton							                            */
/*	Version:		1.0	, 11/20/2003													*/
/*  Description:	Class to maintain a dynamic vertex buffer							*/
/****************************************************************************************/
#ifndef JE_VERTEX_BUFFER_IMPL_H
#define JE_VERTEX_BUFFER_IMPL_H

#include <d3dx9.h>
#include <assert.h>
#include "dcommon.h"

/****************************************************************************************/
/* enum JVB_LOCK_FLAGS																	*/
/*																						*/
/* Description:		custom flags to encapsulate d3d locking flags						*/
/****************************************************************************************/
enum JVB_LOCK_FLAGS
{
	JVB_LOCKFLAGS_FLUSH  = D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD,
	JVB_LOCKFLAGS_APPEND = D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE
};

/****************************************************************************************/
/* enum JVB_BUFFER_TYPE																	*/
/*																						*/
/* Description:		type of data in buffer												*/
/****************************************************************************************/
enum JVB_BUFFER_TYPE
{
	JVB_TRIANGLE_LIST =0,
	JVB_TRIANGLE_FAN  =1,
	JVB_TRIANGLE_STRIP=2
};

/****************************************************************************************/
/* class jeVertexBuffer																*/
/*																						*/
/* Description:		Jet3D dynamic vertex buffer class									*/
/****************************************************************************************/
class jeVertexCache 
{
	private :
		LPDIRECT3DVERTEXBUFFER9								m_pVB;
		D3DVERTEXBUFFER_DESC								m_Desc;
		unsigned int										m_iVertexCount;
		unsigned int										m_iPosition;
		unsigned int										m_iSize;
		bool												m_bLocked;
		bool												m_bFlush;
		DWORD												m_FVF;
		JVB_BUFFER_TYPE										m_Type;
		void*												pLockedData;	
	public :
		~jeVertexCache();
		jeVertexCache(	const LPDIRECT3DDEVICE9 lpD3D,const DWORD& FVF,
						const unsigned int& VertexCount, unsigned int Size, JVB_BUFFER_TYPE bufType);

		void*												GetInterface();
		void												SetVertexBuffer(void* vb);
		unsigned int										GetSize();
		DWORD												GetFVF();
		JVB_BUFFER_TYPE										GetType();
		unsigned int										GetVertexCount();
		UINT												GetPosition();
		void*												Lock( const unsigned int LockCount, 
															unsigned int *StartVertex );											
		void												Unlock();	
		void												FlushAtFrameStart();
};

#endif // JE_VERTEX_BUFFER_IMPL_H