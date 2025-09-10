/****************************************************************************************/
/*  QUATERN.C                                                                           */
/*                                                                                      */
/*  Author: Mike Sandige                                                                */
/*  Description: Quaternion mathematical system implementation                          */
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
#include <assert.h>
#include <math.h>

#include "BaseType.h"
#include "Quatern.h"


#ifndef NDEBUG
	static jeBoolean jeQuaternion_MaximalAssertionMode = JE_TRUE;
	#define jeQuaternion_Assert if (jeQuaternion_MaximalAssertionMode) assert

	JETAPI void JETCC jeQuaternion_SetMaximalAssertionMode( jeBoolean Enable )
	{
		assert( (Enable == JE_TRUE) || (Enable == JE_FALSE) );
		jeQuaternion_MaximalAssertionMode = Enable;
	}
#else
	#define jeQuaternion_Assert assert
#endif

#define UNIT_TOLERANCE 0.001  
	// Quaternion magnitude must be closer than this tolerance to 1.0 to be 
	// considered a unit quaternion

#define QZERO_TOLERANCE 0.00001 
	// quaternion magnitude must be farther from this tolerance to 0.0 to be 
	// normalized

#define TRACE_QZERO_TOLERANCE 0.1
	// trace of matrix must be greater than this to be used for converting a matrix
	// to a quaternion.

#define AA_QZERO_TOLERANCE 0.0001
	

JETAPI jeBoolean JETCC jeQuaternion_IsValid(const jeQuaternion *Q)
{
	if (Q == NULL)
		return JE_FALSE;
	if ((Q->W * Q->W) < 0.0f)
		return JE_FALSE;
	if ((Q->X * Q->X) < 0.0f)
		return JE_FALSE;
	if ((Q->Y * Q->Y) < 0.0f)
		return JE_FALSE;
	if ((Q->Z * Q->Z) < 0.0f)
		return JE_FALSE;
	return JE_TRUE;
}

JETAPI void JETCC jeQuaternion_Set( 
	jeQuaternion *Q, jeFloat W, jeFloat X, jeFloat Y, jeFloat Z)
{
	assert( Q != NULL );

	Q->W = W;
	Q->X = X;
	Q->Y = Y;
	Q->Z = Z;
	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
}

JETAPI void JETCC jeQuaternion_SetVec3d(
	jeQuaternion *Q, jeFloat W, const jeVec3d *V)
{
	assert( Q != NULL );
	assert( jeVec3d_IsValid(V) != JE_FALSE );

	Q->W = W;
	Q->X = V->X;
	Q->Y = V->Y;
	Q->Z = V->Z;
}	

JETAPI void JETCC jeQuaternion_Get( 
	const jeQuaternion *Q, 
	jeFloat *W, 
	jeFloat *X, 
	jeFloat *Y, 
	jeFloat *Z)
	// get quaternion components into W,X,Y,Z
{
	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	assert( W != NULL );
	assert( X != NULL );
	assert( Y != NULL );
	assert( Z != NULL );

	*W = Q->W;
	*X = Q->X;
	*Y = Q->Y;
	*Z = Q->Z;
}

JETAPI void JETCC jeQuaternion_SetFromAxisAngle(jeQuaternion *Q, const jeVec3d *Axis, jeFloat Theta)
	// set a quaternion from an axis and a rotation around the axis
{
	jeFloat sinTheta;
	assert( Q != NULL);
	assert( jeVec3d_IsValid(Axis) != JE_FALSE);
	assert( (Theta * Theta) >= 0.0f );
	assert( ( fabs(jeVec3d_Length(Axis)-1.0f) < AA_QZERO_TOLERANCE) );
	
	Theta = Theta * (jeFloat)0.5f;
	Q->W     = (jeFloat) cos(Theta);
	sinTheta = (jeFloat) sin(Theta);
	Q->X = sinTheta * Axis->X;
	Q->Y = sinTheta * Axis->Y;
	Q->Z = sinTheta * Axis->Z;

	jeQuaternion_Assert( jeQuaternion_IsUnit(Q) == JE_TRUE );
}


