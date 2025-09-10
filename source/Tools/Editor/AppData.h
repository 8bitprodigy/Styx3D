/****************************************************************************************/
/*  APPDATA.H                                                                           */
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

#ifndef APPDATA_H
#define APPDATA_H

typedef struct tagAppData AppData ;

#include "Basetype.h"


#ifdef __cplusplus
extern "C" {
#endif

AppData	*		AppData_Create( void ) ;
void			AppData_Destroy( AppData ** ppAppData ) ;

uint32			AppData_GetHandleCornerBitmap( void ) ;
uint32			AppData_GetHandleEdgeBitmap( void ) ;

uint32			AppData_GetHandleRotateTL( void ) ;
uint32			AppData_GetHandleRotateTR( void ) ;
uint32			AppData_GetHandleRotateBL( void ) ;
uint32			AppData_GetHandleRotateBR( void ) ;

uint32			AppData_GetHandleShearLR( void ) ;
uint32			AppData_GetHandleShearTB( void ) ;

uint32			AppData_GetVertex( void ) ;
uint32			AppData_GetSelectedVertex( void ) ;

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: AppData.h */