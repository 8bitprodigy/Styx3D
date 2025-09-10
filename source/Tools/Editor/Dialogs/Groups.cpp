/****************************************************************************************/
/*  GROUPS.CPP                                                                          */
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
#include "Resource.h"
#include "Ram.h"
#include "util.h"

#include "Groups.h"
#include "AddModel.h"

// Krouer: import/export of prefab
#include "MaterialIdentList.h"

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

typedef struct tagGroupInfo
{
	Group*			pGroup;
	CTreeCtrlEx	*	pList ;
	HTREEITEM		hItemGroup ;
	CJweDoc*		pDoc;
} GroupInfo ;

/////////////////////////////////////////////////////////////////////////////
// CGroups dialog


CGroups::CGroups(CWnd* pParent /*=NULL*/)
	: CDialog(CGroups::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGroups)
	m_Lock = FALSE;
	//}}AFX_DATA_INIT
    m_lHiddenItemCount = 0;
}

CGroups::~CGroups()
{

}

void CGroups::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroups)
	DDX_Control(pDX, LIST_TV_ITEMS, m_List);
	DDX_Control(pDX, GRUP_CB_CURRENT, m_CBList);
	DDX_Check(pDX, GROUP_LOCK, m_Lock);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGroups, CDialog)
	//{{AFX_MSG_MAP(CGroups)
	ON_NOTIFY(TVN_SELCHANGED, LIST_TV_ITEMS, OnSelchangedTvItems)
	ON_BN_CLICKED(IDC_ADDGROUP, OnAddgroup)
	ON_CBN_SELCHANGE(GRUP_CB_CURRENT, OnSelchangeCbCurrent)
	ON_BN_CLICKED(GROUP_LOCK, OnLock)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_KEYDOWN, LIST_TV_ITEMS, OnKeydownTvItems)
    ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, LIST_TV_ITEMS, OnRclickTvItems)
	ON_COMMAND(ID_WORLDMODEL_EXPORTTOPREFAB, OnWorldGroupExporttoprefab)
	ON_COMMAND(ID_WORLDMODEL_IMPORTFROM, OnWorldGroupImportfrom)
	ON_COMMAND(ID_WORLDMODEL_CREATE, OnWorldGroupCreate)
	//}}AFX_MSG_MAP
    ON_COMMAND(ID_WORLDMODEL_INVERTSHOW, OnWorldmodelInvertshow)
    ON_COMMAND(ID_WORLDMODEL_SHOW, OnWorldmodelShow)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroups message handlers

