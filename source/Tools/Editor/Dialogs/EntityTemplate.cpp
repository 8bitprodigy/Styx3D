/****************************************************************************************/
/*  ENTITYTEMPLATE.CPP                                                                  */
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

#include "Resource.h"

#include "AddModel.h"
#include "EclipseNames.h"
#include "EntityTable.h"
#include "jwe.h"
#include "MainFrm.h"
#include "MfcUtil.h"

#include "EntityTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct tagEntityListInfo
{
	CComboBox	*	pCB ;
	jeSymbol	*	pSelect ;
} EntityListInfo ;

/////////////////////////////////////////////////////////////////////////////
// CEntityTemplate dialog


CEntityTemplate::CEntityTemplate(CWnd* pParent /*=NULL*/)
	: CDialog(CEntityTemplate::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEntityTemplate)
	m_csDefString = _T("");
	m_csDef1 = _T("");
	m_csDef2 = _T("");
	m_csDef3 = _T("");
	m_csFieldName = _T("");
	m_csName = _T("");
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	//}}AFX_DATA_INIT
	m_pEntities = NULL ;
}


void CEntityTemplate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEntityTemplate)
	DDX_Control(pDX, DEFE_BN_COLOR, m_bnColor);
	DDX_Control(pDX, DEFE_CB_FIELDTYPE, m_FieldTypeCB);
	DDX_Control(pDX, DEFE_CB_DEFTF, m_DefEnumCB);
	DDX_Control(pDX, DEFE_LB_PROPERTIES, m_Properties);
	DDX_Control(pDX, DEFE_CB_ENTITIES, m_EntitiesCB);
	DDX_Text(pDX, DEFE_ED_DEFSTRING, m_csDefString);
	DDV_MaxChars(pDX, m_csDefString, 127);
	DDX_Text(pDX, DEFE_ED_DEFV1, m_csDef1);
	DDV_MaxChars(pDX, m_csDef1, 31);
	DDX_Text(pDX, DEFE_ED_DEFV2, m_csDef2);
	DDV_MaxChars(pDX, m_csDef2, 31);
	DDX_Text(pDX, DEFE_ED_DEFV3, m_csDef3);
	DDV_MaxChars(pDX, m_csDef3, 31);
	DDX_Text(pDX, DEFE_ED_FIELDNAME, m_csFieldName);
	DDV_MaxChars(pDX, m_csFieldName, 31);
	DDX_Text(pDX, DEFE_ED_NAME, m_csName);
	DDV_MaxChars(pDX, m_csName, 31);
	DDX_Check(pDX, IDM_TOOLS_PLACEARCH, m_Arch);
	DDX_Check(pDX, IDM_TOOLS_PLACECONE, m_Cone);
	DDX_Check(pDX, IDM_TOOLS_PLACECUBE, m_Cube);
	DDX_Check(pDX, IDM_TOOLS_PLACECYLINDER, m_Cylinder);
	DDX_Check(pDX, IDM_TOOLS_PLACEENTITY, m_Entity);
	DDX_Check(pDX, IDM_TOOLS_PLACELIGHT, m_Light);
	DDX_Check(pDX, IDM_TOOLS_PLACESPHEROID, m_Sphere);
	DDX_Check(pDX, IDM_TOOLS_PLACESTAIRCASE, m_Stair);
	DDX_Check(pDX, IDM_TOOLS_PLACETERRAIN, m_Terrain);
	//}}AFX_DATA_MAP
}

IMPLEMENT_DYNCREATE(CEntityTemplate, CDialog)

