/****************************************************************************************/
/*  MAINFRM.CPP                                                                         */
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
#include <CommCtrl.h>

#include "jwe.h"
#include "Util.h"

#include "MainFrm.h"
#include "Settings.h"
#include ".\mainfrm.h"

//	tom morris feb 2005
#include "texturesdlg.h"
//	end tom morris feb 2005
#include "View.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const TCHAR * pszDesktopKey = _T("DESKTOP");
static const TCHAR * pszPlacementKey = _T("PLACEMENT");
#define WM_REBUILDPROPERIES WM_USER+0x100

// To add a new command panel dialog:
// Add a CDialog derived class, see caveats in InitCommandPanel
// Make member variable for dialog, load in InitCommandPanel
// Add Dialog tab name resource, put in tabNames
// Update MAINFRM_COMMANDPANEL_TAB
// Handle show/hide change in SetCommandPanelTab
// Add to DialogFromIndex

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CJ3DMainFrame)

BEGIN_MESSAGE_MAP(CMainFrame, CJ3DMainFrame)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//	tom morris may 2005
	ON_WM_TIMER()
	//	end tom morris may 2005

//	ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
//	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateViewToolbar)

	ON_COMMAND(IDM_VIEW_ALLMATERIALS, OnViewAllmaterials)

	ON_COMMAND(ID_MODE_ABORT, OnModeAbort)
	ON_UPDATE_COMMAND_UI(ID_MODE_ABORT, OnUpdateModeAbort)

	ON_WM_SIZING()
	ON_WM_SIZE()

	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_MOVE()
	ON_WM_INITMENU()
	ON_MESSAGE( WM_REBUILDPROPERIES, OnRebuildProperties )

	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpFinder)

	//	added HELP file access back in - Ken Deel Jan-1-06
	ON_COMMAND(ID_IDR_DELETEITEM, OnIdrDeleteitem)
	ON_UPDATE_COMMAND_UI(ID_IDR_DELETEITEM, OnUpdateIdrDeleteitem)
	ON_COMMAND(ID_IDR_ADDGROUP, OnIdrAddgroup)
	ON_UPDATE_COMMAND_UI(ID_IDR_ADDGROUP, OnUpdateIdrAddgroup)
	// end tom morrris feb 2005

	ON_COMMAND_EX(ID_VIEW_PROPERTIES, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPERTIES, OnUpdateControlBarMenu)
	ON_COMMAND_EX(IDM_VIEW_LISTS, OnBarCheck)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_LISTS, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_EDIT, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_EDIT, OnUpdateControlBarMenu)
	ON_COMMAND_EX(IDM_VIEW_OBJECTSBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_OBJECTSBAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_POSSIZE, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_POSSIZE, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_FULLSCREEN, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCREEN, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_MODE, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODE, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateControlBarMenu)

    ON_UPDATE_COMMAND_UI(IDM_POSSIZE_X, OnUpdateObjectPosSizeItem)
    ON_UPDATE_COMMAND_UI(IDM_POSSIZE_Y, OnUpdateObjectPosSizeItem)
    ON_UPDATE_COMMAND_UI(IDM_POSSIZE_Z, OnUpdateObjectPosSizeItem)
    ON_UPDATE_COMMAND_UI(IDM_POSSIZE_XSIZE, OnUpdateObjectPosSizeItem)
    ON_UPDATE_COMMAND_UI(IDM_POSSIZE_YSIZE, OnUpdateObjectPosSizeItem)
    ON_UPDATE_COMMAND_UI(IDM_POSSIZE_ZSIZE, OnUpdateObjectPosSizeItem)

END_MESSAGE_MAP()

const static UINT indicators[] =
{
	ID_SEPARATOR,  // Text Area. Also holds progress control
    ID_SEPARATOR,
    ID_INDICATOR_PERF,
	ID_INDICATOR_POS,
	ID_INDICATOR_SIZE,
};

typedef enum {
	sbPosSeparator=1,
	sbPos  ,
	sbPosX ,
	sbPosY ,
	sbPosZ,
	sbSizeSeparator,
	sbSize,
	sbSizeX ,
	sbSizeY ,
	sbSizeZ,
	sbSeparator
} eStatusBarId;


#define MAX_TAB_NAME_LENGTH (32)
// This must match the ID's in the header file
const static UINT tabNames[] =
{
	IDS_TAB_TEXTURES,
	IDS_TAB_LISTS,
	IDS_TAB_MODELS,
	IDS_TAB_GROUP
} ;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() :  m_pCurrentDoc(NULL),m_iAnim_State(0)
{
	
}

CMainFrame::~CMainFrame()
{
}


///////////////////////////////////////////////////////////////////////////////
//	AutosaveTimerChildProc	-	tom morris may 2005
//	
///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK AutosaveTimerChildProc( HWND hwnd,  LPARAM lParam)
{
		//	if the window being tested is a CtDirectorView, then send that
		//	window a message that "it is time to autosave"
	if (DYNAMIC_DOWNCAST(CJweView,CWnd::FromHandle(hwnd)) != NULL)
	{
		::PostMessage(hwnd,WM_JWEVIEW_AUTOSAVETIMER,0,0);
	}
	return(TRUE);
}

