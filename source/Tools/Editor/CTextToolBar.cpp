/****************************************************************************************/
/*  CTextToolBar.CPP                                                                    */
/*                                                                                      */
/*  Author:			Joachim Hellmann                                                    */
/*  Description:    Helper Class for  Toolbars with 'Subtitles'                         */
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
/****************************************************************************************/
#include "stdafx.h"
#include "CTextToolBar.h"

BOOL CTextToolBar::LoadToolBar(LPCTSTR lpszResourceName, int iXSize , int iYSize)
{
	iElementXSize = iXSize;
	iElementYSize = iYSize;

	BOOL bReturn = CToolBar::LoadToolBar(lpszResourceName);

	if (iXSize == 0) 
		return bReturn;

	// Check if we loaded the toolbar.
	if (bReturn == FALSE)
		return bReturn;

	// Make it flat.
	//ModifyStyle(0, GetStyle()|TBSTYLE_FLAT);

	// Set the text for each button
	CToolBarCtrl& bar = GetToolBarCtrl();


	// Remove the string map in case we are loading another toolbar into this control
	if (m_pStringMap)
	{
		delete m_pStringMap;
		m_pStringMap = nullptr;
	}

	int		nIndex = 0;
	TBBUTTON	tb;

	for (nIndex = bar.GetButtonCount() - 1; nIndex >= 0; nIndex--)
	{
		ZeroMemory(&tb, sizeof(TBBUTTON));
		bar.GetButton(nIndex, &tb);

		// Do we have a separator?
		if ((tb.fsStyle & TBSTYLE_SEP) ==  TBSTYLE_SEP)
			continue;

		// Have we got a valid command id?
		if (tb.idCommand == 0)
			continue;

		// Get the resource string if there is one.
		CString strText;
		LPCTSTR lpszButtonText = NULL;
		CString	strButtonText(_T(""));
		_TCHAR	seps[] = _T("\n");

		strText.LoadString(tb.idCommand);

		if (!strText.IsEmpty())
		{
			lpszButtonText = _tcstok((LPTSTR)(LPCTSTR)strText, seps);

			while(lpszButtonText)
			{
				strButtonText = lpszButtonText;
				lpszButtonText = _tcstok(nullptr, seps);
			}
		}

		if (!strButtonText.IsEmpty())
			SetButtonText(nIndex, strButtonText);
	}

	// Resize the buttons so that the text will fit.
	CRect rc(0, 0, 0, 0);
	CSize sizeMax(0, 0);

	for (nIndex = bar.GetButtonCount() - 1; nIndex >= 0; nIndex--)
	{
		bar.GetItemRect(nIndex, rc);

		rc.NormalizeRect();
		sizeMax.cx = __max(rc.Size().cx, sizeMax.cx);
		sizeMax.cy = __max(rc.Size().cy, sizeMax.cy);
	}


	SetSizes(sizeMax, CSize(iXSize,iYSize));
	return bReturn;
}

int	CTextToolBar::ChangeToolBar(CWnd *pObjectWnd, UINT sourceId, UINT destId, UINT Style, CFont *ObjectFont)
{
    int		index = 0;
    RECT	rect{};

    while(GetItemID(index)!=sourceId) index++;

    SetButtonInfo(index, sourceId, Style, 120);
    GetItemRect(index, &rect);

    if (iElementXSize>0) rect.bottom -= 15;

    if (Style!=0)
    { 
        rect.top += 2;
        rect.bottom += 15+200;
    }

    if (Style==0)
    { 
        if (!((CStatic*)pObjectWnd)->Create("",WS_CHILD|WS_VISIBLE | SS_LEFT | SS_SUNKEN|SS_LEFTNOWORDWRAP|SS_CENTERIMAGE,                                       
            rect,this, destId))

        {  
            TRACE0("Failed to create object\n");
            return -1;
        }
    }
    else if (!((CComboBox*)pObjectWnd)->Create(WS_CHILD|WS_VISIBLE | CBS_AUTOHSCROLL | 
        CBS_DROPDOWNLIST | CBS_HASSTRINGS ,
        rect, this, destId))
    {
        TRACE0("Failed to create combo-box\n");
        return -1;
    }

    ((CStatic*)pObjectWnd)->SetFont( ObjectFont, TRUE);    
    ((CStatic*)pObjectWnd)->EnableWindow(TRUE);
    ((CStatic*)pObjectWnd)->ShowWindow(SW_SHOW); 
    return TRUE;
}
// EOF JH