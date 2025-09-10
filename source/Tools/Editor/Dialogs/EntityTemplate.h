/****************************************************************************************/
/*  ENTITYTEMPLATE.H                                                                    */
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
#if !defined(AFX_ENTITYTEMPLATE_H__817D0941_F890_11D2_8B42_00104B70D76D__INCLUDED_)
#define AFX_ENTITYTEMPLATE_H__817D0941_F890_11D2_8B42_00104B70D76D__INCLUDED_

#include "Basetype.h"	// Added by ClassView
#include "ColorBtn.h"
#include "ParEdit.h"
#include "Symbol.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CEntityTemplate dialog

class CEntityTemplate : public CDialog
{
// Construction
public:
	DECLARE_DYNCREATE(CEntityTemplate)
	void SetCurrentDocument( jeSymbol_Table * pEntities, const char * pszType );
	CEntityTemplate(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEntityTemplate)
	enum { IDD = IDD_TEMPLATEENTITY };
	CColorButton m_bnColor;
	CComboBox	m_FieldTypeCB;
	CComboBox	m_DefEnumCB;
	CListBox	m_Properties;
	CComboBox	m_EntitiesCB;
	CString	m_csDefString;
	CString	m_csDef1;
	CString	m_csDef2;
	CString	m_csDef3;
	CString	m_csFieldName;
	CString	m_csName;
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
	//{{AFX_VIRTUAL(CEntityTemplate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	LRESULT Update( WPARAM wParam, LPARAM lParam ) ;
	LRESULT OnChangeColor( WPARAM wParam, LPARAM lParam ) ;
	// Generated message map functions
	//{{AFX_MSG(CEntityTemplate)
	virtual BOOL OnInitDialog();
	afx_msg void OnToolsPlacecube();
	afx_msg void OnToolsPlacelight();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnSelchangeCbFieldtype();
	afx_msg void OnSelchangeCbEntities();
	afx_msg void OnSelchangeLbProperties();
	afx_msg void OnSetfocusEdFieldname();
	afx_msg void OnBnAddfield();
	afx_msg void OnBnRemovefield();
	afx_msg void OnBnApply();
	afx_msg void OnBnNew();
	afx_msg void OnBnDelete();
	afx_msg void OnToolsPlaceterrain();
	afx_msg void OnKillfocusEdName();
	afx_msg void OnToolsPlacespheroid();
	afx_msg void OnToolsPlaceentity();
	afx_msg void OnToolsAddbrush();
	afx_msg void OnToolsPlacecylinder();
	afx_msg void OnAdd();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnToolsPlacesheet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void FillProperties( void );
	static jeBoolean FillEntityTypesCB( jeSymbol *pSymbol, void *lParam );
	void FillEntityTypes( jeSymbol *pSelect );
	void ShowFieldsBySymbolType(jeSymbol_Type Type ) ;
	void SetFields(jeSymbol *pField) ;
	jeBoolean GetFieldData(jeSymbol_Type Type, char *pszDefaultValue) ;
	jeSymbol_Table *	m_pEntities;
	jeSymbol_Type		m_SymbolType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTITYTEMPLATE_H__817D0941_F890_11D2_8B42_00104B70D76D__INCLUDED_)
