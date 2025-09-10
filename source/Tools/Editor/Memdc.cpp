#include "stdafx.h"
#include "MemDC.h"
#include <Limits.h>  // for ULONG_MAX


CBufferMemDC::CBufferMemDC()
{
	m_pBitmap = NULL;
	m_IsDirty = TRUE;
	m_LowMemory = FALSE;
	m_SizeBitmap.SetRectEmpty();
};


BOOL CBufferMemDC::SetSizeBuffer( CDC *pDC, CRect rect )
{
	if (m_pBitmap)	// if we have a buffer
	{
					// if the size of the buffer has been changed we have to recreate the bitmap
		if ( (m_SizeBitmap.Width() < rect.Width() ) | (m_SizeBitmap.Height() < rect.Height() ))
			Reset();

//		if (m_SizeBitmap != rect)
//			Reset();
	}

							// if we have no buffer we create a new one. If we had no memory problems
	if (!m_pBitmap && !m_LowMemory)		// save the size of the buffer
	{
		m_SizeBitmap = rect;

		m_pBitmap = new CBitmap;
		if (!m_pBitmap->CreateCompatibleBitmap(pDC, m_SizeBitmap.Width(), 
				m_SizeBitmap.Height()))
		{
			AfxMessageBox("Cannot allocate background bitmap for screen buffering. Perhaps your system has not much memory. Screen buffering will be turned off");
			m_pBitmap = NULL;
		}
	}
	return m_pBitmap != NULL;
}


CBufferMemDC::~CBufferMemDC()
{
	delete m_pBitmap;
}


void CBufferMemDC::Reset()
{
	delete m_pBitmap; 
	m_pBitmap = NULL;
	m_IsDirty = TRUE;
	m_SizeBitmap.SetRectEmpty();
}


BOOL CBufferMemDC::IsDirty()
{
	return m_pBitmap == NULL || m_IsDirty;
}


void CBufferMemDC::EndModify()
{
	m_IsDirty = FALSE;
}


CBitmap* CBufferMemDC::GetBitmap()
{
	return m_pBitmap;
}


CRect CBufferMemDC::GetSizeBuffer() const
{
	ASSERT_VALID(m_pBitmap);
	return m_SizeBitmap;
}


void CBufferMemDC::Dirty()
{
	m_IsDirty = TRUE;
}


void CBufferMemDC::OnScroll( int dx, int dy)
{
	if (!m_pBitmap)
		return;

	CDC dcBuffer;
	dcBuffer.CreateCompatibleDC(NULL);
	CBitmap* pOldBitmap = dcBuffer.SelectObject( m_pBitmap );

	int xFrom;
	int xTo;
	
	if (dx>0)
	{
		xFrom = m_SizeBitmap.left+dx;
		xTo = m_SizeBitmap.left;
	} 
	else 
	{
		xFrom = m_SizeBitmap.left;
		xTo = m_SizeBitmap.left-dx;
	}

	int yFrom;
	int yTo;
	
	if (dy>0)
	{
		yFrom = m_SizeBitmap.left+dy;
		yTo = m_SizeBitmap.left;
	} 
	else 
	{
		yFrom = m_SizeBitmap.left;
		yTo = m_SizeBitmap.left-dy;
	}
		
	dcBuffer.BitBlt(xFrom, yFrom, m_SizeBitmap.Width()-abs(dx), m_SizeBitmap.Height()-abs(dy), &dcBuffer, xTo, yTo, SRCCOPY);	

	dcBuffer.SelectObject(pOldBitmap);
}


IMPLEMENT_DYNAMIC( CMemDC, CDC )


CMemDC::CMemDC(CDC* pDC, CRect rectClientWnd, CRect clipRect, CBufferMemDC* pBufferMemDC) 
: CDC(), m_oldBitmap(NULL), m_pDC(pDC), m_ClipRect(clipRect)
{

	ASSERT(m_pDC != NULL); // If you asserted here, you passed in a NULL CDC.

	m_LastCopyToScreen = GetTickCount();
	
					// store the buffer we should us in this instance
	ASSERT_VALID( pBufferMemDC );
	m_pBufferMemDC = pBufferMemDC;

					// disable buffering if we are printing
	m_bMemDC = !pDC->IsPrinting();

	if (m_bMemDC)
	{
		CreateCompatibleDC(pDC);	// Create a Memory DC

// if we have no clip rect we set the clip rect we interpret this as there is no
// clip rect set. So we set the clip rect of the memory bitmap to the client rect

		if (m_ClipRect.Width() == 0 || m_ClipRect.Height() == 0)
			m_ClipRect = rectClientWnd;

// assure the buffers main area is the same as the main area supplied in the constructor

		if (!m_pBufferMemDC->SetSizeBuffer(pDC, rectClientWnd))
		{
					// if we have problems with buffering we turn off buffering
			DeleteDC();
			m_ClipRect.SetRectEmpty();
			m_bMemDC = FALSE;

		} 
		else 
		{
					// select the bitmap of the buffer for writing
			m_oldBitmap = SelectObject( m_pBufferMemDC->GetBitmap() );

					// set the clip rect of the memory bitmap to the same as the clip rect of pDC
			IntersectClipRect( m_ClipRect );

					// set the origin of all drawing operations to the top left coordinate of the buffer. This 
					// should always be 0/0. If not i want to be informed about this

			ASSERT(rectClientWnd.left == 0 && rectClientWnd.top == 0);

			SetWindowOrg(rectClientWnd.left, rectClientWnd.top);
		}

	}
// if we are printing or if we have problems with the buffer we bypass pDC to this by
// making a copy of the relevent parts of the current DC for printing

	if (!m_bMemDC)
	{
		m_bPrinting = pDC->m_bPrinting;
		m_hDC = pDC->m_hDC;
		m_hAttribDC = pDC->m_hAttribDC;
	}
}

