/*!
	@file TERRAIN.H

	@author Charles Bloom
	@brief  Terrain object interface

    @par Licence
	The contents of this file are subject to the Jet3D Public License       
	Version 1.02 (the "License"); you may not use this file except in         
	compliance with the License. You may obtain a copy of the License at       
	http://www.jet3d.com                                                        

	@par
	Software distributed under the License is distributed on an "AS IS"           
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See           
	the License for the specific language governing rights and limitations          
	under the License.                                                               
	@par
	The Original Code is Jet3D, released December 12, 1999.                            
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
*/

#ifndef JE_TERRAIN_H
#define JE_TERRAIN_H

#include "jeTypes.h"
#include "VFile.h"
#include "Bitmap.h"
#include "jeFrustum.h"
#include "Object.h"
#include "jePtrMgr.h"
#include "jeLight.h"

//#include "engine.h"  // for jeEngine
typedef struct jeEngine	jeEngine;

#ifdef __cplusplus
extern "C" {
#endif


typedef struct jeTerrain		jeTerrain;

enum
{
	TERRAIN_PROPERTY_HEIGHTMAP = PROPERTY_LOCAL_DATATYPE_START,
	TERRAIN_PROPERTY_MAXQUADS,
	TERRAIN_PROPERTY_MINERROR,
	TERRAIN_PROPERTY_TEXDIMLOG2,
//	TERRAIN_PROPERTY_VERTEXLIGHTING,
//	TERRAIN_PROPERTY_TEXTURELIGHTING,
//	TERRAIN_PROPERTY_NOLIGHTING,
	TERRAIN_PROPERTY_LIGHTING_LIST,
	TERRAIN_PROPERTY_SIZE,
	TERRAIN_PROPERTY_SIZE_X,
	TERRAIN_PROPERTY_SIZE_Y,
	TERRAIN_PROPERTY_SIZE_Z,
};

/*******************************************************/

JETAPI void *	JETCC jeTerrain_Create(void);
JETAPI void *	JETCC jeTerrain_CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr);
JETAPI jeTerrain *	JETCC jeTerrain_CreateFromFileExt(jeVFile * File,jeVFile *ResourceBaseFS,jePtrMgr *PtrMgr);
JETAPI jeBoolean 	JETCC jeTerrain_Destroy(void ** ppTerrain);
JETAPI void 		JETCC jeTerrain_CreateRef(void * pTerrain);

JETAPI jeBoolean 	JETCC jeTerrain_WriteToFile(const void *pTerrain,jeVFile * File, jePtrMgr *PtrMgr);

/*******************************************************/

// NOTEZ : GetHeight and GetNormal are in "Terrain Space", a 1x1x1 cube

JETAPI jeFloat		JETCC jeTerrain_GetHeightAtXY(const jeTerrain *T,jeFloat X,jeFloat Y);
JETAPI void 		JETCC jeTerrain_GetNormalAtXY(const jeTerrain *T,jeFloat X,jeFloat Y,jeVec3d *pNormal);

JETAPI void			JETCC jeTerrain_GetNormalAtXY_Rough(const jeTerrain *Terrain,jeFloat X,jeFloat Y,jeVec3d *pNormal);

JETAPI jeFloat		JETCC jeTerrain_GetHeightAtWorldSpaceVec(const jeTerrain *Terrain,const jeVec3d *pV);
JETAPI void			JETCC jeTerrain_GetNormalAtWorldSpaceVec(const jeTerrain *Terrain,const jeVec3d *pV,jeVec3d *pN);

JETAPI jeBoolean	JETCC jeTerrain_GetExtBox(const void *T,jeExtBox * pBox);

/*******************************************************/

JETAPI jeBoolean	JETCC jeTerrain_IntersectsRay(const jeTerrain *T,jeVec3d *pStart,jeVec3d *pDirection);

//JETAPI jeBoolean	JETCC jeTerrain_ExtBoxIsVis(const jeTerrain *T,const jeExtBox *pBox,const jeCamera *pCamera);

JETAPI jeBoolean JETCC jeTerrain_SphereCollision(const jeTerrain *T,
													const jeVec3d *pFrom, const jeVec3d *pTo, jeFloat Radius,
													jeVec3d *Impact, jePlane *Plane);

