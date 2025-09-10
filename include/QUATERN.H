/****************************************************************************************/
/*  QUATERN.H                                                                           */
/*                                                                                      */
/*  Author: Mike Sandige                                                                */
/*  Description: Quaternion mathematical system interface                               */
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
#ifndef JE_QUATERNION_H
#define JE_QUATERNION_H

/***************************************************************************

	the quatern module contains basic support for a quaternion object.

	quaternions are an extension of complex numbers that allows an
	expression for rotation that can be easily interpolated.  jeQuaternion_s are also 
	more numericaly stable for repeated rotations than matrices.

	
	A quaternion is a 4 element 'vector'  [w,x,y,z] where:

	q = w + xi + yj + zk
	i*i = -1
	j*j = -1
	k*k = -1
	i*j = -j*i = k
	j*k = -k*j = i
	k*i = -i*k = j
	q' (conjugate) = w - xi - yj - zk
	||q|| (magnitude) = sqrt(q*q') = sqrt(w*w + x*x + y*y + z*z)
	unit quaternion ||q|| == 1; this implies  q' == qinverse 
	quaternions are associative (q1*q2)*q3 == q1*(q2*q3)
	quaternions are not commutative  q1*q2 != q2*q1
	qinverse (inverse (1/q) ) = q'/(q*q')
	
	q can be expressed by w + xi + yj + zk or [w,x,y,z] 
	or as in this implementation (s,v) where s=w, and v=[x,y,z]

	quaternions can represent a rotation.  The rotation is an angle t, around a 
	unit vector u.   q=(s,v);  s= cos(t/2);   v= u*sin(t/2).

	quaternions can apply the rotation to a point.  let the point be p [px,py,pz],
	and let P be a quaternion(0,p).  Protated = q*P*qinverse 
	( Protated = q*P*q' if q is a unit quaternion)

	concatenation rotations is similar to matrix concatenation.  given two rotations
	q1 and q2,  to rotate by q1, then q2:  let qc = (q2*q1), then the combined 
	rotation is given by qc*P*qcinverse (= qc*P*qc' if q is a unit quaternion)

	multiplication: 
	q1 = w1 + x1i + y1j + z1k
	q2 = w2 + x2i + y2j + z2k
	q1*q2 = q3 =
			(w1*w2 - x1*x2 - y1*y2 - z1*z2)     {w3}
	        (w1*x2 + x1*w2 + y1*z2 - z1*y2)i	{x3}
			(w1*y2 - x1*z2 + y1*w2 + z1*x2)j    {y3}
			(w1*z2 + x1*y2 + y1*x2 + z1*w2)k	{z3}

	also, 
	q1 = (s1,v1) = [s1,(x1,y1,z1)]
	q2 = (s2,v2) = [s2,(x2,y2,z2)]
	q1*q2 = q3	=	(s1*s2 - dot_product(v1,v2),			{s3}
					(s1*v2 + s2*v1 + cross_product(v1,v2))	{v3}


	interpolation - it is possible (and sometimes reasonable) to interpolate between
	two quaternions by interpolating each component.  This does not quarantee a 
	resulting unit quaternion, and will result in an animation with non-linear 
	rotational velocity.

	spherical interpolation: (slerp) treat the quaternions as vectors 
	find the angle between them (w = arccos(q1 dot q2) ).
	given 0<=t<=1,  q(t) = q1*(sin((1-t)*w)/sin(w) + q2 * sin(t*w)/sin(w).
	since q == -q, care must be taken to rotate the proper way.  

	this implementation uses the notation quaternion q = (quatS,quatV) 
	  where quatS is a scalar, and quatV is a 3 element vector.

********************************************/

#include "BaseType.h"
#include "Xform3d.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
	jeFloat	W;
	jeFloat   X,Y,Z;
	//jeVec3d   QuatV;
} jeQuaternion;


#define	QUATERNION_PI	(JE_PI)

JETAPI jeBoolean JETCC jeQuaternion_IsValid( const jeQuaternion *Q );
	// return JE_TRUE if Q is non null and for has no NAN's in its components

JETAPI void JETCC jeQuaternion_Set( jeQuaternion *Q, jeFloat W, jeFloat X, jeFloat Y, jeFloat Z);
	// set quaternion components.  Doesn't normalize
JETAPI void JETCC jeQuaternion_SetVec3d( jeQuaternion *Q, jeFloat W, const jeVec3d *V);
	// set quaternion components.  Doesn't normalize
JETAPI void JETCC jeQuaternion_SetFromAxisAngle(jeQuaternion *Q, const jeVec3d *Axis, jeFloat Theta);
	// set a quaternion from an axis and a rotation around the axis
JETAPI jeBoolean JETCC jeQuaternion_GetAxisAngle(const jeQuaternion *Q, jeVec3d *Axis, jeFloat *Theta);
	// gets an axis and angle of rotation around the axis from a quaternion
	// returns JE_TRUE if there is an axis.  
	// returns JE_FALSE if there is no axis (and Axis is set to 0,0,0, and Theta is 0)

