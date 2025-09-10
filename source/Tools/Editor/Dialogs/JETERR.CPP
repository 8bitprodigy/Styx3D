/****************************************************************************************/
/*  JETERR.CPP                                                                          */
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
#include "jeterr.h"
#include "errorlog.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


jeBoolean HasValidError( int32 CurErrorN )
{
	jeErrorLog_ErrorClassType Error = JE_ERR_INTERNAL_RESOURCE;
	const char *UserString;
	const char *Context = NULL;

	for( ;CurErrorN > 0; CurErrorN-- )
	{
		if( jeErrorLog_Report(CurErrorN-1, &Error, &UserString, &Context) )
		{
			if( Error >= JE_ERR_MEMORY_RESOURCE && Error <= JE_ERR_SUBSYSTEM_FAILURE )
				return( JE_TRUE );
		}
	}
	return( JE_FALSE );
}


jeBoolean ReportErrors( jeBoolean bWarning )
{
	CJetErr ErrorDialog;
	int ErrorN;

	ErrorDialog.bWarning = bWarning;
	ErrorN = jeErrorLog_Count();
	if( ErrorN == 0 )
		return( JE_TRUE );

	ErrorDialog.m_CurErrorN = ErrorN;
	while( ErrorDialog.m_CurErrorN  )
	{
		if( !HasValidError(ErrorDialog.m_CurErrorN) )
			break;
		ErrorDialog.DoModal();
	}
	jeErrorLog_Clear();
	return( JE_TRUE );
}

/////////////////////////////////////////////////////////////////////////////
// CJetErr dialog


CJetErr::CJetErr(CWnd* pParent /*=NULL*/)
	: CDialog(CJetErr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CJetErr)
	m_Error = _T("");
	m_Log = _T("");
	//}}AFX_DATA_INIT
	m_CurErrorN = 0;
}


void CJetErr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJetErr)
	DDX_Control(pDX, IDC_WARNINGICON, m_Icon);
	DDX_Text(pDX, IDC_ERROR, m_Error);
	DDX_Text(pDX, IDC_LOG, m_Log);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJetErr, CDialog)
	//{{AFX_MSG_MAP(CJetErr)
	ON_BN_CLICKED(IDC_MORE, OnMore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJetErr message handlers

BOOL CJetErr::OnInitDialog() 
{
	jeErrorLog_ErrorClassType Error = JE_ERR_INTERNAL_RESOURCE;
	const char *UserString;
	const char *Context = NULL;
	char *ErrString;
	RECT Rect;

	CDialog::OnInitDialog();
	
	GetWindowRect( &Rect );
	Rect.bottom = Rect.top + 160;
	MoveWindow( &Rect ); 
	UpdateData( TRUE );
	if( bWarning )
		m_Icon.SetIcon( LoadIcon( NULL, IDI_EXCLAMATION ) );
	else
		m_Icon.SetIcon( LoadIcon( NULL, IDI_HAND ) );

	for( ;m_CurErrorN > 0; m_CurErrorN-- )
	{
		if( jeErrorLog_Report(m_CurErrorN-1, &Error, &UserString, &Context) )
		{
			if( UserString )
				m_Log += UserString;
			if (Context)
				if( Context[0] != 0 )
					{
						// [MLB-ICE] NOTE:Strings are small, but they have a right to be free ;)
						//m_Log += Util_LoadLocalRcString( IDS_CONTEXT_OPEN );
						//m_Log += Context;
						//m_Log += Util_LoadLocalRcString( IDS_CONTEXT_CLOSE );
						ErrString = Util_LoadLocalRcString( IDS_CONTEXT_OPEN );
						m_Log += ErrString;
						if ( ErrString )
							jeRam_Free(ErrString);

						m_Log += Context;

						ErrString = Util_LoadLocalRcString( IDS_CONTEXT_CLOSE );
						m_Log += ErrString;
						if ( ErrString )
							jeRam_Free(ErrString);
						// [MLB-ICE]
					}
			m_Log += "\r\n";
			if( Error != JE_ERR_SUBSYSTEM_FAILURE && Error != JE_ERR_INTERNAL_RESOURCE )
				break;
		}
	}
	if( m_CurErrorN > 0 )
	{
		if( Error >= JE_ERR_MEMORY_RESOURCE && Error <= JE_ERR_SUBSYSTEM_FAILURE )
		{
			ErrString = Util_LoadLocalRcString( Error + IDS_MEMORY_ERR );
			if( ErrString )
			{
				m_Error = ErrString;
				m_Error += " ";
				jeRam_Free( ErrString );
			}
			if ( (Error >= JE_ERR_FILEIO_OPEN)  && (Error <= JE_ERR_FILEIO_VERSION) )
			{
				if( Context )
				{
					m_Error += Context;
				}
			}
		}
		else
		{
			ErrString = Util_LoadLocalRcString( IDS_UNKNOWN_ERR );
			if( ErrString )
			{
				m_Error = ErrString;
				m_Error += " ";
				jeRam_Free( ErrString );
			}
		}
		m_CurErrorN--;
	}
	else
	{
		ErrString = Util_LoadLocalRcString( JE_ERR_SUBSYSTEM_FAILURE + IDS_MEMORY_ERR);
		if( ErrString )
		{
			m_Error = ErrString;
			jeRam_Free( ErrString );
		}
		if( Context )
			m_Error += Context;
	}

	UpdateData( FALSE );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CJetErr::OnMore() 
{
	RECT Rect;

	
	GetWindowRect( &Rect );
	Rect.bottom = Rect.top + 340;
	MoveWindow( &Rect ); 
}
