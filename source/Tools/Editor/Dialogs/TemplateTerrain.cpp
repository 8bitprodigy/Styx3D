/****************************************************************************************/
/*  TEMPLATETERRAIN.CPP                                                                 */
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
#include "TemplateTerrain.h"
#include "Defs.h"
#include "MainFrm.h"
#include "MfcUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplateTerrain dialog


CTemplateTerrain::CTemplateTerrain(CWnd* pParent /*=NULL*/)
	: CDialog(CTemplateTerrain::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTemplateTerrain)
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


void CTemplateTerrain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateTerrain)
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


BEGIN_MESSAGE_MAP(CTemplateTerrain, CDialog)
	//{{AFX_MSG_MAP(CTemplateTerrain)
	ON_BN_CLICKED(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_BN_CLICKED(IDM_TOOLS_PLACEENTITY, OnToolsPlaceentity)
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateTerrain message handlers

BOOL CTemplateTerrain::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetupTemplateDialogIcons( this ) ;

	PositionDialogUnderTabs( this ) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTemplateTerrain::OnToolsPlacecube() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_BOX ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_BOXBRUSH ) ;
	}	
}

void CTemplateTerrain::OnToolsPlaceentity() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_ENTITY ) ;
	}	
}

void CTemplateTerrain::OnToolsPlacelight() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_LIGHT, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}	
}

void CTemplateTerrain::OnToolsPlacespheroid() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SPHERE ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SPHEREBRUSH ) ;
	}
	
}

void CTemplateTerrain::OnToolsPlaceterrain() 
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	UpdateData( true );
	pDoc->SetTemplateVisable( m_Terrain );
	UpdateData( false );
}

BOOL CTemplateTerrain::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if( wParam == 1 )
		return( true );
	
	return CDialog::OnCommand(wParam, lParam);
}

void CTemplateTerrain::OnToolsPlacecylinder() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_CYLINDER ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_CYLNDBRUSH ) ;
	}
}

void CTemplateTerrain::OnAdd() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Addbrush();
}

void CTemplateTerrain::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	UpdateData( true );
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = TRUE;
	UpdateData( false );
	
}

void CTemplateTerrain::OnToolsPlacesheet() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SHEET ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SHEETBRUSH ) ;
	}
	
}
