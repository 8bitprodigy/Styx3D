/****************************************************************************************/
/*  QUAD.H                                                                              */
/*                                                                                      */
/*  Author:  Charles Bloom                                                              */
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
#ifndef QUAD_H
#define QUAD_H

#include "BaseType.h"
#include "jeLight.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QuadTree			QuadTree;

typedef struct jeTerrain		jeTerrain;
typedef struct jeTerrain_Light	jeTerrain_Light;

QuadTree *	QuadTree_Create(const jeTerrain *T);

void		QuadTree_Destroy(QuadTree **pQT);

jeBoolean	QuadTree_SetTexDim(QuadTree *QT,int Dim);

jeBoolean	QuadTree_Tesselate(QuadTree *QT,jeVec3d * pPos,jeFrustum *pFrustum);

jeBoolean	QuadTree_Render(const QuadTree *QT,jeEngine *E,jeCamera *Cam,jeFrustum *F);

jeBoolean	QuadTree_LightTesselatedPoints(QuadTree *QT,jeTerrain_Light * Lights,int NumLights);

void		QuadTree_LightAllPoints(QuadTree *QT,jeLight ** Lights,int NumLights);

void		QuadTree_LightTexture(  QuadTree *QT,jeLight ** Lights,int NumLights,jeBoolean SelfShadow,jeBoolean WorldShadow);

void		QuadTree_ShowStats(const QuadTree *QT);

void		QuadTree_SetParameters(QuadTree * QT,uint32 BaseDepth,uint32 MaxQuads,float MinError);

jeBoolean	QuadTree_IsValid(const QuadTree *QT);

void		QuadTree_GetExtBox(const QuadTree * QT,jeExtBox * Box);

jeBoolean	QuadTree_IntersectRay(QuadTree *QT,jeVec3d *pStart,jeVec3d *pDirection);

jeBoolean	QuadTree_IntersectThickRay(const QuadTree * QT,const jeVec3d * From,const jeVec3d * To,jeFloat Radius,jeVec3d * pImpact);

void		QuadTree_ResetAllVertexLighting(QuadTree * QT);

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
#endif // QUAD_H
