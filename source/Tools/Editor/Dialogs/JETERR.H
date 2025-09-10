/****************************************************************************************/
/*  JETERR.H                                                                            */
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
#if !defined(AFX_GENERR_H__A31355C0_5A3B_11D3_B323_004033AA0441__INCLUDED_)
#define AFX_GENERR_H__A31355C0_5A3B_11D3_B323_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CJetErr dialog

class CJetErr : public CDialog
{
// Construction
public:
	CJetErr(CWnd* pParent = NULL);   // standard constructor
	int	m_CurErrorN;
	BOOL bWarning;

// Dialog Data
	//{{AFX_DATA(CJetErr)
	enum { IDD = IDD_ERROR };
	CStatic	m_Icon;
	CString	m_Error;
	CString	m_Log;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJetErr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CJetErr)
	virtual BOOL OnInitDialog();
	afx_msg void OnMore();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERR_H__A31355C0_5A3B_11D3_B323_004033AA0441__INCLUDED_)
