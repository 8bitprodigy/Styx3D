/****************************************************************************************/
/*  TIMELINE.CPP                                                                        */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Timeline editor for motions								            */
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
#define	WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "errorlog.h"
#include "ram.h"
#include "jeProperty.h"

#include "jwe.h"
#include "timeline.h"
#include "object.h"
#include "objectlist.h"
#include "ObjectMsg.h"
#include "Util.h"

#include "MainFrm.h"
#include "doc.h"



/*
												| BORDERY %
					*							+ _MARK %			^
		K		K	|		K		K			+ _KEY %			|
								|				+ _TIC %			|
		+----------------------------			0					|SizeY	
								|				- _TIC %			|
			N		|	N						- _KEY %			|
					*							- _MARK %			v  
												| BORDERY %
<		> BORDERX %

		^ + is PositionX,PositionY
		<        SizeX				>
*/

#ifndef JET_20
#define JE_ERR_WINDOWS_API_FAILURE 99
#define JE_ERR_SUBSYSTEM_FAILURE   98
#define jeErrorLog_IntToString(xx) NULL
#endif

#define TIMELINE_CUT_THRESHOLD 0.0001f
#define ZOOM_MIN	1.0f
#define ZOOM_MAX	25000.0f
#define MIN_KEY_TIME 0.01f				// allows 100 Hz
#define MAX_KEY_TIME 99999.0f			// About 1 day(realtime) :)
#define HORIZ_SCROLL_MAX 64000
#define HORIZ_SCROLL_MIN_TIME 20

#define TIMEEDIT_BORDERX	5
#define TIMEEDIT_BORDERY	5

#define TIMEEDIT_BORDER_LEFT	60

static int TimeEdit_RegisterCount = 0;

//#define MAX_KEYS 3


#define LINESIZE			18
#define RULERSIZE			18
#define TITLESIZE			18

#define DUAL_KEY_YOFF			(-LINESIZE*4+LINESIZE/2)

#define ROT_KEY_ELLIPSE_YOFF	(DUAL_KEY_YOFF+LINESIZE-LINESIZE/2)
#define POS_KEY_ELLIPSE_YOFF	(DUAL_KEY_YOFF+LINESIZE*2-LINESIZE/2)
#define EVENT_ELLIPSE_YOFF		(DUAL_KEY_YOFF+LINESIZE*3-LINESIZE/2)

#define TIMELINE_TEXT_YOFF		8
#define TIMELINE_TICK_YOFF		(DUAL_KEY_YOFF+LINESIZE*4-LINESIZE/2)

#define ELLIPSE_SIZE			(LINESIZE/4)
#define ELLIPSE_SEL_SIZE		(LINESIZE/3)

#define TIMEBAR_TOP_SIZE		(LINESIZE*4)
#define TIMEBAR_BOT_SIZE		(LINESIZE+RULERSIZE)

#define TIMEBAR_HEIGHT (TIMEBAR_TOP_SIZE + TIMEBAR_BOT_SIZE)

#define	LINE_OFFSET (TIMEBAR_HEIGHT+TIMELINE_TEXT_YOFF*4)

#define TIMEBAR_START			110

typedef enum ChannelDef
{
CHANNEL_NULL = -1,
CHANNEL_POS = 0,
CHANNEL_ROT,
CHANNEL_EVENT,
MAX_CHANNELS,
};


int Yoff[MAX_CHANNELS] =		{POS_KEY_ELLIPSE_YOFF,	ROT_KEY_ELLIPSE_YOFF,	EVENT_ELLIPSE_YOFF};
float LimAmt[MAX_CHANNELS] =	{0.0f,					0.0f,					0.01f};



#define DRAG_TIMER 1
#define SCROLL_TIMER 2
#define DRAG_DEBOUCE 125

//#define MAX_TIME_LINES 64

typedef struct TimeData
{
	char		Descr[256];		// Description

	Channel		Channel[MAX_CHANNELS];
	int			ChannelPropID[MAX_CHANNELS];

	// time bar
	float		CurrTime;
	int			CurrTimeProp;
	jeBoolean	TimeBarSelected;

	float		TimeStart;			// start time of time line
	float		TimeEnd;			// end time of time line
}TimeData;

typedef struct TimeEdit
{
	HWND		hwnd;
	HINSTANCE	hinst;
	int			PositionX,PositionY;
	int			SizeX,SizeY;
	int			WindowAcross,WindowDown;

	int			TimeLineCount;
	TimeData	*TimeLine;
	TimeData	*SaveTimeLine;		// used to save the selected time line
	TimeData	TimeLineList[MAX_TIME_LINES];

	// scroll
	int			ScrollY;
	float		LastKeyTime;
	int			ScrollPageSize;
	int			ScrollTimer;
	int			DragTimer;

	int			ReselectedKeyChannel;

	// dragging vars
	float		TimeDragStartX;
	float		TimeDragStartY;
	float		TimeDragX;
	float		TimeDragY;
	int			DragStartX;
	int			DragStartY;
	int			DragX;
	int			DragY;
	jeBoolean   Dragging;

	// dragging vars (right Mousbutton)
	jeBoolean   rDragging;
	
	// When moving move all bars
	jeBoolean   MoveAll;
	// Display Timemode
	jeBoolean TimeMode;

	CTimeLine	*pCTimeLine;

} TimeEdit;

static jeBoolean	TimeEdit_Paint(TimeEdit *TE);
					
static jeBoolean	TimeEdit_AnimateSelected(TimeEdit *TE);
static jeBoolean	TimeEdit_Select(TimeEdit *TE, int MouseX, int MouseY);
static jeBoolean	TimeEdit_SelectTest(TimeEdit *TE, int, int, int MouseX, int MouseY, int Yoff);
static float		TimeEdit_XToTime(TimeEdit* TE, int MouseX);
static int			TimeEdit_TimeToX(TimeEdit* TE, double Time);

static jeBoolean	TimeEdit_MoveSelectedKeys(TimeEdit *TE, float Offset, int MouseY);
static jeBoolean	TimeEdit_DupSelectedKeys(TimeEdit *TE);
static jeBoolean	TimeEdit_SelectMultiTest(TimeEdit *TE, int MouseX, int MouseY);
static int			TimeEdit_CountSelectedKeys(TimeEdit *TE);

static jeBoolean	TimeEdit_AddKey(TimeEdit* TE, int MouseX, int MouseY);
static jeBoolean	TimeEdit_DelKey(TimeEdit* TE);

static jeBoolean	TimeEdit_AddEvent(TimeEdit* TE, int MouseX, int MouseY);
static jeBoolean	TimeEdit_AddEventTime(TimeEdit* TE, float Time);

static jeBoolean	TimeEdit_IsEventArea(TimeEdit *TE, int MouseX, int MouseY);
static int			TimeEdit_TimeToX(TimeEdit* TE, double Time);
static jeBoolean	TimeEdit_InsertTime(TimeEdit *TE);
static jeBoolean	TimeEdit_CursorInInsertArea(TimeEdit* TE, int MouseX, int MouseY);
static void			TimeEdit_SetScrollPos(TimeEdit *TE, float Time);
static void			Callback_GetIndividualTime(TimeEdit *TE, int TimeLineNdx, float *Time);
static jeBoolean TimeEdit_IsCutLeft(Channel *cp, int ndx);
static jeBoolean TimeEdit_IsCutRight(Channel *cp, int ndx);

HWND TimeEdit_CreateWindow(HWND hwndParent, int32 Width, int32 Height);
jeBoolean TimeEdit_GetValidTimeDataFromProperties(TimeEdit* TE, jeProperty_List *List, int StartNdx, TimeData *TD);
TimeEdit *TimeEdit_CreateTimeEditData(HWND hWnd);

void TimeEdit_SelectTimeLineFromIndex(TimeEdit *TE, int TimeLineNdx);
jeBoolean TimeEdit_SelectNewTimeLineFromMouse(TimeEdit *TE, int MouseY);
void Callback_GetCurrentTime(TimeData *TD, float *Time);
jeBoolean TimeEdit_GetPathObject(Object** op);

static jeBoolean	TimeEdit_SetEventReadOnly(TimeEdit* TE,jeBoolean ReadOnly);


void TimeEdit_QueryControllerForTimeLine(TimeEdit *TE);
jeBoolean TimeEdit_InitAddTimeLine(TimeEdit* TE, int NumChannels);
void Callback_GetKeyData(TimeEdit* TE, int ChannelNdx, int KeyNdx);
void Callback_ReturnTime(TimeData *TD);
void Callback_ReturnData(TimeData *TD, int ChannelNdx);
jeBoolean Callback_SetPlayMode(TimeEdit* TE, jeBoolean Play);
jeBoolean Callback_GetPlayMode(TimeEdit* TE, int *PlayMode);
void TimeEdit_SelectObject(TimeEdit *TE);
void TimeEdit_MarkObject(TimeEdit *TE, int TimeLineNdx);
jeObject *TimeEdit_GetObject(TimeEdit *TE, int TimeLineNdx);
static void TimeEdit_ZoomOut(TimeEdit* TE);
static void TimeEdit_ZoomIn(TimeEdit* TE);
static void TimeEdit_ZoomReset(TimeEdit* TE);

// Added JH
void TimeEdit_DrawLine (HDC hdc,int left, int top, int right, int bottom,DWORD Color1, DWORD Color2);
void TimeEdit_DrawRect (HDC hdc,int left, int top, int right, int bottom,DWORD Color1, int Type);



#define TIMEEDIT_WINDOW_CLASS "JDesignerTimeEditor"

// timeline.cpp : implementation file
//

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Bהההההה, Globals JH
HFONT hBigFont;
HFONT hSmallFont;
HFONT hMedFont;

/////////////////////////////////////////////////////////////////////////////
// CTimeLine dialog


CTimeLine::CTimeLine(CWnd* pParent /*=NULL*/)
	: CDialog(CTimeLine::IDD, pParent)
{
	CMainFrame			*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;	

	this->m_TE = NULL;

	//{{AFX_DATA_INIT(CTimeLine)
	m_PlayModeValue = FALSE;
	//}}AFX_DATA_INIT

	hMedFont = CreateFont(15, 0, 0, 0, 0, FALSE, FALSE, 0, ANSI_CHARSET, 
			  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
			  DEFAULT_PITCH|FF_SWISS,
			  __TEXT("Arial") );
	hSmallFont = CreateFont(12, 0, 0, 0, 0, FALSE, FALSE, 0, ANSI_CHARSET, 
			  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
			  DEFAULT_PITCH|FF_SWISS,
			  __TEXT("Arial") );

	hBigFont = CreateFont
			(25, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, 
			  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
			  DEFAULT_PITCH|FF_SWISS,
			  __TEXT("Arial") );

}

CTimeLine::~CTimeLine()
{
	DeleteObject (hMedFont);
	DeleteObject (hSmallFont);
	DeleteObject (hBigFont);
}

void CTimeLine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimeLine)
	DDX_Control(pDX, IDC_TIMELINE_CUT, m_InsertCut);
	DDX_Control(pDX, IDC_TIMELINE_MODE, m_PlayMode);
	DDX_Control(pDX, IDC_TIME_OBJECTNAME, m_ObjectName);
	DDX_Control(pDX, IDC_TIME_MESSAGEID , m_ObjectMessage);
	DDX_Control(pDX, IDC_TIMELINE_EVENTTEXT, m_EventText);
	DDX_Control(pDX, IDC_STATIC1, m_Static1);
	DDX_Control(pDX, IDC_STATIC2, m_Static2);
	DDX_Control(pDX, IDC_STATIC3, m_Static3);
	DDX_Control(pDX, IDC_STATIC3, m_Static3);
	DDX_Control(pDX, IDC_TIMELINE_CLR, m_CLR);
	DDX_Check  (pDX, IDC_TIMELINE_MODE, m_PlayModeValue);

	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimeLine, CDialog)
	//{{AFX_MSG_MAP(CTimeLine)
	ON_EN_CHANGE(IDC_TIMELINE_EVENTTEXT, OnChangeTimelineEventtext)
	ON_WM_SIZE()
