/****************************************************************************************/
/*  TEXTURES.CPP                                                                        */
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

#include <assert.h>
#include "stdafx.h"
#include "jwe.h"
#include "MainFrm.h"
#include "MfcUtil.h"
#include "Ram.h"

#include "Textures.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IMAGES_WIDTH	(128+5)		// 1+32+1+32+1+32+1+32+1 5 for dividers
#define INFO_HEIGHT		(26)
#define IMAGE_MAX_HEIGHT	(128)

/////////////////////////////////////////////////////////////////////////////
// CTextures dialog

typedef struct tagNameInfo
{
	CComboBox				*	pCB ;
	const Material_Struct	*	pCurrent ;
} NameInfo ;


CTextures::CTextures(CWnd* pParent /*=NULL*/)
	: CDialog(CTextures::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTextures)
	m_Info = _T("");
	m_TexSize = 0;
	//}}AFX_DATA_INIT
}


void CTextures::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextures)
	DDX_Control(pDX, TXTR_CB_NAME, m_Names);
	DDX_Control(pDX, TXTR_SB, m_Scroll);
	DDX_Control(pDX, TXTR_PC_IMAGES, m_Bitmap);
	DDX_Text(pDX, TXTR_ST_INFO, m_Info);
	DDV_MaxChars(pDX, m_Info, 63);
	DDX_Radio(pDX, TXTR_BN_128, m_TexSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextures, CDialog)
	//{{AFX_MSG_MAP(CTextures)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(TXTR_BN_128, OnBn128)
	ON_BN_CLICKED(TXTR_BN_32, OnBn32)
	ON_BN_CLICKED(TXTR_BN_64, OnBn64)
	ON_WM_LBUTTONUP()
	ON_CBN_SELCHANGE(TXTR_CB_NAME, OnSelchangeCbName)
    ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextures message handlers

BOOL CTextures::OnInitDialog() 
{
//	CWnd				*	pWnd ;
//	RECT					rect ;
//	int						nTextureHeight ;
	CJweApp				*	pApp = (CJweApp*)AfxGetApp() ;
	HICON					hIcon ;
	
	m_pMaterials = pApp->GetMaterialList( ) ;
	m_Info = Materials_GetName( MaterialList_GetCurMaterial( m_pMaterials ) ) ;

	hIcon = pApp->LoadIcon( IDI_TILE128 );
	SendDlgItemMessage( TXTR_BN_128, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon) ;
	hIcon = pApp->LoadIcon( IDI_TILE64 ) ;
	SendDlgItemMessage( TXTR_BN_64, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon) ;
	hIcon = pApp->LoadIcon( IDI_TILE32 ) ;
	SendDlgItemMessage( TXTR_BN_32, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon) ;

	CDialog::OnInitDialog();
	
	// We need 128x(128xn) for the images, and those are hard to set in the dialog
	// editor, so we move the controls here.
//	pWnd = GetDlgItem( TXTR_PC_IMAGES ) ;
//	pWnd->GetClientRect( &rect ) ;
//	nTextureHeight = rect.bottom ; 
//	pWnd->SetWindowPos
//	( 
//		NULL, 
//		0, 0, 
//		IMAGES_WIDTH, nTextureHeight, 
//		SWP_NOZORDER|SWP_NOMOVE
//	) ;
//	pWnd->GetWindowRect( &rect ) ;	// Screen coords of bitmap window
//	this->ScreenToClient( &rect ) ;	// Client coords of bitmap window

//	pWnd = GetDlgItem( TXTR_SB ) ;	// Scroll Bar
//	pWnd->SetWindowPos
//	( 
//		NULL, 
//		rect.left+IMAGES_WIDTH, rect.top, 
//		GetSystemMetrics( SM_CXVSCROLL ), nTextureHeight, 
//		SWP_NOZORDER
//	) ;
//	
//	pWnd = GetDlgItem( TXTR_ST_INFO ) ;	// Info static
//	pWnd->SetWindowPos( NULL, rect.left, rect.bottom+1, IMAGES_WIDTH, INFO_HEIGHT, SWP_NOZORDER ) ;

//	PositionDialogUnderTabs( this ) ;
	m_Bitmap.SetScrollTop( 0 ) ;
	m_Bitmap.SetTile( CMyStatic::THUMBNAIL_128 ) ;

//	m_Scroll.SetScrollRange( 0, m_Bitmap.GetVirtualHeight() - nTextureHeight, true ) ;
	m_Scroll.SetScrollPos( 0, true ) ;

	UpdateNames() ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}// OnInitDialog

