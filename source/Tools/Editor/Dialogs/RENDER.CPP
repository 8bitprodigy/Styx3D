/****************************************************************************************/
/*  RENDER.CPP                                                                          */
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
#include "render.h"
#include "MfcUtil.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRender dialog


CRender::CRender(CWnd* pParent /*=NULL*/)
	: CDialog(CRender::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRender)
	m_Areas = 0;
	m_Brushes = 0;
	m_Faces = 0;
	m_Leafs = 0;
	m_Nodes = 0;
	m_Splits = 0;
	m_Portals = 0;
	m_Subfaces = 0;
	m_TotalFaces = 0;
	m_VisFaces = 0;
	m_VisPortals = 0;
	m_LightUpdate = 3;
	m_BrushUpdate = 1;
	m_BrushLighting = FALSE;
	//}}AFX_DATA_INIT
}

void CRender::SetData( const jeBSP_DebugInfo * pDebugInfo )
{
	UpdateData( true );
	m_Areas =		pDebugInfo->NumAreas;
	m_Brushes =		pDebugInfo->NumBrushes;
	m_Faces =		pDebugInfo->NumDrawFaces;
	m_Leafs =		pDebugInfo->NumLeafs;
	m_Nodes =		pDebugInfo->NumNodes;
	m_Portals =		pDebugInfo->NumPortals;
	m_Splits =		pDebugInfo->NumSplits;
	m_Subfaces =	pDebugInfo->NumSubdividedDrawFaces;
	m_TotalFaces = pDebugInfo->NumTotalBrushFaces;
	m_VisFaces =	pDebugInfo->NumVisibleBrushFaces;
	m_VisPortals =	pDebugInfo->NumVisPortals;
	UpdateData( false );
}

void CRender::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRender)
	DDX_Text(pDX, IDC_AREAS, m_Areas);
	DDX_Text(pDX, IDC_BRUSHES, m_Brushes);
	DDX_Text(pDX, IDC_FACES, m_Faces);
	DDX_Text(pDX, IDC_LEAFS, m_Leafs);
	DDX_Text(pDX, IDC_NODES, m_Nodes);
	DDX_Text(pDX, IDC_SPLITS, m_Splits);
	DDX_Text(pDX, IDC_PORTALS, m_Portals);
	DDX_Text(pDX, IDC_SUBFACES, m_Subfaces);
	DDX_Text(pDX, IDC_TOTALFACES, m_TotalFaces);
	DDX_Text(pDX, IDC_VISFACES, m_VisFaces);
	DDX_Text(pDX, IDC_VISPORTALS, m_VisPortals);
	DDX_Radio(pDX, IDC_LIGHT_ONUPDATE, m_LightUpdate);
	DDX_Radio(pDX, IDC_BRUSH_ONUPDATE, m_BrushUpdate);
	DDX_Check(pDX, IDC_BRUSH_LIGHTING, m_BrushLighting);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRender, CDialog)
	//{{AFX_MSG_MAP(CRender)
	ON_BN_CLICKED(IDC_BRUSH_LIGHTING, OnBrushLighting)
	ON_BN_CLICKED(IDC_BRUSH_ONCHANGE, OnBrushOnchange)
	ON_BN_CLICKED(IDC_BRUSH_ONDESELECT, OnBrushOndeselect)
	ON_BN_CLICKED(IDC_BRUSH_ONUPDATE, OnBrushOnupdate)
	ON_BN_CLICKED(IDC_LIGHT_ONCHANGE, OnLightOnchange)
	ON_BN_CLICKED(IDC_LIGHT_ONDESELECT, OnLightOndeselect)
	ON_BN_CLICKED(IDC_LIGHT_ONUPDATE, OnLightOnupdate)
	ON_BN_CLICKED(IDC_LIGHT_REALTIME, OnLightRealtime)
	ON_BN_CLICKED(IDC_UPDATE_SEL, OnUpdateSel)
	ON_BN_CLICKED(IDC_UPDATEALL, OnUpdateall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRender message handlers

BOOL CRender::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	PositionDialogUnderTabs( this ) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRender::SetCurrentDocument( CJweDoc *pDoc )
{
	UpdateData( true );
	m_BrushUpdate = pDoc->GetBrushUpdate();
	m_LightUpdate = pDoc->GetLightUpdate();
	m_BrushLighting = pDoc->GetBrushLighting();
	UpdateData( false );
}

void CRender::UpdateLevelData()
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;

	UpdateData( true );
	pDoc->SetBrushUpdate( m_BrushUpdate );
	pDoc->SetLightUpdate( m_LightUpdate );
	pDoc->SetBrushLighting(m_BrushLighting);
	UpdateData( false );
}

void CRender::OnBrushLighting() 
{
	UpdateLevelData();
	
}

void CRender::OnBrushOnchange() 
{
	UpdateLevelData();
	
}

void CRender::OnBrushOndeselect() 
{
	UpdateLevelData();
	
}

void CRender::OnBrushOnupdate() 
{
	UpdateLevelData();
	
}

void CRender::OnLightOnchange() 
{
	UpdateLevelData();
	
}

void CRender::OnLightOndeselect() 
{
	UpdateLevelData();
	
}

void CRender::OnLightOnupdate() 
{
	UpdateLevelData();
	
}

void CRender::OnLightRealtime() 
{
	UpdateLevelData();
	
}

void CRender::OnUpdateSel() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;

	pDoc->UpdateSelection();
	
}

void CRender::OnUpdateall() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;

	pDoc->UpdateAll();
	
}

BOOL CRender::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if( wParam == 1 && lParam == 0 )
		return(JE_TRUE );
	
	return CDialog::OnCommand(wParam, lParam);
}
