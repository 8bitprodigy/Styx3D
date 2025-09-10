/****************************************************************************************/
/*  BUILDEREDIT.CPP                                                                     */
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
#include "builderedit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#include "MainFrm.h"
#include "jeProperty.h"
#include <float.h>

IMPLEMENT_DYNCREATE(CBuilderEdit, CEdit)

/////////////////////////////////////////////////////////////////////////////
// CBuilderEdit

CBuilderEdit::CBuilderEdit(int DataType, int DataId )
{
	m_DataType = DataType;
	m_DataId = DataId;
	m_Min = FLT_MIN;
	m_Max = FLT_MAX;
	m_Increment = 1.0f;
}

CBuilderEdit::~CBuilderEdit()
{
}


BEGIN_MESSAGE_MAP(CBuilderEdit, CEdit)
	//{{AFX_MSG_MAP(CBuilderEdit)
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuilderEdit message handlers


void CBuilderEdit::SetProperty()
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	jeProperty_Data Data;
	char	  TempBuffer[265];

	GetWindowText(TempBuffer, 256 );
	switch( m_DataType )
	{
	case STRING_TYPE:
		Data.String = TempBuffer;
		pDoc->SetProperty( m_DataId, m_DataType, &Data );
		break;

	case INT_STRING_TYPE:
		Data.Int = atoi( TempBuffer);
		if( Data.Int > (int)m_Max )
			Data.Int = (int)m_Max;
		if( Data.Int < (int)m_Min )
			Data.Int = (int)m_Min;
		pDoc->SetProperty( m_DataId, m_DataType, &Data );
		break;

	case FLOAT_STRING_TYPE:
		Data.Float = (float)atof(TempBuffer);
		if( Data.Float > m_Max )
			Data.Float = m_Max;
		if( Data.Float < m_Min )
			Data.Float = m_Min;
		pDoc->SetProperty( m_DataId, m_DataType, &Data );
		break;
	}
}
void CBuilderEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	
	SetProperty();
}

void CBuilderEdit::SetValueRange( float min, float max, float increment )
{
	m_Min = min;
	m_Max = max;
	m_Increment = increment;
}

void CBuilderEdit::Increment( BOOL bDown )
{
	jeProperty_Data Data;
	char	  TempBuffer[265];
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	
	GetWindowText(TempBuffer, 256 );
	switch( m_DataType )
	{
	case STRING_TYPE:
		ASSERT( 0 );
		break;

	case INT_STRING_TYPE:
		Data.Int = atoi( TempBuffer);
		if( bDown )
		{
			Data.Int-= (int)m_Increment;
			if( Data.Int < (int)m_Min )
				Data.Int = (int)m_Min;
		}
		else
		{
			Data.Int += (int)m_Increment;
			if( Data.Int > (int)m_Max )
				Data.Int = (int)m_Max;
		}
		sprintf( TempBuffer, "%d", Data.Int );
		break;

	case FLOAT_STRING_TYPE:
		Data.Float = (float)atof( TempBuffer);
		if( bDown )
		{
			Data.Float -= m_Increment;
			if( Data.Float < m_Min )
				Data.Float = m_Min;
		}
		else
		{
			Data.Float += m_Increment;
			if( Data.Float > m_Max )
				Data.Float = m_Max;
		}
		CString TempString;
		TempString.Format( "%10.4g", Data.Float) ;
		TempString.TrimLeft(" ");
		strcpy (TempBuffer,(LPCTSTR) TempString);
		break;
	}
	SetWindowText( TempBuffer );
	pDoc->SetProperty( m_DataId, m_DataType, &Data );
}

void CBuilderEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	
	if( nChar == VK_RETURN )
	{
		SetProperty();
		return;
	}
	
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CBuilderEdit::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CEdit::OnCommand(wParam, lParam);
}
