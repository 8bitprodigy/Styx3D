/****************************************************************************************/
/*  ENTITYCP.CPP                                                                        */
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
#include "Doc.h"
#include "EntityTable.h"
#include "jwe.h"
#include "MainFrm.h"
#include "MfcUtil.h"
#include "Ram.h"
#include "Util.h"

#include "EntityCP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum
{
	IMAGE_SELECTED,
	IMAGE_UNSELECTED,
	IMAGE_ACTOR,
	IMAGE_BRUSH,
	IMAGE_ENTITY,
	IMAGE_LAST
} ;


/////////////////////////////////////////////////////////////////////////////
// CEntityCP dialog


CEntityCP::CEntityCP(CWnd* pParent /*=NULL*/)
	: CDialog(CEntityCP::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEntityCP)
	m_csDefString = _T("");
	m_csDef1 = _T("");
	m_csDef2 = _T("");
	m_csDef3 = _T("");
	m_csName = _T("");
	m_csNumber = _T("");
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	m_Sheet = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	//}}AFX_DATA_INIT
	m_pEntities = NULL ;
}


void CEntityCP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEntityCP)
	DDX_Control(pDX, ENTY_BN_COLOR1, m_bnColor);
	DDX_Control(pDX, ENTY_BN_ORTHOCOLOR, m_bnOrthoColor);
	DDX_Control(pDX, ENTY_CB_DEFTF, m_DefEnumCB);
	DDX_Control(pDX, ENTY_LB_PROPERTIES, m_Properties);
	DDX_Text(pDX, ENTY_ED_DEFSTRING, m_csDefString);
	DDV_MaxChars(pDX, m_csDefString, 127);
	DDX_Text(pDX, ENTY_ED_DEFV1, m_csDef1);
	DDV_MaxChars(pDX, m_csDef1, 31);
	DDX_Text(pDX, ENTY_ED_DEFV2, m_csDef2);
	DDV_MaxChars(pDX, m_csDef2, 31);
	DDX_Text(pDX, ENTY_ED_DEFV3, m_csDef3);
	DDV_MaxChars(pDX, m_csDef3, 31);
	DDX_Text(pDX, ENTY_ED_NAME, m_csName);
	DDV_MaxChars(pDX, m_csName, 31);
	DDX_Text(pDX, ENTY_ED_NUMBER, m_csNumber);
	DDV_MaxChars(pDX, m_csNumber, 10);
	DDX_Check(pDX, IDM_TOOLS_PLACEARCH, m_Arch);
	DDX_Check(pDX, IDM_TOOLS_PLACECONE, m_Cone);
	DDX_Check(pDX, IDM_TOOLS_PLACECUBE, m_Cube);
	DDX_Check(pDX, IDM_TOOLS_PLACECYLINDER, m_Cylinder);
	DDX_Check(pDX, IDM_TOOLS_PLACEENTITY, m_Entity);
	DDX_Check(pDX, IDM_TOOLS_PLACELIGHT, m_Light);
	DDX_Check(pDX, IDM_TOOLS_PLACESHEET, m_Sheet);
	DDX_Check(pDX, IDM_TOOLS_PLACESPHEROID, m_Sphere);
	DDX_Check(pDX, IDM_TOOLS_PLACESTAIRCASE, m_Stair);
	DDX_Check(pDX, IDM_TOOLS_PLACETERRAIN, m_Terrain);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEntityCP, CDialog)
	//{{AFX_MSG_MAP(CEntityCP)
	ON_WM_DRAWITEM()
	ON_WM_DELETEITEM()
	ON_LBN_SELCHANGE(ENTY_LB_PROPERTIES, OnSelchangeLbProperties)
	ON_BN_CLICKED(ENTY_BN_APPLY, OnBnApply)
	ON_EN_KILLFOCUS(ENTY_ED_NAME, OnKillfocusEdName)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_BN_CLICKED(IDM_TOOLS_PLACESTAIRCASE, OnToolsPlacestaircase)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_BN_CLICKED(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_BN_CLICKED(IDM_TOOLS_PLACEENTITY, OnToolsPlaceentity)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_BN_CLICKED(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_BN_CLICKED(IDM_TOOLS_PLACECONE, OnToolsPlacecone)
	ON_BN_CLICKED(IDM_TOOLS_PLACEARCH, OnToolsPlacearch)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_UPDATECOLOR, OnChangeColor)
END_MESSAGE_MAP()

typedef struct tagPropertyListInfo
{
	CListBox	*	pLB ;
	jeSymbol	*	pSelect ;
	CJweDoc		*	pDoc  ;
} PropertyListInfo ;


/////////////////////////////////////////////////////////////////////////////
// CEntityCP message handlers

BOOL CEntityCP::OnInitDialog()
{
	CBitmap		bitmap ;
	CPalette	Palette ;

	CDialog::OnInitDialog();

	SetupTemplateDialogIcons( this) ;
	PositionDialogUnderTabs( this ) ;

	m_Properties.ResetContent() ;

	ShowFieldsBySymbolType( JE_SYMBOL_TYPE_INT ) ;
	// Restrict edit control to alpha--from MSDN::CTRLTEST
	m_edit1.SubclassEdit( ENTY_ED_NAME, this, PES_LETTERS|PES_SPACE ) ;

	if( !EnableToolTips(true) )
	{
		return false;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}// OnInitDialog

void CEntityCP::FillProperties()
{
	m_Properties.ResetContent() ;
	if( m_pEntities )
	{

	}
}// FillProperties

void CEntityCP::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	int				nNameWidth ;
	int				nValueWidth ;
	RECT			rName ;
	RECT			rValue ;
	COLORREF		OldColor ;
	COLORREF		OldBk ;
	char			szName[ENTITY_MAXNAMELENGTH] ;
	char			szValue[ENTITY_MAXSTRINGLENGTH] ;
	HBRUSH			hOldBrush ;
	jeSymbol	*	pField ;
	FieldInfo	*	pfi ;


	if( nIDCtl != ENTY_LB_PROPERTIES )
	{
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
		return ;
	}
	
	if( lpDrawItemStruct->itemID == -1 )
		return ;

	pfi = (FieldInfo*)lpDrawItemStruct->itemData ;
	pField = pfi->pField ;	

//	nNameWidth = (lpDrawItemStruct->rcItem.right/2)-1 ;
	nNameWidth = 50 ;
	nValueWidth = lpDrawItemStruct->rcItem.right-nNameWidth-3 ;
	lpDrawItemStruct->itemData ;	
	rName = lpDrawItemStruct->rcItem ;
	rName.left++ ;
	rName.right = nNameWidth ;
	rValue = lpDrawItemStruct->rcItem ;
	rValue.left = rName.right + 2 ;
	rValue.right-- ;

	if( lpDrawItemStruct->itemState & ODS_SELECTED )
	{
		OldColor = ::SetTextColor( lpDrawItemStruct->hDC, GetSysColor( COLOR_HIGHLIGHTTEXT ) ) ;
		OldBk = ::SetBkColor( lpDrawItemStruct->hDC, GetSysColor( COLOR_HIGHLIGHT ) ) ;
	}
	else
	{
		OldColor = ::SetTextColor( lpDrawItemStruct->hDC, GetSysColor( COLOR_WINDOWTEXT ) ) ;
		OldBk = ::SetBkColor( lpDrawItemStruct->hDC, GetSysColor( COLOR_WINDOW ) ) ;
	}
	
	strcpy( szName, jeSymbol_GetName( pField ) ) ;
	TrimStringByPixelCount( lpDrawItemStruct->hDC, szName, nNameWidth ) ;

	if( pfi->bInit == JE_TRUE )
	{
		switch( pfi->Type )
		{
			case JE_SYMBOL_TYPE_VEC3D :
				sprintf( szValue, "%4.2f %4.2f %4.2f", pfi->Value.Vec3d.X, pfi->Value.Vec3d.Y, pfi->Value.Vec3d.Z ) ;
				break ;
			case JE_SYMBOL_TYPE_INT :
				sprintf( szValue, "%d", pfi->Value.Integer ) ;
				break ;
			case JE_SYMBOL_TYPE_FLOAT :
				sprintf( szValue, "%4.2f", pfi->Value.Float ) ;
				break ;
			case JE_SYMBOL_TYPE_COLOR :
				sprintf( szValue, "R:%d G:%d B:%d", (int)pfi->Value.Color.r, (int)pfi->Value.Color.g, (int)pfi->Value.Color.b ) ;
				break ;
			case JE_SYMBOL_TYPE_STRING :
				strcpy( szValue, pfi->Value.pChar ) ;
				break ;
		}
	}
	else
	{
		szValue[0] = 0 ;
	}
	TrimStringByPixelCount( lpDrawItemStruct->hDC, szValue, nValueWidth ) ;

	::ExtTextOut
	(
		lpDrawItemStruct->hDC,
		rName.left,
		rName.top,
		ETO_CLIPPED|ETO_OPAQUE,
		&rName,
		szName,
		strlen( szName ),
		NULL
	) ;
	::ExtTextOut
	(
		lpDrawItemStruct->hDC,
		rValue.left,
		rValue.top,
		ETO_CLIPPED|ETO_OPAQUE,
		&rValue,
		szValue,
		strlen( szValue ),
		NULL
	) ;

	::SetTextColor( lpDrawItemStruct->hDC, OldColor ) ;
	::SetBkColor( lpDrawItemStruct->hDC, OldBk ) ;

	hOldBrush = (HBRUSH)::SelectObject( lpDrawItemStruct->hDC, ::GetStockObject( NULL_BRUSH ) ) ;
	::Rectangle( lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, lpDrawItemStruct->rcItem.right, lpDrawItemStruct->rcItem.bottom ) ;
	::Rectangle( lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left+nNameWidth, lpDrawItemStruct->rcItem.top, lpDrawItemStruct->rcItem.right, lpDrawItemStruct->rcItem.bottom ) ;
	::SelectObject( lpDrawItemStruct->hDC, hOldBrush ) ;

}// OnDrawItem


BOOL CEntityCP::NeedTextNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	strcpy( pTTT->szText, "test me" ) ;

//	::PostMessage( m_hWnd, WM_USER+100, 0, 0 ) ;

	return false ;
	id;pResult;
}// NeedTextNotify