JETAPI jeBoolean JETCC jeQuaternion_GetAxisAngle(const jeQuaternion *Q, jeVec3d *Axis, jeFloat *Theta)
{	
	float OneOverSinTheta;
	float HalfTheta;
	assert( Q != NULL );
	assert( Axis != NULL );
	assert( Theta != NULL );
	jeQuaternion_Assert( jeQuaternion_IsUnit(Q) != JE_FALSE );
	
	HalfTheta  = (jeFloat)acos( Q->W );
	if (HalfTheta>QZERO_TOLERANCE)
		{
			OneOverSinTheta = 1.0f / (jeFloat)sin( HalfTheta );
			Axis->X = OneOverSinTheta * Q->X;
			Axis->Y = OneOverSinTheta * Q->Y;
			Axis->Z = OneOverSinTheta * Q->Z;
			*Theta = 2.0f * HalfTheta;
			jeQuaternion_Assert( jeVec3d_IsValid(Axis) != JE_FALSE );
			jeQuaternion_Assert( (*Theta * *Theta) >= 0.0f);
			return JE_TRUE;
		}
	else
		{
			Axis->X = Axis->Y = Axis->Z = 0.0f;
			*Theta = 0.0f;
			return JE_FALSE;
		}
}


JETAPI void JETCC jeQuaternion_GetVec3d( 
	const jeQuaternion *Q, 
	jeFloat *W, 
	jeVec3d *V)
	// get quaternion components into W and V
{
	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	assert( W != NULL );
	assert( V != NULL );
	
	*W   = Q->W;
	V->X = Q->X;
	V->Y = Q->Y;
	V->Z = Q->Z;
}


JETAPI void JETCC jeQuaternion_FromMatrix(
	const jeXForm3d		*M,
	      jeQuaternion	*Q)
	// takes upper 3 by 3 portion of matrix (rotation sub matrix) 
	// and generates a quaternion
{
	jeFloat trace,s;

	assert( M != NULL );
	assert( Q != NULL );
	jeQuaternion_Assert( jeXForm3d_IsOrthonormal(M)==JE_TRUE );

	trace = M->AX + M->BY + M->CZ;
	if (trace > 0.0f)
		{
			s = (jeFloat)sqrt(trace + 1.0f);
			Q->W = s * 0.5f;
			s = 0.5f / s;

			Q->X = (M->CY - M->BZ) * s;
			Q->Y = (M->AZ - M->CX) * s;
			Q->Z = (M->BX - M->AY) * s;
		}
	else
		{
			int biggest;
			enum {A,E,I};
			if (M->AX > M->BY)
				{
					if (M->CZ > M->AX)
						biggest = I;	
					else
						biggest = A;
				}
			else
				{
					if (M->CZ > M->AX)
						biggest = I;
					else
						biggest = E;
				}

			// in the unusual case the original trace fails to produce a good sqrt, try others...
			switch (biggest)
				{
				case A:
					s = (jeFloat)sqrt( M->AX - (M->BY + M->CZ) + 1.0);
					if (s > TRACE_QZERO_TOLERANCE)
						{
							Q->X = s * 0.5f;
							s = 0.5f / s;
							Q->W = (M->CY - M->BZ) * s;
							Q->Y = (M->AY + M->BX) * s;
							Q->Z = (M->AZ + M->CX) * s;
							break;
						}
							// I
							s = (jeFloat)sqrt( M->CZ - (M->AX + M->BY) + 1.0);
							if (s > TRACE_QZERO_TOLERANCE)
								{
									Q->Z = s * 0.5f;
									s = 0.5f / s;
									Q->W = (M->BX - M->AY) * s;
									Q->X = (M->CX + M->AZ) * s;
									Q->Y = (M->CY + M->BZ) * s;
									break;
								}
							// E
							s = (jeFloat)sqrt( M->BY - (M->CZ + M->AX) + 1.0);
							if (s > TRACE_QZERO_TOLERANCE)
								{
									Q->Y = s * 0.5f;
									s = 0.5f / s;
									Q->W = (M->AZ - M->CX) * s;
									Q->Z = (M->BZ + M->CY) * s;
									Q->X = (M->BX + M->AY) * s;
									break;
								}
							break;
				case E:
					s = (jeFloat)sqrt( M->BY - (M->CZ + M->AX) + 1.0);
					if (s > TRACE_QZERO_TOLERANCE)
						{
							Q->Y = s * 0.5f;
							s = 0.5f / s;
							Q->W = (M->AZ - M->CX) * s;
							Q->Z = (M->BZ + M->CY) * s;
							Q->X = (M->BX + M->AY) * s;
							break;
						}
							// I
							s = (jeFloat)sqrt( M->CZ - (M->AX + M->BY) + 1.0);
							if (s > TRACE_QZERO_TOLERANCE)
								{
									Q->Z = s * 0.5f;
									s = 0.5f / s;
									Q->W = (M->BX - M->AY) * s;
									Q->X = (M->CX + M->AZ) * s;
									Q->Y = (M->CY + M->BZ) * s;
									break;
								}
							// A
							s = (jeFloat)sqrt( M->AX - (M->BY + M->CZ) + 1.0);
							if (s > TRACE_QZERO_TOLERANCE)
								{
									Q->X = s * 0.5f;
									s = 0.5f / s;
									Q->W = (M->CY - M->BZ) * s;
									Q->Y = (M->AY + M->BX) * s;
									Q->Z = (M->AZ + M->CX) * s;
									break;
								}
					break;
				case I:
					s = (jeFloat)sqrt( M->CZ - (M->AX + M->BY) + 1.0);
					if (s > TRACE_QZERO_TOLERANCE)
						{
							Q->Z = s * 0.5f;
							s = 0.5f / s;
							Q->W = (M->BX - M->AY) * s;
							Q->X = (M->CX + M->AZ) * s;
							Q->Y = (M->CY + M->BZ) * s;
							break;
						}
							// A
							s = (jeFloat)sqrt( M->AX - (M->BY + M->CZ) + 1.0);
							if (s > TRACE_QZERO_TOLERANCE)
								{
									Q->X = s * 0.5f;
									s = 0.5f / s;
									Q->W = (M->CY - M->BZ) * s;
									Q->Y = (M->AY + M->BX) * s;
									Q->Z = (M->AZ + M->CX) * s;
									break;
								}
							// E
							s = (jeFloat)sqrt( M->BY - (M->CZ + M->AX) + 1.0);
							if (s > TRACE_QZERO_TOLERANCE)
								{
									Q->Y = s * 0.5f;
									s = 0.5f / s;
									Q->W = (M->AZ - M->CX) * s;
									Q->Z = (M->BZ + M->CY) * s;
									Q->X = (M->BX + M->AY) * s;
									break;
								}
					break;
				default:
					assert(0);
				}
		}
	jeQuaternion_Assert( jeQuaternion_IsUnit(Q) == JE_TRUE );
}

