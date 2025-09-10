/****************************************************************************************/
/*  TEMPLATESPHERE.CPP                                                                  */
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
#include "TemplateSphere.h"
#include "MainFrm.h"
#include "MfcUtil.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_RADIUS		(32.0f)
#define DEFAULT_HBAND		(4)
#define DEFAULT_VBAND		(8)

/////////////////////////////////////////////////////////////////////////////
// TemplateSphere dialog


TemplateSphere::TemplateSphere(CWnd* pParent /*=NULL*/)
	: CDialog(TemplateSphere::IDD, pParent)
{
	//{{AFX_DATA_INIT(TemplateSphere)
	m_Radius = 0.0f;
	m_HBand = 0;
	m_VBand = 0;
	m_Name = _T("");
	m_ShowTemplate = FALSE;
	m_Cube = FALSE;
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	//}}AFX_DATA_INIT
}


void TemplateSphere::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TemplateSphere)
	DDX_Text(pDX, SPHTMP_ED_RADIUS, m_Radius);
	DDX_Text(pDX, SPHTMP_ED_HBAND, m_HBand);
	DDX_Text(pDX, SPHTMP_ED_VBAND, m_VBand);
	DDX_Text(pDX, SPHTMP_ED_NAME, m_Name);
	DDX_Check(pDX, IDM_TOOLS_PLACESPHEROID, m_ShowTemplate);
	DDX_Check(pDX, IDM_TOOLS_PLACECUBE, m_Cube);
	DDX_Check(pDX, IDM_TOOLS_PLACEARCH, m_Arch);
	DDX_Check(pDX, IDM_TOOLS_PLACECONE, m_Cone);
	DDX_Check(pDX, IDM_TOOLS_PLACECYLINDER, m_Cylinder);
	DDX_Check(pDX, IDM_TOOLS_PLACEENTITY, m_Entity);
	DDX_Check(pDX, IDM_TOOLS_PLACELIGHT, m_Light);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TemplateSphere, CDialog)
	//{{AFX_MSG_MAP(TemplateSphere)
	ON_EN_KILLFOCUS(SPHTMP_ED_RADIUS, OnKillfocusEdRadius)
	ON_BN_CLICKED(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_BN_CLICKED(IDM_TOOLS_PLACEENTITY, OnToolsPlaceentity)
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_EN_KILLFOCUS(SPHTMP_ED_NAME, OnKillfocusEdName)
	ON_EN_KILLFOCUS(SPHTMP_ED_VBAND, OnKillfocusEdVband)
	ON_EN_KILLFOCUS(SPHTMP_ED_HBAND, OnKillfocusEdHband)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_SUBTRACT, OnSubtract)
	ON_BN_CLICKED(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TemplateSphere message handlers

BOOL TemplateSphere::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetupTemplateDialogIcons( this ) ;

	PositionDialogUnderTabs( this ) ;
	
	m_Name.LoadString( IDS_DEFAULTSPHERETEMPLATENAME ) ;
	m_Radius = DEFAULT_RADIUS ;
	m_HBand = DEFAULT_HBAND ;
	m_VBand = DEFAULT_VBAND ;

	// Restrict edit control to alpha--from MSDN::CTRLTEST
	//m_edit1.SubclassEdit( SPHTMP_ED_NAME, this, PES_LETTERS|PES_SPACE ) ;

	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TemplateSphere::OnKillfocusEdRadius() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
	
}


void TemplateSphere::OnToolsPlacecube() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_BOX ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_BOXBRUSH ) ;
	}
}

void TemplateSphere::OnToolsPlaceentity() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_ENTITY ) ;
	}	
}

void TemplateSphere::OnToolsPlacelight() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_LIGHT, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}
}

void TemplateSphere::OnToolsPlaceterrain() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_TERRAIN, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_TERRAIN ) ;
	}	
}

void TemplateSphere::UpdateTemplate()
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc != NULL )
	{
		pDoc->SetSphereInfo( m_HBand, m_VBand, m_Radius );
	}
}// UpdateTemplate

void TemplateSphere::OnKillfocusEdName() 
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

void TemplateSphere::OnKillfocusEdVband() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
}

void TemplateSphere::OnKillfocusEdHband() 
{
	UpdateData( true ) ;
	UpdateTemplate();
	UpdateData( false ) ;
}

void TemplateSphere::OnToolsPlacespheroid() 
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	UpdateData( true );
	pDoc->SetTemplateVisable( m_ShowTemplate );
	UpdateData( false );

}

BOOL TemplateSphere::OnCommand(WPARAM wParam, LPARAM lParam) 
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

void TemplateSphere::OnToolsPlacecylinder() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_CYLINDER ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_CYLNDBRUSH ) ;
	}
}

void TemplateSphere::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	UpdateData( true );
	m_ShowTemplate = TRUE;
	m_Cube = FALSE;
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	UpdateData( false );
	
}

void TemplateSphere::OnAdd() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Addbrush();
	
}

void TemplateSphere::OnReset() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->ResetTemplate();
	}	
	
}

void TemplateSphere::OnSubtract() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Subtractbrush();	
}

void TemplateSphere::OnToolsPlacesheet() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SHEET ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SHEETBRUSH ) ;
	}
	
}
