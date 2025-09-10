/****************************************************************************************/
/*  jeWorld.H                                                                           */
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

#ifndef jeWorld_H
#define jeWorld_H

#include "jeModel.h"  // Added by Incarnadine

#include "jeFaceInfo.h"		// jeFaceInfo_Array
#include "jeMaterial.h"		// jeMaterial_Array
#include "BaseType.h"
#include "VFile.h"
#include "jeLight.h"
#include "Camera.h"
#include "jePtrMgr.h"
#include "jeUserPoly.h"
#include "Object.h"
#include "Sound.h"
#include "jeChain.h"
#include "jeResource.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================

typedef struct jeWorld jeWorld;
typedef struct jeActor jeActor;
typedef struct jeCollisionInfo
{
	jeVec3d		Impact;			// Impact location
	jePlane		Plane;			// Impact plane
	jeObject	*Object;		// Object that was hit
	jeBoolean	IsValid;		// Infos are valid - Icestorm
} jeCollisionInfo;

// Added by Icestorm
typedef struct
{
	jeExtBox	ImpactBox;		// Impact box
	jePlane		Plane;			// Impact plane
	jeObject	*Object;		// Object that was hit
	jeBoolean	IsValid;		// Infos are valid - Icestorm
} jeChangeBoxCollisionInfo;

// Added by Incarnadine
// Flags for collision functions
#define COLLIDE_EXTBOX 0
#define COLLIDE_BONES 1
#define COLLIDE_MESH 2 // Not currently implemented

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
} jeWorld_DebugInfo;

#define MAX_VISIBLE_DLIGHTS			32			// Max 32 visible dlights in the frustum

//========================================================================================
// Misc functions

JETAPI jeWorld	*	JETCC jeWorld_Create( jeResourceMgr * pResourceMgr );
JETAPI jeBoolean	JETCC jeWorld_WriteToFile(const jeWorld *World, jeVFile *VFile, jePtrMgr *PtrMgr);
JETAPI jeWorld	*	JETCC jeWorld_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr, jeResourceMgr * pResourceMgr );

// =====================
// Incarnadine
// -------------------
// This function simplifies the loading process for a level.  Pass it the filename of a 
// level created in JEdit and it will automatically open the appropriate, file and fork, and 
// then read in the world.  The second two parameters here CAN be NULL.  If they are NULL,
// they will be created automatically.
JETAPI jeWorld	*	JETCC jeWorld_CreateFromEditorFile(const char* FileName, jePtrMgr *pPtrMgr, jeResourceMgr * pResourceMgr );

JETAPI jeBoolean	JETCC jeWorld_CreateRef(jeWorld *World);
JETAPI void			JETCC jeWorld_Destroy(jeWorld **World);

JETAPI jeBoolean JETCC jeWorld_RebuildBSP(jeWorld *World, 
										jeBSP_Options Options, 
										jeBSP_Logic Logic, 
										jeBSP_LogicBalance LogicBalance);

#ifdef USE_MODELS

//========================================================================================
// Model list manipulation

JETAPI jeBoolean	JETCC jeWorld_AddModel(jeWorld *World, jeModel *Model);
JETAPI jeBoolean	JETCC jeWorld_RemoveModel(jeWorld *World, jeModel *Model);
JETAPI jeBoolean	JETCC jeWorld_HasModel(jeWorld *World, jeModel *Model);
JETAPI jeModel	*	JETCC jeWorld_GetNextModel(const jeWorld *World, const jeModel *Start);

#endif

//========================================================================================
// Actor Manipulation Functions - Added by Incarnadine
//
// Actor list manipulation
//    Use these to manage adding/removing actors from the world.  When a world
// knows about actors, they get rendered during a call to jeWorld_Render() and they
// get checked for collisions during a call to jeWorld_Collision().
//JETAPI jeBoolean	jeWorld_AddActor(jeWorld *World, jeActor *Actor);
//JETAPI jeBoolean	jeWorld_RemoveActor(jeWorld *World, jeActor *Actor);
//JETAPI jeBoolean	jeWorld_HasActor(jeWorld *World, jeActor *Actor);
//JETAPI jeActor	*	jeWorld_GetNextActor(const jeWorld *World, jeActor *Start);

//========================================================================================


//========================================================================================
// All the sub-arrays

JETAPI jeMaterial_Array		* JETCC jeWorld_GetMaterialArray(const jeWorld *World);
JETAPI jeFaceInfo_Array		* JETCC jeWorld_GetFaceInfoArray(const jeWorld *World);
JETAPI jeChain				* JETCC  jeWorld_GetLightChain(const jeWorld *World);
JETAPI jeChain				* JETCC jeWorld_GetDLightChain(const jeWorld *World);

//========================================================================================
// SetEngine Then Render

