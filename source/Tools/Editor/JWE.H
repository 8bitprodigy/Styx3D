/****************************************************************************************/
/*  JWE.H                                                                               */
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

/* Open Source Revision -----------------------------------------------------------------
 By: Dennis Tierney (DJT) dtierney@oneoverz.com
 On: 12/27/99 9:01:22 PM
 Comments: Added mouse settings member.
----------------------------------------------------------------------------------------*/

#if !defined(AFX_GWE_H__37F4562E_C0E1_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_GWE_H__37F4562E_C0E1_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "AppData.h"
#include "J3DApp.h"
#include "Settings.h"
#include "MaterialList.h"
#include "WndReg.h"
#include "MsgLog.h"
#include "DlgAutorecover.h"
#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CJweApp:
// See gwe.cpp for the implementation of this class
//

class CJweApp : public CJ3DApp
{
public:
	AppData * m_pAppData;
	Settings * m_pSettings;

//---------------------------------------------------
// Added DJT
//---------------------------------------------------
	MouseSettings *m_pMouseSettings;
//---------------------------------------------------
// End DJT
//---------------------------------------------------

	WindowRegister *m_WndReg;
	MsgLog *		m_pMsgLog;
	CJweApp();
	MaterialList_Struct *GetMaterialList();

	void StartLogRecord();
	void EndLogRecord();
	void StartLogPlay();
	void EndLogPlay();
	jeBoolean GetAppPath( char * PathString, int BuffSize );
	BOOL InitObjects( char * DllPath);
	//	by tom morris feb 2005
	bool	RefreshMaterialList();
	//	end tom morris
    // Redefine the Registry key to the short application name
    void SetRegistryKey(LPCTSTR lpszRegistryKey);

private:
	MaterialList_Struct *MaterialList;

	BOOL RegisterObject( HINSTANCE	DllHandle );

	BOOL m_bMaterialListInitialised;

	//	tom morris may 2005
	bool	SquareAwayAutosave();
	CDlgAutorecover	m_dlgAutoRecover;
	//	end tom morris may 2005

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJweApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

public:// Implementation
	BOOL			GetDriverAndMode(jeEngine* pEngine, jeDriver **ppDriver, jeDriver_Mode **ppMode) ;
	const char*		GetDriverPath(void) ;

	// Krouer
	BOOL  HasInitMaterialList() { return m_bMaterialListInitialised; };

	//	by tom morris feb 2005 - moved to public by Krouer
	bool	InitMaterialList(jeEngine* pEngine, jeResourceMgr* pResMgr);
	//	end tom morris

	CString m_strDriverPath;

	COleTemplateServer m_server;
		// Server object for document creation
	//{{AFX_MSG(CJweApp)
	afx_msg void OnAppAbout();

	//	tom morris feb 2005
	afx_msg void OnProgressBarBegin(WPARAM, LPARAM);
	afx_msg void OnProgressBarSetRange(WPARAM, LPARAM);
	afx_msg void OnProgressBarSetStep(WPARAM, LPARAM);
	afx_msg void OnProgressBarStep(WPARAM, LPARAM);
	afx_msg void OnProgressBarEnd(WPARAM, LPARAM);
	//	end tom morris feb 2005

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//	by trilobite jan. 2011
extern CJweApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GWE_H__37F4562E_C0E1_11D2_8B41_00104B70D76D__INCLUDED_)
