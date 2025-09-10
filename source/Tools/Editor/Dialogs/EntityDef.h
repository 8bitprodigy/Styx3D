/****************************************************************************************/
/*  ENTITYDEF.H                                                                         */
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
#if !defined(AFX_ENTITYDEF_H__991D2D21_F3FC_11D2_8B42_00104B70D76D__INCLUDED_)
#define AFX_ENTITYDEF_H__991D2D21_F3FC_11D2_8B42_00104B70D76D__INCLUDED_

#include "Symbol.h"	// Added by ClassView
#include "Basetype.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CEntityDef dialog

class CEntityDef : public CDialog
{
// Construction
public:
	static jeBoolean UpdateDefinitionsCB( jeSymbol * pSymbol, void * lParam );
	jeSymbol_Table * m_pEntities;
	jeSymbol_Type m_SymbolType;
	void ShowFieldsBySymbolType( jeSymbol_Type Type );
	CEntityDef(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEntityDef)
	enum { IDD = IDD_DEFINEENTITIES };
	CComboBox	m_DefEnumCB;
	CListBox	m_Properties;
	CComboBox	m_FieldTypeCB;
	CComboBox	m_EntitiesCB;
	CString	m_csDefString;
	CString	m_csDef1;
	CString	m_csDef2;
	CString	m_csDef3;
	CString	m_csFieldName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEntityDef)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEntityDef)
	virtual BOOL OnInitDialog();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnSelchangeCbFieldtype();
	afx_msg void OnSelchangeCbEntities();
	afx_msg void OnSetfocusEdFieldname();
	afx_msg void OnSelchangeLbProperties();
	afx_msg void OnBnRemovefield();
	afx_msg void OnBnAddfield();
	afx_msg void OnBnApply();
	afx_msg void OnBnNew();
	afx_msg void OnBnDelete();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	jeBoolean GetFieldData( jeSymbol_Type Type, char * pszDefaultValue );
	void SetFields( jeSymbol * pField );
	void FillProperties( void );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTITYDEF_H__991D2D21_F3FC_11D2_8B42_00104B70D76D__INCLUDED_)