//	ON_EN_KILLFOCUS(IDC_TIMELINE_EVENTTEXT, OnKillfocusTimelineEventtext)
	ON_BN_CLICKED(IDC_TIMELINE_ZOOM_IN, OnTimelineZoomIn)
	ON_BN_CLICKED(IDC_TIMELINE_ZOOM_OUT, OnTimelineZoomOut)
	ON_BN_CLICKED(IDC_TIMELINE_RESET, OnTimelineReset)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_TIMELINE_MODE, OnTimelineMode)
	ON_BN_CLICKED(IDC_TIMELINE_MOD, OnTimelineMod)
	ON_BN_CLICKED(IDC_TIMELINE_CUT, OnTimelineCut)
	ON_BN_CLICKED(IDC_TIMELINE_CUTPOINT, OnTimelineCutPoint)
	ON_BN_CLICKED(IDC_TIMELINE_MOVEALL, OnMoveAll)
	ON_BN_CLICKED(IDC_TIMELINE_SMS, OnSMS)
	ON_BN_CLICKED(IDC_TIMELINE_HHMMSS, OnHHMMSS)
	ON_BN_CLICKED(IDC_TIMELINE_CLR, OnCLR)
	
	ON_CBN_SELCHANGE (IDC_TIME_OBJECTNAME, OnSelchangeObjectName)
	ON_CBN_SELCHANGE (IDC_TIME_MESSAGEID, OnSelchangeMessageId)

	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CTimeLine::OnSetObjectNames()
{
	CMainFrame	*pMainFrm	= (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc		*pDoc		= pMainFrm->GetCurrentDocument() ;
	jeWorld		*World;
	jeObject	*CurrObject;
	const char	*Name;


	if (pDoc == NULL) return;

	World = pDoc->GetWorld();

	assert (World);

	// loop initializers
	CurrObject = NULL;

	m_ObjectName.ResetContent();

	while (TRUE)
	{
		CurrObject = jeWorld_GetNextObject(World, CurrObject);
		
		if (CurrObject == NULL)
			break;
		
		Name = jeObject_GetName( CurrObject );
		
		if (!Name) continue;
		m_ObjectName.AddString(Name);
	}

}


void CTimeLine::OnSelchangeObjectName()
{	
	SetEventProperties();
	::InvalidateRect(m_TE->hwnd, NULL, FALSE);
}


void CTimeLine::OnSelchangeMessageId()
{	SetEventProperties();
	::InvalidateRect(m_TE->hwnd, NULL, FALSE);
}

void CTimeLine::OnChangeTimelineEventtext() 
{	SetEventProperties();
	::InvalidateRect(m_TE->hwnd, NULL, FALSE);
}


jeBoolean TimeEdit_UpdateLastKeyTime(TimeEdit *TE, float NewTime)
	{
	if (NewTime >= 0.0f)
		{
		if (NewTime > TE->LastKeyTime)
			TE->LastKeyTime = NewTime;

		return JE_TRUE;
		}
	else
		{
		int i,c;
		int TimeLineNdx;
		Channel *cp;
		float LastTime = 0.0f;

		for (TimeLineNdx = 0; TimeLineNdx < TE->TimeLineCount; TimeLineNdx++)
			{
			for (c = 0; c < MAX_CHANNELS; c++)
				{
				cp = &TE->TimeLineList[TimeLineNdx].Channel[c];

				if (cp->Disabled) continue;

				for (i = 0; i < cp->KeyCount; i++)
					{	
					if (cp->KeyList[i] > LastTime)
						LastTime = cp->KeyList[i];
					}
				}
			}

		TE->LastKeyTime = LastTime;
		return JE_TRUE;
		}
	}

jeBoolean	CTimeLine::UpdateTimeDelta(float TimeDelta )
{
	float Time;
	int TimeLineNdx;
	jeBoolean PlayBack;
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;

	assert(m_TE);

	Callback_GetPlayMode(m_TE, &PlayBack);
	if (PlayBack == JE_FALSE)
		return JE_TRUE;

	pDoc->UpdateAllViews( NULL, DOC_HINT_ORTHO, NULL );

	//Callback_GetCurrentTime(m_TE->TimeLine, &Time);

	for (TimeLineNdx = 0; TimeLineNdx < m_TE->TimeLineCount; TimeLineNdx++)
	{
		jeFloat Len = m_TE->TimeLineList[TimeLineNdx].TimeEnd- m_TE->TimeLineList[TimeLineNdx].TimeStart;
		Callback_GetIndividualTime(m_TE, TimeLineNdx, &Time);
		m_TE->TimeLineList[TimeLineNdx].CurrTime = Time;


		if (Time-Len/2<0)
		{
			m_TE->TimeLineList[TimeLineNdx].TimeStart = 0;
			m_TE->TimeLineList[TimeLineNdx].TimeEnd   = Len;
		}
		else
		{ 
			m_TE->TimeLineList[TimeLineNdx].TimeStart = Time-Len/2;
			m_TE->TimeLineList[TimeLineNdx].TimeEnd   = Time+Len/2;
		}

//			Time

	/*	if (Time > m_TE->TimeLineList[TimeLineNdx].TimeEnd)
			{
			TimeData *ptr = m_TE->TimeLine;
			m_TE->TimeLine = m_TE->TimeLineList + TimeLineNdx;
			::SendMessage(m_TE->hwnd, WM_HSCROLL, MAKEWPARAM(SB_PAGERIGHT,0), 0);
			::SendMessage(m_TE->hwnd, WM_HSCROLL, MAKEWPARAM(SB_PAGERIGHT,0), 0);
			m_TE->TimeLine = ptr;
			}

		if (Time < m_TE->TimeLineList[TimeLineNdx].TimeStart)
			{
			TimeData *ptr = m_TE->TimeLine;
			m_TE->TimeLine = m_TE->TimeLineList + TimeLineNdx;
			::SendMessage(m_TE->hwnd, WM_HSCROLL, MAKEWPARAM(SB_PAGELEFT,0), 0);
			::SendMessage(m_TE->hwnd, WM_HSCROLL, MAKEWPARAM(SB_PAGELEFT,0), 0);
			m_TE->TimeLine = ptr;
			}*/
		}

	::InvalidateRect(m_TE->hwnd, NULL, FALSE);

	return JE_TRUE;
	TimeDelta;
}

jeBoolean CTimeLine::SubSelectObject(jeObject *pObj)
{
	int TimeLineNdx;

	assert(pObj);

	// select time line in correct position

	// send stop playback message if trying to edit
	Callback_SetPlayMode(m_TE, JE_FALSE);
	CTimeLine::TimelineModeSet(JE_FALSE);

	for (TimeLineNdx = 0; TimeLineNdx < m_TE->TimeLineCount; TimeLineNdx++)
		{
		if (TimeEdit_GetObject(m_TE, TimeLineNdx) == pObj)
			{
			//m_TE->TimeLine = &m_TE->TimeLineList[TimeLineNdx];
			TimeEdit_SelectTimeLineFromIndex(m_TE, TimeLineNdx);
			TimeEdit_SelectObject(m_TE);
			::InvalidateRect(m_TE->hwnd, NULL, FALSE);
			break;
			}
		}

	return JE_TRUE;
}

jeBoolean CTimeLine::SubSelectEndMove(jeObject *pObj)
{
	Channel *cp;

	assert(pObj);

	Callback_SetPlayMode(m_TE, JE_FALSE);
	CTimeLine::TimelineModeSet(JE_FALSE);

	cp = &m_TE->TimeLine->Channel[CHANNEL_POS];

	// send stop playback message if trying to edit
	if (cp->KeysSelected[0] >= 0 && cp->KeysSelected[1] == -1)
		{
		Callback_GetKeyData(m_TE, CHANNEL_POS, cp->KeysSelected[0]);
		}

	Callback_ReturnData(m_TE->TimeLine, CHANNEL_POS);

	// get the current XForm
	return JE_TRUE;
}

jeBoolean CTimeLine::SubSelectEndRotate(jeObject *pObj)
{
	Channel *cp;

	assert(pObj);

	// send stop playback message if trying to edit
	Callback_SetPlayMode(m_TE, JE_FALSE);
	CTimeLine::TimelineModeSet(JE_FALSE);

	cp = &m_TE->TimeLine->Channel[CHANNEL_ROT];

	// if single key is selected - not multiple!
	if (cp->KeysSelected[0] >= 0 && cp->KeysSelected[1] == -1)
		{
		Callback_GetKeyData(m_TE, CHANNEL_ROT, cp->KeysSelected[0]);
		}

	Callback_ReturnData(m_TE->TimeLine, CHANNEL_ROT);

	// get the current XForm
	return JE_TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTimeLine Property List handlers

BOOL CTimeLine::HasTimeLineData( jeProperty_List *pArray )
{
	int i;

	for( i = 0; i < pArray->jePropertyN; i++ )
		if( pArray->pjeProperty[i].Type == PROPERTY_TIME_GROUP_TYPE )
		{
#pragma message( "Validate Time line properties here")
			return TRUE;
		}
	return( FALSE );
}

static jeBoolean TimeEdit_SetEventReadOnly (TimeEdit* TE,jeBoolean ReadOnly)
{
	TE->pCTimeLine->m_EventText.ShowWindow(ReadOnly ? 0:1);
	TE->pCTimeLine->m_ObjectName.ShowWindow(ReadOnly? 0:1);
	TE->pCTimeLine->m_ObjectMessage.ShowWindow(ReadOnly? 0:1);
	TE->pCTimeLine->m_Static1.ShowWindow(ReadOnly? 0:1);
	TE->pCTimeLine->m_Static2.ShowWindow(ReadOnly? 0:1);
	TE->pCTimeLine->m_Static3.ShowWindow(ReadOnly? 0:1);
	TE->pCTimeLine->m_CLR.ShowWindow(ReadOnly? 0:1);

	return JE_TRUE;
}


jeBoolean TimeEdit_GetValidTimeDataFromProperties(TimeEdit* TE, jeProperty_List *List, int StartNdx, TimeData *TD)
{
	int i;
	jeBoolean ValidChannel = JE_FALSE, ValidCurrTime = JE_FALSE;

	for (i = StartNdx; i < List->jePropertyN; i++)
		{
		jeProperty *p = &List->pjeProperty[i];

		if (p->Type == PROPERTY_TIME_GROUP_TYPE)
			{
			// description from field name
			strcpy(TD->Descr, p->FieldName);
			}
		else
		if (p->Type == PROPERTY_GROUP_END_TYPE)
			{
			// stop looking
			break;
			}
		else
		if (p->Type == PROPERTY_CHANNEL_POS_TYPE)
			{
			TD->ChannelPropID[CHANNEL_POS] = p->DataId;
			memcpy(&TD->Channel[CHANNEL_POS], p->Data.Ptr, sizeof(Channel));
			ValidChannel = JE_TRUE;
			}
		else
		if (p->Type == PROPERTY_CHANNEL_ROT_TYPE)
			{
			TD->ChannelPropID[CHANNEL_ROT] = p->DataId;
			memcpy(&TD->Channel[CHANNEL_ROT], p->Data.Ptr, sizeof(Channel));
			}
		else
		if (p->Type == PROPERTY_CHANNEL_EVENT_TYPE)
			{
			TD->ChannelPropID[CHANNEL_EVENT] = p->DataId;
			memcpy(&TD->Channel[CHANNEL_EVENT], p->Data.Ptr, sizeof(Channel));
			}
		else
		if (p->Type == PROPERTY_CURTIME_TYPE)
			{
			TD->CurrTimeProp = p->DataId;
			TD->CurrTime = p->Data.Float;
			ValidCurrTime = JE_TRUE;
			}
		}

	return (ValidChannel && ValidCurrTime);
	TE;
}


BOOL CTimeLine::BuildFromDescriptor( jeProperty_List *List )
{
	int i;

	assert(List);

	if( !HasTimeLineData( List ) )
	{
		ShowWindow( SW_HIDE );
		return TRUE;
	}

	ShowWindow( SW_SHOW );

	{
	static Object *CurrPathObj;
	Object *op;
	jeBoolean PlayBack;

	Callback_GetPlayMode(m_TE, &PlayBack);

	TimeEdit_GetPathObject(&op);
	if (op != CurrPathObj)
		{
		CurrPathObj = op;
		CTimeLine::TimelineModeSet(PlayBack);
		}
	}

	TimeEdit_UpdateLastKeyTime(m_TE, -1.0f);

	m_TE->TimeLineCount = 0;
	for (i = 0; i < List->jePropertyN; i++)
		{
		jeProperty *p = &List->pjeProperty[i];

		if (p->Type == PROPERTY_TIME_GROUP_TYPE)
			{
			if (TimeEdit_GetValidTimeDataFromProperties(m_TE, List, i, &m_TE->TimeLineList[m_TE->TimeLineCount]))
				{
				TimeEdit_MarkObject(m_TE, m_TE->TimeLineCount);
				strcpy(m_TE->TimeLineList[m_TE->TimeLineCount].Descr, p->FieldName);
				//m_TE->TimeLineList[m_TE->TimeLineCount].TimeBarSelected = JE_FALSE; don't want this here
				m_TE->TimeLineCount++;
				}
			}
		}

	TimeEdit_QueryControllerForTimeLine(m_TE);

	::SetScrollRange(m_TE->hwnd, SB_VERT, 0, (m_TE->TimeLineCount * LINE_OFFSET), FALSE);
	::SetScrollRange(m_TE->hwnd, SB_HORZ, 0, 64000, FALSE);

	return( TRUE );
}

void CTimeLine::Reset()
{
	this->ShowWindow( SW_HIDE );
}

BOOL CTimeLine::UpdateDataByArray( jeProperty_List *pArray )
{
	CTimeLine::BuildFromDescriptor( pArray );
	return( TRUE );
}

void TimeEdit_SetTimeLineFromIndex(TimeEdit *TE, int TimeLineNdx)
{
	// this routine does not do SELECTION - it just sets the TimeLine and PositionY vars
	if (TimeLineNdx < 0)
		return;

	if (TimeLineNdx >= TE->TimeLineCount)
		return;

	// select current time line
	TE->TimeLine = &TE->TimeLineList[TimeLineNdx];
	// set Y postion of the current time line
	TE->PositionY = (TimeLineNdx) * LINE_OFFSET - TE->ScrollY + TIMEBAR_START;
}

void TimeEdit_SelectTimeLineFromIndex(TimeEdit *TE, int TimeLineNdx)
{
	// this routine does everything needed for SELECTION
	TimeEdit_SetTimeLineFromIndex(TE, TimeLineNdx);

	TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
}

jeBoolean TimeEdit_SelectNewTimeLineFromMouse(TimeEdit *TE, int MouseY)
{
	int TimeLineNdx;

	TimeLineNdx = ((MouseY + TE->ScrollY-(LINE_OFFSET-TIMEBAR_START)+TITLESIZE - 4) / LINE_OFFSET);

	if (TimeLineNdx == TE->TimeLine - TE->TimeLineList)
		return JE_FALSE;

	if (TimeLineNdx < 0)
		return JE_FALSE;

	if (TimeLineNdx >= TE->TimeLineCount)
		return JE_FALSE;

	TimeEdit_SelectTimeLineFromIndex(TE, TimeLineNdx);

	return JE_TRUE;
}

void TimeEdit_SetTimeLineFromCursor(TimeEdit *TE, int MouseY)
{
	int TimeLineNdx;

	TimeLineNdx = ((MouseY + TE->ScrollY-(LINE_OFFSET-TIMEBAR_START)+TITLESIZE - 4) / LINE_OFFSET);

	if (TimeLineNdx < 0)
		return;

	if (TimeLineNdx >= TE->TimeLineCount)
		return;

	// select current time line
	TE->TimeLine = &TE->TimeLineList[TimeLineNdx];
	// set Y postion of the current time line
	TE->PositionY = (TimeLineNdx) * LINE_OFFSET - TE->ScrollY + TIMEBAR_START;
}


void TimeEdit_SetScrollPos(TimeEdit *TE, float Time)
	{
	int pos;
	float LastTime;

	LastTime = __max(20, TE->LastKeyTime);
	pos = (int)((HORIZ_SCROLL_MAX/LastTime) * Time);

	SetScrollPos(TE->hwnd, SB_HORZ, pos, TRUE);
	InvalidateRect(TE->hwnd, NULL, FALSE);
	}

void TimeEdit_SetScrollPosValue(TimeEdit *TE, float Time)
{
	int i;
	float len;

	len = (TE->TimeLine->TimeEnd - TE->TimeLine->TimeStart);

	if (TE->MoveAll)
		{
		for (i=0;i<TE->TimeLineCount;i++)
		  {
			  len = (TE->TimeLineList[i].TimeEnd - TE->TimeLineList[i].TimeStart);
	  		  TE->TimeLineList[i].TimeStart -= Time;
			  TE->TimeLineList[i].TimeEnd    = TE->TimeLineList[i].TimeStart+len;
		  
			  if (TE->TimeLineList[i].TimeStart<0)
				{ TE->TimeLineList[i].TimeStart = 0;
				  TE->TimeLineList[i].TimeEnd   = len;
				}
			  if (TE->TimeLineList[i].TimeEnd>MAX_KEY_TIME)
				{ TE->TimeLineList[i].TimeStart = MAX_KEY_TIME-len;
				  TE->TimeLineList[i].TimeEnd   = MAX_KEY_TIME;
				}

			  if ( TE->LastKeyTime<TE->TimeLineList[i].TimeEnd)
					TimeEdit_UpdateLastKeyTime (TE,TE->TimeLineList[i].TimeEnd);
		  }
		}
	else
	  {
		TE->TimeLine->TimeStart -= Time;
		TE->TimeLine->TimeEnd    = TE->TimeLine->TimeStart+len;

		if (TE->TimeLine->TimeStart<0)
			{ TE->TimeLine->TimeStart = 0;
			  TE->TimeLine->TimeEnd   = len;
			}
		if (TE->TimeLine->TimeEnd>MAX_KEY_TIME)
			{ TE->TimeLine->TimeStart = MAX_KEY_TIME-len;
			  TE->TimeLine->TimeEnd   = MAX_KEY_TIME;
			}
		if ( TE->LastKeyTime<TE->TimeLine->TimeEnd)
			  TimeEdit_UpdateLastKeyTime (TE,TE->TimeLine->TimeEnd);
	  }
}

void TimeEdit_HorizScroll(TimeEdit *TE, int wParam)
	{
	float len = (TE->TimeLine->TimeEnd - TE->TimeLine->TimeStart);

	#define	HORIZ_PAGE_SCROLL_SCALE 0.5f
	#define	HORIZ_LINE_SCROLL_SCALE 0.1f
	#define HORIZ_SCROLL_MAX 64000

	SetScrollRange(TE->hwnd, SB_HORZ, 0, HORIZ_SCROLL_MAX, FALSE);

	switch(LOWORD(wParam))
		{
		case SB_PAGELEFT:
		case SB_LEFT:
			TimeEdit_SetScrollPosValue (TE,HORIZ_PAGE_SCROLL_SCALE);
			TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
			break;
		case SB_LINELEFT:
			TimeEdit_SetScrollPosValue (TE,HORIZ_LINE_SCROLL_SCALE);
			TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
			break;
		case SB_PAGERIGHT:
		case SB_RIGHT:
			TimeEdit_SetScrollPosValue (TE,-HORIZ_PAGE_SCROLL_SCALE);
			TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
			break;
		case SB_LINERIGHT:
			TimeEdit_SetScrollPosValue (TE,-HORIZ_LINE_SCROLL_SCALE);
			TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
			break;
		case SB_THUMBPOSITION:
			len = len;
		case SB_THUMBTRACK:
			{  
			//	by trilobite	Jan. 2011
			//static last_pos;
			static int last_pos;
			//
			static int pos;
			float LastTime = __max(20, TE->LastKeyTime);
			
			last_pos = pos;
			pos = HIWORD(wParam);

			if (pos == 0)
				{
				// insure that 0 is hit correctly

				TE->TimeLine->TimeStart = 0.0f;
				TE->TimeLine->TimeEnd = len;
				SetScrollPos(TE->hwnd, SB_HORZ, 0, TRUE);
				InvalidateRect(TE->hwnd, NULL, FALSE);
				}
			else
				{
				float ThisTime;
				float NextTimeStart;
				float PrevTimeStart;
				float scroll_amt;

				scroll_amt = ( HORIZ_LINE_SCROLL_SCALE);

				// determine where the pos would be on a SB_LINERIGHT
				NextTimeStart	= TE->TimeLine->TimeStart + scroll_amt;
				PrevTimeStart	= TE->TimeLine->TimeStart - scroll_amt;

				ThisTime = (LastTime/(float)HORIZ_SCROLL_MAX) * pos;

				LastTime;

			/*	if (ThisTime > NextTimeStart)
					{
					diff = ThisTime - NextTimeStart;
					mul =  (diff/scroll_amt);
*/
				//	TE->TimeLine->TimeStart = NextTimeStart + (scroll_amt * mul);
				//	TE->TimeLine->TimeEnd   = TE->TimeLine->TimeStart + len;
					TimeEdit_SetScrollPosValue (TE,-(ThisTime-NextTimeStart));

					TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
		/*			}*/
			/*	else
				if (ThisTime < PrevTimeStart)
					{
					diff = PrevTimeStart - ThisTime;
					mul = (diff/scroll_amt);

				//	TE->TimeLine->TimeStart = PrevTimeStart - (scroll_amt * mul);
				//	TE->TimeLine->TimeEnd = TE->TimeLine->TimeStart + len;

					TimeEdit_SetScrollPosValue (TE,-(-ThisTime-NextTimeStart));

					TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
					}*/
				}

			break;
			}
		}
	}

VOID CALLBACK TimeEdit_DragTimerProc(HWND hwnd, UINT iMsg, UINT iTimerID, DWORD dwTime)
{
	TimeEdit *TE;
	TE = (TimeEdit *)GetWindowLong(hwnd,0);

	assert(TE);

	// only allow dragging after small amount of time
	// otherwise its just a "selection" and not a drag

	TE->Dragging = JE_TRUE;
	if (TE->DragTimer)
		{
		KillTimer(hwnd, TE->DragTimer);
		TE->DragTimer = 0;
		}

	return;
	iMsg, iTimerID, dwTime;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Windows Procedure for TimeEdit

LRESULT CALLBACK TimeEdit_WndProc(HWND hWnd, UINT iMessage,
						 WPARAM wParam, LPARAM lParam)
{
	TimeEdit *TE;
	TE = (TimeEdit *)GetWindowLong(hWnd,0);
	if (TE == NULL)
		return DefWindowProc(hWnd, iMessage, wParam, lParam);

	switch	(iMessage)
	{
		case WM_QUIT:
		case WM_DESTROY:
			lParam = lParam;
			break;

		case WM_HSCROLL:
			{
			TimeEdit_HorizScroll(TE, wParam);
			break;
			}

		case WM_RBUTTONUP:
			{
				TE->rDragging = JE_FALSE;
				break;
			}
		case WM_RBUTTONDOWN:
			{
				TE->DragStartX = TE->DragX = LOWORD(lParam);  // horizontal position of cursor 
				TE->DragStartY = TE->DragY = HIWORD(lParam);  // vertical position 

				TE->rDragging = JE_TRUE;
				SetCursor(LoadCursor(TE->hinst, MAKEINTRESOURCE(IDC_HAND_OPEN)));
				break;
			}


		case WM_VSCROLL:
			{
			switch(LOWORD(wParam))
				{
				case SB_LINEDOWN:
					InvalidateRect(hWnd, NULL, FALSE);
					TE->ScrollY += 10;
					break;

				case SB_PAGEDOWN:
					InvalidateRect(hWnd, NULL, FALSE);
					TE->ScrollY += LINE_OFFSET;
					break;

				case SB_PAGEUP:
					InvalidateRect(hWnd, NULL, FALSE);
					TE->ScrollY -= LINE_OFFSET;
					if (TE->ScrollY < 0)
						TE->ScrollY = 0;
					break;

				case SB_LINEUP:
					InvalidateRect(hWnd, NULL, FALSE);
					TE->ScrollY -= 10;
					if (TE->ScrollY < 0)
						TE->ScrollY = 0;
					break;

				case SB_TOP:
					InvalidateRect(hWnd, NULL, FALSE);
					TE->ScrollY = 0;
					break;

				case SB_BOTTOM:
					InvalidateRect(hWnd, NULL, FALSE);
					TE->ScrollY = TE->TimeLineCount * LINE_OFFSET;
					break;

				case SB_THUMBPOSITION:
				case SB_THUMBTRACK:
					InvalidateRect(hWnd, NULL, FALSE);
					TE->ScrollY = HIWORD(wParam);
					break;
				}

			SetScrollPos(TE->hwnd, SB_VERT, TE->ScrollY, TRUE);

			break;
			}

		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_PRIOR:	// page up
					{
					InvalidateRect(hWnd, NULL, FALSE);
					TE->TimeLine--;

					if (TE->TimeLine < TE->TimeLineList)
						TE->TimeLine = TE->TimeLineList;

					break;
					}

				case VK_NEXT:	// page down
					{
					InvalidateRect(hWnd, NULL, FALSE);
					TE->TimeLine++;

					if (TE->TimeLine - TE->TimeLineList >= TE->TimeLineCount)
						TE->TimeLine = TE->TimeLineList + TE->TimeLineCount - 1;

					break;
					}

				case VK_HOME:
					{
					break;
					}

				case VK_ADD:
					{
					TimeEdit_ZoomOut(TE);
					break;
					}

				case VK_SUBTRACT:
					{
					TimeEdit_ZoomIn(TE);
					break;
					}

				case VK_INSERT :
					TimeEdit_InsertTime(TE);
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				case VK_DELETE :

					if (TimeEdit_DelKey(TE) == JE_FALSE)
						{
						jeErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_DelKey() failed.", NULL);
						break; // error occured
						}

					InvalidateRect(hWnd, NULL, FALSE);
					break;
			}
			break;
		}

		case WM_PAINT:
			{
				PAINTSTRUCT PaintStruct;
				BeginPaint(hWnd,&PaintStruct);

				TimeEdit_Paint(TE);

				EndPaint(hWnd,&PaintStruct);
				break;
			}

		case WM_MOUSEWHEEL:
			{
			int zDelta = (short) HIWORD(wParam)/WHEEL_DELTA;  
			int i;

			if (zDelta>0)
			for (i=0;i<zDelta;i++)
				TimeEdit_ZoomIn (TE);

			if (zDelta<0)
				for (i=0;i<(-zDelta);i++)
					TimeEdit_ZoomOut (TE);

				
			InvalidateRect(hWnd, NULL, FALSE);
			break;
			}

		case WM_MOUSEMOVE:
			{
			int MouseX, MouseY;
			
			
			MouseX = LOWORD(lParam); 
			MouseY = HIWORD(lParam);  


			if (wParam & MK_RBUTTON)
			if (TE->rDragging)
				{
				  double Start;
				  double Scale;
				  double len = ((double)TE->TimeLine->TimeEnd - (double)TE->TimeLine->TimeStart);

				  TE->DragX = MouseX;  
				  TE->DragY = MouseY;  
				  
				  SetCursor(LoadCursor(TE->hinst, MAKEINTRESOURCE(IDC_HAND_OPEN)));
				  Start=TimeEdit_XToTime (TE,TE->PositionX+10);
				  Scale=TimeEdit_XToTime (TE,TE->PositionX+11)-Start;

				  TimeEdit_SetScrollPosValue (TE,(float)(-(TE->DragStartX-TE->DragX)*Scale));

			      if (TE->MoveAll)
						 TimeEdit_SetScrollPos (TE,TE->TimeLine->TimeStart);
					else TimeEdit_SetScrollPos (TE,TE->TimeLineList[0].TimeStart);

				  TE->DragStartX = MouseX;  // horizontal position of cursor 
				  TE->DragStartY = MouseY;  // vertical position 

				  InvalidateRect(hWnd, NULL, FALSE);
				  break;
				}


			if (TimeEdit_CursorInInsertArea(TE, MouseX, MouseY))
				SetCursor(LoadCursor(TE->hinst, MAKEINTRESOURCE(IDC_CURSOR_PLUS)));

			if (wParam & MK_LBUTTON)
				{
				// set x,y for drag
				// the paint function will use these
				TE->DragX = MouseX;  
				TE->DragY = MouseY;  

				// HACK, there must be an constant for max +int
;				if (TE->DragX<30000)
				TE->TimeDragX = TimeEdit_XToTime (TE,TE->DragX);

				if (TimeEdit_AnimateSelected(TE) == JE_FALSE)
					{
					jeErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_AnimateSelected() failed.", NULL);
					break; //error
					}

				// let the paint function go
				InvalidateRect(hWnd, NULL, FALSE);
				}

	/*		if (TimeEdit_SelectNewTimeLineFromMouse(TE, MouseY))
				{
				TimeEdit_SelectObject(TE);
				if (!TimeEdit_CursorInInsertArea(TE, MouseX, MouseY))
					{
					break;
					}
				}
*/
			break;
			}

		case WM_LBUTTONUP:
			{
			int MouseX;
			int MouseY;

			MouseX = LOWORD(lParam);  // horizontal position of cursor 
			MouseY = HIWORD(lParam);  // vertical position 

			// only move if we were dragging
			if (TE->Dragging)
				{
				if (TE->DragStartX!= MouseX)
					{
					float	TimeDelta = TimeEdit_XToTime(TE,MouseX)-TE->TimeDragStartX;
					if (TimeEdit_MoveSelectedKeys(TE, TimeDelta, MouseY) == JE_FALSE)
						{
						jeErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_MoveSelectedKeys() failed.", NULL);
						break; //error
						}
					}
				}

			// this code is to handle un-selecting an already selected key
			if (!TE->Dragging || (TE->Dragging && abs(TE->DragX - MouseX) <= 1))
				{
				// this flag is set on LBUTTONDOWN in TimeEdit_Select()
				if (TE->ReselectedKeyChannel >= 0)
					{
					// clear selection key that has been reselected
					TE->TimeLine->Channel[TE->ReselectedKeyChannel].KeysSelected[0] = -1;
					Callback_ReturnData(TE->TimeLine, TE->ReselectedKeyChannel);
					}
				}

			// Turn Dragging off and kill the DragTimer
			TE->Dragging = JE_FALSE;
			ReleaseCapture();

			if (TE->DragTimer)
				{
				KillTimer(hWnd, TE->DragTimer);
				TE->DragTimer = 0;
				}
			if (TE->ScrollTimer)
				{
				KillTimer(hWnd, TE->ScrollTimer);
				TE->ScrollTimer = 0;
				}

			InvalidateRect(hWnd, NULL, FALSE);
			break;
			}

		case WM_TIMER:
			{
			switch (wParam)
				{
				case SCROLL_TIMER:
					{
					POINT point;
					GetCursorPos(&point);
					TE->DragX = point.x;
					if (TimeEdit_AnimateSelected(TE) == JE_FALSE)
						{
						jeErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_AnimateSelected() failed.", NULL);
						break; //error
						}
					break;
					}
				}
			break;
			}

		case WM_LBUTTONDOWN:
			{
			int MouseX;
			int MouseY;

			InvalidateRect(hWnd, NULL, FALSE);

			SetFocus(hWnd);

			TE->DragStartX = TE->DragX = MouseX = LOWORD(lParam);  // horizontal position of cursor 
			TE->DragStartY = TE->DragY = MouseY = HIWORD(lParam);  // vertical position 

			TE->TimeDragStartX	= TimeEdit_XToTime (TE,TE->DragStartX);
			TE->TimeDragX		= TimeEdit_XToTime (TE,TE->DragX);

			TE->TimeLine->TimeBarSelected = JE_FALSE;

			if (TimeEdit_SelectNewTimeLineFromMouse(TE, MouseY))
				{
				TimeEdit_SelectObject(TE);
				if (!TimeEdit_CursorInInsertArea(TE, MouseX, MouseY))
					{
					break;
					}
				break;
				}

			if (TimeEdit_CursorInInsertArea(TE, MouseX, MouseY))
				{
				if (TimeEdit_IsEventArea(TE, MouseX, MouseY) == JE_TRUE)
					{
					if (TimeEdit_AddEvent(TE, MouseX, MouseY) == JE_FALSE)
						{
						jeErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_AddEvent() failed.", NULL);
						break; //error
						}
					}
				else
					{
					if (TimeEdit_AddKey(TE, MouseX, MouseY) == JE_FALSE)
						{
						jeErrorLog_AddString(-1,"TimeEdit_WndProc: TimeEdit_AddKey() failed.", NULL);
						break; //error
						}
					}
				}
			else
				{
				// start drag timer determine whether this click will be a selection or a drag
				if (!TE->DragTimer)
					{
					TE->DragTimer = SetTimer(hWnd, DRAG_TIMER, DRAG_DEBOUCE, TimeEdit_DragTimerProc);
					if (TE->DragTimer == 0)
						{
						jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE,"TimeEdit_WndProc: SetTimer() failed.", jeErrorLog_IntToString(GetLastError()));
						}
					}

				SetCapture(TE->hwnd);

				if (wParam & MK_CONTROL)
					{
					if (TimeEdit_SelectMultiTest(TE, MouseX, MouseY) == JE_FALSE)
						break;
					}
				else if (wParam & MK_SHIFT)
					{
						TimeEdit_DupSelectedKeys (TE);
						if (!TE->DragTimer)
							{
							TE->DragTimer = SetTimer(hWnd, DRAG_TIMER, DRAG_DEBOUCE, TimeEdit_DragTimerProc);
							if (TE->DragTimer == 0)
								{
								jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE,"TimeEdit_WndProc: SetTimer() failed.", jeErrorLog_IntToString(GetLastError()));
								}
							}
					}	
				else
					{
						if (TimeEdit_Select(TE, MouseX, MouseY) == JE_FALSE)
							break;
					}
				}

			break;
			}

		default:
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Local Functions

static void TimeEdit_ZoomReset(TimeEdit* TE)
	{
	int i;

	// reset all Channels, all vars
	if (TE->MoveAll)
		{
		for (i=0;i<TE->TimeLineCount;i++)
		  {	TE->TimeLineList[i].TimeStart = 0.0f;
			TE->TimeLineList[i].TimeEnd	= 60.0f;
			TE->TimeLineList[i].CurrTime = 0.0f;
			Callback_ReturnTime(&TE->TimeLineList[i]);
		  }
		}
	// reset all vars
	else
	{		TE->TimeLine->TimeStart = 0.0f;
			TE->TimeLine->TimeEnd	= 60.0f;
			TE->TimeLine->CurrTime = 0.0f;
			Callback_ReturnTime(TE->TimeLine);
	}
	TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
	}

static void TimeEdit_ZoomOut(TimeEdit* TE)
{
	float len;
	int   i;

	len = TE->TimeLine->TimeEnd - TE->TimeLine->TimeStart;

	if (len > ZOOM_MAX) return;

	if (TE->MoveAll)
		{
		for (i=0;i<TE->TimeLineCount;i++)
			{ len = TE->TimeLineList[i].TimeEnd - TE->TimeLineList[i].TimeStart;

			  TE->TimeLineList[i].TimeStart -=  (TE->TimeLineList[i].CurrTime-TE->TimeLineList[i].TimeStart);
			  TE->TimeLineList[i].TimeEnd   +=  (TE->TimeLineList[i].TimeEnd-TE->TimeLineList[i].CurrTime);

			  if (TE->TimeLineList[i].TimeStart<0)
			  {
				TE->TimeLineList[i].TimeStart = 0;
				TE->TimeLineList[i].TimeEnd = len*2;
			  }
			  if (TE->TimeLineList[i].TimeEnd>MAX_KEY_TIME)
			  {
					TE->TimeLineList[i].TimeStart = MAX_KEY_TIME-len*2;
					TE->TimeLineList[i].TimeEnd = MAX_KEY_TIME;
			  }
	
			}
		}
	else
		{
		  len = TE->TimeLine->TimeEnd - TE->TimeLine->TimeStart;

		  TE->TimeLine->TimeStart -=  (TE->TimeLine->CurrTime-TE->TimeLine->TimeStart);
		  TE->TimeLine->TimeEnd   +=  (TE->TimeLine->TimeEnd-TE->TimeLine->CurrTime);


			  if (TE->TimeLine->TimeStart<0)
			  {
				TE->TimeLine->TimeStart = 0;
				TE->TimeLine->TimeEnd = len*2;
			  }
			  if (TE->TimeLine->TimeEnd>MAX_KEY_TIME)
			  {
					TE->TimeLine->TimeStart = MAX_KEY_TIME-len*2;
					TE->TimeLine->TimeEnd = MAX_KEY_TIME;
			  }
		}

	TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
}

static void TimeEdit_ZoomIn(TimeEdit* TE)
{
	float len;
	int   i;

	if (TE->MoveAll)
		{
		for (i=0;i<TE->TimeLineCount;i++)
			{
			len = TE->TimeLineList[i].TimeEnd - TE->TimeLineList[i].TimeStart;

			if (len < ZOOM_MIN)
				return;
			
			TE->TimeLineList[i].TimeStart +=  (TE->TimeLineList[i].CurrTime-TE->TimeLineList[i].TimeStart)/2;
			TE->TimeLineList[i].TimeEnd   -=  (TE->TimeLineList[i].TimeEnd-TE->TimeLineList[i].CurrTime)/2;
			
			if (TE->TimeLineList[i].TimeStart<0)
				{	
					TE->TimeLineList[i].TimeStart = 0;
					TE->TimeLineList[i].TimeEnd   =  TE->TimeLineList[i].CurrTime + len/2;
				}
			}
		}
	else
		{

		len = TE->TimeLine->TimeEnd - TE->TimeLine->TimeStart;

		if (len < ZOOM_MIN)
			return;

			TE->TimeLine->TimeStart +=  (TE->TimeLine->CurrTime-TE->TimeLine->TimeStart)/2;
			TE->TimeLine->TimeEnd   -=  (TE->TimeLine->TimeEnd-TE->TimeLine->CurrTime)/2;
		
		if (TE->TimeLine->TimeStart<0)
			{	
				TE->TimeLine->TimeStart = 0;
				TE->TimeLine->TimeEnd   = TE->TimeLine->CurrTime + len/2;
			}

		}

	TimeEdit_SetScrollPos(TE, TE->TimeLine->TimeStart);
}

static float TimeEdit_XToTime(TimeEdit* TE, int MouseX)
{
	int NewX;
	float NewTime;
	float TimeDiff;
	
	assert(TE);

	TimeDiff = TE->TimeLine->TimeEnd - TE->TimeLine->TimeStart;

	NewX = MouseX - TE->PositionX;
	NewTime = TE->TimeLine->TimeStart + (NewX * (TimeDiff/TE->SizeX));

	if (NewTime < 0)
		NewTime = 0;
	if (NewTime > TE->TimeLine->TimeEnd)
		NewTime = TE->TimeLine->TimeEnd;

	return NewTime;
}

static int TimeEdit_TimeToX(TimeEdit* TE, double Time)
{
	int PosX;
	double TimeDiff;
	
	assert(TE);

	TimeDiff = (double)TE->TimeLine->TimeEnd - (double)TE->TimeLine->TimeStart;

	PosX = TE->PositionX + (int)((double)TE->SizeX * (((double)Time - (double)TE->TimeLine->TimeStart)/(double)TimeDiff));

	return PosX;
}

static jeBoolean TimeEdit_SetPosition(TimeEdit *TE)
{
	RECT Rect;
	int LeftBorder;
	assert( TE != NULL );

	#define LEFT_BORDER 60
	#define RIGHT_BORDER 25
	#define TOP_BORDER 10
	#define BOTTOM_BORDER 10

	if (GetClientRect(TE->hwnd,&Rect)==0)
		{
			jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"TimeEdit_SetPosition:  GetWindowRect failed");
			return JE_FALSE;
		}
	TE->WindowAcross = (Rect.right - Rect.left);
	TE->WindowDown   = (Rect.bottom- Rect.top);

	LeftBorder = int(LEFT_BORDER * ((float)TE->WindowAcross/700.0f));

	TE->SizeX = TE->WindowAcross - LeftBorder - RIGHT_BORDER;
	TE->SizeY = TE->WindowDown - TOP_BORDER - BOTTOM_BORDER;

	TE->PositionX = Rect.left + LeftBorder;
	TE->PositionY = Rect.top + LINE_OFFSET;

	return JE_TRUE;
}

