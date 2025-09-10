/****************************************************************************************/
/*  BUILDEREDIT.H                                                                       */
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
#if !defined(AFX_BUILDEREDIT_H__95E5FA20_1FF2_11D3_B323_004033AA0441__INCLUDED_)
#define AFX_BUILDEREDIT_H__95E5FA20_1FF2_11D3_B323_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBuilderEdit window

class CBuilderEdit : public CEdit
{
// Construction
public:
	CBuilderEdit(int DataType = 0, int DataId = 0);
	void SetProperty();

// Attributes
public:

// Operations
public:
	void Increment( BOOL bDown );
	void SetValueRange( float min, float max, float increment );
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBuilderEdit)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBuilderEdit();

	// Generated message map functions
protected:
	DECLARE_DYNCREATE(CBuilderEdit)
	//{{AFX_MSG(CBuilderEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	int m_DataType;
	int m_DataId;
	float m_Min;
	float m_Max;
	float m_Increment;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUILDEREDIT_H__95E5FA20_1FF2_11D3_B323_004033AA0441__INCLUDED_)
