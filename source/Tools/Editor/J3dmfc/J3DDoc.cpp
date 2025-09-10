/****************************************************************************************/
/*  J3DDOC.CPP                                                                          */
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
#include "J3DDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJ3DDoc

IMPLEMENT_DYNAMIC(CJ3DDoc, CDocument)

CJ3DDoc::CJ3DDoc()
{
	m_pWorld = NULL;
}

CJ3DDoc::~CJ3DDoc()
{
	// Make sure someone has cleaned up the world.
	ASSERT(m_pWorld == NULL);
}


BEGIN_MESSAGE_MAP(CJ3DDoc, CDocument)
	//{{AFX_MSG_MAP(CJ3DDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJ3DDoc diagnostics

#ifdef _DEBUG
void CJ3DDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CJ3DDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CJ3DDoc commands

