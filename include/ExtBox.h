/****************************************************************************************/
/*  EXTBOX.H                                                                            */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: Axial aligned bounding box (extent box) support                        */
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
#ifndef JE_EXTBOX_H
#define JE_EXTBOX_H

#include "BaseType.h"
#include "Vec3d.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct jeExtBox
{
	jeVec3d Min;
	jeVec3d Max;
} jeExtBox;

// Set the values in a box
JETAPI void JETCC jeExtBox_Set (  jeExtBox *B,
				  jeFloat X1,	  jeFloat Y1,	  jeFloat Z1,
				  jeFloat X2,	  jeFloat Y2,	  jeFloat Z2 );

// Test a box for validity ( non NULL and max >= min )
JETAPI jeBoolean JETCC jeExtBox_IsValid(  const jeExtBox *B );

// Added by Icestorm
// Test a box for point-degeneration: Min=Max
JETAPI jeBoolean JETCC jeExtBox_IsPoint(  const jeExtBox *B );

// Set box Min and Max to the passed point
JETAPI void JETCC jeExtBox_SetToPoint ( jeExtBox *B, const jeVec3d *Point );

// Extend a box to encompass the passed point
JETAPI void JETCC jeExtBox_ExtendToEnclose( jeExtBox *B, const jeVec3d *Point );

// Return result of box intersection.
// If no intersection, returns JE_FALSE and bResult is not modified.
// If intersection, returns JE_TRUE and fills bResult (if not NULL)
// with the intersected box,
// bResult may be one of b1 or b2.
// 
JETAPI jeBoolean JETCC jeExtBox_Intersection ( const jeExtBox *B1, const jeExtBox *B2, jeExtBox *Result	);

// computes union of b1 and b2 and returns in bResult.
JETAPI void JETCC jeExtBox_Union ( const jeExtBox *B1, const jeExtBox *B2, jeExtBox *Result );

JETAPI jeBoolean JETCC jeExtBox_ContainsPoint ( const jeExtBox *B, const jeVec3d  *Point );

// CB note : there are some really bad names in here.  GetTranslation ? Scaling ? what?
JETAPI void JETCC jeExtBox_GetTranslation ( const jeExtBox *B,       jeVec3d *pCenter );
JETAPI void JETCC jeExtBox_SetTranslation (       jeExtBox *B, const jeVec3d *pCenter );
JETAPI void JETCC jeExtBox_Translate      (       jeExtBox *B, jeFloat DX, jeFloat DY, jeFloat DZ );

// Icestorm Begin

// Make pOrigin the new origin of the box => B relative to pOrigin
JETAPI void JETCC jeExtBox_SetNewOrigin( jeExtBox *B, const jeVec3d *pOrigin);

// Make B's center the new origin of the box => Make B realtive to it's center
// Also if OldCenter is not NULL, save old position/center
JETAPI void JETCC jeExtBox_MoveToOrigin( jeExtBox *B, jeVec3d *OldCenter );

// Same as: First jeExtBox_Translate by vMove then jeExtBox_MoveToOrigin
JETAPI void JETCC jeExtBox_TranslateAndMoveToOrigin( jeExtBox *B, const jeVec3d *vMove, jeVec3d *MovedCenter );

// Icestorm End

JETAPI void JETCC jeExtBox_GetScaling     ( const jeExtBox *B,       jeVec3d *pScale );
JETAPI void JETCC jeExtBox_SetScaling     (       jeExtBox *B, const jeVec3d *pScale );
JETAPI void JETCC jeExtBox_Scale          (       jeExtBox *B, jeFloat DX, jeFloat DY,jeFloat DZ );

//  Creates a box that encloses the entire area of a box that moves along linear path
JETAPI void JETCC jeExtBox_LinearSweep(	const jeExtBox *BoxToSweep, 
						const jeVec3d *StartPoint, 
						const jeVec3d *EndPoint, 
						jeExtBox *EnclosingBox );

// Collides a ray with box B.  The ray is directed, from Start to End.  
//   Only returns a ray hitting the outside of the box.  
//     on success, JE_TRUE is returned, and 
//       if T is non-NULL, T is returned as 0..1 where 0 is a collision at Start, and 1 is a collision at End
//       if Normal is non-NULL, Normal is the surface normal of the box where the collision occured.
JETAPI jeBoolean JETCC jeExtBox_RayCollision( const jeExtBox *B, const jeVec3d *Start, const jeVec3d *End, 
								jeFloat *T, jeVec3d *Normal );

JETAPI void JETCC jeExtBox_GetPoint( const jeExtBox *B, const int iPoint, jeVec3d *vPoint);

// Added by Icestorm (fast, rewritten version of Incarnadine's one)
// (ca. 7-12 times faster than old ver.)
// ----------------------------------------
// Collides a moving box (or ray) against a stationary box.  The moving box
// must be relative to the path and move from Start to End.
//   Only returns a ray/box hitting the outside of the box.  
//     on success, JE_TRUE is returned, and 
//       if T is non-NULL, T is returned as 0..1 where 0 is a collision at Start, and 1 is a collision at End
//       if Normal is non-NULL, Normal is the surface normal of the box where the collision occured.
JETAPI jeBoolean JETCC jeExtBox_Collision(	const jeExtBox *B, const jeExtBox *MovingBox,
											const jeVec3d *Start, const jeVec3d *End, 
											jeFloat *T, jeVec3d *Normal );

// Added by Icestorm
// ----------------------------------------
// Collides a changing box against a stationary box.  The changing box
// must be relative to Pos.
//   Only returns a box hitting the outside of the box.  
//     on success, JE_TRUE is returned, and 
//       if T is non-NULL, T is returned as 0..1 where 0 is a collision at Start, and 1 is a collision at End
//       if Normal is non-NULL, Normal is the surfacenormal of the box where the collision occured.
//       if Point is non-NULL, Point is a point of the surface where the collision occured.
JETAPI jeBoolean JETCC jeExtBox_ChangeBoxCollision(	const jeExtBox *B, const jeVec3d *Pos,
													const jeExtBox *StartBox, const jeExtBox *EndBox,
													jeFloat *T, jeVec3d *Normal, jeVec3d *Point );

#ifdef __cplusplus
	}
#endif

#endif
		