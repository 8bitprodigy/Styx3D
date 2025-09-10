/****************************************************************************************/
/*  LISTS.CPP                                                                           */
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
#include "MainFrm.h"
#include "MfcUtil.h"
#include "Ram.h"
#include "Class.h"
#include "Lists.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLists dialog


CLists::CLists(CWnd* pParent /*=NULL*/)
	: CDialog(CLists::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLists)
	//}}AFX_DATA_INIT
}


void CLists::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLists)
	DDX_Control(pDX, LIST_TV_ITEMS, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLists, CDialog)
	//{{AFX_MSG_MAP(CLists)
	ON_NOTIFY(TVN_SELCHANGED, LIST_TV_ITEMS, OnSelchangedTvItems)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
    ON_WM_SIZE()
	ON_NOTIFY(TVN_KEYDOWN, LIST_TV_ITEMS, OnKeydownTvItems)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CLists message handlers

BOOL CLists::OnInitDialog() 
{
	CString		cstr ;
	CDialog::OnInitDialog();
	
//	PositionDialogUnderTabs( this ) ;

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}// OnInitDialog

void CLists ::OnSize(UINT nType, int cx, int cy) 
{
    CDialog::OnSize(nType, cx, cy);
    CWnd * pWnd = GetDlgItem(LIST_TV_ITEMS);
    if(pWnd)
        pWnd->MoveWindow(4,4,cx-8,cy-8);
}

void CLists::SetCurrentDocument(CJweDoc *pDoc)
{

	// Doc has changed, lists need complete rebuild
	ASSERT( pDoc != NULL ) ;

	Reset();
	pDoc->EnumObjects( &m_List, CLists::ObjectCB ) ;
}// SetCurrentDocument

void CLists::Reset()
{
	DeRefAllObjects();
	m_List.DeleteAllItems( );

}

void CLists::AddSelection(CJweDoc *pDoc)
{

	pDoc->EnumSelected( &m_List, AddSelectionCB ) ;
}// AddSelection

void CLists::RemoveDeleted( )
{
	HTREEITEM hGroupItem;
	HTREEITEM hObjectItem;
	HTREEITEM hNextItem;
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
		hGroupItem = m_List.GetNextSiblingItem(hGroupItem );
	}
	return;
}

void CLists::AddObject(Object *pObject)
{

	AddSelectionCB( pObject, &m_List ) ;
}// AddObject

void CLists::RenameObject( Object *pObject )
{
	HTREEITEM hItem;
	char * pszDisplayName;
	
	hItem = FindObjectItem( pObject );
	if( hItem == NULL )
		return;
	pszDisplayName = Object_GetNameAndTag( pObject ) ;
	if( pszDisplayName != NULL )
	{
		m_List.SetItemText( hItem, pszDisplayName );
		jeRam_Free( pszDisplayName ) ;
	}

}

// SELECTION has changed
void CLists::Update(CJweDoc *pDoc)
{
	m_List.ClearSelection( false ) ;	// From derived TV
	pDoc->EnumSelected( &m_List, SelectCB ) ;	
}// Update


jeBoolean CLists::ModelCB(Model *pModel, void *lParam)
{
	Model_EnumBrushes( pModel, lParam, (BrushListCB)CLists::ObjectCB );
	return( JE_TRUE );
}// ModelCB

HTREEITEM CLists::FindObjectKind( CTreeCtrlEx * pList, Object * pObject )
{
	HTREEITEM hItem;
	int Kind;
	CString ItemText;
	char * KindName;
	Class * pClass;
	
	Kind = Object_GetKind( pObject);
	hItem = pList->GetRootItem();

	while( hItem != NULL )
	{
		pClass = (Class*)pList->GetItemData( hItem );
		if( Class_GetClassKind( pClass ) == Kind )
		{
			if( Kind == KIND_USEROBJ )
			{
				ItemText = pList->GetItemText( hItem );
				KindName = Object_CreateKindName( pObject );
				if( KindName != NULL )
				{
					if( strcmp( KindName, ItemText.GetBuffer(0) ) == 0 )
					{
						jeRam_Free( KindName );
						return( hItem );
					}
					jeRam_Free( KindName );
				}
			}
			else
				return( hItem );
		}
		hItem = pList->GetNextSiblingItem(hItem );
	}
	return( NULL );
}