static jeBoolean TimeEdit_IsEventArea(TimeEdit *TE, int MouseX, int MouseY)
{
	assert(TE);

	return (MouseY > TE->PositionY+EVENT_ELLIPSE_YOFF);
	TE,MouseX,MouseY;
}

static jeBoolean TimeEdit_SelectTest(TimeEdit *TE, int ChannelNdx, int KeyNdx, int MouseX, int MouseY, int Yoff)
{
	int x_ul, y_ul, x_lr, y_lr;
	int PosX;
	int xsize, ysize;
	int xoff;
	Channel *cp;

	assert(TE);

	cp = &TE->TimeLine->Channel[ChannelNdx];

	PosX = TimeEdit_TimeToX(TE, cp->KeyList[KeyNdx]);

	xoff = 0;
	xsize = ysize = ELLIPSE_SEL_SIZE;

	if (TimeEdit_IsCutLeft(cp, KeyNdx))
		{
		xoff = -(int)(ELLIPSE_SIZE*1.5);
		xsize = (int)(ELLIPSE_SIZE*1.5f);
		ysize = ELLIPSE_SEL_SIZE;
		}
	else
	if (TimeEdit_IsCutRight(cp, KeyNdx))
		{
		xoff = (int)(ELLIPSE_SIZE*1.5);
		xsize = (int)(ELLIPSE_SIZE*1.5f);
		ysize = ELLIPSE_SEL_SIZE;
		}

	x_ul = xoff + PosX - xsize;
	y_ul = TE->PositionY - ysize + Yoff+LINESIZE/2;

	x_lr = xoff + PosX + xsize;
	y_lr = TE->PositionY + ysize + Yoff+LINESIZE/2;

	if (MouseX < x_ul)
		return JE_FALSE;
	if (MouseY < y_ul)
		return JE_FALSE;
	if (MouseX > x_lr)
		return JE_FALSE;
	if (MouseY > y_lr)
		return JE_FALSE;

	return JE_TRUE;
}


