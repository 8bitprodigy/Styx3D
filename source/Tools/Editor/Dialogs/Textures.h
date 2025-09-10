/****************************************************************************************/
/*  TEXTURES.H                                                                          */
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
#if !defined(AFX_TEXTURES_H__EF299B22_C4E2_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_TEXTURES_H__EF299B22_C4E2_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyStatic.h"
#include "Basetype.h"	// Added by ClassView


/////////////////////////////////////////////////////////////////////////////
// CTextures dialog

class CTextures : public CDialog
{
// Construction
public:
	CBitmap m_Textures;
	CTextures(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTextures)
	enum { IDD = IDD_TEXTURE };
	CComboBox	m_Names;
	CScrollBar	m_Scroll;
	CMyStatic	m_Bitmap;
	CString	m_Info;
	int		m_TexSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextures)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTextures)
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBn128();
	afx_msg void OnBn32();
	afx_msg void OnBn64();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSelchangeCbName();
    afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	static jeBoolean UpdateNamesCB( Material_Struct * pMaterial, void * lParam );
	static jeBoolean UpdateNamesTR( Material_Struct * pShader, void * lParam );
	MaterialList_Struct * m_pMaterials;
	MaterialList_Struct * m_pShaders;
	void UpdateNames( void );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURES_H__EF299B22_C4E2_11D2_8B41_00104B70D76D__INCLUDED_)
