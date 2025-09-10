/****************************************************************************************/
/*  JEPLANEARRAY.H                                                                      */
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

#ifndef GEPLANEARRAY_H
#define GEPLANEARRAY_H

#include "BaseType.h"
#include "jePlane.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct		jePlaneArray		jePlaneArray;

typedef uint32		jePlaneArray_Index;

#define JE_PLANEARRAY_MAX_PLANES		((0xffffffff>>1)-1)
#define	JE_PLANEARRAY_NULL_INDEX		(JE_PLANEARRAY_MAX_PLANES+1)

#define jePlaneArray_IndexSided(p) (p&1)
#define jePlaneArray_IndexIsCoplanar(p1, p2) ((p1&~1) == (p2&~1))
#define jePlaneArray_IndexIsCoplanarAndFacing(p1, p2) (p1 == p2)
#define jePlaneArray_IndexIsCoplanarAndNotFacing(p1, p2) (p1 == (p2^1))
#define jePlaneArray_IndexReverse(p) (p^1)
#define	jePlaneArray_IndexGetPositive(p) (p&(~1))

jePlaneArray	*jePlaneArray_Create(int32 StartPlanes);
void			jePlaneArray_Destroy(jePlaneArray **Array);
jeBoolean		jePlaneArray_IsValid(const jePlaneArray *Array);
jePlaneArray_Index jePlaneArray_SharePlane(jePlaneArray *Array, const jePlane *Plane);
void			jePlaneArray_RemovePlane(jePlaneArray *Array, jePlaneArray_Index *Index);
jeBoolean		jePlaneArray_RefPlaneByIndex(jePlaneArray *Array, jePlaneArray_Index Index);

const jePlane * PLANE_CC jePlaneArray_GetPlaneByIndex(const jePlaneArray *Array, jePlaneArray_Index Index);

#ifdef __cplusplus
}
#endif

#endif
