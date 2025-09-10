///////////////////////////////////////////////////////////////////////////////////
//	jMinApp ver. 1.0.b for Jet3D ver.2.x
//	July 23, 2002
//	by Tom Morris
//
//	The target for this application is PROGRAMMERS interested in  real time
//	3D rendering application development. jMinApp is an application shell that
//	exploits the rendering capabilities of Jet3D -- a real time rendering engine.
//	jMinApp demonstrates how to start your own custom applcation shell (game shell)
//	for Jet3D. jMinApp shows how to do this as an MFC application. This MFC approach
//	makes all the huge power of MFC available to you including a large variety
//	of container classes, serialization, document management, and much more...
//
//	You may freely modify this code to transform it into anything you want. Or use bits 
//	of it to build your own, unique shell.
//
//	Good luck and have fun.
//	-Tom
//	www.jet3d.com
/////////////////////////////////////////////////////////////////////////////////////


// jMinApp.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "jMinApp.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CjMinAppApp

BEGIN_MESSAGE_MAP(CjMinAppApp, CWinApp)

END_MESSAGE_MAP()


// CjMinAppApp construction

CjMinAppApp::CjMinAppApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CjMinAppApp object

CjMinAppApp theApp;

// CjMinAppApp initialization

BOOL CjMinAppApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME/*, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL*/);
	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOWMAXIMIZED);

	pFrame->UpdateWindow();


	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}




