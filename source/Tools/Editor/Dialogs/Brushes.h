/****************************************************************************************/
/*  BRUSHES.H                                                                           */
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
#if !defined(AFX_BRUSHES_H__62800583_C71A_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_BRUSHES_H__62800583_C71A_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "jwe.h"

// Brushes.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CBrushes dialog

class CBrushes : public CDialog
{
// Construction
public:
	CBrushes(CWnd* pParent = NULL);   // standard constructor
	void Update( CJweDoc *pDoc );
	void FillFields( uint32 Contents, int32 BlankFieldFlag );

// Dialog Data
	//{{AFX_DATA(CBrushes)
	enum { IDD = IDD_BRUSH };
	CButton	m_LockTextures;
	CButton	m_Flocking;
	CComboBox	m_Type;
	int		m_Draw;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrushes)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBrushes)
	virtual BOOL OnInitDialog();
	afx_msg void OnToolsPlaceterrain();
	afx_msg void OnToolsPlacespheroid();
	afx_msg void OnToolsPlacelight();
	afx_msg void OnToolsPlacecylinder();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCkFlocking();
	afx_msg void OnRadioCut();
	afx_msg void OnRadioEmpty();
	afx_msg void OnRadioSolid();
	afx_msg void OnCkLocktexture();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SetDrawMode();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BRUSHES_H__62800583_C71A_11D2_8B41_00104B70D76D__INCLUDED_)
