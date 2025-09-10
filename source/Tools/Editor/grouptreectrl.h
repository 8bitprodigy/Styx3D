/****************************************************************************************/
/*  GROUPTREECTRL.H                                                                     */
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
#if !defined(AFX_GROUPTREECTRL_H__42A85663_14DE_11D3_B322_004033AA0441__INCLUDED_)
#define AFX_GROUPTREECTRL_H__42A85663_14DE_11D3_B322_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "TreeCtrlEx.h"

// grouptreectrl.h : header file
//
#define TVC_DRAG_END 0x0005
/////////////////////////////////////////////////////////////////////////////
// CGroupTreeCtrl window

class CGroupTreeCtrl : public CTreeCtrlEx
{
// Construction
public:
	BOOL        m_bDragging;
	HTREEITEM   m_hitemDrag;
	HTREEITEM   m_hitemDrop;
	CImageList  *m_pimagelist;
	CGroupTreeCtrl();

// Attributes
public:

// Operations
public:
	// From TreeDragDropDemo
	// Author: Sudheesh.P.S
	// Url: http://www.codeproject.com/treectrl/MultiSelect_DragImage.asp
	CImageList* CreateDragImageEx();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGroupTreeCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGroupTreeCtrl)
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	BOOL	IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent);
	HTREEITEM	TransferItem( HTREEITEM hitemDrag, HTREEITEM hitemDrop );

	CPoint ptLButtonDown;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPTREECTRL_H__42A85663_14DE_11D3_B322_004033AA0441__INCLUDED_)