///////////////////////////////////////////////////////////////////////////////
//	OnTimer	-	tom morris may 2005
//	
///////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnTimer(UINT nIDEvent) 
{
#ifdef	NDEBUG
		//	if it's the autosave timer, enumerate through the child windows
		//	use the AutosaveTimerChildProc callback to test each window's identity
	if (nIDEvent == ID_AUTOSAVE_TIMER)
		::EnumChildWindows(m_hWnd, AutosaveTimerChildProc, NULL);	
#endif	
	CMDIFrameWnd::OnTimer(nIDEvent);
}






void CMainFrame::DockControlBarLeftOf(CToolBar* Bar,CToolBar* LeftOf)
{
	CRect rect;
	DWORD dw;
	UINT n{};

	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout();
	LeftOf->GetWindowRect(&rect);
	rect.OffsetRect(1,0);
	dw=LeftOf->GetBarStyle();
	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;

	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line.  By calculating a rectangle, we in effect
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar,n,&rect);
}// DockControlBarLeftOf (From MSDN)

//extern Tcl_Interp * theInterp;




int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int			i{};
	TC_ITEM		TabItem{};
	char		szBuffer[MAX_TAB_NAME_LENGTH] ;
	HINSTANCE	hResources{};
//	RECT		rect;
	int			iToolBars_XSize=19;
	DWORD		dwToolBarStyle = 0;
	
	if (Settings_GetGlobal_ToolbarText()) 
			 iToolBars_XSize=19;
		else iToolBars_XSize=0;

	if (Settings_GetGlobal_ToolbarFlat()) 
			 dwToolBarStyle=TBSTYLE_FLAT;
		else dwToolBarStyle=0;

	hResources = AfxGetResourceHandle() ;

	if( CMDIFrameWnd::OnCreate(lpCreateStruct) == -1 )
		return -1;

	// Added JH 4.3.2000
	cSmallFont.CreateFont
			(12, 0, 0, 0, 0, FALSE, FALSE, 0, ANSI_CHARSET, 
			  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
			  DEFAULT_PITCH|FF_SWISS,
			  "Arial");
	cMedFont.CreateFont
			(15, 0, 0, 0, 0, FALSE, FALSE, 0, ANSI_CHARSET, 
			  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
			  DEFAULT_PITCH|FF_SWISS,
			  "Arial");
	cBigFont.CreateFont
			(25, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, 
			  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
			  DEFAULT_PITCH|FF_SWISS,
			  "Arial");

	// EOF JH

	m_SoundSystem = jeSound_CreateSoundSystem( this->GetSafeHwnd());
	if( !m_wndToolBar.CreateEx	// GENERAL TOOLBAR
		(
			this, 
			dwToolBarStyle, 
			WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_GRIPPER|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC,
			CRect(0,0,0,0), 
            ID_VIEW_TOOLBAR
		) || !m_wndToolBar.LoadToolBar(MAKEINTRESOURCE(IDR_MAINFRAME) ,iToolBars_XSize,18) )
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if( !m_wndModeBar.CreateEx	// MODE BAR
		(
			this,
			dwToolBarStyle, 
			WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_GRIPPER|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC,
			CRect(0,0,0,0), 
			ID_VIEW_MODE
		) || !m_wndModeBar.LoadToolBar( MAKEINTRESOURCE(IDR_MODE) ,iToolBars_XSize,18)  )
	{
		TRACE0("Failed creating ModeBar\n");
		return -1 ;
	}

	// Added JH 5.3.2000
	if( !m_wndPosSizeBar.CreateEx	// PosSize BAR
		(
			this,
			dwToolBarStyle, 
			WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_GRIPPER|CBRS_TOOLTIPS|CBRS_FLYBY,
			CRect(0,0,0,0), 
			ID_VIEW_POSSIZE
		) || !m_wndPosSizeBar.LoadToolBar( MAKEINTRESOURCE(IDR_POSSIZE),iToolBars_XSize,18 )  )
	{
		TRACE0("Failed creating PosSize Bar\n");
		return -1 ;
	}

	
	if (iToolBars_XSize==0)
		{
		m_wndPosSizeBar.ChangeToolBar (&m_sXPos,IDM_POSSIZE_X,IDM_POSSIZE_X, TBBS_BUTTON,&cSmallFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sYPos,IDM_POSSIZE_Y,IDM_POSSIZE_Y, TBBS_BUTTON,&cSmallFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sZPos,IDM_POSSIZE_Z,IDM_POSSIZE_Z, TBBS_BUTTON,&cSmallFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sXSize,IDM_POSSIZE_XSIZE,IDM_POSSIZE_XSIZE, TBBS_BUTTON,&cSmallFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sYSize,IDM_POSSIZE_YSIZE,IDM_POSSIZE_YSIZE, TBBS_BUTTON,&cSmallFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sZSize,IDM_POSSIZE_ZSIZE,IDM_POSSIZE_ZSIZE, TBBS_BUTTON,&cSmallFont );
		}
	else{
		m_wndPosSizeBar.ChangeToolBar (&m_sXPos,IDM_POSSIZE_X,IDM_POSSIZE_X, TBBS_BUTTON,&cMedFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sYPos,IDM_POSSIZE_Y,IDM_POSSIZE_Y, TBBS_BUTTON,&cMedFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sZPos,IDM_POSSIZE_Z,IDM_POSSIZE_Z, TBBS_BUTTON,&cMedFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sXSize,IDM_POSSIZE_XSIZE,IDM_POSSIZE_XSIZE, TBBS_BUTTON,&cMedFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sYSize,IDM_POSSIZE_YSIZE,IDM_POSSIZE_YSIZE, TBBS_BUTTON,&cMedFont );
		m_wndPosSizeBar.ChangeToolBar (&m_sZSize,IDM_POSSIZE_ZSIZE,IDM_POSSIZE_ZSIZE, TBBS_BUTTON,&cMedFont );
		}

	if( !m_wndObjectToolBar.CreateEx	// ObjectTool BAR
		(
			this,
			dwToolBarStyle, 
			WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_GRIPPER|CBRS_TOOLTIPS|CBRS_FLYBY,
			CRect(0,0,0,0), 
			IDM_VIEW_OBJECTSBAR
		) || !m_wndObjectToolBar.LoadToolBar( MAKEINTRESOURCE(IDR_OBJECTS),iToolBars_XSize,18 ) )
	{
		TRACE0("Failed creating ObjectToolBar\n");
		return -1 ;
	}
	m_wndObjectToolBar.ChangeToolBar(&m_wndObjectType, IDM_TOOLS_COMBOPLACEHOLDER, IDC_OBJECTTYPES, TBBS_SEPARATOR, &cMedFont);

	if( !m_wndEditBar.CreateEx	// Edit BAR
		(
			this,
			dwToolBarStyle, 
			WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_GRIPPER|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC,
			CRect(0,0,0,0), 
			ID_VIEW_EDIT
		) || !m_wndEditBar.LoadToolBar( MAKEINTRESOURCE(IDR_EDIT),iToolBars_XSize,18 ) )
	{
		TRACE0("Failed creating ModeBar\n");
		return -1 ;
	}
	 

	if( !m_wndFullscreenBar.CreateEx	// Fullscreen BAR
		(
			this,
			dwToolBarStyle, 
			WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_GRIPPER|CBRS_TOOLTIPS|CBRS_FLYBY,
			CRect(0,0,0,0), 
			ID_VIEW_FULLSCREEN
		) || !m_wndFullscreenBar.LoadToolBar( MAKEINTRESOURCE(IDR_FULLSCREEN),iToolBars_XSize,18 ) )
	{
		TRACE0("Failed creating Fullscreen Bar\n");
		return -1 ;
	}

    if( !m_PropBar.Create("Properties", this, CSize(200,200), TRUE, ID_VIEW_PROPERTIES)) {
		TRACE0("Failed creating m_PropBar\n") ;
		return -1;
    }
    m_PropBar.SetBarStyle(m_PropBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

    if( !m_ListBar.Create("Browsers", this, CSize(160,160), TRUE, IDM_VIEW_LISTS)) {
		TRACE0("Failed creating m_ListBar\n") ;
		return -1 ;
    }
    m_ListBar.SetBarStyle(m_ListBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	if( !m_PropertiesDlg.Create( IDD_PROPERTIEDLG, &m_PropBar ) )
	{
		TRACE0("Failed creating m_PropertiesDlg\n") ;
		return -1 ;
	}
    m_PropBar.m_wndChild=&m_PropertiesDlg;

    if( !m_CommandPanel.Create( IDD_COMMANDPANEL, &m_ListBar, this))
    {
		TRACE0("Failed creating m_CommandPanel\n") ;
		return -1 ;
    }
    m_ListBar.m_wndChild=&m_CommandPanel;

	// Initialize Tab Control
	TabItem.mask = TCIF_TEXT ;
	for( i=0; i< sizeof(tabNames)/sizeof(*tabNames); i++ )
	{
		::LoadString( hResources, tabNames[i], szBuffer, sizeof szBuffer ) ;
		TabItem.pszText = szBuffer ;
		TabItem.cchTextMax = strlen( szBuffer ) + sizeof szBuffer[0] ;
		m_CommandPanel.SendDlgItemMessage( CPNL_TC_TABS, TCM_INSERTITEM , i, (LPARAM)&TabItem );
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
    m_wndStatusBar.SetPaneInfo(3, ID_INDICATOR_POS, SBPS_NORMAL, 100);
    m_wndStatusBar.SetPaneInfo(4, ID_INDICATOR_SIZE, SBPS_NORMAL, 100);

	// Assign Titles to bars
	::LoadString( hResources, IDS_TOOLBAR_GENERAL, szBuffer, sizeof szBuffer ) ;
	m_wndToolBar.SetWindowText( szBuffer ) ;
	::LoadString( hResources, IDS_TOOLBAR_MODE, szBuffer, sizeof szBuffer ) ;
	m_wndModeBar.SetWindowText( szBuffer ) ;
	::LoadString( hResources, IDS_TOOLBAR_OBJECT, szBuffer, sizeof szBuffer ) ;
	m_wndObjectToolBar.SetWindowText( szBuffer ) ;
	::LoadString( hResources, IDS_TOOLBAR_POSANDSIZE, szBuffer, sizeof szBuffer ) ;
	m_wndPosSizeBar.SetWindowText( szBuffer ) ;
	::LoadString( hResources, IDS_TOOLBAR_EDIT, szBuffer, sizeof szBuffer ) ;
	m_wndEditBar.SetWindowText( szBuffer ) ;
	::LoadString( hResources, IDS_TOOLBAR_FULLSCREEN, szBuffer, sizeof szBuffer ) ;
	m_wndFullscreenBar.SetWindowText( szBuffer ) ;

    InitCommandPanel();

	m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY ) ;
	m_wndModeBar.EnableDocking( CBRS_ALIGN_ANY ) ;
	m_wndEditBar.EnableDocking( CBRS_ALIGN_ANY ) ;
	m_wndFullscreenBar.EnableDocking( CBRS_ALIGN_ANY ) ;
	m_wndObjectToolBar.EnableDocking(CBRS_ALIGN_BOTTOM  |CBRS_ALIGN_TOP) ;
	m_wndPosSizeBar.EnableDocking( CBRS_ALIGN_BOTTOM  |CBRS_ALIGN_TOP ) ;
	
    m_PropBar.EnableDocking(CBRS_ALIGN_ANY);
    m_ListBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking( CBRS_ALIGN_ANY ) ;

    DockControlBar( &m_wndToolBar ) ;
    DockControlBar( &m_PropBar);
    DockControlBar( &m_ListBar);

	DockControlBarLeftOf( &m_wndObjectToolBar, &m_wndToolBar ) ;
	DockControlBarLeftOf( &m_wndPosSizeBar, &m_wndObjectToolBar ) ;
	DockControlBarLeftOf( &m_wndEditBar, &m_wndPosSizeBar ) ;
	DockControlBarLeftOf( &m_wndModeBar, &m_wndEditBar);
	DockControlBarLeftOf( &m_wndFullscreenBar, &m_wndModeBar ) ;
	
	RecalcLayout();

	if( Util_IsKeyDown( VK_SHIFT ) == JE_FALSE )
	{
		LoadBarState( pszDesktopKey ) ;
	    CSizingControlBar::GlobalLoadState(this, pszDesktopKey);
		{	// load the window placement
			UINT nSize = 0;
			BOOL bSuccess = FALSE;
			BYTE* pData = NULL;
			bSuccess = AfxGetApp()->GetProfileBinary(pszDesktopKey, pszPlacementKey, &pData, &nSize);
			if( bSuccess && (nSize == sizeof(WINDOWPLACEMENT)) )
			{
				SetWindowPlacement((WINDOWPLACEMENT*)pData);
			}
			//	tom morris feb 2005
			else
			{
			    DockControlBar(&m_PropBar, AFX_IDW_DOCKBAR_RIGHT, NULL);	
				DockControlBar(&m_ListBar, AFX_IDW_DOCKBAR_LEFT, NULL);
			}
			//	end tom morris 2005
			delete pData;
		}
	}
		//	set time interval for autosave feature -- tom morris may 2005
		//SetTimer(ID_AUTOSAVE_TIMER, Settings_GetAutosaveMinutes()*60*1000, NULL); 

	return 0;
}// OnCreate


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
	
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG



/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::UpdateTimeDelta(  float TimeDelta )
{
	m_TimeLine.UpdateTimeDelta( TimeDelta );
}


BOOL CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	int			idCtrl{};
	NMHDR* pnmh{};

    idCtrl = (int)wParam ;
	pnmh = (LPNMHDR)lParam ; 

	if( CPNL_TC_TABS == idCtrl && TCN_SELCHANGE == pnmh->code )
	{
		SetCommandPanelTab( (MAINFRM_COMMANDPANEL_TAB)TabCtrl_GetCurSel( pnmh->hwndFrom ) ) ;
	}

	return CJ3DMainFrame::OnNotify(wParam, lParam, pResult);
}// OnNotify

