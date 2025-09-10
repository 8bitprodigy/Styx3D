/****************************************************************************************/
/*  WNDREG.H                                                                            */
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

#ifndef WINREG_H
#define WINREG_H

typedef struct tagWindowRegister WindowRegister ;
#include <Windowsx.h>
#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

WindowRegister *WndReg_Create();
void WndReg_Destroy( WindowRegister *hWndReg );

jeBoolean WndReg_RegisterWindow(  HWND pHwnd, int32 Signiture );
int32 WndReg_GetSigniture(  HWND pHwnd );
HWND  WndReg_GetWindow( int32 Signiture );

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: WndReg.h */