static jeBoolean TimeEdit_SelectMultiTest(TimeEdit *TE, int MouseX, int MouseY)
{
	int i,c;
	int s;
	jeBoolean Found;

	assert(TE);

	for (c = 0; c < MAX_CHANNELS; c++)
		{
		Channel *cp = &TE->TimeLine->Channel[c];

		if (cp->Disabled) continue;

		for (i = 0; i < cp->KeyCount; i++)
			{

			if (TimeEdit_SelectTest(TE, c,i, MouseX, MouseY, Yoff[c]) == JE_TRUE)
				{
				Found = JE_FALSE;
				for (s = 0; cp->KeysSelected[s] != -1; s++)
					{
					if (cp->KeysSelected[s] == i)
						Found = JE_TRUE;
					}
		
				if (!Found)
					{	cp->KeysSelected[s]   = i;
						cp->KeysSelected[s+1] = -1;
					}
				}
			}
		}

	if (TimeEdit_CountSelectedKeys(TE)==0)
			TimeEdit_SetEventReadOnly(TE,TRUE);
	if (TimeEdit_CountSelectedKeys(TE)>1)
			TimeEdit_SetEventReadOnly(TE,TRUE);

	return JE_TRUE;
}	

static jeBoolean TimeEdit_SelectTimeBarTest(TimeEdit *TE, int MouseX, int MouseY)
{
	int x_ul, y_ul, x_lr, y_lr;
	int PosX;

	assert(TE);

	PosX = TimeEdit_TimeToX(TE, TE->TimeLine->CurrTime);

	x_ul = PosX - 6;
	y_ul = TE->PositionY - (TIMEBAR_TOP_SIZE) - 6;

	x_lr = PosX + 6;
	y_lr = TE->PositionY - (TIMEBAR_TOP_SIZE) + 6;

	if (MouseX > x_ul && MouseX < x_lr && MouseY > y_ul && MouseY < y_lr)
		return JE_TRUE;

	x_ul = PosX - 6;
	y_ul = TE->PositionY + (TIMEBAR_BOT_SIZE) - 6;

	x_lr = PosX + 6;
	y_lr = TE->PositionY + (TIMEBAR_BOT_SIZE) + 6;

	if (MouseX > x_ul && MouseX < x_lr && MouseY > y_ul && MouseY < y_lr)
		return JE_TRUE;

	return JE_FALSE;
}

static jeBoolean TimeEdit_SelectTimeLineTest(TimeEdit *TE, int MouseX, int MouseY)
{
	int x_ul, y_ul, x_lr, y_lr;
	int PosX;

	assert(TE);

	PosX = TimeEdit_TimeToX(TE, TE->TimeLine->CurrTime);

	x_ul = TE->PositionX;
	y_ul = TE->PositionY - 4;

	x_lr = TE->PositionX + TE->SizeX;
	y_lr = TE->PositionY + 4;

	if (MouseX < x_ul)
		return JE_FALSE;
	if (MouseY < y_ul)
		return JE_FALSE;
	if (MouseX > x_lr)
		return JE_FALSE;
	if (MouseY > y_lr)
		return JE_FALSE;

	return JE_TRUE;
}

static jeBoolean TimeEdit_AnimateSelected(TimeEdit *TE)
{
	assert(TE);

	if (TE->TimeLine->TimeBarSelected)
		{
		TE->TimeLine->CurrTime = TimeEdit_XToTime(TE, TE->DragX);

		if (TE->TimeLine->CurrTime >= TE->TimeLine->TimeEnd)
			{
			SendMessage(TE->hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT,0), 0);
			TE->TimeLine->CurrTime = TE->TimeLine->TimeEnd;
			if (!TE->ScrollTimer)
				TE->ScrollTimer = SetTimer(TE->hwnd, SCROLL_TIMER, 100, NULL);
			}
		else
		if (TE->TimeLine->CurrTime <= TE->TimeLine->TimeStart)
			{
			SendMessage(TE->hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINELEFT,0), 0);
			TE->TimeLine->CurrTime = TE->TimeLine->TimeStart;
			if (!TE->ScrollTimer)
				TE->ScrollTimer = SetTimer(TE->hwnd, SCROLL_TIMER, 100, NULL);
			}
		else
			{
			if (TE->ScrollTimer)
				{
				KillTimer(TE->hwnd, TE->ScrollTimer);
				TE->ScrollTimer = 0;
				}
			}
		Callback_ReturnTime(TE->TimeLine);
		}

	else
	{
		//float MouseTime = TimeEdit_XToTime(TE, TE->DragX);
		
		if (TE->TimeDragX>= TE->TimeLine->TimeEnd)
			{
			SendMessage(TE->hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT,0), 0);
		//	TE->TimeLine->CurrTime = TE->TimeLine->TimeEnd;
			if (!TE->ScrollTimer)
				TE->ScrollTimer = SetTimer(TE->hwnd, SCROLL_TIMER, 100, NULL);
			}
		else
		if (TE->TimeDragX <= TE->TimeLine->TimeStart)
			{
			SendMessage(TE->hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINELEFT,0), 0);
	//		TE->TimeLine->CurrTime = TE->TimeLine->TimeStart;
			if (!TE->ScrollTimer)
				TE->ScrollTimer = SetTimer(TE->hwnd, SCROLL_TIMER, 100, NULL);
			}
		else
			{
			if (TE->ScrollTimer)
				{
				KillTimer(TE->hwnd, TE->ScrollTimer);
				TE->ScrollTimer = 0;
				}
			}
	}

	return JE_TRUE;
}

static jeBoolean TimeEdit_Select(TimeEdit *TE, int MouseX, int MouseY)
{
	int i,c;
	Channel *cp,*cp1;
	jeBoolean SomethingSelected = JE_FALSE;
	jeBoolean ChannelKeySelected[MAX_CHANNELS];

	assert(TE);

	TE->ReselectedKeyChannel = -1;
	

	for (c = 0; c < MAX_CHANNELS; c++)
		{
		cp = &TE->TimeLine->Channel[c];

		if (cp->Disabled) continue;

		//cp->KeysSelected[0] = -1;
		memset(ChannelKeySelected, 0, sizeof(ChannelKeySelected));

		for (i = 0; i < cp->KeyCount; i++)
			{	
			if (TimeEdit_SelectTest(TE, c, i, MouseX, MouseY, Yoff[c]) == JE_TRUE)
				{
				// select single key
				if (cp->KeysSelected[0] == i) // clicking an already selected key
					{
					TE->ReselectedKeyChannel = c;
					}
				else
					cp->KeysSelected[0] = i; // new key


				if (c == CHANNEL_EVENT)
					{
					TimeEdit_SetEventReadOnly(TE,FALSE);
					TE->pCTimeLine->GetEventProperties();
					}

				else TimeEdit_SetEventReadOnly(TE,TRUE);

				// pass the first key only
				SomethingSelected = JE_TRUE;
				ChannelKeySelected[c] = JE_TRUE;

				TimeEdit_SelectObject(TE);
				}
			}

		if (!ChannelKeySelected[c])
			{ cp->KeysSelected[0] = -1;
			}
		cp->KeysSelected[1] = -1;	// Deselect all other keys (JH)

		Callback_ReturnData(TE->TimeLine, c);
		}


	//
	// selects a pos and rot as a single key if they have the EXACT same time
	//
	if (!TE->TimeLine->Channel[CHANNEL_POS].Disabled && !TE->TimeLine->Channel[CHANNEL_ROT].Disabled)
		{
		int ChannelList[MAX_CHANNELS] = {CHANNEL_POS, CHANNEL_ROT, CHANNEL_POS};

		for (c = 0; c < MAX_CHANNELS-1; c++) // don't do the event channel
			{
			cp = &TE->TimeLine->Channel[ChannelList[c]];
			cp1 = &TE->TimeLine->Channel[ChannelList[c+1]]; // ChannelList is one larger than the maximum number of channels

			// compare rot and pos
			if (cp->KeysSelected[0] >= 0 && cp1->KeysSelected[0] == -1)
				{
				for (i = 0; i < cp1->KeyCount; i++)
					{	
					int ndx = cp->KeysSelected[0];

					if (cp1->KeyList[i] == cp->KeyList[ndx])
						{
						// select single key
						cp1->KeysSelected[0] = i;
						break;
						}
					}
				}

			Callback_ReturnData(TE->TimeLine, ChannelList[c+1]);
			}
		}

	TE->TimeLine->TimeBarSelected = JE_FALSE;

	// only try selecting the timebar or timeline if we aren't selecting a key/event
	if (SomethingSelected == JE_FALSE)
		{	
		if (TimeEdit_SelectTimeBarTest(TE, MouseX, MouseY))
			{
			TE->TimeLine->TimeBarSelected = JE_TRUE;
			Callback_ReturnTime(TE->TimeLine);
			TimeEdit_SelectObject(TE);
			}
		else
		if (TimeEdit_SelectTimeLineTest(TE, MouseX, MouseY))
			{
			TimeEdit_SelectObject(TE);
			}
		else
			{
			// move the current time
			Callback_ReturnTime(TE->TimeLine); // return SAME time so motion will get rebuilt
			TE->TimeLine->CurrTime = TimeEdit_XToTime(TE, MouseX);
			Callback_ReturnTime(TE->TimeLine); // return NEW time so motion will be sampled
			}
		}

	if (TimeEdit_CountSelectedKeys(TE)==0)
			TimeEdit_SetEventReadOnly(TE,TRUE);
	if (TimeEdit_CountSelectedKeys(TE)>1)
			TimeEdit_SetEventReadOnly(TE,TRUE);

	return JE_TRUE;
}


static void TimeEdit_SafeKeyTime(Channel* cp, float NewTime, jeBoolean IsCut, float *ReturnTime)
{
	int i;
	jeBoolean blocker;
	float start_time, end_time;

	blocker = JE_FALSE;

	start_time = NewTime;

	if (IsCut)
		end_time = NewTime + TIMELINE_CUT_THRESHOLD;
	else
		end_time = NewTime;

	for (i = 0; i < cp->KeyCount; i++)
		{
		if (TimeEdit_IsCutLeft(cp, i))
			{
			float st = cp->KeyList[i];
			float et = cp->KeyList[i+1];

			if ( ! (end_time < st || start_time > et &&
				fabs(st - end_time) >= MIN_KEY_TIME && fabs(start_time - et) >= MIN_KEY_TIME) )
				{
				blocker = JE_TRUE; // this time is blocked - try another one
				NewTime += MIN_KEY_TIME/4;
				TimeEdit_SafeKeyTime(cp, NewTime, IsCut, ReturnTime);
				// skip past cut right
				i++;
				break;
				}
			}
		else
		if ((cp->KeyList[i] >= start_time && cp->KeyList[i] <= end_time) ||
			fabs(cp->KeyList[i] - start_time) < MIN_KEY_TIME || fabs(cp->KeyList[i] - end_time) < MIN_KEY_TIME)
			{
			blocker = JE_TRUE; // this time is blocked - try another one
			NewTime += MIN_KEY_TIME/4;
			TimeEdit_SafeKeyTime(cp, NewTime, IsCut, ReturnTime);
			break;
			}
		}

	if (!blocker)
		{
		*ReturnTime = NewTime;
		}

	return;
}


static jeBoolean TimeEdit_CursorInInsertArea(TimeEdit* TE, int MouseX, int MouseY)
{
	int ChannelList[2] = {-1,-1};
	Channel *cp;
	int Count = 0;
	int i,k;
	int TimeLineNdx;
	int EventLineY,PosLineY,RotLineY,DualLineY;
	jeBoolean ret = JE_TRUE;

	assert(TE);

	TimeLineNdx = TE->TimeLine - TE->TimeLineList;
	// sets the time line
	TimeEdit_SetTimeLineFromCursor(TE, MouseY);

	if (MouseX < TE->PositionX - ELLIPSE_SEL_SIZE)
		{
		ret = JE_FALSE;
		goto end;
		}

	if (MouseX > (TE->PositionX + TE->SizeX) + ELLIPSE_SEL_SIZE)
		{
		ret = JE_FALSE;
		goto end;
		}

	if (TimeEdit_SelectTimeBarTest(TE, MouseX, MouseY))
		{
		ret = JE_FALSE;
		goto end;
		}

	EventLineY = TE->PositionY + EVENT_ELLIPSE_YOFF;
	PosLineY   = TE->PositionY + POS_KEY_ELLIPSE_YOFF;
	RotLineY   = TE->PositionY + ROT_KEY_ELLIPSE_YOFF;
	DualLineY  = TE->PositionY + DUAL_KEY_YOFF;

	if ((MouseY > PosLineY ) && (MouseY < EventLineY))
		{
		ChannelList[Count++] = CHANNEL_POS;
		}
	else
	if ((MouseY > RotLineY ) && (MouseY < PosLineY))
		{
		ChannelList[Count++] = CHANNEL_ROT;
		}
	else
	if ((MouseY > EventLineY) && (MouseY < EventLineY+ LINESIZE))
		{
		ChannelList[Count++] = CHANNEL_EVENT;
		}
	else
	if ((MouseY > DualLineY) && (MouseY <RotLineY))
		{
		ChannelList[Count++] = CHANNEL_POS;
		ChannelList[Count++] = CHANNEL_ROT;
		}

	if (Count == 0)
		{
		ret = JE_FALSE;
		}

	for (i = 0; i < Count; i++)
	{
		cp = &TE->TimeLine->Channel[ChannelList[i]];

		if (cp->Disabled || !ret)
			{
			ret = JE_FALSE;
			break;
			}

		for (k = 0; k < cp->KeyCount; k++)
			{	
			if (TimeEdit_SelectTest(TE, ChannelList[i], k, MouseX, MouseY, Yoff[ChannelList[i]]) == JE_TRUE)
				{
				ret = JE_FALSE;
				break;
				}
			}
	}

end:

	TimeEdit_SetTimeLineFromIndex(TE, TimeLineNdx);

	return ret;
}


static jeBoolean TimeEdit_AddKeyTime(TimeEdit* TE, int ChannelNdx, float Time)
{
	Channel *cp;

	cp = &TE->TimeLine->Channel[ChannelNdx];
	
	if (cp->Disabled) 
		return JE_TRUE;

	if (cp->KeyCount+2 >= MAX_KEYS)
		{
		jeErrorLog_AddString(-1,"TimeEdit_AddKey: Key limit hit.", NULL);
		return JE_FALSE;
		}

	Callback_GetKeyData(TE, ChannelNdx, cp->KeyCount);
	cp->KeyList[cp->KeyCount++] = Time;

	if (TE->pCTimeLine->m_InsertCut.GetCheck() == 1)
		{
		Callback_GetKeyData(TE, ChannelNdx, cp->KeyCount);
		cp->KeyList[cp->KeyCount++] = Time + (TIMELINE_CUT_THRESHOLD/2);
		}

	TimeEdit_SetEventReadOnly(TE,TRUE);
	return JE_TRUE;
}

static jeBoolean TimeEdit_MoveKeyTime(TimeEdit* TE, int ChannelNdx, int KeyNdx, float Time)
{
	Channel *cp;

	cp = &TE->TimeLine->Channel[ChannelNdx];
	
	if (cp->Disabled) 
		return JE_TRUE;

	if (TimeEdit_IsCutLeft(cp, KeyNdx))
		{
		// left
		cp->KeyList[KeyNdx] = Time;
		cp->KeyList[KeyNdx+1] = Time + (TIMELINE_CUT_THRESHOLD/2);
		}
	else
	if (TimeEdit_IsCutRight(cp, KeyNdx))
		{
		// right
		cp->KeyList[KeyNdx] = Time + (TIMELINE_CUT_THRESHOLD/2);
		cp->KeyList[KeyNdx-1] = Time;
		}
	else
		{
		cp->KeyList[KeyNdx] = Time;
		}

	return JE_TRUE;
}

