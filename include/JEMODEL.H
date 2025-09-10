/****************************************************************************************/
/*  jeModel.H                                                                           */
/*                                                                                      */
/*  Author:  John Pollard                                                               */
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

#ifndef JEMODEL_H
#define JEMODEL_H

#include "jeBrush.h"
#include "BaseType.h"
#include "VFile.h"
#include "jeLight.h"
#include "jeChain.h"
#include "jePtrMgr.h"
#include "jeBSP.h"

#ifdef __cplusplus
extern "C" {
#endif

//**********
// Temporary, until we get this stuff in jeTypes.h, or Jet.h?

typedef struct		jeEngine		jeEngine;
typedef struct		jeCamera		jeCamera;
typedef struct		jeFrustum		jeFrustum;

typedef struct		jeModel			jeModel;
typedef struct jeBSPSetup { // Incarnadine
	jeBSP_Options Options;
	jeBSP_Logic Logic;
	jeBSP_LogicBalance LogicBalance;
} jeBSPSetup;

JETAPI jeModel		* JETCC jeModel_Create(void);
JETAPI jeBoolean	JETCC jeModel_WriteToFile(const jeModel *Model, jeVFile *VFile, jePtrMgr *PtrMgr);
JETAPI jeModel		* JETCC jeModel_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);
JETAPI jeBoolean	JETCC jeModel_CreateRef(jeModel *Model);
JETAPI void			JETCC jeModel_Destroy(jeModel **Model);
JETAPI void			JETCC jeModel_SetArrays(jeModel *Model, jeFaceInfo_Array *FArray, jeMaterial_Array *Marray, jeChain *LChain, jeChain *DLChain);
JETAPI jeBoolean	JETCC jeModel_AddBrush(jeModel *Model, jeBrush *Brush, jeBoolean Update, jeBoolean AutoLight);
JETAPI jeBoolean	JETCC jeModel_RemoveBrush(jeModel *Model, jeBrush *Brush, jeBoolean Update);
JETAPI jeBoolean	JETCC jeModel_UpdateBrush(jeModel *Model, jeBrush *Brush, jeBoolean AutoLight);
JETAPI jeBrush		* JETCC jeModel_GetNextBrush(const jeModel *Model, const jeBrush *Start);
JETAPI jeBoolean	JETCC jeModel_HasBrush(const jeModel *Model, const jeBrush *Brush);

JETAPI jeBoolean	JETCC jeModel_AddObject(jeModel *Model, jeObject *Object);
JETAPI jeBoolean	JETCC jeModel_RemoveObject(jeModel *Model, jeObject *Object);

JETAPI jeBoolean	JETCC jeModel_UpdateBrushFace(jeModel *Model, const jeBrush_Face *Face, jeBoolean AutoLight);
JETAPI jeBoolean	JETCC jeModel_RebuildLightsFromPoint(jeModel *Model, const jeVec3d *Pos, jeFloat Radius);

JETAPI jeBoolean	JETCC jeModel_RebuildBSP(	jeModel *Model, 
								jeBSP_Options Options, 
								jeBSP_Logic Logic, 
								jeBSP_LogicBalance LogicBalance);
JETAPI jeBoolean	JETCC jeModel_RebuildBSPFaces(jeModel *Model);
JETAPI jeBoolean	JETCC jeModel_PatchLighting(jeModel *Model);
JETAPI jeBoolean	JETCC jeModel_RebuildLights(jeModel *Model);
JETAPI jeBoolean	JETCC jeModel_SetEngine(jeModel *Model, jeEngine *Engine);
JETAPI jeBoolean	JETCC jeModel_SetWorld(jeModel *Model, jeWorld *World);
JETAPI jeBoolean	JETCC jeModel_SetRenderOptions(jeModel *Model, jeBSP_RenderMode RenderMode);
JETAPI jeBoolean	JETCC jeModel_SetDefaultContents(jeModel *Model, jeBrush_Contents DefaultContents);
JETAPI jeBoolean	JETCC jeModel_Render(jeModel *Model, jeCamera *Camera, jeFrustum *CameraSpaceFrustum);

JETAPI jeBoolean	JETCC jeModel_RayIntersectsBrushes(const jeModel *Model, const jeVec3d *Front, const jeVec3d *Back, jeBrushRayInfo *Info);

JETAPI jeBoolean	JETCC jeModel_SetBrushFaceCB(jeModel *Model, jeBSPNode_DrawFaceCB *CB, void *Context);
JETAPI jeBoolean	JETCC jeModel_SetBrushFaceCBOnOff(jeModel *Model, const jeBrush_Face *Face, jeBoolean OnOff);

// Temporary??
JETAPI const		jeBSP_DebugInfo *JETCC jeModel_GetBSPDebugInfo(const jeModel *Model);

JETAPI jeBoolean	JETCC jeModel_Collision(	const jeModel	*Model, 
											const jeExtBox	*Box, 
											const jeVec3d	*Front, 
											const jeVec3d	*Back, 
											jeVec3d			*Impact, 
											jePlane			*Plane);

JETAPI jeBoolean	JETCC jeModel_ChangeBoxCollision(	const jeModel	*Model, 
												const jeVec3d	*Pos, 
												const jeExtBox	*FrontBox, 
												const jeExtBox	*BackBox, 
												jeExtBox		*ImpactBox, 
												jePlane			*Plane);

JETAPI jeBoolean	JETCC jeModel_SetXForm(jeModel *Model, const jeXForm3d *XForm);
JETAPI const		jeXForm3d * JETCC jeModel_GetXForm(jeModel *Model );

#ifdef __cplusplus
}
#endif

#endif
