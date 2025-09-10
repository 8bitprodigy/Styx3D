/****************************************************************************************/
/*  TIMELINE.H                                                                          */
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
#if !defined(AFX_TIMELINE_H__B55E05E0_509A_11D3_B323_004033AA0441__INCLUDED_)
#define AFX_TIMELINE_H__B55E05E0_509A_11D3_B323_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTimeLine dialog
#include "jeProperty.h"

#include "basetype.h"

#include "BtnST.h"

typedef struct TimeEdit TimeEdit;

#define MAX_KEYS 512
#define MAX_TIME_LINES 64

typedef union ChannelData
{
	char		*String;
	jeXForm3d	XForm;
}ChannelData;

typedef struct Channel
{
	jeBoolean	Disabled;
	ChannelData	KeyData[MAX_KEYS];
	float		KeyList[MAX_KEYS];	// list of key times
	int			KeyCount;
	int 		KeysSelected[MAX_KEYS];	// need to allow for selection of muliple keys
}Channel;

class CTimeLine : public CDialog
{
// Construction
public:
	CTimeLine(CWnd* pParent = NULL);   // standard constructor
	~CTimeLine(); 

	BOOL BuildFromDescriptor( jeProperty_List *pArray );
	void Reset();
	BOOL UpdateDataByArray( jeProperty_List *pArray );

	jeBoolean SubSelectObject(jeObject *pObj);
	jeBoolean SubSelectEndMove(jeObject *pObj);
	jeBoolean SubSelectEndRotate(jeObject *pObj);
	jeBoolean UpdateTimeDelta(float TimeDelta );

	void SetEventProperties();
	void GetEventProperties();

// Dialog Data
	//{{AFX_DATA(CTimeLine)
	enum { IDD = IDD_TIMELINE };
	CButton	m_InsertCut;
	CButton	m_PlayMode;
	int		m_PlayModeValue;
	CComboBox	m_ObjectName;
	CComboBox	m_ObjectMessage;
	CEdit		m_EventText;
	CStatic		m_Static1;
	CStatic		m_Static2;
	CStatic		m_Static3;
	CStatic		m_CLR;
	
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeLine)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTimeLine)

	virtual BOOL OnInitDialog();
	afx_msg void OnChangeTimelineEventtext();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimelineZoomIn();
	afx_msg void OnTimelineZoomOut();
	afx_msg void OnTimelineReset();
	afx_msg void OnClose();
	afx_msg void OnTimelineMod();
	afx_msg void OnTimelineMode();
	afx_msg void OnTimelineCut();
	afx_msg void OnTimelineCutPoint();
	afx_msg void OnMoveAll();
	afx_msg void OnSMS();
	afx_msg void OnHHMMSS();
	afx_msg void OnCLR();
	afx_msg void OnActivate( UINT, CWnd*, BOOL );
	afx_msg void OnSelchangeObjectName();
	afx_msg void OnSelchangeMessageId();

		//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HWND m_hWnd;
	BOOL HasTimeLineData( jeProperty_List *pArray );
	TimeEdit *m_TE;
	void TimelineModeSet(int);
	void OnSetObjectNames();
	

	CButtonST	bZoomIn;
	CButtonST	bZoomOut;
	CButtonST	bPlay;
	CButtonST	bInsertCut;
	CButtonST	bReset;
	CButtonST	bMoveAll;

	CButtonST	bSMS;
	CButtonST	bHHMMSS;
	CButtonST	bCLR;



	//TimeLineToolBar ToolBar;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMELINE_H__B55E05E0_509A_11D3_B323_004033AA0441__INCLUDED_)