JETAPI void JETCC jeQuaternion_ToMatrix(
	const jeQuaternion	*Q, 
		  jeXForm3d		*M)
	// takes a unit quaternion and fills out an equivelant rotation
	// portion of a xform
{
	jeFloat X2,Y2,Z2;		//2*QX, 2*QY, 2*QZ
	jeFloat XX2,YY2,ZZ2;	//2*QX*QX, 2*QY*QY, 2*QZ*QZ
	jeFloat XY2,XZ2,XW2;	//2*QX*QY, 2*QX*QZ, 2*QX*QW
	jeFloat YZ2,YW2,ZW2;	// ...

	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	assert( M != NULL );
	jeQuaternion_Assert( jeQuaternion_IsUnit(Q) == JE_TRUE );
	
	
	X2  = 2.0f * Q->X;
	XX2 = X2   * Q->X;
	XY2 = X2   * Q->Y;
	XZ2 = X2   * Q->Z;
	XW2 = X2   * Q->W;

	Y2  = 2.0f * Q->Y;
	YY2 = Y2   * Q->Y;
	YZ2 = Y2   * Q->Z;
	YW2 = Y2   * Q->W;
	
	Z2  = 2.0f * Q->Z;
	ZZ2 = Z2   * Q->Z;
	ZW2 = Z2   * Q->W;
	
	M->AX = 1.0f - YY2 - ZZ2;
	M->AY = XY2  - ZW2;
	M->AZ = XZ2  + YW2;

	M->BX = XY2  + ZW2;
	M->BY = 1.0f - XX2 - ZZ2;
	M->BZ = YZ2  - XW2;

	M->CX = XZ2  - YW2;
	M->CY = YZ2  + XW2;
	M->CZ = 1.0f - XX2 - YY2;

	M->Translation.X = M->Translation.Y = M->Translation.Z = 0.0f;

#ifdef USE_CONVENTIONS
	M->Convention = JE_XFORM3D_RIGHT_HANDED;
#endif

	jeQuaternion_Assert( jeXForm3d_IsOrthonormal(M)==JE_TRUE );

}


