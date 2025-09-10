/****************************************************************************************/
/*  TEMPLATECYLND.CPP                                                                   */
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
#include "Defs.h"
#include "jwe.h"
#include "MainFrm.h"
#include "MfcUtil.h"

#include "Util.h"
#include "templatecylnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_BOTX	(64.0f)
#define DEFAULT_BOTZ	(64.0f)
#define DEFAULT_TOPX	(64.0f)
#define DEFAULT_TOZ		(64.0f)
#define DEFAULT_STRIPES		(6)

/////////////////////////////////////////////////////////////////////////////
// TemplateCylnd dialog


TemplateCylnd::TemplateCylnd(CWnd* pParent /*=NULL*/)
	: CDialog(TemplateCylnd::IDD, pParent)
{
	//{{AFX_DATA_INIT(TemplateCylnd)
	m_BotX = 0.0f;
	m_BotZ = 0.0f;
	m_Name = _T("");
	m_Stripes = 0;
	m_TopX = 0.0f;
	m_TopZ = 0.0f;
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


void TemplateCylnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TemplateCylnd)
	DDX_Text(pDX, CYLD_ED_BOTX, m_BotX);
	DDX_Text(pDX, CYLD_ED_BOTZ, m_BotZ);
	DDX_Text(pDX, CYLD_ED_NAME, m_Name);
	DDX_Text(pDX, CYLD_ED_STRIPES, m_Stripes);
	DDX_Text(pDX, CYLD_ED_TOPX, m_TopX);
	DDX_Text(pDX, CYLD_ED_TOPZ, m_TopZ);
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


BEGIN_MESSAGE_MAP(TemplateCylnd, CDialog)
	//{{AFX_MSG_MAP(TemplateCylnd)
	ON_EN_KILLFOCUS(CYLD_ED_BOTX, OnKillfocusEdBotx)
	ON_EN_KILLFOCUS(CYLD_ED_BOTZ, OnKillfocusEdBotz)
	ON_EN_KILLFOCUS(CYLD_ED_NAME, OnKillfocusEdName)
	ON_EN_KILLFOCUS(CYLD_ED_STRIPES, OnKillfocusEdStripes)
	ON_EN_KILLFOCUS(CYLD_ED_TOPX, OnKillfocusEdTopx)
	ON_EN_KILLFOCUS(CYLD_ED_TOPZ, OnKillfocusEdTopz)
	ON_BN_CLICKED(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_BN_CLICKED(IDM_TOOLS_PLACEENTITY, OnToolsPlaceentity)
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_SUBTRACT, OnSubtract)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TemplateCylnd message handlers

BOOL TemplateCylnd::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	SetupTemplateDialogIcons( this) ;

	PositionDialogUnderTabs( this ) ;
	
	m_Name.LoadString( IDS_BRUSH_CYLINDER ) ;
	m_BotX =  DEFAULT_BOTX;
	m_BotZ =  DEFAULT_BOTZ;
	m_TopX =  DEFAULT_TOPX;
	m_TopZ =  DEFAULT_TOZ;
	m_Stripes =	  DEFAULT_STRIPES	;

	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TemplateCylnd::OnKillfocusEdBotx() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
}

void TemplateCylnd::OnKillfocusEdBotz() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
}

void TemplateCylnd::OnKillfocusEdName() 
{
	// TODO: Add your control notification handler code here
	
}

void TemplateCylnd::OnKillfocusEdStripes() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
}

void TemplateCylnd::OnKillfocusEdTopx() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
}

void TemplateCylnd::OnKillfocusEdTopz() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
}

void TemplateCylnd::OnToolsPlacecube() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_BOX ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_BOXBRUSH ) ;
	}	
}

void TemplateCylnd::OnToolsPlacecylinder() 
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	UpdateData( true );
	pDoc->SetTemplateVisable( m_Cylinder );
	UpdateData( false );
}

void TemplateCylnd::OnToolsPlaceentity() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_ENTITY ) ;
	}
}

void TemplateCylnd::OnToolsPlacelight() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_LIGHT, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}
}

void TemplateCylnd::OnToolsPlacespheroid() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SPHERE ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SPHEREBRUSH ) ;
	}	
}

void TemplateCylnd::OnToolsPlaceterrain() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_TERRAIN, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_TERRAIN ) ;
	}
	
}

void TemplateCylnd::UpdateTemplate()
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc != NULL )
	{
		pDoc->SetCylndInfo( m_TopX, m_BotX, m_Stripes, m_TopZ, m_BotZ );
	}
}// UpdateTemplate

BOOL TemplateCylnd::OnCommand(WPARAM wParam, LPARAM lParam) 
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

void TemplateCylnd::OnAdd() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Addbrush();
}

void TemplateCylnd::OnSubtract() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Subtractbrush();	
}

void TemplateCylnd::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	UpdateData( true );
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = TRUE;
	m_Entity = FALSE;
	m_Light = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	UpdateData( false );
	
}

void TemplateCylnd::OnToolsPlacesheet() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SHEET ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SHEETBRUSH ) ;
	}
	
}

void TemplateCylnd::OnReset() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->ResetTemplate();
	}	
}
