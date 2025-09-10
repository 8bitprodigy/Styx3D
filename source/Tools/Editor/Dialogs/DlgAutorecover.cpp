/****************************************************************************************/
/*  DlgAutorecover.cpp                                                                        */
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
#include "stdafx.h"
#include "jwe.h"
#include "DlgAutorecover.h"
#include ".\dlgautorecover.h"
#include "Doc.h"
#include "MainFrm.h"


// CDlgAutorecover dialog

IMPLEMENT_DYNAMIC(CDlgAutorecover, CDialog)
CDlgAutorecover::CDlgAutorecover(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAutorecover::IDD, pParent)
	
{
	m_bFindFile = FALSE;
}

CDlgAutorecover::~CDlgAutorecover()
{
}

void CDlgAutorecover::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_AUTORECOVER, m_staticAutorecover);
}


BEGIN_MESSAGE_MAP(CDlgAutorecover, CDialog)
	ON_BN_CLICKED(ID_AUTORECOVER_DELETE, OnBnClickedAutorecoverDelete)
	ON_BN_CLICKED(ID_AUTORECOVER_OPEN, OnBnClickedAutorecoverOpen)
END_MESSAGE_MAP()


// CDlgAutorecover message handlers

BOOL CDlgAutorecover::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	CString strMessage = _T("Autosaved file found...\n");
	strMessage += _T("Preparing to AutoRecover this file...\n\n");
	strMessage += _T("It is recommended that you choose to open this file.\n");
	strMessage += _T("Otherwise choose to delete this file... permanently.\n");

	m_staticAutorecover.SetWindowText(strMessage);
	UpdateData(FALSE);

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


bool	CDlgAutorecover::SquareAwayAutosave()
{
	//	Establish autosave directory
	char		buffer[MAX_PATH +1];
	CString		base;
	int			iAutosaveMinutes = 1;
	BOOL		bFoundFile = FALSE;

	Settings_SetAutosaveMinutes(iAutosaveMinutes);

	if (GetWindowsDirectory(buffer,MAX_PATH) == 0)
		DWORD lastError = ::GetLastError();
	else
		m_strWindowsDir = buffer;

	if (m_fileFind.FindFile(m_strWindowsDir,0))
	{
		base = buffer;
	}
	else
		base = "C:\\";

	m_strAutosaveDirectory = base+"\\TEMP";
	Settings_SetAutosaveDirectory(m_strAutosaveDirectory);

	CString fileWildCard = m_strAutosaveDirectory +  _T("\\*.JDB");

	bFoundFile = m_fileFind.FindFile(fileWildCard.GetBuffer(1),0);
	if (bFoundFile)
	{
		ShowWindow(SW_SHOW);
		BringWindowToTop();
		return true;
	}
	return true;
}



void CDlgAutorecover::OnBnClickedAutorecoverDelete()
{	
	ShowWindow(SW_HIDE);

	while(m_fileFind.FindNextFile() !=0)
	{
		DeleteFile(m_fileFind.GetFilePath().GetBuffer(1));
	}	

	DeleteFile(m_fileFind.GetFilePath().GetBuffer(1));

	SetToAppDirectory();
}



void CDlgAutorecover::OnBnClickedAutorecoverOpen()
{
	ShowWindow(SW_HIDE);

	BOOL		bFileFound = TRUE;
	CJweDoc		*pDoc = NULL;
	CString		strLevelPath;
	char		charAppPath[MAX_PATH];
	char		*pcharAppPath = charAppPath;

	((CJweApp*)AfxGetApp())->GetAppPath(pcharAppPath, MAX_PATH);
	
	strLevelPath = charAppPath;
#if _MFC_VER < 0x0700
	strLevelPath += _T("\\Levels\\");
#else
	strLevelPath.Append(_T("\\Levels\\"));
#endif

	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	while(bFileFound)
	{
		bFileFound = m_fileFind.FindNextFile();
		CString strFilePathJDB, strRenamedFile, strNewPath;;
		strFilePathJDB = m_fileFind.GetFilePath();
		strRenamedFile = m_fileFind.GetFileName();

		strRenamedFile.Delete(strRenamedFile.GetLength()-4, 4);
#if _MFC_VER < 0x0700
		strRenamedFile += _T("_BAK.j3d");
#else
		strRenamedFile.Append(_T("_BAK.j3d"));
#endif
		strNewPath = strLevelPath + strRenamedFile;

		if (::MoveFileEx(strFilePathJDB, strNewPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
		{
			if (pDoc)
			{
				pDoc->OnCloseDocument();
				pDoc = (CJweDoc *)AfxGetApp()->OpenDocumentFile(strNewPath);
			}
		}
	}
	SetToAppDirectory();
}


void	CDlgAutorecover::SetToAppDirectory()
{
	char	charAppPath[MAX_PATH];
	char	*pcharAppPath = charAppPath;
	((CJweApp*)AfxGetApp())->GetAppPath(pcharAppPath, MAX_PATH);

	::SetCurrentDirectory(pcharAppPath);

}
