/****************************************************************************************/
/*  JWE.CPP                                                                             */
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
 Comments: Added creation and initialization of mouse properties settings.
----------------------------------------------------------------------------------------*/


#include "stdafx.h"
#include "jwe.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "Doc.h"
#include "DrvList.h"
#include "Util.h"
#include "View.h"
#include "ram.h"
#include "Disclaimer.h"
#include "vfile.h"
#include "Errorlog.h"
#include "jeVersion.h"
#include "BuildNumber.h"
#include "BuildType.h"
#include "label.h"

#include "settings.h"
#include "spawn.h"

#include "DocManagerEx.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "ReportErr.h"

/////////////////////////////////////////////////////////////////////////////
// CJweApp

BEGIN_MESSAGE_MAP(CJweApp, CWinApp)
	//{{AFX_MSG_MAP(CJweApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
		
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	
	// Standard print setup command
	//ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	//	tom morris feb 2005
	ON_THREAD_MESSAGE(IDM_PROGRESS_CONTROL_CREATE, OnProgressBarBegin )
	ON_THREAD_MESSAGE(IDM_PROGRESS_CONTROL_SETRANGE, OnProgressBarSetRange )
	ON_THREAD_MESSAGE(IDM_PROGRESS_CONTROL_SETSTEP, OnProgressBarSetStep )
	ON_THREAD_MESSAGE(IDM_PROGRESS_CONTROL_STEP, OnProgressBarStep )
	ON_THREAD_MESSAGE(IDM_PROGRESS_CONTROL_DESTROY, OnProgressBarEnd )
	//	 end tom morris 2005
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJweApp construction

CJweApp::CJweApp() : m_pAppData(NULL), m_pSettings(NULL), m_WndReg( NULL ), m_pMsgLog(NULL)
{
	// TODO: add construction code here, Place all significant initialization in InitInstance
EnableHtmlHelp( ); //Add by Ken Deel 1-4-06
m_pDocManager = NULL; // inintialize to NULL -- if used at all -- which I recommend NOT.
	//	end tom morris march 2005

	m_bMaterialListInitialised = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CJweApp object

CJweApp theApp;

// This identifier was generated to be statistically unique for your app.
// You may change it if you prefer to choose a specific identifier.

// {37F45629-C0E1-11D2-8B41-00104B70D76D}
static const CLSID clsid =
{ 0x37f45629, 0xc0e1, 0x11d2, { 0x8b, 0x41, 0x0, 0x10, 0x4b, 0x70, 0xd7, 0x6d } };

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_AboutText;
	CString m_BuildInfoText;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CLabel	m_link1;
	CLabel	m_link2;
	CLabel	m_link3;
	CLabel	m_link4;
public:
	afx_msg void OnBnClickedAboutTutorials();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_AboutText = _T("");
	m_BuildInfoText = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_EDIT1, m_AboutText);
	DDX_Text(pDX, IDC_TEXT_BUILDINFO, m_BuildInfoText);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ABOUT_TUTORIALS, OnBnClickedAboutTutorials)
END_MESSAGE_MAP()

// App command to run the dialog
void CJweApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


void CAboutDlg::OnBnClickedAboutTutorials()
{
//	Spawn_App( "..\\tutorials\\tutorials.htm", NULL, Spawn_Async );	//by trilobite
	Spawn_App("https://www.jet3d.com/tutorials/tutorial_jedit_1.html", NULL, Spawn_Async);
	EndDialog( IDOK );
	
}

BOOL CAboutDlg::OnInitDialog() 
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

	// Added JH 23.2000
	m_link1.SubclassDlgItem ( IDC_TEXT_JETLINK, this);
	m_link1.SetLink(TRUE)
		.SetTextColor(RGB(0,0,255))
		.SetFontUnderline(TRUE)
		.SetLinkCursor(AfxGetApp()->LoadCursor(IDC_HAND_JET))
		.SetBkColor(GetSysColor(COLOR_3DFACE));

	//	commented out by tom morris April 2023. Jet Forum link