void CTextures::OnSize(UINT nType, int cx, int cy) 
{
    CDialog::OnSize(nType, cx, cy);
    CWnd * pWnd = GetDlgItem(TXTR_PC_IMAGES);
    int offset = (cx-(IMAGES_WIDTH+GetSystemMetrics( SM_CXVSCROLL )+8))/2;
    if(offset < 0)
        offset = 0;
    if(pWnd) {
        pWnd->MoveWindow(4+offset,48,IMAGES_WIDTH,cy-64);
    }
	pWnd = GetDlgItem( TXTR_SB );
    if(pWnd) {
        pWnd->MoveWindow(4+IMAGES_WIDTH+offset,48,GetSystemMetrics( SM_CXVSCROLL ),cy-64);
	    m_Scroll.SetScrollRange( 0, m_Bitmap.GetVirtualHeight() - cy+64, true ) ;
    }
	pWnd = GetDlgItem( TXTR_ST_INFO );
    if(pWnd) {
        pWnd->MoveWindow(4,cy-14,cx-100,14);
    }
	pWnd = GetDlgItem( TXTR_BN_32 );
    if(pWnd) {
        pWnd->MoveWindow(cx-36,cy-14,32,14);
    }
	pWnd = GetDlgItem( TXTR_BN_64 );
    if(pWnd) {
        pWnd->MoveWindow(cx-68,cy-14,32,14);
    }
	pWnd = GetDlgItem( TXTR_BN_128 );
    if(pWnd) {
        pWnd->MoveWindow(cx-100,cy-14,32,14);
    }
	pWnd = GetDlgItem( TXTR_CB_CATAGORY );
    if(pWnd) {
        pWnd->MoveWindow(40,2,cx-44,14);
    }
	pWnd = GetDlgItem( TXTR_CB_NAME );
    if(pWnd) {
        pWnd->MoveWindow(40,24,cx-44,14);
    }
}

void CTextures::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{

	if( nSBCode != SB_ENDSCROLL )
	{

		// locals
		RECT	Rect;
		int		NewScrollPos;
		int		ScrollHeight;
		int		BitmapHeight;
		int		BitmapPerPage;
		int		ScrollRangeMin, ScrollRangeMax;
		int		StartingScrollPos;

		// setup some size data
		StartingScrollPos = NewScrollPos = pScrollBar->GetScrollPos();
		BitmapHeight = m_Bitmap.GetTileHeight();
		assert( BitmapHeight > 0 );
		pScrollBar->GetClientRect( &Rect );
		ScrollHeight = Rect.bottom - Rect.top;
		assert( ScrollHeight > 0 );
		BitmapPerPage = ScrollHeight / BitmapHeight;
		//assert( BitmapPerPage > 0 );
		// Changed by Incarnadine.  It's possible that this will work out to 0.
		if(BitmapPerPage < 1) BitmapPerPage = 1;

		// get scroll range limits
		assert( pScrollBar != NULL );
		pScrollBar->GetScrollRange( &ScrollRangeMin, &ScrollRangeMax );

		// process scroll bar message
		switch( nSBCode )
		{

			// scroll texture list one texture at a time
			case SB_LINEDOWN:
			{
				NewScrollPos = ( ( NewScrollPos / BitmapHeight ) + 1 ) * BitmapHeight;
				break;
			}
			case SB_LINEUP:
			{
				NewScrollPos = ( ( NewScrollPos / BitmapHeight ) - 1 ) * BitmapHeight;
				break;
			}

			// scroll texture list on page of textures at a time
			case SB_PAGEDOWN:
			{
				NewScrollPos = ( ( NewScrollPos / BitmapHeight ) + BitmapPerPage ) * BitmapHeight;
				break;
			}
			case SB_PAGEUP:
			{
				NewScrollPos = ( ( NewScrollPos / BitmapHeight ) - BitmapPerPage ) * BitmapHeight;
				break;
			}

			// scroll texture list based on slider bar
			case SB_THUMBTRACK:
			{
				if( nPos == (UINT)ScrollRangeMax )
					NewScrollPos = nPos;
				else
					NewScrollPos = ( nPos / BitmapHeight ) * BitmapHeight;
				break;
			}
		}

		// cap the scroll bar position if it exceeds limits
		if ( NewScrollPos < ScrollRangeMin )
		{
			NewScrollPos = ScrollRangeMin;
		}
		else if ( NewScrollPos > ScrollRangeMax )
		{
			NewScrollPos = ScrollRangeMax;
		}

		// if there has been a change in the scroll bar position then make some adjustments
		if ( StartingScrollPos != NewScrollPos )
		{

			// set new scroll bar position
			pScrollBar->SetScrollPos( NewScrollPos, true ) ;

			// let the textures know about the new scroll bar position
			m_Bitmap.SetScrollTop( NewScrollPos ) ;
			m_Bitmap.Invalidate() ;
		}
	}

}// OnVScroll

