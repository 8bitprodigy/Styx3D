/****************************************************************************************/
/*  LIGHTTEMPLATE.H                                                                     */
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
#if !defined(AFX_LIGHTTEMPLATE_H__330654A2_F0B7_11D2_8B42_00104B70D76D__INCLUDED_)
#define AFX_LIGHTTEMPLATE_H__330654A2_F0B7_11D2_8B42_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ColorBtn.h"
#include "ParEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CLightTemplate dialog

class CLightTemplate : public CDialog
{
// Construction
public:
	CLightTemplate(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLightTemplate)
	enum { IDD = IDD_TEMPLATELIGHT };
	CColorButton m_bnColor;
	CString	m_csName;
	float	m_fRadius;
	float	m_fIntensity;
	BOOL	m_Arch;
	BOOL	m_Cone;
	BOOL	m_Cube;
	BOOL	m_Cylinder;
	BOOL	m_Entity;
	BOOL	m_Light;
	BOOL	m_Sphere;
	BOOL	m_Stair;
	BOOL	m_Terrain;
	//}}AFX_DATA
	
	CParsedEdit	m_edit1 ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLightTemplate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	LRESULT Update( WPARAM wParam, LPARAM lParam ) ;
	LRESULT OnChangeColor( WPARAM wParam, LPARAM lParam ) ;
	// Generated message map functions
	//{{AFX_MSG(CLightTemplate)
	virtual BOOL OnInitDialog();
	afx_msg void OnToolsPlacecube();
	afx_msg void OnToolsPlaceentity();
	afx_msg void OnToolsPlaceterrain();
	afx_msg void OnKillfocusEdIntensity();
	afx_msg void OnKillfocusEdName();
	afx_msg void OnKillfocusEdRadius();
	afx_msg void OnToolsPlacespheroid();
	afx_msg void OnToolsPlacelight();
	afx_msg void OnToolsPlacecylinder();
	afx_msg void OnAdd();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnToolsPlacesheet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateTemplate( LightInfo *pLightInfo, int32 BlankFieldFlag );
	void GetFields( LightInfo * pLightInfo );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTTEMPLATE_H__330654A2_F0B7_11D2_8B42_00104B70D76D__INCLUDED_)