/*	m_link2.SubclassDlgItem(IDC_TEXT_JETLINK1, this);
	m_link2.SetLink(TRUE)
		.SetTextColor(RGB(0,0,255))
		.SetFontUnderline(TRUE)
		.SetLinkCursor(AfxGetApp()->LoadCursor(IDC_HAND_JET))
		.SetBkColor(GetSysColor(COLOR_3DFACE));
	m_link2.SubclassDlgItem(IDC_TEXT_JETLINK1, this);
*/

	m_link3.SubclassDlgItem ( IDC_TEXT_JETLINK2, this);
	m_link3.SetLink(TRUE)
		.SetTextColor(RGB(0,0,255))
		.SetFontUnderline(TRUE)
		.SetLinkCursor(AfxGetApp()->LoadCursor(IDC_HAND_JET))
		.SetBkColor(GetSysColor(COLOR_3DFACE));

//	commented out by tom morris feb 2005
//	m_link4.SubclassDlgItem ( IDC_TEXT_JETLINK3, this);
//	m_link4.SetLink(TRUE)
//		.SetTextColor(RGB(0,0,255))
//		.SetFontUnderline(TRUE)
//		.SetLinkCursor(AfxGetApp()->LoadCursor(IDC_HAND_JET))
//		.SetBkColor(GetSysColor(COLOR_3DFACE));
//	end tom morris feb 2005

	CMainFrame			*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;	
	GetDlgItem(IDC_STATIC_PROP)->SetFont( &pMainFrm->cBigFont, true);

	GetDlgItem(IDC_EDIT1)->SetFont( &pMainFrm->cSmallFont, true);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CJweApp initialization


BOOL CJweApp::RegisterObject( HINSTANCE	DllHandle )
{
	jeBoolean (*RegisterDef)(float MajorVersion, float MinorVersion);

	ASSERT( DllHandle );

	RegisterDef = (jeBoolean (*)(float MajorVersion, float MinorVersion))GetProcAddress( DllHandle, "Object_RegisterDef" );
	return( (*RegisterDef)( JET_MAJOR_VERSION, JET_MINOR_VERSION) );
}

BOOL CJweApp::InitObjects( char * DllPath)
{
	// locals
	jeVFile* DllDir{};
	jeVFile_Finder* Finder{};
	jeVFile_Properties	Properties{};
	HINSTANCE			DllHandle{};
	char* FullName{};

	// open dll directory
	DllDir = jeVFile_OpenNewSystem(
		NULL,
		JE_VFILE_TYPE_DOS,
		DllPath,
		NULL,
		JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY );
	if ( DllDir != NULL )
	{

		// create our directory finder
		#ifndef NDEBUG
		Finder = jeVFile_CreateFinder( DllDir, "*.ddl" );
		#else
		Finder = jeVFile_CreateFinder( DllDir, "*.dll" );
		#endif
		
		if( Finder != nullptr )
		{

			// start processing files
			while ( jeVFile_FinderGetNextFile( Finder ) == JE_TRUE )
			{

				// get properties of current file
				if( jeVFile_FinderGetProperties( Finder, &Properties ) == JE_FALSE )
				{
					jeErrorLog_AddString( JE_ERR_FILEIO_READ, "InitObjects: Unable to get dll file properties.", NULL );
					goto ERROR_INITOBJECTS;
				}


				// save dll full name
				FullName = (char*)jeRam_Allocate( strlen( DllPath ) + strlen( Properties.Name ) + 2 );
				if ( FullName == nullptr )
				{
					jeErrorLog_AddString( JE_ERR_MEMORY_RESOURCE, "InitObjects: Unable to allocate dll full name.", NULL );
					goto ERROR_INITOBJECTS;
				}
				strcpy( FullName, DllPath );
				strcat( FullName, "\\" );
				strcat( FullName, Properties.Name );

				// load up the dll
				DllHandle = LoadLibrary( FullName );
				if ( DllHandle == nullptr )
				{
					jeErrorLog_AddString( JE_ERR_FILEIO_READ, "InitObjects: Unable to load object dll.", Properties.Name );
					jeRam_Free( FullName );
					continue;
				}

				// setup the object functions
				if ( RegisterObject( DllHandle ) == JE_FALSE )
				{
					jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "InitObjects: failed to find get functions for object dll.", Properties.Name  );
					jeRam_Free( FullName );
					continue;
				}
				jeRam_Free( FullName );

			}

			// destroy finder
			jeVFile_DestroyFinder( Finder );
		}

		// close file system
		jeVFile_Close( DllDir );
	}
	return( TRUE );