void TimeEdit_SortChannel(Channel *cp, float *v, unsigned n)
	{
	unsigned i, j, ln, rn;
	
	float save_float;
	ChannelData save_keydata;
	ChannelData *d;
	
	// the correct solution would be to use the built in qsort function
	// rather than this hacked up qsort function - however
	// that would require a data structure change and touch a lot of
	// code.  Probably want to do that when shipping isn't a priority.

	#define swap(var1, var2, save) {save = var1; var1 = var2; var2 = save;}
	
	while (n > 1)
		{
		swap(v[0], v[n/2], save_float);
		d = &cp->KeyData[v - cp->KeyList];
		swap(d[0], d[n/2], save_keydata);
		for (i = 0, j = n; ; )
            {
			do
				--j;
			while (v[j] > v[0]);
			do
				++i;
			while (i < j && v[i] < v[0]);
			if (i >= j)
				break;
			swap(v[i], v[j], save_float);
			d = &cp->KeyData[v - cp->KeyList];
			swap(d[i], d[j], save_keydata);
            }
		swap(v[j], v[0], save_float);
		d = &cp->KeyData[v - cp->KeyList];
		swap(d[j], d[0], save_keydata);
		ln = j;
		rn = n - ++j;
		if (ln < rn)
            {
			TimeEdit_SortChannel(cp, v, ln);
			v += j;
			n = rn;
            }
		else
            {
			TimeEdit_SortChannel(cp, v + j, rn);
			n = ln;
            }
		}
	}

static jeBoolean TimeEdit_DelKeyTime(TimeEdit* TE, int ChannelNdx, int KeyNdx)
{
	Channel *cp;
	int CutNdx = -1;
	jeBoolean Sort = JE_FALSE;
	int i;

	cp = &TE->TimeLine->Channel[ChannelNdx];
	if (cp->Disabled) 
		return JE_TRUE;
	
	for (i = 0; cp->KeysSelected[i] != -1; i++)
		{
		 KeyNdx = cp->KeysSelected[i];

		 if (TimeEdit_IsCutLeft(cp, KeyNdx))
			{
			CutNdx = KeyNdx+1;
			}
		else
		if (TimeEdit_IsCutRight(cp, KeyNdx))
			{
			CutNdx = KeyNdx-1;
			}

		if (TimeEdit_IsCutRight(cp, cp->KeyCount-1))
			{
			Sort = JE_TRUE;
			}

		if (CutNdx >= 0)
			{
			cp->KeyList[CutNdx] = cp->KeyList[cp->KeyCount-1];
			cp->KeyData[CutNdx] = cp->KeyData[cp->KeyCount-1];
			cp->KeyCount--;
			}

		cp->KeyList[KeyNdx] = cp->KeyList[cp->KeyCount-1];
		cp->KeyData[KeyNdx] = cp->KeyData[cp->KeyCount-1];
		cp->KeyCount--;

		// if the last key is a cut key then the easiest thing to do is re-sort to maintain integrity of cuts
		if (Sort)
			{
			TimeEdit_SortChannel(cp, cp->KeyList, cp->KeyCount);
			}
	}
	return JE_TRUE;
}

static jeBoolean TimeEdit_AddKey(TimeEdit* TE, int MouseX, int MouseY)
{
	float Time, ReturnTime;
	int diff_pos, diff_rot, diff_both, diff_min;
	int ChannelEnabled[MAX_CHANNELS] = {JE_FALSE, JE_FALSE, JE_FALSE};
	Channel *cp;
	int i;
	jeBoolean ret = JE_TRUE;
	assert(TE);

	// add a new key
	Time = TimeEdit_XToTime(TE, MouseX);

	// find the line that we're closest too
	diff_pos = abs(MouseY - (TE->PositionY + POS_KEY_ELLIPSE_YOFF+LINESIZE/2));
	diff_rot = abs(MouseY - (TE->PositionY + ROT_KEY_ELLIPSE_YOFF+LINESIZE/2));
	diff_both = abs(MouseY - (TE->PositionY + DUAL_KEY_YOFF+LINESIZE/2));

	diff_min = __min(diff_pos, __min(diff_rot, diff_both));

	if (diff_min == diff_pos || diff_min == diff_both)
	{
		ChannelEnabled[CHANNEL_POS] = JE_TRUE;
	}

	if (diff_min == diff_rot || diff_min == diff_both)
	{
		ChannelEnabled[CHANNEL_ROT] = JE_TRUE;
	}

	for (i = 0; i < MAX_CHANNELS; i++)
	{
		cp = &TE->TimeLine->Channel[i];

		if (cp->Disabled)
			continue;

		cp->KeysSelected[0] = -1;

		if (ChannelEnabled[i])
			{

			TimeEdit_SafeKeyTime(cp, Time, TE->pCTimeLine->m_InsertCut.GetCheck(), &ReturnTime);

			ret = TimeEdit_AddKeyTime(TE, i, ReturnTime);

			TimeEdit_UpdateLastKeyTime(TE, ReturnTime);
			}

		Callback_ReturnData(TE->TimeLine, i);
	}

	TE->pCTimeLine->m_InsertCut.SetCheck(0);

	return ret;
}


static jeBoolean TimeEdit_DelKey(TimeEdit* TE)
{
	Channel *cp;
	int c;

	assert(TE);

	// need to extend this to delete multiple keys
	for (c = 0; c < MAX_CHANNELS; c++)
		{
		cp = &TE->TimeLine->Channel[c];

		if (cp->Disabled) continue;

		if (cp->KeysSelected[0] >= 0)
			{
			int ndx = cp->KeysSelected[0];

			if (cp->KeyList[ndx] == TE->LastKeyTime)
				TimeEdit_UpdateLastKeyTime(TE, -1.0f); // find another "LastKeyTime"

			TimeEdit_DelKeyTime(TE, c, ndx);

			// deselect keys
			cp->KeysSelected[0] = -1;

			Callback_ReturnData(TE->TimeLine, c);
			}
		}

	return JE_TRUE;
}

static jeBoolean TimeEdit_AddEventTime(TimeEdit* TE, float Time)
{	
	float ReturnTime;
	Channel *cp;

	assert(TE);

	// add a new Event
	cp = &TE->TimeLine->Channel[CHANNEL_EVENT];

	if (cp->Disabled) 
		return JE_TRUE;

	TimeEdit_SafeKeyTime(cp, Time, JE_FALSE, &ReturnTime);

	cp->KeyList[cp->KeyCount] = ReturnTime;

	TimeEdit_UpdateLastKeyTime(TE, ReturnTime);

	if (cp->KeyCount++ >= MAX_KEYS)
		{
		jeErrorLog_AddString(-1,"TimeEdit_InitAddEvent: Event limit hit.", NULL);
		return JE_FALSE;
		}

	Callback_ReturnData(TE->TimeLine, CHANNEL_EVENT);

	return JE_TRUE;

}

static jeBoolean TimeEdit_AddEvent(TimeEdit* TE, int MouseX, int MouseY)
{
	Channel *cp;
	jeFloat Time;
	int i;

	assert(TE);

	// add a new Event
	Time = TimeEdit_XToTime(TE, MouseX);
	
	TimeEdit_AddEventTime (TE,Time);

	cp = &TE->TimeLine->Channel[CHANNEL_EVENT];

	if (cp->Disabled) 
		return JE_TRUE;

	for (i = 0; i < MAX_CHANNELS-1; i++)
	{
		Channel *cpk = &TE->TimeLine->Channel[i];

		if (cpk->Disabled) continue;

		cpk->KeysSelected[0] = -1;
		Callback_ReturnData(TE->TimeLine, i);
	}

	cp->KeysSelected[0] = -1;


	return JE_TRUE;
	MouseY;
}


static int TimeEdit_CountSelectedKeys(TimeEdit *TE)
{
  int c,i,count=0;
  Channel *cp;

	for (c = 0; c < MAX_CHANNELS; c++)
	{
	cp = &TE->TimeLine->Channel[c];

	if (cp->Disabled) continue;

	for (i = 0; cp->KeysSelected[i] != -1; i++)
			count ++;
	}

  return count;
}


static jeBoolean TimeEdit_MoveSelectedKeys(TimeEdit *TE, float Offset, int MouseY)
{
	int i,c;
	float Time,MouseAtTime=-1,ReturnTime;
	Channel *cp;

	
	assert(TE);
	
//	TimeAdd = TimeEdit_XToTime(TE, 200+abs(Offset))-TimeEdit_XToTime(TE, 200);
//	if (Offset <0) TimeAdd *=-1;
		
	for (c = 0; c < MAX_CHANNELS; c++)
		{
		cp = &TE->TimeLine->Channel[c];

		if (cp->Disabled) continue;

		for (i = 0; cp->KeysSelected[i] != -1; i++)
			{
			int ndx = cp->KeysSelected[i];

			Time = cp->KeyList[ndx];

			// Move relative
			MouseAtTime =  Time + Offset;

			if (Time == MouseAtTime)
				continue;

			if (MouseAtTime < LimAmt[c])
				MouseAtTime = LimAmt[c];

		/*	if (MouseAtTime > TE->TimeLine->TimeEnd)
				MouseAtTime = TE->TimeLine->TimeEnd;*/

			if (TimeEdit_IsCutRight(cp, ndx))
				{
				TimeEdit_SafeKeyTime(cp, MouseAtTime, JE_TRUE, &ReturnTime);
				TimeEdit_MoveKeyTime(TE, c, ndx-1, ReturnTime);
				}
			else
				{
				TimeEdit_SafeKeyTime(cp, MouseAtTime, TimeEdit_IsCutLeft(cp, ndx), &ReturnTime);
				TimeEdit_MoveKeyTime(TE, c, ndx, ReturnTime);
				}


			if (Time == TE->LastKeyTime) // moving the last key
				{
				TimeEdit_UpdateLastKeyTime(TE, -1.0f); // find another "LastKeyTime"
				}
			else
				{
				// not moving the last key - see if this is now the last key
				TimeEdit_UpdateLastKeyTime(TE, ReturnTime); // find another "LastKeyTime"
				}

			if (TE->TimeLine->TimeStart>MouseAtTime)
			if (TE->TimeLine->CurrTime>MouseAtTime)
				TE->TimeLine->CurrTime=MouseAtTime;
				
			if (TE->TimeLine->TimeEnd<MouseAtTime)
			if (TE->TimeLine->CurrTime<MouseAtTime)
				TE->TimeLine->CurrTime=MouseAtTime;
			// unselect the key
		/*	cp->KeysSelected[i] = -1;*/
			}

		Callback_ReturnData(TE->TimeLine, c);
		}


	Callback_ReturnTime(TE->TimeLine);

	return JE_TRUE;
	MouseY;
}

static jeBoolean TimeEdit_DupSelectedKeys(TimeEdit *TE)
{
	int i,c;
	Channel *cp;

	
	assert(TE);
		
	for (c = 0; c < MAX_CHANNELS; c++)
		{
		cp = &TE->TimeLine->Channel[c];

		if (cp->Disabled) continue;
	
		for (i = 0; cp->KeysSelected[i] != -1; i++)
			{
			int ndx = cp->KeysSelected[i];
			int	  ret;
			float ReturnTime;
			float time = cp->KeyList[ndx];
			

			if (c==CHANNEL_EVENT)
				{
					TimeEdit_SafeKeyTime  (cp, time, TE->pCTimeLine->m_InsertCut.GetCheck(), &ReturnTime);
					TimeEdit_AddEventTime (TE,ReturnTime);

					if (cp->KeyData[ndx].String!=NULL)
						{ 
						  cp->KeyData[cp->KeyCount-1].String=Util_StrDup(cp->KeyData[ndx].String);
						}
				}
			else
				{
					TimeEdit_SafeKeyTime(cp, time, TE->pCTimeLine->m_InsertCut.GetCheck(), &ReturnTime);
		
					ret = TimeEdit_AddKeyTime (TE,c,ReturnTime);
					
					TE->TimeLine->Channel[c].KeyData[cp->KeyCount-1].XForm = TE->TimeLine->Channel[c].KeyData[ndx].XForm;
				}

			TimeEdit_UpdateLastKeyTime(TE, ReturnTime);

			cp->KeysSelected[i]=cp->KeyCount-1;
			// Copy Data

			}
		Callback_ReturnData(TE->TimeLine, c);
		}

	TE->pCTimeLine->m_InsertCut.SetCheck(0);		
	return JE_TRUE;
}
static jeBoolean TimeEdit_InsertTime(TimeEdit *TE)
{
	float TimeOffset,Time;
	int i,c;

	assert(TE);

	if (TE->TimeLine->TimeBarSelected)
		{
		for (c = 0; c < MAX_CHANNELS; c++)
			{
			Channel *cp = &TE->TimeLine->Channel[c];

			if (cp->Disabled) continue;

			for (i = 0; i < cp->KeyCount; i++)
				{
				Time = cp->KeyList[i];
				TimeOffset = Time + ((TE->TimeLine->TimeEnd - TE->TimeLine->TimeStart)/10);

				if (Time >= TE->TimeLine->CurrTime)
					{
					cp->KeyList[i] = TimeOffset;
					}
				}

			Callback_ReturnData(TE->TimeLine, c);
			}	

		}

	return JE_TRUE;
}


jeBoolean TimeEdit_IsCutLeft(Channel *cp, int ndx)
	{
	return (ndx+1 < cp->KeyCount && fabs(cp->KeyList[ndx] - cp->KeyList[ndx+1]) <= TIMELINE_CUT_THRESHOLD);
	}

jeBoolean TimeEdit_IsCutRight(Channel *cp, int ndx)
	{
	return (ndx-1 >= 0 && fabs(cp->KeyList[ndx] - cp->KeyList[ndx-1]) <= TIMELINE_CUT_THRESHOLD);
	}

/////////////////////////////////////////////////////////////////////////////////////////////
// Painting functions