HTREEITEM CLists::AddObjectKind( CTreeCtrlEx * pList, char * pszDisplayName, int Kind )
{
	HTREEITEM			hItem = NULL ;
	Class				* pClass;

	CJweDoc	*		pDoc ;


	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	hItem = pList->InsertItem( pszDisplayName, NULL, TVI_SORT ) ;
	if( hItem != NULL)
	{
		pClass = pDoc->CreateClass( pszDisplayName, Kind );
		if( pClass == NULL )
		{
			pList->DeleteItem( hItem );
			return( NULL );
		}
		pList->SetItemData( hItem, (long)pClass ) ;
	}
	return( hItem );
}

jeBoolean CLists::ObjectCB(Object *pObject, void *lParam)
{
	HTREEITEM			hItem = NULL ;
	HTREEITEM			hParentItem;
	CTreeCtrlEx	*	pList = (CTreeCtrlEx*)lParam ;
	char			*	pszDisplayName ;
	char			*	pszKindName;


	pszDisplayName = Object_GetNameAndTag( pObject ) ;
	if( pszDisplayName != NULL )
	{
		hParentItem = FindObjectKind( pList, pObject );
		if( hParentItem == NULL )
		{
			pszKindName = Object_CreateKindName(  pObject );
			if( pszKindName == NULL )
				return( JE_FALSE );
			hParentItem = AddObjectKind( pList, pszKindName, Object_GetKind( pObject) );
			jeRam_Free( pszKindName );
			if( hParentItem == NULL )
				return( JE_FALSE );
		}
		hItem = pList->InsertItem( pszDisplayName, hParentItem, TVI_SORT ) ;
		if( hItem != NULL )
		{
			pList->SetItemData( hItem, (DWORD)pObject ) ;
			Object_AddRef( pObject );
		}
		jeRam_Free( pszDisplayName ) ;
	}
	return (hItem == NULL) ? JE_FALSE : JE_TRUE ;
}// BrushCB




////////////////////////////////////////////////////////////////////////////////////////
//
//	CLists::SelectCB()
//
////////////////////////////////////////////////////////////////////////////////////////



jeBoolean CLists::SelectCB(Object *pObject, void *lParam)
{
	CTreeCtrlEx	*	pList= (CTreeCtrlEx	*)lParam ;
	char		*	pszDisplayName ;
	HTREEITEM		hClassItem ;
	HTREEITEM		hItem  = NULL;
	CString		cstr ;

	ASSERT( pObject );
	ASSERT( lParam );

	pszDisplayName = Object_GetNameAndTag( pObject ) ;
	if( pszDisplayName == NULL )
		return JE_FALSE ;
	if( Object_GetKind( pObject ) == KIND_CLASS )
	{ 
		hClassItem = pList->GetRootItem();
		while( hClassItem ) 
		{
			cstr = pList->GetItemText( hClassItem ) ;
			if( cstr.Compare( pszDisplayName ) == 0 )
			{
				hItem =  hClassItem;
				break;
			}

			hClassItem = pList->GetNextSiblingItem( hClassItem ) ;
		}
	}
	else
	{
		hClassItem = FindObjectKind( pList, pObject  );
		if( hClassItem == NULL )
			return( JE_FALSE );

		hItem = TreeViewIsInBranch( pList, hClassItem, pszDisplayName ) ;
	}
	ASSERT( hItem != NULL ) ;
	if( hItem != NULL )
		pList->SelectItemEx( hItem, true ) ;	// From derived TV

	jeRam_Free( pszDisplayName ) ;

	return (hItem == NULL) ? JE_FALSE : JE_TRUE ;
}// SelectCB