ERROR_INITOBJECTS:
	jeVFile_DestroyFinder( Finder );
	jeVFile_Close( DllDir );
	return( FALSE );


}

// Krouer: Redefine the Registry key to the short application name
void CJweApp::SetRegistryKey(LPCTSTR lpszRegistryKey)
{
	ASSERT(m_pszRegistryKey == NULL);
	ASSERT(lpszRegistryKey != NULL);
	ASSERT(m_pszAppName != NULL);

	BOOL bEnable = AfxEnableMemoryTracking(FALSE);
	free((void*)m_pszRegistryKey);
	m_pszRegistryKey = _tcsdup(lpszRegistryKey);
	free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(m_pszAppName);
	//	by trilobite	Jan. 2011
    //TCHAR* end = _tcsstr(m_pszProfileName, _T(" "));
	//	*end = 0;
    const char* end = _tcsstr(m_pszProfileName, _T(" "));
	end = 0;
	//
	AfxEnableMemoryTracking(bEnable);
}

BOOL CJweApp::InitInstance()
{
	CString	cstr ;
	//jeXForm3d_SetMaximalAssertionMode( JE_FALSE ) ;
	CString	ObjectDllPath;
	char Path[MAX_PATH];

	//jeMemAllocInfo_Create("Preferences.jMAI");	// Added by Icestorm: Use this for memory debugging

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	
	InitCommonControls();
	CWinApp::InitInstance();
	
	if( GetAppPath( Path, MAX_PATH ) )
	{
		CString	ObjectDllPath;
		//char Path[MAX_PATH];

		//GetAppPath( Path, MAX_PATH );
		ObjectDllPath = Path;
		m_strDriverPath = Path;
		ObjectDllPath += "\\Objects";
		InitObjects( ObjectDllPath.GetBuffer(0));
	}
	
	cstr.LoadString( IDS_REGISTRYKEY ) ;
	SetRegistryKey( cstr );
	
	m_pAppData = AppData_Create( ) ;
	if( m_pAppData == NULL )
		return FALSE ;

	m_pSettings = Settings_Create( ) ;
	if( m_pSettings == NULL )
		return FALSE ;

//---------------------------------------------------
// Added DJT
//---------------------------------------------------
	// Create and initialize mouse settings
	m_pMouseSettings = MouseSettings_Create( ) ;
	if( m_pMouseSettings == nullptr )
		return FALSE ;

	// Get values from registry
	if (!MouseSettings_Restore())
	{
		// Set default values
		MouseSettings_Save();
	}
//---------------------------------------------------
// End DJT
//---------------------------------------------------

	m_WndReg = WndReg_Create( ) ;
	if( m_WndReg == nullptr )
		return FALSE ;

	m_pMsgLog = MsgLog_Create();
	if( m_pMsgLog == nullptr )
		return FALSE ;

	//	begin tom morris 2005
	//	moved code to new InitMaterialList()
//	MaterialList = MaterialList_Create();
//	if( MaterialList == NULL )
//		return FALSE ;
//
//	//This is Temporary.  Will eventuly init material list using user defined prefs
//	{
//		Material_Struct *FirstMaterial;
//		MaterialIterator MI;
//
//		if( !MaterialList_LoadFromDir( MaterialList,"GlobalMaterials" ))
//		{
//			//cyrius
//			MessageBox(NULL, "'GlobalMaterials' directory NOT found. Program cannot continue\nPlease create a GlobalMaterials directory", "INTERNAL ERROR", MB_OK);
//			return( FALSE );
//		}
//		FirstMaterial = MaterialList_GetFirstMaterial(	MaterialList, &MI  );
//		MaterialList_SetCurMaterial( MaterialList, FirstMaterial );
//	}

	//InitMaterialList();
	//	end tom morris feb 2005

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MFC_VER < 0x0700
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

	Util_Init( (unsigned long) AfxGetResourceHandle() ) ;

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_GWFTYPE,
		RUNTIME_CLASS(CJweDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CJweView));

	// krouer: reenable our DocManager
	m_pDocManager = new CDocManagerEx;

	AddDocTemplate(pDocTemplate);	//	reinstituted by tom morris march 2005 //Commented out JH 19.3.2000

