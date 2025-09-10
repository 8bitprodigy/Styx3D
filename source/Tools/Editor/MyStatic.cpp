/****************************************************************************************/
/*  MYSTATIC.CPP                                                                        */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description:                                                                        */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.jet3d.com                                                                */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Jet3D, released December 12, 1999.                             */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#include "stdafx.h"

#include "Bmp.h"
#include "jwe.h"
#include "materiallist.h"
#include "Ram.h"

#include "MyStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MARGIN_PAD	(5)
/////////////////////////////////////////////////////////////////////////////
// CMyStatic

CMyStatic::CMyStatic()
{
	m_pBitmap = NULL ;
	m_nTop = 0 ;
	m_nTile = THUMBNAIL_128 ;
}

CMyStatic::~CMyStatic()
{
}


BEGIN_MESSAGE_MAP(CMyStatic, CStatic)
	//{{AFX_MSG_MAP(CMyStatic)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyStatic message handlers

void CMyStatic::OnPaint() 
{
	RECT					rect ;
	CPaintDC				dc(this); // device context for painting
	CDC						MemDC ;
	CJweApp				*	pApp = (CJweApp*)AfxGetApp() ;
	MaterialList_Struct	*	pMaterials ;
	Material_Struct		*	pMaterial = NULL ;
	MaterialIterator		MI ;
#ifdef _USE_BITMAPS
	jeBitmap			*	pBitmap ;
#else
	jeMaterialSpec		*	pMatSpec;
#endif
//	HBITMAP					hBitmap ;
	int						x, y ;
	int						nLeftMargin ;
	int						vy ;
	int						yOffset = 0 ;
	int						srcyOffset ;
	CPen					WhitePen( PS_SOLID, 1, RGB(255,0,0) ) ;
	CPen				*	pOldPen ;
	CRgn					ClipRegion ;
	Material_Struct		*	pCurrentMaterial ;

	pMaterials = pApp->GetMaterialList( ) ;

	pCurrentMaterial = MaterialList_GetCurMaterial( pMaterials ) ;
	MemDC.CreateCompatibleDC( &dc ) ;
	GetClientRect( &rect ) ;
	dc.FillSolidRect( 0, 0, rect.right, rect.bottom, RGB( 0,0,0) ) ;
	pOldPen = dc.SelectObject( &WhitePen ) ;
	ClipRegion.CreateRectRgn( rect.left, rect.top, rect.right, rect.bottom ) ;
	dc.SelectObject( &ClipRegion ) ;

	// Find the start material
	vy = m_nTop / m_nTile ;
	if( m_nTile == THUMBNAIL_64 )
		vy *= 2 ;
	else if( m_nTile == THUMBNAIL_32 )
		vy *= 4 ;
	
	pMaterial = MaterialList_GetFirstMaterial( pMaterials, &MI ) ;
	while( vy-- )
	{
		if( pMaterial == NULL )
			break;
		pMaterial = MaterialList_GetNextMaterial( pMaterials, &MI ) ;
	}
	
	yOffset = m_nTop - ((m_nTop/m_nTile)*m_nTile) ;

	if( m_nTile == THUMBNAIL_128 )
		nLeftMargin = 2 ;
	else
		nLeftMargin = 1 ;
	x = nLeftMargin ;

	if( yOffset == 0 )	// Make a border across the top when the graphic
		y = 1 ;			// isn't partially scrolled
	else
		y = 0 ;

	while( pMaterial != NULL )
	{
#ifdef _USE_BITMAPS
		jeBitmap * Lock;
		pBitmap = (jeBitmap*) Materials_GetBitmap( pMaterial ) ;
		if( pMaterial == pCurrentMaterial )
			dc.FillSolidRect( x-1, y-1, m_nTile+2, m_nTile-yOffset+2, RGB(255,255,255) ) ;  

	//	hBitmap = CreateHBitmapFromgeBitmap( pBitmap, MemDC.m_hDC ) ;

		if( jeBitmap_LockForRead( pBitmap, &Lock, 0, 0, JE_PIXELFORMAT_24BIT_BGR, JE_FALSE, 0 ) )
		{
			BITMAPINFO			bmi ;
			jeBitmap_Info		info ;

			jeBitmap_GetInfo(Lock,&info,NULL);

			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biHeight = -info.Height;
			bmi.bmiHeader.biWidth = info.Stride ;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 24;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = 0;
			bmi.bmiHeader.biXPelsPerMeter = bmi.bmiHeader.biYPelsPerMeter = 10000;
			bmi.bmiHeader.biClrUsed = bmi.bmiHeader.biClrImportant = 0;

			srcyOffset = yOffset ;
			switch( info.Height )
			{
			case 256 :	srcyOffset = yOffset * 2 ;	break ;
			case 128 :	srcyOffset = yOffset ;		break ;
			case 64  :  srcyOffset = yOffset / 2 ;	break ;
			case 32  :	srcyOffset = yOffset / 4 ;	break ;
			case 16  :  srcyOffset = yOffset / 8 ;	break ;
			}

			SetStretchBltMode (dc.m_hDC,COLORONCOLOR);	//ADDED JH 22.3.2000
			StretchDIBits
			(
				dc.m_hDC,				// handle to device context
				x,					    // x-coord upper-left corner of dest. rectangle
				y,						// y-coord upper-left corner of dest. rectangle
				m_nTile,				// width of destination rectangle
				m_nTile-yOffset,		// height of destination rectangle
				0,						// x-coord upper-left corner of source rectangle
				0,						// y-coord upper-left corner of source rectangle
				info.Stride,			// width of source rectangle
				info.Height-srcyOffset,	// height of source rectangle
				jeBitmap_GetBits( Lock ),// address of bitmap bits
				&bmi,					// address of bitmap data
				DIB_RGB_COLORS,			// usage flags
				SRCCOPY					// raster operation code
			) ;
			jeBitmap_UnLock (Lock);

		}
	//	DeleteBitmap( hBitmap ) ;
#else
		pMatSpec = (jeMaterialSpec*) Materials_GetMaterialSpec( pMaterial ) ;
		if( pMaterial == pCurrentMaterial )
			dc.FillSolidRect( x-1, y-1, m_nTile+2, m_nTile-yOffset+2, RGB(255,255,255) ) ;  

	//	hBitmap = CreateHBitmapFromgeBitmap( pBitmap, MemDC.m_hDC ) ;
		jeMaterialSpec_Thumbnail* pThumb = NULL;

		if( (pThumb = jeMaterialSpec_GetThumbnail( pMatSpec )) != NULL )
		{
			BITMAPINFOHEADER	bmi ;

			bmi.biSize = sizeof(BITMAPINFOHEADER);
			bmi.biHeight = -pThumb->height;
			bmi.biWidth = pThumb->width ;
			bmi.biPlanes = 1;
			bmi.biBitCount = 24;
			bmi.biCompression = BI_RGB;
			bmi.biSizeImage = 0;
			bmi.biXPelsPerMeter = bmi.biYPelsPerMeter = 10000;
			bmi.biClrUsed = bmi.biClrImportant = 0;

			srcyOffset = yOffset ;
			switch( pThumb->height )
			{
			case 256 :	srcyOffset = yOffset * 2 ;	break ;
			case 128 :	srcyOffset = yOffset ;		break ;
			case 64  :  srcyOffset = yOffset / 2 ;	break ;
			case 32  :	srcyOffset = yOffset / 4 ;	break ;
			case 16  :  srcyOffset = yOffset / 8 ;	break ;
			}

			SetStretchBltMode (dc.m_hDC,COLORONCOLOR);	//ADDED JH 22.3.2000
			StretchDIBits
			(
				dc.m_hDC,				// handle to device context
				x,					    // x-coord upper-left corner of dest. rectangle
				y,						// y-coord upper-left corner of dest. rectangle
				m_nTile,				// width of destination rectangle
				m_nTile-yOffset,		// height of destination rectangle
				0,						// x-coord upper-left corner of source rectangle
				0,						// y-coord upper-left corner of source rectangle
				pThumb->width,			// width of source rectangle
				pThumb->height-srcyOffset,	// height of source rectangle
				pThumb->contents,		// address of bitmap bits
				(BITMAPINFO*)&bmi,					// address of bitmap data
				DIB_RGB_COLORS,			// usage flags
				SRCCOPY					// raster operation code
			) ;

		}
	//	DeleteBitmap( hBitmap ) ;
#endif

		x += (m_nTile+1) ;
		if( x >= (rect.right-MARGIN_PAD) )
		{
			x = nLeftMargin ;
			y += m_nTile+1-yOffset ;
			yOffset = 0 ;
		}

		if( y >= rect.bottom )
			break ;

		pMaterial = MaterialList_GetNextMaterial( pMaterials, &MI ) ;
	}

	MemDC.DeleteDC( ) ;
	dc.SelectObject( pOldPen ) ;

	// Do not call CStatic::OnPaint() for painting messages
}// OnPaint

