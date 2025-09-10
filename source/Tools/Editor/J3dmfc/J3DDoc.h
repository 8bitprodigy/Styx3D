/****************************************************************************************/
/*  J3DDOC.H                                                                            */
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
#if !defined(AFX_J3DDOC_H__FF27C6E2_B43C_11D2_BE7E_00A0C96E625A__INCLUDED_)
#define AFX_J3DDOC_H__FF27C6E2_B43C_11D2_BE7E_00A0C96E625A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../include/jeWorld.h"

class CJ3DView;

/////////////////////////////////////////////////////////////////////////////
// CJ3DDoc document

class CJ3DDoc : public CDocument
{
protected:
	CJ3DDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNAMIC(CJ3DDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJ3DDoc)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	jeWorld* GetWorld(void) { return(m_pWorld); }
	virtual BOOL Render(CJ3DView* pView) = 0;
	virtual ~CJ3DDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CJ3DDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	jeWorld* m_pWorld;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_J3DDOC_H__FF27C6E2_B43C_11D2_BE7E_00A0C96E625A__INCLUDED_)