//	tom morris march 2005 -- commented out. Not necessary / redundant
//	// Added JH 19.3.2000
//	ASSERT(m_pDocManager == NULL);
//	m_pDocManager = new CDocManagerEx;
//	m_pDocManager->AddDocTemplate(pDocTemplate); // or just AddDocTemplate(pDocTemplate);
//	// EOFJH
//	end tom morris march 2005

	// Connect the COleTemplateServer to the document template.
	//  The COleTemplateServer creates new documents on behalf
	//  of requesting OLE containers by using information
	//  specified in the document template.
	m_server.ConnectTemplate(clsid, pDocTemplate, FALSE);

	// Register all OLE server factories as running.  This enables the
	//  OLE libraries to create objects from other applications.
	COleTemplateServer::RegisterAll();
		// Note: MDI applications register all server objects without regard
		//  to the /Embedding or /Automation on the command line.

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();

	//	tom morris march 2005-- revised to force jwe doc registration
	//	so *.j3d file opens on double-click
	//	RegisterShellFileTypes(TRUE);
	RegisterShellFileTypes(TRUE);
	//	end tom morris march 2005
	((CDocManagerEx*)m_pDocManager)->RegisterOtherFileTypes();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	//	tom morris march 2005 -- revised to force CWinApp API and not ATL versions
	//	ParseCommandLine(cmdInfo);
	CWinApp::ParseCommandLine(cmdInfo);
	//	end tom morris march 2005

	// Check to see if launched as OLE server
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		// Application was run with /Embedding or /Automation.  Don't show the
		//  main window in this case.
		return TRUE;
	}

	// When a server application is launched stand-alone, it is a good idea
	//  to update the system registry in case it has been damaged.
	m_server.UpdateRegistry(OAT_DISPATCH_OBJECT);
	COleObjectFactory::UpdateRegistryAll();

// Commented out by Incarnadine.  This can't be called twice!  It's already called in MainFrame on create.
//	pMainFrame->InitCommandPanel() ;	// Command panels MUST be present when doc inits

	// Dispatch commands specified on the command line AND init Doc
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	pMainFrame->InitObjectList();
	{
		CString CurDirectory = Path;
		CurDirectory += "\\Levels";
		if( !SetCurrentDirectory( CurDirectory.GetBuffer(0) ) ) 
			jeErrorLog_Add( JE_ERR_FILEIO_OPEN, "Unable to set Level Directory" );
	}

	Disclaimer DisclaimerDlg;
		
// Added JH 24.3.2000
	DisclaimerDlg.m_Show = Settings_GetJEdit_ShowDisclaimer();
	if ((Settings_GetJEdit_Version()<atoi(BUILD_NUMBER))||
		(!DisclaimerDlg.m_Show ))
		DisclaimerDlg.DoModal();

	Settings_SetJEdit_Version(atoi(BUILD_NUMBER));	
	Settings_SetJEdit_ShowDisclaimer(DisclaimerDlg.m_Show);
// EOF JH

	//	tom morris may 2005
	SquareAwayAutosave();
	//	end tom morris may 2005

	return TRUE;
}// InitInstance


int CJweApp::ExitInstance() 
{
	if( MaterialList != nullptr )
		MaterialList_Destroy( &MaterialList ) ;

	if( m_pAppData != nullptr )
		AppData_Destroy( &m_pAppData ) ;

	if( m_pSettings != nullptr )
		Settings_Destroy( &m_pSettings ) ;
	
	//jeMemAllocInfo_Destroy();	// Added by Icestorm : Use this for memory debugging
		
	return CWinApp::ExitInstance();
}// ExitInstance


