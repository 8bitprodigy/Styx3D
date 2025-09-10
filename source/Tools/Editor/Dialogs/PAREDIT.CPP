/****************************************************************************************/
/*  PAREDIT.CPP                                                                         */
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
//#include "ctrltest.h"

#include "paredit.h"

/////////////////////////////////////////////////////////////////////////////
// ParsedEdit

CParsedEdit::CParsedEdit()
{
	m_wParseStyle = 0;
}

BEGIN_MESSAGE_MAP(CParsedEdit, CEdit)
	//{{AFX_MSG_MAP(CParsedEdit)
	ON_WM_CHAR()
	ON_WM_VSCROLL()     // for associated spin controls
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Creating from C++ code

BOOL CParsedEdit::Create(DWORD dwStyle, const RECT& rect,
		CWnd* pParentWnd, UINT nID)
{
	m_wParseStyle = LOWORD(dwStyle);
	// figure out edit control style
	DWORD dwEditStyle = MAKELONG(ES_LEFT, HIWORD(dwStyle));
	return CWnd::Create(_T("EDIT"), NULL, dwEditStyle, rect, pParentWnd, nID);
}

/////////////////////////////////////////////////////////////////////////////
// Aliasing on top of an existing Edit control

BOOL CParsedEdit::SubclassEdit(UINT nID, CWnd* pParent, WORD wParseStyle)
{
	m_wParseStyle = wParseStyle;
	return SubclassDlgItem(nID, pParent);
}

/////////////////////////////////////////////////////////////////////////////
// Input character filter

void CParsedEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	WORD type;

	if (nChar < 0x20)
		type = PES_ALL;                         // always allow control chars
	else if( nChar == 0x20 )
		type = PES_SPACE ;
	else if (IsCharAlphaNumeric((TCHAR)nChar) && !IsCharAlpha((TCHAR)nChar))
		type = PES_NUMBERS;
	else if (IsCharAlpha((TCHAR)nChar))
		type = PES_LETTERS;
	else
		type = PES_OTHERCHARS;

	if (m_wParseStyle & type)
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);  // permitted
	}
	else
	{
		// illegal character - inform parent
		OnBadInput();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Spin controls will send scroll messages

void CParsedEdit::OnVScroll(UINT nSBCode, UINT, CScrollBar*)
{
	int nDelta = 0;
	if (nSBCode == SB_LINEDOWN)
		nDelta = -1;
	else if (nSBCode == SB_LINEUP)
		nDelta = +1;
	else
		return; // nothing special

	// set the focus to this edit item and select it all
	SetFocus();

	//Get the number in the control.
	BOOL bOk;
	int nOld = GetParent()->GetDlgItemInt(GetDlgCtrlID(), &bOk);
	if (bOk)
	{
		// The MuScrl32 control also supports range checking
		// for this example, we just prevent overflow
		int nNew = nOld + nDelta;
		if (nNew >= 0 && nNew <= 32767)
			GetParent()->SetDlgItemInt(GetDlgCtrlID(), nNew);
		else
			bOk = FALSE;
	}

	if (!bOk)
		OnBadInput();
	SetSel(0, -1);
}

/////////////////////////////////////////////////////////////////////////////
// default bad input handler, beep (unless parent notification
//    returns -1.  Most parent dialogs will return 0 or 1 for command
//    handlers (i.e. Beep is the default)

void CParsedEdit::OnBadInput()
{
	if (GetParent()->SendMessage(WM_COMMAND,
		MAKELONG(GetDlgCtrlID(), PEN_ILLEGALCHAR), (LPARAM)m_hWnd) != -1)
	{
		MessageBeep((UINT)-1);
	}
}

/////////////////////////////////////////////////////////////////////////////
