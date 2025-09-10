/****************************************************************************************/
/*  MODELS.H                                                                            */
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
#if !defined(AFX_MODELS_H__DE77E6A7_48FA_11D3_B323_004033AA0441__INCLUDED_)
#define AFX_MODELS_H__DE77E6A7_48FA_11D3_B323_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DragTree.h"
#include "Doc.h"
#include "GroupTreeCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CModel dialog

class CModel : public CDialog
{
// Construction
public:
	CModel(CWnd* pParent = NULL);   // standard constructor
	void SetCurrentDocument(CJweDoc *pDoc);
	jeBoolean AddObject( Object *pObject );
	void RenameObject( Object *pObject );
	void AddSelection(CJweDoc *pDoc);
	void RemoveDeleted();
	void Update(CJweDoc *pDoc);
	void Reset();

// Dialog Data
	//{{AFX_DATA(CModel)
	enum { IDD = IDD_MODELS };
	CComboBox	m_CBList;
	CGroupTreeCtrl	m_List;
	BOOL	m_Lock;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModel)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddmodel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLock();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangedTvItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeCbCurrent();
	afx_msg void OnKeydownTvItems(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void DeRefAllObjects();
	CImageList m_ImageList;
	static jeBoolean AddSelectionCB(Object *pObject, void *lParam);
	static jeBoolean ModelListCB( Model *pModel, void *lParam);
	static jeBoolean ModelComboCB( Model *pModel, void *lParam);
	static jeBoolean BrushCB( Brush *pBrush, void *lParam);
	static jeBoolean RemoveSelectionCB(Object *pObject, void *lParam);
	static jeBoolean SelectCB(Object *pObject, void *lParam);
	static HTREEITEM GetObjectItem( CTreeCtrl *pList, Object *pObject );
	void	ChangeModels( HTREEITEM hItem );
	void	SelectGroup( HTREEITEM hGroupItem, jeBoolean bSelect );
	void UpdateCurModel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODELS_H__DE77E6A7_48FA_11D3_B323_004033AA0441__INCLUDED_)
