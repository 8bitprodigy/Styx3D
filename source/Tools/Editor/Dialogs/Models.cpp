/****************************************************************************************/
/*  MODELS.CPP                                                                          */
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
#include "jwe.h"
#include "models.h"
#include "AddModel.h"
#include "MainFrm.h"
#include "MfcUtil.h"
#include "Resource.h"
#include "Ram.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModel dialog


CModel::CModel(CWnd* pParent /*=NULL*/)
	: CDialog(CModel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModel)
	m_Lock = FALSE;
	//}}AFX_DATA_INIT
}

typedef struct tagModelInfo
{
	CTreeCtrlEx	*	pList ;
	HTREEITEM		hItemGroup ;
} ModelInfo ;

void CModel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModel)
	DDX_Control(pDX, GRUP_CB_CURRENT, m_CBList);
	DDX_Control(pDX, LIST_TV_ITEMS, m_List);
	DDX_Check(pDX, GROUP_LOCK, m_Lock);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModel, CDialog)
	//{{AFX_MSG_MAP(CModel)
	ON_BN_CLICKED(IDC_ADDMODEL, OnAddmodel)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(GROUP_LOCK, OnLock)
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_SELCHANGED, LIST_TV_ITEMS, OnSelchangedTvItems)
	ON_CBN_SELCHANGE(GRUP_CB_CURRENT, OnSelchangeCbCurrent)
	ON_NOTIFY(TVN_KEYDOWN, LIST_TV_ITEMS, OnKeydownTvItems)
    ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModel message handlers

