/****************************************************************************************/
/*  CAMERA.H                                                                            */
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
#ifndef jeCAMERA_H
#define jeCAMERA_H

#include "BaseType.h"
#include "Vec3d.h"
#include "Xform3d.h"
#include "jeTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//================================================================================
//	Structure defines
//================================================================================
typedef struct jeCamera	jeCamera;


//================================================================================
//	Function ProtoTypes
//================================================================================

//-----------------------------------------------------
//	Create/Destroy
//

JETAPI jeCamera *	JETCC jeCamera_Create(jeFloat FovRadians, const jeRect *Rect);
JETAPI void			JETCC jeCamera_Destroy(jeCamera **pCamera);

//-----------------------------------------------------
//	Camera XForm's
//

JETAPI jeBoolean	JETCC jeCamera_SetXForm(jeCamera *Camera, const jeXForm3d *XForm);
JETAPI void			JETCC jeCamera_GetXForm( const jeCamera *Camera,jeXForm3d *pXForm);
JETAPI jeBoolean	JETCC jeCamera_SetTransposeXForm(jeCamera *Camera, const jeXForm3d *XForm);
JETAPI void			JETCC jeCamera_GetTransposeXForm( const jeCamera *Camera,jeXForm3d *pXForm);

JETAPI jeBoolean	JETCC jeCamera_PushXForm(jeCamera *Camera);
JETAPI jeBoolean	JETCC jeCamera_PopXForm( jeCamera *Camera);

//-----------------------------------------------------
//	Misc Get/Set
//

JETAPI void		JETCC jeCamera_GetClippingRect(const jeCamera *Camera, jeRect *Rect);

// Added by Jeff 02/09/05:  Returns Camera's FOV and Rect
JETAPI void     JETCC jeCamera_GetAttributes(jeCamera *Camera, jeFloat *FovRadians, jeRect *Rect);

JETAPI void		JETCC jeCamera_SetAttributes(jeCamera *Camera, jeFloat FovRadians, const jeRect *Rect);
JETAPI void		JETCC jeCamera_SetZScale(jeCamera *Camera, jeFloat ZScale);
JETAPI jeFloat	JETCC jeCamera_GetZScale(const jeCamera *Camera);

// BEGIN - Far clip plane - paradoxnj 2/9/2005
JETAPI void		JETCC jeCamera_SetFarClipPlane(jeCamera *Camera, jeBoolean Enable, jeFloat ZFar);
JETAPI void		JETCC jeCamera_GetFarClipPlane(const jeCamera *Camera, jeBoolean *Enable, jeFloat *ZFar);
// END - Far clip plane - paradoxnj 2/9/2005

//-----------------------------------------------------
//	Transform/Project :
//
JETAPI void JETCC jeCamera_ScreenPointToWorld(	const jeCamera	*Camera,
														int32			 ScreenX,
														int32			 ScreenY,
														jeVec3d			*Vector);
JETAPI void JETCC jeCamera_Project(	const jeCamera	*Camera, 
											const jeVec3d	*PointInCameraSpace, 
											jeVec3d			*ProjectedPoint);
JETAPI void JETCC jeCamera_ProjectArray(const jeCamera	*Camera, 
												const jeVec3d	*FmPoints, 
												int32			FmStride,
												jeVec3d			*ToPoints, 
												int32			ToStride, 
												int32			Count);
JETAPI void JETCC jeCamera_ProjectAndClampArray(const jeCamera	*Camera, 
												const jeVec3d	*FmPoints, 
												int32			FmStride,
												jeVec3d			*ToPoints, 
												int32			ToStride, 
												int32			Count);
JETAPI void JETCC jeCamera_ProjectZ(const jeCamera	*Camera, 
											const jeVec3d	*PointInCameraSpace, 
											jeVec3d			*ProjectedPoint);
											
JETAPI void JETCC jeCamera_ProjectAndClamp(const jeCamera	*Camera, 
										const jeVec3d	*PointInCameraSpace, 
										jeVec3d			*ProjectedPoint);

JETAPI void JETCC jeCamera_Transform(	const jeCamera	*Camera, 
												const jeVec3d	*WorldSpacePoint, 
												jeVec3d			*CameraSpacePoint);
JETAPI void JETCC jeCamera_TransformVecArray(	const jeCamera	*Camera, 
														const jeVec3d	*WorldSpacePointPtr, 
														jeVec3d			*CameraSpacePointPtr,
														int32			Count);

JETAPI void JETCC jeCamera_TransformAndProjectVecArray(	const jeCamera *Camera, 
																const jeVec3d *WorldSpacePointPtr, 
																jeVec3d *ProjectedSpacePointPtr,
																int32 Count);
JETAPI void JETCC jeCamera_TransformAndProjectArray(const jeCamera	*Camera, 
															const jeVec3d	*WorldSpacePointPtr, 
															int32			WorldStride,
															jeVec3d			*ProjectedSpacePointPtr, 
															int32			ProjectedStride,
															int32			Count);
JETAPI void JETCC jeCamera_TransformAndProjectLArray(	const jeCamera		*Camera, 
																const jeLVertex	*WorldSpacePointPtr, 
																jeTLVertex			*ProjectedSpacePointPtr,
																int32				Count);
JETAPI void JETCC jeCamera_TransformAndProject(	const	jeCamera *Camera,
														const	jeVec3d *Point, 
														jeVec3d	*ProjectedPoint);
JETAPI void JETCC jeCamera_TransformAndProjectL(const jeCamera *Camera,
														const jeLVertex *Point, 
														jeTLVertex *ProjectedPoint);

JETAPI void JETCC jeCamera_TransformAndProjectAndClampArray(const jeCamera	*Camera, 
															const jeVec3d	*WorldSpacePointPtr, 
															int32			WorldStride,
															jeVec3d			*ProjectedSpacePointPtr, 
															int32			ProjectedStride,
															int32			Count);
JETAPI void JETCC jeCamera_TransformLArray(	const jeCamera	*Camera, 
																	const jeLVertex		*WorldSpacePointPtr, 
																	jeLVertex			*CameraSpacePointPtr,
																	int32				Count);
JETAPI void JETCC jeCamera_ProjectAndClampLArray(	const jeCamera		*Camera, 
															const jeLVertex		*CameraSpacePointPtr, 
															jeTLVertex			*ProjectedSpacePointPtr,
															int32				Count);
JETAPI void JETCC jeCamera_TransformAndProjectAndClampLArray(	const jeCamera		*Camera, 
																const jeLVertex	*WorldSpacePointPtr, 
																jeTLVertex			*ProjectedSpacePointPtr,
																int32				Count);
JETAPI void JETCC jeCamera_TransformAndProjectAndClamp(	const	jeCamera *Camera,
														const	jeVec3d *Point, 
														jeVec3d	*ProjectedPoint);
JETAPI void JETCC jeCamera_TransformL(	const jeCamera	*Camera,
												const jeLVertex *Point, 
												jeLVertex		*TransformedPoint);
JETAPI void JETCC jeCamera_ProjectAndClampL(const jeCamera	*Camera,
													const jeLVertex *Point, 
													jeTLVertex		*ProjectedPoint);
JETAPI void JETCC jeCamera_TransformAndProjectAndClampL(const jeCamera *Camera,
														const jeLVertex *Point, 
														jeTLVertex *ProjectedPoint);

JETAPI jeVec3d *JETCC jeCamera_GetPov2(jeCamera *Camera);

//-----------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
