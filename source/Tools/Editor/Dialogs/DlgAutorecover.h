/****************************************************************************************/
/*  DlgAutorecover.h                                                                        */
/*                                                                                      */
/*  Author:  tom morris - may 2005                                                                           */
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

#pragma once
#include "afxwin.h"


// CDlgAutorecover dialog

class CDlgAutorecover : public CDialog
{
	DECLARE_DYNAMIC(CDlgAutorecover)

public:
	CDlgAutorecover(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgAutorecover();

// Dialog Data
	enum { IDD = IDD_DLG_AUTORECOVER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
//	CString  m_staticAutorecover;
public:
bool	SquareAwayAutosave();
	virtual BOOL OnInitDialog();
private:
	CStatic m_staticAutorecover;
	CFileFind	m_fileFind;
	CString		m_strWindowsDir;
	CString		m_strAutosaveDirectory;
	BOOL		m_bFindFile;

	void	SetToAppDirectory();
public:
	afx_msg void OnBnClickedAutorecoverDelete();
	afx_msg void OnBnClickedAutorecoverOpen();
};