BEGIN_MESSAGE_MAP(CEntityTemplate, CDialog)
	//{{AFX_MSG_MAP(CEntityTemplate)
	ON_BN_CLICKED(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_WM_DRAWITEM()
	ON_CBN_SELCHANGE(DEFE_CB_FIELDTYPE, OnSelchangeCbFieldtype)
	ON_CBN_SELCHANGE(DEFE_CB_ENTITIES, OnSelchangeCbEntities)
	ON_LBN_SELCHANGE(DEFE_LB_PROPERTIES, OnSelchangeLbProperties)
	ON_EN_SETFOCUS(DEFE_ED_FIELDNAME, OnSetfocusEdFieldname)
	ON_BN_CLICKED(DEFE_BN_ADDFIELD, OnBnAddfield)
	ON_BN_CLICKED(DEFE_BN_REMOVEFIELD, OnBnRemovefield)
	ON_BN_CLICKED(DEFE_BN_APPLY, OnBnApply)
	ON_BN_CLICKED(DEFE_BN_NEW, OnBnNew)
	ON_BN_CLICKED(DEFE_BN_DELETE, OnBnDelete)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_EN_KILLFOCUS(DEFE_ED_NAME, OnKillfocusEdName)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_BN_CLICKED(IDM_TOOLS_PLACEENTITY, OnToolsPlaceentity)
	ON_COMMAND(IDM_TOOLS_ADDBRUSH, OnToolsAddbrush)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_UPDATE, Update)
	ON_MESSAGE(WM_UPDATECOLOR, OnChangeColor)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEntityTemplate message handlers

BOOL CEntityTemplate::OnInitDialog() 
{
	int				i ;
	int				nIndex ;
	CString			cstr ;

	CDialog::OnInitDialog();
	
	SetupTemplateDialogIcons( this ) ;

	PositionDialogUnderTabs( this ) ;

	// Restrict edit control to alpha--from MSDN::CTRLTEST
	m_edit1.SubclassEdit( DEFE_ED_NAME, this, PES_LETTERS|PES_SPACE ) ;

	m_FieldTypeCB.ResetContent() ;
	for( i=0; i< IDS_ENTITYTYPE_PORTAL-IDS_ENTITYTYPE_INT; i++ )
	{
		cstr.LoadString( IDS_ENTITYTYPE_INT+i ) ;
		nIndex = m_FieldTypeCB.AddString( cstr ) ;
		if( nIndex != CB_ERR && nIndex != CB_ERRSPACE )
		{
			m_FieldTypeCB.SetItemData( nIndex, i ) ;
		}
	}
	m_FieldTypeCB.SetCurSel( 0 ) ;
	m_SymbolType = (jeSymbol_Type)m_FieldTypeCB.GetItemData( 0 ) ;
	ShowFieldsBySymbolType( m_SymbolType ) ;

	FillEntityTypes( NULL ) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}//OnInitDialog

void CEntityTemplate::OnToolsPlacecube() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_BOX ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_BOXBRUSH ) ;
	}
}// OnToolsPlacecube

void CEntityTemplate::OnToolsPlacelight() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_LIGHT, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}
}// OnToolsPlacelight

void CEntityTemplate::OnToolsPlaceterrain() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_TERRAIN, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_TERRAIN ) ;
	}	
}// OnToolsPlaceterrain

void CEntityTemplate::SetCurrentDocument(jeSymbol_Table *pEntities, const char *pszType)
{
#pragma message( "Handle current Type" )
	if( m_pEntities != pEntities )
	{
		m_pEntities = pEntities ;
		FillEntityTypes( NULL ) ;
		FillProperties( ) ;
	}
	pszType ;
}// SetCurrentDocument

void CEntityTemplate::FillEntityTypes(jeSymbol *pSelect)
{
	EntityListInfo	eli ;

	eli.pCB = &m_EntitiesCB ;
	eli.pSelect = pSelect ;

	m_EntitiesCB.ResetContent() ;
	if( m_pEntities != NULL )
	{
		EntityTable_EnumDefinitions( m_pEntities, &eli, CEntityTemplate::FillEntityTypesCB ) ;
	}
	if( pSelect == NULL )
		m_EntitiesCB.SetCurSel( 0 ) ;

}// FillEntityTypes

