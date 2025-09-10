/****************************************************************************************/
/*  TEMPLATES.CPP                                                                       */
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
// Templates.cpp : implementation file
// NOTE:  THIS IS REALLY CubeTemplate.cpp

#include "stdafx.h"
#include "Defs.h"
#include "jwe.h"
#include "MainFrm.h"
#include "MfcUtil.h"

#include "Templates.h"
#include "Util.h"

#define DEFAULT_BOTX	(64.0f)
#define DEFAULT_BOTZ	(64.0f)
#define DEFAULT_TOPX	(64.0f)
#define DEFAULT_TOZ		(64.0f)
#define DEFAULT_Y		(64.0f)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplates dialog


CTemplates::CTemplates(CWnd* pParent /*=NULL*/)
	: CDialog(CTemplates::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTemplates)
	m_BotX = 0.0f;
	m_BotZ = 0.0f;
	m_TopX = 0.0f;
	m_TopZ = 0.0f;
	m_Y = 0.0f;
	m_Name = _T("");
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	//}}AFX_DATA_INIT
}


void CTemplates::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplates)
	DDX_Text(pDX, BOX_ED_BOTX, m_BotX);
	DDX_Text(pDX, BOX_ED_BOTZ, m_BotZ);
	DDX_Text(pDX, BOX_ED_TOPX, m_TopX);
	DDX_Text(pDX, BOX_ED_TOPZ, m_TopZ);
	DDX_Text(pDX, BOX_ED_Y, m_Y);
	DDX_Text(pDX, BOX_ED_NAME, m_Name);
	DDX_Check(pDX, IDM_TOOLS_PLACEARCH, m_Arch);
	DDX_Check(pDX, IDM_TOOLS_PLACECONE, m_Cone);
	DDX_Check(pDX, IDM_TOOLS_PLACECUBE, m_Cube);
	DDX_Check(pDX, IDM_TOOLS_PLACECYLINDER, m_Cylinder);
	DDX_Check(pDX, IDM_TOOLS_PLACEENTITY, m_Entity);
	DDX_Check(pDX, IDM_TOOLS_PLACELIGHT, m_Light);
	DDX_Check(pDX, IDM_TOOLS_PLACESPHEROID, m_Sphere);
	DDX_Check(pDX, IDM_TOOLS_PLACESTAIRCASE, m_Stair);
	DDX_Check(pDX, IDM_TOOLS_PLACETERRAIN, m_Terrain);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTemplates, CDialog)
	//{{AFX_MSG_MAP(CTemplates)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_BN_CLICKED(IDM_TOOLS_PLACEENTITY, OnToolsPlaceentity)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_EN_KILLFOCUS(BOX_ED_BOTX, OnKillfocusEdBotx)
	ON_EN_KILLFOCUS(BOX_ED_BOTZ, OnKillfocusEdBotz)
	ON_EN_KILLFOCUS(BOX_ED_TOPX, OnKillfocusEdTopx)
	ON_EN_KILLFOCUS(BOX_ED_TOPZ, OnKillfocusEdTopz)
	ON_EN_KILLFOCUS(BOX_ED_Y, OnKillfocusEdY)
	ON_EN_KILLFOCUS(BOX_ED_NAME, OnKillfocusEdName)
	ON_BN_CLICKED(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_COMMAND(IDM_TOOLS_ADDBRUSH, OnToolsAddbrush)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_SUBTRACT, OnSubtract)
	ON_BN_CLICKED(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplates message handlers

BOOL CTemplates::OnInitDialog() 
{

	SetupTemplateDialogIcons( this) ;

	PositionDialogUnderTabs( this ) ;
	
	m_Name.LoadString( IDS_BRUSH_BOX ) ;
	m_BotX =  DEFAULT_BOTX;
	m_BotZ =  DEFAULT_BOTZ;
	m_TopX =  DEFAULT_TOPX;
	m_TopZ =  DEFAULT_TOZ;
	m_Y =	  DEFAULT_Y	;

	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}// OnInitDialog

void CTemplates::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CTemplates::OnToolsPlacelight() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_LIGHT, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}
}// OnToolsPlacelight

void CTemplates::OnToolsPlaceentity() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_ENTITY ) ;
	}
}// OnToolsPlaceentity

void CTemplates::OnToolsPlaceterrain() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_TERRAIN, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_TERRAIN ) ;
	}
	
}

void CTemplates::OnToolsPlacespheroid() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SPHERE ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SPHEREBRUSH ) ;
	}	
}

void CTemplates::OnKillfocusEdBotx() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
	
}

void CTemplates::OnKillfocusEdBotz() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
	
}

void CTemplates::OnKillfocusEdTopx() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
	
}

void CTemplates::OnKillfocusEdTopz() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
	
}

void CTemplates::OnKillfocusEdY() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
	
}


void CTemplates::UpdateTemplate()
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc != NULL )
	{
		pDoc->SetBoxInfo( m_TopX, m_BotX, m_Y, m_TopZ, m_BotZ );
	}
}// UpdateTemplate

void CTemplates::OnKillfocusEdName() 
{
	UpdateData( true ) ;

	TrimString( m_Name ) ;
	if( m_Name.IsEmpty() )
		m_Name.LoadString( IDS_DEFAULTSPHERETEMPLATENAME ) ;

	UpdateData( false ) ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc != NULL )
	{
		Object_SetName( pDoc->GetTemplate(), m_Name, SELECT_INVALID_NNUMBER ) ;
	}
}

void CTemplates::OnToolsPlacecube() 
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	UpdateData( true );
	pDoc->SetTemplateVisable( m_Cube );
	UpdateData( false );
}

void CTemplates::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	UpdateData( true );
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = TRUE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	UpdateData( false );
	
}

void CTemplates::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

BOOL CTemplates::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	
	if( wParam == 1 )
	{
		if( Util_IsKeyDown( VK_CONTROL ) )
			pDoc->Subtractbrush();
		else
			pDoc->Addbrush();
		return( false );
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CTemplates::OnToolsAddbrush() 
{
	int x;

	x = 1;
	if( x )
		x = 2;
}

void CTemplates::OnToolsPlacecylinder() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_CYLINDER ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_CYLNDBRUSH ) ;
	}
}

void CTemplates::OnAdd() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Addbrush();
}

void CTemplates::OnReset() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->ResetTemplate();
	}	
	
}

void CTemplates::OnSubtract() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Subtractbrush();	
	
}

void CTemplates::OnToolsPlacesheet() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SHEET ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SHEETBRUSH ) ;
	}
	
}
