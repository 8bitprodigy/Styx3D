/*!
	@file jeBrush.h 
	
	@author John Pollard
	@brief Brushes and Faces API definitions

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

#ifndef JEBRUSH_H
#define JEBRUSH_H

#include "BaseType.h"

#include "jeFaceInfo.h"
#include "Xform3d.h"
#include "jePlane.h"
#include "jeVertArray.h"
#include "jePtrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeEngine		jeEngine;
typedef struct jeCamera		jeCamera;

//========================================================================================
//	Typedefs/#defines
//========================================================================================
typedef uint32		jeBrush_Contents;

typedef struct		jeBrush					jeBrush;
typedef struct		jeBrush_Face			jeBrush_Face;

#define JE_BSP_CONTENTS_FLOCK		(1<<0)
#define JE_BSP_CONTENTS_SOLID		(1<<1)
#define JE_BSP_CONTENTS_AIR			(1<<2)
#define JE_BSP_CONTENTS_SHEET		(1<<3)
#define JE_BSP_CONTENTS_EMPTY		(1<<4)

#define JE_BSP_VISIBLE_CONTENTS		(JE_BSP_CONTENTS_FLOCK | JE_BSP_CONTENTS_SHEET| JE_BSP_CONTENTS_SOLID | JE_BSP_CONTENTS_EMPTY | JE_BSP_CONTENTS_AIR)
//#define JE_BSP_SEP_CONTENTS		(JE_BSP_CONTENTS_SOLID|JE_BSP_CONTENTS_EMPTY|JE_BSP_CONTENTS_AIR)
#define JE_BSP_SEP_CONTENTS			(~JE_BSP_CONTENTS_SHEET)
#define JE_BSP_CONTENTS_EXCLUSIVE	(JE_BSP_CONTENTS_SOLID | JE_BSP_CONTENTS_AIR);

//========================================================================================
//	Structure defs
//========================================================================================
typedef struct
{
	jeBrush				*Brush;
	jeBrush_Face		*BrushFace;
	jeVec3d				Impact;
	jePlane				Plane;
	jeBrush_Contents	c1, c2;		// For testing.  c1 = Front leaf contents, c2 = Back leaf contents
} jeBrushRayInfo;

//========================================================================================
//	Function prototypes
//========================================================================================
JETAPI jeBrush* JETCC jeBrush_Create(int32 EstimatedVerts);


JETAPI jeBrush* JETCC jeBrush_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMGr);
JETAPI jeBoolean	JETCC jeBrush_WriteToFile(const jeBrush *Brush, jeVFile *VFile, jePtrMgr *PtrMGr);

JETAPI jeBoolean	JETCC jeBrush_CreateRef(jeBrush *Brush);
JETAPI void			JETCC jeBrush_Destroy(jeBrush **Brush);
JETAPI jeBoolean	JETCC jeBrush_IsValid(const jeBrush *Brush);
JETAPI jeBoolean	JETCC jeBrush_IsConvex(const jeBrush *Brush);
JETAPI void			JETCC jeBrush_SetContents(jeBrush *Brush, jeBrush_Contents Contents);
JETAPI jeBrush_Contents JETCC jeBrush_GetContents(const jeBrush *Brush);
JETAPI void			JETCC jeBrush_SetXForm(jeBrush *Brush, const jeXForm3d *XForm, jeBoolean Locked);
JETAPI const jeXForm3d* JETCC jeBrush_GetXForm(jeBrush *Brush);

JETAPI const jeXForm3d* JETCC jeBrush_GetWorldToLockedXForm(jeBrush *Brush);
JETAPI const jeXForm3d* JETCC jeBrush_GetLockedToWorldXForm(jeBrush *Brush);
JETAPI jeVertArray* JETCC jeBrush_GetVertArray(const jeBrush *Brush);

JETAPI jeBoolean	JETCC jeBrush_SetFaceInfoArray(jeBrush *Brush, jeFaceInfo_Array *Array);

JETAPI jeBrush_Face* JETCC jeBrush_CreateFace(jeBrush *Brush, int32 NumVerts);
JETAPI jeBrush_Face* JETCC jeBrush_CreateFaceFromFile(jeBrush *Brush, jeVFile *VFile);
JETAPI jeBoolean	JETCC jeBrush_WriteFaceToFile(const jeBrush *Brush, const jeBrush_Face *Face, jeVFile *VFile);
JETAPI void			JETCC jeBrush_DestroyFace(jeBrush *Brush, jeBrush_Face **Face);
JETAPI int32		JETCC jeBrush_GetFaceCount(const jeBrush *Brush);
JETAPI jeBrush_Face* JETCC jeBrush_GetNextFace(const jeBrush *Brush, const jeBrush_Face *Start);
	// If start is NULL, the first face will be returned...
JETAPI jeBrush_Face* JETCC jeBrush_GetPrevFace(const jeBrush *Brush, const jeBrush_Face *Start);
	// Start CANNOT be NULL!  It MUST be a valid object...
JETAPI jeBrush_Face* JETCC jeBrush_GetFaceByIndex(const jeBrush *Brush, int32 Index);

JETAPI jeBrush* JETCC jeBrush_FaceGetBrush(const jeBrush_Face *Face);
JETAPI int32	JETCC jeBrush_FaceGetVertCount(const jeBrush_Face *Face);

/*! @fn void jeBrush_FaceSetVertByIndex(jeBrush_Face *Face, int32 Index, const jeVec3d *Vert)
	@brief Add or modify a vertex of a face of a brush
	@param Face The Brush face to reset vertex data
	@param Index The index of the vertex to reset
	@param Vert The vertex data to write at @p Index slot of vertex of the @p Face
*/
JETAPI void		 JETCC jeBrush_FaceSetVertByIndex(jeBrush_Face *Face, int32 Index, const jeVec3d *Vert);

/*! @fn void jeBrush_FaceMoveVertByIndex(jeBrush_Face *Face, int32 Index, const jeVec3d *Vert)
	@brief Move a vertex of a face of a brush
	@param Face The Brush face to reset vertex data
	@param Index The index of the vertex to reset
	@param Vert The move vector data for vertex of the @p Face
*/
JETAPI void		 JETCC jeBrush_FaceMoveVertByIndex(jeBrush_Face *Face, int32 Index, const jeVec3d *Vert);
JETAPI const jeVec3d* JETCC jeBrush_FaceGetVertByIndex(const jeBrush_Face *Face, int32 Index);
JETAPI jeVec3d	 JETCC jeBrush_FaceGetWorldSpaceVertByIndex(const jeBrush_Face *Face, int32 Index);

JETAPI void		JETCC jeBrush_FaceCalcPlane(jeBrush_Face *Face);
JETAPI const jeVec3d* JETCC jeBrush_FaceGetNormal(jeBrush_Face *Face);

JETAPI jeBoolean	JETCC jeBrush_FaceGetFaceInfo(jeBrush_Face *Face, jeFaceInfo *FaceInfo);
JETAPI jeBoolean	JETCC jeBrush_FaceSetFaceInfo(jeBrush_Face *Face, const jeFaceInfo *FaceInfo);

JETAPI jeFaceInfo_ArrayIndex JETCC jeBrush_FaceGetFaceInfoIndex(jeBrush_Face *Face);

JETAPI jeBoolean	JETCC jeBrush_Render(const jeBrush *Brush, const jeEngine *Engine, const jeCamera *Camera);

#ifdef __cplusplus
}
#endif

#endif
