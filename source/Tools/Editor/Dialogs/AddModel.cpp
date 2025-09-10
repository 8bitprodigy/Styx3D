/****************************************************************************************/
/*  ADDMODEL.CPP                                                                        */
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
#include "AddModel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddModel dialog


CAddModel::CAddModel(CWnd* pParent /*=NULL*/)
	: CDialog(CAddModel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddModel)
	m_csName = _T("");
	//}}AFX_DATA_INIT
}


void CAddModel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddModel)
	DDX_Text(pDX, NMDL_ED_NAME, m_csName);
	DDV_MaxChars(pDX, m_csName, 31);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddModel, CDialog)
	//{{AFX_MSG_MAP(CAddModel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddModel message handlers

BOOL CAddModel::OnInitDialog() 
{
	CString cstr ;
	CDialog::OnInitDialog();
	
	cstr.LoadString( m_nTitleID ) ;
	SetWindowText( cstr ) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