void CLists::OnSelchangedTvItems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM		hItem ;
	//HTREEITEM		hParentItem ;
	NM_TREEVIEW*	pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	Object	*		pObject ;
	CJweDoc	*		pDoc ;

	if( pNMTreeView->action != TVC_BYCTREECTRL )
		return ;

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc == NULL )
		return ;
	
	// I added TVC_BYTREECTRL to try to filter out the duplicate messages
	// I'm still not getting correct multi-sel messages
	
	hItem = pNMTreeView->itemOld.hItem ;
	if( hItem != NULL ) 
	{
		if( m_List.GetItemState( hItem, TVIS_SELECTED ) != TVIS_SELECTED )
			pDoc->DeselectAll( JE_FALSE );
	}

	hItem = pNMTreeView->itemNew.hItem ;
	if( hItem != NULL )
	{
		//hParentItem = m_List.GetParentItem( hItem );
		//if( hParentItem != NULL ) //This is not a root item
		//{
			pObject = (Object*)m_List.GetItemData( hItem ) ;
			if( pObject != NULL )
			{
				pDoc->SelectObject( pObject, LEVEL_SELECT ) ;
			}
		//}
	}
	

	*pResult = 0;
}// OnSelchangedTvItems


jeBoolean CLists::AddSelectionCB(Object *pObject, void *lParam)
{
	HTREEITEM		hClassItem = NULL ;
	HTREEITEM		hItem ;
	CTreeCtrlEx *  pList = (CTreeCtrlEx *)lParam ;
	char		*	pszDisplayName ;
	char		*	pszKindName;
	
	hClassItem = FindObjectKind( pList, pObject  );
	if( hClassItem == NULL )
	{
		pszKindName = Object_CreateKindName(  pObject  );
		if( pszKindName == NULL )
			return( FALSE );
		hClassItem  = AddObjectKind( pList, pszKindName, Object_GetKind( pObject ));
		jeRam_Free( pszKindName );
		if( hClassItem == NULL )
			return( FALSE );
	}
	pszDisplayName = Object_GetNameAndTag( pObject ) ;
	if( pszDisplayName == NULL )
		return JE_FALSE ;

	hItem = pList->InsertItem( pszDisplayName, hClassItem, TVI_SORT ) ;
	jeRam_Free( pszDisplayName ) ;

	if( hItem == NULL )
		return JE_FALSE ;

	pList->SetItemData( hItem, (DWORD)pObject ) ;
	Object_AddRef( pObject );

	return JE_TRUE ;
}// AddSelectionCB

HTREEITEM CLists::FindObjectItem( Object* pObject )
{
	HTREEITEM		hClassItem = NULL ;
	HTREEITEM		hItem ;
	Object *		pListObject;

	hClassItem = FindObjectKind( &m_List, pObject  );
	if( hClassItem == NULL )
		return( NULL );

	hItem = m_List.GetChildItem( hClassItem );

	while( hItem != NULL )
	{
		pListObject = (Object*)m_List.GetItemData( hItem );
		ASSERT( pListObject != NULL );
		if( pObject == pListObject )
		{
			return(hItem );
		}
		hItem = m_List.GetNextSiblingItem( hItem );
	}
	return( NULL );
}




void CLists::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	UpdateData( true );
	UpdateData( false );
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}

void CLists::DeRefAllObjects()
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
		hItem = m_List.GetNextSiblingItem(hItem );
	}
}
void CLists::OnDestroy() 
{
	
	DeRefAllObjects();
	CDialog::OnDestroy();
	
	
}


BOOL CLists::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if( wParam == 1 && lParam == 0 )
		return(JE_TRUE );
	return CDialog::OnCommand(wParam, lParam);
}

void CLists::OnKeydownTvItems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	((CMainFrame*)AfxGetMainWnd())->PostMessage( WM_KEYDOWN, pTVKeyDown->wVKey, 0 );
	*pResult = 0;
}
