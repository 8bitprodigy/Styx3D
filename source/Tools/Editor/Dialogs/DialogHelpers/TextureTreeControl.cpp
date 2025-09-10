/****************************************************************************************/
/*  TextureTreeControl.cpp                                                              */
/*                                                                                      */
/*  Author: Tom Morris                                                                  */
/*  Description:                                                                        */
/*  Date: Feb. 2005                                                                     */
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
#include "TextureTreeControl.h"
#include "TexturesDlg.h"
#include ".\texturetreecontrol.h"


// CTextureTreeControl

IMPLEMENT_DYNAMIC(CTextureTreeControl, CTreeCtrl)
CTextureTreeControl::CTextureTreeControl()
{
	m_pEdit = NULL;
}

CTextureTreeControl::~CTextureTreeControl()
{
}


BEGIN_MESSAGE_MAP(CTextureTreeControl, CTreeCtrl)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_DROPACTION, OnDropAction)
	ON_WM_SHOWWINDOW()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTvnSelchanged)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnTvnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnTvnEndlabeledit)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
END_MESSAGE_MAP()



// CTextureTreeControl message handlers


int CTextureTreeControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


HTREEITEM	CTextureTreeControl::GetDropItem()
{
	return m_htDropItem;
}


LRESULT CTextureTreeControl::OnDropAction(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	CString		strNULL = _T("");
	CString		strTest;
	CPoint		ptCursor;
	GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);
	m_htDropItem = NULL;
	HTREEITEM	htParent = NULL;	

	m_htDropItem = 	HitTest(ptCursor);
	m_htRoot = GetRootItem();
	htParent = GetParentItem(m_htDropItem);

	if (m_htDropItem != m_htRoot)
	{
		if (htParent != m_htRoot)
		{
			m_htDropItem = htParent;
		}

		SelectDropTarget(m_htDropItem);

		strTest = m_DropTarget.GetstrDroppedFileName();
		if (strTest != strNULL)
		{
			if (NotAlreadyInGroup(m_htDropItem, strTest))
			{
				InsertItem(strTest, m_htDropItem);
				if (m_hWnd)
					if (IsWindowVisible())
						UpdateData(FALSE);
			}
		}
		Expand(m_htDropItem, TVE_EXPAND);
	}
	SortChildren(m_htDropItem);
	SelectDropTarget(NULL);
	return 0;	
}


bool	CTextureTreeControl::NotAlreadyInGroup(HTREEITEM htParent, CString	strDroppedItem)
{
	HTREEITEM	htChild = NULL;
	htChild = GetNextItem(htParent, TVGN_CHILD);
		while (htChild)
		{
			if (GetItemText(htChild) == strDroppedItem)
				return false;
			
			htChild = GetNextItem(htChild, TVGN_NEXT);
		}
	return true;
}


void CTextureTreeControl::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_htDropItem = NULL;
	m_htDropItem = 	HitTest(point);

	CTreeCtrl::OnMouseMove(nFlags, point);
}


void CTextureTreeControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CTreeCtrl::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		m_DropTarget.Revoke();

		if (GetSafeHwnd())
		{
			BOOL br = m_DropTarget.Register(this);
			// Remove this if you use br
			UNREFERENCED_PARAMETER(br);
		}
	}
	// TODO: Add your message handler code here
}


void	CTextureTreeControl::DeleteSelectedItem()
{
	CString		strMaster;
	strMaster.LoadString(IDS_STRING_MASTER);
	HTREEITEM	htSelectedItem = NULL;
	htSelectedItem = GetSelectedItem();

	CTexturesDlg	*pTextureDlg = NULL;
	pTextureDlg = (CTexturesDlg*)GetParent();
	if (pTextureDlg)
	{
		if (pTextureDlg->IsKindOf(RUNTIME_CLASS(CTexturesDlg)))
		{
			if (GetParentItem(htSelectedItem) == GetRootItem())
			{
				if (GetItemText(htSelectedItem) != strMaster)
					DeleteItem(htSelectedItem);

				pTextureDlg->SetSelectedGroup(GetRootItem());
			}
			else
			{
				if (GetItemText(GetParentItem(htSelectedItem)) != strMaster)
				{
					m_htCurrentGroup = GetParentItem(htSelectedItem);
					DeleteItem(htSelectedItem);

					pTextureDlg->SetSelectedGroup(m_htCurrentGroup);
				}
			}
		}
	}
}


void	CTextureTreeControl::SetSelectedTexture(CString	strTextureName)
{
	HTREEITEM	htChild = NULL;
	htChild = GetNextItem(m_htCurrentGroup, TVGN_CHILD);
	while (htChild)
	{
		if (GetItemText(htChild) == strTextureName)
		{
			this->SelectItem(htChild);
			return;
		}

		htChild = GetNextItem(htChild, TVGN_NEXT);
	}
}


