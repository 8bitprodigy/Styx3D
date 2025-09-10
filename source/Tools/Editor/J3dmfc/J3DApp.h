/****************************************************************************************/
/*  J3DAPP.H                                                                            */
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

#if !defined(AFX_J3DAPP_H__84F34CE6_B116_11D2_BE7D_00A0C96E625A__INCLUDED_)
#define AFX_J3DAPP_H__84F34CE6_B116_11D2_BE7D_00A0C96E625A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Jet.h"

class CJ3DApp : public CWinApp  
{
public:
	virtual BOOL GetDriverAndMode(jeEngine* pEngine, jeDriver **ppDriver, jeDriver_Mode **ppMode) = 0;
	virtual const char* GetDriverPath(void) = 0;
	CJ3DApp();
	virtual ~CJ3DApp();
	CMultiDocTemplate * GetJ3DViewDocTemplate(void);
	// Should call this in the derived class's InitInstance!
	//BOOL CreateDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJ3DApp)
	public:
	//}}AFX_VIRTUAL

protected:
	CMultiDocTemplate * m_pJ3DDocTemplate;
};

#endif // !defined(AFX_J3DAPP_H__84F34CE6_B116_11D2_BE7D_00A0C96E625A__INCLUDED_)
