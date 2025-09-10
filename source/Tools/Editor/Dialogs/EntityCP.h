/****************************************************************************************/
/*  ENTITYCP.H                                                                          */
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
#if !defined(AFX_ENTITYCP_H__2E1DB562_C4FB_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_ENTITYCP_H__2E1DB562_C4FB_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ColorBtn.h"
#include "ParEdit.h"
#include "Symbol.h"
#include "Basetype.h"	// Added by ClassView

typedef struct tagFieldInfo
{
	jeSymbol	*	pField ;
	union
	{
		jeFloat		Float ;
		jeVec3d		Vec3d ;
		int			Integer ;
		JE_RGBA		Color ;
		char	*	pChar ;
		void	*	pVoid ;
	} Value ;
	jeSymbol_Type	Type ;
	jeBoolean		bInit ;
} FieldInfo ;

/////////////////////////////////////////////////////////////////////////////
// CEntityCP dialog

class CEntityCP : public CDialog
{
// Construction
public:
	jeBoolean GetField( FieldInfo * pfi );
	void Update( CJweDoc * pDoc );
	void SetCurrentDocument( jeSymbol_Table * pEntities );
	jeSymbol_Table * m_pEntities;
	void FillProperties( void );
	CEntityCP(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEntityCP)
	enum { IDD = IDD_ENTITIES };
	CColorButton m_bnColor;
	CColorButton m_bnOrthoColor;
	CComboBox	m_DefEnumCB;
	CListBox	m_Properties;
	CString	m_csDefString;
	CString	m_csDef1;
	CString	m_csDef2;
	CString	m_csDef3;
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
	//{{AFX_VIRTUAL(CEntityCP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	BOOL NeedTextNotify( UINT id, NMHDR * pTTTStruct, LRESULT * pResult ) ;
	int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

// Implementation
protected:
	LRESULT OnChangeColor( WPARAM wParam, LPARAM lParam ) ;
	// Generated message map functions
	//{{AFX_MSG(CEntityCP)
	virtual BOOL OnInitDialog();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
	afx_msg void OnSelchangeLbProperties();
	afx_msg void OnBnApply();
	afx_msg void OnKillfocusEdName();
	afx_msg void OnToolsPlaceterrain();
	afx_msg void OnToolsPlacestaircase();
	afx_msg void OnToolsPlacespheroid();
	afx_msg void OnToolsPlacesheet();
	afx_msg void OnToolsPlacelight();
	afx_msg void OnToolsPlaceentity();
	afx_msg void OnToolsPlacecylinder();
	afx_msg void OnToolsPlacecube();
	afx_msg void OnToolsPlacecone();
	afx_msg void OnToolsPlacearch();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	mutable UINT m_UglyGlobalItemId;
private:
	static jeBoolean EnumFieldsCB( jeSymbol *pSymbol, void *lParam );
	void ShowFieldsBySymbolType( jeSymbol_Type Type ) ;
	void SetFields( const FieldInfo * pfi ) ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTITYCP_H__2E1DB562_C4FB_11D2_8B41_00104B70D76D__INCLUDED_)
