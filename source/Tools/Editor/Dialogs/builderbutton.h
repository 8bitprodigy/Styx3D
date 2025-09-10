/****************************************************************************************/
/*  BUILDERBUTTON.H                                                                     */
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
#if !defined(AFX_BUILDERBUTTON_H__95E5FA21_1FF2_11D3_B323_004033AA0441__INCLUDED_)
#define AFX_BUILDERBUTTON_H__95E5FA21_1FF2_11D3_B323_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// builderbutton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBuilderButton window

class CBuilderButton : public CButton
{
// Construction
public:
	CBuilderButton( int DataType, int DataId);
	void SetColor( COLORREF Color);
	void DrawColorButton(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void DrawGroupButton(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBuilderButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBuilderButton();
	void	SetState(UINT nFlags);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBuilderButton)
	afx_msg void OnClicked();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	int m_DataType;
	int m_DataId;
	int	m_state;
	COLORREF mCurrentColor;

	HICON hIconRight;
	HICON hIconDown;


};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUILDERBUTTON_H__95E5FA21_1FF2_11D3_B323_004033AA0441__INCLUDED_)
