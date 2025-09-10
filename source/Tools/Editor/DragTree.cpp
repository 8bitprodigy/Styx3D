/****************************************************************************************/
/*  DRAGTREE.CPP                                                                        */
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
#include "DragTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDragTree

CDragTree::CDragTree()
{
}

CDragTree::~CDragTree()
{
}


BEGIN_MESSAGE_MAP(CDragTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CDragTree)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDragTree message handlers

bool CDragTree::Init(UINT nIDImageList)
{
	m_ImageList.Create( nIDImageList, 16, 1, RGB(255,0,255) ) ;
	SetImageList( &m_ImageList, TVSIL_NORMAL ) ;
	SetImageList( &m_ImageList, TVSIL_STATE ) ;

	return true ;
}// Init

