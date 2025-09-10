/****************************************************************************************/
/*  MFCUTIL.H                                                                           */
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
#pragma once
#ifndef MFCUTIL_H
#define MFCUTIL_H

#include "Jet.h"

void		PositionDialogUnderTabs( CDialog * pDlg ) ;
BOOL		LoadBMPImage( LPCTSTR sBMPFile, CBitmap& bitmap, CPalette *pPal ) ;
void		TrimStringByPixelCount( HDC hDC, char * psz, const int nMaxPixels ) ;
void		TrimString( CString &strString ) ;
void		SetupTemplateDialogIcons( CDialog * pDlg ) ;

// TREE VIEW
HTREEITEM	TreeViewIsInBranch( CTreeCtrl *pTV, HTREEITEM hItem, const char * psz) ;

// BMP Stuff
BOOL WriteWindowToDIB( jeVFile	*	pF, jePtrMgr	*pPtrMgr , CWnd *pWnd );

#endif // Prevent multiple inclusion
/* EOF: MfcUtil.h */