BOOL CModel::OnInitDialog() 
{

	CBitmap             bitmap;

	CDialog::OnInitDialog();
	
//	PositionDialogUnderTabs( this ) ;
	m_ImageList.Create( IDR_IMAGELIST1, 18, 4, RGB(255,0,255) ) ;
	m_List.SetImageList( &m_ImageList, TVSIL_NORMAL    );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModel::OnSize(UINT nType, int cx, int cy) 
{
    CDialog::OnSize(nType, cx, cy);
    CWnd * pWnd = GetDlgItem(LIST_TV_ITEMS);
    if(pWnd)
        pWnd->MoveWindow(4,32,cx-8,cy-36);
    pWnd = GetDlgItem(IDC_ADDMODEL);
    if(pWnd)
        pWnd->MoveWindow(cx-68,4,64,24);
}

void CModel::SetCurrentDocument(CJweDoc *pDoc)
{
	Model *pModel;
	int		ComboItem;


	// Doc has changed, lists need complete rebuild
	ASSERT( pDoc != NULL ) ;

	Reset();
	ModelList_EnumModels( pDoc->GetModelList(), &m_List, CModel::ModelListCB ) ;
	ModelList_EnumModels( pDoc->GetModelList(), &m_CBList, CModel::ModelComboCB ) ;
	pModel = pDoc->GetCurrentModel();
	if( pModel != NULL )
	{
		char * Name;

		Name = Object_GetNameAndTag( (Object*)pModel);
		if( Name )
		{
			ComboItem = m_CBList.SelectString( 0, Name );
			jeRam_Free( Name );
		}
		UpdateData( true ) ;
		m_Lock= Model_IsLocked( pModel );
		UpdateData( false );
	}
}// SetCurrentDocument

jeBoolean CModel::BrushCB( Brush *pBrush, void *lParam)
{
	HTREEITEM			hItem = NULL ;
	ModelInfo		*	pModelInfo = (ModelInfo*)lParam ;
	char			*	pszDisplayName ;

	pszDisplayName = Object_GetNameAndTag( (Object*)pBrush ) ;
	if( pszDisplayName != NULL )
	{
		hItem = pModelInfo->pList->InsertItem( pszDisplayName, pModelInfo->hItemGroup, TVI_SORT ) ;

		if( hItem != NULL )
		{
			pModelInfo->pList->SetItemData( hItem, (DWORD)(Object*)pBrush ) ;
			Object_AddRef( (Object*)pBrush );
		}
		jeRam_Free( pszDisplayName ) ;
	}
	return ( hItem == NULL ) ? JE_FALSE : JE_TRUE ;
}// BrushCB

jeBoolean CModel::ModelListCB( Model *pModel, void *lParam)
{
	HTREEITEM			hItem = NULL ;
	CTreeCtrlEx		*	pList = (CTreeCtrlEx*)lParam ;
	char			*	pszDisplayName ;
	ModelInfo			ModelInfoData;

	pszDisplayName = Object_GetNameAndTag( (Object*)pModel ) ;
	if( pszDisplayName != NULL )
	{
		hItem = pList->InsertItem( pszDisplayName, TVI_ROOT, TVI_SORT ) ;

		if( hItem != NULL )
		{
			ModelInfoData.hItemGroup = hItem;
			ModelInfoData.pList = pList;
			pList->SetItemData( hItem, (DWORD)pModel ) ;
			Object_AddRef( (Object*)pModel );
			if( Model_EnumBrushes( pModel, &ModelInfoData, CModel::BrushCB ) == JE_FALSE )
				hItem = NULL ;
		}
		jeRam_Free( pszDisplayName );
	}
	return ( hItem == NULL ) ? JE_FALSE : JE_TRUE ;
}// ModelListCB

jeBoolean CModel::ModelComboCB( Model *pModel, void *lParam)
{
	int					nIndex = CB_ERR ;
	CComboBox			*pCBList = (CComboBox*)lParam ;
	char			*	pszDisplayName ;
	

	pszDisplayName = Object_GetNameAndTag( (Object*)pModel ) ;
	if( pszDisplayName != NULL )
	{
		pCBList->AddString( pszDisplayName );
		nIndex = pCBList->FindString( 0, pszDisplayName );
		if( nIndex != CB_ERR )
			pCBList->SetItemDataPtr( nIndex, pModel );
		jeRam_Free( pszDisplayName );

	}
	return ( nIndex != CB_ERR ) ? JE_FALSE : JE_TRUE ;
}// GroupCB

jeBoolean CModel::AddObject( Object* pObject )
{
	HTREEITEM hItem;
	HTREEITEM hObjectItem;
	Model *pModel;
	char  *	pszDisplayName ;
	Brush * pBrush;
	
	ASSERT( pObject );

	if( Object_GetKind( pObject ) != KIND_BRUSH )
		return( JE_TRUE );
	pBrush = (Brush*)pObject;

	hItem = m_List.GetRootItem();

	while( hItem != NULL )
	{
		pModel = (Model*)m_List.GetItemData( hItem );
		ASSERT( pModel != NULL );
		if( Brush_GetModel( pBrush ) == pModel )
		{
			pszDisplayName = Object_GetNameAndTag( (Object*)pBrush ) ;
			if( pszDisplayName == NULL )
				return( JE_FALSE );
			hObjectItem	= m_List.InsertItem( pszDisplayName, hItem, TVI_SORT );
			jeRam_Free( pszDisplayName );
			if( hObjectItem == NULL )
				return( JE_FALSE );
			m_List.SetItemData( hObjectItem, (DWORD)pBrush ) ;

			return( JE_TRUE );
		}
		hItem = m_List.GetNextSiblingItem(hItem );
	}
	return( JE_FALSE );
}

HTREEITEM CModel::GetObjectItem( CTreeCtrl *pList, Object *pObject )
{
	HTREEITEM hModelItem;
	HTREEITEM hObjectItem;
	Object *pListObject;
	Model  *pModel;
	int32 Kind;
	
	ASSERT( pObject );
	ASSERT( pList );

	hModelItem = pList->GetRootItem();

	Kind = Object_GetKind( pObject );
	ASSERT( Kind == KIND_BRUSH || Kind == KIND_MODEL );

	while( hModelItem != NULL )
	{
		pModel = (Model*)pList->GetItemData( hModelItem );
		if( pObject == (Object*)pModel )
			return( hModelItem );
		ASSERT( hModelItem != NULL );
		if( Kind == KIND_BRUSH )
		{
			if( pModel == Brush_GetModel( (Brush *)pObject ) )
			{
				hObjectItem = pList->GetChildItem( hModelItem );
				while( hObjectItem )
				{
					pListObject = (Object*)pList->GetItemData( hObjectItem );
					if( pListObject == pObject )
					{
						return( hObjectItem );
					}
					hObjectItem = pList->GetNextSiblingItem( hObjectItem );
				}
				return( 0 );
			}
		}
		hModelItem = pList->GetNextSiblingItem(hModelItem );
	}
	return( 0 );
}

void CModel::RenameObject( Object *pObject )
{
	HTREEITEM hItem;
	char * pszDisplayName;
	
	if( Object_GetKind( pObject ) != KIND_BRUSH )
		return;
	hItem = GetObjectItem( &m_List, pObject );
	if( hItem == NULL )
		return;
	pszDisplayName = Object_GetNameAndTag( pObject ) ;
	if( pszDisplayName != NULL )
	{
		m_List.SetItemText( hItem, pszDisplayName );
		jeRam_Free( pszDisplayName ) ;
	}

}

jeBoolean CModel::AddSelectionCB(Object *pObject, void *lParam)
{
	HTREEITEM hItem;
	HTREEITEM hObjectItem;
	Model *pModel;
	char  *	pszDisplayName ;
	CTreeCtrlEx	*pList = (CTreeCtrlEx	*)lParam;
	Brush * pBrush;
	
	ASSERT( pObject );
	ASSERT( pList );

	if( Object_GetKind( pObject ) != KIND_BRUSH )
		return( JE_TRUE );
	pBrush = (Brush*)pObject;

	hItem = pList->GetRootItem();

	while( hItem != NULL )
	{
		pModel = (Model*)pList->GetItemData( hItem );
		ASSERT( pModel != NULL );
		if( Brush_GetModel( pBrush ) == pModel )
		{
			pszDisplayName = Object_GetNameAndTag( pObject ) ;
			if( pszDisplayName == NULL )
				return( JE_FALSE );
			hObjectItem	= pList->InsertItem( pszDisplayName, hItem, TVI_SORT );
			jeRam_Free( pszDisplayName );
			if( hObjectItem == NULL )
				return( JE_FALSE );
			pList->SetItemData( hObjectItem, (DWORD)pObject ) ;
			Object_AddRef( pObject );
			return( JE_TRUE );
		}
		hItem = pList->GetNextSiblingItem(hItem );
	}
	return( JE_FALSE );
}

void CModel::AddSelection(CJweDoc *pDoc)
{

	pDoc->EnumSelected( &m_List, AddSelectionCB ) ;
}// AddSelection

void CModel::UpdateCurModel()
{
	CJweDoc	*		pDoc ;
	Model *		pCurModel;
	char * pName;

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	pCurModel = pDoc->GetCurrentModel(  );
	if( pCurModel )
	{
		UpdateData( true ) ;
		m_Lock= Model_IsLocked( pCurModel );
		UpdateData( false );
		pName = Object_GetNameAndTag( (Object*)pCurModel );
		if( pName )
		{
			m_CBList.SelectString( 0, pName );
			jeRam_Free( pName );
		}
	}
}

void CModel::RemoveDeleted( )
{
	HTREEITEM hGroupItem;
	HTREEITEM hObjectItem;
	HTREEITEM hNextItem;
	HTREEITEM hNextGroupItem;
	Object *pListObject;
	
	hGroupItem = m_List.GetRootItem();

	while( hGroupItem != NULL )
	{
		hObjectItem = m_List.GetChildItem( hGroupItem );
		while( hObjectItem )
		{
			pListObject = (Object*)m_List.GetItemData( hObjectItem );
			ASSERT( pListObject );
			hNextItem = m_List.GetNextSiblingItem( hObjectItem );
			if( !Object_IsInLevel( pListObject ) )
			{
				m_List.DeleteItem( hObjectItem );
				Object_Free( &pListObject );
			}
			hObjectItem = hNextItem;
		}
		hNextGroupItem = m_List.GetNextSiblingItem(hGroupItem );
		pListObject = (Object*)m_List.GetItemData( hGroupItem );
		if( !Object_IsInLevel( pListObject ) )
		{
			char * Name;
			m_List.DeleteItem( hGroupItem );
			Object_Free( &pListObject );
			Name = Object_GetNameAndTag( pListObject );
			if( Name )
			{
				int index;

				index = m_CBList.FindString( -1, Name );
				if( index >=0 )
					m_CBList.DeleteString( index );
				jeRam_Free( Name );
			}
		}
		hGroupItem = hNextGroupItem;
	}
	UpdateCurModel();
	return;
}

void CModel::ChangeModels( HTREEITEM hItem )
{
	HTREEITEM	hParentItem;
	Model	* pModel;
	Model	* pOldModel;
	Brush   * pBrush;
	CJweDoc	*		pDoc ;

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	hParentItem = m_List.GetParentItem( hItem );
	ASSERT( hParentItem );

	pBrush = (Brush*)m_List.GetItemData( hItem );
	ASSERT( pBrush );

	pModel = (Model*)m_List.GetItemData( hParentItem );
	ASSERT( pModel );

	pOldModel = Brush_GetModel( pBrush );
	ASSERT( pOldModel );


	pDoc->SelectObject( (Object*)pBrush, LEVEL_DESELECT);
	Model_RemoveBrush( pOldModel, pBrush ) ;
	Model_AddBrush( pModel, pBrush ) ;
	Model_AddBrushWorld( pModel, pBrush, JE_FALSE, JE_FALSE );
	pDoc->UpdateAll();
}

void CModel::SelectGroup( HTREEITEM hGroupItem, jeBoolean bSelect )
{
	HTREEITEM		hItem;
	LEVEL_STATE		State;
	Object *		pObject;
	CJweDoc	*		pDoc ;


	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( bSelect ) 
		State = LEVEL_SELECT;
	else
		State = LEVEL_DESELECT;

	hItem = m_List.GetChildItem( hGroupItem );
	while( hItem )
	{
		m_List.SelectItemEx( hItem, bSelect );
		pObject = (Object*)m_List.GetItemData( hItem ) ;
		if( pObject == NULL )
			return;
		pDoc->SelectObject( pObject, State ) ;
		hItem = m_List.GetNextSiblingItem( hItem );
	}
}

void CModel::OnSelchangedTvItems(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	HTREEITEM		hItem ;
	NM_TREEVIEW*	pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	Object	*		pObject ;
	CJweDoc	*		pDoc ;

	if(  pNMTreeView->action == TVC_DRAG_END )
	{
		ChangeModels( pNMTreeView->itemNew.hItem );
	}
	if( pNMTreeView->action != TVC_BYCTREECTRL )
		return ;

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc == NULL )
		return ;
	
	// I added TVC_BYTREECTRL to try to filter out the duplicate messages
	// I'm still not getting correct multi-sel messages
	
	if( !Util_IsKeyDown( VK_CONTROL ) )
		pDoc->DeselectAll( JE_FALSE );
	hItem = pNMTreeView->itemNew.hItem ;
	if( hItem != NULL )
	{
		pObject = (Object*)m_List.GetItemData( hItem ) ;
		if( pObject == NULL  )
			return;
		if(Object_GetKind(pObject) == KIND_MODEL )
		{
			pDoc->SelectObject( pObject, LEVEL_TOGGLE ) ;
		}
		else
		{
			pDoc->SelectObject( pObject, LEVEL_TOGGLE ) ;
		}
	}
	*pResult = 0;
}

