/****************************************************************************************/
/*  JUWORLD.H                                                                           */
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

#ifndef JUWORLD_H
#define JUWORLD_H

#include "juModel.h"  // Added by Incarnadine

#include "jeFaceInfo.h"		// jeFaceInfo_Array
#include "jeMaterial.h"		// jeMaterial_Array
#include "BaseType.h"
#include "VFile.h"
#include "jeLight.h"
#include "camera.h"
#include "jePtrMgr.h"
#include "jeUserPoly.h"
#include "object.h"
#include "Sound.h"
#include "jeChain.h"
#include "jeResource.h"


#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================

typedef struct juWorld juWorld;
typedef struct jeActor jeActor;
typedef struct
{
	jeVec3d		Impact;			// Impact location
	jePlane		Plane;			// Impact plane
	jeObject	*Object;		// Object that was hit
	jeActor *Actor; // Actor collided with (target) - Incarnadine
	char *SourceBone; // Name of the bone colliding from the source actor - Incarnadine
	char *TargetBone; // Name of the bone collided with from Actor - Incarnadine
} jeCollisionInfo;

// Added by Incarnadine
// Flags for collision functions
#define COLLIDE_ACTORS (1<<0) 
#define COLLIDE_WORLD (1<<1)
#define COLLIDE_BONES (1<<2)
#define COLLIDE_MESH (1<<3) // Not currently implemented

// Temp hack structure of all mothers
typedef struct
{
	int32			NumObjects;
	int32			NumRenders;
	int32			NumPortals;
	int32			NumDLights;

	int32			NumTransformedPolys;
	int32			NumRenderedPolys;
	int32			NumActorPolys;
	int32			NumNodes;
	int32			NumLeaves;

	int32			NumVisibleAreas;
} juWorld_DebugInfo;

#define MAX_VISIBLE_DLIGHTS			32			// Max 32 visible dlights in the frustum

//========================================================================================
// Misc functions

JETAPI juWorld	*	juWorld_Create( jeResourceMgr * pResourceMgr );

JETAPI jeBoolean	juWorld_WriteToFile(const juWorld *World, jeVFile *VFile, jePtrMgr *PtrMgr);
JETAPI juWorld	*	juWorld_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr, jeResourceMgr * pResourceMgr );

JETAPI jeBoolean	juWorld_CreateRef(juWorld *World);
JETAPI void			juWorld_Destroy(juWorld **World);

JETAPI jeBoolean juWorld_RebuildBSP(juWorld *World, 
										jeBSP_Options Options, 
										jeBSP_Logic Logic, 
										jeBSP_LogicBalance LogicBalance);

#ifdef USE_MODELS

//========================================================================================
// Model list manipulation

JETAPI jeBoolean	juWorld_AddModel(juWorld *World, juModel *Model);
JETAPI jeBoolean	juWorld_RemoveModel(juWorld *World, juModel *Model);
JETAPI jeBoolean	juWorld_HasModel(juWorld *World, juModel *Model);
JETAPI juModel	*	juWorld_GetNextModel(const juWorld *World, const juModel *Start);

#endif

//========================================================================================
// Actor Manipulation Functions - Added by Incarnadine
//
// Actor list manipulation
//    Use these to manage adding/removing actors from the world.  When a world
// knows about actors, they get rendered during a call to juWorld_Render() and they
// get checked for collisions during a call to juWorld_Collision().
JETAPI jeBoolean	juWorld_AddActor(juWorld *World, jeActor *Actor);
JETAPI jeBoolean	juWorld_RemoveActor(juWorld *World, jeActor *Actor);
JETAPI jeBoolean	juWorld_HasActor(juWorld *World, jeActor *Actor);
JETAPI jeActor	*	juWorld_GetNextActor(const juWorld *World, jeActor *Start);

//========================================================================================


//========================================================================================
// All the sub-arrays

JETAPI jeMaterial_Array		*juWorld_GetMaterialArray(const juWorld *World);
JETAPI jeFaceInfo_Array		*juWorld_GetFaceInfoArray(const juWorld *World);
JETAPI jeChain				*juWorld_GetLightChain(const juWorld *World);
JETAPI jeChain				*juWorld_GetDLightChain(const juWorld *World);

//========================================================================================
// SetEngine Then Render

JETAPI jeBoolean	juWorld_SetEngine(juWorld *World, jeEngine *Engine);

#ifdef USE_MODELS
JETAPI jeBoolean	juWorld_SetRenderOptions(juWorld *World, jeBSP_RenderMode RenderMode);
#endif

JETAPI	int32		juWorld_GetRenderRecursion( const juWorld *World );
JETAPI jeBoolean	juWorld_Render(juWorld *World, jeCamera *Camera, jeFrustum *CameraSpaceFrustum);

// Lights
JETAPI jeBoolean	juWorld_AddLight(juWorld *World, jeLight *Light, jeBoolean Update);
JETAPI jeBoolean	juWorld_RemoveLight(juWorld *World, jeLight *Light, jeBoolean Update);
JETAPI jeBoolean	juWorld_UpdateLight(juWorld *World, jeLight *Light);
JETAPI jeLight		*juWorld_GetNextLight(juWorld *World, jeLight *Start);

#ifdef USE_MODELS
JETAPI jeBoolean	juWorld_PatchLighting(juWorld *World);
#endif
JETAPI jeBoolean	juWorld_RebuildLights(juWorld *World); // Moved by Incarnadine


JETAPI jeBoolean	juWorld_AddDLight(juWorld *World, jeLight *Light);
JETAPI jeBoolean	juWorld_RemoveDLight(juWorld *World, jeLight *Light);
JETAPI jeLight		*juWorld_GetNextDLight(juWorld *World, jeLight *Start);

JETAPI jeBoolean	juWorld_AddUserPoly(juWorld *World, jeUserPoly *Poly, jeBoolean AutoRemove);
JETAPI jeBoolean	juWorld_RemoveUserPoly(juWorld *World, jeUserPoly *Poly);

// Objects
JETAPI jeBoolean	juWorld_AddObject(juWorld *World,jeObject *Object);
JETAPI jeBoolean	juWorld_HasObject(const juWorld *World,const jeObject *Object);
JETAPI jeBoolean	juWorld_RemoveObject(juWorld *World,jeObject *Object);
JETAPI jeObject		*juWorld_GetNextObject(juWorld *World, jeObject *Start);
JETAPI jeObject		*juWorld_FindObjectByDefName(juWorld *World, const char *DefName);
JETAPI jeBoolean	juWorld_Frame(juWorld *World, float TimeDelta );

JETAPI jeBoolean	juWorld_Collision(	const juWorld *World, 
											const jeExtBox *Box, 
											const jeVec3d *Front, 
											const jeVec3d *Back, 
											const uint32 Flags, // Valid Flags are COLLIDE_WORLD, COLLIDE_ACTORS, COLLIDE_BONES
											jeCollisionInfo *CollisionInfo);
//External Attachments
JETAPI jeBoolean	juWorld_AttachSoundSystem(juWorld *World, jeSound_System * pSoundSystem );
JETAPI jeSound_System *	juWorld_GetSoundSystem(juWorld *World );

// resource manager
JETAPI jeResourceMgr * juWorld_GetResourceMgr(juWorld *World);

#ifdef __cplusplus
}
#endif

#endif

