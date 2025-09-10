/****************************************************************************************/
/*  JETVIEW.H                                                                           */
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
#if !defined(AFX_JETVIEW_H__E0B7E841_C1C6_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_JETVIEW_H__E0B7E841_C1C6_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "J3DView.h"
#include "Doc.h"
/////////////////////////////////////////////////////////////////////////////
// CJetView view

class CJetView : public CJ3DView
{
protected:
	CJetView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CJetView)

// Attributes
public:

// Operations
public:
	CJweDoc* GetDocument();
	jeCamera* GetCamera(void);
	void OnViewType( UINT nID );
	void SetCameraPos( jeVec3d * Pos );
	jeBoolean  RegisterBitmap( jeBitmap * pBitmap );
	jeBoolean	FullscreenView( void );
	jeBoolean	ChooseWindowVideoSettings( void );
	jeBoolean	ChooseFullscreenVideoSettings( void );
	jeBoolean	UpdateWindow( void );
	void		Animate( jeBoolean bAnimate );

	jeBoolean	SetFullscreenModeByString(char	*sDriverMode); // Added JH 7.3.2000
	jeBoolean	SetWindowModeByString(char	*sDriverMode);	// Added JH 7.3.2000

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJetView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	protected:
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CJetView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CJetView)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnViewCenterselction();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void On3dviewLines();
	afx_msg void On3dviewTextured();
	afx_msg void On3dviewTexturedwlights();
	afx_msg void On3dviewFlat();
	afx_msg void On3dviewBspsplits();
	afx_msg void OnBilinear();
	//}}AFX_MSG
	afx_msg void OnUpdateViewType( CCmdUI* pCmdUI ) ;
	DECLARE_MESSAGE_MAP()
private:
	void RotateCameraUpDown(long Delta);
	void RotateCameraLeftRight(long Delta);
	void MoveCameraInOut(long Delta);
	void MoveCameraLeftRight(long Delta);
	void MoveCameraUpDown(long Delta);
	char * GetModeName( );
	void ShowMenu( CPoint point);

	CPoint m_Anchor;
	UINT m_nViewType;

	// Camera stuff
	jeVec3d m_CameraPos;
	jeVec3d m_CameraLeft;
	jeVec3d m_CameraUp;
	jeVec3d m_CameraIn;
	float	m_CameraRotX;
	float	m_CameraRotY;
	jeXForm3d m_CameraXForm;
	BOOL m_bRecalcCamera;
	jeCamera* m_pCamera;
	bool m_bDragging;
	bool m_bAnimate;
	static int m_CXDRAG ;
	static int m_CYDRAG ;
	int		m_RenderMode;
	float	m_LastTime;

};

#ifndef _DEBUG  // debug version in J3DView.cpp
inline CJweDoc* CJetView::GetDocument()
   { return (CJweDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JETVIEW_H__E0B7E841_C1C6_11D2_8B41_00104B70D76D__INCLUDED_)