int CEntityCP::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
{
	CPoint pt;
	UINT Index;
	BOOL bOutside;

	// Make point relative to properties list box
	pt = point;
	ClientToScreen( &pt ) ;
	m_Properties.ScreenToClient( &pt ) ;

	// If the passed point is within the boundaries of the properties list box,
	// and is over one of the properties, instruct the caller to display
	// the tooltip obtained from EntityTable_GetToolTip.
	Index = m_Properties.ItemFromPoint( pt, bOutside ) ;
	if( bOutside )
	{
		return -1 ;
	}

	pTI->lpszText = LPSTR_TEXTCALLBACK;
	pTI->hwnd = m_hWnd;
	pTI->uId = (UINT)NULL ; // m_Properties.m_hWnd;
	pTI->uFlags |= (TTF_IDISHWND | TTF_NOTBUTTON);
	// Ok, this is a seriously ugly hack.
	// But I can't seem to get it to work any other way.
	// I need to pass the item index to the NeedTextNotify callback, but there
	// is no field for it (apparently the above uId and uFlags must be set
	// exactly like they are, or things don't work).
	// Anyway, what I do here is set an object-global index and then retrieve it 
	// in the NeedTextNotify function.
	m_UglyGlobalItemId = Index;

	return true ;
}
#if 0
LRESULT CEntityCP::MyPostMessage (WPARAM wParam, LPARAM lParam)
{
	wParam;
	lParam;
	EnableToolTips( false ) ;
	EnableToolTips( true ) ;
	return false ;
}
#endif

