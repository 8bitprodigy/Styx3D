/****************************************************************************************/
/*  PTRTYPES.H                                                                          */
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
#ifndef JE_PTRTYPES_H
#define JE_PTRTYPES_H

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

// System.h
typedef struct	jeEngine			jeEngine;

// Light.h
typedef struct	Light_LightInfo		Light_LightInfo;

//	Surface.h
typedef	struct	Surf_SurfInfo		Surf_SurfInfo;
typedef	struct	Surf_TexVert		Surf_TexVert;

// World.h
typedef	struct	jeWorld				jeWorld;

// Frustum.h
typedef	struct	Frustum_Info		Frustum_Info;

// World.h
typedef struct	World_BSP			World_BSP;
typedef struct	jeWorld_Leaf		jeWorld_Leaf;

				
// Mesh.h
typedef struct	Mesh_MeshInfo		Mesh_MeshInfo;
typedef struct	Mesh_MeshDef		Mesh_MeshDef;
typedef struct	Mesh_RenderQ		Mesh_RenderQ;

// Entities.h
typedef struct	jeEntity_EntitySet jeEntity_EntitySet;

// User.h
typedef struct	User_Info		User_Info;
typedef struct  jeUPoly			jeUPoly;

#ifdef __cplusplus
}
#endif
#endif
