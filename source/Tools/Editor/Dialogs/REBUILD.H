/****************************************************************************************/
/*  REBUILD.H                                                                           */
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
#if !defined(AFX_REBUILD_H__DBA9FD04_12A9_11D3_B322_004033AA0441__INCLUDED_)
#define AFX_REBUILD_H__DBA9FD04_12A9_11D3_B322_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "jeModel.h"

#define IDBSP 1000
/////////////////////////////////////////////////////////////////////////////
// CRebuild dialog

class CRebuild : public CDialog
{
// Construction
public:
	CRebuild(CWnd* pParent = NULL);   // standard constructor
	int GetBalance();
	BOOL GetCSG();
	BOOL GetSolid();
	BOOL GetVis();
	jeBSP_Logic	 GetLogic();
	int m_Balance;


// Dialog Data
	//{{AFX_DATA(CRebuild)
	enum { IDD = IDD_REBUILD };
	CSliderCtrl	ctrl_Balance;
	BOOL	m_CSG;
	BOOL	m_Solid;
	BOOL	m_Vis;
	int		m_Logic;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRebuild)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRebuild)
	afx_msg void OnReleasedcaptureBalance(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnLightsonly();
	afx_msg void OnRebuildbsp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REBUILD_H__DBA9FD04_12A9_11D3_B322_004033AA0441__INCLUDED_)
