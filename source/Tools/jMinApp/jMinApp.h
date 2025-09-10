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


// jMinApp.h : main header file for the jMinApp application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CjMinAppApp:
// See jMinApp.cpp for the implementation of this class
//

class CjMinAppApp : public CWinApp
{
public:
	CjMinAppApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	DECLARE_MESSAGE_MAP()
};

extern CjMinAppApp theApp;