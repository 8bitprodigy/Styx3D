/****************************************************************************************/
/*  J3DMAINFRM.H                                                                        */
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

#if !defined(AFX_J3DMAINFRM_H__84F34CE5_B116_11D2_BE7D_00A0C96E625A__INCLUDED_)
#define AFX_J3DMAINFRM_H__84F34CE5_B116_11D2_BE7D_00A0C96E625A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CJ3DMainFrame : public CMDIFrameWnd  
{
	DECLARE_DYNAMIC(CJ3DMainFrame)
public:
	CJ3DMainFrame();
	virtual ~CJ3DMainFrame();

// Implementation
public:
	void OpenJ3DWindow();

	// Generated message map functions
private:
	//{{AFX_MSG(CJ3DMainFrame)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_J3DMAINFRM_H__84F34CE5_B116_11D2_BE7D_00A0C96E625A__INCLUDED_)