#define EPSILON (0.00001)



JETAPI void JETCC jeQuaternion_Slerp(
	const jeQuaternion		*Q0, 
	const jeQuaternion		*Q1, 
	jeFloat					T,		
	jeQuaternion			*QT)
	// spherical interpolation between q0 and q1.   0<=t<=1 
	// resulting quaternion is 'between' q0 and q1
	// with t==0 being all q0, and t==1 being all q1.
{
	jeFloat omega,cosom,sinom,Scale0,Scale1;
	jeQuaternion QL;
	assert( Q0 != NULL );
	assert( Q1 != NULL );
	assert( QT  != NULL );
	assert( ( 0 <= T ) && ( T <= 1.0f ) );
	jeQuaternion_Assert( jeQuaternion_IsUnit(Q0) == JE_TRUE );
	jeQuaternion_Assert( jeQuaternion_IsUnit(Q1) == JE_TRUE );

	cosom =		(Q0->W * Q1->W) + (Q0->X * Q1->X) 
			  + (Q0->Y * Q1->Y) + (Q0->Z * Q1->Z);

	if (cosom < 0)
		{
			cosom = -cosom;
			QL.X = -Q1->X;
			QL.Y = -Q1->Y;
			QL.Z = -Q1->Z;
			QL.W = -Q1->W;
		}
	else
		{
			QL = *Q1;
		}
			

	if ( (1.0f - cosom) > EPSILON )
		{
			omega  = (jeFloat) acos( cosom );
			sinom  = (jeFloat) sin( omega );
			Scale0 = (jeFloat) sin( (1.0f-T) * omega) / sinom;
			Scale1 = (jeFloat) sin( T*omega) / sinom;
		}
	else
		{
			// has numerical difficulties around cosom == 0
			// in this case degenerate to linear interpolation
		
			Scale0 = 1.0f - T;
			Scale1 = T;
		}


	QT-> X = Scale0 * Q0->X + Scale1 * QL.X;
	QT-> Y = Scale0 * Q0->Y + Scale1 * QL.Y;
	QT-> Z = Scale0 * Q0->Z + Scale1 * QL.Z;
	QT-> W = Scale0 * Q0->W + Scale1 * QL.W;
	jeQuaternion_Assert( jeQuaternion_IsUnit(QT) == JE_TRUE );
}




