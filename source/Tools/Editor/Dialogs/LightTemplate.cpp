/****************************************************************************************/
/*  LIGHTTEMPLATE.CPP                                                                   */
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
#include "Light.h"
#include "MainFrm.h"
#include "MfcUtil.h"

#include "LightTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_RADIUS		(100.0f)
#define DEFAULT_BRIGHTNESS	(5.0f)

/////////////////////////////////////////////////////////////////////////////
// CLightTemplate dialog


CLightTemplate::CLightTemplate(CWnd* pParent /*=NULL*/)
	: CDialog(CLightTemplate::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLightTemplate)
	m_csName = _T("");
	m_fRadius = 0.0f;
	m_fIntensity = 0.0f;
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


void CLightTemplate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightTemplate)
	DDX_Control(pDX, LTMP_BN_COLOR, m_bnColor);
	DDX_Text(pDX, LTMP_ED_NAME, m_csName);
	DDV_MaxChars(pDX, m_csName, 31);
	DDX_Text(pDX, LTMP_ED_RADIUS, m_fRadius);
	DDV_MinMaxFloat(pDX, m_fRadius, 0.f, 200.f);
	DDX_Text(pDX, LTMP_ED_INTENSITY, m_fIntensity);
	DDV_MinMaxFloat(pDX, m_fIntensity, 0.f, 10.f);
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


BEGIN_MESSAGE_MAP(CLightTemplate, CDialog)
	//{{AFX_MSG_MAP(CLightTemplate)
	ON_BN_CLICKED(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_BN_CLICKED(IDM_TOOLS_PLACEENTITY, OnToolsPlaceentity)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_EN_KILLFOCUS(LTMP_ED_INTENSITY, OnKillfocusEdIntensity)
	ON_EN_KILLFOCUS(LTMP_ED_NAME, OnKillfocusEdName)
	ON_EN_KILLFOCUS(LTMP_ED_RADIUS, OnKillfocusEdRadius)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_UPDATE, Update)
	ON_MESSAGE(WM_UPDATECOLOR, OnChangeColor)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightTemplate message handlers

BOOL CLightTemplate::OnInitDialog() 
{
	SetupTemplateDialogIcons( this ) ;

	PositionDialogUnderTabs( this ) ;
	
	m_csName.LoadString( IDS_DEFAULTLIGHTTEMPLATENAME ) ;
	m_fRadius = DEFAULT_RADIUS ;
	m_fIntensity = DEFAULT_BRIGHTNESS ;

	// Restrict edit control to alpha--from MSDN::CTRLTEST
	m_edit1.SubclassEdit( LTMP_ED_NAME, this, PES_LETTERS|PES_SPACE ) ;

	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}// OnInitDialog

void CLightTemplate::OnToolsPlacecube() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_BOX ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_BOXBRUSH ) ;
	}
}// OnToolsPlacecube

void CLightTemplate::OnToolsPlaceentity() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_ENTITY ) ;
	}
}// OnToolsPlaceentity

void CLightTemplate::OnToolsPlaceterrain() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_TERRAIN, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_TERRAIN ) ;
	}	
}// OnToolsPlaceterrain

LRESULT CLightTemplate::Update( WPARAM wParam, LPARAM lParam )
{
	LightInfo		LightInfo ;
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	
	if( pDoc != NULL )
	{
		pDoc->SetTemplateMode( KIND_LIGHT, 0 ) ;
	
		GetFields( &LightInfo ) ;
		// DON'T set the POS
		UpdateTemplate( &LightInfo, (LIGHT_FIELD_BRIGHTNESS|LIGHT_FIELD_COLOR|LIGHT_FIELD_RADIUS) ) ;
	}
	wParam;
	lParam;

	return 0 ;
}// Update

void CLightTemplate::OnKillfocusEdIntensity() 
{
	LightInfo		LightInfo ;
	GetFields( &LightInfo ) ;
	UpdateTemplate( &LightInfo, LIGHT_FIELD_BRIGHTNESS ) ;

}// OnKillfocusEdIntensity

void CLightTemplate::OnKillfocusEdName() 
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
}// OnKillfocusEdName

void CLightTemplate::OnKillfocusEdRadius() 
{
	LightInfo		LightInfo ;

	GetFields( &LightInfo ) ;
	UpdateTemplate( &LightInfo, LIGHT_FIELD_RADIUS ) ;
}// OnKillfocusEdRadius

void CLightTemplate::GetFields(LightInfo *pLightInfo)
{
	COLORREF	Color ;
	ASSERT( pLightInfo != NULL ) ;
	UpdateData( true ) ;

	TrimString( m_csName ) ;
	if( m_csName.IsEmpty() )
		m_csName.LoadString( IDS_DEFAULTLIGHTTEMPLATENAME ) ;

	pLightInfo->Brightness = m_fIntensity  ;
	pLightInfo->Radius = m_fRadius ;
	Color = m_bnColor.GetColor( ) ;
	pLightInfo->Color.X = (jeFloat)GetRValue( Color ) ;
	pLightInfo->Color.Y = (jeFloat)GetGValue( Color ) ;
	pLightInfo->Color.Z = (jeFloat)GetBValue( Color ) ;

	UpdateData( false ) ;
}// GetFields

void CLightTemplate::UpdateTemplate(LightInfo *pLightInfo, int32 BlankFieldFlag )
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc != NULL )
	{
		Light_SetInfo( (Light*)pDoc->GetTemplate(), pLightInfo, BlankFieldFlag ) ;
	}
}// UpdateTemplate

LRESULT CLightTemplate::OnChangeColor( WPARAM wParam, LPARAM lParam )
{
	COLORREF	color  = (COLORREF)lParam ;
	LightInfo	sLightInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc != NULL )
	{
		sLightInfo.Color.X = (jeFloat)GetRValue( color ) ;
		sLightInfo.Color.Y = (jeFloat)GetGValue( color ) ;
		sLightInfo.Color.Z = (jeFloat)GetBValue( color ) ;

		Light_SetInfo( (Light*)pDoc->GetTemplate(), &sLightInfo, LIGHT_FIELD_COLOR ) ;
	}
	return 0 ;
	wParam;
}// OnChangeColor

void CLightTemplate::OnToolsPlacespheroid() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SPHERE ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SPHEREBRUSH ) ;
	}
	
}

void CLightTemplate::OnToolsPlacelight() 
{
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();

	UpdateData( true );
	pDoc->SetTemplateVisable( m_Light );
	UpdateData( false );
}

BOOL CLightTemplate::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( wParam == 1 )
	{
		pDoc->Addbrush();
		return( false );
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CLightTemplate::OnToolsPlacecylinder() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_CYLINDER ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_CYLNDBRUSH ) ;
	}
	
}

void CLightTemplate::OnAdd() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	pDoc->Addbrush();
	
}

void CLightTemplate::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	UpdateData( true );
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = TRUE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	UpdateData( false );
	
}

void CLightTemplate::OnToolsPlacesheet() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SHEET ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SHEETBRUSH ) ;
	}
	
}