jeBoolean CEntityTemplate::FillEntityTypesCB(jeSymbol *pSymbol, void *lParam)
{
	int					nIndex ;
	EntityListInfo	*	peli = (EntityListInfo*)lParam ;

	nIndex = peli->pCB->AddString( jeSymbol_GetName( pSymbol ) ) ;
	if( nIndex != CB_ERR && nIndex != CB_ERRSPACE )
	{
		peli->pCB->SetItemData( nIndex, (DWORD)pSymbol ) ;
		if( peli->pSelect && jeSymbol_Compare( pSymbol, peli->pSelect) == JE_TRUE )
			peli->pCB->SetCurSel( nIndex ) ;
	}
	return JE_TRUE ;
}// FillEntityTypesCB

void CEntityTemplate::FillProperties()
{
	int				nIndex ;
	int				nItem ;
	jeSymbol	*	pEntityDef ;
	jeSymbol	*	pField ;
	jeSymbol_List *	pFieldList ;
	jeBoolean		bSuccess ;

	m_Properties.ResetContent() ;
	if( m_pEntities == NULL )
		return ;

	nIndex = m_EntitiesCB.GetCurSel() ;
	pEntityDef = (jeSymbol*)m_EntitiesCB.GetItemData( nIndex ) ;

	bSuccess = jeSymbol_GetProperty
	(	
		pEntityDef, 
		jeEclipseNames( m_pEntities, JE_ECLIPSENAMES_STRUCTUREFIELDS), 
		&pFieldList, 
		sizeof pFieldList, 
		JE_SYMBOL_TYPE_LIST
	) ;
	if( bSuccess )	// No fields if not set
	{
		nIndex = 0 ;
		while( bSuccess && (pField = jeSymbol_ListGetSymbol( pFieldList, nIndex )) != NULL )
		{
			nIndex++ ;
			nItem = m_Properties.AddString( jeSymbol_GetName( pField ) ) ;
			if( nItem != LB_ERR && nItem != LB_ERRSPACE )
			{
				m_Properties.SetItemData( nItem, (DWORD)pField ) ;
			}
		}
		m_Properties.SetCurSel( 0 ) ;
		jeSymbol_ListDestroy( &pFieldList ) ;
	}

}//FillProperties