//	begin tom morris feb 2005

bool	 CJweApp::RefreshMaterialList()
{
	CString	strPath;
	CMainFrame *pMainFrame = nullptr;
	pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrame)
	{
		CJweDoc *pDoc = NULL;
		pDoc = pMainFrame->GetCurrentDocument();

        if( MaterialList != nullptr )
			MaterialList_Destroy( &MaterialList ) ;

		MaterialList = NULL;

		if (pDoc)
		{
			jeEngine* pEngine = pDoc->GetJetEngine();
			jeResourceMgr* pResMgr = pDoc->GetResourceMgr();
			InitMaterialList(pEngine, pResMgr);

			if (pDoc)
			{
				pMainFrame->RebuildLists(pDoc);
			}
		}

/*
        if (pDoc)
		{
			strPath = pDoc->GetPathName();

            int	iResult;
		    iResult = AfxMessageBox("Level will close and reopen.\n\nSave Level Now?", MB_YESNOCANCEL | MB_ICONQUESTION );

			switch (iResult)
			{
			case IDYES:
				pMainFrame->CloseCurDoc();
				break;
			case IDNO:
				pDoc->OnCloseDocument();
				break;
			case IDCANCEL:
				return false;
			default:
				pMainFrame->CloseCurDoc();
			}

            if (strPath != "")
		    {
			    OpenDocumentFile(strPath);
		    }
		    else
		    {
			    OnFileNew();
		    }
        }
*/
	}
	return true;
}


bool	CJweApp::InitMaterialList(jeEngine* pEngine, jeResourceMgr* pResMgr)
{
	MaterialList = MaterialList_Create();
	if( MaterialList == nullptr )
		return false ;

	//This is Temporary.  Will eventuly init material list using user defined prefs
	{
		Material_Struct* FirstMaterial{};
		MaterialIterator MI{};

		if( !MaterialList_LoadFromDir( MaterialList, pEngine, pResMgr, "GlobalMaterials" ))
		{
			//cyrius
			MessageBox(NULL, "'GlobalMaterials' directory NOT found. Program cannot continue\nPlease create a GlobalMaterials directory", "INTERNAL ERROR", MB_OK);
			return( false );
		}
		FirstMaterial = MaterialList_GetFirstMaterial(	MaterialList, &MI  );
		MaterialList_SetCurMaterial( MaterialList, FirstMaterial );
	}
	m_bMaterialListInitialised = TRUE;
	return true;
}


//	end tom morris feb 2005



/////////////////////////////////////////////////////////////////////////////
//	SquareAwayAutosave - tom morris may 2005
//	
/////////////////////////////////////////////////////////////////////////////
bool	CJweApp::SquareAwayAutosave()
{
	m_dlgAutoRecover.Create(IDD_DLG_AUTORECOVER, ((CMainFrame*)AfxGetMainWnd()));
	m_dlgAutoRecover.SquareAwayAutosave();
	return true;
}




BOOL CJweApp::GetDriverAndMode(jeEngine* pEngine, jeDriver **ppDriver, jeDriver_Mode **ppMode)
{

	// locals
	jeBoolean	DisplayVideoSettingsDialog = JE_FALSE;

	// determine if video settings dialog should be displayed
	{
		int	count;
		for( count = 0; count < __argc; count++ )
		{
			if ( stricmp( __argv[count], "-videosettings" ) == 0 )
			{
				DisplayVideoSettingsDialog = JE_TRUE;
			}
		}
	}

	// pick driver
	return DrvList_PickDriver(m_hInstance, *GetMainWnd(), pEngine, ppDriver, ppMode, DisplayVideoSettingsDialog, DRVLIST_WINDOW | DRVLIST_SOFTWARE | DRVLIST_HARDWARE );

}// GetDriverAndMode

const char* CJweApp::GetDriverPath(void)
{
	return(m_strDriverPath);
	//return "." ;
}

MaterialList_Struct* CJweApp::GetMaterialList()
{
	return( MaterialList );
}


