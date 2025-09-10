/****************************************************************************************/
/*  GROUPS.H                                                                            */
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
#if !defined(AFX_GROUPS_H__E77B58C1_C292_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_GROUPS_H__E77B58C1_C292_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DragTree.h"
#include "Doc.h"
#include "GroupTreeCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CGroups dialog

class CGroups : public CDialog
{
// Construction
public:
	CGroups(CWnd* pParent = NULL);   // standard constructor
	~CGroups();
	void SetCurrentDocument(CJweDoc *pDoc);
	jeBoolean AddObject( Object* pObject );
	void RenameObject( Object *pObject );
	void AddSelection(CJweDoc *pDoc);
	void RemoveDeleted();
	void Update(CJweDoc *pDoc);
	void Reset();
	void ExportPrefab() { OnWorldGroupExporttoprefab(); }
	void ImportPrefab() { OnWorldGroupImportfrom(); }
    bool IsCurrentSelectionHidable();
    bool IsCurrentSelectionShowable();
    bool HasHiddenItem() { return (m_lHiddenItemCount > 0); }
    void ToggleSelectionVisibleState() { OnWorldmodelShow(); }
    void ShowAllGroups();

// Dialog Data
	//{{AFX_DATA(CGroups)
	enum { IDD = IDD_GROUPS };
	CGroupTreeCtrl	m_List;
	CComboBox	m_CBList;
	BOOL	m_Lock;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroups)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGroups)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTvItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddgroup();
	afx_msg void OnSelchangeCbCurrent();
	afx_msg void OnLock();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnKeydownTvItems(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRclickTvItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnWorldGroupExporttoprefab();
	afx_msg void OnWorldGroupImportfrom();
	afx_msg void OnWorldGroupCreate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CImageList m_ImageList;
    long m_lHiddenItemCount;
	
	static jeBoolean AddSelectionCB(Object *pObject, void *lParam);
	static jeBoolean GroupListCB( Group *pGroup, void *lParam);
	static jeBoolean GroupComboCB( Group *pGroup, void *lParam);
	static jeBoolean ObjectCB( Object *pObject, void *lParam);
	static jeBoolean RemoveSelectionCB(Object *pObject, void *lParam);
	static jeBoolean SelectCB(Object *pObject, void *lParam);
	static HTREEITEM GetObjectItem( CTreeCtrl *pList, Object *pObject );

	void   DeRefAllObjects();
	void   ChangeGroups( HTREEITEM hItem );
	void   SelectGroup( HTREEITEM hGroupItem, jeBoolean bSelect );

	static jeBoolean GroupNameCB( Group *pGroup, void *lParam);
	static jeBoolean AddObjectCB( Object *pBrush, void *lParam);
public:
    afx_msg void OnWorldmodelInvertshow();
    afx_msg void OnWorldmodelShow();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPS_H__E77B58C1_C292_11D2_8B41_00104B70D76D__INCLUDED_)
