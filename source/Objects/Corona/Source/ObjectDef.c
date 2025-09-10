/****************************************************************************************/
/*  OBJECTDEF.C                                                                         */
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
#pragma warning ( disable : 4115 )
#include <windows.h>
#pragma warning ( default : 4115 )
#include "jeTypes.h"
#include "Corona.h"
#include "Object.h"
#include "ObjectDef.h"
#include "jeVersion.h"
#include "Errorlog.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Object definition
////////////////////////////////////////////////////////////////////////////////////////
jeObjectDef ObjectDef =
{
	JE_OBJECT_TYPE_UNKNOWN,
	"Corona",
	0,
	CreateInstance,
	CreateRef,
	Destroy,
	AttachWorld,
	DettachWorld,
	AttachEngine,
	DettachEngine,
	AttachSoundSystem,
	DettachSoundSystem,
	Render,
	Collision,
	GetExtBox,
	CreateFromFile,
	WriteToFile,
	GetPropertyList,
	SetProperty,
	NULL,			// GetProperty
	SetXForm,
	GetXForm,
	GetXFormModFlags,
	GetChildren,
	AddChild,
	RemoveChild,
	EditDialog,
	SendAMessage,
	Frame,
	//Royce
	DuplicateInstance,
	//---
	ChangeBoxCollision, //Icestorm
	NULL,	// GetGlobalPropertyList
	NULL,	// SetGlobalProperty
	NULL,	//SetRenderNextTime,
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	DLL Entry point
//
////////////////////////////////////////////////////////////////////////////////////////
int WINAPI DllMain(
	HINSTANCE	hInstance,
	DWORD		fdwReason,
	PVOID		pvReserved )
{

	// process launch reason
	switch( fdwReason )
	{

		// dll load
		case DLL_PROCESS_ATTACH:
		{
			if ( InitClass( hInstance ) == JE_FALSE )
			{
				jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Failed to init class" );
				return FALSE;
			}
			break;
		}

		// dll free
		case DLL_PROCESS_DETACH:
		{
			if ( DeInitClass() == JE_FALSE )
			{
				jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Failed to deinit class" );
				return FALSE;
			}
			break;
		}
	}

	// all done
	return TRUE;

	// eliminate warnings
	pvReserved;

} // DllMain()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Object_RegisterDef()
//
////////////////////////////////////////////////////////////////////////////////////////
DLLExport jeBoolean Object_RegisterDef(
	float Major,	// major version number
	float Minor )	// minor version number
{

	// fail if versions don't match
	if ( ( Major != JET_MAJOR_VERSION ) || ( Minor != JET_MINOR_VERSION ) )
	{
		jeErrorLog_Add( JE_ERR_DATA_FORMAT, "Version numbers don't match" );
		return JE_FALSE;
	}

	// register def
	if ( jeObject_RegisterGlobalObjectDef( &ObjectDef ) == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Failed to register global object def" );
		return JE_FALSE;
	}

	// all done
	return JE_TRUE;

} // Object_RegisterDef()
