// Preferences.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "jwe.h"
#include "Util.h"
#include "doc.h"
#include "MainFrm.h"

#include "Preferences.h"
#include "Settings.h"

#include "JetView.h"
#include "DRVLIST.h"

#include "PressKey.h"
#include "BtnST.h"

#include "ExtFileDialog.h"       


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct KeyBinding
{
	char KeyName[100];
	UINT KeyId;
	LONG KeyCode;
};

#define PREFS_MAX_KEYS	200

struct KeyBinding sKeyBinding[PREFS_MAX_KEYS]=
{
       
	{"         --- Add Objects ---"	,kb_SEPERATOR,0},
	{"Add Cube"						,kb_ADD_CUBE,0},
	{"Add Cylinder"					,kb_ADD_CYLINDER,0},
	{"Add Sphere"					,kb_ADD_SPHERE,0},
	{"Add UserBrush"				,kb_ADD_USERBRUSH,0},
	{"Add Light"					,kb_ADD_LIGHT,0},
	{"Add Camera"					,kb_ADD_CAMERA,0},
	{"Add Object"					,kb_ADD_OBJECT,0},
	
	{"        --- Modify Objects ---",kb_SEPERATOR,0},
	{"Move/Scale Objects"			,kb_MOD_MOVESCALE,0},
	{"Rotate/Shear Objects"			,kb_MOD_ROTSHARE,0},
	{"Vertex Manipulation"			,kb_MOD_VERTEXMAN,0},

	{"        --- Update ---"		,kb_SEPERATOR,0},
	{"Update Selection"				,kb_UPDATE_SELECTION,0},
	{"Update All"					,kb_UPDATE_ALL,0},
	
	{"        --- Edit ---"			,kb_SEPERATOR,0},
	{"Select All Objects"			,kb_EDIT_SELECTALL,0},
	{"Select No Object"				,kb_EDIT_SELECTNONE,0},
	{"Inverse Selection"			,kb_EDIT_SELECTINVERT,0},

	{"        --- Edit Align---"			,kb_SEPERATOR,0},
	
	{"Align Objects to the top"		,kb_EDIT_TOP,0},
	{"Align Objects to the bottom"	,kb_EDIT_BOTTOM,0},
	{"Align Objects to the left"	,kb_EDIT_LEFT,0},
	{"Align Objects to the right"	,kb_EDIT_RIGHT,0},

	{"        --- Edit Rotate---"			,kb_SEPERATOR,0},
	{"Rotate Objects -90°"		,kb_EDIT_ROTL,0},
	{"Rotate Objects +90°"		,kb_EDIT_ROTR,0},


	{"        --- View  ---"		,kb_SEPERATOR,0},	
	{"Center View on Selection"		,kb_VIEW_CENTERSELCTION,0},
	{"Toggle Snap"					,kb_OPTIONS_SNAPTOGRID,0},

	{"        --- Dialogs  ---"		,kb_SEPERATOR,0},	
	{"Rebuild Dialog"				,kb_TOOLS_REBUILDALL,0},
	{"Preferences Dialog"			,kb_FILE_PREFS,0},
	{"Properties Dialog"			,kb_FILE_FILEPROPERTIES,0},

	{"        --- Misc ---"			,kb_SEPERATOR,0},
	{"Animate"						,kb_ANIMATE,0},
	{"Switch to Fullscreen"			,kb_MISC_FULLSCREEN,0},
	{"",-1,-1}
};


/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CPreferences 