static jeBoolean TimeEdit_TimeLinePaint(TimeEdit *TE,	HDC hdc)
{
	int i,c;
	int x_ul, y_ul, x_lr, y_lr;
	int x_st,y_st,x_ed,y_ed;
	int X;
	float Time;

	HPEN NewBrush, OldBrush;
	HBRUSH hBrush;
	HPEN BluePen, BlackPen;
	TEXTMETRIC tm;
	int cxChar, cyChar;
	DWORD	TextActive  = ::GetSysColor(COLOR_CAPTIONTEXT);
	DWORD	TextInactive= ::GetSysColor(COLOR_INACTIVECAPTIONTEXT);

	GetTextMetrics(hdc, &tm);
	cxChar = tm.tmAveCharWidth;
	cyChar = tm.tmHeight + tm.tmExternalLeading;

	if (TE->SaveTimeLine == TE->TimeLine)
				 SetTextColor(hdc,::GetSysColor(COLOR_WINDOWTEXT));
			else SetTextColor(hdc,TextInactive);

	SelectObject(hdc,(HFONT)hSmallFont);

	assert(TE);

	BlackPen = (HPEN)GetStockObject(BLACK_PEN);
	if (BlackPen == NULL)
		{	
		jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: GetStockObject for pen failed", NULL);
		return JE_FALSE;
		}

	// draw time keys
	NewBrush = (HPEN)GetStockObject(BLACK_BRUSH);
	if (NewBrush == NULL)
		{	
		jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: GetStockObject for pen failed", NULL);
		return JE_FALSE;
		}

	OldBrush = (HPEN)SelectObject(hdc,NewBrush);
	if (OldBrush == NULL)
		{	
		jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: SelectObject for pen failed", NULL);
		return JE_FALSE;
		}

	BluePen = (HPEN)CreatePen(PS_SOLID, 0, RGB(0,0,255));
	if (BluePen == NULL)
		{	
		jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: CreatePen failed", NULL);
		return JE_FALSE;
		}
	
	hBrush = (HBRUSH)SelectObject(hdc, CreateSolidBrush(RGB(255,255,255)));

	for (c = 0; c < MAX_CHANNELS; c++)
		{
		Channel *cp = &TE->TimeLine->Channel[c];

		if (cp->Disabled) continue;

		for (i = 0; i < cp->KeyCount; i++)
			{
			Time = cp->KeyList[i];
			X = TimeEdit_TimeToX(TE, Time);

			if ((X < TE->PositionX) || (X > (TE->PositionX + TE->SizeX)))
				continue;

			if ((c == CHANNEL_EVENT)&&(cp->KeyData[i].String))
				{
				  TextOut(hdc, X+8,TE->PositionY - ELLIPSE_SIZE + Yoff[c]+LINESIZE/2, cp->KeyData[i].String, strlen(cp->KeyData[i].String));
				}


			if (TimeEdit_IsCutLeft(cp, i))
				{
				int x_st,y_st,x_ed,y_ed;

				x_ul = X - ELLIPSE_SIZE*3;
				y_ul = TE->PositionY - ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

				x_lr = X + ELLIPSE_SIZE*3;
				y_lr = TE->PositionY + ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

				x_st = X;
				y_st = y_ul;

				x_ed = X;
				y_ed = y_lr;

				Pie(hdc, x_ul, y_ul, x_lr, y_lr, x_st, y_st, x_ed, y_ed);

				x_st = X;
				y_st = y_lr;

				x_ed = X;
				y_ed = y_ul;

				Pie(hdc, x_ul, y_ul, x_lr, y_lr, x_st, y_st, x_ed, y_ed);

				TimeEdit_DrawLine (hdc, X, TE->PositionY + DUAL_KEY_YOFF, X, TE->PositionY,GetSysColor(COLOR_3DFACE),GetSysColor(COLOR_3DSHADOW));

				i++;
				}
			else
				{
				x_ul = X - ELLIPSE_SIZE;
				y_ul = TE->PositionY - ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

				x_lr = X + ELLIPSE_SIZE;
				y_lr = TE->PositionY + ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

				Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);
/*
				MoveToEx(hdc, X, TE->PositionY + Yoff[c], NULL);
				LineTo(hdc, X, TE->PositionY);*/
				}

			}
		}

	DeleteObject(SelectObject(hdc, hBrush));
	SelectObject(hdc, BlackPen);
	DeleteObject(BluePen);

	// highlight selected keys
	NewBrush = (HPEN)GetStockObject(GRAY_BRUSH);
	if (NewBrush == NULL)
		{	
		jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: GetStockObject for pen failed", NULL);
		return JE_FALSE;
		}
	OldBrush = (HPEN)SelectObject(hdc,NewBrush);
	if (OldBrush == NULL)
		{	
		jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE, "TimeEdit_TimeLinePaint: SelectObject for pen failed", NULL);
		return JE_FALSE;
		}

	hBrush = (HBRUSH)SelectObject(hdc, CreateSolidBrush(RGB(255,0,0)));

	for (c = 0; c < MAX_CHANNELS; c++)
		{
		Channel *cp = &TE->TimeLine->Channel[c];

		if (cp->Disabled) continue;

		for (i = 0; cp->KeysSelected[i] != -1; i++)
			{
			int ndx = cp->KeysSelected[i];
			jeBoolean CutLeft, CutRight;


			Time = cp->KeyList[ndx];
			X = TimeEdit_TimeToX(TE, Time);

		/*	if ((X < TE->PositionX) || (X > (TE->PositionX + TE->SizeX)))
				continue;*/

			CutLeft = CutRight = JE_FALSE;
			x_ul = X - ELLIPSE_SIZE*3;
			y_ul = TE->PositionY - ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

			x_lr = X + ELLIPSE_SIZE*3;
			y_lr = TE->PositionY + ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;
	
			if (TimeEdit_IsCutLeft(cp, ndx))
				{
				CutLeft = JE_TRUE;

				x_st = X;
				y_st = y_ul;

				x_ed = X;
				y_ed = y_lr;

				Pie(hdc, x_ul, y_ul, x_lr, y_lr, x_st, y_st, x_ed, y_ed);
				}
			else
			if (TimeEdit_IsCutRight(cp, ndx))
				{
				CutRight = JE_TRUE;

				x_st = X;
				y_st = y_lr;

				x_ed = X;
				y_ed = y_ul;

				Pie(hdc, x_ul, y_ul, x_lr, y_lr, x_st, y_st, x_ed, y_ed);
				}
			else
				{
				x_ul = X - ELLIPSE_SIZE;
				y_ul = TE->PositionY - ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

				x_lr = X + ELLIPSE_SIZE;
				y_lr = TE->PositionY + ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

				Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);
				}

			if (TE->TimeLine != TE->SaveTimeLine)
				{
				// don't drag if this isn't the selected time line
				continue;
				}

			if (TE->Dragging)
				{

				Time = cp->KeyList[ndx];
				X    = TimeEdit_TimeToX(TE,TE->TimeDragX-TE->TimeDragStartX+Time);

			//	Offset = TE->DragX - DragStartX;

				// X = Offset;

			/*	if (X < TE->PositionX)
					X = TE->PositionX;
				if (X > TE->PositionX + TE->SizeX - 1)
					X = TE->PositionX + TE->SizeX - 1;
*/
				if (CutLeft || CutRight)
					{
					int x_st,y_st,x_ed,y_ed;
					HPEN RedPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, RGB(255,0,0)));

					MoveToEx(hdc, X, TE->PositionY + DUAL_KEY_YOFF, NULL);
					LineTo(hdc, X, TE->PositionY);

					DeleteObject(SelectObject(hdc, RedPen));

					x_ul = X - ELLIPSE_SIZE*3;
					y_ul = TE->PositionY - ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

					x_lr = X + ELLIPSE_SIZE*3;
					y_lr = TE->PositionY + ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

					x_st = X;
					y_st = y_ul;

					x_ed = X;
					y_ed = y_lr;

					Pie(hdc, x_ul, y_ul, x_lr, y_lr, x_st, y_st, x_ed, y_ed);

					x_st = X;
					y_st = y_lr;

					x_ed = X;
					y_ed = y_ul;

					Pie(hdc, x_ul, y_ul, x_lr, y_lr, x_st, y_st, x_ed, y_ed);
					}
				else
					{
					x_ul = X - ELLIPSE_SIZE;
					y_ul = TE->PositionY - ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

					x_lr = X + ELLIPSE_SIZE;
					y_lr = TE->PositionY + ELLIPSE_SIZE + Yoff[c]+LINESIZE/2;

					Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);
					}

				}
			}
		}
	DeleteObject(SelectObject(hdc, hBrush));

	if (TE->TimeLine->TimeBarSelected)
		hBrush = (HBRUSH)SelectObject(hdc, CreateSolidBrush(RGB(255,0,0)));
	else
		hBrush = (HBRUSH)SelectObject(hdc, CreateSolidBrush(RGB(0,0,0)));

	if (TE->TimeLine->TimeBarSelected && TE->Dragging)
		{
		X = TimeEdit_TimeToX(TE, TE->TimeLine->CurrTime);
		if (X < TE->PositionX)
			{
			X = TE->PositionX;
			}
		if (X > TE->PositionX + TE->SizeX - 1)
			{
			X = TE->PositionX + TE->SizeX - 1;
			}

		}
	else
		X = TimeEdit_TimeToX(TE, TE->TimeLine->CurrTime);

	if (X >= TE->PositionX && X <= TE->PositionX + TE->SizeX)
		{
		char str[64];

		

		x_ul = X - ELLIPSE_SIZE;
		y_ul = TE->PositionY - ELLIPSE_SIZE - TIMEBAR_TOP_SIZE+LINESIZE/4;

		x_lr = X + ELLIPSE_SIZE;
		y_lr = TE->PositionY + ELLIPSE_SIZE - TIMEBAR_TOP_SIZE+LINESIZE/4;


		TimeEdit_DrawLine (hdc, X-1, TE->PositionY + TIMEBAR_BOT_SIZE-LINESIZE+LINESIZE/4,
							   X-1, TE->PositionY - TIMEBAR_TOP_SIZE+LINESIZE/4,
							   GetSysColor(COLOR_3DSHADOW),GetSysColor(COLOR_3DHIGHLIGHT));
		Ellipse(hdc, x_ul, y_ul, x_lr, y_lr);

		sprintf(str,"%.3f", TE->TimeLine->CurrTime);
		TextOut(hdc, (X)+10,TE->PositionY + ELLIPSE_SIZE - TIMEBAR_TOP_SIZE - cyChar/2+LINESIZE/4, str, strlen(str));

		}

	DeleteObject(SelectObject(hdc, hBrush));

	return JE_TRUE;
}


static jeBoolean TimeEdit_Paint(TimeEdit *TE)
{
	HDC hdc;
	HPEN NewBrush;
	HPEN OldBrush;

	char str[64];
	TEXTMETRIC tm;
	int cxChar, cyChar;
	int TimeLineNdx;
	int SavePosY;
	RECT	hdcRect;

	// Added JH 25.4.2000
	HDC		memhdc = NULL;
	HBITMAP offscreenBitmap = NULL;
	HGDIOBJ holdBitmap =NULL;
	POINT	oldWindowPos;

	HFONT	holdFont = NULL;
	DWORD	Gray1 = RGB(150,150,150);
	DWORD	Gray2 = RGB(200,200,200);
	DWORD	Gray3 = ::GetSysColor(COLOR_BTNFACE);
	DWORD	Gray4 = ::GetSysColor(COLOR_3DHIGHLIGHT);
	DWORD	BarActive   = ::GetSysColor(COLOR_ACTIVECAPTION);
	DWORD	BarInactive = ::GetSysColor(COLOR_INACTIVECAPTION);
	DWORD	TextActive  = ::GetSysColor(COLOR_CAPTIONTEXT);
	DWORD	TextInactive= ::GetSysColor(COLOR_INACTIVECAPTIONTEXT);

	HRGN	ClipRegion;
	
	int		TimeLineHeight;


	assert( TE ) ;


 
	if (TE->TimeLineCount == 0)
		return JE_TRUE;

	if (TimeEdit_SetPosition(TE)==JE_FALSE)
	{
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"TimeEdit_Paint:  TimeEdit_SetPosition failed");
		return JE_FALSE;
	}

	hdc = GetDC(TE->hwnd);
	if (hdc == NULL)
	{
		jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"TimeEdit_Paint:  GetDC for drawing failed");
		return JE_FALSE;
	}

	
	NewBrush = (HPEN)GetStockObject(BLACK_BRUSH);
	if (NewBrush == NULL)
	{
		jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"TimeEdit_Paint:  GetStockObject for pen failed");
		ReleaseDC(TE->hwnd,hdc);
		return JE_FALSE;
	}
				
	OldBrush = (HPEN)SelectObject(hdc,NewBrush);
	if (OldBrush == NULL)
	{
		jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"TimeEdit_Paint:  SelectObject for pen failed");
		ReleaseDC(TE->hwnd,hdc);
		return JE_FALSE;
	}
	
	// Create Offscreen DC

	GetClipBox(hdc,&hdcRect);

	memhdc = CreateCompatibleDC(hdc);

 	if (memhdc==NULL) 
	{
		ReleaseDC(TE->hwnd,hdc);
		return JE_FALSE;
	}

	offscreenBitmap = CreateCompatibleBitmap(hdc,hdcRect.right-hdcRect.left,hdcRect.bottom-hdcRect.top);

 	if (offscreenBitmap==NULL) 
	{
		DeleteDC(memhdc);
		ReleaseDC(TE->hwnd,hdc);
		return JE_FALSE;
	}

	holdBitmap		= SelectObject( memhdc,offscreenBitmap );

	// Draw Background
	TimeEdit_DrawRect(memhdc,0,0,hdcRect.right-hdcRect.left,hdcRect.bottom-hdcRect.top, ::GetSysColor(COLOR_BACKGROUND),0);

	SetWindowOrgEx (memhdc,hdcRect.left,hdcRect.top,&oldWindowPos);

	SetBkMode(memhdc, TRANSPARENT); 

	if (memhdc == NULL)
		memhdc = hdc;

	TE->SaveTimeLine = TE->TimeLine;
	SavePosY = TE->PositionY;

	TE->PositionY = 0;
	for (TimeLineNdx = 0; TimeLineNdx < TE->TimeLineCount; TimeLineNdx++)
	{
		TE->TimeLine = &TE->TimeLineList[TimeLineNdx];
		TE->PositionY = ((TimeLineNdx) * LINE_OFFSET) - TE->ScrollY + TIMEBAR_START;
	
		SelectObject(memhdc,NewBrush);
		
		ClipRegion = CreateRectRgn(
				  TE->PositionX+1-hdcRect.left,   
				  TE->PositionY - TIMEBAR_TOP_SIZE - ELLIPSE_SIZE-hdcRect.top,   
				  TE->PositionX + TE->SizeX-hdcRect.left, 
				  TE->PositionY + TIMELINE_TICK_YOFF + RULERSIZE+1-hdcRect.top);

	
		// Select font for title
		if (holdFont==NULL)
				 holdFont = (HFONT)SelectObject(memhdc,(HFONT)hMedFont);
			else holdFont = (HFONT)SelectObject(memhdc,(HFONT)hMedFont);

		GetTextMetrics(memhdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cyChar = tm.tmHeight + tm.tmExternalLeading;

		// Draw active Title
		if (TE->SaveTimeLine == TE->TimeLine)
		{
			TimeEdit_DrawRect(memhdc,4,TE->PositionY - (TIMEBAR_TOP_SIZE) ,  TE->WindowAcross-4,TE->PositionY + (TIMEBAR_BOT_SIZE), Gray3,1);
			TimeEdit_DrawRect(memhdc,4,TE->PositionY - (TIMEBAR_TOP_SIZE) - cyChar - 4,  TE->WindowAcross-4, TE->PositionY - (TIMEBAR_TOP_SIZE)- cyChar - 4 +TITLESIZE, BarActive,1);
			SetTextColor(memhdc,TextActive);
		}

      // Draw Incative Title
		else			
		{
			TimeEdit_DrawRect(memhdc,4,TE->PositionY - (TIMEBAR_TOP_SIZE)  ,  TE->WindowAcross-4,TE->PositionY + (TIMEBAR_BOT_SIZE) , Gray1,0);
			TimeEdit_DrawRect(memhdc,4,TE->PositionY - (TIMEBAR_TOP_SIZE) - cyChar - 4, TE->WindowAcross-4, TE->PositionY - (TIMEBAR_TOP_SIZE)- cyChar - 4 +TITLESIZE , BarInactive,1);
			SetTextColor(memhdc,TextInactive);
		}

		sprintf(str,"%s", TE->TimeLine->Descr);
		TextOut(memhdc, 13, TE->PositionY - (TIMEBAR_TOP_SIZE) - cyChar-1, str, strlen(str));
		SelectObject(memhdc,(HFONT)hSmallFont);

		if (TE->SaveTimeLine == TE->TimeLine)
				 SetTextColor(memhdc,::GetSysColor(COLOR_WINDOWTEXT));
			else SetTextColor(memhdc,TextInactive);

		// Draw Pos Bar
		if (!TE->TimeLine->Channel[CHANNEL_POS].Disabled)
			{
			SIZE size;
			char *ptr;
			char *strptr;

			TimeEdit_DrawLine (memhdc,TE->PositionX,TE->PositionY + POS_KEY_ELLIPSE_YOFF,
							   TE->PositionX + TE->SizeX,TE->PositionY + POS_KEY_ELLIPSE_YOFF,
							   Gray1,Gray3);
			TimeEdit_DrawRect(memhdc,TE->PositionX,TE->PositionY + POS_KEY_ELLIPSE_YOFF,
							   TE->PositionX + TE->SizeX,TE->PositionY + POS_KEY_ELLIPSE_YOFF+LINESIZE-1, Gray1,0);

			strptr = str;
			sprintf(str,"Pos  ");
			
			TimeLineHeight = ROT_KEY_ELLIPSE_YOFF;

			ptr = strstr(TE->TimeLine->Descr,"(");
			if (ptr)
				{
				char *start = ++ptr;
				ptr = strstr(start,")");
				assert(ptr);
				memset(str,0,sizeof(str));
				memcpy(str, start, ptr-start);
				strcat(str, "  ");

				ptr = strstr(str,"->");
				if (ptr)
					strptr = &ptr[2];
				TimeLineHeight = POS_KEY_ELLIPSE_YOFF;
				}

			SelectObject(memhdc,NewBrush);
			GetTextExtentPoint32(memhdc, strptr, strlen(strptr), &size);

			TextOut(memhdc, (TE->PositionX) - size.cx,
			TE->PositionY + Yoff[CHANNEL_POS] - cyChar/2+LINESIZE/2, strptr, strlen(strptr));
			}

		// Draw Rot Bar
		if (!TE->TimeLine->Channel[CHANNEL_ROT].Disabled)
			{
			SIZE size;
			TimeEdit_DrawLine (memhdc,TE->PositionX,TE->PositionY + ROT_KEY_ELLIPSE_YOFF,
							   TE->PositionX + TE->SizeX,TE->PositionY + ROT_KEY_ELLIPSE_YOFF,
							   Gray1,Gray3);
			TimeEdit_DrawRect(memhdc,TE->PositionX,TE->PositionY + ROT_KEY_ELLIPSE_YOFF,
							   TE->PositionX + TE->SizeX,TE->PositionY +ROT_KEY_ELLIPSE_YOFF +LINESIZE-1, Gray1,0);


			SelectObject(memhdc,NewBrush);
			sprintf(str,"Rot  ");
			GetTextExtentPoint32(memhdc, str, strlen(str), &size);
			TextOut(memhdc, (TE->PositionX) - size.cx,
				TE->PositionY + Yoff[CHANNEL_ROT] - cyChar/2+LINESIZE/2, str, strlen(str));
			}

		// Draw Pos&Rot Bar
		if (!TE->TimeLine->Channel[CHANNEL_ROT].Disabled && !TE->TimeLine->Channel[CHANNEL_POS].Disabled)
			{
			SIZE size;
			TimeEdit_DrawLine (memhdc,TE->PositionX,TE->PositionY + DUAL_KEY_YOFF,
							   TE->PositionX + TE->SizeX,TE->PositionY + DUAL_KEY_YOFF,
							   Gray1,Gray3);

			SelectObject(memhdc,NewBrush);
			sprintf(str,"Pos&Rot  ");
			GetTextExtentPoint32(memhdc, str, strlen(str), &size);
			TextOut(memhdc, (TE->PositionX) - size.cx,
				TE->PositionY + DUAL_KEY_YOFF - cyChar/2+LINESIZE/2, str, strlen(str));
			}

		// Draw Event Bar
		if (!TE->TimeLine->Channel[CHANNEL_EVENT].Disabled)
			{
			SIZE size;
			TimeEdit_DrawLine (memhdc,TE->PositionX,TE->PositionY + EVENT_ELLIPSE_YOFF,
							   TE->PositionX + TE->SizeX,TE->PositionY + EVENT_ELLIPSE_YOFF,
							   Gray1,Gray3);
			TimeEdit_DrawRect(memhdc,TE->PositionX,TE->PositionY + EVENT_ELLIPSE_YOFF,
							   TE->PositionX + TE->SizeX,TE->PositionY + EVENT_ELLIPSE_YOFF+LINESIZE-1, Gray1,0);
			SelectObject(memhdc,NewBrush);
			sprintf(str,"Event  ");
			GetTextExtentPoint32(memhdc, str, strlen(str), &size);
			TextOut(memhdc, (TE->PositionX) - size.cx,
				TE->PositionY + Yoff[CHANNEL_EVENT] - cyChar/2+LINESIZE/2, str, strlen(str));
			}


		SelectObject(memhdc,NewBrush);

		// Set Clipping region
 		SelectClipRgn(memhdc, ClipRegion);

		// Draw Rulers
		if (!TE->TimeLine->Channel[CHANNEL_EVENT].Disabled)
		{
			double ThisTime;
			double StartTime;
			int   PosX = 0;
			double Range;
			int   XStep;
			double len;
			double fract, integer;
			double RangeAdd=0;

			StartTime     = TE->TimeLine->TimeStart;

			SelectClipRgn(memhdc, NULL);

			TimeEdit_DrawRect(memhdc,TE->PositionX-1,TE->PositionY + EVENT_ELLIPSE_YOFF+LINESIZE,
								   TE->PositionX + TE->SizeX,TE->PositionY +TIMELINE_TICK_YOFF+RULERSIZE, Gray2,1);
			if (TE->SaveTimeLine == TE->TimeLine)
					 TimeEdit_DrawRect(memhdc,TE->PositionX,TE->PositionY + EVENT_ELLIPSE_YOFF+LINESIZE+1,
								   TE->PositionX + TE->SizeX-1,TE->PositionY +TIMELINE_TICK_YOFF+RULERSIZE-1, Gray2,0);
				else TimeEdit_DrawRect(memhdc,TE->PositionX,TE->PositionY + EVENT_ELLIPSE_YOFF+LINESIZE+1,
								   TE->PositionX + TE->SizeX-1,TE->PositionY +TIMELINE_TICK_YOFF+RULERSIZE-1, Gray1,0);

			SelectClipRgn(memhdc, ClipRegion);

			len = TE->TimeLine->TimeEnd - TE->TimeLine->TimeStart;
			
	
			 Range = (len)/10.0f;

			 if (Range==0) Range=0.6f*10000;

			 ThisTime	= (StartTime)/Range;
			 fract		= modf (ThisTime,&integer);
			 ThisTime	= integer;
			 ThisTime  *= Range;


			SetTextColor(memhdc,::GetSysColor(COLOR_WINDOWTEXT));

			while (PosX<TE->PositionX + TE->SizeX+10)
				{
				  PosX = TimeEdit_TimeToX (TE,ThisTime+RangeAdd);

				  if (!TE->TimeMode)
					{ if (len < 5.0f)
						sprintf(str,"%.3f",ThisTime+RangeAdd);
					  else if (len < 7.0f)
						sprintf(str,"%.2f",ThisTime+RangeAdd);
					  else if (len < 15.0f)
						sprintf(str,"%.1f",ThisTime+RangeAdd);
					  else
						sprintf(str,"%.f",ThisTime+RangeAdd);
					}
				  else 
					{
					  fract	= modf (ThisTime+RangeAdd+0.0001,&integer);
					  sprintf(str,"%d:%02d:%02d:%04d",(int32)(integer/3600),(int32)(integer/60)%60,(int32)integer%60,(int32)(fract*1000));
					}

				  TimeEdit_DrawLine (memhdc,PosX,TE->PositionY+ TIMELINE_TICK_YOFF+3,
							   PosX,TE->PositionY+ TIMELINE_TICK_YOFF+RULERSIZE-2,
							   Gray4,0);
				  if (!TE->TimeLine->Channel[CHANNEL_EVENT].Disabled)
					TimeEdit_DrawLine (memhdc,PosX,TE->PositionY+ TimeLineHeight+2,
							   PosX,TE->PositionY+ TIMELINE_TICK_YOFF-2,
							   Gray1,Gray2);				  
				  TextOut(memhdc, PosX+6 , TE->PositionY +TIMELINE_TICK_YOFF+3 , str, strlen(str));
	
				  for (XStep = 0; XStep < 10; XStep++)
					{
					  RangeAdd+=(Range/10.0);					
					  PosX = TimeEdit_TimeToX (TE,ThisTime+RangeAdd);
					  MoveToEx(memhdc,PosX,TE->PositionY + TIMELINE_TICK_YOFF+RULERSIZE-4,NULL);
					  LineTo(memhdc,PosX,TE->PositionY+ TIMELINE_TICK_YOFF+RULERSIZE-1);
					}
			}
		}

		// draw time line data
		if (TimeEdit_TimeLinePaint(TE, memhdc) == JE_FALSE)
			{	
			jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE, "TimeEdit_Paint: TimeLinePaint failed", NULL);
			
			SelectClipRgn(memhdc, NULL);
			DeleteObject (ClipRegion);
			SelectObject(hdc,holdBitmap);
			DeleteObject(offscreenBitmap);
			SelectObject(memhdc,holdFont);
			DeleteDC (memhdc);

			ReleaseDC(TE->hwnd,hdc);
			return JE_FALSE;
			}

		SelectClipRgn(memhdc, NULL);
		DeleteObject (ClipRegion);

		}

    BitBlt(hdc,hdcRect.left,hdcRect.top, hdcRect.right-hdcRect.left,hdcRect.bottom-hdcRect.top,
                        memhdc, hdcRect.left,hdcRect.top, SRCCOPY);
	
  	SelectClipRgn(memhdc, NULL);
 
   DeleteObject (ClipRegion);

   SelectObject(memhdc,holdFont);
            //Swap back the original bitmap.
	SelectObject(hdc,holdBitmap);
            //Delete Bitmap
	DeleteObject(offscreenBitmap);
			// Free 
	DeleteDC (memhdc);

	ReleaseDC(TE->hwnd,hdc);

	TE->TimeLine = TE->SaveTimeLine;
	TE->PositionY = SavePosY;
 
	return JE_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Public Functions