void CEntityTemplate::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
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
	jeSymbol_Type	Type ;
	jeFloat			fValue ;
	jeVec3d			Vec ;
	int				nValue ;
	JE_RGBA			Color ;
	char		*	pChar ;

	if( nIDCtl != DEFE_LB_PROPERTIES )
	{
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
		return ;
	}
	
	if( lpDrawItemStruct->itemID == -1 )
		return ;

	pField = (jeSymbol*)lpDrawItemStruct->itemData ;
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
	
	
	Type = jeSymbol_GetType( pField ) ;
	switch( Type )
	{
		case JE_SYMBOL_TYPE_VEC3D :
			jeSymbol_GetProperty( pField, pField, &Vec, sizeof Vec, Type ) ;
			sprintf( szValue, "%4.2f %4.2f %4.2f", Vec.X, Vec.Y, Vec.Z ) ;
			break ;
		case JE_SYMBOL_TYPE_INT :
			jeSymbol_GetProperty( pField, pField, &nValue, sizeof nValue, Type ) ;
			sprintf( szValue, "%d", nValue ) ;
			break ;
		case JE_SYMBOL_TYPE_FLOAT :
			jeSymbol_GetProperty( pField, pField, &fValue, sizeof fValue, Type ) ;
			sprintf( szValue, "%4.2f", fValue ) ;
			break ;
		case JE_SYMBOL_TYPE_COLOR :
			jeSymbol_GetProperty( pField, pField, &Color, sizeof Color, Type ) ;
			sprintf( szValue, "R:%d G:%d B:%d", (int)Color.r, (int)Color.g, (int)Color.b ) ;
			break ;
		case JE_SYMBOL_TYPE_STRING :
			jeSymbol_GetProperty( pField, pField, &pChar, sizeof szValue, Type ) ;
			strcpy( szValue, pChar ) ;
			break ;
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

void CEntityTemplate::ShowFieldsBySymbolType(jeSymbol_Type Type )
{
	CString	cstr ;
	int		nIndex ;

	UpdateData( true ) ;

	GetDlgItem( DEFE_ED_DEFSTRING )->ShowWindow( false ) ;
	GetDlgItem( DEFE_ED_DEFV1 )->ShowWindow( false ) ;
	GetDlgItem( DEFE_ED_DEFV2 )->ShowWindow( false ) ;
	GetDlgItem( DEFE_ED_DEFV3 )->ShowWindow( false ) ;
	GetDlgItem( DEFE_CB_DEFTF )->ShowWindow( false ) ;
	GetDlgItem( DEFE_BN_COLOR )->ShowWindow( false ) ;

	m_csDefString = _T("");
	m_csDef1 = _T("");
	m_csDef2 = _T("");
	m_csDef3 = _T("");

	switch( Type )
	{
	case JE_SYMBOL_TYPE_VEC3D :
		GetDlgItem( DEFE_ED_DEFV3 )->ShowWindow( true ) ;
		GetDlgItem( DEFE_ED_DEFV2 )->ShowWindow( true ) ;
		// Fall Thru

	case JE_SYMBOL_TYPE_INT :
	case JE_SYMBOL_TYPE_FLOAT :
		GetDlgItem( DEFE_ED_DEFV1 )->ShowWindow( true ) ;
		GetDlgItem( DEFE_ED_DEFV1 )->SetFocus() ;
		break ;

	case JE_SYMBOL_TYPE_ENUM :
	case JE_SYMBOL_TYPE_STRING :
		GetDlgItem( DEFE_ED_DEFSTRING )->ShowWindow( true ) ;
		GetDlgItem( DEFE_ED_DEFSTRING )->SetFocus() ;
		break ;

	case JE_SYMBOL_TYPE_COLOR :
		GetDlgItem( DEFE_BN_COLOR )->ShowWindow( true ) ;
		GetDlgItem( DEFE_BN_COLOR )->SetFocus() ;
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
		GetDlgItem( DEFE_CB_DEFTF )->ShowWindow( true ) ;
		GetDlgItem( DEFE_CB_DEFTF )->SetFocus() ;
		break ;
	}

	UpdateData( false ) ;

}// ShowFieldsBySymbolType

void CEntityTemplate::OnSelchangeCbFieldtype() 
{
	int	nIndex ;

	nIndex = m_FieldTypeCB.GetCurSel() ;
	m_SymbolType = (jeSymbol_Type)m_FieldTypeCB.GetItemData( nIndex ) ;
	ShowFieldsBySymbolType( m_SymbolType ) ;

}// OnSelchangeCbFieldtype

void CEntityTemplate::OnSelchangeCbEntities() 
{
	FillProperties() ;	
	OnSelchangeLbProperties() ;

	Update(0,0) ;	// Change template type if possible
}// OnSelchangeCbEntities

void CEntityTemplate::OnSelchangeLbProperties() 
{
	int				nItem ;
	jeSymbol_Type	Type ;

	nItem = m_Properties.GetCurSel() ;	// LB item
	if( nItem != -1 )
	{
		jeSymbol	*	pField ;
		
		pField = (jeSymbol*)m_Properties.GetItemData( nItem ) ;
		Type = jeSymbol_GetType( pField ) ;
		m_FieldTypeCB.SetCurSel( Type ) ;

		ShowFieldsBySymbolType( Type ) ;
		SetFields( pField ) ;

		GetDlgItem( DEFE_CB_FIELDTYPE )->EnableWindow( false ) ;
		GetDlgItem( DEFE_BN_REMOVEFIELD )->EnableWindow( true ) ;
		GetDlgItem( DEFE_BN_ADDFIELD )->EnableWindow( false ) ;
		GetDlgItem( DEFE_BN_APPLY )->EnableWindow( true ) ;
	}
}// OnSelchangeLbProperties

void CEntityTemplate::SetFields(jeSymbol *pField)
{
	jeSymbol_Type	Type ;
	jeFloat			fValue ;
	jeVec3d			Vec ;
	int				nValue ;
	JE_RGBA			Color ;
	char		*	pChar ;

	UpdateData( true ) ;
	Type = jeSymbol_GetType( pField ) ;

	switch( Type )
	{
		case JE_SYMBOL_TYPE_VEC3D :
			jeSymbol_GetProperty( pField, pField, &Vec, sizeof Vec, Type ) ;
			m_csDef3.Format( "%f", Vec.Z ) ;
			m_csDef2.Format( "%f", Vec.Y ) ;
			m_csDef1.Format( "%f", Vec.X ) ;
			break ;
		case JE_SYMBOL_TYPE_INT :
			jeSymbol_GetProperty( pField, pField, &nValue, sizeof nValue, Type ) ;
			m_csDef1.Format( "%d", nValue ) ;
			break ;
		case JE_SYMBOL_TYPE_FLOAT :
			jeSymbol_GetProperty( pField, pField, &fValue, sizeof fValue, Type ) ;
			m_csDef1.Format( "%f", fValue ) ;
			break ;
		case JE_SYMBOL_TYPE_COLOR :
			jeSymbol_GetProperty( pField, pField, &Color, sizeof Color, Type ) ;
			m_csDef1.Format( "R:%d G:%d B:%d", (int)Color.r, (int)Color.g, (int)Color.b ) ;
			break ;
		case JE_SYMBOL_TYPE_STRING :
			jeSymbol_GetProperty( pField, pField, &pChar, sizeof pChar, Type ) ;
			m_csDefString = pChar ;
			break ;
	}
	UpdateData( false ) ;
}// SetFields

void CEntityTemplate::OnSetfocusEdFieldname() 
{
	m_Properties.SetCurSel( -1 ) ;
	m_Properties.Invalidate( true ) ;
	GetDlgItem( DEFE_CB_FIELDTYPE )->EnableWindow( true ) ;
	GetDlgItem( DEFE_BN_REMOVEFIELD )->EnableWindow( false ) ;
	GetDlgItem( DEFE_BN_REMOVEFIELD )->EnableWindow( false ) ;
	GetDlgItem( DEFE_BN_ADDFIELD )->EnableWindow( true ) ;
	GetDlgItem( DEFE_BN_APPLY )->EnableWindow( false ) ;
}// OnSetfocusEdFieldname

void CEntityTemplate::OnBnAddfield() 
{
	jeSymbol_Type	Type ;
	int				iIndex ;
	int				iEntity ;
	char			szDefault[ENTITY_MAXSTRINGLENGTH] ;
	jeSymbol	*	pEntity ;

	UpdateData( true ) ;		// Get the fields
	TrimString( m_csFieldName ) ;
	TrimString( m_csDefString ) ;
	TrimString( m_csDef1 ) ;
	TrimString( m_csDef2 ) ;
	TrimString( m_csDef3 ) ;

	iEntity = m_EntitiesCB.GetCurSel() ;
	ASSERT( iEntity != -1 ) ;
	pEntity = (jeSymbol*)m_EntitiesCB.GetItemData( iEntity ) ;
	ASSERT( pEntity != NULL ) ;

	if( m_csFieldName.IsEmpty() )
	{
		AfxMessageBox( IDS_MUSTSUPPLYNAME, MB_OK, 0 ) ;
		GetDlgItem( DEFE_ED_FIELDNAME )->SetFocus() ;
		return ;
	}
	
	iIndex = m_FieldTypeCB.GetCurSel() ;
	Type = (jeSymbol_Type)m_FieldTypeCB.GetItemData( iIndex ) ;
	if( !GetFieldData( Type, szDefault ) )
		return ;

	if( !EntityTable_AddField( m_pEntities, pEntity, m_csFieldName, Type, szDefault ) )
	{
		AfxMessageBox( IDS_OUTOFMEMORY, MB_OK, 0 ) ;
		return ;
	}

	if( !EntityTable_AddFieldToInstances( m_pEntities, pEntity, m_csFieldName, Type, szDefault ) )
	{
		AfxMessageBox( IDS_OUTOFMEMORY, MB_OK, 0 ) ;
		return ;
	}

	FillProperties() ;
}// OnBnAddfield

void CEntityTemplate::OnBnRemovefield() 
{
	int	iField ;
	int	iEntity ;

	iField = m_Properties.GetCurSel( ) ;
	if( iField != -1 )
	{	
		jeSymbol	*	pField ;
		jeSymbol	*	pEntity ;

		pField = (jeSymbol*)m_Properties.GetItemData( iField ) ;
		iEntity = m_EntitiesCB.GetCurSel() ;
		ASSERT( iEntity != -1 ) ;
		pEntity = (jeSymbol*)m_EntitiesCB.GetItemData( iEntity ) ;
		ASSERT( pEntity != NULL ) ;
		
		// The level must remove this field from all instances...
		CJweDoc*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument() ;
		ASSERT( pDoc->IsKindOf(RUNTIME_CLASS(CJweDoc)) ) ;

		pDoc->RemoveEntityField( pEntity, pField ) ;
		EntityTable_RemoveDefaultEntityField( m_pEntities, pField ) ;
		m_Properties.DeleteString( iField ) ;
		OnSelchangeLbProperties() ;
	}
}// OnBnRemovefield

void CEntityTemplate::OnBnApply() 
{
	int				iEntity ;
	int				iField ;
	jeSymbol	*	pEntityDef ;
	jeSymbol	*	pField ;
	char			szValue[ENTITY_MAXSTRINGLENGTH] ;

	iEntity = m_EntitiesCB.GetCurSel() ;
	ASSERT( iEntity != -1 ) ;
	pEntityDef = (jeSymbol*)m_EntitiesCB.GetItemData( iEntity ) ;
	ASSERT( pEntityDef != NULL ) ;

	iField = m_Properties.GetCurSel( ) ;
	pField = (jeSymbol*)m_Properties.GetItemData( iField ) ;

	if( GetFieldData( jeSymbol_GetType( pField ), szValue ) == JE_FALSE )
		return ;
	
	EntityTable_SetDefaultValue( m_pEntities, pField, szValue ) ;

	m_Properties.Invalidate() ;
}// OnBnApply

void CEntityTemplate::OnBnNew() 
{
	CAddModel		AddModelDialog ;
	jeSymbol	*	pEntityDef ;
	EntityListInfo	eli ;

	AddModelDialog.m_nTitleID = IDS_NEWENTITY ;
	if( AddModelDialog.DoModal( ) == IDOK )
	{
		TrimString( AddModelDialog.m_csName ) ;
		if( AddModelDialog.m_csName.IsEmpty() )
		{
			AfxMessageBox( IDS_MUSTSUPPLYNAME, MB_OK, 0 ) ;
			return ;		
		}
		pEntityDef = EntityTable_CreateType( m_pEntities, AddModelDialog.m_csName ) ;
		if( pEntityDef == NULL )
		{
			AfxMessageBox( IDS_FAILEDTOCREATE, MB_OK, 0 ) ;
			return ;
		}
		// Add to and select 
		eli.pCB = &m_EntitiesCB ;
		eli.pSelect = pEntityDef ;
		
		m_EntitiesCB.ResetContent() ;
		EntityTable_EnumDefinitions( m_pEntities, &eli, CEntityTemplate::FillEntityTypesCB ) ;
		
		FillProperties() ;
		OnSelchangeLbProperties() ;
	}
}// OnBnNew

void CEntityTemplate::OnBnDelete() 
{
	int					iEntity ;
	jeSymbol_List	*	pList ;
	jeSymbol		*	pEntityDef ;
	int					nInstances ;
	CString				csMessage ;
	int					nItemsLeft ;
	int					nRet ;
	
	iEntity = m_EntitiesCB.GetCurSel() ;
	ASSERT( iEntity != -1 ) ;
	pEntityDef = (jeSymbol*)m_EntitiesCB.GetItemData( iEntity ) ;
	ASSERT( pEntityDef != NULL ) ;
	
	// Get all the instances (and the def) in a list
	pList = jeSymbol_TableGetQualifiedSymbolList( m_pEntities, jeSymbol_GetQualifier(pEntityDef) ) ;
	nInstances = EntityTable_ListGetNumItems( pList ) - 1 ;
	jeSymbol_ListDestroy( &pList ) ;
	if( nInstances != 0 )
	{
		csMessage.Format( IDS_CONFIRMDELETEENTITIES, nInstances ) ;
		nRet = AfxMessageBox( csMessage, MB_OKCANCEL, 0 ) ;
	}
	else
	{
		nRet = IDOK ;
	}

	if( IDOK == nRet )
	{
		EntityTable_RemoveEntityAndInstances( m_pEntities, pEntityDef ) ;
		m_EntitiesCB.DeleteString( iEntity ) ;
		nItemsLeft = m_EntitiesCB.GetCount() ;
		if( nItemsLeft > 0 )
		{
			if( iEntity < nItemsLeft )
				m_EntitiesCB.SetCurSel( iEntity ) ;
			else
				m_EntitiesCB.SetCurSel( nItemsLeft-1 ) ;

		}
		FillProperties() ;
		OnSelchangeLbProperties() ;
	}
}// OnBnDelete

jeBoolean CEntityTemplate::GetFieldData(jeSymbol_Type Type, char *pszDefaultValue)
{
	int				iIndex ;

	UpdateData( true ) ;
	switch( Type )
	{
	case JE_SYMBOL_TYPE_BOOLEAN :
		iIndex = m_DefEnumCB.GetCurSel( ) ;
		if( m_DefEnumCB.GetItemData( iIndex ) == JE_TRUE )
			strcpy( pszDefaultValue, "1" ) ;
		else
			strcpy( pszDefaultValue, "0" ) ;
		break ;

	case JE_SYMBOL_TYPE_COLOR :
#pragma message( "fix color" )
		sprintf( pszDefaultValue, "%d %d %d", 1,2,3 ) ;
		break ;

	case JE_SYMBOL_TYPE_STRING :
		if( m_csDefString.IsEmpty() )
		{
			AfxMessageBox( IDS_MUSTSUPPLYDEFAULT, MB_OK, 0 ) ;
			GetDlgItem( DEFE_ED_DEFSTRING )->SetFocus() ;
			return JE_FALSE ;
		}
		strcpy( pszDefaultValue, m_csDefString ) ;
		break ;

	case JE_SYMBOL_TYPE_INT :
	case JE_SYMBOL_TYPE_FLOAT :
		if( m_csDef1.IsEmpty() )
		{
			AfxMessageBox( IDS_MUSTSUPPLYDEFAULT, MB_OK, 0 ) ;
			GetDlgItem( DEFE_ED_DEFV1 )->SetFocus() ;
			return JE_FALSE ;
		}
		strcpy( pszDefaultValue, m_csDef1 ) ;
		break ;

	case JE_SYMBOL_TYPE_VEC3D :
		if( m_csDef1.IsEmpty() || m_csDef2.IsEmpty() || m_csDef3.IsEmpty() )
		{
			AfxMessageBox( IDS_MUSTSUPPLYDEFAULT, MB_OK, 0 ) ;
			GetDlgItem( DEFE_ED_DEFV1 )->SetFocus() ;
			return JE_FALSE ;
		}
		sprintf( pszDefaultValue, "%s %s %s", m_csDef1, m_csDef2, m_csDef3 ) ;
		break ;

	case JE_SYMBOL_TYPE_ENUM :
		ASSERT( 0 ) ;
		break ;

	default :
		pszDefaultValue[0] = 0 ;
	}
	return JE_TRUE ;
}// GetFieldData

LRESULT CEntityTemplate::Update( WPARAM wParam, LPARAM lParam )
{
	// This function is called by MainFrm when a template subtab is chosen
	// Here we see if there is a good selected entity and set that mode
	int			iIndex ;
	jeSymbol *	pEntityDef ;

	if( m_pEntities == NULL )
		return 0 ;

	iIndex = m_EntitiesCB.GetCurSel() ;
	if( iIndex != -1 )
	{
		CJweDoc * pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument() ;

		pEntityDef = (jeSymbol*)m_EntitiesCB.GetItemData( iIndex ) ;
		if( pDoc != NULL )
		{
			pDoc->SetEntityTemplateType( jeSymbol_GetName( pEntityDef ) ) ;
			pDoc->SetTemplateMode( KIND_ENTITY, 0 ) ;
		}
		UpdateData( true ) ;
		m_csName = jeSymbol_GetName( jeSymbol_GetQualifier( pEntityDef ) ) ;
		UpdateData( false ) ;
	}
	return 0 ;
	wParam;lParam;
}// Update	

LRESULT CEntityTemplate::OnChangeColor( WPARAM wParam, LPARAM lParam )
{
	COLORREF		Color  = (COLORREF)lParam ;
	int				iEntity ;
	int				iField ;
	CJweDoc		*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	jeSymbol	*	pEntityDef ;
	jeSymbol	*	pField ;
	char			szValue[ENTITY_MAXSTRINGLENGTH] ;

	iEntity = m_EntitiesCB.GetCurSel( ) ;
	if( iEntity != -1 && pDoc != NULL )
	{
		pEntityDef = (jeSymbol*)m_EntitiesCB.GetItemData( iEntity ) ;
		iField = m_Properties.GetCurSel() ;
		ASSERT( iField != -1 ) ;
		pField = (jeSymbol*)m_Properties.GetItemData( iField ) ;
		ASSERT( pField != NULL ) ;

		sprintf( szValue, "%d %d %d", GetRValue( Color ),GetGValue( Color ),GetBValue( Color ) ) ;
		EntityTable_SetDefaultValue( m_pEntities, pField, szValue ) ;
		m_Properties.Invalidate( true ) ;
	}
	return 0 ;
	wParam;
}// OnChangeColor

void CEntityTemplate::OnKillfocusEdName() 
{
	int			iIndex ;
	jeSymbol *	pEntityDef ;

	UpdateData( true ) ;

	TrimString( m_csName ) ;
	if( m_csName.IsEmpty() )
	{
		iIndex = m_EntitiesCB.GetCurSel() ;
		if( iIndex == -1 )	// No Defintions
		{
			AfxMessageBox( IDS_NOSELECTION, MB_OK, 0 ) ;
			UpdateData( false ) ;
			GetDlgItem( DEFE_ED_NAME )->SetFocus() ;
			return ;
		}
		pEntityDef = (jeSymbol*)m_EntitiesCB.GetItemData( iIndex ) ;		
		m_csName = jeSymbol_GetName( jeSymbol_GetQualifier( pEntityDef ) ) ;
	}

	UpdateData( false ) ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc != NULL )
	{
		Object_SetName( pDoc->GetTemplate(), m_csName, SELECT_INVALID_NNUMBER ) ;
	}
}// OnKillfocusEdName

void CEntityTemplate::OnToolsPlacespheroid() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SPHERE ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SPHEREBRUSH ) ;
	}
	
}

void CEntityTemplate::OnToolsPlaceentity() 
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	UpdateData( true );
	pDoc->SetTemplateVisable( m_Entity );
	UpdateData( false );
}

BOOL CEntityTemplate::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( wParam == 1 )
	{
		pDoc->Addbrush();
		return( false );
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void CEntityTemplate::OnToolsAddbrush() 
{
	// TODO: Add your command handler code here
	
}

void CEntityTemplate::OnToolsPlacecylinder() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_CYLINDER ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_CYLNDBRUSH ) ;
	}
	
}

void CEntityTemplate::OnAdd() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Addbrush();
}

void CEntityTemplate::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	UpdateData( true );
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = TRUE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	UpdateData( false );
	
}

void CEntityTemplate::OnToolsPlacesheet() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SHEET ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SHEETBRUSH ) ;
	}
	
}
