/****************************************************************************************/
/*  JUMODEL.H                                                                           */
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

typedef struct		juModel			juModel;
typedef struct jeBSPSetup { // Incarnadine
	jeBSP_Options Options;
	jeBSP_Logic Logic;
	jeBSP_LogicBalance LogicBalance;
} jeBSPSetup;

JETAPI juModel		*juModel_Create(void);
JETAPI jeBoolean	juModel_WriteToFile(const juModel *Model, jeVFile *VFile, jePtrMgr *PtrMgr);
JETAPI juModel		*juModel_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);

JETAPI jeBoolean	juModel_CreateRef(juModel *Model);
JETAPI void			juModel_Destroy(juModel **Model);
JETAPI void			juModel_SetArrays(juModel *Model, jeFaceInfo_Array *FArray, jeMaterial_Array *Marray, jeChain *LChain, jeChain *DLChain);
JETAPI jeBoolean	juModel_AddBrush(juModel *Model, jeBrush *Brush, jeBoolean Update, jeBoolean AutoLight);
JETAPI jeBoolean	juModel_RemoveBrush(juModel *Model, jeBrush *Brush, jeBoolean Update);
JETAPI jeBoolean	juModel_UpdateBrush(juModel *Model, jeBrush *Brush, jeBoolean AutoLight);
JETAPI jeBrush		*juModel_GetNextBrush(const juModel *Model, const jeBrush *Start);
JETAPI jeBoolean	juModel_HasBrush(const juModel *Model, const jeBrush *Brush);

JETAPI jeBoolean	juModel_AddObject(juModel *Model, jeObject *Object);
JETAPI jeBoolean	juModel_RemoveObject(juModel *Model, jeObject *Object);

JETAPI jeBoolean	juModel_UpdateBrushFace(juModel *Model, const jeBrush_Face *Face, jeBoolean AutoLight);
JETAPI jeBoolean	juModel_RebuildLightsFromPoint(juModel *Model, const jeVec3d *Pos, jeFloat Radius);

JETAPI jeBoolean	juModel_RebuildBSP(	juModel *Model, 
								jeBSP_Options Options, 
								jeBSP_Logic Logic, 
								jeBSP_LogicBalance LogicBalance);
JETAPI jeBoolean	juModel_RebuildBSPFaces(juModel *Model);
JETAPI jeBoolean	juModel_PatchLighting(juModel *Model);
JETAPI jeBoolean	juModel_RebuildLights(juModel *Model);
JETAPI jeBoolean	juModel_SetEngine(juModel *Model, jeEngine *Engine);

JETAPI jeBoolean	juModel_SetRenderOptions(juModel *Model, jeBSP_RenderMode RenderMode);
JETAPI jeBoolean	juModel_SetDefaultContents(juModel *Model, jeBrush_Contents DefaultContents);
JETAPI jeBoolean	juModel_Render(juModel *Model, jeCamera *Camera, jeFrustum *CameraSpaceFrustum);

JETAPI jeBoolean	juModel_RayIntersectsBrushes(const juModel *Model, const jeVec3d *Front, const jeVec3d *Back, jeBrushRayInfo *Info);

JETAPI jeBoolean	juModel_SetBrushFaceCB(juModel *Model, jeBSPNode_DrawFaceCB *CB, void *Context);
JETAPI jeBoolean	juModel_SetBrushFaceCBOnOff(juModel *Model, const jeBrush_Face *Face, jeBoolean OnOff);

// Temporary??
JETAPI const		jeBSP_DebugInfo *juModel_GetBSPDebugInfo(const juModel *Model);

JETAPI jeBoolean	juModel_Collision(	const juModel	*Model, 
											const jeExtBox	*Box, 
											const jeVec3d	*Front, 
											const jeVec3d	*Back, 
											jeVec3d			*Impact, 
											jePlane			*Plane);

JETAPI jeBoolean	juModel_SetXForm(juModel *Model, const jeXForm3d *XForm);
JETAPI const		jeXForm3d *juModel_GetXForm(juModel *Model );

#ifdef __cplusplus
}
#endif

#endif
