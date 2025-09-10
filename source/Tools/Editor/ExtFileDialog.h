/****************************************************************************************/
/*  ExtFileDialog.H                                                                     */
/*                                                                                      */
/*  Author:			J.Hellmann                                                          */
/*  Description:    New Filedialog with Level info and preview                          */
/****************************************************************************************/

#if !defined(AFX_EXTFILEDIALOG_H__D4222D6E_A188_43DB_B2B9_2918F147DBEE__INCLUDED_)
#define AFX_EXTFILEDIALOG_H__D4222D6E_A188_43DB_B2B9_2918F147DBEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "DrawTool.h"
#include "Label.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CExtFileDialog 

class CExtFileDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CExtFileDialog)

public:
	CExtFileDialog(BOOL bOpenFileDialog, // TRUE für FileOpen, FALSE für FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL,
		int preview=false
		);
	
	CExtFileDialog (BOOL bOpenFileDialog, BOOL Preview);

	int		m_preview;
	LPBITMAPINFOHEADER	lpbiPreview;
	char		*		PreviewData	;

protected:
	//{{AFX_MSG(CExtFileDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

		
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnFileNameChange();
	virtual void OnFolderChange();

	CLabel			m_stRect;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_EXTFILEDIALOG_H__D4222D6E_A188_43DB_B2B9_2918F147DBEE__INCLUDED_
