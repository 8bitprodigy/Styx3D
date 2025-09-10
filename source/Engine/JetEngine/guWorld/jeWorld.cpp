/****************************************************************************************/
/*  jeWorld.C                                                                           */
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
/****************************************************************************************/
/*      jeWorld.c                                                                       */
/*                                                                                      */
/*      REVISION: 01-13-1999  8:32 p.m.                                                 */
/*			John Pollard : Created                                                      */
/*                                                                                      */
/*      Copyright (c) 1999, Eclipse Entertainment; All rights reserved.                 */
/*                                                                                      */
/****************************************************************************************/
#include <stdio.h>
#include <memory.h>		// memset
#include <assert.h>
#include <string.h>
#include <stdlib.h> //free

#include "Dcommon.h"
#include "Engine.h"

// Public dependents
#include "jeWorld.h"
#include "Actor.h"  // Added by Incarnadine

// Private dependents
#include "Ram.h"
#include "Errorlog.h"
#include "jeMaterial._h"		// jeMaterial_ArraySetEngine
#include "jeFrustum.h"
#include "jeChain.h"
#include "jePortal.h"
#include "Util.h"			// Added by Icestorm [MLB-ICE]

#include "jePtrMgr._h"
#include "log.h"

//#define FIRST_OBJECT_IN_HIERARCHY_IS_MODEL_HACK

#ifdef _DEBUG 
	#define WORLD_DEBUG_OUTPUT_LEVEL	1
	//#define WORLD_DEBUG_OUTPUT_LEVEL	2
#else
	#define WORLD_DEBUG_OUTPUT_LEVEL	0
#endif

//
//	Please note that this module is called jeWorld.  It is only temporary 
//	until jeWorld replaces jeWorld... (uh huh...)
//
void ProcUtil_Init(void);

#pragma message (" Clean up Add/Remove code.  There are some leaks on error...")

//========================================================================================
// Local #defines
//========================================================================================
#define	JU_WORLD_START_FACEINFO		16
#define	JU_WORLD_START_MATERIALS	16
#define JU_WORLD_START_LIGHTS		16

#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

//========================================================================================
//	Local static defines
//========================================================================================
static jeBoolean jeWorld_DestroyAutoRemoveUserPolys(jeWorld *World);
static jeBoolean jeWorld_RenderUserPolys(const jeWorld *World, const jeCamera *Camera, const jeFrustum *WorldSpaceFrustum);
static jeBoolean jeWorld_CreateArrays(jeWorld *World);
static jeBoolean jeWorld_WriteHeader(const jeWorld *World, jeVFile *VFile);
static jeBoolean jeWorld_ReadHeader(jeWorld *World, jeVFile *VFile);

static jeBoolean WriteObject(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *);
static jeBoolean jeWorld_WriteArrays(const jeWorld *World, jeVFile *VFile, jePtrMgr *PtrMgr);
static jeBoolean WriteLight(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr);
static jeBoolean WritePtrMgrVerification(jeVFile *VFile, const jePtrMgr *PtrMgr);

static jeBoolean ReadObject(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr);
static jeBoolean jeWorld_ReadArrays(jeWorld *World, jeVFile *VFile, jePtrMgr *PtrMgr);
static jeBoolean ReadLight(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr);
static jeBoolean ReadPtrMgrVerification(jeVFile *VFile, const jePtrMgr *PtrMgr);


typedef struct jeWorld
{
	int32						RefCount;

	// Various arrays shared by ALL objects
	jeFaceInfo_Array			*FaceInfoArray;
	jeMaterial_Array			*MaterialArray;

	jeChain						*LightChain;		// Linked list of lights
	jeChain						*OldLights;			// Linked list of backup lights

	jeChain						*Objects;			// Linked list of objects

	jeChain						*DLightChain;							// Dynamic light chain

	jeChain						*UserPolys;
	jeChain						*AutoRemoveUserPolys;

	//jeChain						*Actors;  // Added by Incarnadine
	jeChain *CollisionObjectTypes; // Incarnadine
	int32 CollisionLevel; // Incarnadine

	// paradoxnj - Useless and incomplete
	//jeChain						*ShaderChain; // Added by CyRiuS (Timothy Roff)
	//jeChain						*ActorScriptChain; //Added by cyrius (Timothy Roff)

	jeEngine					*Engine;
	jeSound_System				*SoundSystem;
	
	jeResourceMgr				*ResourceMgr;

	int32						Recursion;

	jeObject					*Model;
	
} jeWorld;

jeWorld_DebugInfo				g_WorldDebugInfo;

static jeBoolean jeWorld_RenderALL(jeWorld *World, jeCamera *Camera, jeFrustum *CameraSpaceFrustum);
static jeBoolean JETCC jeWorld_RenderFromMirrorPortal(const jePortal *Portal, const jeWorld *World, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum);

/*
jeVFile * jeWorld_GetObjectDirectory(jeWorld *World)
{
	assert(World);
	return World->FileDirectory;
}

jeObject * jeWorld_FindObjectFromName(jeWorld *World, char *NameToFind)
{
	jeObject *CurrObject;
	const char *Name;

	assert (World);
	assert (NameToFind);

	// loop initializers
	CurrObject = nullptr;

	while (JE_TRUE)
		{
		CurrObject = jeWorld_GetNextObject(World, CurrObject);

		if (CurrObject == nullptr)
			break;

		Name = jeObject_GetName( CurrObject );

		if (!Name) continue;

		if (stricmp(NameToFind, Name) == 0)
			return (CurrObject);
		}

	return nullptr;
}
*/

////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_StrDup()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Util_StrDup(
	const char	*const String )	// string to copy
{

	// locals
	char	*NewString{};

	// ensure valid data
	assert( String != nullptr );

	// copy string
	NewString = (char *)jeRam_Allocate( strlen( String ) + 1 );
	if ( NewString ) 
	{
		strcpy( NewString, String );
	}
	else
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, nullptr );
	}

	// return string
	return NewString;

} // Util_StrDup()




// HACK of all mothers!  This should be the next thing cleaned up, this is just to get it working!!!
void					*h_World;

typedef struct 
{
	const jePlane		*Plane;
	const jeXForm3d		*FaceXForm;
	jeWorld				*World;
	jeCamera			*Camera;
	jeFrustum			*Frustum;
} PortalMsgData;

//========================================================================================
//	CreateMirrorObjectInstance
//========================================================================================
void * JETCC CreateMirrorObjectInstance(void)
{
	return jePortal_Create();
}

//========================================================================================
//	RefMirrorObjectInstance
//========================================================================================
void JETCC RefMirrorObjectInstance(void *Portal)
{
	jePortal_CreateRef((jePortal*)Portal);
}

//========================================================================================
//	DestroyMirrorObjectInstance
//========================================================================================
jeBoolean JETCC DestroyMirrorObjectInstance(void **Portal)
{
	jePortal_Destroy((jePortal**)Portal);

	return JE_TRUE;
}

#define MAX_MIRROR_RECURSION		1

#pragma message ("Fix this big hack-a-rama (MirrorRecursion global)")
int32 MirrorRecursion		= 0;
jeBoolean					h_LeftHanded;

//========================================================================================
//	RenderMirrorObjectInstance
//========================================================================================
static jeBoolean JETCC RenderMirrorObjectInstance(const void *PortalInst, const jeWorld *World, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags)
{
	return JE_TRUE;
}


void *JETCC ReadMirrorObjectInstance(jeVFile *VFile, jePtrMgr *PtrMgr)
{
	return jePortal_Create();
}

//========================================================================================
//	WriteMirrorObjectInstance
//========================================================================================
jeBoolean JETCC WriteMirrorObjectInstance(const void *Instance, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	return JE_TRUE;
}


//========================================================================================
//	RenderMirrorObjectInstance2
//========================================================================================
static jeBoolean RenderMirrorObjectInstance2(void *Portal, const jePlane *Plane, const jeXForm3d *FaceXForm, jeWorld *World, jeCamera *Camera, jeFrustum *Frustum)
{
	jeXForm3d		XForm{};
	jeXForm3d		MirrorXForm{};
	jePlane			FrontPlane{};
	jeBoolean		Ret{};
	jeXForm3d		WorldToCameraXForm{};
	jePortal		*P = (jePortal*)Portal;

	if (P->Recursion > 0)
		return JE_TRUE;

	// Get Camera XForm
	jeCamera_GetXForm((jeCamera*)Camera, &XForm);

	// Get the WorldToCameraXForm
	jeCamera_GetTransposeXForm(Camera, &WorldToCameraXForm);

	// Transform the FacePlane to camera space
	jePlane_Transform(Plane, &WorldToCameraXForm, &FrontPlane);

	// Add the Plane to the Frustum
	if (!jeFrustum_AddPlane(Frustum, &FrontPlane, JE_TRUE))
		return JE_FALSE;

	// Mirror the camera XForm
	jeXForm3d_Mirror(&XForm, &Plane->Normal, Plane->Dist, &MirrorXForm);
	
	h_LeftHanded = !h_LeftHanded;

	// Put the new mirrored XForm into the camera
	jeCamera_SetXForm(Camera, &MirrorXForm);

	// Increase the portal recursion count
	P->Recursion++;

	// Render the scene from this camera
	Ret = jeWorld_Render(World, Camera, Frustum);

	// Decrease the portal recursion count
	assert(P->Recursion > 0);
	P->Recursion--;

	// Restore Camera XForm
	jeCamera_SetXForm(Camera, &XForm);
	h_LeftHanded = !h_LeftHanded;

	return Ret;
}

