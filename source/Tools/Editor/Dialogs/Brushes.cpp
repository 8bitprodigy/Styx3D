/****************************************************************************************/
/*  BRUSHES.CPP                                                                         */
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
#include "MfcUtil.h"
#include "MainFrm.h"

#include "Brushes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CBrushes dialog


CBrushes::CBrushes(CWnd* pParent /*=NULL*/)
	: CDialog(CBrushes::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBrushes)
	m_Draw = -1;
	//}}AFX_DATA_INIT
}


void CBrushes::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBrushes)
	DDX_Control(pDX, IDC_CK_LOCKTEXTURE, m_LockTextures);
	DDX_Control(pDX, BRSH_CK_FLOCKING, m_Flocking);
	DDX_Control(pDX, BRSH_CB_TYPE, m_Type);
	DDX_Radio(pDX, IDC_RADIO_CUT, m_Draw);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBrushes, CDialog)
	//{{AFX_MSG_MAP(CBrushes)
	ON_BN_CLICKED(IDM_TOOLS_PLACETERRAIN, OnToolsPlaceterrain)
	ON_BN_CLICKED(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_BN_CLICKED(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_BN_CLICKED(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(BRSH_CK_FLOCKING, OnCkFlocking)
	ON_BN_CLICKED(IDC_RADIO_CUT, OnRadioCut)
	ON_BN_CLICKED(IDC_RADIO_EMPTY, OnRadioEmpty)
	ON_BN_CLICKED(IDC_RADIO_SOLID, OnRadioSolid)
	ON_BN_CLICKED(IDC_CK_LOCKTEXTURE, OnCkLocktexture)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrushes message handlers

BOOL CBrushes::OnInitDialog() 
{
	int		nID ;
	CString	cstr ;
	CDialog::OnInitDialog();
	
	SetupTemplateDialogIcons( this) ;
	PositionDialogUnderTabs( this ) ;

	m_Type.ResetContent() ;
	for( nID = IDS_BRUSH_SOLID; nID < IDS_BRUSH_RESERVED1; nID++ )
	{
		cstr.LoadString( nID ) ;
		m_Type.AddString( cstr ) ;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}// OnInitDialog

void CBrushes::OnToolsPlaceterrain() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_TERRAIN, 0 ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_TERRAIN ) ;
	}	
}

void CBrushes::OnToolsPlacestaircase() 
{
	// TODO: Add your control notification handler code here
	
}

void CBrushes::OnToolsPlacespheroid() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_SPHERE ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_SPHEREBRUSH ) ;
	}
	
}

void CBrushes::OnToolsPlacesheet() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}
	
}

void CBrushes::OnToolsPlacelight() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_LIGHT ) ;
	}
	
}

void CBrushes::OnToolsPlaceentity() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		// DO NOT call SetTemplateMode
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_ENTITY ) ;
	}
}

void CBrushes::OnToolsPlacecylinder() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_CYLINDER ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_CYLNDBRUSH ) ;
	}
	
}

void CBrushes::OnToolsPlacecube() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	if( pDoc != NULL ) 
	{
		pDoc->SetTemplateMode( KIND_BRUSH, BRUSH_BOX ) ;
		pMainFrm->SetTemplateSubtype( MAINFRM_TEMPLATE_BOXBRUSH ) ;
	}
	
}

void CBrushes::OnToolsPlacecone() 
{
	// TODO: Add your control notification handler code here
	
}

void CBrushes::OnToolsPlacearch() 
{
	// TODO: Add your control notification handler code here
	
}

void CBrushes::OnShowWindow(BOOL bShow, UINT nStatus) 
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

void CBrushes::Update( CJweDoc *pDoc )
{
	uint32		Contents ;
	int32		BlankFieldFlag ;
	ASSERT( pDoc->IsKindOf( RUNTIME_CLASS(CJweDoc)) ) ;

	pDoc->GetBrushInfo( &Contents, &BlankFieldFlag );

	FillFields( Contents, BlankFieldFlag );
}

void CBrushes::FillFields( uint32 Contents, int32 BlankFieldFlag )
{
	UpdateData( true );
	if( BlankFieldFlag & BRUSH_FIELD_FLOCK ) 
		m_Flocking.SetCheck( 2 ) ;
	else
		m_Flocking.SetCheck( (Contents & JE_BSP_CONTENTS_FLOCK) ? true : false ) ;

	if( BlankFieldFlag & BRUSH_FIELD_TEXT_LOCK ) 
		m_LockTextures.SetCheck( 2 ) ;
	else
		m_LockTextures.SetCheck( (Contents & BRUSH_FIELD_TEXT_LOCK) ? true : false ) ;

	
	if( BlankFieldFlag & BRUSH_FIELD_DRAW ) 
		m_Draw = -1;
	else
	if( Contents & JE_BSP_CONTENTS_AIR) 
		m_Draw = 0;
	else
	if( Contents & JE_BSP_CONTENTS_EMPTY) 
		m_Draw = 1;
	else
		m_Draw = 2;
	UpdateData( false );

}


void CBrushes::OnCkFlocking() 
{
	int32 Contents;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	if( m_Flocking.GetCheck( ) )
		Contents = JE_BSP_CONTENTS_FLOCK ;
	else
		Contents = 0 ;

	pDoc->SetBrushInfo( Contents, BRUSH_FIELD_FLOCK ) ;
	
}

void CBrushes::SetDrawMode()
{
	int32 Contents;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	UpdateData( true );
	switch( m_Draw )
	{
	case 0:
		Contents = JE_BSP_CONTENTS_AIR;
		break;

	case 1:
		Contents = JE_BSP_CONTENTS_EMPTY;
		break;

	case 2:
		Contents = JE_BSP_CONTENTS_SOLID;
		break;
	}
	UpdateData( false );
	pDoc->SetBrushInfo( Contents, BRUSH_FIELD_DRAW ) ;
}

void CBrushes::OnRadioCut() 
{
	SetDrawMode();
	
}

void CBrushes::OnRadioEmpty() 
{
	SetDrawMode();
	
}

void CBrushes::OnRadioSolid() 
{
	SetDrawMode();
	
}

void CBrushes::OnCkLocktexture() 
{
	int32 Contents;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	if( m_LockTextures.GetCheck( ) )
		Contents = BRUSH_FIELD_TEXT_LOCK ;
	else
		Contents = 0 ;

	pDoc->SetBrushInfo( Contents, BRUSH_FIELD_TEXT_LOCK ) ;
	
}