void TimeEdit_Clear(TimeEdit* TE)
{
	int c;
	assert(TE);

	for (c = 0; c < MAX_CHANNELS; c++)
		{
		Channel *cp = &TE->TimeLine->Channel[c];

		if (cp->Disabled) continue;

		cp->KeyCount = 0;
		memset(cp->KeysSelected, -1, sizeof(cp->KeysSelected));
		}

	TE->TimeLine->CurrTime = 0.0f;
	TE->Dragging = JE_FALSE;
	InvalidateRect(TE->hwnd, NULL, FALSE);
}

jeBoolean TimeEdit_InitAddPosKey(TimeEdit* TE, float Time)
{
	Channel *cp;

	assert(TE);

	cp = &TE->TimeLine->Channel[CHANNEL_POS];

	cp->KeyList[cp->KeyCount] = Time;
	if (cp->KeyCount++ >= MAX_KEYS)
		{
		jeErrorLog_AddString(-1,"TimeEdit_InitAddKey: Key limit hit.", NULL);
		return JE_FALSE;
		}
	return JE_TRUE;
}

jeBoolean TimeEdit_InitAddRotKey(TimeEdit* TE, float Time)
{
	Channel *cp;
	assert(TE);

	cp = &TE->TimeLine->Channel[CHANNEL_ROT];

	cp->KeyList[cp->KeyCount] = Time;
	if (cp->KeyCount++ >= MAX_KEYS)
		{
		jeErrorLog_AddString(-1,"TimeEdit_InitAddKey: Key limit hit.", NULL);
		return JE_FALSE;
		}
	return JE_TRUE;
}

jeBoolean TimeEdit_InitAddEvent(TimeEdit* TE, float Time)
{
	Channel *cp;
	assert(TE);

	cp = &TE->TimeLine->Channel[CHANNEL_EVENT];

	cp->KeyList[cp->KeyCount] = Time;
	if (cp->KeyCount++ >= MAX_KEYS)
		{
		jeErrorLog_AddString(-1,"TimeEdit_InitAddKey: Key limit hit.", NULL);
		return JE_FALSE;
		}
	return JE_TRUE;
}

jeBoolean TimeEdit_InitAddTimeLine(TimeEdit* TE, int ChannelFlags)
{
	int ThisTimeLine;
	int i;

	assert(TE);

	ThisTimeLine = TE->TimeLineCount;

	for (i = 0; i < MAX_CHANNELS; i++)
		{
		TE->TimeLineList[ThisTimeLine].Channel[i].Disabled = JE_TRUE;

		if (ChannelFlags & 1<<i)
			{
			memset(TE->TimeLineList[ThisTimeLine].Channel[i].KeyList, 0, sizeof(TE->TimeLineList[ThisTimeLine].Channel[i].KeyList));
			memset(TE->TimeLineList[ThisTimeLine].Channel[i].KeysSelected, -1, sizeof(TE->TimeLineList[ThisTimeLine].Channel[i].KeysSelected));
			TE->TimeLineList[ThisTimeLine].Channel[i].Disabled = JE_FALSE;
			TE->TimeLineList[ThisTimeLine].Channel[i].KeyCount = 0;
			}
		}

	TE->TimeLineCount++;
	//TimeEdit_SelectTimeLineFromIndex(TE, ThisTimeLine);
	//TimeEdit_SetCurrentTimeLine(TE, ThisTimeLine);
	TE->TimeLine = &TE->TimeLineList[ThisTimeLine];

	return JE_TRUE;
}


jeBoolean TimeEdit_GetPathObject(Object** op)
{
	ObjectList *OL;
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	ObjectIterator Iterator;

	assert(pDoc);

	*op = NULL;

	OL = pDoc->GetSelectList();

	if (!OL || ObjectList_GetNumItems(OL) != 1)
		{
		return JE_TRUE;
		}

	*op = ObjectList_GetFirst(OL, &Iterator);

	if (*op == NULL)
		{
		return JE_TRUE;
		}

	if (strcmp(Object_GetName(*op),"PathObject") != 0)
		{
		*op = NULL;
		return JE_TRUE;
		}

	return JE_TRUE;
	}

void Callback_ReturnTime(TimeData *TD)
{
	jeProperty_Data Data;
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;

	Data.Float = TD->CurrTime;
	pDoc->SetProperty( TD->CurrTimeProp, PROPERTY_CURTIME_TYPE, &Data );
}

void Callback_ReturnData(TimeData *TD, int ChannelNdx)
{
	int PropType[MAX_CHANNELS] = {PROPERTY_CHANNEL_POS_TYPE, PROPERTY_CHANNEL_ROT_TYPE, PROPERTY_CHANNEL_EVENT_TYPE};
	jeProperty_Data Data;
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;

	Data.Ptr = &TD->Channel[ChannelNdx];
	pDoc->SetProperty( TD->ChannelPropID[ChannelNdx], PropType[ChannelNdx], &Data );
}

void TimeEdit_QueryControllerForTimeLine(TimeEdit *TE)
{
	Object *op;
	int TimeLineNdx,ReturnedTimeLineNdx;
	Object_TimeGetTimeLine data;

	TimeEdit_GetPathObject(&op);
	if (!op)
		{
		return;
		}

	TimeLineNdx = TE->TimeLine - TE->TimeLineList;
	data.TimeLineNdx = TimeLineNdx;

	Object_SendMessage(op, OBJECT_TIMELINE_GET_CURRENT_TIMELINE_MSG, &data);
	
	ReturnedTimeLineNdx = data.ReturnTimeLineNdx;

	if (ReturnedTimeLineNdx != TimeLineNdx)
		{
		TimeEdit_SelectTimeLineFromIndex(TE, TimeLineNdx);
		TimeEdit_SelectObject(TE);
		InvalidateRect(TE->hwnd, NULL, FALSE);
		}

	TimeEdit_SelectObject(TE);

	return;
}

jeObject *TimeEdit_GetObject(TimeEdit *TE, int TimeLineNdx)
{
	Object *op;
	Object_TimeGetObject data;
	jeObject *obj;

	TimeEdit_GetPathObject(&op);
	if (!op)
		{
		return NULL;
		}

	data.TimeLineNdx = TimeLineNdx;

	Object_SendMessage(op, OBJECT_TIMELINE_GET_JEOBJECT_MSG, &data);
	
	obj = data.ReturnObj;

	return obj;
	TE;
}

void TimeEdit_SelectObject(TimeEdit *TE)
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	jeObject *obj;

	obj = TimeEdit_GetObject(TE, TE->TimeLine - TE->TimeLineList);

	if (!obj)
		return;

	pDoc->DeselectAllSub();
	pDoc->SubSelectgeObject(obj, LEVEL_SELECT);
}

void TimeEdit_MarkObject(TimeEdit *TE, int TimeLineNdx)
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	jeObject *obj;

	obj = TimeEdit_GetObject(TE, TimeLineNdx);

	if (!obj)
		return;

	pDoc->MarkSubSelect(obj, SubSelect_Hilite|SubSelect_Move|SubSelect_Rotate);
}


void Callback_GetKeyData(TimeEdit *TE, int ChannelNdx, int KeyNdx)
{
	Object *op;
	int TimeLineNdx;
	Object_TimeKeyData data;

	TimeEdit_GetPathObject(&op);
	if (!op)
		{
		return;
		}

	TimeLineNdx = TE->TimeLine - TE->TimeLineList;
	data.ChannelNdx = ChannelNdx;
	data.TimeLineNdx = TimeLineNdx;

	Object_SendMessage(op, OBJECT_TIMELINE_GET_KEY_DATA_MSG, &data);

	switch (ChannelNdx)
	{
		case CHANNEL_POS:
			{
			//TE->TimeLine->Channel[ChannelNdx].KeyData[KeyNdx].XForm.Translation = data.ReturnXF.Translation;
			//break;
			}
		case CHANNEL_ROT:
			{
			TE->TimeLine->Channel[ChannelNdx].KeyData[KeyNdx].XForm = data.ReturnXF;
			break;
			}
		case CHANNEL_EVENT:
			{
			assert(TRUE == FALSE);
			break;
			}
	}
}

jeBoolean Callback_SetPlayMode(TimeEdit *TE, jeBoolean Play)
{
	Object_TimePlayMode data;
	Object *op;

	TimeEdit_GetPathObject(&op);
	if (!op)
		{
		return JE_TRUE;
		}

	data.TimeLineNdx = TE->TimeLine - TE->TimeLineList;
	data.PlayMode = Play;

	Object_SendMessage(op, OBJECT_TIMELINE_SET_PLAY_MODE_MSG, &data);

	return JE_TRUE;
}



jeBoolean Callback_GetPlayMode(TimeEdit *TE, int *PlayMode)
{
	Object_TimePlayMode data;
	Object *op;

	TimeEdit_GetPathObject(&op);
	if (!op)
	{
		*PlayMode = JE_FALSE;
		return JE_TRUE;
	}
		
	data.TimeLineNdx = TE->TimeLine - TE->TimeLineList;
	Object_SendMessage(op, OBJECT_TIMELINE_GET_PLAY_MODE_MSG, &data);

	*PlayMode = data.PlayMode;

	return JE_TRUE;
}

void Callback_GetCurrentTime(TimeData *TD, float *Time)
{
	Object *op;
	Object_TimeGetTime data;

	TimeEdit_GetPathObject(&op);
	if (!op)
		{
		return;
		}

	data.TimeLineNdx = -1;
	Object_SendMessage(op, OBJECT_TIMELINE_GET_CURRENT_TIME_MSG, &data);
	*Time = data.ReturnTime;

	return;
	TD;
}

void Callback_GetIndividualTime(TimeEdit *TE, int TimeLineNdx, float *Time)
{
	Object *op;
	Object_TimeGetTime data;

	TimeEdit_GetPathObject(&op);
	if (!op)
		{
		return;
		}

	data.TimeLineNdx = TimeLineNdx;
	Object_SendMessage(op, OBJECT_TIMELINE_GET_CURRENT_TIME_MSG, &data);
	*Time = data.ReturnTime;

	return;
	TE;
}

void JETCC TimeEdit_FreeTimeLine(TimeEdit *TE, int TimeLineNdx)
{
	int k;

	// the only thing allocated are strings for events
	for (k = 0; k < MAX_KEYS; k++)
		{
		if (TE->TimeLineList[TimeLineNdx].Channel[CHANNEL_EVENT].KeyData[k].String)
			{
			jeRam_Free(TE->TimeLineList[TimeLineNdx].Channel[CHANNEL_EVENT].KeyData[k].String);
			}
		}
}

void TimeEdit_Destroy(TimeEdit *TE)
{
	assert(TE);

	if (TE->hwnd)
		{
		DestroyWindow(TE->hwnd);
		}

	TimeEdit_RegisterCount--;
	if (TimeEdit_RegisterCount==0)
		{
		UnregisterClass(TIMEEDIT_WINDOW_CLASS,TE->hinst);
		}

	if (TE)
		{
		int i;

		for (i = 0; i < MAX_TIME_LINES; i++)
			{
			TimeEdit_FreeTimeLine(TE, i);
			}
		jeRam_Free(TE);
		}
}

TimeEdit *TimeEdit_CreateTimeEditData(HWND hWnd)
{
	TimeEdit *TE;
	int i;

	TE = JE_RAM_ALLOCATE_STRUCT(TimeEdit);

	if (TE == NULL)
		{
			jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE,"TimeEdit_CreateWindow:  Couldn't create TimeEdit object",NULL);
			goto Error;
		}

	TE->Dragging = JE_FALSE;
	TE->rDragging= JE_FALSE;
	TE->MoveAll	 = JE_FALSE;
	TE->TimeMode = JE_FALSE;
	TE->ScrollY  = 0;

	TE->TimeLine = NULL;
	TE->TimeLineCount = 0;
	memset(TE->TimeLineList, 0, sizeof(TE->TimeLineList));

	TimeEdit_InitAddTimeLine(TE, (1<<CHANNEL_POS)|(1<<CHANNEL_ROT)|(1<<CHANNEL_EVENT));
	TE->TimeLine->TimeEnd = 10.0f;
	TE->TimeLine->TimeStart = 0.0f;
	for (i = 0; i < MAX_TIME_LINES; i++)
		{
		TE->TimeLineList[i].TimeEnd = 60.0f;
		}

	TE->hwnd = hWnd;
	TE->hinst = AfxGetInstanceHandle();
	SetLastError(0);

	if (SetWindowLong(hWnd,0,(long)TE)==0)
		{
		if (GetLastError()!=0)
			{
			jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE,"TimeEdit_CreateWindow:  SetWindowLong failed",jeErrorLog_IntToString(GetLastError()));
			goto Error;
			}
		}

	return(TE);

