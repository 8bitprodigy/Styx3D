/****************************************************************************************/
/*  SPAWN.C                                                                             */
/*                                                                                      */
/*  Author:  Peter Siamidis                                                             */
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
#pragma warning ( disable : 4514 )
#include <assert.h>
#include "Spawn.h"



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spawn_App()
//
//	Spawn a new application based on a launch string. The launch string could
//	be the name of an exe, document, url, etc.
//
//	Returns
//		HANDLE
//			Handle to the spawned app.
//		NULL
//			No app was spawned, or it was a sync spawn in which
//			case the app spawned and exited.
//
////////////////////////////////////////////////////////////////////////////////////////
HANDLE Spawn_App(
	char		*LaunchString,	// spawn launch string
	char		*WorkingDir,	// working directory
	Spawn_Style	Style )			// spawn style
{

	// locals
	SHELLEXECUTEINFO	ShellInfo;

	// init ShellInfo struct
	memset( &ShellInfo, 0, sizeof( SHELLEXECUTEINFO ) );
	ShellInfo.cbSize = sizeof( SHELLEXECUTEINFO );
	ShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShellInfo.lpFile = LaunchString;
	ShellInfo.nShow = SW_SHOWNORMAL;
	ShellInfo.lpDirectory = WorkingDir;

	// launch new process
	if ( ShellExecuteEx( &ShellInfo ) == 0 )
	{
		return NULL;
	}

	// if the launch string is not an exe file, then ShellInfo.hProcess may be null,
	// so we need to get a handle to the app that was loaded
	if ( ShellInfo.hProcess == NULL )
	{

		// get app handle from hinstance
		//undone how do i do this?

	}

	// if its a synchronous spawn, then wait for the app to finish then null its handle
	if ( Style == Spawn_Sync )
	{
		while ( Spawn_IsActive( ShellInfo.hProcess ) );
		ShellInfo.hProcess = NULL;
	}

	// return spawed app handle
	return ShellInfo.hProcess;

} // Spawn_App()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spawn_IsActive()
//
//	Determines if an app is still active.
//
//	Returns
//		TRUE
//			App is still active.
//		FALSE
//			App is not active.
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL Spawn_IsActive(
	HANDLE	hApp )	// handle to app that we will query
{

	// locals
	BOOL	AppQuery;
	DWORD	Status = 0;

	// fail if we have a bad handle
	if ( hApp == NULL )
	{
		assert( 0 );
		return FALSE;
	}

	// get app status
	AppQuery = GetExitCodeProcess( hApp, &Status );
	if ( AppQuery == FALSE )
	{
		return FALSE;
	}

	// return app status
	if ( Status & STILL_ACTIVE )
	{
		return TRUE;
	}
	return FALSE;

} // Spawn_IsActive()