void CTextures::OnBn128() 
{

	CWnd				*	pWnd ;
	RECT					rect ;
	int						nTextureHeight ;

	pWnd = GetDlgItem( TXTR_PC_IMAGES ) ;
	pWnd->GetClientRect( &rect ) ;
	nTextureHeight = rect.bottom ; 

	m_Bitmap.SetTile( CMyStatic::THUMBNAIL_128 ) ;
	m_Scroll.SetScrollPos( 0, true ) ;
	m_Bitmap.SetScrollTop( 0 ) ;
	m_Scroll.SetScrollRange( 0, m_Bitmap.GetVirtualHeight() - nTextureHeight, true ) ;
	m_Bitmap.Invalidate() ;	
	UpdateData( true );
	m_TexSize = 0;
	UpdateData(false );
}//OnBn128

void CTextures::OnBn64() 
{
	CWnd				*	pWnd ;
	RECT					rect ;
	int						nTextureHeight ;

	pWnd = GetDlgItem( TXTR_PC_IMAGES ) ;
	pWnd->GetClientRect( &rect ) ;
	nTextureHeight = rect.bottom ; 

	m_Bitmap.SetTile( CMyStatic::THUMBNAIL_64 ) ;
	m_Scroll.SetScrollPos( 0, true ) ;
	m_Bitmap.SetScrollTop( 0 ) ;
	m_Scroll.SetScrollRange( 0, m_Bitmap.GetVirtualHeight() - nTextureHeight, true ) ;
	m_Bitmap.Invalidate() ;	
	UpdateData( true );
	m_TexSize = 1;
	UpdateData(false );
}// OnBn64

void CTextures::OnBn32() 
{
	CWnd				*	pWnd ;
	RECT					rect ;
	int						nTextureHeight ;

	pWnd = GetDlgItem( TXTR_PC_IMAGES ) ;
	pWnd->GetClientRect( &rect ) ;
	nTextureHeight = rect.bottom ; 

	m_Bitmap.SetTile( CMyStatic::THUMBNAIL_32 ) ;
	m_Scroll.SetScrollPos( 0, true ) ;
	m_Bitmap.SetScrollTop( 0 ) ;
	m_Scroll.SetScrollRange( 0, m_Bitmap.GetVirtualHeight() - nTextureHeight, true ) ;
	m_Bitmap.Invalidate() ;	
	UpdateData( true );
	m_TexSize = 2;
	UpdateData(false );
}// OnBn32