JETAPI jeBoolean	JETCC jeWorld_SetEngine(jeWorld *World, jeEngine *Engine);

#ifdef USE_MODELS
JETAPI jeBoolean	JETCC jeWorld_SetRenderOptions(jeWorld *World, jeBSP_RenderMode RenderMode);
#endif

JETAPI	int32		JETCC jeWorld_GetRenderRecursion( const jeWorld *World );
JETAPI jeBoolean	JETCC jeWorld_Render(jeWorld *World, jeCamera *Camera, jeFrustum *CameraSpaceFrustum);

// Lights
JETAPI jeBoolean	JETCC jeWorld_AddLight(jeWorld *World, jeLight *Light, jeBoolean Update);
JETAPI jeBoolean	JETCC jeWorld_RemoveLight(jeWorld *World, jeLight *Light, jeBoolean Update);
JETAPI jeBoolean	JETCC jeWorld_UpdateLight(jeWorld *World, jeLight *Light);
JETAPI jeLight		* JETCC jeWorld_GetNextLight(jeWorld *World, jeLight *Start);

#ifdef USE_MODELS
JETAPI jeBoolean	JETCC jeWorld_PatchLighting(jeWorld *World);
#endif
JETAPI jeBoolean	JETCC jeWorld_RebuildLights(jeWorld *World); // Moved by Incarnadine


JETAPI jeBoolean	JETCC jeWorld_AddDLight(jeWorld *World, jeLight *Light);
JETAPI jeBoolean	JETCC jeWorld_RemoveDLight(jeWorld *World, jeLight *Light);
JETAPI jeLight		* JETCC jeWorld_GetNextDLight(jeWorld *World, jeLight *Start);

JETAPI jeBoolean	JETCC jeWorld_AddUserPoly(jeWorld *World, jeUserPoly *Poly, jeBoolean AutoRemove);
JETAPI jeBoolean	JETCC jeWorld_RemoveUserPoly(jeWorld *World, jeUserPoly *Poly);

// Objects
JETAPI jeBoolean	JETCC jeWorld_AddObject(jeWorld *World,jeObject *Object);
JETAPI jeBoolean	JETCC jeWorld_HasObject(const jeWorld *World,const jeObject *Object);
JETAPI jeBoolean	JETCC jeWorld_RemoveObject(jeWorld *World,jeObject *Object);
JETAPI jeObject	* JETCC jeWorld_GetNextObject(jeWorld *World, jeObject *Start);
JETAPI jeObject	* JETCC jeWorld_FindObjectByDefName(jeWorld *World, const char *DefName);
JETAPI jeBoolean	JETCC jeWorld_Frame(jeWorld *World, float TimeDelta );

// Icestorm: If CollisionInfo is NULL, they only test, whether there is an collision
JETAPI jeBoolean	JETCC jeWorld_Collision(	const jeWorld *World, 
											const jeExtBox *Box, 
											const jeVec3d *Front, 
											const jeVec3d *Back, 
											jeCollisionInfo *CollisionInfo);

// Added by Icestorm
JETAPI jeBoolean	JETCC jeWorld_ChangeBoxCollision(	const jeWorld *World, 
												const jeVec3d *Pos, 
												const jeExtBox *FrontBox, 
												const jeExtBox *BackBox, 
												jeChangeBoxCollisionInfo *CollisionInfo);

//External Attachments
JETAPI jeBoolean	JETCC jeWorld_AttachSoundSystem(jeWorld *World, jeSound_System * pSoundSystem );
JETAPI jeSound_System *	JETCC jeWorld_GetSoundSystem(jeWorld *World );

// resource manager
JETAPI jeResourceMgr * JETCC jeWorld_GetResourceMgr(jeWorld *World);


//========================================================================================
// Collision Manipulation Functions - Added by Incarnadine
//
JETAPI jeBoolean	JETCC jeWorld_CanCollide(const jeWorld *World, const char* Type);
JETAPI jeBoolean	JETCC jeWorld_EnableCollision(jeWorld *World, const char* Type);
JETAPI jeBoolean  JETCC jeWorld_DisableCollision(jeWorld *World, const char *Type);
JETAPI char * JETCC jeWorld_GetNextCollisionExclusion(const jeWorld *World, const char *Start);
//JETAPI void jeWorld_SetCollisionOptions(jeWorld* World, int32 Level, const char *Include, const char *Exclude);
//JETAPI void jeWorld_ResetCollisionOptions(jeWorld* World);
JETAPI int32 JETCC jeWorld_GetCollisionLevel(const jeWorld* World);
JETAPI void JETCC jeWorld_SetCollisionLevel(jeWorld* World, int32 Level);
//========================================================================================

#ifdef __cplusplus
}
#endif

#endif
