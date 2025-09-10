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
#ifdef WIN32
#pragma warning ( disable : 4115 )
#include <windows.h>
#pragma warning ( default : 4115 )
#endif


#ifdef BUILD_BE

#include <image.h>
// Here we emulate a DllMain by calling it as soon as it is loaded.
#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH 2 // the two thread ones are never called under be..
#define DLL_THREAD_DETACH 3
#define DLL_PROCESS_DETACH 4

#define WINAPI __declspec(dllexport)
#define DWORD long
#define PVOID void*

#ifdef __cplusplus
extern "C" {
#endif
int WINAPI DllMain(image_id	hInstance,DWORD		fdwReason, PVOID		pvReserved );
#ifdef __cplusplus
}
#endif

#endif

#include "jeTypes.h"
#include "PulsingLight.h"
#include "Object.h"
#include "ObjectDef.h"
#include "jeVersion.h"

////////////////////////////////////////////////////////////////////////////////////////
//	Object definition
////////////////////////////////////////////////////////////////////////////////////////
jeObjectDef ObjectDef =
{
	JE_OBJECT_TYPE_UNKNOWN,
	"Pulsing Light",
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
	DuplicateInstance, //DuplicateInstance
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
#ifdef WIN32
	HINSTANCE	hInstance,
#endif
#ifdef BUILD_BE
	image_id 	hInstance,
#endif

	DWORD		fdwReason,
	PVOID		pvReserved )
{

	// process launch reason
	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
		{
			Init_Class( hInstance );
			break;
		}

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		{
			break;
		}

		case DLL_PROCESS_DETACH:
		{
			DeInit_Class();
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
		return JE_FALSE;
	}

	// register def
	return ( jeObject_RegisterGlobalObjectDef( &ObjectDef ) );

} // Object_RegisterDef()
