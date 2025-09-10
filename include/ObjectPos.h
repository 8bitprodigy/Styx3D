/****************************************************************************************/
/*  OBJECTPOS.H                                                                         */
/*                                                                                      */
/*  Author: David Eisele	                                                        */
/*  Description: Simplify and improve rotation and tranlation of objects                */
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
/*  This file was not part of the original Jet3D, released December 12, 1999.           */
/*                                                                                      */
/****************************************************************************************/

#ifndef JE_POSITION_H
#define JE_POSITION_H

#include "Vec3d.h"
#include "XForm3d.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	jeFloat x[3][4];
} Matrix34;

typedef union MatrixXForm {
	struct{ 
		Matrix34	Matrix;
		jeVec3d		Translation;
	};
	jeXForm3d	XForm;
} MatrixXForm;

typedef struct jeObjectPos {
	jeFloat		Phi;							// Spin-Angle
	jeFloat		Psi;							// Slope-Angle
	jeFloat		Rho;							// Tilt-Angle
	jeFloat     CPhi;							// Their sin/cos values
	jeFloat     SPhi;
	jeFloat     CPsi;
	jeFloat     SPsi;
	jeFloat     CRho;
	jeFloat     SRho;
	Matrix34	RMatrix;						// Relative rotationmatrix
	Matrix34	BMatrix; 						// Base     rotationmatrix
	union { //  Matrix34 & jeVec3d <=> jeXForm3d
		struct { 
			Matrix34	Matrix;					// Resulting rotationmatrix
			jeVec3d		Translation;			// Translation
		};
		jeXForm3d	XForm;						// Resulting rotation- and translationmatrix
	};
	int         Flags;							// Normal  = BMatrix is identity
												// XFormed = BMatrix is NOT identity
												// Used internally for rotationacceleration
} jeObjectPos;



// Initialization-Methods

JETAPI void JETCC jeObjectPos_SetIdentity(jeObjectPos *APos);
// Initialization of APos:  Do nothing

JETAPI void JETCC jeObjectPos_SetTranslation(jeObjectPos *APos, jeFloat X, jeFloat Y, jeFloat Z);
// Initialization of APos:  Set up a translation by (X,Y,Z)

JETAPI void JETCC jeObjectPos_SetTranslationByVec(jeObjectPos *APos, jeVec3d *T);
// Initialization of APos:  Set up a translation by T
							
JETAPI void JETCC jeObjectPos_SetRotation(jeObjectPos *APos, jeFloat Phi, jeFloat Psi, jeFloat Rho);
// Initialization of APos:  Set up a rotation:
//							Spin by Phi, slope by Psi, tilt by Rho
//							Same as RotateZ(Rho), RotateX(Psi), RotateY(Phi)

JETAPI void JETCC jeObjectPos_SetBaseXForm(jeObjectPos *APos, const jeXForm3d *BaseXF);
// Initialization of APos:  Set up relative Identity:
//							Rotate by BaseXF,no translation or further rotation

JETAPI void JETCC jeObjectPos_SetBaseXFormByAxis(jeObjectPos *APos, const jeVec3d *X, const jeVec3d *Y, const jeVec3d *Z);
// Same as above, but instead of a XForm the axis are given:
//							X = "Left"-axisvector
//							Y = "Up"-axisvector
//							Z = "Forward"-axisvector



// Set-Methods

void __inline jeObjectPos_SetNewTranslationByVec(jeObjectPos *APos, jeVec3d *V) { APos->Translation=*V; }
// Moveto V

void __inline jeObjectPos_SetNewTranslationVecFromMatrix(jeObjectPos *APos, jeXForm3d *XForm) { APos->Translation=XForm->Translation; }
// Get translation of XForm and moveto there

JETAPI void JETCC jeObjectPos_SetNewRotation(jeObjectPos *APos, jeFloat Phi, jeFloat Psi, jeFloat Rho);
// Rotate to the new angles