bool CMainFrame::InitCommandPanel()
{
	// Dialog templates created for the command panel MUST have the WS_CHILD FLAG
	// MUST NOT HAVE CAPTION, BORDER, VISIBLE and 
	// MUST be sized to fit in the command panel
	// Be sure to override WM_INITDIALOG and call PositionDialogUnderTabs
	
	//	by trilobite jan. 2011 
	CTabCtrl * pTC = nullptr;
	pTC = (CTabCtrl*)m_CommandPanel.GetDlgItem( CPNL_TC_TABS ) ;
	
	ASSERT( pTC != NULL ) ;
	
	m_GroupDialog.Create( IDD_GROUPS, pTC ) ;
    m_ModelsDialog.Create( IDD_MODELS, pTC );	
	m_ListsDialog.Create( IDD_LISTS, pTC ) ;
	m_TimeLine.Create( IDD_TIMELINE, pTC );

//	tom morris feb 2005
//	m_TextureDialog.Create( IDD_TEXTURE, pTC ) ;
	m_TextureDialog.Create( IDD_TEXTURES, pTC ) ;
//	end tom morris 2005

	// Default to Template
	m_TextureDialog.ShowWindow( SW_SHOW ) ;
	
	pTC->SetCurSel( MAINFRM_COMMANDPANEL_TEXTURES ) ;
	
	m_eCurrentTab = MAINFRM_COMMANDPANEL_TEXTURES ;
	return true ;
}// InitCommandPanel