void CEntityCP::SetCurrentDocument(jeSymbol_Table *pEntities)
{
	if( m_pEntities != pEntities )
	{
		m_pEntities = pEntities ;
		FillProperties() ;
	}
}// SetCurrentDocument

void CEntityCP::Update(CJweDoc *pDoc)
{
	const char		*	pszType ;
	const char		*	pszName ;
	int32				nNumber ;
	jeSymbol_Table	*	pEntities ;
	PropertyListInfo	pli ;

	ASSERT( pDoc->IsKindOf( RUNTIME_CLASS(CJweDoc)) ) ;
	
	// If all entity types are the same...
	if( pDoc->GetSelType() & LEVEL_SELENTITYTYPE )
	{
		m_Properties.ResetContent() ;
		pszType = pDoc->GetFirstEntityType( ) ;
		pEntities = pDoc->GetEntityTable( ) ;

		pli.pLB = &m_Properties ;
		pli.pSelect = NULL ;
		pli.pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument() ;
		EntityTable_EnumFields( pDoc->GetEntityTable( ), pszType, &pli, EnumFieldsCB ) ;
		m_Properties.SetCurSel( 0 ) ;
		OnSelchangeLbProperties( ) ;
		
		pszName = pDoc->GetSelectionName( &nNumber ) ;
		UpdateData( true ) ;
		if( pszName == NULL )
		{
			m_csName = _T("") ;
			m_csNumber = _T("") ;
		}
		else
		{
			m_csName = pszName ;
			if( SELECT_INVALID_NNUMBER == nNumber )
				m_csNumber.LoadString( IDS_MULTSELNAME ) ;
			else
				m_csNumber.Format( "%d", nNumber ) ;
		}
		UpdateData( false ) ;
	}
	else
	{
		m_Properties.ResetContent() ;
		UpdateData( true ) ;
		m_csName = _T("") ;
		m_csNumber = _T("") ;
		UpdateData( false ) ;
	}
}//Update

