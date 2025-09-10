/****************************************************************************************/
/*  JEPLANE.H                                                                           */
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

#ifndef GEPLANE_H
#define GEPLANE_H

#include "BaseType.h"
#include "Vec3d.h"
#include "ExtBox.h"
#include "Xform3d.h"

#ifdef __cplusplus
extern "C" {
#endif

// Update by cjp. defining PLANE_CC as __fastcall on win32
#ifdef WIN32
#define PLANE_CC _fastcall
#else
#define PLANE_CC // unknown platform
#endif

//	jePlane_Type just simply says what axis the plane is aligned on.
//	It does not say what direction it is facing on the aligned axis.

//	DON'T change these numbers!!!!  Doing so will result in BAD things!!!
//	The reason, is code uses these enums to index into the X,Y,Z elements:
//		Val = jeVec3d_GetElement(&Plane->Normal, Plane->Type%Type_AnyX);
typedef enum
{
	Type_X=0,							// X aligned
	Type_Y=1,							// Y aligned
	Type_Z=2,							// Z aligned
	Type_AnyX=3,						// Arbitrary, X dominent axis
	Type_AnyY=4,						// Arbitrary, Y dominent axis
	Type_AnyZ=5,						// Arbitrary, Z dominent axis
	Type_Any=6,							// Arbitrary (Any axis)
} jePlane_Type;

typedef uint8			jePlane_Side;

// flags for plane on side types
#define PSIDE_FRONT		(1<<0)
#define PSIDE_BACK		(1<<1)
#define PSIDE_FACING	(1<<2)

#define PSIDE_BOTH		(PSIDE_FRONT|PSIDE_BACK)

typedef struct jePlane
{
	jeVec3d			Normal;				// Unit Orientation
	float			Dist;				// Distance from origin
	jePlane_Type	Type;				// jePlane_Type
} jePlane;


JETAPI void	JETCC jePlane_SetFromVerts(jePlane *Plane, const jeVec3d *V1, const jeVec3d *V2, const jeVec3d *V3);
JETAPI void	JETCC jePlane_Inverse(jePlane *Plane);
JETAPI void	JETCC jePlane_Rotate(const jePlane *In, const jeXForm3d *XForm, jePlane *Out);
JETAPI void	JETCC jePlane_Transform(const jePlane *In, const jeXForm3d *XForm, jePlane *Out);
JETAPI void JETCC jePlane_TransformRenorm(const jePlane *In, const jeXForm3d *XForm, jePlane *Out);
jePlane_Type	jePlane_TypeFromUnitVector(const jeVec3d *V1);
JETAPI jeBoolean JETCC jePlane_GetAAVectors(const jePlane *Plane, jeVec3d *Xv, jeVec3d *Yv);

float PLANE_CC jePlane_PointDistance(const jePlane *Plane, const jeVec3d *Point);
float PLANE_CC jePlane_PointDistanceFast(const jePlane *Plane, const jeVec3d *Point);

jePlane_Side	jePlane_BoxSide(const jePlane *Plane, const jeExtBox *Box, jeFloat Epsilon);
jeBoolean		jePlane_Compare(const jePlane *Plane1, const jePlane *Plane2, float NEpsilon, float DEpsilon);
	// NEpsilon = Normal Epsilon, DEpsilon = Dist Epsilon
#ifdef __cplusplus
}
#endif

#endif
