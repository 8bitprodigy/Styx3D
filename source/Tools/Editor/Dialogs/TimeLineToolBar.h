/****************************************************************************************/
/*  TIMELINETOOLBAR.H                                                                   */
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
#if !defined(AFX_TIMELINETOOLBAR_H__9E5A7701_53F8_11D3_B681_004033AA0431__INCLUDED_)
#define AFX_TIMELINETOOLBAR_H__9E5A7701_53F8_11D3_B681_004033AA0431__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../resource.h"

/////////////////////////////////////////////////////////////////////////////
// TimeLineToolBar dialog

class TimeLineToolBar : public CDialog
{
// Construction
public:
	TimeLineToolBar(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(TimeLineToolBar)
	enum { IDD = IDD_TIMELINE_BAR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TimeLineToolBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TimeLineToolBar)
	afx_msg void OnTimelineDelete();
	afx_msg void OnChangeTimelineEventtext();
	afx_msg void OnTimelineInsert();
	afx_msg void OnTimelinePlay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMELINETOOLBAR_H__9E5A7701_53F8_11D3_B681_004033AA0431__INCLUDED_)
