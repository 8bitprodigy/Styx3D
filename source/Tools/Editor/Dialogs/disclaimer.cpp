/****************************************************************************************/
/*  DISCLAIMER.CPP                                                                      */
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
#include "disclaimer.h"
#include "util.h"
#include "ram.h"
#include "spawn.h"
#include "BuildNumber.h"
#include "BuildType.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Disclaimer dialog


Disclaimer::Disclaimer(CWnd* pParent /*=NULL*/)
	: CDialog(Disclaimer::IDD, pParent)
{
	//{{AFX_DATA_INIT(Disclaimer)
	m_Show		= 0;
	m_AboutText = _T("");
	m_BuildInfoText = _T("");
	//}}AFX_DATA_INIT
}

void Disclaimer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Disclaimer)
	DDX_Check(pDX, IDC_SHOW, m_Show);
	DDX_Text(pDX, IDC_EDIT1, m_AboutText);
	DDX_Text(pDX, IDC_TEXT_BUILDINFO, m_BuildInfoText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Disclaimer, CDialog)
	//{{AFX_MSG_MAP(Disclaimer)
	ON_BN_CLICKED(IDC_TUTORIAL, OnTutorial)
//	ON_BN_CLICKED(IDC_TUTORIAL2, OnTutorial2)
	ON_BN_CLICKED(IDC_DEMO, OnDemo)
	ON_BN_CLICKED(IDC_DEMO2, OnDemo2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Disclaimer message handlers

BOOL Disclaimer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	char * AboutText;

	AboutText = Util_LoadText( IDR_TEXT1 );
	if( AboutText != NULL )
	{
		UpdateData( true );
		m_AboutText = AboutText;
		jeRam_Free( AboutText );
		m_BuildInfoText = _T(BUILD_TYPE "   " BUILD_NUMBER);
		UpdateData( false );
	}

	CMainFrame			*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;	
	
	GetDlgItem(IDC_STATIC_PROP)->SetFont( &pMainFrm->cBigFont, true);

	GetDlgItem(IDC_EDIT1)->SetFont( &pMainFrm->cSmallFont, true);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void Disclaimer::OnTutorial() 
{
	//WinExec("Notepad ..\\Tutorial.txt",SW_SHOW);
	//Spawn_App( "..\\tutorials\\tutorials.htm", NULL, Spawn_Async ); // by trilobite
	Spawn_App("https://www.jet3d.com/tutorials/tutorial_jedit_1.html", NULL, Spawn_Async);
	EndDialog( IDOK );
	
}


void Disclaimer::OnDemo() 
{
	CWinApp *pApp = NULL;
	CMainFrame	*pMainFrame = NULL;

	pApp = AfxGetApp();
	pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pApp)
	{
		pApp->OpenDocumentFile( "dancer.j3d" );

		if( pMainFrame )
		{
			pMainFrame->PostMessage( WM_COMMAND, IDM_FULLSCREEN_VIEW, 0 );
		}
	}
	
}

void Disclaimer::OnDemo2() 
{
	CWinApp *pApp = NULL;
	CMainFrame	*pMainFrame = NULL;

	pApp = AfxGetApp();
	pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pApp)
	{
		pApp->OpenDocumentFile( "tutorial2_complete.j3d" );

		if( pMainFrame )
		{
			pMainFrame->PostMessage( WM_COMMAND, IDM_FULLSCREEN_VIEW, 0 );
		}
	}
}
