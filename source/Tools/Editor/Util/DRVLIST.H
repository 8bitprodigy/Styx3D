/****************************************************************************************/
/*  DRVLIST.H                                                                           */
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
#ifndef	DRVLIST_H
#define DRVLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#define DRVLIST_WINDOW		( 1 << 0 )
#define DRVLIST_FULLSCREEN	( 1 << 1 )
#define DRVLIST_SOFTWARE	( 1 << 2 )
#define DRVLIST_HARDWARE	( 1 << 3 )
#define DRVLIST_ALL			( 1 << 4 )

jeBoolean	DrvList_PickDriver(HANDLE hInstance, 
							   HWND hwndParent, 
							   jeEngine *Engine, 
							   jeDriver **Driver, 
							   jeDriver_Mode **Mode, 
							   jeBoolean DisplayDialog,
							   int ModeFilter );

// Added JH 7.3.2000
jeBoolean DrvList_GetDriverByName(
								jeEngine		*Engine,
								char			*sDriverMode,
								jeDriver		**Driver,
								jeDriver_Mode	**Mode);

#ifdef __cplusplus
}
#endif

#endif // DRVLIST_H

