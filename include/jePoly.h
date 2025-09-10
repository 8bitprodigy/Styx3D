/****************************************************************************************/
/*  JEPOLY.H                                                                            */
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

#ifndef GEPOLY_H
#define GEPOLY_H

#include "Vec3d.h"
#include "jePlane.h"

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
//=======================================================================================
typedef uint16	jePoly_NumVertType;
typedef jeVec3d	jePoly_VertType;

#define JE_POLY_MAX_VERTS	((uint32)1<<(sizeof(jePoly_NumVertType)*8))

//=======================================================================================
//=======================================================================================

typedef struct jePoly
{
	jePoly_NumVertType	NumVerts;
	jePoly_VertType		*Verts;

#ifdef _DEBUG
	struct jePoly		*Self;
#endif

} jePoly;

//=======================================================================================
//	Function prototypes
//=======================================================================================
jePoly		*jePoly_Create(int32 NumVerts);
jePoly		*jePoly_CreateFromPoly(const jePoly *Poly, jeBoolean Reverse);
jePoly		*jePoly_CreateFromPlane(const jePlane *Plane, float Scale);
void		jePoly_Destroy(jePoly **Poly);
float		jePoly_Area(const jePoly *Poly);
jeBoolean	jePoly_ClipEpsilon(jePoly **Poly, float Epsilon, const jePlane *Plane, jeBoolean FlipSide);
				// Poly will be NULL if clipped away.  Poly pointer will chanje if clipped...
jeBoolean	jePoly_SplitEpsilon(jePoly **InPoly, float Epsilon, const jePlane *Plane, jeBoolean FlipSide, jePoly **Front, jePoly **Back);
				// InPoly is freed
jeBoolean	jePoly_IsTiny (const jePoly *Poly);
jeBoolean	jePoly_IsValid(const jePoly *Poly);
jeBoolean	jePoly_EdgeExist(const jePoly *Poly, const jeVec3d *v1, const jeVec3d *v2, int32 *i1, int32 *i2);
jeBoolean	jePoly_Merge(const jePoly *Poly1, const jePoly *Poly2, const jeVec3d *Normal, jePoly **Out);
jeBoolean	jePoly_RemoveDegenerateEdges(jePoly *Poly, jeFloat Epsilon);
#ifdef __cplusplus
}
#endif

#endif
