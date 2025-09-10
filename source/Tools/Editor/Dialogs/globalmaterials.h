/****************************************************************************************/
/*  GLOBALMATERIALS.H                                                                   */
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
#if !defined(AFX_GLOBALMATERIALS_H__9EA467C6_1E83_11D3_95F4_004033AA0439__INCLUDED_)
#define AFX_GLOBALMATERIALS_H__9EA467C6_1E83_11D3_95F4_004033AA0439__INCLUDED_

#include "MaterialList2.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CGlobalMaterials dialog

class CGlobalMaterials : public CDialog
{
// Construction
public:
	CGlobalMaterials(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGlobalMaterials)
	enum { IDD = IDD_GLOBALMATERIALS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGlobalMaterials)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGlobalMaterials)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCbGroupname();
	afx_msg void OnSelchangeCbMaterialname();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CDialog				m_MaterialsList;
	MaterialDirectory	*RootMaterialDirectory;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GLOBALMATERIALS_H__9EA467C6_1E83_11D3_95F4_004033AA0439__INCLUDED_)