void CMyStatic::SetBitmap(CBitmap *pBitmap)
{
	ASSERT( pBitmap != NULL ) ;

	m_pBitmap = pBitmap ;
}//SetBitmap

void CMyStatic::SetScrollTop(int nTopPixel)
{
	m_nTop = nTopPixel ;
}// SetScrollTop

void CMyStatic::SetTile(const eTHUMBSIZE eTile)
{
	m_nTile = eTile ;
}// SetTile

int CMyStatic::GetVirtualHeight()
{
	int						nItems ;
	CJweApp				*	pApp = (CJweApp*)AfxGetApp() ;
	MaterialList_Struct	*	pMaterials ;

	pMaterials = pApp->GetMaterialList( ) ;

	nItems = MaterialList_GetNumItems( pMaterials ) ;
	if( m_nTile == THUMBNAIL_64 )
		nItems /= 2 ;
	else if( m_nTile == THUMBNAIL_32 )
		nItems /= 4 ;

	return nItems * m_nTile ;
}// GetVirtualHeight

int CMyStatic::GetTileHeight()
{
	return m_nTile ;
}// GetTileHeight

Material_Struct * CMyStatic::GetMaterialAtPoint(const CPoint &rPoint)
{
	MaterialList_Struct	*	pMaterials ;
	Material_Struct		*	pMaterial = NULL ;
	MaterialIterator		MI ;
	CPoint					vPoint ;
	int						nMaterial ;

	vPoint.x = rPoint.x ;
	vPoint.y = m_nTop + rPoint.y ;

	nMaterial = vPoint.y / m_nTile ;
	if( m_nTile == THUMBNAIL_64 )
		nMaterial *= 2 ;
	else if( m_nTile == THUMBNAIL_32 )
		nMaterial *= 4 ;
	nMaterial += vPoint.x / m_nTile ;

	pMaterials = ((CJweApp*)AfxGetApp())->GetMaterialList() ;
	if( nMaterial >= MaterialList_GetNumItems( pMaterials ) )
		return NULL ;	// Past the end

	pMaterial = MaterialList_GetFirstMaterial( pMaterials, &MI ) ;
	while( nMaterial-- )
		pMaterial = MaterialList_GetNextMaterial( pMaterials, &MI ) ;

	return pMaterial ;
}// GetMaterialAtPoint

