/****************************************************************************************/
/*  TextureTreeControl.h                                                                */
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

#pragma once
#include "MyDropTarget.h"


// CTextureTreeControl

class CTextureTreeControl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CTextureTreeControl)

public:
	CTextureTreeControl();
	virtual ~CTextureTreeControl();


protected:
	DECLARE_MESSAGE_MAP()

	CMyDropTarget	m_DropTarget;
	HTREEITEM		m_htRoot, m_htDragItem, m_htDropItem, m_htCurrentGroup;
	CEdit			*m_pEdit;
	CRectTracker	*m_pRectTracker;



private:
	bool	NotAlreadyInGroup(HTREEITEM htParent, CString	strDroppedItem);
	void	ActivateHighlightedTexture();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	LRESULT		OnDropAction(WPARAM wParam = 0, LPARAM lParam = 0);

public:
	HTREEITEM	GetDropItem();
	void		SetSelectedTexture(CString	strTextureName);
	void		DeleteSelectedItem();
	void		OnIdrDeleteitem();
	void		OnUpdateIdrDeleteitem(CCmdUI *pCmdUI);
	void		OnIdrAddgroup();
	void		OnUpdateIdrAddgroup(CCmdUI *pCmdUI);
	void		OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);


	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};