JETAPI void JETCC jeObjectPos_SetNewBaseXForm(jeObjectPos *APos, const jeXForm3d *BaseXF);
// Setup a new baseorientation by a XForm

JETAPI void JETCC jeObjectPos_SetNewBaseXFormByAxis(jeObjectPos *APos, const jeVec3d *X, const jeVec3d *Y, const jeVec3d *Z);
// Setup a new baseorientation by left/up/forward vectors




// Transform-Methods

JETAPI void JETCC jeObjectPos_Spin(jeObjectPos *APos, jeFloat DPhi);
// Rotate CounterClockWise around the "up/Y-axis" by DPhi
// pos. : "rotate right"
// neg. : "rotate left"

JETAPI void JETCC jeObjectPos_Slope(jeObjectPos *APos, jeFloat DPsi);
// Rotate CCW around the "right/X-axis" by DPsi
// pos. : "rotate up"
// neg. : "rotate down"

JETAPI void JETCC jeObjectPos_Tilt(jeObjectPos *APos, jeFloat DRho);
// Rotate CCW around the "back/Z-axis" by DRho
// pos. : "fall left"
// neg. : "fall right"

JETAPI void JETCC jeObjectPos_Rotate(jeObjectPos *APos, jeFloat DPhi, jeFloat DPsi, jeFloat DRho);
// Rotate CCW around all axis by DPhi,DPsi,DRho

JETAPI void JETCC jeObjectPos_Translate(jeObjectPos *APos, jeFloat DX, jeFloat DY, jeFloat DZ);
// Move by (DX,DY,DZ)

JETAPI void JETCC jeObjectPos_Move(jeObjectPos *APos, jeVec3d *Dir, jeFloat Dist);
// Move by scaled Dir vector
// Normally used for normalized vectors

JETAPI void JETCC jeObjectPos_MoveIn(jeObjectPos *APos, jeFloat Dist);
// Move forward by Dist

JETAPI void JETCC jeObjectPos_MoveLeft(jeObjectPos *APos, jeFloat Dist);
// Move left by Dist

JETAPI void JETCC jeObjectPos_MoveUp(jeObjectPos *APos, jeFloat Dist);
// Move up by Dist

JETAPI void JETCC jeObjectPos_MoveByDifference(const jeObjectPos *OPos, const jeObjectPos *NPos, jeObjectPos *APos);
// Get difference translation vector of OPos and NPos  and move APos by it

JETAPI void JETCC jeObjectPos_TransformByDifference(const jeObjectPos *OPos, const jeObjectPos *NPos, jeObjectPos *APos);
// Same as above, but additionally spin/slope/tilt APos by difference


// Query-Methods

void __inline jeObjectPos_GetTranslation(const jeObjectPos *APos, jeVec3d *V) { *V=APos->Translation; }
// return translation

void __inline jeObjectPos_ToMatrix(const jeObjectPos *APos, jeXForm3d *XForm) { *XForm=APos->XForm; }
// return rotation and translation

JETAPI void JETCC jeObjectPos_GetIn(const jeObjectPos *APos, jeVec3d *V);
// Get relative forwardvector (to baseorientation)

JETAPI void JETCC jeObjectPos_GetLeft(const jeObjectPos *APos, jeVec3d *V);
// Get relative leftvector

JETAPI void	JETCC jeObjectPos_GetUp(const jeObjectPos *APos, jeVec3d *V);
// Get relative upwardvector

JETAPI jeBoolean JETCC jeObjectPos_IsValid(const jeObjectPos *APos);
// Are all variables correct initialized?



#ifdef NDEBUG
	#define jeObjectPos_SetMaximalAssertionMode(Enable )
#else
	JETAPI 	void JETCC jeObjectPos_SetMaximalAssertionMode( jeBoolean Enable );
#endif

#ifdef __cplusplus
}
#endif

#endif
