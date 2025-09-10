/****************************************************************************************/
/*  OBJECTDEF.C                                                                         */
/*                                                                                      */
/*  Author: Anthony Rufrano													            */
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
#include <windows.h>
#include "jeTypes.h"
#include "Object.h"
#include "ObjectDef.h"
#include "jeVersion.h"
#include "StaticMeshObj.h"

jeObjectDef ObjectDef = {
	JE_OBJECT_TYPE_UNKNOWN,
	"StaticMesh",
	0,

	CreateInstance,
	CreateRef,
	Destroy,

	AttachWorld,
	DetachWorld,

	AttachEngine,
	DetachEngine,

	AttachSoundSystem,
	DetachSoundSystem,

	Render,
	Collision,
	GetExtBox,

	CreateFromFile,
	WriteToFile,

	GetPropertyList,
	SetProperty,
	NULL,

	SetXForm,
	GetXForm,
	GetXFormModFlags,

	GetChildren,
	AddChild,
	RemoveChild,

	EditDialog,
	//	by trilobite	Jan. 2011
	//MessageFunction,
	//UpdateTimeDelta,
	 SendAMessage,
	 Frame,

	NULL,

	ChangeBoxCollision,

	NULL,
	NULL,
	NULL,
};

////////////////////////////////////////////////////////////////////////////////////////
//
//	Object_RegisterDef()
//
////////////////////////////////////////////////////////////////////////////////////////
OBJECT_API jeBoolean Object_RegisterDef(
	float Major,	// major version number
	float Minor )	// minor version number
{

	// fail if versions don't match
	if ( ( Major != JET_MAJOR_VERSION ) || ( Minor != JET_MINOR_VERSION ) )
	{
		return JE_FALSE;
	}

	// register def
	return jeObject_RegisterGlobalObjectDef(&ObjectDef);

} // Object_RegisterDef()