JETAPI jeBoolean JETCC jeTerrain_BoxCollision(const void *T, const jeExtBox *Box, 
													const jeVec3d *Front, const jeVec3d *Back, 
													jeVec3d *Impact, jePlane *Plane);

/*******************************************************/

JETAPI jeBoolean	JETCC jeTerrain_SetSize(jeTerrain * T,jeVec3d * pSize);

JETAPI jeBoolean	JETCC jeTerrain_SetXForm(void * T,const jeXForm3d *pXF);
JETAPI jeBoolean	JETCC jeTerrain_GetXForm(const void * T,jeXForm3d *pXF);

JETAPI jeBoolean	JETCC jeTerrain_SetHeightmap(jeTerrain * T,jeBitmap * Bmp);
JETAPI jeBoolean	JETCC jeTerrain_GetHeightmap(jeTerrain * T,jeBitmap ** pBmp);

JETAPI jeBoolean	JETCC jeTerrain_SetParameters(jeTerrain *T,uint32 MaxQuads,float MinError);
														// 250 and 0.005 are reasonable numbers here

JETAPI jeBoolean	JETCC jeTerrain_SetLightsOnVertsFromWorld(jeTerrain *T,jeWorld *World);
										// LightsOnVerts & DLights are mutually exclusive (DLights overwrite vert lights)

JETAPI jeBoolean JETCC jeTerrain_SetLightsOnVerts(jeTerrain *T, jeLight **SrcLights, int Count);

JETAPI jeBoolean	JETCC jeTerrain_SetATexture(jeTerrain *T,const jeBitmap * Bmp,int tx,int ty);
										// tx & ty in [0,TexDim-1]

JETAPI jeBoolean	JETCC jeTerrain_GetTextureAtXY(const jeTerrain *T,jeFloat X,jeFloat Y,const jeBitmap ** pBmp,int *pTX,int *pTY);
JETAPI jeBoolean	JETCC jeTerrain_GetTextureAtWorldVec(const jeTerrain *T,const jeVec3d *pVec,const jeBitmap ** pBmp,int *pTX,int *pTY);
									// fills out pBmp,pTX,pTY unless null; tx and ty are texture x & t's for use in SetATexture

JETAPI jeBoolean	JETCC jeTerrain_SetTexDim(jeTerrain *T,int TexDim);
									// TexDim must be a power of two <= 8 (eg. 1,2,4,8)

/*******************************************************/

JETAPI jeBoolean	JETCC jeTerrain_RenderPrep(jeTerrain * T,jeEngine *Engine,jeCamera *C);

JETAPI jeBoolean	JETCC jeTerrain_RenderThroughCamera( jeTerrain *T,const jeWorld *World, const jeEngine *E,jeCamera *Camera);

JETAPI jeBoolean	JETCC jeTerrain_RenderThroughFrustum(jeTerrain *T,const jeWorld *World, const jeEngine *E,jeCamera *Camera, const jeFrustum *F);

// you can render & whatnot without attaching & detaching if you like
JETAPI jeBoolean	JETCC jeTerrain_AttachEngine(void *T,jeEngine *Engine);
JETAPI jeBoolean	JETCC jeTerrain_DetachEngine(void *T,jeEngine *Engine);
				   
/*******************************************************/

JETAPI void			JETCC jeTerrain_InitObject(const jeTerrain *T,jeObject *O);

JETAPI jeBoolean	JETCC jeTerrain_RegisterObjectDef(void);

/*******************************************************/

/*!
	@fn void jeTerrain_SetRenderNextTime(jeTerrain* T, jeBoolean RenderNext)
	@brief Enable/diable rendering in the next Object render loop

	@param[in] T The terrain object the flag applies
	@param[in] RenderNext The next rendering flag value (JE_TRUE means rendered, JE_FALSE means ignored)

	@author Bruno Pettorelli (krouer@jet3d.com)
    @see jeObject_SetRenderNextPass
*/
JETAPI void			JETCC jeTerrain_SetRenderNextTime(void* T, jeBoolean RenderNext);

#ifdef __cplusplus
}
#endif

#endif // JE_TERRAIN_H


