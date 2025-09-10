/****************************************************************************************/
/*  LIGHTS.CPP                                                                          */
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

#include "Doc.h"
#include "jwe.h"
#include "MainFrm.h"
#include "MfcUtil.h"

#include "Lights.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLights dialog


CLights::CLights(CWnd* pParent /*=NULL*/)
	: CDialog(CLights::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLights)
	m_csfBrightness = _T("");
	m_csfRadius = _T("");
	m_csfX = _T("");
	m_csfY = _T("");
	m_csfZ = _T("");
	m_csName = _T("");
	m_csNumber = _T("");
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	m_Sheet = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	//}}AFX_DATA_INIT
}


void CLights::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLights)
	DDX_Control(pDX, LGHT_BN_COLOR, m_bnColor);
	DDX_Text(pDX, LGHT_ED_BRIGHTNESS, m_csfBrightness);
	DDV_MaxChars(pDX, m_csfBrightness, 10);
	DDX_Text(pDX, LGHT_ED_RADIUS, m_csfRadius);
	DDV_MaxChars(pDX, m_csfRadius, 10);
	DDX_Text(pDX, LGHT_ED_X, m_csfX);
	DDV_MaxChars(pDX, m_csfX, 10);
	DDX_Text(pDX, LGHT_ED_Y, m_csfY);
	DDV_MaxChars(pDX, m_csfY, 10);
	DDX_Text(pDX, LGHT_ED_Z, m_csfZ);
	DDV_MaxChars(pDX, m_csfZ, 10);
	DDX_Text(pDX, LGHT_ED_NAME, m_csName);
	DDV_MaxChars(pDX, m_csName, 31);
	DDX_Text(pDX, LGHT_ED_NUMBER, m_csNumber);
	DDV_MaxChars(pDX, m_csNumber, 10);
	DDX_Check(pDX, IDM_TOOLS_PLACEARCH, m_Arch);
	DDX_Check(pDX, IDM_TOOLS_PLACECONE, m_Cone);
	DDX_Check(pDX, IDM_TOOLS_PLACECUBE, m_Cube);
	DDX_Check(pDX, IDM_TOOLS_PLACECYLINDER, m_Cylinder);
	DDX_Check(pDX, IDM_TOOLS_PLACEENTITY, m_Entity);
	DDX_Check(pDX, IDM_TOOLS_PLACELIGHT, m_Light);
	DDX_Check(pDX, IDM_TOOLS_PLACESHEET, m_Sheet);
	DDX_Check(pDX, IDM_TOOLS_PLACESPHEROID, m_Sphere);
	DDX_Check(pDX, IDM_TOOLS_PLACESTAIRCASE, m_Stair);
	DDX_Check(pDX, IDM_TOOLS_PLACETERRAIN, m_Terrain);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLights, CDialog)
	//{{AFX_MSG_MAP(CLights)
	ON_EN_KILLFOCUS(LGHT_ED_BRIGHTNESS, OnKillfocusEdBrightness)
	ON_EN_KILLFOCUS(LGHT_ED_RADIUS, OnKillfocusEdRadius)
	ON_EN_KILLFOCUS(LGHT_ED_X, OnKillfocusEdX)
	ON_EN_KILLFOCUS(LGHT_ED_Y, OnKillfocusEdY)
	ON_EN_KILLFOCUS(LGHT_ED_Z, OnKillfocusEdZ)
	ON_BN_CLICKED(LGHT_BN_COLOR, OnBnColor)
	ON_EN_KILLFOCUS(LGHT_ED_NAME, OnKillfocusEdName)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_BN_CLICKED(IDM_TOOLS_PLACESTAIRCASE, OnToolsPlacestaircase)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_BN_CLICKED(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_BN_CLICKED(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_BN_CLICKED(IDM_TOOLS_PLACECONE, OnToolsPlacecone)
	ON_BN_CLICKED(IDM_TOOLS_PLACEARCH, OnToolsPlacearch)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDM_TOOLS_PLACEENTITY, OnToolsPlaceentity)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_UPDATECOLOR, OnChangeColor)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLights message handlers