//------------------------------------------------------------------------------

CMemDC::~CMemDC()
{
// we don't force a copy to the screen in the destructor because we want to be free
// when to do this
	if (m_bMemDC) 
	{
		SelectObject(m_oldBitmap);	//Swap back the original bitmap.
	}
	else 
	{
		// All we need to do is replace the DC with an illegal value,
		// this keeps us from accidently deleting the handles associated with
		// the CDC that was passed to the constructor.
		m_hDC = m_hAttribDC = NULL;
	}
}

//----------------------------------------------------------------------

static DWORD MillisecondsElapsed( DWORD t1 )
{
	DWORD t2 = GetTickCount();	// we have the situation where the clock jumps back to zero
	
	if (t2>=t1)
		return t2-t1;
	else
		return (ULONG_MAX-t1)+t2;
}

//------------------------------------------------------------------------------

BOOL CMemDC::CopyToScreen( DWORD interval )
{
	if ( interval==0 || MillisecondsElapsed( m_LastCopyToScreen ) > interval )
	{
		if (m_bMemDC)
		{
				// copy the part of the clip rect to the screen
			m_pDC->BitBlt(m_ClipRect.left, m_ClipRect.top, m_ClipRect.Width(), m_ClipRect.Height(), this, m_ClipRect.left, m_ClipRect.top, SRCCOPY);
			m_LastCopyToScreen = GetTickCount();
			return TRUE;
		}
	}
	return FALSE;
}

//------------------------------------------------------------------------------

void CMemDC::CopyFrom( CBufferMemDC* pBufferFrom )
{
	ASSERT_VALID( pBufferFrom );
	ASSERT_VALID( m_pBufferMemDC); // buffer deleted in the mean time?
	ASSERT( pBufferFrom != m_pBufferMemDC );

// bitblt from pBufferMemDC to this. It won't be visible because m_pDC is the device context of
// the screen

	if (m_bMemDC && pBufferFrom && m_pBufferMemDC)
	{
		CDC dcBufferFrom;
		dcBufferFrom.CreateCompatibleDC(m_pDC);
		CBitmap* pOldBitmap = dcBufferFrom.SelectObject( pBufferFrom->GetBitmap() );

		CRect rectBufferFrom( pBufferFrom->GetSizeBuffer() );
		dcBufferFrom.SetWindowOrg(rectBufferFrom.left, rectBufferFrom.top);

		BitBlt(m_ClipRect.left, m_ClipRect.top, m_ClipRect.Width(), m_ClipRect.Height(), &dcBufferFrom, m_ClipRect.left, m_ClipRect.top, SRCCOPY);

		dcBufferFrom.SelectObject(pOldBitmap);
	}
}

//------------------------------------------------------------------------------

void CMemDC::CopyTo( CBufferMemDC* pBufferTo )
{
	ASSERT_VALID( pBufferTo );
	
	if (m_bMemDC && pBufferTo && pBufferTo->GetBitmap() ) 
	{
		ASSERT_VALID( m_pBufferMemDC); // buffer deleted in the mean time?
		ASSERT( !m_pBufferMemDC->IsDirty() );
		ASSERT( pBufferTo != m_pBufferMemDC );

						// bitblt from this to a cdc based on pBufferTo

		CDC dcBufferTo;
		dcBufferTo.CreateCompatibleDC(m_pDC);
		CBitmap* pOldBitmap = dcBufferTo.SelectObject( pBufferTo->GetBitmap() );

		CRect rectBufferTo( pBufferTo->GetSizeBuffer() );
		dcBufferTo.SetWindowOrg(rectBufferTo.left, rectBufferTo.top);

		dcBufferTo.BitBlt(m_ClipRect.left, m_ClipRect.top, m_ClipRect.Width(), m_ClipRect.Height(), this, m_ClipRect.left, m_ClipRect.top, SRCCOPY);
		
		dcBufferTo.SelectObject(pOldBitmap);
	}
}

//------------------------------------------------------------------------------

CDC* CMemDC::GetSafeCDC()
{
	if (m_bMemDC)
		return this;
	else
		return m_pDC;
}