Error:

	if (TE)
		jeRam_Free(TE);

	return (NULL);
}


HWND TimeEdit_CreateWindow(HWND hwndParent, int32 Width, int32 Height)
{
	WNDCLASS		wc;
	HWND			hWnd=0;
	TimeEdit		*TE=0;

	wc.style         = 0/*CS_HREDRAW | CS_VREDRAW*/;
	wc.lpfnWndProc   = TimeEdit_WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(TimeEdit *);
	wc.hInstance     = AfxGetInstanceHandle();
	wc.hIcon         = LoadIcon(AfxGetInstanceHandle(), IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.lpszMenuName  = (const char*)NULL;
	wc.lpszClassName = TIMEEDIT_WINDOW_CLASS;

	if (TimeEdit_RegisterCount==0)
		{
			if (RegisterClass(&wc)==0)
				{
					jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE,"TimeEdit_CreateWindow: RegisterClass for TimeEdit window",jeErrorLog_IntToString(GetLastError()));
					goto Error;
				}
		}
	TimeEdit_RegisterCount++;

	hWnd = CreateWindowEx(
		0,
		wc.lpszClassName,
		NULL,
		WS_CHILD|WS_VSCROLL|WS_HSCROLL,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		Width,
		Height,
		hwndParent,
		NULL,
		AfxGetInstanceHandle(),
		NULL);

	if (!hWnd)
		{
			jeErrorLog_AddString(JE_ERR_WINDOWS_API_FAILURE,"TimeEdit_CreateWindow: CreateWindowEx failed for TimeEdit window",jeErrorLog_IntToString(GetLastError()));
			goto Error;
		}

	SetWindowPos(hWnd, HWND_TOP, 0, 50, Width, Height, NULL);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	
 
	return hWnd;

	Error:
	if (hWnd)
		{
		DestroyWindow(hWnd);
		}

	TimeEdit_RegisterCount--;
	if (TimeEdit_RegisterCount==0)
		{
		UnregisterClass(TIMEEDIT_WINDOW_CLASS,AfxGetInstanceHandle());
		}

	if (TE)
		{
		jeRam_Free(TE);
		}
	return NULL;
}




BOOL CTimeLine::OnInitDialog() 
{

	CDialog::OnInitDialog();


	m_hWnd = TimeEdit_CreateWindow(this->GetSafeHwnd(), 820, 250);
	m_TE = TimeEdit_CreateTimeEditData(m_hWnd);
	m_TE->pCTimeLine = this;

	CString	Tooltip;

	Tooltip = "Zoom out";
	bZoomIn.SubclassDlgItem ( IDC_TIMELINE_ZOOM_IN, this);
	bZoomIn.SetIcon  ( IDI_ZOOMIN);
	bZoomIn.SetAlign ( CButtonST::ST_ALIGN_VERT);  
	bZoomIn.SetActiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bZoomIn.SetInactiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bZoomIn.SetTooltipText (&Tooltip,TRUE);

	Tooltip = "Zoom into active timeline";
	bZoomOut.SubclassDlgItem ( IDC_TIMELINE_ZOOM_OUT, this);
	bZoomOut.SetIcon  ( IDI_ZOOMOUT);
	bZoomOut.SetAlign ( CButtonST::ST_ALIGN_VERT);  
	bZoomOut.SetActiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bZoomOut.SetInactiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bZoomOut.SetTooltipText (&Tooltip,TRUE);

	Tooltip = "Reset settings to default";
	bReset.SubclassDlgItem ( IDC_TIMELINE_RESET, this);
	bReset.SetIcon  ( IDI_RESET);
	bReset.SetAlign ( CButtonST::ST_ALIGN_VERT);  
	bReset.SetActiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bReset.SetInactiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bReset.SetTooltipText (&Tooltip,TRUE);

	Tooltip = "Start/Stop playing animation";
	bPlay.SubclassDlgItem ( IDC_TIMELINE_MOD, this);
	bPlay.SetIcon  ( IDI_PLAY);
	bPlay.SetAlign ( CButtonST::ST_ALIGN_VERT);  
	bPlay.SetActiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bPlay.SetInactiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bPlay.SetTooltipText (&Tooltip,TRUE);

	Tooltip = "Insert cutpoint";
	bInsertCut.SubclassDlgItem ( IDC_TIMELINE_CUTPOINT, this);
	bInsertCut.SetIcon  ( IDI_CUT);
	bInsertCut.SetAlign ( CButtonST::ST_ALIGN_VERT);  
	bInsertCut.SetActiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bInsertCut.SetInactiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bInsertCut.SetTooltipText (&Tooltip,TRUE);
	  
	Tooltip = "Handle all timeline as one";
	bMoveAll.SubclassDlgItem ( IDC_TIMELINE_MOVEALL, this);
	bMoveAll.SetIcon  ( IDI_UNLOCK);
	bMoveAll.SetAlign ( CButtonST::ST_ALIGN_VERT);  
	bMoveAll.SetActiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bMoveAll.SetInactiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bMoveAll.SetTooltipText (&Tooltip,TRUE);

	Tooltip = "Switch to SS.MS";
	bSMS.SubclassDlgItem ( IDC_TIMELINE_SMS, this);
	bSMS.SetIcon  ( IDI_TIMECOLOR);
	bSMS.SetActiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bSMS.SetInactiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bSMS.SetTooltipText (&Tooltip,TRUE);

	Tooltip = "Switch to HH.MM.SS.MS";
	bHHMMSS.SubclassDlgItem ( IDC_TIMELINE_HHMMSS, this);
	bHHMMSS.SetIcon  ( IDI_TIME);
	bHHMMSS.SetActiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bHHMMSS.SetInactiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bHHMMSS.SetTooltipText (&Tooltip,TRUE);

/*	Tooltip = "Clear Object Settings";
	bCLR.SubclassDlgItem    (IDC_TIMELINE_CLR, this);
	bCLR.SetActiveFgColor   (::GetSysColor(COLOR_WINDOWTEXT));
	bCLR.SetInactiveFgColor (::GetSysColor(COLOR_WINDOWTEXT));
	bCLR.SetTooltipText (&Tooltip,TRUE);
*/
	// event edit box starts out read only
	TimeEdit_SetEventReadOnly (m_TE,JE_TRUE);
	//ToolBar.Create(IDD,this);

	this->MoveWindow( 0, 0, 820, 300, TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTimeLine::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (nState == WA_ACTIVE)
		::SetFocus(this->m_TE->hwnd);
	
	OnSetObjectNames();
}

void CTimeLine::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (!this->m_TE)
		return;
	
	::SetWindowPos(this->m_TE->hwnd, HWND_TOP, 0, 60, cx, cy-60, NULL);

}

static char	UpdateChangeText=0;		// Sync  GetEventProperties with SetEventProperties (should use semaphores)

void CTimeLine::OnCLR() 
{
	m_TE->pCTimeLine->m_EventText.SetWindowText("");
	m_TE->pCTimeLine->m_ObjectName.SetCurSel(-1);
	m_TE->pCTimeLine->m_ObjectMessage.SetCurSel(-1);
	SetEventProperties();
	::InvalidateRect(m_TE->hwnd, NULL, FALSE);
}

void CTimeLine::GetEventProperties() 
{
	Channel *cp;
	char ObjectName[512];
	char Temp[3*512];
	char *ptr;
	int	 ObjectId;		// This should be a name not a number (later ...)

	if (UpdateChangeText==1) return;
	
	UpdateChangeText=1;

	cp = &m_TE->TimeLine->Channel[CHANNEL_EVENT];

	if (cp->KeysSelected[0] >= 0 && cp->KeysSelected[1] == -1)
		{
		if (cp->KeyData[cp->KeysSelected[0]].String)
			{	
			
//			  m_TE->pCTimeLine->m_EventText.SetWindowText(cp->KeyData[cp->KeysSelected[0]].String);
			  strcpy (Temp,cp->KeyData[cp->KeysSelected[0]].String);
			  ptr = strstr(Temp,";");
			  if (ptr==NULL)
				{
					m_TE->pCTimeLine->m_EventText.SetWindowText(cp->KeyData[cp->KeysSelected[0]].String);
				}
			  else
				{   m_TE->pCTimeLine->m_EventText.SetWindowText((ptr+1));
					sscanf (Temp,"%s %d",ObjectName,&ObjectId);
					m_ObjectMessage.SetCurSel( ObjectId);
					m_ObjectName.SelectString(-1,ObjectName);
				}

			}
		else
			{	m_TE->pCTimeLine->m_EventText.SetWindowText("");
				m_TE->pCTimeLine->m_ObjectName.SetCurSel(-1);
				m_TE->pCTimeLine->m_ObjectMessage.SetCurSel(-1);
			}
		}

	UpdateChangeText=0;
}


void CTimeLine::SetEventProperties() 
{
	Channel *cp;
	char EventText[3*512];
	char ObjectName[512];
	char text[512];
	int  MessageId;

	if (UpdateChangeText==1) return;
	
	UpdateChangeText=1;

	cp = &m_TE->TimeLine->Channel[CHANNEL_EVENT];

	m_EventText.GetWindowText(EventText, 512);

	if (m_ObjectName.GetCurSel()!=-1)
		m_ObjectName.GetLBText(m_ObjectName.GetCurSel() , ObjectName );
	else
		strcpy (ObjectName,"");

	ObjectName[511]='\0';		// to be save :)
	EventText[511] ='\0';		// to be save :)

	MessageId = m_ObjectMessage.GetCurSel( );

	if ((MessageId == -1)||(m_ObjectName.GetCurSel() == -1))
			 sprintf (text,"%s",EventText);
		else sprintf (text,"%s %d;%s",ObjectName, MessageId ,EventText);

	if (cp->KeysSelected[0] >= 0 && cp->KeysSelected[1] == -1)
		{
		if (strlen(text) >= 256)
			{
			// if greater than 256 then fall back to deallocate and strdup'ing
			if (cp->KeyData[cp->KeysSelected[0]].String)
				{
				jeRam_Free(cp->KeyData[cp->KeysSelected[0]].String);
				}

			cp->KeyData[cp->KeysSelected[0]].String = Util_StrDup(text);
			}
		else
			{
			// if less than 256 then allocate a fixed buffer for this
			if (!cp->KeyData[cp->KeysSelected[0]].String)
				{
				cp->KeyData[cp->KeysSelected[0]].String = (char*)jeRam_Allocate(256);
				}

			// just do a copy - we're garanteed to have room
			strcpy(cp->KeyData[cp->KeysSelected[0]].String, text);
			}

		Callback_ReturnData(m_TE->TimeLine, CHANNEL_EVENT);
		}

	UpdateChangeText=0;
}

void CTimeLine::OnTimelineZoomIn() 
{
	TimeEdit_ZoomIn(m_TE);
}

void CTimeLine::OnTimelineZoomOut() 
{
	TimeEdit_ZoomOut(m_TE);
}

void CTimeLine::OnTimelineReset() 
{
	TimeEdit_ZoomReset(m_TE);
}

void CTimeLine::OnClose() 
{
	ShowWindow( SW_HIDE );
	return;	
	//CDialog::OnClose();
}


void CTimeLine::OnTimelineMode() 
{
	int playmode;

	UpdateData(TRUE);
	playmode = m_PlayModeValue;
	UpdateData(FALSE);

	switch (playmode)
		{
		case 0:
			Callback_SetPlayMode(m_TE, JE_FALSE);
			m_PlayMode.SetWindowText("Play");
			m_PlayMode.InvalidateRect(NULL, FALSE);
			break;
		case 1:
			Callback_SetPlayMode(m_TE, JE_TRUE);
			m_PlayMode.SetWindowText("Stop");
			m_PlayMode.InvalidateRect(NULL, FALSE);
			break;
		}
}
void CTimeLine::OnSMS() 
{
	bSMS.SetIcon  ( IDI_TIMECOLOR);
	bHHMMSS.SetIcon  ( IDI_TIME);
	GetDlgItem( IDC_TIMELINE_SMS    )->InvalidateRect(NULL, FALSE);
	GetDlgItem( IDC_TIMELINE_HHMMSS )->InvalidateRect(NULL, FALSE);
	this->m_TE->TimeMode = 0;
	::InvalidateRect(m_TE->hwnd, NULL, FALSE);
}

void CTimeLine::OnHHMMSS() 
{
	bSMS.SetIcon  ( IDI_TIME);
	bHHMMSS.SetIcon  ( IDI_TIMECOLOR);
	GetDlgItem( IDC_TIMELINE_SMS    )->InvalidateRect(NULL, FALSE);
	GetDlgItem( IDC_TIMELINE_HHMMSS )->InvalidateRect(NULL, FALSE);
	this->m_TE->TimeMode = 1;
	::InvalidateRect(m_TE->hwnd, NULL, FALSE);
}


void CTimeLine::OnMoveAll() 
{

	switch (this->m_TE->MoveAll)
		{
		case 0:
			bMoveAll.SetIcon  ( IDI_LOCK);
			GetDlgItem( IDC_TIMELINE_MOVEALL )->SetWindowText("Connect");
			GetDlgItem( IDC_TIMELINE_MOVEALL )->InvalidateRect(NULL, FALSE);
			this->m_TE->MoveAll = 1;
			break;
		case 1:
			bMoveAll.SetIcon  ( IDI_UNLOCK);
			GetDlgItem( IDC_TIMELINE_MOVEALL )->SetWindowText("Single");
			GetDlgItem( IDC_TIMELINE_MOVEALL )->InvalidateRect(NULL, FALSE);
			this->m_TE->MoveAll = 0;
			break;
		}
}


void CTimeLine::OnTimelineMod() 
{
	int playmode;

	UpdateData(TRUE);

	if (m_PlayModeValue)
		m_PlayModeValue=0;
	else
		m_PlayModeValue=1;

	playmode = m_PlayModeValue;

	UpdateData(FALSE);

	switch (playmode)
		{
		case 0:
			m_PlayMode.InvalidateRect(NULL, FALSE);
			bPlay.SetIcon  ( IDI_PLAY);

			Callback_SetPlayMode(m_TE, JE_FALSE);
			m_PlayMode.SetWindowText("Play");
			GetDlgItem( IDC_TIMELINE_MOD )->SetWindowText("Play");
			break;
		case 1:
			GetDlgItem( IDC_TIMELINE_MOD )->SetWindowText("Stop");
			bPlay.SetIcon  ( IDI_PLAYING);

			Callback_SetPlayMode(m_TE, JE_TRUE);
			m_PlayMode.SetWindowText("Stop");
			m_PlayMode.InvalidateRect(NULL, FALSE);
			break;
		}
}

void CTimeLine::TimelineModeSet(int Mode) 
{
	switch (Mode)
		{
		case 0:
			m_PlayMode.SetWindowText("Play");
			m_PlayMode.SetCheck(0);
			m_PlayMode.InvalidateRect(NULL, FALSE);
			break;
		case 1:
			m_PlayMode.SetWindowText("Stop");
			m_PlayMode.SetCheck(1);
			m_PlayMode.InvalidateRect(NULL, FALSE);
			break;
		}
}

void CTimeLine::OnTimelineCut() 
{
	// TODO: Add your control notification handler code here
	
}

void CTimeLine::OnTimelineCutPoint() 
{
	// TODO: Add your control notification handler code here
/*	if (m_InsertCut) m_InsertCut=0;
	else m_InsertCut=1;*/
	if (m_InsertCut.GetCheck())
			m_InsertCut.SetCheck(0);
		else m_InsertCut.SetCheck(1);
}


// Added JH 
void TimeEdit_DrawLine (HDC hdc,int left, int top, int right, int bottom,DWORD Color1, DWORD Color2)
{
	HPEN hPen;

	if (left==right)
		{
		hPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, Color2));
		MoveToEx(hdc,left,top,NULL);
		LineTo(hdc,right,bottom);
		DeleteObject(SelectObject(hdc, hPen));
		
		hPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, Color1));
		MoveToEx(hdc,left+1,top,NULL);
		LineTo(hdc,right+1,bottom);
		DeleteObject(SelectObject(hdc, hPen));
		}
	else
		{
		hPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, Color2));
		MoveToEx(hdc,left,top,NULL);
		LineTo(hdc,right,bottom);
		DeleteObject(SelectObject(hdc, hPen));
		
		hPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, Color1));
		MoveToEx(hdc,left,top+1,NULL);
		LineTo(hdc,right,bottom+1);
		DeleteObject(SelectObject(hdc, hPen));
		}

}

void TimeEdit_DrawRect (HDC hdc,int left, int top, int right, int bottom,DWORD Color1, int Type)
{
	HPEN hPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, Color1));
	HBRUSH hBrush = (HBRUSH)SelectObject(hdc, CreateSolidBrush(Color1));

// Draw Background
	Rectangle (hdc,right,top,left,bottom);

	DeleteObject(SelectObject(hdc, hPen));
	DeleteObject(SelectObject(hdc, hBrush));

	if (Type)
		 hPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, RGB(255,255,255)));
	else hPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, RGB(0,0,0)));

// Draw Line
	MoveToEx(hdc,left,bottom,NULL);
	LineTo(hdc,left,top);
	LineTo(hdc,right,top);
	DeleteObject(SelectObject(hdc, hPen));

	if (Type)
	  	 hPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, RGB(0,0,0)));
	else hPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 0, RGB(255,255,255)));
	LineTo(hdc,right,bottom);
	LineTo(hdc,left,bottom);
	DeleteObject(SelectObject(hdc, hPen));
}