BOOL CLights::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetupTemplateDialogIcons( this) ;
	PositionDialogUnderTabs( this ) ;
	
	m_bnColor.SetColor( RGB( 255, 255, 255 ) ) ;
	// Restrict edit control to alpha--from MSDN::CTRLTEST
	m_edit1.SubclassEdit( LGHT_ED_NAME, this, PES_LETTERS|PES_SPACE ) ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}// OnInitDialog

// Selection has changed
void CLights::Update(CJweDoc *pDoc)
{
	LightInfo			LightInfo ;
	int32			BlankFieldFlag ;
	int32			nNumber ;
	const char *	pszName ;

	ASSERT( pDoc->IsKindOf( RUNTIME_CLASS(CJweDoc)) ) ;

	pDoc->GetLightInfo( &LightInfo, &BlankFieldFlag ) ;

	pszName = pDoc->GetSelectionName( &nNumber ) ;

	FillFields( &LightInfo, ~BlankFieldFlag, pszName, nNumber ) ;

}// Update

void CLights::FillFields(LightInfo *pLightInfo, int32 BlankFieldFlag, const char * pszName, int32 nNumber )
{
	COLORREF	Color ;
	UpdateData( true ) ;

	m_csfBrightness = _T("");
	m_csfRadius = _T("");
	m_csfX = _T("");
	m_csfY = _T("");
	m_csfZ = _T("");
	
	if( pszName == NULL )
	{
		m_csName = _T("") ;
		m_csNumber = _T("") ;
	}
	else
	{
		m_csName = pszName ;
		if( SELECT_INVALID_NNUMBER == nNumber )
			m_csNumber.LoadString( IDS_MULTSELNAME ) ;
		else
			m_csNumber.Format( "%d", nNumber ) ;
	}

	if( BlankFieldFlag & LIGHT_FIELD_BRIGHTNESS )
	{
		m_csfBrightness.Format( "%5.2f", pLightInfo->Brightness ) ;
		TrimString( m_csfBrightness ) ;
	}
	if( BlankFieldFlag & LIGHT_FIELD_RADIUS )
	{
		m_csfRadius.Format( "%5.2f", pLightInfo->Radius ) ;
		TrimString( m_csfRadius ) ;
	}
	if( BlankFieldFlag & LIGHT_FIELD_COLOR )
		Color = RGB( (int)pLightInfo->Color.X, (int)pLightInfo->Color.Y, (int)pLightInfo->Color.Z ) ;
	else
		Color = RGB( 255, 255, 255 ) ;
	m_bnColor.SetColor( Color ) ;

	if( BlankFieldFlag & LIGHT_FIELD_POS )
	{
		SetPosition( &pLightInfo->Pos ) ;		
	}
	UpdateData( false ) ;
}// FillFields

void CLights::OnKillfocusEdBrightness() 
{
	LightInfo LightInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc == NULL )
		return ;
	
	UpdateData( true ) ;
	LightInfo.Brightness = (jeFloat)atof( m_csfBrightness ) ;
	pDoc->SetLightInfo( &LightInfo, LIGHT_FIELD_BRIGHTNESS ) ;
	m_csfBrightness.Format( "%5.2f", LightInfo.Brightness ) ;
	TrimString( m_csfBrightness ) ;

	UpdateData( false ) ;
}// OnKillfocusEdBrightness

void CLights::OnKillfocusEdRadius() 
{
	LightInfo LightInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc == NULL )
		return ;

	UpdateData( true ) ;
	LightInfo.Radius = (jeFloat)atof( m_csfRadius ) ;
	pDoc->SetLightInfo( &LightInfo, LIGHT_FIELD_RADIUS ) ;
	m_csfRadius.Format( "%5.2f", LightInfo.Radius ) ;
	TrimString( m_csfRadius ) ;

	UpdateData( false ) ;
}// OnKillfocusEdRadius

void CLights::OnKillfocusEdX() 
{
	LightInfo LightInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc == NULL )
		return ;
	
	GetPosition( &LightInfo.Pos ) ;
	pDoc->SetLightInfo( &LightInfo, LIGHT_FIELD_POS ) ;
	SetPosition( &LightInfo.Pos ) ;

	UpdateData( false ) ;
}// OnKillfocusEdX

