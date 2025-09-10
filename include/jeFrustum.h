/****************************************************************************************/
/*  JEFRUSTUM.H                                                                         */
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
#ifndef JE_FRUSTUM2_H		// For now, also defined in Frustum.h (that file will be gone soon)
#define JE_FRUSTUM2_H

#include "BaseType.h"
#include "jeTypes.h"
#include "Camera.h"
#include "jePlane.h"

#ifdef __cplusplus
extern "C" {
#endif

//================================================================================
//================================================================================
#define JE_FRUSTUM_MAX_PLANES		32	// 32 bits in uint32, sigh...
#define CLIP_PLANE_EPSILON			0.001f	// CB note : just use JE_EPSILON instead

//================================================================================
//================================================================================
typedef struct jeFrustum
{
	int32			NumPlanes;
	jePlane		Planes[JE_FRUSTUM_MAX_PLANES];

	jePlane		*FrontPlane;		// Pointer into the above list of planes if there is a front clip plane, NULL otherwise

	// Quick LUTS For BBox testing against frustum
	//	CB note : need to document how this is used : !!
	int32			FrustumBBoxIndexes[JE_FRUSTUM_MAX_PLANES*6];
	int32			*pFrustumBBoxIndexes[JE_FRUSTUM_MAX_PLANES];
} jeFrustum;

// NOTE - SrcVerts can be the same array as Work1, but SrcVerts cannot be same as Work2!!!
typedef struct
{
	uint32		ClipFlags;			// Bit for each frustum plane

	int32			NumSrcVerts;
	const jeLVertex	*SrcVerts;			// Verts to be clipped

	jeLVertex	*Work1;				// Working temps (should be at least as big as NumSrcVerts+1)
	jeLVertex	*Work2;

	// This is to be filled in by clip function
	// DstVerts could be a pointer to SrcVerts, Work1, or Work2
	int32			NumDstVerts;		// Num DstVerts
	jeLVertex	*DstVerts;			// Dest array
} jeFrustum_LClipInfo;

typedef struct
{
	uint32		ClipFlags;			// Bit for each frustum plane

	int32			NumSrcVerts;
	const jeVec3d	*SrcVerts;			// Verts to be clipped

	jeVec3d		*Work1;				// Working temps (should be at least as big as NumSrcVerts+1)
	jeVec3d		*Work2;

	// This is to be filled in by clip function
	// DstVerts could be a pointer to SrcVerts, Work1, or Work2
	int32			NumDstVerts;		// Num DstVerts
	jeVec3d		*DstVerts;			// Dest array
} jeFrustum_ClipInfo;

//================================================================================
// frustum setup functions

JETAPI void		JETCC jeFrustum_SetFromCamera(jeFrustum *Frustum, const jeCamera *Camera);
JETAPI void		JETCC jeFrustum_SetWorldSpaceFromCamera(jeFrustum *Frustum, const jeCamera *Camera);
JETAPI jeBoolean	JETCC jeFrustum_SetFromVerts(jeFrustum *Frustum, const jeVec3d *POV, const jeVec3d *Verts, int32 NumVerts);
JETAPI jeBoolean	JETCC jeFrustum_SetFromVerts2(jeFrustum *Frustum, const jeVec3d *Verts, int32 NumVerts);
JETAPI jeBoolean	JETCC jeFrustum_SetFromLVerts(jeFrustum *Frustum, const jeVec3d *POV, const jeLVertex *Verts, int32 NumVerts);
JETAPI jeBoolean  JETCC jeFrustum_SetFromLVerts2(jeFrustum *Frustum, const jeLVertex *Verts, int32 NumVerts, jeBoolean Flip);

JETAPI jeBoolean	JETCC jeFrustum_AddPlane(jeFrustum *Frustum, const jePlane *SrcPlane, jeBoolean FrontPlane);
	// Returns JE_TRUE on success, JE_FALSE if plane could not be added (Out of space)

JETAPI void		JETCC jeFrustum_RotateToWorldSpace(	const jeFrustum *In, const jeCamera *Camera, jeFrustum *Out);
JETAPI void		JETCC jeFrustum_TransformToWorldSpace(const jeFrustum *In, const jeCamera *Camera, jeFrustum *Out);
JETAPI void		JETCC jeFrustum_Rotate(const jeFrustum *In, const jeXForm3d *XForm, jeFrustum *Out);
JETAPI void		JETCC jeFrustum_Transform(const jeFrustum *In, const jeXForm3d *XForm, jeFrustum *Out);
JETAPI void		JETCC jeFrustum_TransformRenorm(const jeFrustum *In, const jeXForm3d *XForm, jeFrustum *Out);
JETAPI void		JETCC jeFrustum_TransformAnchored(jeFrustum *F, const jeXForm3d *XForm, const jeVec3d * Anchor);

//================================================================================
// miscellaneous

JETAPI jeBoolean	JETCC jeFrustum_SetClipFlagsFromExtBox(const jeFrustum *Frustum,const jeExtBox *BBox,uint32 InClipFlags,uint32 *pClipFlags);
					// returns JE_FALSE if the bbox is totally outside the frustum
					//	pClipFlags is optional

//================================================================================
// Clip functions :
// CB note : these should probably take a MaxOutVerts parameter too, to assert on !!

JETAPI jeBoolean JETCC jeFrustum_ClipLVertsToPlaneXYZUV(	const jePlane *pPlane, 
											const jeLVertex *pIn, jeLVertex *pOut,
											int32 NumVerts, int32 *OutVerts);
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsToPlaneXYZUVRGB(	const jePlane *pPlane, 
												const jeLVertex *pIn, jeLVertex *pOut,
												int32 NumVerts, int32 *OutVerts);
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsToPlaneXYZUVRGBA(	const jePlane *pPlane, 
												const jeLVertex *pIn, jeLVertex *pOut,
												int32 NumVerts, int32 *OutVerts);
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsToPlaneXYZUVRGBAS(const jePlane *pPlane, 
												const jeLVertex *pIn, jeLVertex *pOut,
												int32 NumVerts, int32 *OutVerts);
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsXYZUV(const jeFrustum *Frustum, jeFrustum_LClipInfo *ClipInfo);
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsXYZUVRGB(const jeFrustum *Frustum, jeFrustum_LClipInfo *ClipInfo);
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsXYZUVRGBA(const jeFrustum *Frustum, jeFrustum_LClipInfo *ClipInfo);
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsXYZUVRGBAS(const jeFrustum *Frustum, jeFrustum_LClipInfo *ClipInfo);

JETAPI jeBoolean JETCC jeFrustum_ClipVertsToPlane(	const jePlane *pPlane, 
										const jeVec3d *pIn, jeVec3d *pOut,
										int32 NumVerts, int32 *OutVerts);

JETAPI jeBoolean JETCC jeFrustum_ClipVerts(const jeFrustum *Frustum, jeFrustum_ClipInfo *ClipInfo);

JETAPI jeBoolean JETCC jeFrustum_PointCollision(const jeFrustum *Frustum, const jeVec3d *Point, jeFloat Radius);
//================================================================================

#ifdef __cplusplus
}
#endif

#endif