JETAPI void JETCC jeQuaternion_Get( const jeQuaternion *Q, 
					jeFloat *W, jeFloat *X, jeFloat *Y, jeFloat *Z);
	// get quaternion components into W,X,Y,Z
JETAPI void JETCC jeQuaternion_GetVec3d( const jeQuaternion *Q, jeFloat *W, jeVec3d *V);
	// get quaternion components into W and V

JETAPI void JETCC jeQuaternion_FromMatrix(
	const jeXForm3d		*RotationMatrix,
	      jeQuaternion	*QDest);
	// takes upper 3 by 3 portion of matrix (rotation sub matrix) 
	// and generates a quaternion

JETAPI void JETCC jeQuaternion_ToMatrix(
	const jeQuaternion	*Q, 
		  jeXForm3d		*RotationMatrixDest);
	// takes a unit quaternion and makes RotationMatrixDest an equivelant rotation xform.
	// (any translation in RotationMatrixDest will be list)

JETAPI void JETCC jeQuaternion_Slerp(
	const jeQuaternion		*Q0, 
	const jeQuaternion		*Q1, 
	jeFloat					T,		
	jeQuaternion			*QT);
	// spherical interpolation between q0 and q1.   0<=t<=1 
	// resulting quaternion is 'between' q0 and q1
	// with t==0 being all q0, and t==1 being all q1.
	// returns a quaternion with a positive W - always takes shortest route
	// through the positive W domain.

JETAPI void JETCC jeQuaternion_SlerpNotShortest(
	const jeQuaternion		*Q0, 
	const jeQuaternion		*Q1, 
	jeFloat					T,		
	jeQuaternion			*QT);
	// spherical interpolation between q0 and q1.   0<=t<=1 
	// resulting quaternion is 'between' q0 and q1
	// with t==0 being all q0, and t==1 being all q1.


JETAPI void JETCC jeQuaternion_Multiply(
	const jeQuaternion	*Q1, 
	const jeQuaternion	*Q2, 
	jeQuaternion			*QProduct);
	// multiplies q1 * q2, and places the result in q.
	// no failure. 	renormalization not automatic

JETAPI void JETCC jeQuaternion_Rotate(
	const jeQuaternion	*Q, 
	const jeVec3d       *V, 
	jeVec3d				*VRotated);
	// Rotates V by the quaternion Q, places the result in VRotated.

JETAPI jeBoolean JETCC jeQuaternion_IsUnit(const jeQuaternion *Q);
	// returns JE_TRUE if q is a unit quaternion.  JE_FALSE otherwise.

JETAPI jeFloat JETCC jeQuaternion_Normalize(jeQuaternion *Q);
	// normalizes q to be a unit quaternion.  returns original magnitude of q

JETAPI void JETCC jeQuaternion_Copy(const jeQuaternion *QSrc, jeQuaternion *QDst);
	// copies quaternion QSrc into QDst

JETAPI void JETCC jeQuaternion_SetNoRotation(jeQuaternion *Q);
	// sets Q to be a quaternion with no rotation (like an identity matrix)

JETAPI void JETCC jeQuaternion_Ln(
	const jeQuaternion *Q, 
	jeQuaternion *LnQ);
	// ln(Q) for unit quaternion only!

JETAPI void JETCC jeQuaternion_Exp(
	const jeQuaternion *Q,
	jeQuaternion *ExpQ);
	// exp(Q) for pure quaternion only!  (zero scalar part (W))

JETAPI void JETCC jeQuaternion_Scale(
	const jeQuaternion *Q,
	jeFloat Scale,
	jeQuaternion *QScaled);
	// Q = Q * Scale  (result is not generally a unit quaternion!)

JETAPI void JETCC jeQuaternion_Add(
	const jeQuaternion *Q1,
	const jeQuaternion *Q2,
	jeQuaternion *QSum);
	// QSum = Q1 + Q2  (result is not generally a unit quaternion!)

JETAPI void JETCC jeQuaternion_Subtract(
	const jeQuaternion *Q1, 
	const jeQuaternion *Q2, 
	jeQuaternion *QSum);
	// QSum = Q1 - Q2  (result is not generally a unit quaternion!)

JETAPI void JETCC jeQuaternion_Inverse(const jeQuaternion *Q, jeQuaternion *QInv);
	// sets QInv to the inverse of Q.  

JETAPI jeFloat JETCC jeQuaternion_Magnitude(const jeQuaternion *Q);
	// returns Magnitude of Q.  

JETAPI jeBoolean JETCC jeQuaternion_Compare( jeQuaternion *Q1, jeQuaternion *Q2, jeFloat Tolerance );
	// return JE_TRUE if quaternions differ elementwise by less than Tolerance.


#ifndef NDEBUG
JETAPI void JETCC jeQuaternion_SetMaximalAssertionMode( jeBoolean Enable );
#endif

#ifdef __cplusplus
}
#endif

#endif