void CLights::OnKillfocusEdY() 
{
	OnKillfocusEdX() ;
}// OnKillfocusEdY

void CLights::OnKillfocusEdZ() 
{
	OnKillfocusEdX() ;
}// OnKillfocusEdZ

void CLights::OnBnColor() 
{
	// TODO: Add your control notification handler code here
	
}// OnBnColor

void CLights::GetPosition(jeVec3d *pPos)
{
	UpdateData( true ) ;

	pPos->X = (jeFloat)atof( m_csfX ) ;
	pPos->Y = (jeFloat)atof( m_csfY ) ;
	pPos->Z	= (jeFloat)atof( m_csfZ ) ;
}// GetPosition

void CLights::SetPosition(jeVec3d *pPos)
{
	m_csfX.Format( "%5.2f", pPos->X ) ;
	TrimString( m_csfX ) ;
	m_csfY.Format( "%5.2f", pPos->Y ) ;
	TrimString( m_csfY ) ;
	m_csfZ.Format( "%5.2f", pPos->Z ) ;
	TrimString( m_csfZ ) ;
}// SetPosition

LRESULT CLights::OnChangeColor( WPARAM wParam, LPARAM lParam )
{
	COLORREF	color  = (COLORREF)lParam ;
	LightInfo		LightInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc == NULL )
		return 0 ;

	LightInfo.Color.X = (jeFloat)GetRValue( color ) ;
	LightInfo.Color.Y = (jeFloat)GetGValue( color ) ;
	LightInfo.Color.Z = (jeFloat)GetBValue( color ) ;

	pDoc->SetLightInfo( &LightInfo, LIGHT_FIELD_COLOR ) ;

	return 0 ;
	wParam;
}// OnChangeColor

void CLights::OnKillfocusEdName() 
{
	int32		nNumber ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	if( pDoc == NULL )
		return ;
	
	UpdateData( true ) ;
	TrimString( m_csName ) ;
	if( m_csName.IsEmpty() )
		m_csName.LoadString( IDS_DEFAULTLIGHTTEMPLATENAME ) ;
	
	pDoc->SetSelectionName( m_csName ) ;

	// See if the number changed...
	pDoc->GetSelectionName( &nNumber ) ;
	if( SELECT_INVALID_NNUMBER == nNumber )
		m_csNumber.LoadString( IDS_MULTSELNAME ) ;
	else
		m_csNumber.Format( "%d", nNumber ) ;

	UpdateData( false ) ;	
}// OnKillfocusEdName

void CLights::OnToolsPlaceterrain() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_TERRAIN, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_TERRAIN ) ;
	}		
}

void CLights::OnToolsPlacestaircase() 
{
	// TODO: Add your control notification handler code here
	
}

void CLights::OnToolsPlacespheroid() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SPHERE ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SPHEREBRUSH ) ;
	}
	
}

void CLights::OnToolsPlacelight() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}
	
}

void CLights::OnToolsPlacesheet() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SHEET ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SHEETBRUSH ) ;
	}
	
}

void CLights::OnToolsPlacecylinder() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_CYLINDER ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_CYLNDBRUSH ) ;
	}
	
}

void CLights::OnToolsPlacecube() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_BOX ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_BOXBRUSH ) ;
	}
	
}

void CLights::OnToolsPlacecone() 
{
	// TODO: Add your control notification handler code here
	
}

void CLights::OnToolsPlacearch() 
{
	// TODO: Add your control notification handler code here
	
}

void CLights::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	m_Arch = FALSE;
	m_Cone = FALSE;
	m_Cube = FALSE;
	m_Cylinder = FALSE;
	m_Entity = FALSE;
	m_Light = FALSE;
	m_Sheet = FALSE;
	m_Sphere = FALSE;
	m_Stair = FALSE;
	m_Terrain = FALSE;
	
}

void CLights::OnToolsPlaceentity() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_ENTITY ) ;
	}	
}
