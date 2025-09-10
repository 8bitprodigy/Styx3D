/*!
  @file jeRay.h

  @author Anthony Rufrano (paradoxnj)
  @brief Ray casting code
                                                                   
  The contents of this file are subject to the Jet3D Public License
  Version 1.02 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.jet3d.com
                                                                     
  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
  the License for the specific language governing rights and limitations
  under the License.
                                 
  The Original Code is Jet3D, released December 12, 1999.
  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved
*/

#ifndef JE_RAY_H
#define JE_RAY_H

#include "BaseType.h"
#include "Vec3d.h"
#include "ExtBox.h"
#include "jePlane.h"

/*!
	@struct jeRay
	@brief Represents a ray in 3D space
*/
typedef struct jeRay
{
	jeVec3d						Origin;			///< Where the ray begins
	jeVec3d						Direction;		///< The direction the ray is pointing
} jeRay;

/*!
	@fn jeRay_Set(jeRay *Ray, jeVec3d *Origin, jeVec3d *Dir)
	@brief Sets the ray's data
	@param[in] Ray The ray to modify
	@param[in] Origin The point at which the ray begins
	@param[in] Dir The direction the ray is pointing
*/
JETAPI void JETCC jeRay_Set(jeRay *Ray, jeVec3d *Origin, jeVec3d *Dir);

/*!
	@fn void jeRay_Get(const jeRay *Ray, jeVec3d *Origin, jeVec3d *Dir)
	@brief Gets the ray's data
	@param[in] Ray The ray to query
	@param[out] Origin The point at which the ray begins
	@param[out] Dir The direction the ray is pointing
*/
JETAPI void JETCC jeRay_Get(const jeRay *Ray, jeVec3d *Origin, jeVec3d *Dir);

/*!
	@fn jeBoolean jeRay_IntersectsWithTriangle(const jeRay *Ray, const jeVec3d *V1, const jeVec3d *V2, const jeVec3d *V3, jeBoolean Cull, float *T, jeVec3d *Impact)
	@brief Checks if a ray is intersecting with a triangle and returns the impact point
	@param[in] Ray The ray to query
	@param[in] V1 A triangle vertex
	@param[in] V2 A triangle vertex
	@param[in] V3 A triangle vertex
	@param[in] Cull Flag to tell the function to cull the ray at the impact point
	@param[out] T The distance from the ray's origin to the point of intersection
	@param[out] Impact The point of impact
	@return JE_TRUE if a collision occurred, JE_FALSE if not
*/
JETAPI jeBoolean JETCC jeRay_IntersectsWithTriangle(const jeRay *Ray, const jeVec3d *V1, const jeVec3d *V2, const jeVec3d *V3, jeBoolean Cull, float *T, jeVec3d *Impact);

/*!
	@fn jeBoolean jeRay_IntersectsWithPlane(const jeRay *Ray, const jePlane *Plane, jeBoolean Cull, float *T, jeVec3d *Impact)
	@brief Checks if a ray intersects with a plane
	@param[in] Ray The ray to check with
	@param[in] Plane The plane to check against
	@param[in] Cull Flag to tell the function to cull the ray at the point of impact
	@param[out] T The distance from the ray's origin to the impact point
	@param[out] Impact The impact point
	@return JE_TRUE if a collision occurred, JE_FALSE if not
*/
JETAPI jeBoolean JETCC jeRay_IntersectsWithPlane(const jeRay *Ray, const jePlane *Plane, jeBoolean Cull, float *T, jeVec3d *Impact);

/*!
	@fn jeBoolean jeRay_IntersectsWithExtBox(const jeRay *Ray, const jeExtBox *ExtBox, jeBoolean Cull, float *T, jeVec3d *Impact)
	@brief Checks if a ray intersects with an axis-aligned bounding box
	@param[in] Ray The ray to check with
	@param[in] ExtBox The bounding box to check against
	@param[in] Cull Flag to tell the function to cull the ray at the point of impact
	@param[out] T The distance from the ray's origin to the point of impact
	@param[out] Impact The impact point
	@return JE_TRUE if a collision occurred, JE_FALSE if not
*/
JETAPI jeBoolean JETCC jeRay_IntersectsWithExtBox(const jeRay *Ray, const jeExtBox *ExtBox, jeBoolean Cull, float *T, jeVec3d *Impact);

#endif