void CTextures::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Is this click on the Bitmap control ?
	RECT	rect ;
	int nIndex;

	m_Bitmap.GetWindowRect( &rect ) ;
	ScreenToClient( &rect ) ;
	if( PtInRect( &rect, point ) )
	{
		Material_Struct	* pMaterial ;
		CJweDoc*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument() ;
		
		point.x -= rect.left ;
		point.y -= rect.top ;
		pMaterial = m_Bitmap.GetMaterialAtPoint( point ) ;
		if( pMaterial != NULL )
		{
			
		//	pMaterial = m_Bitmap.SetActiveMaterial ( point ) ;

			MaterialList_SetCurMaterial( m_pMaterials, pMaterial ) ;
			pDoc->ApplyMaterial() ;
			m_Bitmap.Invalidate() ;
			m_Info = Materials_GetName( pMaterial ) ;
			UpdateData( false ) ;
			nIndex =m_Names.FindString( -1, Materials_GetName(pMaterial) );
			if( nIndex != -1 )
				m_Names.SetCurSel( nIndex );
		}
	}
	
	CDialog::OnLButtonUp(nFlags, point);
}//OnLButtonUp



void CTextures::UpdateNames()
{
	NameInfo	nameInfo ;
	
	m_Names.ResetContent() ;
	nameInfo.pCB = &m_Names ;
	nameInfo.pCurrent = MaterialList_GetCurMaterial( m_pMaterials ) ;
	MaterialList_EnumMaterials( m_pMaterials, &nameInfo, CTextures::UpdateNamesCB ) ;

	//TODO Implement the below

	//m_Names.ResetContent() ;
	//nameInfo.pCB = &m_Names ;
	//nameInfo.pCurrent = MaterialList_GetCurMaterial( m_pShaders ) ;
	//MaterialList_EnumMaterials( m_pShaders, &nameInfo, CTextures::UpdateNamesTR ) ;

}// UpdateNames

jeBoolean CTextures::UpdateNamesCB(Material_Struct *pMaterial, void *lParam)
{
	NameInfo	*	pni = (NameInfo*)lParam ;
	int				nIndex ;

	nIndex = pni->pCB->AddString( Materials_GetName( pMaterial ) ) ;
	if( nIndex == CB_ERR || nIndex == CB_ERRSPACE )
		return JE_FALSE ;

	pni->pCB->SetItemData( nIndex, (DWORD)pMaterial ) ;
	if( pMaterial == pni->pCurrent )
		pni->pCB->SetCurSel( nIndex ) ;

	return JE_TRUE ;
}//UpdateNamesCB

//CYRIUS
jeBoolean CTextures::UpdateNamesTR(Material_Struct *pShader, void *lParam)
{
	NameInfo	*	pni = (NameInfo*)lParam ;
	int				nIndex ;

	nIndex = pni->pCB->AddString( Materials_GetName( pShader ) ) ;
	if( nIndex == CB_ERR || nIndex == CB_ERRSPACE )
		return JE_FALSE ;

	pni->pCB->SetItemData( nIndex, (DWORD)pShader ) ;
	if( pShader == pni->pCurrent )
		pni->pCB->SetCurSel( nIndex ) ;

	return JE_TRUE ;
}//UpdateNamesTR

//END

void CTextures::OnSelchangeCbName() 
{
	Material_Struct	*	pMaterial ;
	int					nIndex ;
	int					nScrollTop ;
	
	nIndex = m_Names.GetCurSel() ;
	pMaterial = (Material_Struct*)m_Names.GetItemData( nIndex ) ;
	if( pMaterial != MaterialList_GetCurMaterial( m_pMaterials ) )
	{
		CJweDoc*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument() ;

		nScrollTop = m_Bitmap.ScrollMaterialInView( pMaterial ) ;
		m_Scroll.SetScrollPos( nScrollTop, true ) ;
		MaterialList_SetCurMaterial( m_pMaterials, pMaterial ) ;
		if( pDoc != NULL ) 
		{
			pDoc->ApplyMaterial( ) ;
		}
		m_Bitmap.Invalidate() ;
	}
}// OnSelchangeCbName

BOOL CTextures::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if( wParam == 1 && lParam == 0 )
		return(JE_TRUE );	
	return CDialog::OnCommand(wParam, lParam);
}
