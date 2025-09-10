/****************************************************************************************/
/*  PAREDIT.H                                                                           */
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
/////////////////////////////////////////////////////////////////////////////
// CParsedEdit is a specialized CEdit control that only allows characters
//  of a given type.
// This class is used in 3 different ways in the samples
#pragma once

class CParsedEdit : public CEdit
{
protected:
	WORD    m_wParseStyle;      // C++ member data
public:
// Construction
	CParsedEdit();

	// explicit construction (see DERTEST.CPP)
	BOOL Create(DWORD dwStyle /* includes PES_ style*/, const RECT& rect,
		CWnd* pParentWnd, UINT nID);

	// subclassed construction (see SUBTEST.CPP)
	BOOL SubclassEdit(UINT nID, CWnd* pParent, WORD wParseStyle);

	// for WNDCLASS Registered window
	static BOOL RegisterControlClass();

// Overridables
	virtual void OnBadInput();

// Implementation
protected:
	//{{AFX_MSG(CParsedEdit)
	afx_msg void OnChar(UINT, UINT, UINT); // for character validation
	afx_msg void OnVScroll(UINT, UINT, CScrollBar*); // for spin buttons
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////
// Parsed edit control sub-styles

#define PES_NUMBERS         0x0001
#define PES_LETTERS         0x0002
#define PES_OTHERCHARS      0x0004
#define PES_SPACE			0x0008
#define PES_ALL             0xFFFF

/////////////////////////////////////////////////////////////////////////////
// Extra control notifications

// above the range for normal EN_ messages
#define PEN_ILLEGALCHAR     0x8000
			// sent to parent when illegal character hit
			// return 0 if you want parsed edit to beep

/////////////////////////////////////////////////////////////////////////////
