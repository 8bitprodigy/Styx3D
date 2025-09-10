/****************************************************************************************/
/*  STATS.CPP                                                                           */
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
#include "stats.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStats dialog


CStats::CStats(CWnd* pParent /*=NULL*/)
	: CDialog(CStats::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(CStats)
	m_Areas = 0;
	m_Brushes = 0;
	m_Faces = 0;
	m_Leafs = 0;
	m_Nodes = 0;
	m_Portals = 0;
	m_Splits = 0;
	m_Subfaces = 0;
	m_TotalsFaces = 0;
	m_VisFaces = 0;
	m_VisPortals = 0;
	//}}AFX_DATA_INIT
}

void CStats::SetData( const jeBSP_DebugInfo * pDebugInfo )
{
	//UpdateData( true );
	m_Areas =		pDebugInfo->NumAreas;
	m_Brushes =		pDebugInfo->NumBrushes;
	m_Faces =		pDebugInfo->NumDrawFaces;
	m_Leafs =		pDebugInfo->NumLeafs;
	m_Nodes =		pDebugInfo->NumNodes;
	m_Portals =		pDebugInfo->NumPortals;
	m_Splits =		pDebugInfo->NumSplits;
	m_Subfaces =	pDebugInfo->NumSubdividedDrawFaces;
	m_TotalsFaces = pDebugInfo->NumTotalBrushFaces;
	m_VisFaces =	pDebugInfo->NumVisibleBrushFaces;
	m_VisPortals =	pDebugInfo->NumVisPortals;
	//UpdateData( false );
}
						
void CStats::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CDialog::OnFinalRelease();
}

void CStats::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStats)
	DDX_Text(pDX, IDC_AREAS, m_Areas);
	DDX_Text(pDX, IDC_BRUSHES, m_Brushes);
	DDX_Text(pDX, IDC_FACES, m_Faces);
	DDX_Text(pDX, IDC_LEAFS, m_Leafs);
	DDX_Text(pDX, IDC_NODES, m_Nodes);
	DDX_Text(pDX, IDC_PORTALS, m_Portals);
	DDX_Text(pDX, IDC_SPLITS, m_Splits);
	DDX_Text(pDX, IDC_SUBFACES, m_Subfaces);
	DDX_Text(pDX, IDC_TOTALFACES, m_TotalsFaces);
	DDX_Text(pDX, IDC_VISFACES, m_VisFaces);
	DDX_Text(pDX, IDC_VISPORTALS, m_VisPortals);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStats, CDialog)
	//{{AFX_MSG_MAP(CStats)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CStats, CDialog)
	//{{AFX_DISPATCH_MAP(CStats)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IStats to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {78722C64-0A31-11D3-B322-004033AA0441}
static const IID IID_IStats =
{ 0x78722c64, 0xa31, 0x11d3, { 0xb3, 0x22, 0x0, 0x40, 0x33, 0xaa, 0x4, 0x41 } };

BEGIN_INTERFACE_MAP(CStats, CDialog)
	INTERFACE_PART(CStats, IID_IStats, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStats message handlers
