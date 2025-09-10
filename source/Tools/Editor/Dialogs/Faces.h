/****************************************************************************************/
/*  FACES.H                                                                             */
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
#if !defined(AFX_FACES_H__62800581_C71A_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_FACES_H__62800581_C71A_11D2_8B41_00104B70D76D__INCLUDED_

#include "Basetype.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CFaces dialog

class CFaces : public CDialog
{
// Construction
public:
	void Update( CJweDoc * pDoc );
	CFaces(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFaces)
	enum { IDD = IDD_FACE };
	CButton	m_CkMirror;
	CButton	m_ckTranparent;
	CButton	m_ckPortal;
	CButton	m_ckGouraud;
	CString	m_csiAngle;
	CString	m_csfShiftU;
	CString	m_csfShiftV;
	CString	m_csfDrawScaleU;
	CString	m_csfDrawScaleV;
	CString	m_csfLMapScaleU;
	CString	m_csfLMapScaleV;
	float	m_Alpha;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFaces)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFaces)
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKillfocusEdTexturex();
	afx_msg void OnCkGouraud();
	afx_msg void OnKillfocusEdAngle();
	afx_msg void OnKillfocusEdDrawscalex();
	afx_msg void OnKillfocusEdDrawscaley();
	afx_msg void OnKillfocusEdLightmapx();
	afx_msg void OnKillfocusEdLightmapy();
	afx_msg void OnKillfocusEdTexturey();
	afx_msg void OnCkInvisible();
	afx_msg void OnCkPortal();
	afx_msg void OnKillfocusEdTransparent();
	afx_msg void OnCkTransparent();
	afx_msg void OnCkMirror();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	jeFloat Increment( jeFloat fCur, jeFloat fMin, jeFloat fMax, jeFloat fInc, jeBoolean bDown );
	void FillFields( jeFaceInfo * pFaceInfo, int32 BlankFieldFlag );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FACES_H__62800581_C71A_11D2_8B41_00104B70D76D__INCLUDED_)