void CTextureTreeControl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	switch (nChar)
	{
		case VK_DELETE:
			DeleteSelectedItem();
			break;
		default:
			break;
	}

	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CTextureTreeControl::OnContextMenu(CWnd* pWnd, CPoint point)
{
			//	Adjust point if needed (for keyboard context menu)
	if( point.x == -1 && point.y == -1 )
	{
		CRect rect;
		GetClientRect( &rect );
		point = rect.TopLeft();
		point.Offset( 5, 5 );
		ClientToScreen( &point );
	}
				//  Load top-level menu from resource
	CMenu mnuTop;
	mnuTop.LoadMenu( IDR_POPUP_TEXTURE_TREELIST );

				//  Get popup menu from first sub-menu
	CMenu	*pContextMenu = NULL;
	pContextMenu = mnuTop.GetSubMenu( 0 );
	ASSERT_VALID( pContextMenu );

				//	Checked state for popup menu items is automatically
				//	managed by standard MFC UPDATE_COMMAND_UI mechanism!

				//  Display popup menu
	
	if(!pContextMenu->TrackPopupMenu(	TPM_LEFTALIGN | TPM_LEFTBUTTON,
							point.x, point.y, AfxGetMainWnd(), NULL ))
							MessageBeep(0);

				//	Popup menu commands are automatically handled
				//	by standard MFC command-routing mechanism!
	pContextMenu->DestroyMenu();
}


void CTextureTreeControl::OnIdrDeleteitem()
{
	DeleteSelectedItem();
}


void CTextureTreeControl::OnUpdateIdrDeleteitem(CCmdUI *pCmdUI)
{
	CString		strMaster;
	strMaster.LoadString(IDS_STRING_MASTER);
	HTREEITEM	htSelectedItem = NULL;
	htSelectedItem = GetSelectedItem();

	if ((htSelectedItem != GetRootItem()) &&
		(GetItemText(htSelectedItem) != strMaster) &&
		(GetItemText(GetParentItem(htSelectedItem)) != strMaster))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}


void CTextureTreeControl::OnIdrAddgroup()
{
	InsertItem("NewGroup", GetRootItem());
//	SortChildren(GetRootItem());
}


void CTextureTreeControl::OnUpdateIdrAddgroup(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


void	CTextureTreeControl::ActivateHighlightedTexture()
{
	CTexturesDlg		*pParent = NULL;
	CString				strSelectedTexture;

	pParent = (CTexturesDlg*)GetParent();

	if (pParent)
	{
		if (!pParent->GetWorkerThreadActivity())
		{

			HTREEITEM	htItem = NULL;
			htItem = GetSelectedItem();
			m_htRoot = GetRootItem();

			if ((htItem != m_htRoot) && (GetParentItem(htItem) != m_htRoot))
			{
				strSelectedTexture = GetItemText(htItem);
				m_htCurrentGroup = GetParentItem(htItem);

				if (pParent->IsKindOf(RUNTIME_CLASS(CTexturesDlg)))
				{
					pParent->SetSelectedGroup(m_htCurrentGroup);
					pParent->SetSelectedTexture(strSelectedTexture, this);
				}
			}

			if (GetParentItem(htItem) == m_htRoot)
			{
				m_htCurrentGroup = htItem;

				if (pParent->IsKindOf(RUNTIME_CLASS(CTexturesDlg)))
				{
					pParent->SetSelectedGroup(m_htCurrentGroup);
				}
			}
		}
	}
}


void CTextureTreeControl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	// TODO: Add your control notification handler code here

	ActivateHighlightedTexture();

	*pResult = 0;
}



void CTextureTreeControl::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	// TODO: Add your control notification handler code here

	ActivateHighlightedTexture();

	*pResult = 0;
}


void CTextureTreeControl::OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here
	HTREEITEM	htSelectedItem = NULL;
	HTREEITEM	htGrandParent = NULL;
	CString		strItem, strGlobalMat, strMaster;

	htSelectedItem = GetSelectedItem();
	htGrandParent = GetParentItem(GetParentItem(htSelectedItem));
	strItem = GetItemText(htSelectedItem);
	strGlobalMat.LoadString(IDS_STRING_GLOBALMATERIALS);
	strMaster.LoadString(IDS_STRING_MASTER);

	if ( (htGrandParent) ||
		(strItem == strGlobalMat) ||
		(strItem == strMaster)
		)
	{
		*pResult = 1;	//	1 prevents editing
		return;
	}

	m_pEdit = GetEditControl();
	*pResult = 0;	//	0 allows editing
}


void CTextureTreeControl::OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here

	m_pEdit = GetEditControl();

	CString		sLabel;
	m_pEdit->GetWindowText(sLabel);
	SetItemText((GetSelectedItem()), sLabel);

	*pResult = 0;
}