JETAPI void JETCC jeQuaternion_SlerpNotShortest(
	const jeQuaternion		*Q0, 
	const jeQuaternion		*Q1, 
	jeFloat					T,		
	jeQuaternion			*QT)
	// spherical interpolation between q0 and q1.   0<=t<=1 
	// resulting quaternion is 'between' q0 and q1
	// with t==0 being all q0, and t==1 being all q1.
{
	jeFloat omega,cosom,sinom,Scale0,Scale1;
	assert( Q0 != NULL );
	assert( Q1 != NULL );
	assert( QT  != NULL );
	assert( ( 0 <= T ) && ( T <= 1.0f ) );
	jeQuaternion_Assert( jeQuaternion_IsUnit(Q0) == JE_TRUE );
	jeQuaternion_Assert( jeQuaternion_IsUnit(Q1) == JE_TRUE );

	cosom =		(Q0->W * Q1->W) + (Q0->X * Q1->X) 
			  + (Q0->Y * Q1->Y) + (Q0->Z * Q1->Z);
	if ( (1.0f + cosom) > EPSILON )
		{
			if ( (1.0f - cosom) > EPSILON )
				{
					omega  = (jeFloat) acos( cosom );
					sinom  = (jeFloat) sin( omega );
					// has numerical difficulties around cosom == nPI/2
					// in this case everything is up for grabs... 
					//  ...degenerate to linear interpolation
					if (sinom < EPSILON)
						{
							Scale0 = 1.0f - T;
							Scale1 = T;	
						}
					else
						{
							Scale0 = (jeFloat) sin( (1.0f-T) * omega) / sinom;
							Scale1 = (jeFloat) sin( T*omega) / sinom;
						}
				}
			else
				{
					// has numerical difficulties around cosom == 0
					// in this case degenerate to linear interpolation
				
					Scale0 = 1.0f - T;
					Scale1 = T;
				}
			QT-> X = Scale0 * Q0->X + Scale1 * Q1->X;
			QT-> Y = Scale0 * Q0->Y + Scale1 * Q1->Y;
			QT-> Z = Scale0 * Q0->Z + Scale1 * Q1->Z;
			QT-> W = Scale0 * Q0->W + Scale1 * Q1->W;
			//#pragma message (" ack:!!!!!!")
			//jeQuaternionNormalize(QT); 
			jeQuaternion_Assert( jeQuaternion_IsUnit(QT));
		}
	else
		{
			QT->X = -Q0->Y; 
			QT->Y =  Q0->X;
			QT->Z = -Q0->W;
			QT->W =  Q0->Z;
			Scale0 = (jeFloat) sin( (1.0f - T) * (QUATERNION_PI*0.5) );
			Scale1 = (jeFloat) sin( T * (QUATERNION_PI*0.5) );
			QT-> X = Scale0 * Q0->X + Scale1 * QT->X;
			QT-> Y = Scale0 * Q0->Y + Scale1 * QT->Y;
			QT-> Z = Scale0 * Q0->Z + Scale1 * QT->Z;
			QT-> W = Scale0 * Q0->W + Scale1 * QT->W;
			jeQuaternion_Assert( jeQuaternion_IsUnit(QT));
		}
}

JETAPI void JETCC jeQuaternion_Multiply(
	const jeQuaternion	*Q1, 
	const jeQuaternion	*Q2, 
	jeQuaternion		*Q)
	// multiplies q1 * q2, and places the result in q.
	// no failure. 	renormalization not automatic

{
	jeQuaternion Q1L,Q2L;
	assert( jeQuaternion_IsValid(Q1) != JE_FALSE );
	assert( jeQuaternion_IsValid(Q2) != JE_FALSE );
	assert( Q  != NULL );
	Q1L = *Q1;
	Q2L = *Q2;

	Q->W  =	(  (Q1L.W*Q2L.W) - (Q1L.X*Q2L.X) 
			 - (Q1L.Y*Q2L.Y) - (Q1L.Z*Q2L.Z) );

	Q->X  =	(  (Q1L.W*Q2L.X) + (Q1L.X*Q2L.W) 
			 + (Q1L.Y*Q2L.Z) - (Q1L.Z*Q2L.Y) );

	Q->Y  =	(  (Q1L.W*Q2L.Y) - (Q1L.X*Q2L.Z) 
			 + (Q1L.Y*Q2L.W) + (Q1L.Z*Q2L.X) );

	Q->Z  = (  (Q1L.W*Q2L.Z) + (Q1L.X*Q2L.Y) 
			 - (Q1L.Y*Q2L.X) + (Q1L.Z*Q2L.W) );
	jeQuaternion_Assert( jeQuaternion_IsValid(Q) != JE_FALSE );

}


JETAPI void JETCC jeQuaternion_Rotate(
	const jeQuaternion	*Q, 
	const jeVec3d         *V, 
	jeVec3d				*VRotated)
	// Rotates V by the quaternion Q, places the result in VRotated.
{
	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	assert( jeVec3d_IsValid(V)  != JE_FALSE );
	assert( VRotated  != NULL );

	jeQuaternion_Assert( jeQuaternion_IsUnit(Q) == JE_TRUE );

	{
		jeQuaternion Qinv,QV,QRotated, QT;
		jeFloat zero;
		jeQuaternion_SetVec3d(&QV ,0.0f,V);
		jeQuaternion_Inverse (Q,&Qinv);
		jeQuaternion_Multiply(Q,&QV,&QT);
		jeQuaternion_Multiply(&QT,&Qinv,&QRotated);
		jeQuaternion_GetVec3d(&QRotated,&zero,VRotated);
	}
	
}