/////////////////////////////////////////////////////////////////////////////
// CJweApp message handlers

#if 0
void CJweApp::OnToolsDefineentities() 
{
	CEntityDef	EntityDefDialog ;

	if( EntityDefDialog.DoModal() == IDOK )
	{

	}
}// OnToolsDefineentities

void CJweApp::OnUpdateToolsDefineentities(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}// OnUpdateToolsDefineentities
#endif

void CJweApp::StartLogRecord()
{
	MsgLog_StartRecord( m_pMsgLog, "Msg.log");
}


void CJweApp::EndLogRecord()
{
	if( MsgLog_IsRecording( m_pMsgLog ) )
		MsgLog_EndRecord( m_pMsgLog );
}

void CJweApp::StartLogPlay()
{
	MsgLog_StartPlay( m_pMsgLog, "Msg.log");
}


void CJweApp::EndLogPlay()
{
	if( MsgLog_IsPlaying( m_pMsgLog ) )
		MsgLog_EndPlay( m_pMsgLog );
}

jeBoolean CJweApp::GetAppPath( char * PathBuf, int BuffSize )
{
	int i;

	// get the exe's path and name
	if(GetModuleFileName( m_hInstance,PathBuf,BuffSize-1)==0)
	{
		return( JE_FALSE );
	}

	// strip off exe name to leave path
	for (i=strlen(PathBuf)-1; i>0; i--)
	{
		if (PathBuf[i]=='\\')
		{
			PathBuf[i]=0;
			break;
		}
	}
	if (i==0)
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
	
}

BOOL CJweApp::PreTranslateMessage(MSG* pMsg) 
{
	if( MsgLog_IsRecording( m_pMsgLog ) )
	{
		int32 WndSigniture;

		WndSigniture = WndReg_GetSigniture( pMsg->hwnd );
		if( WndSigniture )
		{
			MsgLog_Record(m_pMsgLog, pMsg, WndSigniture );
		}
	}	
	if( MsgLog_IsPlaying( m_pMsgLog ) )
	{
		MsgLog_Play( m_pMsgLog );
		if( MsgLog_IsLogMsg( pMsg ) )
			return( TRUE );
	}
	
	if( pMsg->message == WM_KEYDOWN )
		pMsg->message = WM_KEYDOWN;
	return CWinApp::PreTranslateMessage(pMsg);
}

BOOL CJweApp::OnIdle(LONG lCount) 
{

	if( MsgLog_IsPlaying( m_pMsgLog ) )
	{
		MsgLog_Play( m_pMsgLog );
		return( 0 );
	}
	ReportErrors(JE_TRUE);
	return CWinApp::OnIdle(lCount);
}


//	begin tom morris feb 2005
void	CJweApp::OnProgressBarBegin(WPARAM wParam, LPARAM lParam)
{
	LPSTR	strMessage = (LPSTR)wParam;
	
	((CMainFrame*)m_pMainWnd)->ProgressBarBegin(strMessage);
}


void	CJweApp::OnProgressBarSetRange(WPARAM wParam, LPARAM lParam)
{
	int iRange = (int)wParam;
	LPSTR	strMessage = (LPSTR)lParam;
	
	((CMainFrame*)m_pMainWnd)->ProgressBarSetRange(iRange, strMessage);
}


void	CJweApp::OnProgressBarSetStep(WPARAM wParam, LPARAM lParam)
{
	int iStep = (int)wParam;
	LPSTR	strMessage = (LPSTR)lParam;
	
	((CMainFrame*)m_pMainWnd)->ProgressBarSetStep(iStep, strMessage);
}


void	CJweApp::OnProgressBarStep(WPARAM wParam, LPARAM lParam)
{
	LPSTR	strMessage = (LPSTR)wParam;
	((CMainFrame*)m_pMainWnd)->ProgressBarStep(strMessage);
}


void	CJweApp::OnProgressBarEnd(WPARAM wParam, LPARAM lParam)
{
	((CMainFrame*)m_pMainWnd)->ProgressBarEnd();
}

//	end tom morris feb 2005