void CMainFrame::SetStatusText( const char * Text )
{
	m_wndStatusBar.SetPaneText(0, Text);
}

void CMainFrame::Set3DViewStats(jeFloat fps, int32 nbFaces)
{
    char tmp[40];
/*
    sprintf(tmp, "Fps = %6.2f", fps);
    m_wndStatusBar.SetPaneText(3, tmp);
*/
    m_wndStatusBar.SetPaneText(3, "Stats:");

    sprintf_s(tmp, "DrawFaces = %5d", nbFaces);
    m_wndStatusBar.SetPaneText(4, tmp);
}

// Added JH 4.3.2000
void CMainFrame::SetStatusSize(jeFloat X,jeFloat Y,jeFloat Z)
{
    char sSize[40];

    sprintf_s (sSize,"%5.0f",X);
    m_sXSize.SetWindowText(sSize);

    sprintf_s (sSize,"%5.0f",Y);
    m_sYSize.SetWindowText(sSize);

    sprintf_s (sSize,"%5.0f",Z);
    m_sZSize.SetWindowText(sSize);
}

void CMainFrame::SetStatusPos (jeFloat X,jeFloat Y,jeFloat Z)
{
    char sSize[40];

    sprintf_s (sSize,"%5.0f",X);
    m_sXPos.SetWindowText(sSize);

    sprintf_s (sSize,"%5.0f",Y);
    m_sYPos.SetWindowText(sSize);

    sprintf_s (sSize,"%5.0f",Z);
    m_sZPos.SetWindowText(sSize);
}
// EOF JH


