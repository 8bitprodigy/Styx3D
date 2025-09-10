/****************************************************************************************/
/*  REBUILD.CPP                                                                         */
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
#include "rebuild.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRebuild dialog


CRebuild::CRebuild(CWnd* pParent /*=NULL*/)
	: CDialog(CRebuild::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRebuild)
	m_CSG = TRUE;
	m_Solid = FALSE;
	m_Vis = FALSE;
	m_Logic = 1;
	//}}AFX_DATA_INIT
	m_Balance = 2;
}


void CRebuild::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRebuild)
	DDX_Control(pDX, IDC_BALANCE, ctrl_Balance);
	DDX_Check(pDX, IDC_CSG, m_CSG);
	DDX_Check(pDX, IDC_SOLID, m_Solid);
	DDX_Check(pDX, IDC_VIS, m_Vis);
	DDX_Radio(pDX, IDC_LAZY, m_Logic);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRebuild, CDialog)
	//{{AFX_MSG_MAP(CRebuild)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BALANCE, OnReleasedcaptureBalance)
	ON_BN_CLICKED(IDC_LIGHTSONLY, OnLightsonly)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_REBUILDBSP, OnRebuildbsp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRebuild message handlers
int	  CRebuild::GetBalance()
{
	return( m_Balance );
}

BOOL  CRebuild::GetCSG()
{
	return( m_CSG );
}

BOOL  CRebuild::GetSolid()
{
	return( m_Solid );
}

BOOL  CRebuild::GetVis()
{
	return( m_Vis );
}

jeBSP_Logic	  CRebuild::GetLogic()
{
	switch( m_Logic )
	{
	case 0:
		return( Logic_Lazy );
	case 1:
		return( Logic_Normal );
	case 2:
		return( Logic_Smart );
	case 3:
		return( Logic_Super );
	}
	return( Logic_Lazy );

}


void CRebuild::OnReleasedcaptureBalance(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	pNMHDR;
	m_Balance = ctrl_Balance.GetPos();
	*pResult = 0;
}

BOOL CRebuild::OnInitDialog() 
{	


	CDialog::OnInitDialog();
	
	CMainFrame			*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	
	ctrl_Balance.SetRange( 0, 10, FALSE );
	ctrl_Balance.SetPos( m_Balance );

	GetDlgItem(IDC_STATIC_REBUILD_TEXT)->SetFont( &pMainFrm->cBigFont, TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRebuild::OnOK() 
{
	EndDialog( IDOK );
	
	CDialog::OnOK();
}

void CRebuild::OnCancel() 
{
	EndDialog( IDCANCEL );
	
}

void CRebuild::OnLightsonly() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;

	UpdateData( true );
	UpdateData( false );
	pDoc->RebuildLights(  );
	EndDialog( IDCANCEL );
}

void CRebuild::OnRebuildbsp() 
{
	EndDialog( IDBSP );		
}