JETAPI jeBoolean JETCC jeQuaternion_IsUnit(const jeQuaternion *Q)
	// returns JE_TRUE if Q is a unit jeQuaternion.  JE_FALSE otherwise.
{
	jeFloat magnitude;
	assert( Q != NULL );

	magnitude  =   (Q->W * Q->W) + (Q->X * Q->X) 
					  + (Q->Y * Q->Y) + (Q->Z * Q->Z);

	if (( magnitude < 1.0+UNIT_TOLERANCE ) && ( magnitude > 1.0-UNIT_TOLERANCE ))
		return JE_TRUE;
	return JE_FALSE;
}

JETAPI jeFloat JETCC jeQuaternion_Magnitude(const jeQuaternion *Q)
	// returns Magnitude of Q.  
{

	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	return   (Q->W * Q->W) + (Q->X * Q->X)  + (Q->Y * Q->Y) + (Q->Z * Q->Z);
}


JETAPI jeFloat JETCC jeQuaternion_Normalize(jeQuaternion *Q)
	// normalizes Q to be a unit jeQuaternion
{
	jeFloat magnitude,one_over_magnitude;
	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	
	magnitude =   (jeFloat) sqrt ((Q->W * Q->W) + (Q->X * Q->X) 
							  + (Q->Y * Q->Y) + (Q->Z * Q->Z));

	if (( magnitude < QZERO_TOLERANCE ) && ( magnitude > -QZERO_TOLERANCE ))
		{
			return 0.0f;
		}

	one_over_magnitude = 1.0f / magnitude;

	Q->W *= one_over_magnitude;
	Q->X *= one_over_magnitude;
	Q->Y *= one_over_magnitude;
	Q->Z *= one_over_magnitude;
	return magnitude;
}


JETAPI void JETCC jeQuaternion_Copy(const jeQuaternion *QSrc, jeQuaternion *QDst)
	// copies quaternion QSrc into QDst
{
	assert( jeQuaternion_IsValid(QSrc) != JE_FALSE );
	assert( QDst != NULL );
	*QDst = *QSrc;
}

JETAPI void JETCC jeQuaternion_Inverse(const jeQuaternion *Q, jeQuaternion *QInv)
	// sets QInv to the inverse of Q.  
{
	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	assert( QInv != NULL );

	QInv->W =  Q->W;
	QInv->X = -Q->X;
	QInv->Y = -Q->Y;
	QInv->Z = -Q->Z;
}


JETAPI void JETCC jeQuaternion_Add(
	const jeQuaternion *Q1, 
	const jeQuaternion *Q2, 
	jeQuaternion *QSum)
	// QSum = Q1 + Q2  (result is not generally a unit quaternion!)
{
	assert( jeQuaternion_IsValid(Q1) != JE_FALSE );
	assert( jeQuaternion_IsValid(Q2) != JE_FALSE );
	assert( QSum != NULL );
	QSum->W = Q1->W + Q2->W;
	QSum->X = Q1->X + Q2->X;
	QSum->Y = Q1->Y + Q2->Y;
	QSum->Z = Q1->Z + Q2->Z;
}

JETAPI void JETCC jeQuaternion_Subtract(
	const jeQuaternion *Q1, 
	const jeQuaternion *Q2, 
	jeQuaternion *QSum)
	// QSum = Q1 - Q2  (result is not generally a unit quaternion!)
{
	assert( jeQuaternion_IsValid(Q1) != JE_FALSE );
	assert( jeQuaternion_IsValid(Q2) != JE_FALSE );
	assert( QSum != NULL );
	QSum->W = Q1->W - Q2->W;
	QSum->X = Q1->X - Q2->X;
	QSum->Y = Q1->Y - Q2->Y;
	QSum->Z = Q1->Z - Q2->Z;
}


#define ZERO_EPSILON (0.0001f)
 