void CMainFrame::CloseCurDoc( void )
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CJweDoc* pDoc{};

	pDoc =  GetCurrentDocument();
	if( pDoc != NULL && pDoc->HasChanged() )
		pDoc->Save();

	if( pActiveChild != NULL )
		pActiveChild->MDIDestroy();

}

void CMainFrame::InitObjectList( void )
{
	CComboBox* ObjectList{};
	int32 RegObjN{};
	const char* ObjTypeName{};
	uint32	flags{};

	ObjectList = &m_wndObjectType;
	
	RegObjN = jeObject_GetRegisteredN();
	for( int i = 0; i < RegObjN ; i++ )
	{
		flags = jeObject_GetRegisteredFlags( i );

		if( flags & JE_OBJECT_HIDDEN )
			continue;
		ObjTypeName = jeObject_GetRegisteredDefName( i);
		ObjectList->AddString( ObjTypeName );
	}
	if( RegObjN )
		ObjectList->SetCurSel(0);

}

void CMainFrame::EndRotateSub( Object * pObject  )
{
	m_TimeLine.SubSelectEndRotate( Object_GetjeObject( pObject ) );
}

void CMainFrame::EndMoveSub( Object * pObject )
{
	m_TimeLine.SubSelectEndMove( Object_GetjeObject( pObject ) );
}

void  CMainFrame::SubSelectObject( Object * pObject )
{
	m_TimeLine.SubSelectObject( Object_GetjeObject( pObject ) );
}

jeBoolean CMainFrame::GetCurUserObjName( CString * Name )
{
	CComboBox* ObjectList{};
	int CurSel{};

	ObjectList = &m_wndObjectType;

	CurSel = ObjectList->GetCurSel();
	if( CurSel < 0 )
		return( JE_FALSE );
	ObjectList->GetLBText( CurSel, *Name);
	return( JE_TRUE );
}


jeSound_System * CMainFrame::GetSoundSystem()
{
	return( m_SoundSystem );
}
void CMainFrame::SetCommandPanelTab( MAINFRM_COMMANDPANEL_TAB nTab )
{
	CDialog* pOldDialog{};
	CDialog* pNewDialog{};
	ASSERT( nTab < MAINFRM_COMMANDPANEL_LAST ) ;

	if( m_eCurrentTab != nTab )
	{
		// Notify Tab closing
		pOldDialog = DialogFromIndex( m_eCurrentTab ) ;
		pOldDialog->ShowWindow( SW_HIDE ) ;		

		// Notify Tab opening
		pNewDialog = DialogFromIndex( nTab ) ;
		pNewDialog->ShowWindow( SW_SHOW ) ;
		m_eCurrentTab = nTab ;
	}
}// SetCommandPanelTab

void CMainFrame::SetStats( const jeBSP_DebugInfo * pDebugInfo )
{
	pDebugInfo;
}

void  CMainFrame::ResetLists( void )
{
	m_GroupDialog.Reset();
	m_ModelsDialog.Reset();
	m_ListsDialog.Reset();
}

void CMainFrame::ResetProperties( void )
{
	m_PropertiesDlg.Reset();
	m_TimeLine.Reset();
}

CDialog * CMainFrame::DialogFromIndex(MAINFRM_COMMANDPANEL_TAB nTab)
{
	ASSERT( nTab < MAINFRM_COMMANDPANEL_LAST ) ;
	
	switch( nTab )
	{
		case MAINFRM_COMMANDPANEL_TEXTURES :	return &m_TextureDialog ;
		case MAINFRM_COMMANDPANEL_LISTS :		return &m_ListsDialog ;
		case MAINFRM_COMMANDPANEL_MODELS :		return &m_ModelsDialog;
		case MAINFRM_COMMANDPANEL_GROUPS :		return &m_GroupDialog ;
		default :
			ASSERT( 0 ) ;
			break ;
	}
	return &m_GroupDialog ;
}// DialogFromIndex

void CMainFrame::OnClose() 
{
	CSizingControlBar::GlobalSaveState(this, pszDesktopKey);
	SaveBarState( pszDesktopKey ) ;		// Save Toolbar locations

    WINDOWPLACEMENT wndplace;			// Save Window size and position
	GetWindowPlacement(&wndplace);
	if( IsZoomed() ) 
		wndplace.showCmd = SW_SHOWMAXIMIZED; // force the window to maximize next time
	else if( IsIconic() )
		wndplace.showCmd = SW_SHOW; // force the window to show normally next time
	AfxGetApp()->WriteProfileBinary( pszDesktopKey, pszPlacementKey, (BYTE*)&wndplace, sizeof(WINDOWPLACEMENT) );
	
	CJ3DMainFrame::OnClose();
}// OnClose

// NOTE: This can return NULL
CJweDoc * CMainFrame::GetCurrentDocument()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();

	if( pActiveChild == NULL )
		return nullptr ;

	return (CJweDoc*)pActiveChild->GetActiveDocument() ;
}// GetActiveDocument