jeBoolean CModel::SelectCB(Object *pObject, void *lParam)
{
	HTREEITEM hObjectItem;
	CTreeCtrlEx	*pList = (CTreeCtrlEx	*)lParam;
	int32 Kind;
	
	ASSERT( pObject );
	ASSERT( pList );

	Kind = Object_GetKind( pObject );
	if(  !(Kind == KIND_BRUSH ||  Kind == KIND_MODEL) )
		return( JE_TRUE );
	hObjectItem = GetObjectItem( pList, pObject );
	if( hObjectItem )
	{
		pList->SelectItemEx( hObjectItem );
		return( JE_TRUE );
	}
	return( JE_FALSE );
}
// SELECTION has changed


void CModel::Update(CJweDoc *pDoc)
{
	LEVEL_SEL	SelType ;
	m_List.SelectItem( NULL );

	SelType = pDoc->GetSelType( ) ;
	if( (SelType & LEVEL_SELTEMPLATE) == 0 )
	{
		pDoc->EnumSelected(&m_List, SelectCB ) ;	
	}
}// Update

void CModel::DeRefAllObjects()
{
	HTREEITEM hItem;
	HTREEITEM hObjectItem;
	Object *pObject;
	

	hItem = m_List.GetRootItem();

	while( hItem != NULL )
	{
		hObjectItem = m_List.GetChildItem( hItem );
		while( hObjectItem )
		{
			pObject = (Object*)m_List.GetItemData( hObjectItem );
			if( pObject )
				Object_Free( &pObject );
			hObjectItem = m_List.GetNextSiblingItem(hObjectItem );
		}
		pObject = (Object*)m_List.GetItemData( hItem );
		if( pObject )
			Object_Free( &pObject );
		hItem = m_List.GetNextSiblingItem(hItem );
	}
}

