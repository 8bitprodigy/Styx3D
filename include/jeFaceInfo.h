/****************************************************************************************/
/*  JEFACEINFO.H                                                                        */
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

#ifndef GEFACEINFO_H
#define GEFACEINFO_H

#include "Vec3d.h"
#include "jeMaterial.h"
#include "VFile.h"
#include "jeGArray.h"
#include "jePortal.h"
#include "Object.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================
//	Typedefs/#defines
//========================================================================================
typedef struct	jeFaceInfo_Array		jeFaceInfo_Array;
					
typedef jeGArray_Index					jeFaceInfo_ArrayIndex;

#define JE_FACEINFO_ARRAY_MAX_INDEX		JE_GARRAY_MAX_INDEX
#define JE_FACEINFO_ARRAY_NULL_INDEX	JE_GARRAY_NULL_INDEX

#define FACEINFO_GOURAUD				(1<<0)
#define FACEINFO_FLAT					(1<<1)
#define FACEINFO_TRANSPARENT			(1<<2)
#define FACEINFO_FULLBRIGHT				(1<<3)
#define FACEINFO_VIS_PORTAL				(1<<4)
#define FACEINFO_RENDER_PORTAL_ONLY		(1<<5)	

// These flags are for testing convenience, and should NOT be used for assignment
#define FACEINFO_NO_DRAWFACE			(FACEINFO_VIS_PORTAL)
#define FACEINFO_NO_LIGHTMAP			(FACEINFO_FULLBRIGHT | FACEINFO_VIS_PORTAL | FACEINFO_RENDER_PORTAL_ONLY | FACEINFO_FLAT | FACEINFO_GOURAUD)

//========================================================================================
//	Structure defs
//========================================================================================
typedef struct
{
	uint32					Flags;			// See flag definitions above
	jeFloat					Alpha;			// Alpha value (0...255)
	jeFloat					Rotate;			// (0...PI2)
	jeFloat					ShiftU;			// Texture shift U
	jeFloat					ShiftV;			// Texture shift V
	jeFloat					DrawScaleU;		// Texture scale
	jeFloat					DrawScaleV;
	jeFloat					LMapScaleU;		// Lightmap scale
	jeFloat					LMapScaleV;
	jeMaterial_ArrayIndex	MaterialIndex;	// Material for this face

	jeObject				*PortalCamera;
} jeFaceInfo;

//========================================================================================
//	Function prototypes
//========================================================================================
JETAPI void			JETCC jeFaceInfo_SetDefaults(jeFaceInfo *FaceInfo);		// Handy little function
JETAPI jeBoolean	JETCC jeFaceInfo_Compare(const jeFaceInfo *Face1, const jeFaceInfo *Face2);
JETAPI jeBoolean	JETCC jeFaceInfo_NeedsLightmap(const jeFaceInfo *pFaceInfo);


JETAPI jeFaceInfo_Array * JETCC jeFaceInfo_ArrayCreate(int32 StartFaces);


JETAPI jeBoolean	JETCC jeFaceInfo_ArrayWriteToFile(const jeFaceInfo_Array *Array, jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);
JETAPI jeFaceInfo_Array * JETCC jeFaceInfo_ArrayCreateFromFile(jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);

JETAPI jeBoolean	JETCC jeFaceInfo_ArrayCreateRef(jeFaceInfo_Array *Array);
JETAPI void			JETCC jeFaceInfo_ArrayDestroy(jeFaceInfo_Array **Array);
JETAPI jeBoolean	JETCC jeFaceInfo_ArrayIsValid(const jeFaceInfo_Array *Array);
JETAPI jeBoolean	JETCC jeFaceInfo_ArrayIndexIsValid(jeFaceInfo_ArrayIndex Index);
JETAPI jeFaceInfo_ArrayIndex JETCC jeFaceInfo_ArrayAddFaceInfo(jeFaceInfo_Array *Array, const jeFaceInfo *FaceInfo);
JETAPI jeFaceInfo_ArrayIndex JETCC jeFaceInfo_ArrayShareFaceInfo(jeFaceInfo_Array *Array, const jeFaceInfo *FaceInfo);
JETAPI jeBoolean	JETCC jeFaceInfo_ArrayRefFaceInfoIndex(jeFaceInfo_Array *Array, jeFaceInfo_ArrayIndex Index);
JETAPI void			JETCC jeFaceInfo_ArrayRemoveFaceInfo(jeFaceInfo_Array *Array, jeFaceInfo_ArrayIndex *Index);
JETAPI void			JETCC jeFaceInfo_ArraySetFaceInfoByIndex(jeFaceInfo_Array *Array, jeFaceInfo_ArrayIndex Index, const jeFaceInfo *FaceInfo);
JETAPI const		jeFaceInfo * JETCC jeFaceInfo_ArrayGetFaceInfoByIndex(const jeFaceInfo_Array *Array, jeFaceInfo_ArrayIndex Index);

#ifdef __cplusplus
}
#endif

#endif
