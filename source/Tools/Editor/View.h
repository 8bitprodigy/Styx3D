/****************************************************************************************/
/*  VIEW.H                                                                              */
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

/* Open Source Revision -----------------------------------------------------------------
 By: Dennis Tierney (DJT) dtierney@oneoverz.com
 On: 12/27/99 8:58:41 PM
 Comments: 1) Added menu items and handlers for "Clone Selected"
----------------------------------------------------------------------------------------*/


#if !defined(AFX_VIEW_H__37F45639_C0E1_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_VIEW_H__37F45639_C0E1_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Doc.h"
#include "Ortho.h"
#include "Select.h"
//	tom morris feb 2005 -- commented out -- moved to view.cpp
//TOM
//#include "MemDC.h"
//	end tom morris feb 2005


typedef enum {
	VIEW_SCROLL_NONE,
	VIEW_SCROLL_UP		= 0x01,
	VIEW_SCROLL_DOWN	= 0x02,
	VIEW_SCROLL_RIGHT	= 0x04,
	VIEW_SCROLL_LEFT	= 0x08
} VIEW_SCROLL_TYPE;

typedef enum {
	VIEW_MODE_NONE,
	VIEW_MODE_PLACELIGHT,
	VIEW_MODE_PLACEBRUSH,
	VIEW_MODE_DRAG_VERTEX,
	VIEW_MODE_MOVE_SELECTION,
	VIEW_MODE_MOVE_SUBSELECTION,
	VIEW_MODE_ROTATE_SUBSELECTION,
	VIEW_MODE_ROTATE_HANDLE,
	VIEW_MODE_SIZE_HANDLE,
	VIEW_MODE_SHEAR_HANDLE,
	VIEW_MODE_DRAG_BRUSH_RECT,
	VIEW_MODE_DRAG_BRUSH_HEIGHT,
	VIEW_DRAG_HCONSTRUCTOR,
	VIEW_DRAG_VCONSTRUCTOR,
	VIEW_DRAG_BCONSTRUCTOR,
	VIEW_MODE_SELECT_RECT
} VIEW_MODE;

class CJweView : public CView
{
protected: // create from serialization only
	CJweView();
	DECLARE_DYNCREATE(CJweView)

// Attributes
public:
	CJweDoc* GetDocument();
	void SetCameraPos( jeVec3d * Pos );
	void AbortMode();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJweView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//	tom morris may 2005
	virtual void PostNcDestroy();
	//	end tom morris may 2005
	protected:
	//virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	//virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	//virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CJweView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CJweView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnViewZoomin();
	afx_msg void OnUpdateViewZoomin(CCmdUI* pCmdUI);
	afx_msg void OnViewZoomout();
	afx_msg void OnUpdateViewZoomout(CCmdUI* pCmdUI);
	afx_msg void OnViewCenterselction();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
//---------------------------------------------------
// Added DJT
//---------------------------------------------------
	afx_msg void OnEditClone();
	afx_msg void OnUpdateEditClone(CCmdUI* pCmdUI);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
//---------------------------------------------------
// End DJT
//---------------------------------------------------

	//}}AFX_MSG
public:
	void DoZoom( jeFloat fZoomInc ) ;
	afx_msg void OnViewType( UINT nID ) ;
	Ortho	*GetOrtho();	// Added JH

	//	tom morris may 2005
	LRESULT		OnAutosaveTimer(WPARAM w, LPARAM l);
	//	end tom morris

protected:
	afx_msg void OnUpdateViewType( CCmdUI* pCmdUI ) ;
	DECLARE_MESSAGE_MAP()
private:
	int32 GetAutoScrollRegion( POINT * ptCursor );
	void SetAutoScroll( int32 ScrollRegion );
	void SetBeginDragViewMode(POINT ptCursor);
	void Pan( jeVec3d *pWorldDistance );
	jeBoolean IsBeginDrag( POINT ptCursor );
	void Drag( POINT ptCursor, jeVec3d *pWorldDistance );
	void DragEnd(CPoint point);
	int32 GetViewSigniture();
	void ShowMenu(CPoint point) ;
	void SetUpRotateBox(jeExtBox  *pSelBounds );
	void DrawRotateBox( HDC hDC);

	SELECT_HANDLE m_SizeType;
	CPoint m_ptAnchor;
	CPoint m_ptLastMouse;
	CPoint m_ptVertualMouse;  //Used for draging brush height

	jeVec3d	m_Center3d;
	CPoint m_SelCenter;
	RECT   m_RotateBox;
	float  m_RotateRadius;

	VIEW_MODE m_Mode;
	bool m_bCaptured;
	bool m_bDragging;
	bool m_bPanning;
	bool m_bZooming;
	int32 m_ScrollType;
	UINT m_nViewType;
	static int m_CXDRAG ;
	static int m_CYDRAG ;
	bool m_bInit ;
	Ortho * m_pOrtho;

	//	tom morris may 2005
	CProgressCtrl	m_wndProgress;
	bool			m_bPostNcDestroyedOnce;

//	tom morris feb 2005 -- updated, commented out, not needed
//	TOM
//	a buffer for the part of drawing which does not change
//	CBufferMemDC m_ViewBuffer;
							//	a buffer which is used for flicker free redrawing
							//	on top of the background
//	end tom morris feb 2005

};

#ifndef _DEBUG  // debug version in View.cpp
inline CJweDoc* CJweView::GetDocument()
   { return (CJweDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEW_H__37F45639_C0E1_11D2_8B41_00104B70D76D__INCLUDED_)
