/****************************************************************************************/
/*  STATS.H                                                                             */
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
#if !defined(AFX_STATS_H__78722C65_0A31_11D3_B322_004033AA0441__INCLUDED_)
#define AFX_STATS_H__78722C65_0A31_11D3_B322_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "jeModel.h"

/////////////////////////////////////////////////////////////////////////////
// CStats dialog

class CStats : public CDialog
{
// Construction
public:
	CStats(CWnd* pParent = NULL);   // standard constructor
	void SetData( const jeBSP_DebugInfo * pDebugInfo );

// Dialog Data
	//{{AFX_DATA(CStats)
	enum { IDD = IDD_DIALOG1 };
	int		m_Areas;
	int		m_Brushes;
	int		m_Faces;
	int		m_Leafs;
	int		m_Nodes;
	int		m_Portals;
	int		m_Splits;
	int		m_Subfaces;
	int		m_TotalsFaces;
	int		m_VisFaces;
	int		m_VisPortals;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStats)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStats)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CStats)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATS_H__78722C65_0A31_11D3_B322_004033AA0441__INCLUDED_)
