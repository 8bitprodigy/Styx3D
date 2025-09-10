/****************************************************************************************/
/*  D3DDrv.h                                                                            */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: D3D driver                                                             */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
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
#ifndef D3DDRV_H
#define D3DDRV_H
#include <DDraw.h>

#include "DCommon.h"

typedef HRESULT (WINAPI *LPDIRECTDRAWCREATE)( GUID FAR *lpGUID, LPDIRECTDRAW7 FAR *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter );
typedef HRESULT (WINAPI *LPDIRECTDRAWENUMERATE)(  LPDDENUMCALLBACKA lpCallback, LPVOID lpContext );

extern DRV_Window			D3DDrv_ClientWindow;
extern DRV_Driver			D3DDRV;

LPDIRECTDRAWCREATE		D3DDrv_DirectDrawCreate(void);
LPDIRECTDRAWENUMERATE	D3DDrv_DirectDrawEnumerate(void);

void DRIVERCC			ErrorBox(char *Str);
BOOL DRIVERCC			DrvShutdown(void);
void					SetLastDrvError(int32 Error, char *ErrorStr);

#endif