void CMainFrame::SetProperties( jeProperty_List * pArray )
{
	m_PropertiesDlg.Reset();
	m_TimeLine.Reset();
	if( pArray != nullptr )
	{
		m_PropertiesDlg.BuildFromDescriptor( pArray );
		m_TimeLine.BuildFromDescriptor( pArray );
	}
}

void CMainFrame::UpdateProperties( jeProperty_List * pArray )
{
	if( pArray != nullptr )
	{
		if( pArray->bDirty )
		{
			m_PropertiesDlg.Reset();
			m_TimeLine.Reset();
			m_PropertiesDlg.BuildFromDescriptor( pArray );
			m_TimeLine.BuildFromDescriptor( pArray );
		}
		else
		{
			m_TimeLine.UpdateDataByArray( pArray );
			m_PropertiesDlg.UpdateDataByArray( pArray );
		}
	}
}


void CMainFrame::RebuildLists( CJweDoc *pDoc )
{
	m_ListsDialog.SetCurrentDocument( pDoc ) ;
	m_GroupDialog.SetCurrentDocument( pDoc ) ;

	// tom morris feb 2005
	m_ModelsDialog.SetCurrentDocument( pDoc ) ;
	//	end tom morris feb 2005
}

void CMainFrame::SetCurrentDocument(CJweDoc *pDoc)
{
    if( pDoc == nullptr ) {
        m_pCurrentDoc = nullptr;
        return;
    }

	if( m_pCurrentDoc != pDoc )
	{
		m_pCurrentDoc = pDoc ;
		if( m_pCurrentDoc == nullptr )
		{
			ASSERT( 0 ) ;	// Handle no document
		}
		else
		{
			m_ListsDialog.SetCurrentDocument( pDoc ) ;
			m_GroupDialog.SetCurrentDocument( pDoc ) ;
			m_ModelsDialog.SetCurrentDocument( pDoc ) ;
		}
	}
}// SetCurrentDocument

void CMainFrame::UpdatePanel( MAINFRM_PANEL ePanel )
{
	CJweDoc * pDoc = GetCurrentDocument() ;
	
	if( pDoc == nullptr )
	{
		ASSERT( 0 ) ;	// Empty the tab(s) ?
	}
	else
	{
		m_ListsDialog.Update( pDoc ) ;
		m_GroupDialog.Update( pDoc ) ;
		m_ModelsDialog.Update( pDoc );
	}// Good Doc
	ePanel;
}// UpdatePanel


// World object added, update UI
void CMainFrame::AddObject(Object *pObject)
{
	m_ListsDialog.AddObject( pObject ) ;
	m_GroupDialog.AddObject( pObject );
	m_ModelsDialog.AddObject( pObject );
}// AddObject

// World object added, update UI
void CMainFrame::AddObjectEx(Object *pObject, uint32 flags)
{
	if (flags & MAINFRM_ADDOBJECT_LIST) {
		m_ListsDialog.AddObject( pObject ) ;
	}
	if (flags & MAINFRM_ADDOBJECT_GROUP) {
		m_GroupDialog.AddObject( pObject );
	}
	if (flags & MAINFRM_ADDOBJECT_MODEL) {
		m_ModelsDialog.AddObject( pObject );
	}
}// AddObject

// World object added, update UI
void CMainFrame::RenameObject(Object *pObject)
{
	m_ListsDialog.RenameObject( pObject );
	m_GroupDialog.RenameObject( pObject );
	m_ModelsDialog.RenameObject( pObject );
}// AddObject

// World selections added, update UI
void CMainFrame::AddSelection(CJweDoc *pDoc)
{
	ASSERT( pDoc != NULL ) ;
	m_ListsDialog.AddSelection( pDoc ) ;
	m_GroupDialog.AddSelection( pDoc ) ;
	m_ModelsDialog.AddSelection( pDoc );
}// AddSelection

// World selections deleted, update UI
void CMainFrame::RemoveDeleted()
{
	m_ListsDialog.RemoveDeleted();
	m_GroupDialog.RemoveDeleted();
	m_ModelsDialog.RemoveDeleted();
}// RemoveSelection

// begin krouer - new update system
BOOL CMainFrame::OnBarCheck(UINT nID) 
{
	CControlBar* pBar = GetControlBar(nID);
	if (pBar != nullptr)
	{
		ShowControlBar(pBar, (pBar->GetStyle() & WS_VISIBLE) == 0, FALSE);
		return TRUE;
	}
	return CMDIFrameWnd::OnBarCheck(nID);
}

void CMainFrame::OnUpdateControlBarMenu(CCmdUI* pCmdUI) 
{
	CControlBar* pBar = GetControlBar(pCmdUI->m_nID);
	if (pBar != nullptr)
	{
		pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
		return;
	}
	CMDIFrameWnd::OnUpdateControlBarMenu(pCmdUI);
}
// end krouer - new update toolbar system

// begin krouer - enable Object position and size
void CMainFrame::OnUpdateObjectPosSizeItem(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(TRUE);
}
// end krouer - enable Object position and size

void CMainFrame::OnViewAllmaterials() 
{
	//m_MaterialsBar.ShowWindow( SW_SHOW );
	//DockControlBar( &m_MaterialsBar ) ;
}

void CMainFrame::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CJ3DMainFrame::OnSizing(fwSide, pRect);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CJ3DMainFrame::OnSize(nType, cx, cy);
}

