/****************************************************************************************/
/*  J3DVIEW.H                                                                           */
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
#if !defined(AFX_J3DVIEW_H__664E60E1_5211_11D2_8F57_00A0C96E625A__INCLUDED_)
#define AFX_J3DVIEW_H__664E60E1_5211_11D2_8F57_00A0C96E625A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Jet.h"

class CJ3DDoc;

/////////////////////////////////////////////////////////////////////////////
// CJ3DView view

class CJ3DView : public CView
{
protected:
	CJ3DView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CJ3DView)

// Attributes
public:
	CJ3DDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJ3DView)
	public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual BOOL DestroyWindow();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	jeEngine* GetEngine(void) { return(m_pEngine); }
	void PostEnableEngine(void);
	void Render();
protected:
	virtual ~CJ3DView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	BOOL m_bFullScreen;
	BOOL CreateFullWnd();
	HWND m_hFullWnd;
	HWND m_hRenderWnd;
	jeBoolean EnableEngine();
	//-------------------
	// Jet3D stuff
	jeEngine* m_pEngine;
	jeBoolean m_bEngineEnabled;
	jeDriver* m_pDriver;
	jeDriver_Mode* m_pDriverMode;
	//{{AFX_MSG(CJ3DView)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnPrivateMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in J3DView.cpp
inline CJ3DDoc* CJ3DView::GetDocument()
   { return (CJ3DDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_J3DVIEW_H__664E60E1_5211_11D2_8F57_00A0C96E625A__INCLUDED_)
