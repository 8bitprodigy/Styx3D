/****************************************************************************************/
/*  XFORM3D.H                                                                           */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: 3D transform interface                                                 */
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
#ifndef JE_XFORM_H
#define JE_XFORM_H

#include "Vec3d.h"

#ifdef __cplusplus
extern "C" {
#endif

/*{****

// CB note <>
// this would be mighty nice :

typedef struct
{
	union {
		struct Matrix {
			jeFloat AA,AB,AC,AD;
			jeFloat BA,BB,BC,BD;
			jeFloat CA,CB,CC,CD;
		} Matrix;

		struct Matrix {
			jeVec3d X;
			jeVec3d Y;
			jeVec3d Z;
		} Rows;
	}
	jeVec3d Translation;
} jeXForm3d;

***}*/

#define	XFORM3D_NONORTHOGONALISOK	0x00000001

typedef struct jeXForm3d
{	
	jeFloat 	AX,AY,AZ;			// e[0][0],e[0][1],e[0][2]
	uint32		Flags;				// Be careful!  We've sandwiched this in
									//  where Katmai instructions will deal with it
	jeFloat 	BX,BY,BZ, BPad;		// e[1][0],e[1][1],e[1][2]
	jeFloat 	CX,CY,CZ, CPad;		// e[2][0],e[2][1],e[2][2]
	jeVec3d 	Translation;		// e[0][3],e[1][3],e[2][3]
	//	  0,0,0,1					// e[3][0],e[3][1],e[3][2]
} jeXForm3d;

/*   this is essentially a 'standard' 4x4 transform matrix,
     with the bottom row always 0,0,0,1

	| AX, AY, AZ, Translation.X |  
	| BX, BY, BZ, Translation.Y |  
	| CX, CY, CZ, Translation.Z |  
	|  0,  0,  0,      1        |  
*/

//  all jeXForm3d_Set* functions return a right-handed transform.

#define GEXFORM3D_MINIMUM_SCALE (0.00001f)

//is katmai active?
JETAPI	jeBoolean	JETCC	jeXForm3d_UsingKatmai(void);
JETAPI	jeBoolean	JETCC	jeXForm3d_EnableKatmai(jeBoolean useit);

JETAPI void JETCC jeXForm3d_Copy(
	const jeXForm3d *Src, 
	jeXForm3d *Dst);
	// copies Src to Dst.  

JETAPI jeBoolean JETCC jeXForm3d_IsValid(const jeXForm3d *M);
	// returns JE_TRUE if M is 'valid'  
	// 'valid' means that M is non NULL, and there are no NAN's in the matrix.

JETAPI jeBoolean JETCC jeXForm3d_IsOrthonormal(const jeXForm3d *M);
	// returns JE_TRUE if M is orthonormal 
	// (if the rows and columns are all normalized (transform has no scaling or shearing)
	// and is orthogonal (row1 cross row2 = row3 & col1 cross col2 = col3)
	// * does not check for right-handed convention *

JETAPI jeBoolean JETCC jeXForm3d_IsOrthogonal(const jeXForm3d *M);
	// returns JE_TRUE if M is orthogonal
	// (row1 cross row2 = row3 & col1 cross col2 = col3)
	// * does not check for right-handed convention *

JETAPI void JETCC jeXForm3d_Orthonormalize(jeXForm3d *M);
	// essentially removes scaling (or other distortions) from 
	// an orthogonal (or nearly orthogonal) matrix 
	// returns a right-handed matrix


JETAPI void JETCC jeXForm3d_SetIdentity(jeXForm3d *M);			
	// sets M to an identity matrix (clears it)
	
JETAPI void JETCC jeXForm3d_SetXRotation(jeXForm3d *M,jeFloat RadianAngle);
	// sets up a transform that rotates RadianAngle about X axis
	// all existing contents of M are replaced
	
JETAPI void JETCC jeXForm3d_SetYRotation(jeXForm3d *M,jeFloat RadianAngle);
	// sets up a transform that rotates RadianAngle about Y axis
	// all existing contents of M are replaced

JETAPI void JETCC jeXForm3d_SetZRotation(jeXForm3d *M,jeFloat RadianAngle);
	// sets up a transform that rotates RadianAngle about Z axis
	// all existing contents of M are replaced

JETAPI void JETCC jeXForm3d_SetTranslation(jeXForm3d *M,jeFloat x, jeFloat y, jeFloat z);
	// sets up a transform that translates x,y,z
	// all existing contents of M are replaced

JETAPI void JETCC jeXForm3d_SetScaling(jeXForm3d *M,jeFloat x, jeFloat y, jeFloat z);
	// sets up a transform that scales by x,y,z
	// all existing contents of M are replaced

JETAPI void JETCC jeXForm3d_RotateX(jeXForm3d *M,jeFloat RadianAngle);  
	// Rotates M by RadianAngle about X axis   
	// applies the rotation to the existing contents of M

JETAPI void JETCC jeXForm3d_RotateY(jeXForm3d *M,jeFloat RadianAngle);
	// Rotates M by RadianAngle about Y axis
	// applies the rotation to the existing contents of M

JETAPI void JETCC jeXForm3d_RotateZ(jeXForm3d *M,jeFloat RadianAngle);
	// Rotates M by RadianAngle about Z axis
	// applies the rotation to the existing contents of M

JETAPI void JETCC jeXForm3d_PostRotateX(jeXForm3d *M,jeFloat RadianAngle);  
JETAPI void JETCC jeXForm3d_PostRotateY(jeXForm3d *M,jeFloat RadianAngle);
JETAPI void JETCC jeXForm3d_PostRotateZ(jeXForm3d *M,jeFloat RadianAngle);
	// appends the rotation to M
	// this operation does not change the translation in M, unlike the "_Rotate" functions

JETAPI void JETCC jeXForm3d_Translate(jeXForm3d *M,jeFloat x, jeFloat y, jeFloat z);	
	// Translates M by x,y,z
	// applies the translation to the existing contents of M

JETAPI void JETCC jeXForm3d_Scale(jeXForm3d *M,jeFloat x, jeFloat y, jeFloat z);		
	// Scales M by x,y,z
	// applies the scale to the existing contents of M

JETAPI void JETCC jeXForm3d_Multiply(
	const jeXForm3d *M1, 
	const jeXForm3d *M2, 
	jeXForm3d *MProduct);
	// MProduct = matrix multiply of M1*M2
	// Concatenates the transformation in the M2 matrix onto the transformation in M1

JETAPI void JETCC jeXForm3d_Transform(
	const jeXForm3d *M,
	const jeVec3d *V, 
	jeVec3d *Result);
	// Result is Matrix M * Vector V:  V Tranformed by M

JETAPI void JETCC jeXForm3d_TransformVecArray(const jeXForm3d *XForm, 
								const jeVec3d *Source, 
								jeVec3d *Dest, 
								int32 Count);

JETAPI void JETCC jeXForm3d_TransformArray(	const jeXForm3d *XForm, 
								const jeVec3d *Source, 
									int32 SourceStride,
								jeVec3d *Dest,
									int32 DestStride,
								int32 Count);

JETAPI void JETCC jeXForm3d_Rotate(
	const jeXForm3d *M,
	const jeVec3d *V, 
	jeVec3d *Result);
	// Result is Matrix M * Vector V:  V Rotated by M (no translation)


/***
*
	"Left,Up,In" are just the basis vectors in the new coordinate space.
	You can get them by multiplying the unit bases into the transforms.
*
******/

JETAPI void JETCC jeXForm3d_GetLeft(const jeXForm3d *M, jeVec3d *Left);
	// Gets a vector that is 'left' in the frame of reference of M (facing -Z)

JETAPI void JETCC jeXForm3d_GetUp(const jeXForm3d *M,    jeVec3d *Up);
	// Gets a vector that is 'up' in the frame of reference of M (facing -Z)

JETAPI void JETCC jeXForm3d_GetIn(const jeXForm3d *M,  jeVec3d *In);
	// Gets a vector that is 'in' in the frame of reference of M (facing -Z)

JETAPI void JETCC jeXForm3d_GetInverse(const jeXForm3d *M, jeXForm3d *MInv);
	// Gets the inverse transform of M   (M^T) 

JETAPI void JETCC jeXForm3d_GetTranspose(const jeXForm3d *M, jeXForm3d *MTranspose);
	// Gets the Transpose transform of M   (M^T) 
	// Transpose of a matrix is the switch of the rows and columns
	// The transpose is usefull because it is rapidly computed and is equal to the inverse 
	// transform for orthonormal transforms    [inverse is (M')  where M*M' = Identity ]

JETAPI void JETCC jeXForm3d_TransposeTransform(
	const jeXForm3d *M, 
	const jeVec3d *V, 
	jeVec3d *Result);
	// applies the transpose transform of M to V.  Result = (M^T) * V

/*****
*
	the Euler angles are subsequent rotations :
		by Angles->Z around the Z axis
		then by Angles->Y around the Y axis, in the newly rotate coordinates
		then by Angles->X around the X axis
*
******/	

JETAPI void JETCC jeXForm3d_GetEulerAngles(const jeXForm3d *M, jeVec3d *Angles);
	// Finds Euler angles from M and puts them into Angles
	
JETAPI void JETCC jeXForm3d_SetEulerAngles(jeXForm3d *M, const jeVec3d *Angles);
	// Applies Euler angles to build M

JETAPI void JETCC jeXForm3d_SetFromLeftUpIn(
	jeXForm3d *M,
	const jeVec3d *Left, 
	const jeVec3d *Up, 
	const jeVec3d *In);
	// Builds an jeXForm3d from orthonormal Left, Up and In vectors

JETAPI void JETCC jeXForm3d_Mirror(
	const		jeXForm3d *Source, 
	const		jeVec3d *PlaneNormal, 
	float		PlaneDist, 
	jeXForm3d	*Dest);
	// Mirrors a XForm3d about a plane


//--------------

#ifdef NDEBUG
	#define jeXForm3d_SetMaximalAssertionMode(Enable )
#else
	JETAPI 	void JETCC jeXForm3d_SetMaximalAssertionMode( jeBoolean Enable );
#endif


#ifdef __cplusplus
}
#endif

#endif
