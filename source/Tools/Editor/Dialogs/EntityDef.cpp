/****************************************************************************************/
/*  ENTITYDEF.CPP                                                                       */
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

#include "Resource.h"	// Need first

#include "AddModel.h"
#include "Doc.h"
#include "EclipseNames.h"
#include "EntityTable.h"
#include "jwe.h"
#include "MainFrm.h"
#include "MfcUtil.h"

#include "EntityDef.h"

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
// CEntityDef dialog


CEntityDef::CEntityDef(CWnd* pParent /*=NULL*/)
	: CDialog(CEntityDef::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEntityDef)
	m_csDefString = _T("");
	m_csDef1 = _T("");
	m_csDef2 = _T("");
	m_csDef3 = _T("");
	m_csFieldName = _T("");
	//}}AFX_DATA_INIT
}


void CEntityDef::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEntityDef)
	DDX_Control(pDX, DEFE_CB_DEFTF, m_DefEnumCB);
	DDX_Control(pDX, DEFE_LB_PROPERTIES, m_Properties);
	DDX_Control(pDX, DEFE_CB_FIELDTYPE, m_FieldTypeCB);
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
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEntityDef, CDialog)
	//{{AFX_MSG_MAP(CEntityDef)
	ON_WM_DRAWITEM()
	ON_CBN_SELCHANGE(DEFE_CB_FIELDTYPE, OnSelchangeCbFieldtype)
	ON_CBN_SELCHANGE(DEFE_CB_ENTITIES, OnSelchangeCbEntities)
	ON_EN_SETFOCUS(DEFE_ED_FIELDNAME, OnSetfocusEdFieldname)
	ON_LBN_SELCHANGE(DEFE_LB_PROPERTIES, OnSelchangeLbProperties)
	ON_BN_CLICKED(DEFE_BN_REMOVEFIELD, OnBnRemovefield)
	ON_BN_CLICKED(DEFE_BN_ADDFIELD, OnBnAddfield)
	ON_BN_CLICKED(DEFE_BN_APPLY, OnBnApply)
	ON_BN_CLICKED(DEFE_BN_NEW, OnBnNew)
	ON_BN_CLICKED(DEFE_BN_DELETE, OnBnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEntityDef message handlers

BOOL CEntityDef::OnInitDialog() 
{
	int				i ;
	int				nIndex ;
	CString			cstr ;
	EntityListInfo	eli ;

	CDialog::OnInitDialog();
	
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

	ASSERT( m_pEntities != NULL ) ;
	eli.pCB = &m_EntitiesCB ;
	eli.pSelect = NULL ;
	m_EntitiesCB.ResetContent() ;
	EntityTable_EnumDefinitions( m_pEntities, &eli, CEntityDef::UpdateDefinitionsCB ) ;
 	m_EntitiesCB.SetCurSel( 0 ) ;

	FillProperties() ;
	OnSelchangeLbProperties() ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}// OnInitDialog

void CEntityDef::FillProperties()
{
	int				nIndex ;
	int				nItem ;
	jeSymbol	*	pEntityDef ;
	jeSymbol	*	pField ;
	jeSymbol_List *	pFieldList ;
	jeBoolean		bSuccess ;

	m_Properties.ResetContent() ;

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
	}
}// FillProperties

void CEntityDef::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
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
			sprintf( szValue, "R:%f G:%f, B:%f", Color.r, Color.g, Color.b ) ;
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


void CEntityDef::OnSelchangeCbFieldtype() 
{
	int	nIndex ;

	nIndex = m_FieldTypeCB.GetCurSel() ;
	m_SymbolType = (jeSymbol_Type)m_FieldTypeCB.GetItemData( nIndex ) ;
	ShowFieldsBySymbolType( m_SymbolType ) ;
}

void CEntityDef::ShowFieldsBySymbolType(jeSymbol_Type Type )
{
	CString	cstr ;
	int		nIndex ;

	UpdateData( true ) ;

	GetDlgItem( DEFE_ED_DEFSTRING )->ShowWindow( false ) ;
	GetDlgItem( DEFE_ED_DEFV1 )->ShowWindow( false ) ;
	GetDlgItem( DEFE_ED_DEFV2 )->ShowWindow( false ) ;
	GetDlgItem( DEFE_ED_DEFV3 )->ShowWindow( false ) ;
	GetDlgItem( DEFE_CB_DEFTF )->ShowWindow( false ) ;
	GetDlgItem( DEFE_BN_COLOR )->EnableWindow( false ) ;

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
		GetDlgItem( DEFE_BN_COLOR )->EnableWindow( true ) ;
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

jeBoolean CEntityDef::UpdateDefinitionsCB(jeSymbol *pSymbol, void *lParam)
{
	int					nIndex ;
	EntityListInfo	*	peli = (EntityListInfo*)lParam ;

	nIndex = peli->pCB->AddString( jeSymbol_GetName( pSymbol ) ) ;
	if( nIndex != CB_ERR && nIndex != CB_ERRSPACE )
	{
		peli->pCB->SetItemData( nIndex, (DWORD)pSymbol ) ;
		if( pSymbol == peli->pSelect )
			peli->pCB->SetCurSel( nIndex ) ;
	}
	return JE_TRUE ;
}// UpdateDefinitionsCB

void CEntityDef::OnSelchangeCbEntities() 
{
	FillProperties() ;	
	OnSelchangeLbProperties() ;

}// OnSelchangeCbEntities

void CEntityDef::OnSetfocusEdFieldname() 
{
	m_Properties.SetCurSel( -1 ) ;
	m_Properties.Invalidate( true ) ;
	GetDlgItem( DEFE_CB_FIELDTYPE )->EnableWindow( true ) ;
	GetDlgItem( DEFE_BN_REMOVEFIELD )->EnableWindow( false ) ;
	GetDlgItem( DEFE_BN_REMOVEFIELD )->EnableWindow( false ) ;
	GetDlgItem( DEFE_BN_ADDFIELD )->EnableWindow( true ) ;
	GetDlgItem( DEFE_BN_APPLY )->EnableWindow( false ) ;
}// OnSetfocusEdFieldname


void CEntityDef::OnSelchangeLbProperties() 
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

void CEntityDef::OnBnRemovefield() 
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

void CEntityDef::OnBnAddfield() 
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

void CEntityDef::SetFields(jeSymbol *pField)
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


jeBoolean CEntityDef::GetFieldData(jeSymbol_Type Type, char *pszDefaultValue)
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


void CEntityDef::OnBnApply() 
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

void CEntityDef::OnBnNew() 
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
		EntityTable_EnumDefinitions( m_pEntities, &eli, CEntityDef::UpdateDefinitionsCB ) ;
		
		FillProperties() ;
		OnSelchangeLbProperties() ;
	}
}//OnBnNew

void CEntityDef::OnBnDelete() 
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