CPreferences::CPreferences(CWnd* pParent /*=nullptr*/)
	: CDialog(CPreferences::IDD, pParent)
{
	m_pPreferencesIconTab = nullptr;
	//{{AFX_DATA_INIT(CPreferences)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CPreferences::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreferences)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPreferences, CDialog)
	//{{AFX_MSG_MAP(CPreferences)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR1,OnSetColor1)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR2,OnSetColor2)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR3,OnSetColor3)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR4,OnSetColor4)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR5,OnSetColor5)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR6,OnSetColor6)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR7,OnSetColor7)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR8,OnSetColor8)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR9,OnSetColor9)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR10,OnSetColor10)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR11,OnSetColor11)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR12,OnSetColor12)
		ON_BN_CLICKED (IDC_VIEW_SET_COLOR13,OnSetColor13)
		ON_BN_CLICKED (IDC_PREFS_VIEW,OnGridView)
		ON_BN_CLICKED (IDC_PREFS_EDIT,OnGridSnap)
		ON_BN_CLICKED (IDC_PREFS_KEYBOARD,OnKeyboard)
		ON_BN_CLICKED (IDC_PREFS_PATHS,OnPath)
		ON_BN_CLICKED (IDC_PREFS_MOUSE,OnMouse)
		ON_BN_CLICKED (IDC_PREFS_JET,OnJet)
		ON_BN_CLICKED (IDC_CHOOSE_WINDOW,OnEngine_GetWindowMode)
		ON_BN_CLICKED (IDC_CHOOSE_FULLSCREEN,OnEngine_GetFullscreenMode)
		ON_BN_CLICKED (IDC_JET_STAIR, OnJet_Stairs)

		ON_BN_CLICKED (IDC_CHECK_CONTROL,OnKey_CheckControl)
		ON_BN_CLICKED (IDC_CHECK_SHIFT,OnKey_CheckShift)
		ON_BN_CLICKED (IDC_CHECK_ALT,OnKey_CheckAlt)

		ON_BN_CLICKED (IDC_ASSIGN,OnKey_Assign)

		ON_BN_CLICKED (IDC_PRESSKEY,OnKey_PressKey)
		ON_EN_CHANGE(IDC_KEYEDIT, OnChangeKeyEdit)


		ON_BN_CLICKED (IDC_EXPORTKEYS,OnKey_Export)
		ON_BN_CLICKED (IDC_IMPORTKEYS,OnKey_Import)

		ON_NOTIFY(NM_CLICK, IDC_KEYS, OnClickKeys)
		ON_NOTIFY(LVN_KEYDOWN, IDC_KEYS, OnKeydownKeys)

		ON_BN_CLICKED (IDC_OK,OnPrefsOK)

		ON_BN_CLICKED (IDC_PREFS_GLOBAL,OnGlobal)

//		ON_BN_CLICKED (IDC_PREFS_INFO,OnInfo)

		ON_WM_PAINT	  ( )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CPreferences 
 

BOOL CPreferences::OnInitDialog()
{
	HINSTANCE				hRes{};
	hRes = AfxGetResourceHandle() ;
	CJweApp* pApp{};
	pApp = (CJweApp*)AfxGetApp() ;
	CMainFrame* pMainFrm{};
	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;	

	if (pMainFrm)
	{
	//	CJweDoc* pDoc{};
	//	pDoc = pMainFrm->GetCurrentDocument();
	
	CDialog::OnInitDialog();

	m_pPreferencesIconTab = new CIconTabCtrl(this, 20,120);
	if (!m_pPreferencesIconTab) return FALSE;
 	
	m_pPreferencesIconTab->Group_AddObjects (0,"JDesigner Paths...",IDC_PREFS_PATHS,IDI_PREFS_PATHS,IDI_PREFS_PATHS,
				IDC_STATIC_UBRUSH,IDC_PATH_UBRUSH,IDC_PATH_TEXTURES,IDC_SET_PATH_UBRUSH,
				IDC_SET_PATH_TEXTURES,IDC_STATIC_TEXTURES,
				IDC_SET_PATH_SHADERS,IDC_STATIC_SHADERS,IDC_PATH_SHADERS,
				-1);

	m_pPreferencesIconTab->Group_AddObjects (1,"Editor Action Settings...",IDC_PREFS_EDIT,IDI_PREFS_EDIT,IDI_PREFS_EDIT,
				IDC_EDIT_STATIC2,IDC_STATIC_SNAP,IDC_DEGREE_SNAP,IDC_STATIC_SNAPROTATE,
				GRID_CB_ALWAYSSNAPVISIBLE,GRID_RB_1,GRID_RB_2,GRID_RB_3,GRID_RB_4,GRID_RB_5,GRID_RB_6,
				IDC_SNAPVERTEXMANIP,IDC_EDIT_STATIC,IDC_EDIT_WINDOW,IDC_EDIT_CROSSING,-1);

	m_pPreferencesIconTab->Group_AddObjects (2,"Jet Engine Settings...",IDC_PREFS_JET,IDI_PREFS_JET,IDI_PREFS_JET,
				IDC_STATIC_JET_TEXT1,IDC_JET_WINDOW,IDC_CHOOSE_WINDOW,IDC_JET_FULLSCREEN,IDC_CHOOSE_FULLSCREEN,
				IDC_STATIC_JET_TEXT2,IDC_STATIC_JET_TEXT3,IDC_STATIC_JET_TEXT4,
				IDC_JET_COLL,IDC_JET_GRAV,IDC_JET_SLID,IDC_JET_STAIR,IDC_JET_HEIGHT,-1);

	m_pPreferencesIconTab->Group_AddObjects (3,"Keyboard Settings...",IDC_PREFS_KEYBOARD,IDI_PREFS_KEYS,IDI_PREFS_KEYS,
				IDC_KEYS,IDC_ASSIGN,
				IDC_STATIC_SHORTCUT,IDC_STATIC_SHORTCUT1,IDC_STATIC_SHORTCUT2,IDC_STATIC_SHORTCUT3,
				IDC_PRESSKEY,IDC_CHECK_CONTROL,IDC_CHECK_ALT,IDC_CHECK_SHIFT,
				IDC_EXPORTKEYS,IDC_IMPORTKEYS,IDC_KEYEDIT,-1);


	m_pPreferencesIconTab->Group_AddObjects (4,"Mouse Settings...", IDC_PREFS_MOUSE,IDI_PREFS_MOUSE,IDI_PREFS_MOUSE,
				IDC_STATIC_MOUSETEXT1,IDC_STATIC_MOUSETEXT2,IDC_STATIC_MOUSETEXT3,IDC_STATIC_MOUSETEXT4,
				IDC_MOUSE_MIDBUT,IDC_MOUSE_WHEEL2,IDC_HOTSELECT1,IDC_MOUSE_LEFTBUT, IDC_MOUSE_RIGHTBUT,-1);

	m_pPreferencesIconTab->Group_AddObjects (5,"Global Settings...",IDC_PREFS_GLOBAL,IDI_PREFS_GLOBAL,IDI_PREFS_GLOBAL,
				IDC_GLOBAL_STATIC1,IDC_GLOBAL_TOOLBAR_TEXT,IDC_GLOBAL_TOOLBAR_FLAT,
				IDC_GLOBAL_STATIC2,IDC_GLOBAL_STATIC3,IDC_GLOBAL_STATIC4,IDC_GLOBAL_STATIC5,IDC_GLOBAL_STATIC6,
				IDC_GLOBAL_BACKUPONSAVE,IDC_GLOBAL_INCREMENTONSAVE,IDC_GLOBAL_SAVETHUMBERNAIL,IDC_GLOBAL_RECENTFILELIST,
				IDC_GLOBAL_AUTOBACKUP,IDC_GLOBAL_NUMBACKUP,IDC_GLOBAL_INTERVALL,
				IDC_GLOBAL_SPIN1,IDC_GLOBAL_SPIN2,IDC_GLOBAL_SPIN3,
				IDC_GLOBAL_STATIC7,IDC_GLOBAL_STATIC8,IDC_GLOBAL_UNDONUM,IDC_GLOBAL_RELOAD,IDC_GLOBAL_SPIN4,
					-1);


	m_pPreferencesIconTab->Group_AddObjects (6,"Viewport Settings...",IDC_PREFS_VIEW,IDI_PREFS_VIEW,IDI_PREFS_VIEW,
				IDC_STATIC_TEXTCOLOR,
				IDC_VIEW_SET_COLOR1,IDC_VIEW_SET_COLOR2,IDC_VIEW_SET_COLOR3,IDC_VIEW_SET_COLOR4,
				IDC_VIEW_SET_COLOR5,IDC_VIEW_SET_COLOR6,IDC_VIEW_SET_COLOR7,IDC_VIEW_SET_COLOR8,
				IDC_VIEW_SET_COLOR9,IDC_VIEW_SET_COLOR10,IDC_VIEW_SET_COLOR11,IDC_VIEW_SET_COLOR12,
				IDC_VIEW_SET_COLOR13,
				
				IDC_STATIC_SHOW_COLOR1, IDC_STATIC_SHOW_COLOR2,IDC_STATIC_SHOW_COLOR3,IDC_STATIC_SHOW_COLOR4, 
				IDC_STATIC_SHOW_COLOR5, IDC_STATIC_SHOW_COLOR6, IDC_STATIC_SHOW_COLOR7, IDC_STATIC_SHOW_COLOR8, 
				IDC_STATIC_SHOW_COLOR9, IDC_STATIC_SHOW_COLOR10,IDC_STATIC_SHOW_COLOR11,IDC_STATIC_SHOW_COLOR12,
				IDC_STATIC_SHOW_COLOR13,IDC_STATIC_PREVIEW, 
				IDC_STATIC_VIEW_STATIC1,IDC_VIEW_SHOWMOUSEPOS,IDC_VIEW_SHOWSIZE,IDC_VIEW_CROSSCURSOR,IDC_VIEW_RULER,
				IDC_STATIC_VIEW_STATIC2,GRID_VIEW_VIEW1,GRID_VIEW_VIEW2,GRID_VIEW_VIEW3,GRID_VIEW_VIEW4,GRID_VIEW_VIEWSNUM,IDC_STATIC_VIEW_STATIC3,
				-1);


	m_pPreferencesIconTab->RecalcLayout(IDC_STATIC_TEXTCOLOR);
	m_pPreferencesIconTab->SetTitleId (IDC_STATIC_TITLE);
	m_pPreferencesIconTab->SetStatic	(IDC_STATIC_RECT);

	ActKeyCode	=0;

	GetDlgItem( IDC_ASSIGN )->EnableWindow(false);

	// Save Colors
	coSelected		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSelected"	,  RGB( 255, 0, 0 ) );
	coSubSelected	= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSubSelected"	,  RGB( 255, 0, 255));
	coSelectedBk	= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSelectedBk"		,  RGB( 0, 0, 0 ) );
	coGridBackgroud	= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coGridBackgroud"	,  RGB( 128, 128, 128 ) );
	coGrid			= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coGrid"			,  RGB( 100, 100, 100 ) );
	coConstructorLine=((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coConstructorLine",  RGB(0, 200, 200 ) );
	coGridSnap		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coGridSnap"		,  RGB( 0xc0, 0xc0, 0xc0  ) );
	coSubtractBrush	= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSubtractBrush"	,  RGB(  255, 0, 255) );
	coAddBrush		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coAddBrush"		,  RGB(  0, 0, 0) );
	coSubtractNoAssoc=((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSubtractNoAssoc",  RGB( 0, 0, 0 ) );
	coSelectedFace	= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSelectedFace"	,  RGB( 255, 0, 255) );
	coCutBrush		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coCutBrush"		,  RGB( 255, 128, 64 ));

	coTemplate		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coTemplate"		,  RGB( 0, 255, 255 ) );
    
	CButton* pButton{};
	pButton= (CButton*)GetDlgItem(GRID_VIEW_VIEW1);
	pButton->SetIcon(::LoadIcon(hRes,MAKEINTRESOURCE(IDI_ARR_NW)));
	pButton = (CButton*) GetDlgItem(GRID_VIEW_VIEW2);
	pButton->SetIcon(::LoadIcon(hRes,MAKEINTRESOURCE(IDI_ARR_NO)));
	pButton = (CButton*) GetDlgItem(GRID_VIEW_VIEW3);
	pButton->SetIcon(::LoadIcon(hRes,MAKEINTRESOURCE(IDI_ARR_SW)));
	pButton = (CButton*) GetDlgItem(GRID_VIEW_VIEW4);
	pButton->SetIcon(::LoadIcon(hRes,MAKEINTRESOURCE(IDI_ARR_SO)));
	
	
	RECT	ClientRect;
	RECT	WindowRect;
	
	GetDlgItem( IDC_STATIC_RECT )->GetWindowRect( &ClientRect );
	GetWindowRect( &WindowRect );
	WindowRect.bottom=ClientRect.bottom+3;

	MoveWindow( &WindowRect,false );

	if (GetDlgItem(IDC_STATIC_TITLE))
		GetDlgItem(IDC_STATIC_TITLE)->SetFont( &pMainFrm->cBigFont, TRUE);

	OnGlobal ();

	LoadSettings();

	InitKeyListCtrl();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPreferences::OnOK() 
{
}

void CPreferences::OnPrefsOK() 
{
	UpdateData( true ) ;
	SaveSettings();
	delete m_pPreferencesIconTab;
	CDialog::OnOK();
}


int CPreferences::SaveSettings()
{
	char sPrefsString[2000];
	int  iPrefsInt{};

//------------------------- Save Viewport Options

	Settings_SetView_ShowMousePos	(((CButton*)GetDlgItem( IDC_VIEW_SHOWMOUSEPOS))->GetState()&0x0003);
	Settings_SetView_ShowSize		(((CButton*)GetDlgItem( IDC_VIEW_SHOWSIZE))->GetState()&0x0003);
	Settings_SetView_ShowRuler		(((CButton*)GetDlgItem( IDC_VIEW_RULER))->GetState()&0x0003);
	Settings_SetView_CrossCursor	(((CButton*)GetDlgItem( IDC_VIEW_CROSSCURSOR))->GetState()&0x0003);

	if (((CButton*)GetDlgItem( GRID_VIEW_VIEW1))->GetState()&0x0003) iPrefsInt = 0;
		else if (((CButton*)GetDlgItem( GRID_VIEW_VIEW2))->GetState()&0x0003) iPrefsInt = 1;
		else if (((CButton*)GetDlgItem( GRID_VIEW_VIEW3))->GetState()&0x0003) iPrefsInt = 2;
		else if (((CButton*)GetDlgItem( GRID_VIEW_VIEW4))->GetState()&0x0003) iPrefsInt = 3;
	Settings_SetView_PreviewView(iPrefsInt);

	  ((CComboBox*)GetDlgItem( GRID_VIEW_VIEWSNUM))->GetLBText( ((CComboBox*)GetDlgItem( GRID_VIEW_VIEWSNUM))->GetCurSel() , sPrefsString );

	Settings_SetView_Nums ( atoi(sPrefsString) );

//------------------------- Save Grip Options
		
	GetDlgItem(IDC_DEGREE_SNAP)->GetWindowText(sPrefsString,1999);
    Settings_SetGrid_SnapDegrees(sPrefsString);

	Settings_SetGrid_SnapVertexManip(((CButton*)GetDlgItem( IDC_SNAPVERTEXMANIP))->GetState()&0x0003);


	if (((CButton*)GetDlgItem( GRID_RB_1))->GetState()&0x0003) iPrefsInt = 1;
		else if (((CButton*)GetDlgItem( GRID_RB_2))->GetState()&0x0003) iPrefsInt = 2;
		else if (((CButton*)GetDlgItem( GRID_RB_3))->GetState()&0x0003) iPrefsInt = 4;
		else if (((CButton*)GetDlgItem( GRID_RB_4))->GetState()&0x0003) iPrefsInt = 8;
		else if (((CButton*)GetDlgItem( GRID_RB_5))->GetState()&0x0003) iPrefsInt = 16;
		else if (((CButton*)GetDlgItem( GRID_RB_6))->GetState()&0x0003) iPrefsInt = 32;
	Settings_SetGrid_VertexSnap(iPrefsInt);

	if (((CButton*)GetDlgItem( IDC_EDIT_WINDOW))->GetState()&0x0003)
			Settings_SetEdit_Selection(1);
	else	Settings_SetEdit_Selection(0);


//--------------------- Save Settings for Mouse

	Settings_SetMouse_LeftBut	(((CComboBox*)GetDlgItem( IDC_MOUSE_LEFTBUT))->GetCurSel());
	Settings_SetMouse_MidBut	(((CComboBox*)GetDlgItem( IDC_MOUSE_MIDBUT))->GetCurSel());
	Settings_SetMouse_RightBut	(((CComboBox*)GetDlgItem( IDC_MOUSE_RIGHTBUT))->GetCurSel());
	Settings_SetMouse_Wheel		(((CComboBox*)GetDlgItem( IDC_MOUSE_WHEEL2))->GetCurSel());

	Settings_SetMouse_HotSelect(((CButton*)GetDlgItem( IDC_HOTSELECT1))->GetState()&0x0003);

//--------------------- Save Settings for Engine
	Settings_SetJet_Coll	(((CButton*)GetDlgItem( IDC_JET_COLL))->GetState()&0x0003);
	Settings_SetJet_Grav	(((CButton*)GetDlgItem( IDC_JET_GRAV))->GetState()&0x0003);
	Settings_SetJet_Slid	(((CButton*)GetDlgItem( IDC_JET_SLID))->GetState()&0x0003);
	Settings_SetJet_Stair	(((CButton*)GetDlgItem( IDC_JET_STAIR))->GetState()&0x0003);

	GetDlgItem( IDC_JET_HEIGHT)->GetWindowText(sPrefsString,1999);
    Settings_SetJet_Height(sPrefsString);

	GetDlgItem( IDC_JET_WINDOW)->GetWindowText(sPrefsString,1999);
    Settings_SetJet_Window(sPrefsString);

	GetDlgItem( IDC_JET_FULLSCREEN)->GetWindowText(sPrefsString,1999);
    Settings_SetJet_Fullscreen(sPrefsString);

//--------------------- Save Settings for Paths

	GetDlgItem( IDC_PATH_UBRUSH)->GetWindowText(sPrefsString,1999);
    Settings_SetPath_UBrush(sPrefsString);

	GetDlgItem( IDC_PATH_TEXTURES)->GetWindowText(sPrefsString,1999);
    Settings_SetPath_Textures(sPrefsString);

	GetDlgItem( IDC_PATH_SHADERS)->GetWindowText(sPrefsString,1999);
    Settings_SetPath_Shaders(sPrefsString);
	
//-------------------- Save Keysettings

    for (int X=0;sKeyBinding[X].KeyCode!=-1;X++)
		Settings_SetKey(sKeyBinding[X].KeyId, sKeyBinding[X].KeyCode);	

//-------------------- Save Global Settings

    Settings_SetGlobal_ToolbarText	(((CButton*)GetDlgItem( IDC_GLOBAL_TOOLBAR_TEXT))->GetState()&0x0003);
    Settings_SetGlobal_ToolbarFlat	(((CButton*)GetDlgItem( IDC_GLOBAL_TOOLBAR_FLAT))->GetState()&0x0003);
    Settings_SetGlobal_BackupFile	(((CButton*)GetDlgItem( IDC_GLOBAL_BACKUPONSAVE))->GetState()&0x0003);
    Settings_SetGlobal_Thumbnail	(((CButton*)GetDlgItem( IDC_GLOBAL_SAVETHUMBERNAIL))->GetState()&0x0003);

	GetDlgItem( IDC_GLOBAL_UNDONUM)->GetWindowText(sPrefsString,1999);
	if (atoi (sPrefsString)>0)
		Settings_SetGlobal_UndoBuffer	(atoi(sPrefsString));
	else
		Settings_SetGlobal_UndoBuffer	(1);

	// BEGIN - Disable auto save option - paradoxnj 8/11/2005
	Settings_SetAutosaveDisabled(((CButton*)GetDlgItem(IDC_GLOBAL_AUTOBACKUP))->GetState()&0x0003);
	// END - Disable auto save option - paradoxnj 8/11/2005

	return true;
}


int CPreferences::LoadSettings()
{
	char sPrefsString[2000];

//------------------------- Load Viewport Options

	((CButton*)GetDlgItem(IDC_VIEW_SHOWMOUSEPOS))->SetCheck(Settings_GetView_ShowMousePos());
	((CButton*)GetDlgItem(IDC_VIEW_SHOWSIZE))->SetCheck(Settings_GetView_ShowSize());		
	((CButton*)GetDlgItem(IDC_VIEW_RULER))->SetCheck(Settings_GetView_ShowRuler());		
	((CButton*)GetDlgItem(IDC_VIEW_CROSSCURSOR))->SetCheck(Settings_GetView_CrossCursor());


	if (Settings_GetView_PreviewView()==0)  ((CButton*)GetDlgItem( GRID_VIEW_VIEW1))->SetCheck(0x01);
	if (Settings_GetView_PreviewView()==1)  ((CButton*)GetDlgItem( GRID_VIEW_VIEW2))->SetCheck(0x01);
	if (Settings_GetView_PreviewView()==2)  ((CButton*)GetDlgItem( GRID_VIEW_VIEW3))->SetCheck(0x01);
	if (Settings_GetView_PreviewView()==3)  ((CButton*)GetDlgItem( GRID_VIEW_VIEW4))->SetCheck(0x01);

	((CComboBox*)GetDlgItem( GRID_VIEW_VIEWSNUM))->SelectString(-1,_itoa(Settings_GetView_Nums(),sPrefsString,10));



	
//------------------------- Load Grip Options


	GetDlgItem(IDC_DEGREE_SNAP)->SetWindowText(Settings_GetGrid_SnapDegrees(sPrefsString,299));

	if (Settings_GetGrid_VertexSnap()==1)  ((CButton*)GetDlgItem( GRID_RB_1))->SetCheck(0x01);
	if (Settings_GetGrid_VertexSnap()==2)  ((CButton*)GetDlgItem( GRID_RB_2))->SetCheck(0x01);
	if (Settings_GetGrid_VertexSnap()==4)  ((CButton*)GetDlgItem( GRID_RB_3))->SetCheck(0x01);
	if (Settings_GetGrid_VertexSnap()==8)  ((CButton*)GetDlgItem( GRID_RB_4))->SetCheck(0x01);
	if (Settings_GetGrid_VertexSnap()==16) ((CButton*)GetDlgItem( GRID_RB_5))->SetCheck(0x01);
	if (Settings_GetGrid_VertexSnap()==32) ((CButton*)GetDlgItem( GRID_RB_6))->SetCheck(0x01);

	((CButton*)GetDlgItem( IDC_SNAPVERTEXMANIP))->SetCheck( Settings_GetGrid_SnapVertexManip());


	if (Settings_GetEdit_Selection())
			 ((CButton*)GetDlgItem( IDC_EDIT_WINDOW))->SetCheck(0x01);
		else ((CButton*)GetDlgItem( IDC_EDIT_CROSSING))->SetCheck(0x01);

//--------------------- Load Settings for Mouse
	((CComboBox*)GetDlgItem( IDC_MOUSE_LEFTBUT))->SetCurSel(Settings_GetMouse_LeftBut());
	((CComboBox*)GetDlgItem( IDC_MOUSE_MIDBUT))->SetCurSel(Settings_GetMouse_MidBut());
	((CComboBox*)GetDlgItem( IDC_MOUSE_RIGHTBUT))->SetCurSel(Settings_GetMouse_RightBut());
	((CComboBox*)GetDlgItem( IDC_MOUSE_WHEEL2))->SetCurSel(Settings_GetMouse_Wheel());

	((CButton*)GetDlgItem(IDC_HOTSELECT1))->SetCheck(Settings_GetMouse_HotSelect());


//--------------------- Load Settings for Engine

	((CButton*)GetDlgItem(IDC_JET_COLL))->SetCheck(Settings_GetJet_Coll());
	((CButton*)GetDlgItem(IDC_JET_GRAV))->SetCheck(Settings_GetJet_Grav());
	((CButton*)GetDlgItem(IDC_JET_SLID))->SetCheck(Settings_GetJet_Slid());
	((CButton*)GetDlgItem(IDC_JET_STAIR))->SetCheck(Settings_GetJet_Stair());
	if(Settings_GetJet_Stair())
			{	GetDlgItem( IDC_JET_HEIGHT)->EnableWindow (true);
				GetDlgItem( IDC_STATIC_JET_TEXT4)->EnableWindow (true);
			}
	else	{ GetDlgItem( IDC_JET_HEIGHT)->EnableWindow (false);
			  GetDlgItem( IDC_STATIC_JET_TEXT4)->EnableWindow (false);
			}


	GetDlgItem( IDC_JET_HEIGHT)->SetWindowText(Settings_GetJet_Height(sPrefsString,299));
	GetDlgItem( IDC_JET_WINDOW)->SetWindowText(Settings_GetJet_Window(sPrefsString,299));

	GetDlgItem( IDC_JET_FULLSCREEN)->SetWindowText(Settings_GetJet_Fullscreen(sPrefsString,299));

//--------------------- Load Settings for Paths

	GetDlgItem( IDC_PATH_UBRUSH)->SetWindowText(Settings_GetPath_UBrush(sPrefsString,299));
	GetDlgItem( IDC_PATH_TEXTURES)->SetWindowText(Settings_GetPath_Textures(sPrefsString,299));
	GetDlgItem( IDC_PATH_SHADERS)->SetWindowText(Settings_GetPath_Shaders(sPrefsString,299));

//--------------------- Load KeySettings 
	for (int X=0;sKeyBinding[X].KeyCode!=-1;X++)
		{ 
		  sKeyBinding[X].KeyCode = Settings_GetKey(sKeyBinding[X].KeyId);
		}
//-------------------- Load Global Settings

	((CButton*)GetDlgItem(IDC_GLOBAL_TOOLBAR_TEXT))->SetCheck(Settings_GetGlobal_ToolbarText());
	((CButton*)GetDlgItem(IDC_GLOBAL_TOOLBAR_FLAT))->SetCheck(Settings_GetGlobal_ToolbarFlat());
	((CButton*)GetDlgItem(IDC_GLOBAL_BACKUPONSAVE))->SetCheck(Settings_GetGlobal_BackupFile());
 	((CButton*)GetDlgItem(IDC_GLOBAL_SAVETHUMBERNAIL))->SetCheck(Settings_GetGlobal_Thumbnail());

	GetDlgItem(IDC_GLOBAL_UNDONUM)->SetWindowText(_itoa(Settings_GetGlobal_UndoBuffer(),sPrefsString,10));

	// BEGIN - Disable auto save option - paradoxnj 8/11/2005
	((CButton*)GetDlgItem(IDC_GLOBAL_AUTOBACKUP))->SetCheck(Settings_GetAutosaveDisabled());
	// END - Disable auto save option - paradoxnj 8/11/2005

	return true;
}

void CPreferences::OnCancel() 
{
	// Restore Colors
	Settings_SetGridBk				( coGridBackgroud ) ;
	Settings_SetGridColor			( coGrid ) ;
	Settings_SetConstructorColor	( coConstructorLine  );
	Settings_SetGridSnapColor		( coGridSnap ) ;
	Settings_SetSelectedColor		( coSelected ) ;
	Settings_SetSubSelectedColor	( coSubSelected ) ;
	Settings_SetSelectedBk			( coSelectedBk ) ;
	Settings_SetSelectedFaceColor	( coSelectedFace ) ;
	Settings_SetTemplateColor		( coTemplate ) ;
	Settings_SetCutBrushColor		( coCutBrush ) ;

	Settings_SetSubtractBrushColor		( coSubtractBrush ) ;
	Settings_SetAddBrushColor			( coAddBrush  ) ;
	Settings_SetAddSubtractEmptyColor	( coSubtractNoAssoc ) ;

	delete m_pPreferencesIconTab;

	CDialog::OnCancel();
}


void CPreferences::OnSetColor1()		// Settings for SelectedColor
{	
	CColorDialog  GridColorDialog(Settings_GetSelectedColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetSelectedColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor2()		// Settings for SubSelectedColor
{	
	CColorDialog  GridColorDialog(Settings_GetSubSelectedColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetSubSelectedColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor3()		// Settings for GetSelectedBk
{	
	CColorDialog  GridColorDialog(Settings_GetSelectedBk(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetSelectedBk(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor4()		// Settings for Settings_SetGridBk
{	
	CColorDialog  GridColorDialog(Settings_GetGridBk(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetGridBk(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor5()		// Settings for Settings_SetGridColor
{	
	CColorDialog  GridColorDialog(Settings_GetGridColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetGridColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor6()		// Settings for Settings_SetConstructorColor
{	
	CColorDialog  GridColorDialog(Settings_GetConstructorColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetConstructorColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor7()		// Settings for Settings_SetGridSnapColor
{	
	CColorDialog  GridColorDialog(Settings_GetGridSnapColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetGridSnapColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor8()		// Settings for Settings_SetSubtractBrushColor
{	
	CColorDialog  GridColorDialog(Settings_GetSubtractBrushColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetSubtractBrushColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor9()		// Settings for Settings_SetAddBrushColor
{	
	CColorDialog  GridColorDialog(Settings_GetAddBrushColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetAddBrushColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor10()		// Settings for Settings_GetAddSubtractEmptyColor
{	
	CColorDialog  GridColorDialog(Settings_GetAddSubtractEmptyColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetAddSubtractEmptyColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor11()		// Settings for Settings_SetSelectedFaceColor
{	
	CColorDialog  GridColorDialog(Settings_GetSelectedFaceColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetSelectedFaceColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}


void CPreferences::OnSetColor12()		// Settings for Settings_SetCutBrushColor
{	
	CColorDialog  GridColorDialog(Settings_GetCutBrushColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetCutBrushColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}

void CPreferences::OnSetColor13()		// Settings for Settings_SetCutBrushColor
{	
	CColorDialog  GridColorDialog(Settings_GetTemplateColor(),CC_FULLOPEN,this);
	if (GridColorDialog.DoModal())
	{ Settings_SetTemplateColor(GridColorDialog.GetColor( ) );
	  Invalidate (false);
	}
}



// Draw ColorPreview for one button
void CPreferences::DrawGridColor(int DlgItem, int32 Color)
{
	CStatic *pColorStatic = (CStatic*) GetDlgItem(DlgItem);
	
	CBrush ColorBrush;
	ColorBrush.CreateSolidBrush(Color);

	pColorStatic->Invalidate(false);
	
	CDC *pColorCDC = pColorStatic->GetWindowDC( );
	
	pColorCDC->SelectObject(&ColorBrush);
	pColorCDC->Rectangle( 0,0,100,100);
	
	pColorStatic->ReleaseDC(pColorCDC);

}

// Refresh all Views
void CPreferences::OnPaint()
{
	CDialog::OnPaint();

	CBrush ColorBGBrush;

	// Draw Colorpreview
	DrawGridColor(IDC_STATIC_SHOW_COLOR1,Settings_GetSelectedColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR2,Settings_GetSubSelectedColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR3,Settings_GetSelectedBk());
	DrawGridColor(IDC_STATIC_SHOW_COLOR4,Settings_GetGridBk());
	DrawGridColor(IDC_STATIC_SHOW_COLOR5,Settings_GetGridColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR6,Settings_GetConstructorColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR7,Settings_GetGridSnapColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR8,Settings_GetSubtractBrushColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR9,Settings_GetAddBrushColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR10,Settings_GetAddSubtractEmptyColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR11,Settings_GetSelectedFaceColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR12,Settings_GetCutBrushColor());
	DrawGridColor(IDC_STATIC_SHOW_COLOR13,Settings_GetTemplateColor());
	
	
	// Get Pointer to CJweDoc
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;

	if( !pDoc->m_bLoaded )
		return;



	// Draw Preview for new Colors
	CStatic *pColorStatic = (CStatic*) GetDlgItem(IDC_STATIC_PREVIEW);	
	pColorStatic->Invalidate(false);
	pColorStatic->UpdateWindow();

	ColorBGBrush.CreateSolidBrush(Settings_GetGridBk());

	RECT	tempRect;
	pColorStatic->GetWindowRect(&tempRect);

	// Setup default View
	Ortho *Temp=Ortho_Create();
	
	jeVec3d	Angles	  = { 0.0f, 0.0f, 0.0f } ;
	jeVec3d CameraPos = { 0.0f, 0.0f, 0.0f } ;
	
	Ortho_SetZoom  ( Temp, 0.4f ) ;
	Ortho_SetAngles( Temp, &Angles ) ;
	Ortho_SetCameraPos( Temp, &CameraPos ) ;
	Ortho_SetViewType (Temp,Ortho_ViewTop);
    Ortho_ResizeView  (Temp,tempRect.right-tempRect.left,tempRect.bottom-tempRect.top);

	CDC *pStaticCDC = pColorStatic->GetWindowDC( );
	
	pStaticCDC->SelectObject(&ColorBGBrush);
	pStaticCDC->Rectangle( 0,0,tempRect.right-tempRect.left,tempRect.bottom-tempRect.top);
	
	pDoc->DrawGrid( pStaticCDC, Temp);
	pDoc->DrawConstructorLine( pStaticCDC, Temp );
	pDoc->DrawObjects( pStaticCDC, Temp );
	pDoc->DrawSelected( pStaticCDC, Temp );
	pDoc->DrawOrthoName( pStaticCDC, Temp );

	pColorStatic->ReleaseDC(pStaticCDC);
	Ortho_Destroy (&Temp);

}

void	CPreferences::OnInfo()
{		m_pPreferencesIconTab->Group_Display(7);
}

void	CPreferences::OnGridView()
{		m_pPreferencesIconTab->Group_Display(6);
}

void	CPreferences::OnGlobal()
{		m_pPreferencesIconTab->Group_Display(5);
}

void	CPreferences::OnGridSnap()
{		m_pPreferencesIconTab->Group_Display(1);
}


void	CPreferences::OnPath()
{		m_pPreferencesIconTab->Group_Display(0);
}

void	CPreferences::OnJet()
{		m_pPreferencesIconTab->Group_Display(2);
}

void	CPreferences::OnKeyboard()
{		m_pPreferencesIconTab->Group_Display(3);
}

void	CPreferences::OnMouse()
{		m_pPreferencesIconTab->Group_Display(4);
}



void CPreferences::OnEngine_GetWindowMode()
{
	jeDriver* LastDriver{};
	jeDriver_Mode* LastMode{};
	jeDriver* WindowDriver{};
	jeDriver_Mode* WindowMode{};
	jeBoolean		Result{};

	char			VideoName[200];
	const char* cWindowDriver{};
	const char* cWindowMode{};

	CJweApp				*	pApp = (CJweApp*)AfxGetApp() ;

	CMainFrame			*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	
	CJweDoc				*	pDoc = pMainFrm->GetCurrentDocument() ;

	CView* pView=nullptr;

	POSITION pos = pDoc->GetFirstViewPosition();
    while (pos != nullptr)
		   {
			  pView =pDoc->GetNextView(pos);
			  if (pView->IsKindOf(RUNTIME_CLASS(CJ3DView)))
			  { break;
			  }
			 // pDoc->pView->UpdateWindow();
		   }
   
	if (pView!=nullptr)
	{
		// save last driver and mode
		jeEngine_GetDriverAndMode( ((CJ3DView*)pView)->GetEngine(), &LastDriver, &LastMode );
		WindowDriver = LastDriver;
		WindowMode = LastMode;

		// display video mode dialog box
		Result = DrvList_PickDriver(	AfxGetInstanceHandle(),
										GetSafeHwnd(),
										((CJ3DView*)pView)->GetEngine(), 
										&WindowDriver, 
										&WindowMode,
										JE_TRUE,
										DRVLIST_SOFTWARE | DRVLIST_HARDWARE |DRVLIST_WINDOW|DRVLIST_ALL);
		if (Result != JE_FALSE)
			{ jeDriver_GetName( WindowDriver, &cWindowDriver );
			  jeDriver_ModeGetName( WindowMode, &cWindowMode );

			  sprintf_s (VideoName,"%s,%s",cWindowDriver,cWindowMode);
			  GetDlgItem( IDC_JET_WINDOW )->SetWindowText(VideoName);
			}
	}
}

void CPreferences::OnEngine_GetFullscreenMode()
{
	jeDriver* LastDriver{};
	jeDriver_Mode* LastMode{};
	jeDriver* WindowDriver{};
	jeDriver_Mode* WindowMode{};
	jeBoolean		Result{};

	char			VideoName[200];
	const char* cWindowDriver{};
	const char* cWindowMode{};

	CJweApp				*	pApp = (CJweApp*)AfxGetApp() ;

	CMainFrame			*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	
	CJweDoc				*	pDoc = pMainFrm->GetCurrentDocument() ;

	CView* pView=nullptr;

	POSITION pos = pDoc->GetFirstViewPosition();
    while (pos != nullptr)
		   {
			  pView =pDoc->GetNextView(pos);
			  if (pView->IsKindOf(RUNTIME_CLASS(CJ3DView)))
			  { break;
			  }
			 // pDoc->pView->UpdateWindow();
		   }
   
	if (pView!=nullptr)
	{
		// save last driver and mode
		jeEngine_GetDriverAndMode( ((CJ3DView*)pView)->GetEngine(), &LastDriver, &LastMode );
		WindowDriver = LastDriver;
		WindowMode = LastMode;

		// display video mode dialog box
		Result = DrvList_PickDriver(	AfxGetInstanceHandle(),
										GetSafeHwnd(),
										((CJ3DView*)pView)->GetEngine(), 
										&WindowDriver, 
										&WindowMode,
										JE_TRUE,
										DRVLIST_SOFTWARE | DRVLIST_HARDWARE |DRVLIST_FULLSCREEN);
		if (Result != JE_FALSE)
			{ jeDriver_GetName( WindowDriver, &cWindowDriver );
			  jeDriver_ModeGetName( WindowMode, &cWindowMode );

			  sprintf_s (VideoName,"%s,%s",cWindowDriver,cWindowMode);
			  GetDlgItem( IDC_JET_FULLSCREEN )->SetWindowText(VideoName);
			}
	}
}




void CPreferences::OnJet_Stairs()
{
	if (((CButton*)GetDlgItem( IDC_JET_STAIR))->GetState()&0x0003)
			{	GetDlgItem( IDC_JET_HEIGHT)->EnableWindow (true);
				GetDlgItem( IDC_STATIC_JET_TEXT4)->EnableWindow (true);
			}
	else	{ GetDlgItem( IDC_JET_HEIGHT)->EnableWindow (false);
			  GetDlgItem( IDC_STATIC_JET_TEXT4)->EnableWindow (false);
			}
}


void CPreferences::OnKey_PressKey()
{
	CPressKey KeyPressDialog;

	if (KeyPressDialog.DoModal())
	{ ActKeyCode=KeyPressDialog.GetKeyCode();
	  Key_SetName();
	}

/*	int GetKeyNameText(
  LONG lParam,      // second parameter of keyboard message
  LPTSTR lpString,  // pointer to buffer for key name
  int nSize         // maximum length of key-name string length
);
 
   */

//	char	cStringName[200];
//	GetKeyNameText (123,cStringName,199);

}


void    CPreferences::OnKey_CheckControl()
{
	if (((CButton*)GetDlgItem( IDC_CHECK_CONTROL))->GetCheck())
		 ActKeyCode|=2;
	else ActKeyCode&=0xfffffffd;
	Key_SetName();
}

void    CPreferences::OnKey_CheckShift()
{
	if (((CButton*)GetDlgItem( IDC_CHECK_SHIFT))->GetCheck())
		 ActKeyCode|=1;
	else ActKeyCode&=0xfffffffe;
	Key_SetName();
}

void    CPreferences::OnKey_CheckAlt()
{
	if (((CButton*)GetDlgItem( IDC_CHECK_ALT))->GetCheck())
		 ActKeyCode|=4;
	else ActKeyCode&=0xfffffffb;
	Key_SetName();
}

int    CPreferences::InitKeyListCtrl()
{
	char sKeyName[300];
	CListCtrl *pListCtrl = (CListCtrl *) GetDlgItem(IDC_KEYS);

	DWORD dwStyle=pListCtrl->GetExtendedStyle();
	pListCtrl->SetExtendedStyle( dwStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES );
	pListCtrl->DeleteAllItems( );
	pListCtrl->DeleteColumn( 0 );
	pListCtrl->DeleteColumn( 0 );
	pListCtrl->InsertColumn( 0,"Action", LVCFMT_LEFT, 200, 2 );
	pListCtrl->InsertColumn( 1,"Assigned Key", LVCFMT_LEFT, 170, 2 );


	for (int X=0;sKeyBinding[X].KeyCode!=-1;X++)
	{
	  pListCtrl->InsertItem(X,"" );
	  pListCtrl->SetItemText( X, 0, sKeyBinding[X].KeyName);
	  if (sKeyBinding[X].KeyId!=kb_SEPERATOR)
		  pListCtrl->SetItemText( X, 1, TranslateKeyToText(sKeyBinding[X].KeyCode,sKeyName,299) );
	}
	return true;
}


void    CPreferences::OnKey_Assign()
{
	if (ActKeyCode !=0)
		{
			CListCtrl *pListCtrl = (CListCtrl *) GetDlgItem(IDC_KEYS);
			POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
			if (pos != nullptr)
				{
				    int nItem = pListCtrl->GetNextSelectedItem(pos);
					sKeyBinding[nItem].KeyCode = ActKeyCode;
					ActKeyCode=0;
					Key_SetName();
					InitKeyListCtrl();
				}
		}
}


void CPreferences::Key_SetName()
{
	char	cStringName[2000];
	char	cTempStringName[200];


	GetDlgItem( IDC_STATIC_SHORTCUT2)->SetWindowText(TranslateKeyToText(ActKeyCode,cStringName,199));
	cStringName[0]=(char)(ActKeyCode>>16);
	cStringName[1]='\0';
	GetDlgItem( IDC_KEYEDIT)->SetWindowText(cStringName);

	if (strlen (cStringName)==0)
			 GetDlgItem( IDC_ASSIGN )->EnableWindow(false);
		else GetDlgItem( IDC_ASSIGN )->EnableWindow(true);

	((CButton*)GetDlgItem( IDC_CHECK_ALT))->SetCheck((ActKeyCode&0x04)? 1 : 0);
	((CButton*)GetDlgItem( IDC_CHECK_CONTROL))->SetCheck((ActKeyCode&0x02)? 1 : 0);
	((CButton*)GetDlgItem( IDC_CHECK_SHIFT))->SetCheck((ActKeyCode&0x01)? 1 : 0);
	
	cStringName[0]='\0';

	if (ActKeyCode!=0)
	for (int X=0;sKeyBinding[X].KeyCode!=-1;X++)
	{
		if (sKeyBinding[X].KeyCode==ActKeyCode)
			{  
			  if (strlen (cStringName)==0)
				  { strcpy_s (cStringName,"Key used for ");
					sprintf_s (cTempStringName,"'%s' ",sKeyBinding[X].KeyName);
				  }
			  else sprintf_s (cTempStringName,"and '%s' ",sKeyBinding[X].KeyName);
			   
			  strcat_s (cStringName,cTempStringName);
			  if (strlen(cStringName)>1000) break;
			}
		
	}
    GetDlgItem( IDC_STATIC_SHORTCUT3)->SetWindowText(cStringName);
}
//int  ActKeyCode


void CPreferences::OnChangeKeyEdit() 
{
	// TODO: Wenn dies ein RICHEDIT-Steuerelement ist, sendet das Steuerelement diese
	// Benachrichtigung nicht, bevor Sie nicht die Funktion CDialog::OnInitDialog()
	// überschreiben und CRichEditCrtl().SetEventMask() aufrufen, wobei
	// eine ODER-Operation mit dem Attribut ENM_CHANGE und der Maske erfolgt.	

	// TODO: Fügen Sie hier Ihren Code für die Benachrichtigungsbehandlungsroutine des Steuerelements hinzu

	char		cStringName[200];
	static char	UpdateText;

	if (UpdateText==1) return;
	
	UpdateText=1;
	
	GetDlgItem( IDC_KEYEDIT)->GetWindowText(cStringName,199);
	
	cStringName[0]=toupper(cStringName[0]);

	ActKeyCode&=0x0000ffff;
	ActKeyCode|=(cStringName[0]<<16);
	cStringName[1]='\0';
	GetDlgItem( IDC_KEYEDIT)->SetWindowText(cStringName);
	Key_SetName();
	UpdateText=0;
}


char *CPreferences::TranslateKeyToText(UINT KeyCode,char *sKeyText, int Len)
{

	if (KeyCode==0) 
			strcpy (sKeyText,"...");
	else
	{		
	  sprintf (sKeyText,"%s%s%s%c",
		  (KeyCode&0x02)? "CONTROL+" : "" ,
		  (KeyCode&0x04)? "ALT+" : "" ,
		  (KeyCode&0x01)? "SHIFT+" : "" ,
		  (char)(KeyCode>>16));

	}

	return sKeyText;
}


void CPreferences::OnClickKeys(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	*pResult = 0;
	CListCtrl *pListCtrl = (CListCtrl *) GetDlgItem(IDC_KEYS);
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
	if (pos != nullptr)
			{
				int nItem = pListCtrl->GetNextSelectedItem(pos);
				if (sKeyBinding[nItem].KeyId!=kb_SEPERATOR)
					{
					  ActKeyCode = sKeyBinding[nItem].KeyCode;
					  Key_SetName();
					}
				else
					{ pListCtrl->SetItemState(  nItem,0 , -1 );
					  ActKeyCode=0;
					  Key_SetName();
					}
			}
}

void CPreferences::OnKeydownKeys(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	*pResult = 0;
}


void CPreferences::OnKey_Export()
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	FILE	*Out;

	CExtFileDialog *FileDlg= new CExtFileDialog( FALSE,
										  "Export Keyfile...",
										  "*.jky",
										  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,
										  "Jet3D Keyfiles (*.jky)|*.jky|All Files (*.*)|*.*||",
										  this);
	if (FileDlg != nullptr)
		if (FileDlg->DoModal()==IDOK)
		{
		  Out = fopen(FileDlg->GetPathName(),"wt");
		  if (Out==nullptr)
			{ 
			  delete FileDlg;
			  return;
			}
		  fprintf (Out,"#\n#  Jet 3D-KeyFile\n# Please don't try to edit this file.... :)\n#\n");

		  for (int X=0;sKeyBinding[X].KeyCode!=-1;X++)
			{
				fprintf (Out,"%s,%ld,%ld\n",sKeyBinding[X].KeyName,sKeyBinding[X].KeyId,sKeyBinding[X].KeyCode);
			}		
		  fclose (Out);
		}
	delete FileDlg;
}

void CPreferences::OnKey_Import()
{
	FILE* Out{};
	char		sKeyStrings[300];
	char		seps[]   = ",\t\n";
	char* token{};
    int			Y=0;

	CFileDialog *FileDlg= new CFileDialog( TRUE,
										  "Import Keyfile...",
										  "*.jky",
										  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,
										  "Jet3D Keyfiles (*.jky)|*.jky|All Files (*.*)|*.*||",
										  this);
	if (FileDlg != nullptr)
		if (FileDlg->DoModal()==IDOK)
		{
		  Out = fopen (FileDlg->GetPathName(),"rt");
		  if (Out==nullptr)
			{ 
			  delete FileDlg;
			  return;
			}

		  fgets (sKeyStrings,299,Out);
		  fgets (sKeyStrings,299,Out);
		  fgets (sKeyStrings,299,Out);
		  fgets (sKeyStrings,299,Out);

		  for (;;)
		  {
			  if (fgets (sKeyStrings,299,Out)==nullptr) break;

			  token = strtok( sKeyStrings, seps );
			  int X=0;
			  while( token != nullptr )
				{
				if (X==0) strcpy_s (sKeyBinding[Y].KeyName,token);
				if (X==1) sKeyBinding[Y].KeyId	 = atoi (token);
				if (X==2) sKeyBinding[Y].KeyCode = atoi (token);
				X++;
				token = strtok( nullptr, seps );
			  }
			  Y++;
			}		
		  fclose (Out);
		  sKeyBinding[Y].KeyId	= -1;
		  sKeyBinding[Y].KeyCode= -1;
		  Key_SetName();
		  InitKeyListCtrl();
		}
	delete FileDlg;
}