void CModel::Reset()
{
	DeRefAllObjects();
	m_List.DeleteAllItems( );
	m_CBList.ResetContent();
}

void CModel::OnAddmodel() 
{
	HTREEITEM		hItem ;
	CAddModel		AddModelDialog ;
	Model *			pModel;
	CMainFrame*		pMainFrm;
	CJweDoc	*		pDoc ;
	char *			Name;

	pMainFrm = (CMainFrame*)AfxGetMainWnd();
	pDoc = pMainFrm->GetCurrentDocument();

	pDoc->RenderAnimate( JE_FALSE );
//	pMainFrm->SetCurAnimateState( JE_FALSE );	
	#pragma message ("[Small bug - 7.3.2000 JH] When adding a model, animation is turned off, but the state of the toolbar-anim-button is unchanged....[EOFBUG]\n")

	AddModelDialog.m_nTitleID = IDS_NEWGROUP ;
	if( AddModelDialog.DoModal( ) == IDOK )
	{
		TrimString( AddModelDialog.m_csName ) ;
		if( AddModelDialog.m_csName.IsEmpty() )
		{
			AfxMessageBox( IDS_MUSTSUPPLYNAME, MB_OK, 0 ) ;
			return ;		
		}


		pModel = pDoc->CreateModel( AddModelDialog.m_csName  );
		if( pModel )
		{
			Name = Object_GetNameAndTag( (Object*)pModel );
			if( Name )
			{
				hItem = m_List.InsertItem( Name, TVI_ROOT, TVI_SORT ) ;
				jeRam_Free( Name );

				Object_AddRef( (Object*)pModel );
				if( hItem != NULL )
				{
					m_List.SetItemData( hItem, (DWORD)pModel ) ;
				}
			}
		}

		ModelComboCB( pModel, &m_CBList );
		Model_SetLocked( pModel, JE_TRUE );
		//m_CBList.SelectString( 0, AddModelDialog.m_csName );
		//pDoc->SetCurrentModel( pModel );
	}
	
}

void CModel::OnSelchangeCbCurrent() 
{
	int nIndex;
	Model * pModel;
	CJweDoc	*		pDoc ;

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	nIndex = m_CBList.GetCurSel();
	pModel = (Model*)m_CBList.GetItemData( nIndex );
	pDoc->SetCurrentModel( pModel );	
	UpdateData( true ) ;
	m_Lock= Model_IsLocked( pModel );
	UpdateData( false );
}

void CModel::OnLock() 
{
	CJweDoc	*		pDoc ;
	Model * pModel;

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	pModel = pDoc->GetCurrentModel();
	UpdateData( true );
	pDoc->ModelLock( pModel, m_Lock );
	UpdateData( false );
	
}

void CModel::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	UpdateData( true );
	UpdateData( false );
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}

void CModel::OnDestroy() 
{
	DeRefAllObjects();
	CDialog::OnDestroy();
		
}


void CModel::OnKeydownTvItems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;

	((CMainFrame*)AfxGetMainWnd())->PostMessage( WM_KEYDOWN, pTVKeyDown->wVKey, 0 );
	
	*pResult = 0;
}
