// ActorWorkbench.h : main header file for the ActorWorkbench application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CActorWorkbenchApp:
// See ActorWorkbench.cpp for the implementation of this class
//

class CActorWorkbenchApp : public CWinApp
{
public:
	CActorWorkbenchApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CActorWorkbenchApp theApp;