jeBoolean CEntityCP::EnumFieldsCB(jeSymbol *pSymbol, void *lParam)
{
	int					nIndex ;
	PropertyListInfo *	ppli = (PropertyListInfo*)lParam ;	
	FieldInfo		 *	pfi ;
	char			 *	pChar ;

	pfi = JE_RAM_ALLOCATE_STRUCT( FieldInfo ) ;	
	if( pfi == NULL )
		return JE_FALSE ;
	pfi->pField = pSymbol ;
	pfi->Type = jeSymbol_GetType( pSymbol ) ;
	switch( pfi->Type )
	{
		case JE_SYMBOL_TYPE_VEC3D :	
			pfi->bInit = ppli->pDoc->GetEntityField( pSymbol, &pfi->Value.Vec3d, sizeof pfi->Value.Vec3d ) ;	
			break ;
		case JE_SYMBOL_TYPE_INT :	
			pfi->bInit = ppli->pDoc->GetEntityField( pSymbol, &pfi->Value.Integer, sizeof pfi->Value.Integer ) ;	
			break ;
		case JE_SYMBOL_TYPE_FLOAT :	
			pfi->bInit = ppli->pDoc->GetEntityField( pSymbol, &pfi->Value.Float, sizeof pfi->Value.Float ) ;	
			break ;
		case JE_SYMBOL_TYPE_COLOR :	
			pfi->bInit = ppli->pDoc->GetEntityField( pSymbol, &pfi->Value.Color, sizeof pfi->Value.Color ) ;		
			break ;
		case JE_SYMBOL_TYPE_STRING :
			pfi->bInit = ppli->pDoc->GetEntityField( pSymbol, &pChar, sizeof pChar ) ;
			pfi->Value.pChar = Util_StrDup( pChar ) ;
			if( pfi->Value.pChar == NULL ) 
			{
				jeRam_Free( pfi ) ;
				return JE_FALSE ;
			}
			break ;
	}
	nIndex = ppli->pLB->AddString( jeSymbol_GetName( pSymbol ) ) ;
	if( nIndex != CB_ERR && nIndex != CB_ERRSPACE )
	{
		ppli->pLB->SetItemData( nIndex, (DWORD)pfi ) ;
	}
	return JE_TRUE ;
}// EnumFieldsCB