void CMainFrame::RecalcLayout(BOOL bNotify) 
{
	CJ3DMainFrame::RecalcLayout(bNotify);
}

/*
void CMainFrame::OnLogRecordStart() 
{
	((CJweApp*)AfxGetApp())->StartLogRecord();
}

void CMainFrame::OnLogRecordStop() 
{
	((CJweApp*)AfxGetApp())->EndLogRecord();
}

void CMainFrame::OnLogPlayStart() 
{
	((CJweApp*)AfxGetApp())->StartLogPlay();
}
*/


void CMainFrame::OnModeAbort() 
{
	m_pCurrentDoc->AbortMode();
}

void CMainFrame::OnUpdateModeAbort(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI;	
}

void CMainFrame::OnEditClear() 
{
	m_pCurrentDoc->DeleteSelection();
}

void CMainFrame::OnMove(int x, int y) 
{
	// update doc
	if ( m_pCurrentDoc != nullptr )
	{
		m_pCurrentDoc->UpdateWindow( x, y );
	}
}

void CMainFrame::OnInitMenu(CMenu* pMenu) 
{
	CJweDoc * pDoc;
	pDoc =  GetCurrentDocument();

	if( pDoc != nullptr )
	{
		pDoc->RenderAnimate( JE_FALSE );
//		SetCurAnimateState( JE_FALSE );
	}
	CJ3DMainFrame::OnInitMenu(pMenu);
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	return CJ3DMainFrame::OnCommand(wParam, lParam);
}

LRESULT CMainFrame::OnRebuildProperties( WPARAM wParam, LPARAM lParam )
{
	m_pCurrentDoc->UpdateProperties();
	lParam;
	wParam;
	return( TRUE );
}


void CMainFrame::PostUpdateProperties()
{
	PostMessage( WM_REBUILDPROPERIES, 0, 0 );
}

/* to be deleted JH
void CMainFrame::OnUpdateAll() 
{
	m_pCurrentDoc->UpdateAll();
	
}*/

// Added JH 4.3.2000

struct tagACCEL KEY_UserDefined_Accel[]=
{ 
// User defineable Keys
	{FVIRTKEY,0,IDM_MODE_ADJUST},
	{FVIRTKEY,0,IDM_MODE_ROTATESHEAR},
	{FVIRTKEY,0,IDM_MODE_VERTEX},
	{FVIRTKEY,0,IDM_TOOLS_PLACECUBE},
	{FVIRTKEY,0,IDM_TOOLS_PLACECYLINDER},
	{FVIRTKEY,0,IDM_TOOLS_PLACESPHEROID},
	{FVIRTKEY,0,IDM_TOOLS_PLACESHEET},
	{FVIRTKEY,0,IDM_TOOLS_PLACELIGHT},
	{FVIRTKEY,0,IDM_TOOLS_PLACECAMERA},
	{FVIRTKEY,0,IDM_TOOLS_PLACEUSEROBJ},
	{FVIRTKEY,0,IDM_ANIM},
	{FVIRTKEY,0,IDM_TOOLS_UPDATE_SELECTION},
	{FVIRTKEY,0,IDS_UPDATE_ALL},

	{FVIRTKEY,0,IDM_FULLSCREEN},


	{FVIRTKEY,0,IDM_FILE_PREFS},

	{FVIRTKEY,0,IDM_EDIT_SELECTALL},
	{FVIRTKEY,0,IDM_EDIT_SELECTNONE},
	{FVIRTKEY,0,IDM_EDIT_SELECTINVERT},

	
	{FVIRTKEY,0,IDM_VIEW_CENTERSELCTION},

	{FVIRTKEY,0,IDM_OPTIONS_SNAPTOGRID},

	{FVIRTKEY,0,IDM_EDIT_ALIGN_BOTTOM},
	{FVIRTKEY,0,IDM_EDIT_ALIGN_TOP},
	{FVIRTKEY,0,IDM_EDIT_ALIGN_RIGHT},
	{FVIRTKEY,0,IDM_EDIT_ALIGN_LEFT},
	{FVIRTKEY,0,IDM_EDIT_ROTL},
	{FVIRTKEY,0,IDM_EDIT_ROTR},