JETAPI void JETCC jeQuaternion_Ln(
	const jeQuaternion *Q, 
	jeQuaternion *LnQ)
	// ln(Q) for unit quaternion only!
{
	jeFloat Theta;
	jeQuaternion QL;
	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	assert( LnQ != NULL );
	jeQuaternion_Assert( jeQuaternion_IsUnit(Q) == JE_TRUE );
	
	if (Q->W < 0.0f)
		{
			QL.W = -Q->W;
			QL.X = -Q->X;
			QL.Y = -Q->Y;
			QL.Z = -Q->Z;
		}
	else
		{
			QL = *Q;
		}
	Theta    = (jeFloat)  acos( QL.W  );
	 //  0 < Theta < pi
	if (Theta< ZERO_EPSILON)
		{
			// lim(t->0) of t/sin(t) = 1, so:
			LnQ->W = 0.0f;
			LnQ->X = QL.X;
			LnQ->Y = QL.Y;
			LnQ->Z = QL.Z;
		}
	else
		{
			jeFloat Theta_Over_sin_Theta =  Theta / (jeFloat) sin ( Theta );
			LnQ->W = 0.0f;
			LnQ->X = Theta_Over_sin_Theta * QL.X;
			LnQ->Y = Theta_Over_sin_Theta * QL.Y;
			LnQ->Z = Theta_Over_sin_Theta * QL.Z;
		}

}
	
JETAPI void JETCC jeQuaternion_Exp(
	const jeQuaternion *Q,
	jeQuaternion *ExpQ)
	// exp(Q) for pure quaternion only!  (zero scalar part (W))
{
	jeFloat Theta;
	jeFloat sin_Theta_over_Theta;

	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	assert( ExpQ != NULL);
	assert( Q->W == 0.0 );	//check a range?

	Theta = (jeFloat) sqrt(Q->X*Q->X  +  Q->Y*Q->Y  +  Q->Z*Q->Z);
	if (Theta > ZERO_EPSILON)
		{
			sin_Theta_over_Theta = (jeFloat) sin(Theta) / Theta;
		}
	else
		{
			sin_Theta_over_Theta = (jeFloat) 1.0f;
		}

	ExpQ->W   = (jeFloat) cos(Theta);
	ExpQ->X   = sin_Theta_over_Theta * Q->X;
	ExpQ->Y   = sin_Theta_over_Theta * Q->Y;
	ExpQ->Z   = sin_Theta_over_Theta * Q->Z;
}	

JETAPI void JETCC jeQuaternion_Scale(
	const jeQuaternion *Q,
	jeFloat Scale,
	jeQuaternion *QScaled)
	// Q = Q * Scale  (result is not generally a unit quaternion!)
{
	assert( jeQuaternion_IsValid(Q) != JE_FALSE );
	assert( (Scale * Scale) >=0.0f );
	assert( QScaled != NULL);

	QScaled->W = Q->W * Scale;
	QScaled->X = Q->X * Scale;
	QScaled->Y = Q->Y * Scale;
	QScaled->Z = Q->Z * Scale;
}

JETAPI void JETCC jeQuaternion_SetNoRotation(jeQuaternion *Q)
	// sets Q to be a quaternion with no rotation (like an identity matrix)
{
	Q->W = 1.0f;
	Q->X = Q->Y = Q->Z = 0.0f;
	
	/* this is equivalent to 
		{	
			jeXForm3d M;
			jeXForm3d_SetIdentity(&M);
			jeQuaternionFromMatrix(&M,Q);
		}
	*/
}



JETAPI jeBoolean JETCC jeQuaternion_Compare( jeQuaternion *Q1, jeQuaternion *Q2, jeFloat Tolerance )
{
	assert( jeQuaternion_IsValid(Q1) != JE_FALSE );
	assert( jeQuaternion_IsValid(Q2) != JE_FALSE );
	assert ( Tolerance >= 0.0 );

	if (	// they are the same but with opposite signs
			(		(fabs(Q1->X + Q2->X) <= Tolerance )  
				&&  (fabs(Q1->Y + Q2->Y) <= Tolerance )  
				&&  (fabs(Q1->Z + Q2->Z) <= Tolerance )  
				&&  (fabs(Q1->W + Q2->W) <= Tolerance )  
			)
		  ||  // they are the same with same signs
			(		(fabs(Q1->X - Q2->X) <= Tolerance )  
				&&  (fabs(Q1->Y - Q2->Y) <= Tolerance )  
				&&  (fabs(Q1->Z - Q2->Z) <= Tolerance )  
				&&  (fabs(Q1->W - Q2->W) <= Tolerance )  
			)
		)
		return JE_TRUE;
	else
		return JE_FALSE;


	
}