void CEntityCP::OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct) 
{
	FieldInfo		 *	pfi ;

	if( nIDCtl == ENTY_LB_PROPERTIES )
	{
		pfi = (FieldInfo*)lpDeleteItemStruct->itemData ;
		ASSERT( pfi != NULL ) ;
		switch( pfi->Type )
		{
			case JE_SYMBOL_TYPE_STRING :
				ASSERT( pfi->Value.pChar != NULL );
				jeRam_Free( pfi->Value.pChar ) ;
				break ;
		}
		jeRam_Free( pfi ) ;
		lpDeleteItemStruct->itemData = NULL ;
	}
	CDialog::OnDeleteItem(nIDCtl, lpDeleteItemStruct);
}// OnDeleteItem

void CEntityCP::OnSelchangeLbProperties() 
{
	int				iIndex ;
	FieldInfo	*	pfi ;

	iIndex = m_Properties.GetCurSel( ) ;
	if( iIndex != -1 )
	{
		pfi = (FieldInfo*)m_Properties.GetItemData( iIndex ) ;
		ShowFieldsBySymbolType( pfi->Type ) ;
		SetFields( pfi ) ;		
	}
}// OnSelchangeLbProperties

void CEntityCP::ShowFieldsBySymbolType(jeSymbol_Type Type )
{
	CString	cstr ;
	int		nIndex ;

	UpdateData( true ) ;

	GetDlgItem( ENTY_ED_DEFSTRING )->ShowWindow( false ) ;
	GetDlgItem( ENTY_ED_DEFV1 )->ShowWindow( false ) ;
	GetDlgItem( ENTY_ED_DEFV2 )->ShowWindow( false ) ;
	GetDlgItem( ENTY_ED_DEFV3 )->ShowWindow( false ) ;
	GetDlgItem( ENTY_CB_DEFTF )->ShowWindow( false ) ;	// BOOL, ENUM
	GetDlgItem( ENTY_BN_COLOR1 )->ShowWindow( false ) ;

	m_csDefString = _T("");
	m_csDef1 = _T("");
	m_csDef2 = _T("");
	m_csDef3 = _T("");

	switch( Type )
	{
	case JE_SYMBOL_TYPE_VEC3D :
		GetDlgItem( ENTY_ED_DEFV3 )->ShowWindow( true ) ;
		GetDlgItem( ENTY_ED_DEFV2 )->ShowWindow( true ) ;
		// Fall Thru

	case JE_SYMBOL_TYPE_INT :
	case JE_SYMBOL_TYPE_FLOAT :
		GetDlgItem( ENTY_ED_DEFV1 )->ShowWindow( true ) ;
		GetDlgItem( ENTY_ED_DEFV1 )->SetFocus() ;
		break ;

	case JE_SYMBOL_TYPE_ENUM :
	case JE_SYMBOL_TYPE_STRING :
		GetDlgItem( ENTY_ED_DEFSTRING )->ShowWindow( true ) ;
		GetDlgItem( ENTY_ED_DEFSTRING )->SetFocus() ;
		break ;

	case JE_SYMBOL_TYPE_COLOR :
		GetDlgItem( ENTY_BN_COLOR1 )->ShowWindow( true ) ;
		GetDlgItem( ENTY_BN_COLOR1 )->SetFocus() ;
		break ;	
	
	case JE_SYMBOL_TYPE_BOOLEAN :
		m_DefEnumCB.ResetContent() ;
		cstr.LoadString( IDS_TRUE ) ;
		nIndex = m_DefEnumCB.AddString( cstr ) ;
		m_DefEnumCB.SetItemData( nIndex, JE_TRUE ) ;
		cstr.LoadString( IDS_FALSE ) ;
		nIndex = m_DefEnumCB.AddString( cstr ) ;
		m_DefEnumCB.SetItemData( nIndex, JE_FALSE ) ;
		m_DefEnumCB.SetCurSel( 0 ) ;
		GetDlgItem( ENTY_CB_DEFTF )->ShowWindow( true ) ;
		GetDlgItem( ENTY_CB_DEFTF )->SetFocus() ;
		break ;
	}

	UpdateData( false ) ;

}// ShowFieldsBySymbolType