//========================================================================================
//	SendMirrorMessage
//========================================================================================
static jeBoolean JETCC SendMirrorMessage(void *Portal, int32 Msg, void *Data)
{
	switch (Msg)
	{
		case 0:
		{
			PortalMsgData		*MData;
			
			MData = (PortalMsgData*)Data;

			return RenderMirrorObjectInstance2(	Portal, 
												MData->Plane, 
												MData->FaceXForm, 
												MData->World, 
												MData->Camera, 
												MData->Frustum);
		}

		default:
			return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	MirrorObjectDef
//========================================================================================
jeObjectDef MirrorObjectDef = 
{
	JE_OBJECT_TYPE_PORTAL,
	"MirrorObject",
	JE_OBJECT_HIDDEN,

	CreateMirrorObjectInstance,
	RefMirrorObjectInstance,
	DestroyMirrorObjectInstance,

	nullptr,
	nullptr,

	nullptr,
	nullptr,

	nullptr,
	nullptr,

	RenderMirrorObjectInstance,

	nullptr,
	nullptr,

	ReadMirrorObjectInstance,
	WriteMirrorObjectInstance,

	nullptr,
	nullptr,
	nullptr,

	nullptr,
	nullptr,

	nullptr,

	nullptr,
	nullptr,
	nullptr,

	nullptr,
	SendMirrorMessage,
	nullptr,
	nullptr,
	nullptr,	// Added by Icestorm: ChangeBoxCollision
	nullptr,	// GetGlobalPropertyList
	nullptr,	// SetGlobalProperty
	nullptr,	//SetRenderNextTime,
};

//========================================================================================
//	jeWorld_AddDefaultObjects
//========================================================================================
static jeBoolean jeWorld_AddDefaultObjects(jeWorld *World)
{
	jeObject			*MirrorObject;

	// Add the mirror portal object
	MirrorObject = jeObject_Create("MirrorObject");

	if (!MirrorObject)
		return JE_FALSE;

	// [MLB-ICE]
	//MirrorObject->Name = strdup("Mirror1");
	MirrorObject->Name = Util_StrDup("Mirror1");

	if (!jeWorld_AddObject(World, MirrorObject))
	{
		jeObject_Destroy(&MirrorObject);	// Added
		return JE_FALSE;
	}

	jeObject_Destroy(&MirrorObject);	// Added
	// [MLB-ICE] EOB

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_CreateBase
//========================================================================================
static jeWorld *jeWorld_CreateBase(jeResourceMgr *ResourceMgr)
{
	jeWorld		*World{};

	assert(ResourceMgr);

	World = (jeWorld *)jeRam_AllocateClear(sizeof(*World));

	if (!World)
		return nullptr;

	World->RefCount = 1;

	World->Model = nullptr;

	// Added by Incarnadine
	// create the actor chain
	//World->Actors = jeChain_Create();

	//if (!World->Actors)
	//	goto ExitWithError;
	World->CollisionObjectTypes = jeChain_Create();

	if(!World->CollisionObjectTypes)
		goto ExitWithError;

	jeWorld_SetCollisionLevel(World,COLLIDE_EXTBOX);	

	// paradoxnj - Useless and incomplete
	// create the shader chain (cyrius)
	/*World->ShaderChain = jeChain_Create();

	if(!World->ShaderChain)
		goto ExitWithError;

	// create the actor script chain (cyrius)
	World->ActorScriptChain = jeChain_Create();

	if(!World->ActorScriptChain)
		goto ExitWithError;
	*/
	
	// Create the Dyanamic Light Chain
	World->DLightChain = jeChain_Create();

	if (!World->DLightChain)
		goto ExitWithError;

	// Create the UserPoly Chain
	World->UserPolys = jeChain_Create();

	if (!World->UserPolys)
		goto ExitWithError;

	// Create the AutoRemoveUserPoly Chain
	World->AutoRemoveUserPolys = jeChain_Create();

	if (!World->AutoRemoveUserPolys)
		goto ExitWithError;

	// Assign the resource mgr
	World->ResourceMgr = ResourceMgr;

	if ( World->ResourceMgr == nullptr )
		goto ExitWithError;

	// Register the built-in objects
	{
		//extern jeObjectDef	PortalObjectDef;	// Icestorm: Seems to be useless now...

		jeObject_RegisterGlobalObjectDef(&MirrorObjectDef);
		//jeObject_RegisterGlobalObjectDef(&PortalObjectDef);
	}

	// HACK of all mothers!
	h_World = World;

	return World;

	ExitWithError:
	{
		jeErrorLog_AddString(-1, "jeWorld_CreateBase failed...", nullptr);

		if (World)
		{
			if (World->DLightChain)
				jeChain_Destroy(&World->DLightChain);

			if (World->UserPolys)
				jeChain_Destroy(&World->UserPolys);

			if (World->AutoRemoveUserPolys)
				jeChain_Destroy(&World->AutoRemoveUserPolys);

			if (World->Objects)
				jeChain_Destroy(&World->Objects);

			if (World->ResourceMgr)
				jeResource_MgrDestroy(&World->ResourceMgr);

			//if (World->Actors) // Added cause it was needed (cyrius)
			//	jeChain_Destroy(&World->Actors);
			if(World->CollisionObjectTypes)
				jeChain_Destroy(&World->CollisionObjectTypes);

			// paradoxnj - Useless and incomplete
			/*if(World->ShaderChain) //(cyrius)
				jeChain_Destroy(&World->ShaderChain);

			if(World->ActorScriptChain) //(cyrius)
				jeChain_Destroy(&World->ActorScriptChain);
			*/

			jeRam_Free(World);
		}

		return nullptr;
	}
}

//========================================================================================
//	jeWorld_Create
//========================================================================================
JETAPI jeWorld * JETCC jeWorld_Create(jeResourceMgr *pResourceMgr)
{
	jeWorld		*World{};

	assert(pResourceMgr);
	
	World = jeWorld_CreateBase(pResourceMgr);

	if (!World)
		return nullptr;

	if (!jeWorld_CreateArrays(World))
		goto ExitWithError;
	
	// Create the light chain
	World->LightChain = jeChain_Create();

	if (!World->LightChain)
		goto ExitWithError;

	// Create the backup light chain
	World->OldLights = jeChain_Create();

	if (!World->OldLights)
		goto ExitWithError;

	// create the object chain
	World->Objects = jeChain_Create();

	if (!World->Objects)
		goto ExitWithError;

	// paradoxnj - Useless and incomplete
	//Init the ProcUtil routines (cyrius)
	//ProcUtil_Init();
	

#ifndef FIRST_OBJECT_IN_HIERARCHY_IS_MODEL_HACK
	if (!jeWorld_AddDefaultObjects(World))
		goto ExitWithError;
#endif

	return World;

	ExitWithError:
	{
		if (World)
			jeWorld_Destroy(&World);

		return nullptr;
	}
}


//========================================================================================
//	jeWorld_CreateFromFile
//========================================================================================
JETAPI jeWorld * JETCC jeWorld_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr, jeResourceMgr *pResourceMgr)
{
	jeWorld* World{};

	World = jeWorld_CreateBase(pResourceMgr);

	if (!World)
		return nullptr;

	if (!PtrMgr)
	{
		PtrMgr = jePtrMgr_Create();
		
		if (!PtrMgr)
			goto ExitWithError;
	}
	else
	{
		// Ref it, so destroy code can be called in both cases
		if (!jePtrMgr_CreateRef(PtrMgr))
			goto ExitWithError;
	}

	// Krouer: inform the PtrMgr on the resource and world it behaves
	PtrMgr->pWorld = World;
	PtrMgr->pResMgr = pResourceMgr;

	// Read the header
	if (!jeWorld_ReadHeader(World, VFile))
		goto ExitWithError;

	//if (!ReadPtrMgrVerification(VFile, PtrMgr))
	//	goto ExitWithError;

	// Load the world arrays from disk
	if (!jeWorld_ReadArrays(World, VFile, PtrMgr))
		goto ExitWithError;

	// Load the lights off disk
	World->LightChain = jeChain_CreateFromFile(VFile, ReadLight, nullptr, PtrMgr);

	if (!World->LightChain)
		goto ExitWithError;

	// Load the backup lights off disk
	World->OldLights = jeChain_CreateFromFile(VFile, ReadLight, nullptr, PtrMgr);

	if (!World->OldLights)
		goto ExitWithError;

	// Load the objects off disk
	World->Objects = jeChain_CreateFromFile(VFile, ReadObject, World, PtrMgr);

	if (!World->Objects)
		goto ExitWithError;

	//if (!ReadPtrMgrVerification(VFile, PtrMgr))
	//	goto ExitWithError;

	return World;
	
	ExitWithError:
	{
		if (World)
		{
			if (PtrMgr)
				jePtrMgr_Destroy(&PtrMgr);

			jeWorld_Destroy(&World);
		}
		return nullptr;
	}
}



//========================================================================================
//	jeWorld_WriteToFile
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_WriteToFile(const jeWorld *World, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	if (!PtrMgr)		// The world needs an PtrMgr, so create one if one not supplied...
	{
		PtrMgr = jePtrMgr_Create();
		
		if (!PtrMgr)
			goto ExitWithError;
	}
	else
	{
		// Ref it, so destroy code can be called in both cases
		if (!jePtrMgr_CreateRef(PtrMgr))
			goto ExitWithError;
	}

	// Write out header info
	if (!jeWorld_WriteHeader(World, VFile))
	{
		jeErrorLog_AddString(-1, "jeWorld_WriteToFile:  jeWorld_WriteHeader failed.", nullptr);
		goto ExitWithError;
	}

	//if (!WritePtrMgrVerification(VFile, PtrMgr))
	//	goto ExitWithError;

	// Write the arrays
	if (!jeWorld_WriteArrays(World, VFile, PtrMgr))
	{
		jeErrorLog_AddString(-1, "jeWorld_WriteToFile:  jeWorld_WriteArrays failed.", nullptr);
		goto ExitWithError;
	}

	// Write out the Lights
	if (!jeChain_WriteToFile(World->LightChain, VFile, WriteLight, nullptr, PtrMgr))
	{
		jeErrorLog_AddString(-1, "jeWorld_WriteToFile:  jeChain_WriteToFile failed for lights.", nullptr);
		goto ExitWithError;
	}

	// Write out the backup Lights
	if (!jeChain_WriteToFile(World->OldLights, VFile, WriteLight, nullptr, PtrMgr))
	{
		jeErrorLog_AddString(-1, "jeWorld_WriteToFile:  jeChain_WriteToFile failed for lights.", nullptr);
		goto ExitWithError;
	}


	// Write out the objects
	if (!jeChain_WriteToFile(World->Objects, VFile, WriteObject, nullptr, PtrMgr))
	{
		jeErrorLog_AddString(-1, "jeWorld_WriteToFile:  jeChain_WriteToFile failed for objects.", nullptr);
		goto ExitWithError;
	}

	// This is for debugging, so you can run your mouse over, and examine contents
	{
		int32		NumPtrs, PtrRefs;

		jePtrMgr_GetPtrCount(PtrMgr, &NumPtrs);
		jePtrMgr_GetPtrRefs(PtrMgr, &PtrRefs);
	}

	//if (!WritePtrMgrVerification(VFile, PtrMgr))
	//	goto ExitWithError;

	if (PtrMgr)
		jePtrMgr_Destroy(&PtrMgr);

	return JE_TRUE;

	ExitWithError:
	{
		return JE_FALSE;
	}
}

//========================================================================================
//	jeWorld_CreateRef
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_CreateRef(jeWorld *World)
{
	assert(World);
	assert(World->RefCount >= 0);

	World->RefCount++;

	return JE_TRUE;
}


//========================================================================================
//	jeWorld_Destroy
//========================================================================================
JETAPI void JETCC jeWorld_Destroy(jeWorld **pWorld)
{
	jeChain_Link		*Link{};
	jeWorld				*World{};

	assert(pWorld);
	assert(*pWorld);

	World =	*pWorld;

	//assert(World->RefCount > 0);
#ifdef _DEBUG
	Log_Printf("jeWorld_Destroy %p, %d\n", World, World->RefCount);
#endif

	World->RefCount--;

	if (World->RefCount == 0)
	{
		// destroy all objects
		if (World->Objects)
		{
			// destroy each object
			for (Link = jeChain_GetFirstLink(World->Objects); Link; Link = jeChain_LinkGetNext(Link))
			{
				// locals
				jeObject	*Object;

				// jet object pointer
				Object = (jeObject *)jeChain_LinkGetLinkData( Link );

				// BEGIN - Proper destruction of objects - paradoxnj 5/9/2005
				jeObject_RemoveChild(World->Model,Object);

				jeObject_DettachWorld(Object, World);
				
				if (World->SoundSystem != nullptr)
					jeObject_DettachSoundSystem(Object, World->SoundSystem);

				jeObject_DettachEngine(Object, World->Engine);
				// END - Proper destruction of objects - paradoxnj 5/9/2005

				jeObject_Destroy( &Object );
			}

			// destroy object chain
			jeChain_Destroy( &( World->Objects ) );
		}

		// destroy all actors -- Incarnadine
/*		
		if (World->Actors)
		{
			// destroy each object
			for (Link = jeChain_GetFirstLink(World->Actors); Link; Link = jeChain_LinkGetNext(Link))
			{
				// locals
				jeActor *Actor;

				// jet object pointer
				Actor = (jeActor *)jeChain_LinkGetLinkData( Link );
				jeActor_Destroy( &Actor );
			}

			// destroy object chain
			jeChain_Destroy( &( World->Actors ) );
		}
*/
		if (World->CollisionObjectTypes)
		{
			// destroy each object
			for (Link = jeChain_GetFirstLink(World->CollisionObjectTypes); Link; Link = jeChain_LinkGetNext(Link))
			{
				// locals
				char *Data;

				// jet object pointer
				Data = (char *)jeChain_LinkGetLinkData( Link );
				jeRam_Free(Data);				
			}

			// destroy object chain
			jeChain_Destroy( &( World->CollisionObjectTypes ) );
		}


		// paradoxnj - Useless and incomplete
		// destroy all shaders -- CyRiuS
		/*if (World->ShaderChain)
		{
			// destroy each shader
			for(Link = jeChain_GetFirstLink(World->ShaderChain); Link; Link = jeChain_LinkGetNext(Link))
			{
				// locals
				jeShader	*Shader;

				// jet object pointer
				Shader = (jeShader *)jeChain_LinkGetLinkData( Link );
				jeShader_Destroy( &Shader );
			}

			//destroy object chain
			jeChain_Destroy( &( World->ShaderChain ) );
		}

		// destroy all ActorScripts -- CyRiuS
		if (World->ActorScriptChain)
		{
			// destroy each shader
			for(Link = jeChain_GetFirstLink(World->ActorScriptChain); Link; Link = jeChain_LinkGetNext(Link))
			{
				// locals
				jeScript	*ActorScript;

				// jet object pointer
				ActorScript = (jeScript *)jeChain_LinkGetLinkData( Link );
				jeScript_Destroy( &ActorScript );
			}

			//destroy object chain
			jeChain_Destroy( &( World->ActorScriptChain ) );
		}
		*/

		// Destroy all dlights
		if (World->DLightChain)
		{
			for (Link = jeChain_GetFirstLink(World->DLightChain); Link; Link = jeChain_LinkGetNext(Link))
			{
				jeLight		*Light;

				Light = (jeLight*)jeChain_LinkGetLinkData(Link);
				jeLight_Destroy(&Light);
			}
			jeChain_Destroy(&World->DLightChain);
		}

		// Destroy all Lights
		if (World->LightChain)
		{
			for (Link = jeChain_GetFirstLink(World->LightChain); Link; Link = jeChain_LinkGetNext(Link))
			{
				jeLight		*Light;

				Light = (jeLight*)jeChain_LinkGetLinkData(Link);
				jeLight_Destroy(&Light);
			}

			jeChain_Destroy(&World->LightChain);
		}

		// Destroy all backup lights
		if (World->OldLights)
		{
			for (Link = jeChain_GetFirstLink(World->OldLights); Link; Link = jeChain_LinkGetNext(Link))
			{
				jeLight		*Light;

				Light = (jeLight*)jeChain_LinkGetLinkData(Link);
				jeLight_Destroy(&Light);
			}

			jeChain_Destroy(&World->OldLights);
		}

		// Destroy all user polys
		if (World->UserPolys)
		{
			for (Link = jeChain_GetFirstLink(World->UserPolys); Link; Link = jeChain_LinkGetNext(Link))
			{
				jeUserPoly		*Poly;

				Poly = (jeUserPoly*)jeChain_LinkGetLinkData(Link);
				jeUserPoly_Destroy(&Poly);
			}

			jeChain_Destroy(&World->UserPolys);
		}

		// Destroy all autoremove user polys
		if (World->AutoRemoveUserPolys)
		{
			for (Link = jeChain_GetFirstLink(World->AutoRemoveUserPolys); Link; Link = jeChain_LinkGetNext(Link))
			{
				jeUserPoly		*Poly;
	
				Poly = (jeUserPoly*)jeChain_LinkGetLinkData(Link);
				jeUserPoly_Destroy(&Poly);
			}
	
			jeChain_Destroy(&World->AutoRemoveUserPolys);
		}

		// Destroy faceinfo array
		if (World->FaceInfoArray)
			jeFaceInfo_ArrayDestroy(&(World->FaceInfoArray));
	
		// destroy material array
		if (World->MaterialArray)
			jeMaterial_ArrayDestroy(&(World->MaterialArray));

		// destroy resource manager
		if ( World->ResourceMgr != nullptr )
		{
			jeResource_MgrDestroy( &( World->ResourceMgr ) );
		}

		if (World->Model != nullptr)
		{
			jeObject_Destroy(&World->Model);
		}

		jeRam_Free(World);
	}

	*pWorld = nullptr;
}

//========================================================================================
//	jeWorld_SetEngine
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_SetEngine(jeWorld *World, jeEngine *Engine)
{
	jeChain_Link	*Link{};

	assert(World);

	if (!jeMaterial_ArraySetEngine(World->MaterialArray, Engine))
	{
		jeErrorLog_AddString(-1, "jeWorld_SetEngine:  jeMaterial_ArraySetEngine failed.", nullptr);
		return JE_FALSE;
	}

	for (Link = jeChain_GetFirstLink(World->Objects); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeObject* Object{};

		Object = (jeObject*)jeChain_LinkGetLinkData(Link);

		if (!jeObject_AttachEngine( Object, Engine ))
		{
			jeErrorLog_AddString(-1, "jeWorld_SetEngine:  jeObject_AttachEngine failed.", nullptr);
			return JE_FALSE;
		}
	}

	World->Engine = Engine;

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_GetMaterialArray
//========================================================================================
JETAPI jeMaterial_Array * JETCC jeWorld_GetMaterialArray(const jeWorld *World)
{
	assert(World);

	return World->MaterialArray;
}

//========================================================================================
//	jeWorld_GetFaceInfoArray
//========================================================================================
JETAPI jeFaceInfo_Array * JETCC jeWorld_GetFaceInfoArray(const jeWorld *World)
{
	assert(World);

	return World->FaceInfoArray;
}

//========================================================================================
//	jeWorld_GetLightChain
//========================================================================================
JETAPI jeChain * JETCC jeWorld_GetLightChain(const jeWorld *World)
{
	assert(World);

	return World->LightChain;
}

//========================================================================================
//	jeWorld_GetDLightChain
//========================================================================================
JETAPI jeChain * JETCC jeWorld_GetDLightChain(const jeWorld *World)
{
	assert(World);

	return World->DLightChain;
}

JETAPI	int32	JETCC jeWorld_GetRenderRecursion( const jeWorld *World )
{
	return( World->Recursion );
}

static uint32 WorldVisFrame = 0;


/*
//========================================================================================
//	jeWorld_RenderSprite
//	Nothing much here.
//========================================================================================
JETAPI jeBoolean JETCC jeEngine_RenderSprite(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum)
{
	jeFrustum Fru,WorldSpaceFrustum;
	if (!Frustum)
	{
		jeFrustum_SetFromCamera(&Fru, Camera);
		Frustum = &Fru;
	}

	jeFrustum_TransformToWorldSpace(Frustum, Camera, &WorldSpaceFrustum);

	return RenderSprite(Poly, Engine, Camera, &WorldSpaceFrustum);
}
*/

//========================================================================================
//	jeWorld_Render
//	This function can be recursively re-entered...
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_Render(jeWorld *World, jeCamera *Camera, jeFrustum *CameraSpaceFrustum)
{
	assert(World);
	assert(World->Engine);
	assert(Camera);

	#if (WORLD_DEBUG_OUTPUT_LEVEL >= 2)
		OutputDebugString("BEGIN jeWorld_Render\n");
	#endif

	#ifdef _DEBUG
	{
		jeEngine_FrameState	FrameState;

		jeEngine_GetFrameState(World->Engine, &FrameState);
		assert(FrameState == FrameState_Begin);
	}
	#endif
	
	if (World->Recursion == 0)
	{
		memset(&g_WorldDebugInfo, 0, sizeof(g_WorldDebugInfo));
	}

	World->Recursion++;

	g_WorldDebugInfo.NumRenders++;

	// Render the scene
	if (!jeWorld_RenderALL(World, Camera, CameraSpaceFrustum))
		return JE_FALSE;

	assert(World->Recursion > 0);
	World->Recursion--;

	if (World->Recursion == 0)
	{
		// Destroy all AutoRemove UserPolys
		if (!jeWorld_DestroyAutoRemoveUserPolys(World))
			return JE_FALSE;

	#if 0
		jeEngine_FlushScene(World->Engine);
		jeEngine_Printf(World->Engine, 3, 0*17+10, "Renders: %2i, Objects: %2i, Portals: %2i", g_WorldDebugInfo.NumRenders, g_WorldDebugInfo.NumObjects, g_WorldDebugInfo.NumPortals);
		jeEngine_Printf(World->Engine, 3, 1*17+10, "Model Polys: %3i/%3i, Actor Polys: %3i", g_WorldDebugInfo.NumTransformedPolys, g_WorldDebugInfo.NumRenderedPolys, g_WorldDebugInfo.NumActorPolys);
		jeEngine_Printf(World->Engine, 3, 2*17+10, "Nodes: %4i, Leaves: %4i", g_WorldDebugInfo.NumNodes, g_WorldDebugInfo.NumRenderedPolys, g_WorldDebugInfo.NumLeaves);
	#endif
	}

	WorldVisFrame++;

	#if (WORLD_DEBUG_OUTPUT_LEVEL >= 2)
		OutputDebugString("END jeWorld_Render\n");
	#endif

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_AddLight
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_AddLight(jeWorld *World, jeLight *Light, jeBoolean Update)
{
	jeLight* OldLight{};

	assert(World);
	assert(jeLight_IsValid(Light) == JE_TRUE);
	assert(jeChain_FindLink(World->LightChain, Light) == nullptr);
	
	// Add the light to the worlds light chain
	if (!jeChain_AddLinkData(World->LightChain, Light))
		return JE_FALSE;

	// Ref the light
	jeLight_CreateRef(Light);		

	// Make a backup copy of the light
	OldLight = jeLight_CreateFromLight(Light);

	if (!OldLight)
		goto ExitWithError1;

	// Add the light to the worlds backup light chain
	if (!jeChain_AddLinkData(World->OldLights, OldLight))
	{
		jeLight_Destroy(&OldLight);
		goto ExitWithError1;
	}

	// Make sure indexes are the same
	assert(jeChain_LinkDataGetIndex(World->LightChain, Light) == jeChain_LinkDataGetIndex(World->OldLights, OldLight));

	if (Update)
	{
		jeChain_Link		*Link;

		// Update is set, so go through all the objects, and notify them of a light update
		for (Link = jeChain_GetFirstLink(World->Objects); Link; Link = jeChain_LinkGetNext(Link))
		{
			jeObject	*Object;
		
			Object = (jeObject*)jeChain_LinkGetLinkData(Link);

			jeObject_SendMessage (Object, JE_OBJECT_MSG_WORLD_ADD_SLIGHT_UPDATE, Light);
		}
	}

	return JE_TRUE;

	ExitWithError1:
	{
		jeBoolean Ret;

		Ret = jeChain_RemoveLinkData(World->LightChain, Light);
		assert(Ret == JE_TRUE);

		return JE_FALSE;
	}
}

//========================================================================================
//	jeWorld_RemoveLight
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_RemoveLight(jeWorld *World, jeLight *Light, jeBoolean Update)
{
	uint32		Index{};
	jeLight* OldLight{};
	
	assert(World);
	assert(jeLight_IsValid(Light) == JE_TRUE);
	assert(jeChain_FindLink(World->LightChain, Light));

	Index = jeChain_LinkDataGetIndex(World->LightChain, Light);

	if (!jeChain_RemoveLinkData(World->LightChain, Light))
		return JE_FALSE;

	OldLight = (jeLight *)jeChain_GetLinkDataByIndex(World->OldLights, Index);
	assert(OldLight);

	if (!jeChain_RemoveLinkData(World->OldLights, OldLight))
		return JE_FALSE;

	if (Update)
	{
		jeChain_Link		*Link;

		// Update is set, so go through all the objects, and notify them of a light update
		for (Link = jeChain_GetFirstLink(World->Objects); Link; Link = jeChain_LinkGetNext(Link))
		{
			jeObject	*Object;
		
			Object = (jeObject*)jeChain_LinkGetLinkData(Link);

			jeObject_SendMessage (Object, JE_OBJECT_MSG_WORLD_REMOVE_SLIGHT_UPDATE, Light);
		}
	}

	jeLight_Destroy(&Light);
	jeLight_Destroy(&OldLight);

	return JE_FALSE;
}

//========================================================================================
//	jeWorld_UpdateLight
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_UpdateLight(jeWorld *World, jeLight *Light)
{
	if (!jeWorld_RemoveLight(World, Light, JE_TRUE))
		return JE_FALSE;

	if (!jeWorld_AddLight(World, Light, JE_TRUE))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_GetNextLight
//========================================================================================
JETAPI jeLight * JETCC jeWorld_GetNextLight(jeWorld *World, jeLight *Start)
{
	assert(World);

	return (jeLight *)jeChain_GetNextLinkData(World->LightChain, Start);
}

//========================================================================================
//	jeWorld_AddDLight
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_AddDLight(jeWorld *World, jeLight *Light)
{
	assert(World);
	assert(jeLight_IsValid(Light) == JE_TRUE);
	assert(jeChain_FindLink(World->DLightChain, Light) == nullptr);
	
	// Add the light to the worlds dlight chain
	if (!jeChain_AddLinkData(World->DLightChain, Light))
		return JE_FALSE;

	jeLight_CreateRef(Light);		// Ref the light

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_RemoveDLight
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_RemoveDLight(jeWorld *World, jeLight *Light)
{
	assert(World);
	assert(jeLight_IsValid(Light) == JE_TRUE);
	assert(jeChain_FindLink(World->DLightChain, Light));
	
	// Remove the light from the worlds dlight chain
	if (!jeChain_RemoveLinkData(World->DLightChain, Light))
		return JE_FALSE;

	jeLight_Destroy(&Light);		// De-Ref the light

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_GetNextDLight
//========================================================================================
JETAPI jeLight * JETCC jeWorld_GetNextDLight(jeWorld *World, jeLight *Start)
{
	assert(World);

	return (jeLight *)jeChain_GetNextLinkData(World->DLightChain, Start);
}

//========================================================================================
//	jeWorld_AddUserPoly
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_AddUserPoly(jeWorld *World, jeUserPoly *Poly, jeBoolean AutoRemove)
{
	assert(World);
	assert(Poly);

	if (AutoRemove)
	{
		assert(!jeChain_FindLink(World->AutoRemoveUserPolys, Poly));
		if (!jeChain_AddLinkData(World->AutoRemoveUserPolys, Poly))
			return JE_FALSE;
	}
	else
	{
		assert(!jeChain_FindLink(World->UserPolys, Poly));
		if (!jeChain_AddLinkData(World->UserPolys, Poly))
			return JE_FALSE;
	}
	
	if (!jeUserPoly_CreateRef(Poly))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_RemoveUserPoly
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_RemoveUserPoly(jeWorld *World, jeUserPoly *Poly)
{
	assert(World);
	assert(Poly);

	assert(jeChain_FindLink(World->UserPolys, Poly));

	if (!jeChain_RemoveLinkData(World->UserPolys, Poly))
		return JE_FALSE;

	jeUserPoly_Destroy(&Poly);		// Re-ref

	return JE_TRUE;
}

//========================================================================================
//	Objects
//========================================================================================

#ifdef FIRST_OBJECT_IN_HIERARCHY_IS_MODEL_HACK
	static jeObject		*HackModelObject;
#endif

//========================================================================================
//	jeWorld_AddObject
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_AddObject(jeWorld *World, jeObject *Object)
{
	jeBoolean bAdded{};
	assert(World);
	assert(Object);
	assert(!jeChain_FindLink(World->Objects, Object));

	bAdded = JE_FALSE;

#ifdef FIRST_OBJECT_IN_HIERARCHY_IS_MODEL_HACK
	if (HackModelObject)
	{
		// Attach world to object
		if( !jeObject_AttachWorld( Object, World ) )
			goto AO_ERROR;

		// Attach engine
		if( World->Engine != nullptr )
		{
			if( !jeObject_AttachEngine( Object, World->Engine ) )
				goto AO_ERROR;
		}

		// Attach sound system
		if( World->SoundSystem != nullptr )
			jeObject_AttachSoundSystem( Object, World->SoundSystem );

		return jeObject_AddChild(HackModelObject, Object);
	}
#endif
	// Attach world to object
	if( !jeObject_AttachWorld( Object, World ) )
	{
		goto AO_ERROR;
	}

	// Attach engine
	if( World->Engine != nullptr )
	{
		if( !jeObject_AttachEngine( Object, World->Engine ) )
			goto AO_ERROR;
	}

	// Attach sound system
	if( World->SoundSystem != nullptr )
	{
		jeObject_AttachSoundSystem( Object, World->SoundSystem );
	}
	
	// KROUER - Try to put actor into the BSP instead of the World
	{
		uint32 flags = jeObject_GetFlags(Object);
		jeObject_Type objectType = jeObject_GetType(Object);
		if (JE_OBJECT_TYPE_MODEL == objectType && World->Model == nullptr)
		{
			World->Model = Object;
			jeObject_CreateRef(Object);
		}
		if ((flags&JE_OBJECT_VISRENDER) && World->Model)
		{
			jeObject_AddChild(World->Model, Object);
		}
	}

	// add object to the objects list
	if (!jeChain_AddLinkData(World->Objects, Object))
	{
		return JE_FALSE;
	}
	
	// Ref the object
	jeObject_CreateRef( Object );		
	
#ifdef FIRST_OBJECT_IN_HIERARCHY_IS_MODEL_HACK
	HackModelObject = Object;
#endif

	return JE_TRUE;

AO_ERROR:
	jeChain_RemoveLinkData( World->Objects, Object );
	return( JE_FALSE );
}

//========================================================================================
//	jeWorld_HasObject
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_HasObject(const jeWorld *World, const jeObject *Object)
{
	assert(World);
	assert(Object);

	if (jeChain_FindLink(World->Objects, (void*)Object))
		return JE_TRUE;

	return JE_FALSE;
}

//========================================================================================
//	jeWorld_RemoveObject
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_RemoveObject(jeWorld *World, jeObject *Object)
{
	assert(World);
	assert(Object);

#ifdef FIRST_OBJECT_IN_HIERARCHY_IS_MODEL_HACK
	if (Object != HackModelObject)
		return jeObject_RemoveChild(HackModelObject, Object);
#endif

	jeObject_RemoveChild(World->Model, Object); 

	assert(jeChain_FindLink(World->Objects, Object));

	if (!jeChain_RemoveLinkData(World->Objects, Object))
		return JE_FALSE;

	if( World->Engine != nullptr )
		jeObject_DettachEngine(Object, World->Engine);

	jeObject_DettachWorld(Object, World);

	jeObject_Destroy(&Object);

#ifdef FIRST_OBJECT_IN_HIERARCHY_IS_MODEL_HACK
	HackModelObject = nullptr;
#endif

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_GetNextObject
//========================================================================================
JETAPI jeObject * JETCC jeWorld_GetNextObject(jeWorld *World, jeObject *Start)
{
	assert(World);

	return (jeObject *)jeChain_GetNextLinkData(World->Objects, Start);
}

//========================================================================================
//	jeWorld_FindObjectByDefName
//========================================================================================
JETAPI jeObject * JETCC jeWorld_FindObjectByDefName(jeWorld *World, const char *DefName)
{
	jeChain_Link		*Link{};

	assert(World);

	for (Link = jeChain_GetFirstLink(World->Objects); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeObject* Object{};

		Object = (jeObject*)jeChain_LinkGetLinkData(Link);

		if (!strcmp(Object->Methods->Name, DefName))
			return Object;		// Fount it
	}

	return nullptr;		// Not found
}

//========================================================================================
//	jeWorld_Frame
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_Frame(jeWorld *World, float TimeDelta)
{
	jeChain_Link		*Link{};

	assert(World);
	assert(World->Objects);


	for (Link = jeChain_GetFirstLink(World->Objects); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeObject* Object{};

		Object = (jeObject*)jeChain_LinkGetLinkData(Link);

		if (!jeObject_Frame( Object, TimeDelta ))
			return JE_FALSE;
	}

	// paradoxnj - Useless and incomplete
	//get shader objects and run a shader frame for each (CyRiuS)
	/*for (Link = jeChain_GetFirstLink(World->ShaderChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeShader	*Shader;

		Shader = (jeShader*)jeChain_LinkGetLinkData(Link);

		if (!jeShader_Frame( Shader, World, TimeDelta ))
			return JE_FALSE;
	}

	//retrieve each Actor Scriptlet and send it out to be processed
	for (Link = jeChain_GetFirstLink(World->ActorScriptChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeScript	*ActorScript;

		ActorScript = (jeScript*)jeChain_LinkGetLinkData(Link);

		if(!jeScript_Frame( ActorScript, World, TimeDelta ))
			return JE_FALSE;
	}*/

	return( JE_TRUE );
}

//========================================================================================
//	jeWorld_Collision
//	Returns JE_TRUE if there was a collision, JE_FALSE otherwise
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_Collision(	const jeWorld *World, 
										const jeExtBox *Box, 
										const jeVec3d *Front, 
										const jeVec3d *Back, 
										jeCollisionInfo *CollisionInfo)
{
	jeChain_Link	*Link{};
	jeFloat			BestDist{};
	jeBoolean		Hit;
	
	Hit = JE_FALSE;
	BestDist = 999999.0f;

	if (CollisionInfo)	// Invalidate the collision info structure	
		memset(CollisionInfo, 0, sizeof(*CollisionInfo));

	// Call each objects collision function
	for (Link = jeChain_GetFirstLink(World->Objects); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeObject		* Object{}, * SubObject{};
		jeVec3d			Impact{};
		jePlane			Plane{};
		const char		*ObjectType{};
	
		Object = (jeObject*)jeChain_LinkGetLinkData(Link);
		ObjectType = jeObject_GetTypeName(Object);
		if(jeWorld_CanCollide(World,ObjectType))
		{
			// Icestorm: Do we want to know further deatils?
			if (CollisionInfo)
			{
				if (jeObject_Collision(Object, Box, Front, Back, &Impact, &Plane, &SubObject))
				{
					jeFloat		Dist{};

					Dist = jeVec3d_DistanceBetween(Front, &Impact);

					// Record the closest collision point
					if (Dist < BestDist)
					{
						BestDist = Dist; // Added by Incarnadine
						CollisionInfo->Impact = Impact;
						CollisionInfo->Plane = Plane;
						CollisionInfo->Object = SubObject;
						CollisionInfo->IsValid = JE_TRUE;
						Hit = JE_TRUE;
					}
				}
			} else
				if (jeObject_Collision(Object, Box, Front, Back, nullptr, nullptr, &SubObject))
					return JE_TRUE;
		}
	}
	
	return Hit;
}

// Added by Icestorm
//========================================================================================
//	jeWorld_ChangeBoxCollision
//	Returns JE_TRUE if there was a collision, while Box changes, JE_FALSE otherwise
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_ChangeBoxCollision(	const jeWorld *World, 
												const jeVec3d *Pos, 
												const jeExtBox *FrontBox, 
												const jeExtBox *BackBox, 
												jeChangeBoxCollisionInfo *CollisionInfo)
{
	jeChain_Link	* Link{};
	jeFloat			BestDist{};
	jeBoolean		Hit{};
	
	Hit = JE_FALSE;
	BestDist = 999999.0f;

	// Invalidate the collision info structure
	if (CollisionInfo)	
		memset(CollisionInfo, 0, sizeof(*CollisionInfo));

	// Call each objects collision function
	for (Link = jeChain_GetFirstLink(World->Objects); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeObject* Object{}, * SubObject{};
		jeExtBox		ImpactBox{};
		jePlane			Plane{};
		const char* ObjectType{};
	
		Object = (jeObject*)jeChain_LinkGetLinkData(Link);
		ObjectType = jeObject_GetTypeName(Object);
		if(jeWorld_CanCollide(World,ObjectType))
		{
			// Icestorm: Do we want to know further deatils?
			if (CollisionInfo)
			{
				if (jeObject_ChangeBoxCollision(Object, Pos, FrontBox, BackBox, &ImpactBox, &Plane, &SubObject))
				{
					jeFloat		Dist{};

					Dist = jeVec3d_DistanceBetween(&FrontBox->Min, &ImpactBox.Min);
				
					// Record the closest collision point
					if (Dist < BestDist)
					{
						BestDist = Dist;
						CollisionInfo->ImpactBox = ImpactBox;
						CollisionInfo->Plane = Plane;
						CollisionInfo->Object = SubObject;
						Hit = JE_TRUE;
					}
				}
			} else
				if (jeObject_ChangeBoxCollision(Object, Pos, FrontBox, BackBox, nullptr, nullptr, &SubObject))
					return JE_TRUE;
		}
	}
	
	return Hit;
}

//#include "jePolyMgr.h"
//extern jePolyMgr		*HackPolyMgr;

//========================================================================================
//	jeWorld_RenderALL
//	Takes a camera space frustum
//========================================================================================
jeBoolean jeWorld_RenderALL(jeWorld *World, jeCamera *Camera, jeFrustum *CameraSpaceFrustum)
{
	jeChain_Link* Link{};
	jeFrustum				Frustum{}, WorldSpaceFrustum{};
	jeObject_RenderFlags	RenderFlags{};

//	#if (WORLD_DEBUG_OUTPUT_LEVEL >= 2)
//		OutputDebugString("BEGIN jeWorld_RenderALL\n");
//	#endif

	RenderFlags = 0;

	if (!CameraSpaceFrustum)
	{
		// Prepare the default frustum
		jeFrustum_SetFromCamera(&Frustum, Camera);

		RenderFlags |= JE_OBJECT_RENDER_FLAG_CAMERA_FRUSTUM;	// Frustum was created from camera

		CameraSpaceFrustum = &Frustum;
	}

	// Render objects
	for ( Link = jeChain_GetFirstLink( World->Objects ); Link; Link = jeChain_LinkGetNext( Link ) )
	{
		jeObject* Object{};

		Object = (jeObject*)jeChain_LinkGetLinkData(Link);

		if (Object->Methods->Type == JE_OBJECT_TYPE_PORTAL)
			continue;	// Don't render portals like normal objects

//		if (Object->Methods->Type == JE_OBJECT_TYPE_ACTOR)
//			continue;

		if (!jeObject_Render(Object, World, World->Engine, Camera, CameraSpaceFrustum, RenderFlags))
			return JE_FALSE;
	}

/*
	// Render actors
	for ( Link = jeChain_GetFirstLink( World->Actors ); Link; Link = jeChain_LinkGetNext( Link ) )
	{
		jeActor		*Actor;

		Actor = (jeActor*)jeChain_LinkGetLinkData(Link);

		//if (!jeActor_RenderThroughFrustum(Actor, World->Engine, World, Camera, CameraSpaceFrustum))
		//	return JE_FALSE;
		if (!jeActor_Render(Actor, World->Engine, World, Camera))
			return JE_FALSE;
	}
*/

	// User polys need frustum in world space
	jeFrustum_TransformToWorldSpace(CameraSpaceFrustum, Camera, &WorldSpaceFrustum);

	// Render user polys
	if (!jeWorld_RenderUserPolys(World, Camera, &WorldSpaceFrustum))
		return JE_FALSE;

	// paradoxnj - Useless and incomplete
	// Update world based on shader variables (cyrius)
	// AKA Render the shaders
	/*for ( Link = jeChain_GetFirstLink( World->ShaderChain ); Link; Link = jeChain_LinkGetNext( Link ) )
	{
		jeShader	*Shader;

		Shader = (jeShader*)jeChain_LinkGetLinkData(Link);

		// "render" the shader
		// this is different from jeShader_Frame. the Frame call uses the "rendered" info
		// that is retrieved/generated here

		if (!jeShader_Render(Shader, World, World->Engine, Camera, CameraSpaceFrustum))
			return JE_FALSE;
		
	}

	//render ActorScripts
	for ( Link = jeChain_GetFirstLink( World->ActorScriptChain ); Link; Link = jeChain_LinkGetNext( Link ) )
	{
		jeScript	*ActorScript;

		ActorScript = (jeScript*)jeChain_LinkGetLinkData(Link);

		// render the ActorScript
		// This is where the changes to the actor and camera are applied
		// The info used here is update in jeActorScript_Frame()

		if (!jeScript_Render(ActorScript, World, World->Engine, Camera, CameraSpaceFrustum))
			return JE_FALSE;
	}*/

//	#if (WORLD_DEBUG_OUTPUT_LEVEL >= 2)
//		OutputDebugString("END jeWorld_RenderALL\n");
//	#endif

	return JE_TRUE;
}

//========================================================================================
//	** LOCAL Static functions **
//========================================================================================

//========================================================================================
//	jeWorld_RenderUserPolys
//	Frustum is assumed to be in WorldSpace already!!!
//========================================================================================
static jeBoolean jeWorld_RenderUserPolys(const jeWorld *World, const jeCamera *Camera, const jeFrustum *WorldSpaceFrustum)
{
	jeChain_Link	* Link{};
	//jeChain_Link		*Link2;
	jeUserPoly		* Poly{};

	//assert(World);
	//assert(Camera);
	//assert(WorldSpaceFrustum);

//	#if (WORLD_DEBUG_OUTPUT_LEVEL >= 2)
//		OutputDebugString("BEGIN jeWorld_RenderUserPolys\n");
//	#endif

	// Render normal user polys first..or maybe not. Let's do it a bit faster shall we
	// Observe the legendary chinese super special tactic: DOUBLE RENDER PUUUNCH!...
	// yea riiiight....
	for (Link = jeChain_GetFirstLink(World->UserPolys); Link; Link = jeChain_LinkGetNext(Link))
	{
		Poly = (jeUserPoly*)jeChain_LinkGetLinkData(Link);
		//assert(Poly);

		if (Poly && !jeUserPoly_Render(Poly, World->Engine, Camera, WorldSpaceFrustum))
			return JE_FALSE;
	}

	// Render normal user polys first
	for (Link = jeChain_GetFirstLink(World->AutoRemoveUserPolys); Link; Link = jeChain_LinkGetNext(Link))
	{
		Poly = (jeUserPoly*)jeChain_LinkGetLinkData(Link);
		//assert(Poly);

		if (Poly && !jeUserPoly_Render(Poly, World->Engine, Camera, WorldSpaceFrustum))
			return JE_FALSE;
	}

//	#if (WORLD_DEBUG_OUTPUT_LEVEL >= 2)
//		OutputDebugString("END jeWorld_RenderUserPolys\n");
//	#endif

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_DestroyAutoRemoveUserPolys
//========================================================================================
static jeBoolean jeWorld_DestroyAutoRemoveUserPolys(jeWorld *World)
{
	jeChain_Link	* Link{};
	jeUserPoly		* Poly{};

	assert(World);

	// Keep getting links till there are no more
	while (Link = jeChain_GetFirstLink(World->AutoRemoveUserPolys))
	{
		Poly = (jeUserPoly*)jeChain_LinkGetLinkData(Link);
		assert(Poly);

		if (!jeChain_RemoveLink(World->AutoRemoveUserPolys, Link))
			return JE_FALSE;
		
		// Destroy the link
		jeChain_LinkDestroy(&Link);

		// De-ref the poly
		jeUserPoly_Destroy(&Poly);
	}

	assert(jeChain_GetLinkCount(World->AutoRemoveUserPolys) == 0);

	return JE_TRUE;
}

//========================================================================================
//	ReadPtrMgrVerification
//========================================================================================
static jeBoolean ReadPtrMgrVerification(jeVFile *VFile, const jePtrMgr *PtrMgr)
{
	int32		PtrCount1, PtrRefs1;
	int32		PtrCount2, PtrRefs2;

	jePtrMgr_GetPtrCount(PtrMgr, &PtrCount1);
	jePtrMgr_GetPtrRefs(PtrMgr, &PtrRefs1);

	if (!jeVFile_Read(VFile, &PtrCount2, sizeof(PtrCount2)))
		return JE_FALSE;

	if (PtrCount1 != PtrCount2)
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &PtrRefs2, sizeof(PtrRefs2)))
		return JE_FALSE;

	if (PtrRefs1 != PtrRefs2)
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	WritePtrMgrVerification
//========================================================================================
static jeBoolean WritePtrMgrVerification(jeVFile *VFile, const jePtrMgr *PtrMgr)
{
	int32		PtrCount, PtrRefs;

	jePtrMgr_GetPtrCount(PtrMgr, &PtrCount);
	jePtrMgr_GetPtrRefs(PtrMgr, &PtrRefs);

	if (!jeVFile_Write(VFile, &PtrCount, sizeof(PtrCount)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &PtrRefs, sizeof(PtrRefs)))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_CreateArrays
//========================================================================================
static jeBoolean jeWorld_CreateArrays(jeWorld *World)
{
	assert(World);

	// Create the faceinfo array
	World->FaceInfoArray = jeFaceInfo_ArrayCreate(JU_WORLD_START_FACEINFO);

	if (!World->FaceInfoArray)
	{
		jeErrorLog_AddString(-1, "jeWorld_CreateArrays:  jeFaceInfo_ArrayCreate failed.", nullptr);
		goto ExitWithError;
	}

	// Create the material array
	World->MaterialArray = jeMaterial_ArrayCreate(JU_WORLD_START_MATERIALS);

	if (!World->MaterialArray)
	{
		jeErrorLog_AddString(-1, "jeWorld_CreateArrays:  jeMaterial_ArrayCreate failed.", nullptr);
		goto ExitWithError;
	}

	return JE_TRUE;

	ExitWithError:
	{
		if (World->FaceInfoArray)
			jeFaceInfo_ArrayDestroy(&World->FaceInfoArray);
		if (World->MaterialArray)
			jeMaterial_ArrayDestroy(&World->MaterialArray);

		return JE_FALSE;
	}
}

#define FACEINFO_READWRITE_PORTALCAMERA		(1<<0)

//========================================================================================
//	ReadFaceInfo
//========================================================================================
static jeBoolean ReadFaceInfo(jeVFile *VFile, jeGArray_Element *Element, void *Context)
{
	jeFaceInfo		* pFaceInfo{};
	uint8			ReadWriteFlags;

	pFaceInfo = (jeFaceInfo*)Element;

	//if (!jeFaceInfo_Read(pFaceInfo, VFile))
	//	return JE_FALSE;

	if (!jeVFile_Read(VFile, &ReadWriteFlags, sizeof(ReadWriteFlags)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->Flags, sizeof(pFaceInfo->Flags)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->Alpha, sizeof(pFaceInfo->Alpha)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->Rotate, sizeof(pFaceInfo->Rotate)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->ShiftU, sizeof(pFaceInfo->ShiftU)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->ShiftV, sizeof(pFaceInfo->ShiftV)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->DrawScaleU, sizeof(pFaceInfo->DrawScaleU)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->DrawScaleV, sizeof(pFaceInfo->DrawScaleV)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->LMapScaleU, sizeof(pFaceInfo->LMapScaleU)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->LMapScaleV, sizeof(pFaceInfo->LMapScaleV)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &pFaceInfo->MaterialIndex, sizeof(pFaceInfo->MaterialIndex)))
		return JE_FALSE;

	if (ReadWriteFlags & FACEINFO_READWRITE_PORTALCAMERA)
	{
		pFaceInfo->PortalCamera = jeObject_CreateFromFile(VFile, (jePtrMgr *)Context);
		if (!pFaceInfo->PortalCamera)
			return JE_FALSE;
	}
	else
		pFaceInfo->PortalCamera = nullptr;

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_ReadArrays
//========================================================================================
static jeBoolean jeWorld_ReadArrays(jeWorld *World, jeVFile *VFile, jePtrMgr* PtrMgr)
{
	assert(World);

	// Create the faceinfo array
	World->FaceInfoArray = jeFaceInfo_ArrayCreateFromFile(VFile, ReadFaceInfo, PtrMgr, PtrMgr);

	if (!World->FaceInfoArray)
	{
		jeErrorLog_AddString(-1, "jeWorld_CreateArrays:  jeFaceInfo_ArrayCreateFromFile failed.", nullptr);
		goto ExitWithError;
	}

	// Create the material array
	World->MaterialArray = jeMaterial_ArrayCreateFromFile(VFile, PtrMgr);

	if (!World->MaterialArray)
	{
		jeErrorLog_AddString(-1, "jeWorld_CreateArrays:  jeMaterial_ArrayCreateFromFile failed.", nullptr);
		goto ExitWithError;
	}

	return JE_TRUE;

	ExitWithError:
	{
		if (World->FaceInfoArray)
			jeFaceInfo_ArrayDestroy(&World->FaceInfoArray);
		if (World->MaterialArray)
			jeMaterial_ArrayDestroy(&World->MaterialArray);

		return JE_FALSE;
	}
}



#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
		((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

#define JU_WORLD_TAG				MAKEFOURCC('G', 'E', 'W', 'F')		// 'GE' 'W'orld 'F'ile
#define JU_WORLD_VERSION			0x0000

//========================================================================================
//	jeWorld_WriteHeader
//========================================================================================
static jeBoolean jeWorld_WriteHeader(const jeWorld *World, jeVFile *VFile)
{
	uint32		Tag;
	uint16		Version;

	assert(World);
	assert(VFile);

	// Write TAG
	Tag = JU_WORLD_TAG;

	if (!jeVFile_Write(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	// Write version
	Version = JU_WORLD_VERSION;

	if (!jeVFile_Write(VFile, &Version, sizeof(Version)))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_ReadHeader
//========================================================================================
static jeBoolean jeWorld_ReadHeader(jeWorld *World, jeVFile *VFile)
{
	uint32		Tag;
	uint16		Version;

	assert(World);
	assert(VFile);

	if (!jeVFile_Read(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	if (Tag != JU_WORLD_TAG)
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &Version, sizeof(Version)))
		return JE_FALSE;

	if (Version != JU_WORLD_VERSION)
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	WriteFaceInfo
//========================================================================================
static jeBoolean WriteFaceInfo(jeVFile *VFile, jeGArray_Element *Element, void *Context)
{
	jeFaceInfo		* pFaceInfo{};
	uint8			ReadWriteFlags;

	pFaceInfo = (jeFaceInfo*)Element;

	ReadWriteFlags = 0;

	if (pFaceInfo->PortalCamera)
		ReadWriteFlags |= FACEINFO_READWRITE_PORTALCAMERA;

	if (!jeVFile_Write(VFile, &ReadWriteFlags, sizeof(ReadWriteFlags)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->Flags, sizeof(pFaceInfo->Flags)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->Alpha, sizeof(pFaceInfo->Alpha)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->Rotate, sizeof(pFaceInfo->Rotate)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->ShiftU, sizeof(pFaceInfo->ShiftU)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->ShiftV, sizeof(pFaceInfo->ShiftV)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->DrawScaleU, sizeof(pFaceInfo->DrawScaleU)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->DrawScaleV, sizeof(pFaceInfo->DrawScaleV)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->LMapScaleU, sizeof(pFaceInfo->LMapScaleU)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->LMapScaleV, sizeof(pFaceInfo->LMapScaleV)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &pFaceInfo->MaterialIndex, sizeof(pFaceInfo->MaterialIndex)))
		return JE_FALSE;

	if (pFaceInfo->PortalCamera)
	{
		if (!jeObject_WriteToFile(pFaceInfo->PortalCamera, VFile, (jePtrMgr *)Context))
			return JE_FALSE;
	}

	return JE_TRUE;
}


//========================================================================================
//	jeWorld_WriteArrays
//========================================================================================
static jeBoolean jeWorld_WriteArrays(const jeWorld *World, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	assert(World);
	assert(VFile);

	if (!jeFaceInfo_ArrayWriteToFile(World->FaceInfoArray, VFile, WriteFaceInfo, PtrMgr, PtrMgr))
		return JE_FALSE;

	if (!jeMaterial_ArrayWriteToFile(World->MaterialArray, VFile, PtrMgr))
		return JE_FALSE;

	return JE_TRUE;
}


//========================================================================================
//	ReadLight
//========================================================================================
static jeBoolean ReadLight(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr)
{
	*LinkData = jeLight_CreateFromFile(VFile, PtrMgr);
		
	if (!(*LinkData))
		return JE_FALSE;

	/*if (!jeLight_CreateRef(*LinkData))		// Ref it [MLB-ICE] : Object was already refd. in CreateFromFile
		return JE_FALSE;*/

	return JE_TRUE;
}


//========================================================================================
//	WriteLight
//========================================================================================
static jeBoolean WriteLight(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr)
{
	if (!jeLight_WriteToFile((jeLight *)*LinkData, VFile, PtrMgr))
		return JE_FALSE;

	return JE_TRUE;
}


//========================================================================================
//	ReadObject
//========================================================================================
static jeBoolean ReadObject(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr)
{
	jeObject	* Object{};
	jeWorld		* World{};

	World = (jeWorld*)Context;
	assert(World);

	Object = jeObject_CreateFromFile(VFile, PtrMgr);
		
	if (!Object)
		return JE_FALSE;

	
	//jeObject_CreateRef(Object);		// Ref it  [MLB-ICE] : Object was already refd. in CreateFromFile

	// Attach world to object
	if( !jeObject_AttachWorld(Object, World) )
		goto RO_ERROR;

	// Attach engine
	if(World->Engine)
	{
		if( !jeObject_AttachEngine(Object, World->Engine) )
		{
			jeObject_DettachWorld( Object, World );
			goto RO_ERROR;
		}
	}

	// Attach sound system
	if(World->SoundSystem)
		jeObject_AttachSoundSystem(Object, World->SoundSystem);

	// KROUER - Try to put actor into the BSP instead of the World
	{
		uint32 flags = jeObject_GetFlags(Object);
		jeObject_Type objectType = jeObject_GetType(Object);
		if (JE_OBJECT_TYPE_MODEL == objectType && World->Model == nullptr)
		{
			World->Model = Object;
			jeObject_CreateRef(Object);
		}
		if ((flags&JE_OBJECT_VISRENDER) && World->Model)
		{
			jeObject_AddChild(World->Model, Object);
		}
	}

	*LinkData = Object;

	return JE_TRUE;
RO_ERROR:
	jeObject_Destroy( &Object );
	return JE_FALSE;
}



//========================================================================================
//	WriteObject
//========================================================================================
static jeBoolean WriteObject(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr)
{
	if (!jeObject_WriteToFile((jeObject *)*LinkData, VFile, PtrMgr))
		return JE_FALSE;

	return JE_TRUE;
}


//========================================================================================
//	jeWorld_AttachSoundSystem
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_AttachSoundSystem(jeWorld *World, jeSound_System * pSoundSystem )
{
	jeChain_Link	* Link{};

	assert( World );

	World->SoundSystem = pSoundSystem;
	for (Link = jeChain_GetFirstLink(World->Objects); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeObject	* Object{};

		Object = (jeObject*)jeChain_LinkGetLinkData(Link);

		
		if (!jeObject_AttachSoundSystem( Object, pSoundSystem ))
		{
			jeErrorLog_AddString(-1, "jeWorld_AttachSoundSystem:  jeObject_AttachSoundSystem failed.", nullptr);
			return JE_FALSE;
		}
	}
	return( JE_TRUE );
}

//========================================================================================
//	jeWorld_GetSoundSystem
//========================================================================================
JETAPI jeSound_System *	JETCC jeWorld_GetSoundSystem(jeWorld *World )
{
	return( World->SoundSystem );
}

//========================================================================================
//	jeWorld_GetResourceMgr
//========================================================================================
JETAPI jeResourceMgr * JETCC jeWorld_GetResourceMgr(jeWorld *World)
{
	assert( World != nullptr );
	assert( World->ResourceMgr != nullptr );

	jeResource_MgrIncRefcount( World->ResourceMgr );
	return( World->ResourceMgr );
}

//========================================================================================
//	TEST PORTAL OBJECT CODE
//========================================================================================

enum
{
	PORTAL_SKYBOX_CHECK_ID = PROPERTY_LOCAL_DATATYPE_START,
	PORTAL_SPEED_ID,
	PORTAL_RADIOX_ID,
	PORTAL_RADIOY_ID,
	PORTAL_RADIOZ_ID,
};

typedef struct
{
	jePortal		*Portal;

	jeBoolean		SkyBox;
	jeFloat			RotateSpeed;
	jeFloat			Rotation;
	int32			RAxis;

	int32			RefCount;
}  jeWorld_Portal;

static void * JETCC CreatePortalObjectInstance(void)
{
	jeWorld_Portal		* WPortal{};
	jePortal			* Portal{};

	WPortal = JE_RAM_ALLOCATE_STRUCT(jeWorld_Portal);

	if (!WPortal)
		return nullptr;

	ZeroMem(WPortal);

	WPortal->RefCount = 1;

	Portal = jePortal_Create();

	if (!Portal)
	{
		jeRam_Free(WPortal);
		return nullptr;
	}

	Portal->Recursion = 0;

	WPortal->Portal = Portal;
	WPortal->RAxis = 1;

	return WPortal;
}

static void JETCC RefPortalObjectInstance(jeWorld_Portal *WPortal)
{
	assert(WPortal);

	assert(WPortal->RefCount > 0);

	WPortal->RefCount++;
}

static jeBoolean JETCC DestroyPortalObjectInstance(void **WPortal)
{
	jeWorld_Portal	* WPortal2{};

	WPortal2 = *(jeWorld_Portal **)WPortal;

	WPortal2->RefCount--;

	if (WPortal2->RefCount > 0)
		return JE_FALSE;

	jePortal_Destroy(&WPortal2->Portal);

	jeRam_Free(WPortal2);
	*WPortal = nullptr;

	return JE_TRUE;
}

static jeBoolean JETCC RenderPortalObjectInstance(const jeWorld_Portal *WPortal, const jeWorld *World, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags)
{
	return JE_TRUE;
}

jeBoolean JETCC GetPortalPropertyList(jeWorld_Portal *WPortal, jeProperty_List **List)
{
	jeProperty_List		* PropertyList{};
	jeProperty			* Property{};
	jePortal			* Portal{};

	Portal = WPortal->Portal;

#if 1
	PropertyList = jeProperty_ListCreate(1);

	Property = &PropertyList->pjeProperty[0];
	jeProperty_FillCheck(Property, "SkyBox", WPortal->SkyBox, PORTAL_SKYBOX_CHECK_ID);
#else
	PropertyList = jeProperty_ListCreate(5);

	Property = &PropertyList->pjeProperty[0];
	jeProperty_FillCheck(Property, "SkyBox", WPortal->SkyBox, PORTAL_SKYBOX_CHECK_ID);

	Property = &PropertyList->pjeProperty[1];
	jeProperty_FillFloat(Property, "Speed", WPortal->RotateSpeed, PORTAL_SPEED_ID, 0.0f, 100.0f, 1.0f);

	Property = &PropertyList->pjeProperty[2];
	jeProperty_FillRadio(Property, "X Axis", (WPortal->RAxis == 0), PORTAL_RADIOX_ID);

	Property = &PropertyList->pjeProperty[3];
	jeProperty_FillRadio(Property, "Y Axis", (WPortal->RAxis == 1), PORTAL_RADIOY_ID);

	Property = &PropertyList->pjeProperty[4];
	jeProperty_FillRadio(Property, "Z Axis", (WPortal->RAxis == 2), PORTAL_RADIOZ_ID);
#endif

	*List = PropertyList;

	return JE_TRUE;
}

jeBoolean JETCC SetPortalProperty(jeWorld_Portal *WPortal, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	switch (FieldID)
	{
		case PORTAL_SKYBOX_CHECK_ID:
		{
			assert(DataType == PROPERTY_CHECK_TYPE);
			WPortal->SkyBox = *(int32*)pData;
			break;
		}

		case PORTAL_SPEED_ID:
		{
			assert(DataType == PROPERTY_FLOAT_TYPE);
			WPortal->RotateSpeed = *(float*)pData;
			break;
		}

		case PORTAL_RADIOX_ID:
		{
			assert(DataType == PROPERTY_RADIO_TYPE);
			WPortal->RAxis = 0;
			break;
		}

		case PORTAL_RADIOY_ID:
		{
			assert(DataType == PROPERTY_RADIO_TYPE);
			WPortal->RAxis = 1;
			break;
		}

		case PORTAL_RADIOZ_ID:
		{
			assert(DataType == PROPERTY_RADIO_TYPE);
			WPortal->RAxis = 2;
			break;
		}
	}

	return JE_TRUE;
}

jeBoolean JETCC SetPortalXForm(jeWorld_Portal *WPortal, const jeXForm3d *XForm)
{
	jePortal	* Portal{};

	Portal = WPortal->Portal;

	Portal->XForm = *XForm;

	Portal->XForm.Flags = XFORM3D_NONORTHOGONALISOK;
	jeXForm3d_Orthonormalize(&Portal->XForm);

	return JE_TRUE;
}

jeBoolean JETCC GetPortalXForm(const jeWorld_Portal *WPortal, jeXForm3d *XForm)
{
	jePortal	* Portal{};

	Portal = WPortal->Portal;

	*XForm = Portal->XForm;

	return JE_TRUE;
}

jeBoolean JETCC GetPortalExtBox	(const jeWorld_Portal *WPortal, jeExtBox *BBox)
{
	jeExtBox	Box{};
	jeVec3d		* Pos{};
	jePortal	* Portal{};

	Portal = WPortal->Portal;

	Pos = &((jePortal*)Portal)->XForm.Translation;

	jeVec3d_Set(&Box.Min, Pos->X-10.0f, Pos->Y-10.0f, Pos->Z-10.0f);
	jeVec3d_Set(&Box.Max, Pos->X+10.0f, Pos->Y+10.0f, Pos->Z+10.0f);

	*BBox = Box;

	return JE_TRUE;
}


void * JETCC ReadPortalObjectInstance(jeVFile *VFile, jePtrMgr *PtrMgr)
{
	jeWorld_Portal	* WPortal{};

	WPortal = (jeWorld_Portal *)CreatePortalObjectInstance();

	if (!WPortal)
		return nullptr;

	if (!jeVFile_Read(VFile, &WPortal->Portal->XForm, sizeof(WPortal->Portal->XForm)))
	{
		DestroyPortalObjectInstance((void **)&WPortal);
		return nullptr;
	}

	if (!jeVFile_Read(VFile, &WPortal->SkyBox, sizeof(WPortal->SkyBox)))
	{
		DestroyPortalObjectInstance((void **)&WPortal);
		return nullptr;
	}

	return WPortal;
}


jeBoolean JETCC WritePortalObjectInstance(const void *Instance, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	jeWorld_Portal	* WPortal{};

	WPortal = (jeWorld_Portal*)Instance;

	if (!jeVFile_Write(VFile, &WPortal->Portal->XForm, sizeof(WPortal->Portal->XForm)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &WPortal->SkyBox, sizeof(WPortal->SkyBox)))
		return JE_FALSE;

	return JE_TRUE;
}


int	JETCC GetPortalXFormModFlags(const jeWorld_Portal *WPortal)
{
	return JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_ROTATE;
}

static jeBoolean RenderPortalObjectInstance2(jeWorld_Portal *WPortal, jePortal *Portal, const jePlane *Plane, const jeXForm3d *FaceXForm, jeWorld *World, jeCamera *Camera, jeFrustum *Frustum)
{
	jeXForm3d		XForm{}, InvXForm{}, NewXForm{};
	jeBoolean		Ret{};
	jeFloat			ZScale{};

	if (Portal->Recursion > 0)
		return JE_TRUE;

	// Get Camera XForm
	jeCamera_GetXForm(Camera, &XForm);

	if (WPortal->SkyBox)
	{
		// Skymode
		NewXForm = XForm;

		jeVec3d_Clear(&NewXForm.Translation);
		jeXForm3d_Multiply(&Portal->XForm, &NewXForm, &NewXForm);

		#if 0													  
		if (WPortal->Rotation)
		{
			switch (WPortal->RAxis)
			{
				case 0:
					jeXForm3d_RotateX(&NewXForm, WPortal->Rotation);
					break;

				case 1:
					jeXForm3d_RotateY(&NewXForm, WPortal->Rotation);
					break;

				case 2:
					jeXForm3d_RotateZ(&NewXForm, WPortal->Rotation);
					break;
			}
		}
		#endif

		NewXForm.Translation = Portal->XForm.Translation;

		ZScale = jeCamera_GetZScale(Camera);
		jeCamera_SetZScale(Camera, ZScale*20.0f);
	}
	else
	{
		jePlane		FrontPlane{};
		jeXForm3d	WorldToCameraXForm{};

		//	XForm the Camera to the Dest Portal location
		// This is the equation we need to XForm the camera from the FaceXForm to the PortalXForm
		//	P' = PortalXForm*InvFaceXForm*CameraXForm
		//	What this does, is take the point, and XForm against the camera like normal, then XForm by the amount
		//	it would take to get the FaceXForm to line up with the origin, then XForm into the Portal XForm...
		jeXForm3d_GetTranspose(FaceXForm, &InvXForm);

		jeXForm3d_Multiply(&Portal->XForm, &InvXForm, &NewXForm);
		jeXForm3d_Multiply(&NewXForm, &XForm, &NewXForm);
		
		// Get the WorldToCameraXForm
		jeCamera_GetTransposeXForm(Camera, &WorldToCameraXForm);

		// Transform the FacePlane to camera space
		jePlane_Transform(Plane, &WorldToCameraXForm, &FrontPlane);

		// Add the Plane to the Frustum
		if (!jeFrustum_AddPlane(Frustum, &FrontPlane, JE_TRUE))
			return JE_FALSE;
	}

	// Put the new XForm into the camera
	jeCamera_SetXForm((jeCamera*)Camera, &NewXForm);

	Portal->Recursion++;

	// Render the scene from this camera
	Ret = jeWorld_Render(World, Camera, Frustum);

	Portal->Recursion--;

	// Restore Camera XForm
	jeCamera_SetXForm((jeCamera*)Camera, &XForm);

	if (WPortal->SkyBox)
		jeCamera_SetZScale(Camera, ZScale);

	return Ret;
}

static jeBoolean JETCC SendPortalMessage(jeWorld_Portal *WPortal, int32 Msg, void *Data)
{
	switch (Msg)
	{
		case 0:
		{
			PortalMsgData	* MData{};
			
			MData = (PortalMsgData*)Data;

			return RenderPortalObjectInstance2(	WPortal, 
												WPortal->Portal, 
												MData->Plane, 
												MData->FaceXForm, 
												MData->World, 
												MData->Camera, 
												MData->Frustum);
		}

		default:
			return JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean JETCC PortalFrame(jeWorld_Portal *WPortal, jeFloat Time)
{
	WPortal->Rotation += WPortal->RotateSpeed*0.01f;

	return JE_TRUE;
}

// Icestorm: This seems already done in Portals...
/*jeObjectDef PortalObjectDef = 
{
	JE_OBJECT_TYPE_PORTAL,
	"Portal",
	0,

	CreatePortalObjectInstance,
	RefPortalObjectInstance,
	DestroyPortalObjectInstance,

	nullptr,
	nullptr,

	nullptr,
	nullptr,

	nullptr,
	nullptr,

	RenderPortalObjectInstance,

	nullptr,

	GetPortalExtBox,

	ReadPortalObjectInstance,
	WritePortalObjectInstance,

	GetPortalPropertyList,
	SetPortalProperty,
	nullptr,

	SetPortalXForm,			// SetXForm
	GetPortalXForm,			// Get XForm

	GetPortalXFormModFlags,
	
	nullptr,
	nullptr,
	nullptr,

	nullptr,
	SendPortalMessage,
	PortalFrame,
	nullptr
};*/

// Added by Incarnadine
//  -- Does this need to have additional logic to handle child objects?
JETAPI jeBoolean JETCC jeWorld_RebuildBSP(jeWorld *World, 
										jeBSP_Options Options, 
										jeBSP_Logic Logic, 
										jeBSP_LogicBalance LogicBalance)
{
	jeBoolean bResult = JE_TRUE;
	jeBSPSetup BSPSetup{};
	jeObject *pCurrentObj = jeWorld_GetNextObject(World, nullptr);
		
	// Find all models
	while(pCurrentObj)
	{
		if(strcmp(jeObject_GetTypeName(pCurrentObj),"Model")==0)
		{
			// Send a message to the model to tell it to rebuild the BSP
			BSPSetup.Options = Options;
			BSPSetup.Logic = Logic;
			BSPSetup.LogicBalance = LogicBalance;
			if(!jeObject_SendMessage(pCurrentObj, JE_OBJECT_MSG_WORLD_REBUILDBSP, &BSPSetup))
				bResult = JE_FALSE;
		}
		pCurrentObj = jeWorld_GetNextObject(World,pCurrentObj);
	}
	return bResult;
}

// Added by Incarnadine
//  -- Does this need to have additional logic to handle child objects?
JETAPI jeBoolean	JETCC  jeWorld_RebuildLights(jeWorld *World)
{
	jeBoolean bResult = JE_TRUE;
	jeObject *pCurrentObj = jeWorld_GetNextObject(World, nullptr);
		
	// Find all models
	while(pCurrentObj)
	{
		if(strcmp(jeObject_GetTypeName(pCurrentObj),"Model")==0)
		{
			// Send a message to the model to tell it to rebuild lights
			if(!jeObject_SendMessage(pCurrentObj, JE_OBJECT_MSG_WORLD_REBUILDLIGHTS, nullptr))
				bResult = JE_FALSE;
		}
		pCurrentObj = jeWorld_GetNextObject(World,pCurrentObj);	
	}
	return bResult;
}

// paradoxnj - Useless and incomplete
//========================================================================================
//	jeWorld_AddShader - By CyRiuS
//========================================================================================
/*JETAPI jeBoolean JETCC jeWorld_AddShader(jeWorld *World, jeShader *Shader)
{
	assert(World);
	assert(jeChain_FindLink(World->ShaderChain, Shader) == nullptr); //make sure shader doesnt exist in chain already
		
	// Add the shader to the world's shader chain
	if (!jeChain_AddLinkData(World->ShaderChain, Shader))
		return JE_FALSE;

	//I dont think a ref for the shader is needed... might change though
	//jeShader_CreateRef(Shader);		// Ref the shader

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_AddScript - By CyRiuS
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_AddScript(jeWorld *World, jeScript *Script)
{
	assert(World);
	
	//note: scripts can be added to the chain more than once
			
	// Add the script to the world's chain
	if (!jeChain_AddLinkData(World->ActorScriptChain, Script))
		return JE_FALSE;

	return JE_TRUE;
}
*/
// ----------------------------------------------
//  ACTOR SUPPORT FUNCTIONS
// Added by Incarnadine
/*
//========================================================================================
//	jeWorld_AddActor - By Incarnadine
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_AddActor(jeWorld *World, jeActor *Actor)
{
	assert(World);
	assert(jeActor_IsValid(Actor) == JE_TRUE);
	assert(jeChain_FindLink(World->Actors, Actor) == nullptr);
	
	// Add the actor to the world's actor chain
	if (!jeChain_AddLinkData(World->Actors, Actor))
		return JE_FALSE;

	jeActor_CreateRef(Actor);		// Ref the actor

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_RemoveActor - By Incarnadine
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_RemoveActor(jeWorld *World, jeActor *Actor)
{
	assert(World);
	assert(jeActor_IsValid(Actor) == JE_TRUE);
	assert(jeChain_FindLink(World->Actors, Actor));
	
	// Remove the actor from the world's actor chain
	if (!jeChain_RemoveLinkData(World->Actors, Actor))
		return JE_FALSE;

	jeActor_Destroy(&Actor);		// De-ref the actor

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_HasActor - By Incarnadine
//========================================================================================
JETAPI jeBoolean	JETCC jeWorld_HasActor(jeWorld *World, jeActor *Actor)
{
	assert(World);

	if(jeChain_FindLink(World->Actors, Actor))
		return JE_TRUE;

	return JE_FALSE;
}

//========================================================================================
//	jeWorld_GetNextActor - By Incarnadine
//========================================================================================
JETAPI jeActor * JETCC jeWorld_GetNextActor(const jeWorld *World, jeActor *Start)
{
	assert(World);

	return jeChain_GetNextLinkData(World->Actors, Start);
}
*/

//========================================================================================
//	jeWorld_CanCollide - By Incarnadine
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_CanCollide(const jeWorld *World, const char* Type)
{
	jeChain_Link	* Link{};

	assert(World);
	assert(Type);
	
	for (Link = jeChain_GetFirstLink(World->CollisionObjectTypes); Link; Link = jeChain_LinkGetNext(Link))
	{
		char	* Data{};
	
		Data = (char*)jeChain_LinkGetLinkData(Link);
		if(stricmp(Data,Type) == 0)
			return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	jeWorld_EnableCollision - By Incarnadine
//========================================================================================
JETAPI jeBoolean	JETCC jeWorld_EnableCollision(jeWorld *World, const char* Type)
{
	jeChain_Link	* Link{};

	assert(World);
	assert(Type);
	
	if(jeWorld_CanCollide(World,Type)) return JE_TRUE;

	for (Link = jeChain_GetFirstLink(World->CollisionObjectTypes); Link; Link = jeChain_LinkGetNext(Link))
	{
		char	* Data{};
	
		Data = (char*)jeChain_LinkGetLinkData(Link);
		if(stricmp(Data,Type) == 0)
		{
			jeRam_Free(Data);
			return jeChain_RemoveLink(World->CollisionObjectTypes, Link);			
		}
	}

	return JE_FALSE;
}

//========================================================================================
//	jeWorld_DisableCollision - By Incarnadine
//========================================================================================
JETAPI jeBoolean JETCC jeWorld_DisableCollision(jeWorld *World, const char *Type)
{
	assert(World);
	assert(Type);	
	
	if(!jeWorld_CanCollide(World,Type)) return JE_TRUE;

	// Add the actor to the world's actor chain
	if (!jeChain_AddLinkData(World->CollisionObjectTypes, Util_StrDup(Type)))
		return JE_FALSE;	

	return JE_TRUE;
}


//========================================================================================
//	jeWorld_SetCollisionOptions - By Incarnadine
//========================================================================================
/*JETAPI void JETCC jeWorld_SetCollisionOptions(jeWorld* World, int32 Level, const char *Include, const char *Exclude)
{
	char *Parse;
	char seps[] = ", ";
	char *token;

	World->CollisionLevel = Level;		

	if(Include != nullptr)
	{		
		jeChain_Link *Link;		

		Parse = strdup(Include);
		token = strtok(Parse,seps);
	
		for (Link = jeChain_GetFirstLink(World->CollisionObjectTypes); Link; Link = jeChain_LinkGetNext(Link))
		{
			char		*Data;		
		
			Data = (char*)jeChain_LinkGetLinkData(Link);
			jeRam_Free(Data);
			jeChain_RemoveLink(World->CollisionObjectTypes, Link);					
		}

		while(token != nullptr)
		{
			jeWorld_AddCollisionObjectType(World,token);
			token = strtok(nullptr,seps);
		}

		free(Parse);
	}
	if(Exclude != nullptr)
	{	
		Parse = strdup(Exclude);
		token = strtok(Parse,seps);
		while(token != nullptr)
		{
			jeWorld_RemoveCollisionObjectType(World,token);
			token = strtok(nullptr,seps);
		}
		free(Parse);
	}
}
*/
JETAPI int32 JETCC jeWorld_GetCollisionLevel(const jeWorld* World)
{
	return World->CollisionLevel;
}

JETAPI void JETCC jeWorld_SetCollisionLevel(jeWorld* World, int32 Level)
{
	World->CollisionLevel = Level;
}

//========================================================================================
//	jeWorld_GetNextCollisionExclusion - By Incarnadine
//========================================================================================
JETAPI char * JETCC jeWorld_GetNextCollisionExclusion(const jeWorld *World, const char *Start)
{
	jeChain_Link	* Link{};

	assert(World);

	if(Start == nullptr)
	{
		Link = jeChain_GetFirstLink(World->CollisionObjectTypes);
		if(Link == nullptr) return nullptr;
		return((char *)jeChain_LinkGetLinkData(Link));		
	}

	for (Link = jeChain_GetFirstLink(World->CollisionObjectTypes); Link; Link = jeChain_LinkGetNext(Link))
	{
		// locals
		char	* LinkName{};
			
		LinkName = (char *)jeChain_LinkGetLinkData( Link );		
		if(LinkName && stricmp(Start,LinkName)==0)
		{
			if(jeChain_LinkGetNext(Link) == nullptr) return nullptr;			

			return((char *)jeChain_LinkGetLinkData(jeChain_LinkGetNext(Link)));
		}
	}
	return nullptr;
}

// =====================
// Incarnadine
// -------------------
// This function simplifies the loading process for a level.  Pass it the filename of a 
// level created in JEdit and it will automatically open the appropriate, file and fork, and 
// then read in the world.  The second two parameters here CAN be nullptr.  If they are nullptr,
// they will be created automatically.
JETAPI jeWorld	* JETCC jeWorld_CreateFromEditorFile(const char* FileName, jePtrMgr *pPtrMgr, jeResourceMgr * pResourceMgr )
{
	jeWorld* pWorld{};
	jeVFile* pMapFile{};
	jeVFile* pWorldFork{};
	jeBoolean bCreated = JE_FALSE;
	
	if(pPtrMgr == nullptr)	
		pPtrMgr = jePtrMgr_Create();
	else jePtrMgr_CreateRef(pPtrMgr);

	if(pPtrMgr == nullptr )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "OnSaveDocument:jePtrMgr_Create");		
		return nullptr;
	}
	
	if(pResourceMgr == nullptr) 
	{
		bCreated = JE_TRUE;
		pResourceMgr = jeResource_MgrCreateDefault(nullptr);	
	}
	else jeResource_MgrIncRefcount(pResourceMgr);

	if(pResourceMgr == nullptr)
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "OnSaveDocument:jeResource_MgrCreateDefault");
		jePtrMgr_Destroy( &pPtrMgr );		
		return nullptr;
	}
	
	pMapFile = jeVFile_OpenNewSystem
	(
		nullptr, 
		JE_VFILE_TYPE_VIRTUAL,
		FileName,
		nullptr,
		JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY
	);  
	if( pMapFile == nullptr )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "OnOpenDocument:jeVFile_OpenNewSystem", FileName);		
		jePtrMgr_Destroy( &pPtrMgr );
		jeResource_MgrDestroy(&pResourceMgr);
		return nullptr;
	}

	// Open the Jet3D Fork
	pWorldFork = jeVFile_Open( pMapFile, "Jet3D", JE_VFILE_OPEN_READONLY) ;
	if( pWorldFork == nullptr )
	{
		jeVFile_Close( pMapFile ) ;
		jeErrorLog_AddString( JE_ERR_FILEIO_FORMAT, "OnOpenDocument:jeVFile_Open", FileName);		
		jePtrMgr_Destroy( &pPtrMgr );
		jeResource_MgrDestroy(&pResourceMgr);
		return nullptr;
	}		

	pWorld = jeWorld_CreateFromFile( pWorldFork, pPtrMgr, pResourceMgr );

	jeVFile_Close( pWorldFork ) ;
	if( pWorld == nullptr )
	{
		jeVFile_Close( pMapFile ) ;
		jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "OnOpenDocument:jeWorld_CreateFromFile", FileName);		
		jePtrMgr_Destroy( &pPtrMgr );
		jeResource_MgrDestroy(&pResourceMgr);
		return nullptr;
	}

	jeVFile_Close(pMapFile);			
	jePtrMgr_Destroy( &pPtrMgr );

	// pWorld uses the resource manager, so only destroy
	// (decrease refcount) if we didn't create the manager.
	if(!bCreated) jeResource_MgrDestroy(&pResourceMgr);

	return pWorld;
}