	{FVIRTKEY,0,ID_FILE_FILEPROPERTIES},
	{FVIRTKEY,VK_F8,IDM_TOOLS_REBUILDALL},

// Add new keys right here...


// These must not be changed
	{FVIRTKEY|FALT,'C',ID_EDIT_COPY},
	{FVIRTKEY|FCONTROL,'N',ID_FILE_NEW},
	{FVIRTKEY|FCONTROL,'O',ID_FILE_OPEN},
	{FVIRTKEY|FCONTROL,'P',ID_FILE_PRINT},
	{FVIRTKEY|FCONTROL,'S',ID_FILE_SAVE},
	{FVIRTKEY|FCONTROL,'V',ID_EDIT_PASTE},
	{FVIRTKEY|FALT,VK_BACK,ID_EDIT_UNDO},
	{FVIRTKEY,VK_DELETE,ID_EDIT_CLEAR},
	{FVIRTKEY|FSHIFT,VK_DELETE,ID_EDIT_CUT},
	{FVIRTKEY,VK_ESCAPE,ID_MODE_ABORT},
	{FVIRTKEY,VK_F1,ID_HELP},
	{FVIRTKEY|FSHIFT,VK_F1,ID_CONTEXT_HELP},
	{FVIRTKEY,VK_F6,ID_PREV_PANE},
	{FVIRTKEY|FSHIFT,VK_F6,ID_NEXT_PANE},
	{FVIRTKEY,VK_F8,IDM_TOOLS_REBUILDALL},
	{FVIRTKEY|FCONTROL,VK_INSERT,ID_EDIT_COPY},
	{FVIRTKEY|FSHIFT,VK_INSERT,ID_EDIT_PASTE},
	{FVIRTKEY,VK_NEXT,IDM_TOOLS_PREVFACE},
	{FVIRTKEY,VK_PRIOR,IDM_TOOLS_NEXTFACE},
	{FVIRTKEY,VK_RETURN,IDM_TOOLS_UPDATE_SELECTION},
	{FVIRTKEY|FCONTROL,VK_RETURN,IDM_TOOLS_SUBTRACTBRUSH},
	{FVIRTKEY,VK_SUBTRACT,IDM_VIEW_ZOOMOUT},
	{FVIRTKEY,VK_ADD,IDM_VIEW_ZOOMIN},
	{FVIRTKEY|FCONTROL,'X',ID_EDIT_CUT},
	{FVIRTKEY|FCONTROL,'Z',ID_EDIT_UNDO},
	{0,0,0}
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Set Accelerator for MainFrame
//
void CMainFrame::SetAccelerator()
{
	int     cAccelerators=KEYS_ACT_KEYMAPPINGS_DEFINED;
	int		iKeys;

	for (iKeys=0;KEY_UserDefined_Accel[iKeys].cmd != 0;iKeys++);

	
	for (int X=0;X<KEYS_ACT_KEYMAPPINGS_DEFINED;X++)
		{ long Key = Settings_GetKey (X+1);
		  KEY_UserDefined_Accel[X].fVirt=(BYTE)(FVIRTKEY|((Key&0xffff)<<2));
		  KEY_UserDefined_Accel[X].key=(unsigned short)(Key>>16);
		}

	HACCEL  haccel= CreateAcceleratorTable((struct tagACCEL*)&KEY_UserDefined_Accel, iKeys); 

	HACCEL  hOldTable = m_hAccelTable;

	if (!::DestroyAcceleratorTable(hOldTable)) 
		{
		  ::LocalFree(haccel);
		  return ;
		}

	m_hAccelTable = haccel;
	
	return ;
}


// END JH


//	tom morris -- new texturedlg feb.2005


void CMainFrame::OnIdrDeleteitem()
{
	if (m_TextureDialog.m_hWnd)
		m_TextureDialog.OnIdrDeleteitem();
}


void CMainFrame::OnUpdateIdrDeleteitem(CCmdUI *pCmdUI)
{
	if (m_TextureDialog.m_hWnd)
		m_TextureDialog.OnUpdateIdrDeleteitem(pCmdUI);
}


void CMainFrame::OnIdrAddgroup()
{
	if (m_TextureDialog.m_hWnd)
		m_TextureDialog.OnIdrAddgroup();
}


void CMainFrame::OnUpdateIdrAddgroup(CCmdUI *pCmdUI)
{
	if (m_TextureDialog.m_hWnd)
		m_TextureDialog.OnUpdateIdrAddgroup(pCmdUI);
}


BOOL CMainFrame::CreateProgressBar(CProgressCtrl* pProgressCtrl,
				CStatusBar* pStatusBar,
				LPCTSTR szMessage,
				int nPaneIndex,
				int cxMargin,
				int cxMaxWidth,
				UINT nIDControl)
{
	ASSERT_VALID( pProgressCtrl );
	ASSERT_VALID( pStatusBar );

	//	Calculate destination rectangle for progress control
	CRect rc{};
    pStatusBar->GetItemRect( nPaneIndex, &rc );

	//	Define progress bar horizontal offset
	if( szMessage != nullptr )
	{
		// Compute message text extent
		CClientDC dc( pStatusBar );
		CFont* pFont = pStatusBar->GetFont();
		CFont* pOldFont = dc.SelectObject( pFont );
		CSize sizeText = dc.GetTextExtent( szMessage );
		dc.SelectObject( pOldFont );

		rc.left += sizeText.cx + cxMargin;
	}

	//	Compute progress bar width
	if( cxMaxWidth != -1 )
	{
		rc.right = rc.left + min( cxMaxWidth, rc.Width() );
	}

	//	Create progress control
    return pProgressCtrl->Create(	WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
									rc, pStatusBar, nIDControl );
}

void CMainFrame::ProgressBarBegin(CString strMessage)
{
	CreateProgressBar(&m_ProgressControl,&m_wndStatusBar, strMessage, 0, 10, 200, 1);
}

void CMainFrame::ProgressBarSetRange(int iRange, CString strMessage)
{
	m_wndStatusBar.SetPaneText(0, strMessage);
	m_ProgressControl.SetRange(0, iRange);
}

void CMainFrame::ProgressBarSetStep(int iStep, CString strMessage)
{
	m_wndStatusBar.SetPaneText(0, strMessage);
	m_ProgressControl.SetStep(iStep);
}

void CMainFrame::ProgressBarStep(CString strMessage)
{
	m_wndStatusBar.SetPaneText(0, strMessage);
	m_ProgressControl.StepIt();

}

void CMainFrame::ProgressBarEnd()
{
	m_wndStatusBar.SetPaneText(0, "");
	m_ProgressControl.DestroyWindow();
}


CStatusBar	*CMainFrame::GetStatusBar()
{
	return &m_wndStatusBar;
}

//	end tom morris -- new texture dialog
