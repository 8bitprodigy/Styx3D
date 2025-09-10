/****************************************************************************************/
/*  TIMELINETOOLBAR.CPP                                                                 */
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
#include "stdafx.h"
#include "jwe.h"
#include "TimeLineToolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TimeLineToolBar dialog


TimeLineToolBar::TimeLineToolBar(CWnd* pParent /*=NULL*/)
	: CDialog(TimeLineToolBar::IDD, pParent)
{
	//{{AFX_DATA_INIT(TimeLineToolBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void TimeLineToolBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TimeLineToolBar)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TimeLineToolBar, CDialog)
	//{{AFX_MSG_MAP(TimeLineToolBar)
	ON_BN_CLICKED(IDC_TIMELINE_DELETE, OnTimelineDelete)
	ON_EN_CHANGE(IDC_TIMELINE_EVENTTEXT, OnChangeTimelineEventtext)
	ON_BN_CLICKED(IDC_TIMELINE_INSERT, OnTimelineInsert)
	ON_BN_CLICKED(IDC_TIMELINE_PLAY, OnTimelinePlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TimeLineToolBar message handlers

/*
BOOL TimeLineToolBar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL TimeLineToolBar::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	::SendMessage(this->GetParent()->m_hWnd, this->GetCurrentMessage()->message, wParam, lParam);

	return CDialog::OnCommand(wParam, lParam);
}

void TimeLineToolBar::OnTimelineDelete() 
{
	// TODO: Add your control notification handler code here
	
}

void TimeLineToolBar::OnChangeTimelineEventtext() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void TimeLineToolBar::OnTimelineInsert() 
{
	// TODO: Add your control notification handler code here
	
}

void TimeLineToolBar::OnTimelinePlay() 
{
	// TODO: Add your control notification handler code here
	
}
*/