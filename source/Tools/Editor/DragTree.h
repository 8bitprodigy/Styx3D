/****************************************************************************************/
/*  DRAGTREE.H                                                                          */
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
#if !defined(AFX_DRAGTREE_H__32B34801_C588_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_DRAGTREE_H__32B34801_C588_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DragTree.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDragTree window

class CDragTree : public CTreeCtrl
{
// Construction
public:
	CDragTree();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDragTree)
	//}}AFX_VIRTUAL

// Implementation
public:
	bool Init( UINT nIDImageList );
	virtual ~CDragTree();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDragTree)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	CImageList m_ImageList;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAGTREE_H__32B34801_C588_11D2_8B41_00104B70D76D__INCLUDED_)
