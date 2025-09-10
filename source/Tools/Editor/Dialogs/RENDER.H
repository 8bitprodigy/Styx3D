/****************************************************************************************/
/*  RENDER.H                                                                            */
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
#if !defined(AFX_RENDER_H__3A9C0940_0ECF_11D3_B322_004033AA0441__INCLUDED_)
#define AFX_RENDER_H__3A9C0940_0ECF_11D3_B322_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CRender dialog
#include "jeModel.h"
#include "Doc.h"

class CRender : public CDialog
{
// Construction
public:
	CRender(CWnd* pParent = NULL);   // standard constructor
	void SetData( const jeBSP_DebugInfo * pDebugInfo );
	void SetCurrentDocument( CJweDoc *pDoc );


// Dialog Data
	//{{AFX_DATA(CRender)
	enum { IDD = IDD_RENDER };
	int		m_Areas;
	int		m_Brushes;
	int		m_Faces;
	int		m_Leafs;
	int		m_Nodes;
	int		m_Splits;
	int		m_Portals;
	int		m_Subfaces;
	int		m_TotalFaces;
	int		m_VisFaces;
	int		m_VisPortals;
	int		m_LightUpdate;
	int		m_BrushUpdate;
	BOOL	m_BrushLighting;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRender)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRender)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrushLighting();
	afx_msg void OnBrushOnchange();
	afx_msg void OnBrushOndeselect();
	afx_msg void OnBrushOnupdate();
	afx_msg void OnLightOnchange();
	afx_msg void OnLightOndeselect();
	afx_msg void OnLightOnupdate();
	afx_msg void OnLightRealtime();
	afx_msg void OnUpdateSel();
	afx_msg void OnUpdateall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateLevelData();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENDER_H__3A9C0940_0ECF_11D3_B322_004033AA0441__INCLUDED_)
