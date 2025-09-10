/****************************************************************************************/
/*  LISTS.H                                                                             */
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
#if !defined(AFX_LISTS_H__80CE9B61_FE23_11D2_8B42_00104B70D76D__INCLUDED_)
#define AFX_LISTS_H__80CE9B61_FE23_11D2_8B42_00104B70D76D__INCLUDED_

#include "Basetype.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Doc.h"
#include "TreeCtrlEx.h"

/////////////////////////////////////////////////////////////////////////////
// CLists dialog

class CLists : public CDialog
{
// Construction
public:
	void AddObject(Object *pObject);
	void RenameObject( Object *pObject );
	void AddSelection( CJweDoc *pDoc );
	void RemoveDeleted( );
	void Update( CJweDoc * pDoc );
	void SetCurrentDocument( CJweDoc * pDoc );
	void Reset();
	CLists(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLists)
	enum { IDD = IDD_LISTS };
	CTreeCtrlEx	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLists)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLists)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTvItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnKeydownTvItems(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void	DeRefAllObjects();
	static HTREEITEM FindObjectKind( CTreeCtrlEx * pList, Object * pObject );
	static HTREEITEM AddObjectKind( CTreeCtrlEx *pList, char * pszDisplayName, int Kind );
	static jeBoolean ObjectCB(Object *pObject, void *lParam);
	static jeBoolean AddSelectionCB( Object *pObject, void *lParam );
	static jeBoolean SelectCB( Object * pObject, void * lParam );
	static jeBoolean EntityCB( Entity * pEntity, void * lParam );
	static jeBoolean LightCB( Light * pLight, void * lParam );
	static jeBoolean CameraCB( Camera * pCamera, void * lParam );
	static jeBoolean BrushCB( Brush * pBrush, void * lParam );
	static jeBoolean ModelCB( Model * pModel, void* lParam );
	static jeBoolean RemoveSelectionCB(Object *pObject, void *lParam);
	HTREEITEM FindObjectItem( Object* pObject );
	HTREEITEM m_hItemModels;
	HTREEITEM m_hItemLights;
	HTREEITEM m_hItemCameras;
	HTREEITEM m_hItemEntities;
	HTREEITEM m_hItemBrushes;
	HTREEITEM m_hItemActors;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTS_H__80CE9B61_FE23_11D2_8B42_00104B70D76D__INCLUDED_)