BOOL CGroups::OnInitDialog() 
{
	CBitmap             bitmap;

	CDialog::OnInitDialog();
	
	PositionDialogUnderTabs( this ) ;
/*
	pImageList = new CImageList();
	pImageList->Create(32, 16, ILC_MASK, 6, 4);

		bitmap.LoadBitmap( IDB_ADDTOCURRENTGROUP );
		pImageList->Add(&bitmap, (COLORREF)0xFFFFFF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap( IDB_REMOVEFROMCURRENT );
		pImageList->Add(&bitmap, (COLORREF)0xFFFFFF);
		bitmap.DeleteObject();
*/
	m_ImageList.Create( IDR_GROUPS, 18, 8, RGB(255,0,255) ) ;
	m_List.SetImageList( &m_ImageList, TVSIL_NORMAL    );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGroups::OnSize(UINT nType, int cx, int cy) 
{
    CDialog::OnSize(nType, cx, cy);
    CWnd * pWnd = GetDlgItem(LIST_TV_ITEMS);
    if(pWnd)
        pWnd->MoveWindow(4,32,cx-8,cy-36);
    pWnd = GetDlgItem(IDC_ADDGROUP);
    if(pWnd)
        pWnd->MoveWindow(cx-68,4,64,24);
}

void CGroups::SetCurrentDocument(CJweDoc *pDoc)
{
	Group *pGroup;
	int		ComboItem;

	// Doc has changed, lists need complete rebuild
	ASSERT( pDoc != NULL ) ;

	Reset();
	GroupList_EnumGroups( pDoc->GetGroupList(), &m_List, CGroups::GroupListCB ) ;
	GroupList_EnumGroups( pDoc->GetGroupList(), &m_CBList, CGroups::GroupComboCB ) ;
	pGroup = pDoc->GetCurrentGroup();
	if( pGroup != NULL )
	{
		ComboItem = m_CBList.SelectString( 0, Group_GetName(pGroup) );
		UpdateData( true ) ;
		m_Lock= Group_IsLocked( pGroup );
		UpdateData( false );
	}
}// SetCurrentDocument

jeBoolean CGroups::AddObject( Object* pObject )
{
	HTREEITEM hItem;
	HTREEITEM hObjectItem;
	Group *pGroup;
	char  *	pszDisplayName ;
	
	ASSERT( pObject );

	hItem = m_List.GetRootItem();

	while( hItem != NULL )
	{
		pGroup = (Group*)m_List.GetItemData( hItem );
		ASSERT( pGroup != NULL );
		if( Object_GetGroup( pObject ) == pGroup )
		{
			pszDisplayName = Object_GetNameAndTag( pObject ) ;
			if( pszDisplayName == NULL )
				return( JE_FALSE );
			int imageIdx = 0;
			switch (Object_GetKind(pObject)) {
			case KIND_BRUSH: imageIdx = 5; break;
			case KIND_CAMERA: imageIdx = 6; break;
			case KIND_LIGHT: imageIdx = 7; break;
			default: break;
			}
			hObjectItem	= m_List.InsertItem( pszDisplayName, imageIdx, imageIdx, hItem, TVI_SORT );
			jeRam_Free( pszDisplayName );
			if( hObjectItem == NULL )
				return( JE_FALSE );
			m_List.SetItemData( hObjectItem, (DWORD)pObject ) ;
			Object_AddRef( pObject );

			return( JE_TRUE );
		}
		hItem = m_List.GetNextSiblingItem(hItem );
	}
	return( JE_FALSE );
}


void CGroups::RenameObject( Object *pObject )
{
	HTREEITEM hItem;
	char * pszDisplayName;
	
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

jeBoolean CGroups::GroupListCB( Group *pGroup, void *lParam)
{
	HTREEITEM			hItem = NULL ;
	CTreeCtrlEx		*	pList = (CTreeCtrlEx*)lParam ;
	const char			*	pszDisplayName ;
	GroupInfo			GroupInfoData;

	pszDisplayName = Group_GetName( pGroup ) ;
	if( pszDisplayName != NULL )
	{
		hItem = pList->InsertItem( pszDisplayName, TVI_ROOT, TVI_SORT ) ;

		if( hItem != NULL )
		{
			GroupInfoData.hItemGroup = hItem;
			GroupInfoData.pList = pList;
			pList->SetItemData( hItem, (DWORD)pGroup ) ;
			if( ObjectList_EnumObjects( Group_GetObjectList( pGroup ), &GroupInfoData, CGroups::ObjectCB ) == JE_FALSE )
				hItem = NULL ;
		}
	}
	return ( hItem == NULL ) ? JE_FALSE : JE_TRUE ;
}// GroupCB

jeBoolean CGroups::GroupComboCB( Group *pGroup, void *lParam)
{
	int					nIndex = CB_ERR ;
	CComboBox			*pCBList = (CComboBox*)lParam ;
	const char			*	pszDisplayName ;
	

	pszDisplayName = Group_GetName( pGroup ) ;
	if( pszDisplayName != NULL )
	{
		pCBList->AddString( pszDisplayName );
		nIndex = pCBList->FindString( 0, pszDisplayName );
		if( nIndex != CB_ERR )
			pCBList->SetItemDataPtr( nIndex, pGroup );

	}
	return ( nIndex != CB_ERR ) ? JE_FALSE : JE_TRUE ;
}// GroupCB

jeBoolean CGroups::ObjectCB( Object *pObject, void *lParam)
{
	HTREEITEM			hItem = NULL ;
	GroupInfo		*	pGroupInfo = (GroupInfo*)lParam ;
	char			*	pszDisplayName ;

	pszDisplayName = Object_GetNameAndTag( pObject ) ;
	if( pszDisplayName != NULL )
	{
		int imageIdx = 0;
		switch (Object_GetKind(pObject)) {
		case KIND_BRUSH: imageIdx = 5; break;
		case KIND_CAMERA: imageIdx = 6; break;
		case KIND_LIGHT: imageIdx = 7; break;
		default: break;
		}
		hItem = pGroupInfo->pList->InsertItem( pszDisplayName, imageIdx, imageIdx, pGroupInfo->hItemGroup, TVI_SORT ) ;

		if( hItem != NULL )
		{
			pGroupInfo->pList->SetItemData( hItem, (DWORD)pObject ) ;
			Object_AddRef( pObject );
		}
		jeRam_Free( pszDisplayName ) ;
	}
	return ( hItem == NULL ) ? JE_FALSE : JE_TRUE ;
}// ObjectCB

jeBoolean CGroups::AddSelectionCB(Object *pObject, void *lParam)
{
	HTREEITEM hItem;
	HTREEITEM hObjectItem;
	Group *pGroup;
	char  *	pszDisplayName ;
	CTreeCtrlEx	*pList = (CTreeCtrlEx	*)lParam;
	
	ASSERT( pObject );
	ASSERT( pList );

	hItem = pList->GetRootItem();

	while( hItem != NULL )
	{
		pGroup = (Group*)pList->GetItemData( hItem );
		ASSERT( pGroup != NULL );
		if( Object_GetGroup( pObject ) == pGroup )
		{
			pszDisplayName = Object_GetNameAndTag( pObject ) ;
			if( pszDisplayName == NULL )
				return( JE_FALSE );
			int imageIdx = 0;
			switch (Object_GetKind(pObject)) {
			case KIND_BRUSH: imageIdx = 5; break;
			case KIND_CAMERA: imageIdx = 6; break;
			case KIND_LIGHT: imageIdx = 7; break;
			default: break;
			}
			hObjectItem	= pList->InsertItem( pszDisplayName, imageIdx, imageIdx, hItem, TVI_SORT );
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

HTREEITEM CGroups::GetObjectItem( CTreeCtrl *pList, Object *pObject )
{
	HTREEITEM hGroupItem;
	HTREEITEM hObjectItem;
	Object *pListObject;
	Group  *pGroup;
	
	ASSERT( pObject );
	ASSERT( pList );

	hGroupItem = pList->GetRootItem();

	while( hGroupItem != NULL )
	{
		pGroup = (Group*)pList->GetItemData( hGroupItem );
		ASSERT( hGroupItem != NULL );
		if( pGroup == Object_GetGroup( pObject ) )
		{
			hObjectItem = pList->GetChildItem( hGroupItem );
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
		hGroupItem = pList->GetNextSiblingItem(hGroupItem );
	}
	return( 0 );
}


void CGroups::AddSelection(CJweDoc *pDoc)
{
	pDoc->EnumSelected( &m_List, AddSelectionCB ) ;
}// AddSelection

void CGroups::RemoveDeleted( )
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

void CGroups::ChangeGroups( HTREEITEM hItem )
{
	HTREEITEM	hParentItem;
	Group	* pGroup;
	Group	* pOldGroup;
	Object  * pObject;

	hParentItem = m_List.GetParentItem( hItem );
	ASSERT( hParentItem );

	pObject = (Object*)m_List.GetItemData( hItem );
	ASSERT( pObject );

	pGroup = (Group*)m_List.GetItemData( hParentItem );
	ASSERT( pGroup );

	pOldGroup = Object_GetGroup( pObject );
	ASSERT( pOldGroup );

	Group_RemoveObject( pOldGroup, pObject );
	Group_AddObject( pGroup, pObject );
	Object_SetGroup( pObject, pGroup );
}

void CGroups::SelectGroup( HTREEITEM hGroupItem, jeBoolean bSelect )
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

void CGroups::OnSelchangedTvItems(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	HTREEITEM		hItem ;
	NM_TREEVIEW*	pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	Object	*		pObject ;
	CJweDoc	*		pDoc ;
	int				State;

    // Krouer: the tree control is know able to send multiple messages when draging multiple items
	if (pNMTreeView->action == TVC_DRAG_END )
	{
		ChangeGroups( pNMTreeView->itemNew.hItem );
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
		if( pObject == NULL )
			return;
		if( Object_GetKind(pObject) == KIND_GROUP )
		{
			State = m_List.GetItemState( hItem, TVIS_SELECTED );
			if(  State & TVIS_SELECTED )
			{
				SelectGroup( hItem, JE_TRUE );
			}
			else
			{
				SelectGroup( hItem, JE_FALSE );
			}
		}
		else
		{
			pDoc->SelectObject( pObject, LEVEL_TOGGLE ) ;
		}
	}
/*	
	hItem = pNMTreeView->itemOld.hItem ;
	if( hItem != NULL ) 
	{
		pObject = (Object*)m_List.GetItemData( hItem ) ;
		if( pObject != NULL && Object_GetKind(pObject) != KIND_GROUP )
		{
			pDoc->SelectObject( pObject, LEVEL_DESELECT ) ;
		}
	}
*/
	*pResult = 0;
}

jeBoolean CGroups::SelectCB(Object *pObject, void *lParam)
{
	HTREEITEM hObjectItem;
	CTreeCtrlEx	*pList = (CTreeCtrlEx	*)lParam;
	
	ASSERT( pObject );
	ASSERT( pList );

	hObjectItem = GetObjectItem( pList, pObject );
	if( hObjectItem )
	{
		pList->SelectItemEx( hObjectItem );
		return( JE_TRUE );
	}
	return( JE_FALSE );
}
// SELECTION has changed


void CGroups::Update(CJweDoc *pDoc)
{
	LEVEL_SEL	SelType ;
	m_List.SelectItem( NULL );

	SelType = pDoc->GetSelType( ) ;
	if( (SelType & LEVEL_SELTEMPLATE) == 0 )
	{
		pDoc->EnumSelected(&m_List, SelectCB ) ;	
	}
}// Update

void CGroups::Reset()
{
	DeRefAllObjects();
	m_List.DeleteAllItems( );
	m_CBList.ResetContent();
    Sleep(100);
}

void CGroups::OnAddgroup() 
{
	HTREEITEM		hItem ;
	CAddModel		AddModelDialog ;
	Group *			pGroup;
	CJweDoc	*		pDoc ;

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	AddModelDialog.m_nTitleID = IDS_NEWGROUP ;
	if( AddModelDialog.DoModal( ) == IDOK )
	{
		TrimString( AddModelDialog.m_csName ) ;
		if( AddModelDialog.m_csName.IsEmpty() )
		{
			AfxMessageBox( IDS_MUSTSUPPLYNAME, MB_OK, 0 ) ;
			return ;		
		}

		pGroup = pDoc->AddGroup( AddModelDialog.m_csName  );
		hItem = m_List.InsertItem( AddModelDialog.m_csName, TVI_ROOT, TVI_SORT ) ;

		if( hItem != NULL )
		{
			m_List.SetItemData( hItem, (DWORD)pGroup ) ;
		}

		GroupComboCB( pGroup, &m_CBList );
		m_CBList.SelectString( 0, Group_GetName( pGroup  ) );
		pDoc->SetCurrentGroup( pGroup );
		UpdateData( true ) ;
		m_Lock= Group_IsLocked( pGroup );
		UpdateData( false );
	}
	
}

void CGroups::OnSelchangeCbCurrent() 
{
	int nIndex;
	Group * pGroup;
	CJweDoc	*		pDoc ;

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	nIndex = m_CBList.GetCurSel();
	pGroup = (Group*)m_CBList.GetItemData( nIndex );
	pDoc->SetCurrentGroup( pGroup );	
	UpdateData( true ) ;
	m_Lock= Group_IsLocked( pGroup );
	UpdateData( false );
}

void CGroups::OnLock() 
{
	CJweDoc	*		pDoc ;
	Group * pGroup;

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	pGroup = pDoc->GetCurrentGroup();
	UpdateData( true );
	Group_SetLocked( pGroup, m_Lock );
	UpdateData( false );
	
}

void CGroups::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	UpdateData( true );
	UpdateData( false );
	CDialog::OnShowWindow(bShow, nStatus);
}


void CGroups::DeRefAllObjects()
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

void CGroups::OnDestroy() 
{
	DeRefAllObjects();
	CDialog::OnDestroy();
}


void CGroups::OnKeydownTvItems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;

	((CMainFrame*)AfxGetMainWnd())->PostMessage( WM_KEYDOWN, pTVKeyDown->wVKey, 0 );

	*pResult = 0;
}

void CGroups::OnRclickTvItems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CMenu ContextMenu;
	CMenu *SubMenu;

	NMTREEVIEW* pNMTreeItem = (NMTREEVIEW*)pNMHDR;

	CPoint point;
	GetCursorPos(&point);
	CPoint pointClient = point;
	m_List.ScreenToClient(&pointClient);

	ContextMenu.LoadMenu( IDR_WORLDMODEL );
	SubMenu = ContextMenu.GetSubMenu( 0 );

	UINT uFlags;
	HTREEITEM hItem = m_List.HitTest(pointClient, &uFlags);

	CString szName = m_List.GetItemText(hItem);

	if (hItem) {
		Object* pObject = (Object*) m_List.GetItemData(hItem);
		OBJECT_KIND ObjKind = Object_GetKind(pObject);
		if (ObjKind != KIND_BRUSH && szName != "Default")
			SubMenu->EnableMenuItem(ID_WORLDMODEL_EXPORTTOPREFAB, MF_BYCOMMAND | MF_ENABLED);
		else
			SubMenu->EnableMenuItem(ID_WORLDMODEL_EXPORTTOPREFAB, MF_BYCOMMAND | MF_GRAYED);

        if (ObjKind == KIND_GROUP && szName != "Default") {
            Group* pGroup = (Group*) m_List.GetItemData(hItem);
            jeBoolean visFlag = Group_IsVisible(pGroup);
            SubMenu->ModifyMenu(ID_WORLDMODEL_SHOW, MF_BYCOMMAND | MF_STRING, ID_WORLDMODEL_SHOW, visFlag?"&Hide":"&Show");
            SubMenu->ModifyMenu(ID_WORLDMODEL_INVERTSHOW, MF_BYCOMMAND | MF_STRING, ID_WORLDMODEL_INVERTSHOW, visFlag?"In&vert Hide":"In&vert Show");
            SubMenu->EnableMenuItem(ID_WORLDMODEL_INVERTSHOW, MF_BYCOMMAND | MF_GRAYED);  // Krouer: to active when ready
            SubMenu->EnableMenuItem(ID_WORLDMODEL_SHOW, MF_BYCOMMAND | MF_ENABLED);
        } else
        if (ObjKind == KIND_BRUSH) {
            Brush* pBrush = (Brush*) pObject;
            jeBoolean visFlag = Brush_IsVisible(pBrush);
            SubMenu->ModifyMenu(ID_WORLDMODEL_SHOW, MF_BYCOMMAND | MF_STRING, ID_WORLDMODEL_SHOW, visFlag?"&Hide":"&Show");
            SubMenu->ModifyMenu(ID_WORLDMODEL_INVERTSHOW, MF_BYCOMMAND | MF_STRING, ID_WORLDMODEL_INVERTSHOW, visFlag?"In&vert Hide":"In&vert Show");
            SubMenu->EnableMenuItem(ID_WORLDMODEL_INVERTSHOW, MF_BYCOMMAND | MF_GRAYED);  // Krouer: to active when ready
            SubMenu->EnableMenuItem(ID_WORLDMODEL_SHOW, MF_BYCOMMAND | MF_ENABLED);
        } else {
            SubMenu->EnableMenuItem(ID_WORLDMODEL_INVERTSHOW, MF_BYCOMMAND | MF_GRAYED);
            SubMenu->EnableMenuItem(ID_WORLDMODEL_SHOW, MF_BYCOMMAND | MF_GRAYED);
        }
	}

	SubMenu->TrackPopupMenu( TPM_LEFTALIGN, point.x, point.y, this, NULL );
	
	*pResult = 0;
}

#define PREFAB_SIGNATURE	'FP3J'	// J3PF : prefab signature
#define PREFAB_VERSION		1

void CGroups::OnWorldGroupExporttoprefab() 
{
	jeVFile* pFS;
	jeVFile* pFile;
	CMainFrame*		pMainFrm;
	CJweDoc	*		pDoc ;

	uint32 Signature = PREFAB_SIGNATURE;
	uint32 Version = PREFAB_VERSION;

	// Prompt a CFileDialog with default dir : /prefab, def ext *.j3p
	static char* szFilter = "Jet Prefab Mesh (*.j3p)|*.j3p||";

	// copy the Group
    HTREEITEM hItem = m_List.GetFirstSelectedItem();
	//HTREEITEM hItem = m_List.GetSelectedItem();
	if (!m_List.ItemHasChildren(hItem)) {
		hItem = m_List.GetParentItem(hItem);
	}
	Group* pGroup = (Group*) m_List.GetItemData(hItem);

	ObjectList* pObjectList = Group_GetObjectList(pGroup);
	if (ObjectList_GetNumItems(pObjectList) == 0) {
		return;
	}

	pMainFrm = (CMainFrame*)AfxGetMainWnd();
	pDoc = pMainFrm->GetCurrentDocument();
	// rebuild every thing before adding new geometry
	Level_SetBSPBuildOptions(pDoc->GetLevel(), BSP_OPTIONS_CSG_BRUSHES | BSP_OPTIONS_MAKE_VIS_AREAS, Logic_Smart, 7);
	Level_RebuildAll(pDoc->GetLevel(), BSP_OPTIONS_CSG_BRUSHES | BSP_OPTIONS_MAKE_VIS_AREAS, Logic_Smart, 7);

	CFileDialog saveDlg(FALSE, "j3p", NULL, OFN_OVERWRITEPROMPT, szFilter);

	// use the current directory
	char szPath[2000];
	char szDefPath[2000];
	Util_GetAppPath( szPath, 2000 );
	strcpy(szDefPath, szPath);

	// append userbrush directory
	char sPath[2000];
	Settings_GetPath_UBrush(sPath, 2000);
	if (sPath[0]=='.')
		strcat(szPath, sPath + 2);
	else
		strcat(szPath, sPath);

	saveDlg.m_ofn.lpstrInitialDir = szPath;

	if (saveDlg.DoModal() == IDOK) {
		// open a file system
		pFS = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_VIRTUAL, saveDlg.GetPathName(), NULL, JE_VFILE_OPEN_CREATE|JE_VFILE_OPEN_DIRECTORY);

		// create the header
		pFile = jeVFile_Open(pFS, "Version", JE_VFILE_OPEN_CREATE );
		jeVFile_Write(pFile, &Signature, sizeof(Signature));
		jeVFile_Write(pFile, &Version, sizeof(Version));
		jeVFile_Close(pFile);

		jePtrMgr* pPtrMgr = jePtrMgr_Create();

		// create the Group part
		pFile = jeVFile_Open(pFS, "Group", JE_VFILE_OPEN_CREATE );

		jeMaterial_Array* pMaterial = jeWorld_GetMaterialArray(Level_GetjeWorld((const Level*)pDoc->GetLevel()));

		// write Group info into file
		Group_WriteToPrefabFile(pGroup, pFile, pPtrMgr, pMaterial);

		jeVFile_Close(pFile);

		jePtrMgr_Destroy(&pPtrMgr);

		jeVFile_Close(pFS);
	}
	SetCurrentDirectory(szDefPath);
}

jeBoolean CGroups::GroupNameCB( Group *pGroup, void *lParam)
{
	char*	pszName = (char*) lParam;
	char*	pszDisplayName ;
	pszDisplayName = Object_GetNameAndTag( (Object*)pGroup ) ;
	return  strcmp(pszName, pszDisplayName) == 0;
}

jeBoolean CGroups::AddObjectCB( Object *pObject, void *lParam)
{
	HTREEITEM			hItem = NULL ;
	char			*	pszDisplayName ;
	GroupInfo* pInfoGroup = (GroupInfo*) lParam;

	pszDisplayName = Object_GetNameAndTag( pObject ) ;
	if( pszDisplayName != NULL )
	{
		int imageIdx = 0;
		switch (Object_GetKind(pObject)) {
		case KIND_BRUSH: imageIdx = 5; break;
		case KIND_CAMERA: imageIdx = 6; break;
		case KIND_LIGHT: imageIdx = 7; break;
		default: break;
		}
		hItem = pInfoGroup->pList->InsertItem( pszDisplayName, imageIdx, imageIdx, pInfoGroup->hItemGroup, TVI_SORT ) ;

		if( hItem != NULL )
		{
			pInfoGroup->pList->SetItemData( hItem, (DWORD)pObject ) ;
			Object_AddRef( pObject );

			Group_RemoveObject(Object_GetGroup(pObject), pObject);
			Object_SetGroup(pObject, pInfoGroup->pGroup);

			Level* pLevel = pInfoGroup->pDoc->GetLevel();

			if (Object_GetKind(pObject) == KIND_BRUSH) {
				jeBoolean bLightUpdate;
				jeBoolean bBrushUpdate;
				jeBoolean bResult = JE_FALSE;

				Brush* pBrush = (Brush*) pObject;

				Model_AddBrush( Level_GetCurModel(pLevel), pBrush );

				if( Level_GetBrushUpdate(pLevel) == LEVEL_UPDATE_CHANGE )
				{
					bBrushUpdate = JE_TRUE;
					bLightUpdate = Level_GetBrushLighting(pLevel);
				}
				else
				{
					bBrushUpdate = JE_FALSE;
					bLightUpdate = JE_FALSE;
					Object_Dirty( pObject );
				}

				bResult = Model_AddBrushWorld( Level_GetCurModel(pLevel), pBrush, bBrushUpdate, bLightUpdate);

				bResult = Brush_AttachWorld( pBrush, Level_GetjeWorld(pLevel) );

			}

			Group_AddObject( pInfoGroup->pGroup, pObject );
			Object_SetInLevel( pObject, JE_TRUE );

			CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
			pMainFrm->AddObjectEx(pObject, MAINFRM_ADDOBJECT_LIST|MAINFRM_ADDOBJECT_MODEL) ;
		
		}
		jeRam_Free( pszDisplayName ) ;
	}
	return ( hItem == NULL ) ? JE_FALSE : JE_TRUE ;
}

/*! @brief Add group Material to the current World material list
    @param pMatIdent The group material list
    @param lParam The group information
*/
jeBoolean CGroups_LoadTextureCB( MaterialIdent *pMatIdent, void *lParam)
{
    long matIdx = 0;
    jeMaterial* pMaterial = NULL;
	CMainFrame* pMainFrm = (CMainFrame*) AfxGetMainWnd();

	GroupInfo* pInfoGroup = (GroupInfo*) lParam;
	Level* pLevel = pInfoGroup->pDoc->GetLevel();

	jeMaterial_Array* pMatArray = jeWorld_GetMaterialArray(Level_GetjeWorld(pLevel));

    while (pMaterial = jeMaterial_ArrayGetNextMaterial(pMatArray, pMaterial)) {
        if (stricmp(jeMaterial_GetName(pMaterial), pMatIdent->MaterialName) == 0) {
            pMatIdent->WorldMatIdx = matIdx;
            break;
        }
        matIdx++;
    }

    if (pMaterial == NULL) {
    	pMatIdent->WorldMatIdx = jeMaterial_ArrayCreateMaterial(pMatArray, pMatIdent->MaterialName);

#ifdef _USE_BITMAPS
	    jeBitmap* pBitmap = Level_GetMaterialBitmapByName(pLevel, pMatIdent->MaterialName);
	    if (pBitmap == NULL) {
		    pBitmap = Level_GetMaterialBitmapByName(pLevel, "jet3d");
		    strcpy(pMatIdent->BitmapName, "jet3d");
	    }
    	
	    if (pBitmap) {
		    jeMaterial_ArraySetMaterialBitmap(pMatArray, pMatIdent->WorldMatIdx, pBitmap, pMatIdent->BitmapName);
	    }
#else
	    jeMaterialSpec* pMatSpec = Level_GetMaterialSpecByName(pLevel, pMatIdent->MaterialName);
	    if (pMatSpec == NULL) {
		    pMatSpec = Level_GetMaterialSpecByName(pLevel, "jet3d");
		    strcpy(pMatIdent->BitmapName, "jet3d");
	    }
    	
	    if (pMatSpec) {
		    jeMaterial_ArraySetMaterialSpec(pMatArray, pMatIdent->WorldMatIdx, pMatSpec, pMatIdent->BitmapName);
	    }
#endif
    }

	return JE_TRUE;
}

/*! @brief Change the FaceInfo material index to the new one when the material was added
    @param pObject The brush object to patch
    @param lParam The material list
    @see CGroups::OnWorldGroupImportfrom()
*/
jeBoolean CGroups_PatchFaceInfoCB( Object *pObject, void *lParam)
{
	MaterialIdentList* pList = (MaterialIdentList*) lParam;
	
	if (KIND_BRUSH == Object_GetKind(pObject)) {
		Brush* pBrush = (Brush*) pObject;
		jeBrush* pJeBrush = Brush_GetjeBrush(pBrush);
		jeBrush_Face* pFace = jeBrush_GetNextFace(pJeBrush, NULL);
		while (pFace) {
			jeFaceInfo faceinfo;
			jeBrush_FaceGetFaceInfo(pFace, &faceinfo);
			faceinfo.MaterialIndex = MaterialIdentList_GetWorldIndexByFileIndex(pList, faceinfo.MaterialIndex);
			jeBrush_FaceSetFaceInfo(pFace, &faceinfo);
			pFace = jeBrush_GetNextFace(pJeBrush, pFace);
		}
	}
	
	return JE_TRUE;
}

/*! @brief Import a prefab from a file
*/
void CGroups::OnWorldGroupImportfrom() 
{
	jeVFile* pFS;
	jeVFile* pFile;

	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd();
	CJweDoc	*	pDoc = pMainFrm->GetCurrentDocument();

	uint32 Signature = PREFAB_SIGNATURE;
	uint32 Version = PREFAB_VERSION;

	// Prompt a CFileDialog with default dir : /prefab, def ext *.j3p
	static char* szFilter = "Jet Prefab Mesh (*.j3p)|*.j3p|3DS Meshes (*.3ds)|*.3ds|OBJ Meshes (*.obj)|*.obj||";

	CFileDialog openDlg(TRUE, "j3p", NULL, OFN_FILEMUSTEXIST, szFilter);

	// get current directory
	char szPath[2000];
	char szDefPath[2000];
	Util_GetAppPath( szPath, 2000 );
	strcpy(szDefPath, szPath);

	// append userbrush directory
	char sPath[2000];
	Settings_GetPath_UBrush(sPath, 2000);
	if (sPath[0]=='.')
		strcat(szPath, sPath + 2);
	else
		strcat(szPath, sPath);

	openDlg.m_ofn.lpstrInitialDir = szPath;

	Group* pGroup = NULL;

	if (openDlg.DoModal() == IDOK) {
		CString szExt = openDlg.GetFileExt();
		CString szName = openDlg.GetFileTitle();
		if (szExt == "3ds") {
			// goal : convert from 3DS to Group
		}
		if (szExt == "obj") {
			// goal : convert from OBJ to Group
		}
		if (szExt == "j3p") {
			// goal : read prefab to group
			// open a file system
			pFS = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_VIRTUAL, openDlg.GetPathName(), NULL, JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY);

			// read the header
			pFile = jeVFile_Open(pFS, "Version", JE_VFILE_OPEN_READONLY );
			jeVFile_Read(pFile, &Signature, sizeof(Signature));
			jeVFile_Read(pFile, &Version, sizeof(Version));
			jeVFile_Close(pFile);

			if (Signature != PREFAB_SIGNATURE || Version > PREFAB_VERSION) {
				jeVFile_Close(pFS);
				AfxMessageBox("Import of prefab failed");
				return;
			}

			// create a local Pointer Manager
			jePtrMgr* pPtrMgr = jePtrMgr_Create();
			MaterialIdentList* pMatList = MaterialIdentList_Create();

			// open the Group part
			pFile = jeVFile_Open(pFS, "Group", JE_VFILE_OPEN_READONLY );

			// create a Group from the file
			pGroup = Group_CreateFromPrefabFile(pFile, pPtrMgr, pMatList);

			jeVFile_Close(pFile);

			jePtrMgr_Destroy(&pPtrMgr);

			jeVFile_Close(pFS);

			// Add Material of imported Level into world
			GroupInfo GroupInfoData;
			GroupInfoData.pGroup = pGroup;
			GroupInfoData.pDoc =  pDoc;
			GroupInfoData.hItemGroup = NULL;
			GroupInfoData.pList = NULL;
			MaterialIdentList_EnumMaterialIdents(pMatList, &GroupInfoData, CGroups_LoadTextureCB);

			ObjectList_EnumObjects( Group_GetObjectList(pGroup), pMatList, CGroups_PatchFaceInfoCB);

			MaterialIdentList_Destroy(&pMatList);
		}
		if (pGroup) {
			Group* pAddGroup = NULL;
			jeExtBox		WorldBounds;
			CAddModel		AddModelDialog;

			// Add a group into the Document
			HTREEITEM hItem;
			AddModelDialog.m_nTitleID = IDS_NEWGROUP;
			// Prompt for a new name
			if( AddModelDialog.DoModal( ) == IDOK)
			{
				TrimString(AddModelDialog.m_csName);
				if( AddModelDialog.m_csName.IsEmpty())
				{
					AfxMessageBox(IDS_MUSTSUPPLYNAME, MB_OK, 0);
					return ;		
				}

				pAddGroup = pDoc->AddGroup(AddModelDialog.m_csName);

				// Reassign every brush created in this group
				if (pAddGroup) {
					// rebuild every thing before adding new geometry
					Level_SetBSPBuildOptions(pDoc->GetLevel(), BSP_OPTIONS_CSG_BRUSHES | BSP_OPTIONS_MAKE_VIS_AREAS, Logic_Smart, 7);
					Level_RebuildAll(pDoc->GetLevel(), BSP_OPTIONS_CSG_BRUSHES | BSP_OPTIONS_MAKE_VIS_AREAS, Logic_Smart, 7);

					hItem = m_List.InsertItem(AddModelDialog.m_csName, TVI_ROOT, TVI_SORT);
					if( hItem != NULL ) {
						GroupInfo GroupInfoData;
						m_List.SetItemData( hItem, (DWORD)pAddGroup ) ;
						GroupInfoData.hItemGroup = hItem;
						GroupInfoData.pList = &m_List;
						GroupInfoData.pGroup = pAddGroup;
						GroupInfoData.pDoc =  pDoc;

						ObjectList_EnumObjects( Group_GetObjectList(pGroup), &GroupInfoData, CGroups::AddObjectCB);
					}
					// Update all views
					Select_DeselectAll( pDoc->GetLevel(), &WorldBounds );
					pDoc->UpdateAllViews( NULL, DOC_HINT_ORTHO, (CObject*)&WorldBounds);

					// rebuild after new gemotry adds -> make level consistency
					Level_SetBSPBuildOptions(pDoc->GetLevel(), BSP_OPTIONS_CSG_BRUSHES | BSP_OPTIONS_MAKE_VIS_AREAS, Logic_Smart, 7);
					Level_RebuildAll(pDoc->GetLevel(), BSP_OPTIONS_CSG_BRUSHES | BSP_OPTIONS_MAKE_VIS_AREAS, Logic_Smart, 7);
				}
			}

			// Destroy the temporary group
			Group_Destroy(&pGroup);
			pGroup = NULL;
		}
	}

	SetCurrentDirectory(szDefPath);
}

void CGroups::OnWorldGroupCreate() 
{
	OnAddgroup();
}

void CGroups::OnWorldmodelInvertshow()
{
	CMainFrame*		pMainFrm = (CMainFrame*) AfxGetMainWnd();
	CJweDoc	*		pDoc = pMainFrm->GetCurrentDocument();

    HTREEITEM hItem = m_List.GetRootItem();
    HTREEITEM hSelItem = m_List.GetFirstSelectedItem();
	Object* pObject = (Object*) m_List.GetItemData(hItem);
	OBJECT_KIND ObjKind = Object_GetKind(pObject);
}

void CGroups::OnWorldmodelShow()
{
	CMainFrame*		pMainFrm = (CMainFrame*) AfxGetMainWnd();
	CJweDoc	*		pDoc = pMainFrm->GetCurrentDocument();

    HTREEITEM hItem = m_List.GetFirstSelectedItem();
    if (hItem == NULL)
        return;

	Object* pObject = (Object*) m_List.GetItemData(hItem);
	OBJECT_KIND ObjKind = Object_GetKind(pObject);
    if (ObjKind == KIND_GROUP) {
        Group* pGroup = (Group*) pObject;
        jeBoolean visFlag = !Group_IsVisible(pGroup);
        Group_Show(pGroup, visFlag);
        HTREEITEM hChild = m_List.GetChildItem(hItem);
        while (hChild) {
            pObject = (Object*) m_List.GetItemData(hChild);
        	ObjKind = Object_GetKind(pObject);
            if (ObjKind == KIND_BRUSH) {
                Brush* pBrush = (Brush*) pObject;
                jeBoolean visFlag = !Brush_IsVisible(pBrush);
                m_lHiddenItemCount += visFlag?-1:1;
                Brush_Show(pBrush, visFlag);
            }
            hChild = m_List.GetNextSiblingItem(hChild);
        }
        pDoc->UpdateAll();
    } else
    if (ObjKind == KIND_BRUSH) {
        Brush* pBrush = (Brush*) pObject;
        jeBoolean visFlag = !Brush_IsVisible(pBrush);
        m_lHiddenItemCount += visFlag?-1:1;
        Brush_Show(pBrush, visFlag);
        pDoc->UpdateAll();
    }
}

void CGroups::ShowAllGroups()
{
    OBJECT_KIND ObjKind;
    Object* pObject;
    HTREEITEM hItem = m_List.GetRootItem();

	CMainFrame*		pMainFrm = (CMainFrame*) AfxGetMainWnd();
	CJweDoc	*		pDoc = pMainFrm->GetCurrentDocument();

    while (hItem) {
	    pObject = (Object*) m_List.GetItemData(hItem);
	    ObjKind = Object_GetKind(pObject);
        if (ObjKind == KIND_GROUP) {
            Group* pGroup = (Group*) pObject;
            if (!Group_IsVisible(pGroup)) {
                Group_Show(pGroup, JE_TRUE);
            }
            HTREEITEM hChild = m_List.GetChildItem(hItem);
            while (hChild) {
                pObject = (Object*) m_List.GetItemData(hChild);
        	    ObjKind = Object_GetKind(pObject);
                if (ObjKind == KIND_BRUSH) {
                    Brush* pBrush = (Brush*) pObject;
                    Brush_Show(pBrush, JE_TRUE);
                }
                hChild = m_List.GetNextSiblingItem(hChild);
            }
        }

        hItem = m_List.GetNextSiblingItem(hItem);
    }
    m_lHiddenItemCount = 0;
    pDoc->UpdateAll();
}

bool CGroups::IsCurrentSelectionHidable()
{
    HTREEITEM hItem = m_List.GetFirstSelectedItem();
    if (hItem) {
	    Object* pObject = (Object*) m_List.GetItemData(hItem);
	    OBJECT_KIND ObjKind = Object_GetKind(pObject);
        if (ObjKind == KIND_GROUP) {
            Group* pGroup = (Group*) pObject;
            return Group_IsVisible(pGroup);
        }
        if (ObjKind == KIND_BRUSH) {
            Brush* pBrush = (Brush*) pObject;
            return Brush_IsVisible(pBrush);
        }
    }
    return false;
}

bool CGroups::IsCurrentSelectionShowable()
{
    HTREEITEM hItem = m_List.GetFirstSelectedItem();
    if (hItem) {
	    Object* pObject = (Object*) m_List.GetItemData(hItem);
	    OBJECT_KIND ObjKind = Object_GetKind(pObject);
        if (ObjKind == KIND_GROUP) {
            Group* pGroup = (Group*) pObject;
            return !Group_IsVisible(pGroup);
        }
        if (ObjKind == KIND_BRUSH) {
            Brush* pBrush = (Brush*) pObject;
            return !Brush_IsVisible(pBrush);
        }
    }
    return false;
}