void CEntityCP::SetFields( const FieldInfo * pfi )
{
	UpdateData( true ) ;

	if( pfi->bInit == JE_TRUE )
	{
		switch( pfi->Type )
		{
			case JE_SYMBOL_TYPE_VEC3D :
				m_csDef3.Format( "%5.2f", pfi->Value.Vec3d.Z ) ;
				m_csDef2.Format( "%5.2f", pfi->Value.Vec3d.Y ) ;
				m_csDef1.Format( "%5.2f", pfi->Value.Vec3d.X ) ;
				TrimString( m_csDef3 ) ;
				TrimString( m_csDef2 ) ;
				TrimString( m_csDef1 ) ;
				break ;
			case JE_SYMBOL_TYPE_INT :
				m_csDef1.Format( "%d", pfi->Value.Integer ) ;
				break ;
			case JE_SYMBOL_TYPE_FLOAT :
				m_csDef1.Format( "%5.2f", pfi->Value.Float ) ;
				TrimString( m_csDef1 ) ;
				break ;
			case JE_SYMBOL_TYPE_COLOR :
				m_csDef1.Format( "R:%d G:%d B:%d", (int)pfi->Value.Color.r, (int)pfi->Value.Color.g, (int)pfi->Value.Color.b ) ;
				break ;
			case JE_SYMBOL_TYPE_STRING :
				m_csDefString = pfi->Value.pChar ;
				break ;
		}// end fill inited fields
	}
	else
	{
		switch( pfi->Type )
		{
			case JE_SYMBOL_TYPE_VEC3D :
				m_csDef3 = _T("") ;
				m_csDef2 = _T("") ;
				m_csDef1 = _T("") ;
				break ;
			case JE_SYMBOL_TYPE_INT :
				m_csDef1 = _T("") ;
				break ;
			case JE_SYMBOL_TYPE_FLOAT :
				m_csDef1 = _T("") ;
				break ;
			case JE_SYMBOL_TYPE_COLOR :
				m_csDef1 = _T("") ;
				break ;
			case JE_SYMBOL_TYPE_STRING :
				m_csDefString = _T("") ;
				break ;
		}// end fill inited fields
	}// Not inited
	UpdateData( false ) ;
}// SetFields


void CEntityCP::OnBnApply() 
{
	int				iIndex ;
	FieldInfo	*	pfi ;
	CJweDoc		*	pDoc ;
	int32			nDataSize = 0 ;
	
	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument() ;

	iIndex = m_Properties.GetCurSel( ) ;
	if( iIndex != -1 && pDoc != NULL )
	{
		pfi = (FieldInfo*)m_Properties.GetItemData( iIndex ) ;
		if( GetField( pfi )	== JE_FALSE )
		{
			AfxMessageBox( IDS_OUTOFMEMORY, MB_OK, 0 ) ;
			return ;
		}
		
		switch( pfi->Type )
		{
		case JE_SYMBOL_TYPE_VEC3D :
			nDataSize = sizeof pfi->Value.Vec3d ;	break ;
		case JE_SYMBOL_TYPE_INT :
			nDataSize = sizeof pfi->Value.Integer ;	break ;
			break ;
		case JE_SYMBOL_TYPE_FLOAT :
			nDataSize = sizeof pfi->Value.Float ;	break ;
			break ;
		case JE_SYMBOL_TYPE_COLOR :
			nDataSize = sizeof pfi->Value.Color ;	break ;
			break ;
		case JE_SYMBOL_TYPE_STRING :
			nDataSize = sizeof pfi->Value.pChar ;	break ;
		default :
			ASSERT( 0 ) ;
			break ;
		}
		pfi->bInit = JE_TRUE ;
		pDoc->SetEntityField( pfi->pField, &pfi->Value, nDataSize ) ;

		m_Properties.Invalidate( true ) ;
	}
}// OnBnApply

