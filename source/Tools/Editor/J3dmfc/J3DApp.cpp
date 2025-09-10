/****************************************************************************************/
/*  J3DAPP.CPP                                                                          */
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

#include "J3DApp.h"
#include "J3DView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJ3DApp::CJ3DApp()
{
	m_pJ3DDocTemplate = nullptr;
}

CJ3DApp::~CJ3DApp()
{
	if(m_pJ3DDocTemplate != nullptr)
		delete m_pJ3DDocTemplate;
}
/*
BOOL CJ3DApp::CreateDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass)
{
	ASSERT(m_pJ3DDocTemplate == NULL);

	m_pJ3DDocTemplate = new CMultiDocTemplate(
		nIDResource,
		pDocClass,
		RUNTIME_CLASS(CChildFrame),
		RUNTIME_CLASS(CJ3DView));

	if(m_pJ3DDocTemplate == NULL)
		return(FALSE);

	return(TRUE);
}
*/
CMultiDocTemplate* CJ3DApp::GetJ3DViewDocTemplate(void)
{
//#pragma message("Wrong way to store mode")
//	if(strcmp(m_strDriverMode, "WindowMode") != 0)
//		return(m_pRenderFullScreenDocTemplate);

	ASSERT(m_pJ3DDocTemplate != NULL);

	return(m_pJ3DDocTemplate);
}
