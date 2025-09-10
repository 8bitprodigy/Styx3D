/****************************************************************************************/
/*  LIGHTS.H                                                                            */
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
#if !defined(AFX_LIGHTS_H__00CD4901_FC81_11D2_8B42_00104B70D76D__INCLUDED_)
#define AFX_LIGHTS_H__00CD4901_FC81_11D2_8B42_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ColorBtn.h"
#include "ParEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CLights dialog

class CLights : public CDialog
{
// Construction
public:
	void Update( CJweDoc *pDoc );
	CLights(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLights)
	enum { IDD = IDD_LIGHT };
	CColorButton m_bnColor;
	CString	m_csfBrightness;
	CString	m_csfRadius;
	CString	m_csfX;
	CString	m_csfY;
	CString	m_csfZ;
	CString	m_csName;
	CString	m_csNumber;
	BOOL	m_Arch;
	BOOL	m_Cone;
	BOOL	m_Cube;
	BOOL	m_Cylinder;
	BOOL	m_Entity;
	BOOL	m_Light;
	BOOL	m_Sheet;
	BOOL	m_Sphere;
	BOOL	m_Stair;
	BOOL	m_Terrain;
	//}}AFX_DATA

	CParsedEdit	m_edit1 ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLights)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	LRESULT OnChangeColor( WPARAM wParam, LPARAM lParam ) ;
	// Generated message map functions
	//{{AFX_MSG(CLights)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusEdBrightness();
	afx_msg void OnKillfocusEdRadius();
	afx_msg void OnKillfocusEdX();
	afx_msg void OnKillfocusEdY();
	afx_msg void OnKillfocusEdZ();
	afx_msg void OnBnColor();
	afx_msg void OnKillfocusEdName();
	afx_msg void OnToolsPlaceterrain();
	afx_msg void OnToolsPlacestaircase();
	afx_msg void OnToolsPlacespheroid();
	afx_msg void OnToolsPlacelight();
	afx_msg void OnToolsPlacesheet();
	afx_msg void OnToolsPlacecylinder();
	afx_msg void OnToolsPlacecube();
	afx_msg void OnToolsPlacecone();
	afx_msg void OnToolsPlacearch();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnToolsPlaceentity();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetPosition( jeVec3d * pPos );
	void GetPosition( jeVec3d * pPos );
	void FillFields( LightInfo * pLightInfo, int32 BlankFieldFlag, const char * pszName, int32 nNumber );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTS_H__00CD4901_FC81_11D2_8B42_00104B70D76D__INCLUDED_)