#pragma message( "Handle Color, Enum, List, etc." ) 
jeBoolean CEntityCP::GetField(FieldInfo *pfi)
{
	char	* pszNewStr ;
	jeBoolean	bSuccess = JE_TRUE ;
	
	UpdateData( true ) ;
	switch( pfi->Type )
	{
		case JE_SYMBOL_TYPE_VEC3D :
			TrimString( m_csDef1 ) ;
			TrimString( m_csDef2 ) ;
			TrimString( m_csDef3 ) ;
			pfi->Value.Vec3d.X = (jeFloat)atof(m_csDef1);
			pfi->Value.Vec3d.Y = (jeFloat)atof(m_csDef2);
			pfi->Value.Vec3d.Z = (jeFloat)atof(m_csDef3);
			break ;

		case JE_SYMBOL_TYPE_INT :
			TrimString( m_csDef1 ) ;
			pfi->Value.Integer = atoi(m_csDef1);
			break ;
		case JE_SYMBOL_TYPE_FLOAT :
			TrimString( m_csDef1 ) ;
			pfi->Value.Float = (jeFloat)atof(m_csDef1) ;
			break ;
		case JE_SYMBOL_TYPE_COLOR :
//			m_csDef1.Format( "R:%d G:%d B:%d", (int)pfi->Value.Color.r, (int)pfi->Value.Color.g, (int)pfi->Value.Color.b ) ;
			break ;
		case JE_SYMBOL_TYPE_STRING :
			TrimString( m_csDefString ) ;
			pszNewStr = Util_StrDup( m_csDefString ) ;
			if( pszNewStr == NULL )
			{
				bSuccess = JE_FALSE ;
			}
			else
			{
				jeRam_Free( pfi->Value.pChar ) ;
				pfi->Value.pChar = pszNewStr ;
			}
			break ;
	}
	UpdateData( false ) ;
	return bSuccess ;
}// GetField

LRESULT CEntityCP::OnChangeColor( WPARAM wParam, LPARAM lParam )
{
	COLORREF		Color  = (COLORREF)lParam ;
	int				iIndex ;
	FieldInfo	*	pfi ;
	CJweDoc		*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	iIndex = m_Properties.GetCurSel( ) ;
	if( iIndex != -1 && pDoc != NULL )
	{
		pfi = (FieldInfo*)m_Properties.GetItemData( iIndex ) ;
		ASSERT( pfi->Type == JE_SYMBOL_TYPE_COLOR ) ;
		
		pfi->Value.Color.r = (jeFloat)GetRValue( Color ) ; 
		pfi->Value.Color.g = (jeFloat)GetGValue( Color ) ;
		pfi->Value.Color.b = (jeFloat)GetBValue( Color ) ;
		pDoc->SetEntityField( pfi->pField, &pfi->Value.Color, sizeof pfi->Value.Color ) ;
		m_Properties.Invalidate( true ) ;
	}
	return 0 ;
	wParam;
}// OnChangeColor

void CEntityCP::OnKillfocusEdName() 
{
	CJweDoc		*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	UpdateData( true ) ;

	TrimString( m_csName ) ;
	if( m_csName.IsEmpty() )
		m_csName = pDoc->GetFirstEntityType() ;

	if( pDoc != NULL )
		pDoc->SetSelectionName( m_csName ) ;

	UpdateData( false ) ;

}// OnKillfocusEdName

void CEntityCP::OnToolsPlaceterrain() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_TERRAIN, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_TERRAIN ) ;
	}	
}

void CEntityCP::OnToolsPlacestaircase() 
{
	// TODO: Add your control notification handler code here
	
}

void CEntityCP::OnToolsPlacespheroid() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SPHERE ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SPHEREBRUSH ) ;
	}
	
}

void CEntityCP::OnToolsPlacesheet() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SHEET ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SHEETBRUSH ) ;
	}
	
}

void CEntityCP::OnToolsPlacelight() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}	
}

void CEntityCP::OnToolsPlaceentity() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_ENTITY ) ;
	}
}

void CEntityCP::OnToolsPlacecylinder() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_CYLINDER ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_CYLNDBRUSH ) ;
	}
	
}

void CEntityCP::OnToolsPlacecube() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_BOX ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_BOXBRUSH ) ;
	}
	
}

void CEntityCP::OnToolsPlacecone() 
{
	// TODO: Add your control notification handler code here
	
}

void CEntityCP::OnToolsPlacearch() 
{
	// TODO: Add your control notification handler code here
	
}

void CEntityCP::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	m_Sheet = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	
}
