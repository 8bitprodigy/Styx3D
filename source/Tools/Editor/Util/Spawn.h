/****************************************************************************************/
/*  SPAWN.H                                                                             */
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
#ifndef SPAWN_H
#define SPAWN_H

#pragma warning ( disable : 4201 4214 4115 )
#include <windows.h>
#pragma warning ( default : 4201 4214 4115 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Spawn styles
////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	Spawn_Async = 0,
	Spawn_Sync
} Spawn_Style;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

// spawn a new app
extern HANDLE Spawn_App(
	char		*LaunchString,	// spawn launch string
	char		*WorkingDir,	// working directory
	Spawn_Style	Style );		// spawn style

// determines if an app is still active
extern BOOL Spawn_IsActive(
	HANDLE	hApp );	// handle to app that we will query


#ifdef __cplusplus
	}
#endif

#endif
