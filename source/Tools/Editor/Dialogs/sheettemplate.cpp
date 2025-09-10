/****************************************************************************************/
/*  SHEETTEMPLATE.CPP                                                                   */
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
#include "sheettemplate.h"
#include "MainFrm.h"
#include "MfcUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSheetTemplate dialog


CSheetTemplate::CSheetTemplate(CWnd* pParent /*=NULL*/)
	: CDialog(CSheetTemplate::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSheetTemplate)
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entitiy = FALSE;
	m_Light = FALSE;
	m_Sheet = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	m_csName = _T("");
	//}}AFX_DATA_INIT
}


void CSheetTemplate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSheetTemplate)
	DDX_Check(pDX, IDM_TOOLS_PLACEARCH, m_Arch);
	DDX_Check(pDX, IDM_TOOLS_PLACECONE, m_Cone);
	DDX_Check(pDX, IDM_TOOLS_PLACECUBE, m_Cube);
	DDX_Check(pDX, IDM_TOOLS_PLACECYLINDER, m_Cylinder);
	DDX_Check(pDX, IDM_TOOLS_PLACEENTITY, m_Entitiy);
	DDX_Check(pDX, IDM_TOOLS_PLACELIGHT, m_Light);
	DDX_Check(pDX, IDM_TOOLS_PLACESHEET, m_Sheet);
	DDX_Check(pDX, IDM_TOOLS_PLACESPHEROID, m_Sphere);
	DDX_Check(pDX, IDM_TOOLS_PLACESTAIRCASE, m_Stair);
	DDX_Check(pDX, IDM_TOOLS_PLACETERRAIN, m_Terrain);
	DDX_Text(pDX, LTMP_ED_NAME, m_csName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSheetTemplate, CDialog)
	//{{AFX_MSG_MAP(CSheetTemplate)
	ON_EN_KILLFOCUS(LTMP_ED_NAME, OnKillfocusEdName)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_BN_CLICKED(IDM_TOOLS_PLACEENTITY, OnToolsPlaceentity)
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_BN_CLICKED(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSheetTemplate message handlers

BOOL CSheetTemplate::OnInitDialog() 
{	
	SetupTemplateDialogIcons( this ) ;

	PositionDialogUnderTabs( this ) ;
	
	m_csName.LoadString( IDS_DEFAULTSHEETTEMPLATENAME ) ;
	

	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSheetTemplate::OnKillfocusEdName() 
{
	UpdateData( true ) ;

	TrimString( m_csName ) ;
	if( m_csName.IsEmpty() )
		m_csName.LoadString( IDS_DEFAULTLIGHTTEMPLATENAME ) ;

	UpdateData( false ) ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc != NULL )
	{
		Object_SetName( pDoc->GetTemplate(), m_csName, SELECT_INVALID_NNUMBER ) ;
	}	
}

void CSheetTemplate::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	UpdateData( true );
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entitiy = FALSE;
	m_Light = FALSE;
	m_Sheet = TRUE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	UpdateData( false );
	
	// TODO: Add your message handler code here
	
}

void CSheetTemplate::OnAdd() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Addbrush();
	
}

void CSheetTemplate::OnToolsPlacecube() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_BOX ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_BOXBRUSH ) ;
	}
}

void CSheetTemplate::OnToolsPlacecylinder() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_CYLINDER ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_CYLNDBRUSH ) ;
	}
	
}

void CSheetTemplate::OnToolsPlaceentity() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_ENTITY ) ;
	}
}

void CSheetTemplate::OnToolsPlacelight() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_LIGHT, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}
}

void CSheetTemplate::OnToolsPlacesheet() 
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	UpdateData( true );
	pDoc->SetTemplateVisable( m_Sheet );
	UpdateData( false );
	
}

void CSheetTemplate::OnToolsPlacespheroid() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SPHERE ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SPHEREBRUSH ) ;
	}
}

void CSheetTemplate::OnToolsPlaceterrain() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_TERRAIN, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_TERRAIN ) ;
	}	
}

void CSheetTemplate::OnReset() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->ResetTemplate();
	}	
	
}