int CMyStatic::ScrollMaterialInView(const Material_Struct *pMaterial)
{
	MaterialList_Struct	*	pMaterials ;
	MaterialIterator		MI ;
	Material_Struct		*	pMat = NULL ;
	int						nIndex ;
	int						nTopOfImage ;
	int						vy ;
	RECT					rect ;

	pMaterials = ((CJweApp*)AfxGetApp())->GetMaterialList() ;

	// Get Material Index
	nIndex = 0 ;
	pMat = MaterialList_GetFirstMaterial( pMaterials, &MI ) ;
	while( pMat != NULL  )
	{
		if( pMat == pMaterial )
			break ;
		nIndex++ ;
		pMat = MaterialList_GetNextMaterial( pMaterials, &MI ) ;
	}

	vy = nIndex ;
	if( m_nTile == THUMBNAIL_64 )
		vy /= 2 ;
	else if( m_nTile == THUMBNAIL_32 )
		vy /= 4 ;

	GetClientRect( &rect ) ;
	nTopOfImage = vy * m_nTile ;
	if( nTopOfImage > (m_nTop - m_nTile) &&		// Need to "scroll" ?
		nTopOfImage < (m_nTop + rect.bottom ) )
	{
		return nTopOfImage ;
	}

	if( nTopOfImage < GetVirtualHeight() - rect.bottom )
	{
		m_nTop = nTopOfImage ;
		return m_nTop ;
	}
	
	m_nTop = GetVirtualHeight() - rect.bottom ;
	return m_nTop ;
}// SelectMaterial

// JUNK I MIGHT NEED
#if 0
	SetDIBitsToDevice
	(  
		dc.m_hDC,              // handle to device context
		x,						// x-coordinate of upper-left corner of dest. rect.
		y,						// y-coordinate of upper-left corner of dest. rect.
		m_nTile,				// source rectangle width
		m_nTile-yOffset,		// source rectangle height
		0,						// x-coordinate of lower-left corner of source rect.
		0,						// y-coordinate of lower-left corner of source rect.
		0,						// first scan line in array
		m_nTile,				// number of scan lines
		jeBitmap_GetBits( Lock ),// address of array with DIB bits
		&bmi,					// address of structure with bitmap info.
		DIB_RGB_COLORS			// RGB or palette indexes
	);
#endif


/*	
Material_Struct * CMyStatic::SetActiveMaterial(const CPoint &rPoint)
{
	MaterialList_Struct	*	pMaterials ;
	Material_Struct		*	pMaterial = NULL ;
	MaterialIterator		MI ;
	CPoint					vPoint ;
	int						nMaterial ;

	vPoint.x = rPoint.x ;
	vPoint.y = m_nTop + rPoint.y ;

	nMaterial = vPoint.y / m_nTile ;
	if( m_nTile == THUMBNAIL_64 )
		nMaterial *= 2 ;
	else if( m_nTile == THUMBNAIL_32 )
		nMaterial *= 4 ;
	nMaterial += vPoint.x / m_nTile ;

	pMaterials = ((CJweApp*)AfxGetApp())->GetMaterialList() ;
	if( nMaterial >= MaterialList_GetNumItems( pMaterials ) )
		return NULL ;	// Past the end

	pMaterial = MaterialList_GetFirstMaterial( pMaterials, &MI ) ;
	while( nMaterial-- )
		pMaterial = MaterialList_GetNextMaterial( pMaterials, &MI ) ;

	return pMaterial ;
}// GetMaterialAtPoint
*/