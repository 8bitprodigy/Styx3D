/****************************************************************************************/
/*  JETEXVEC.H                                                                          */
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

#ifndef JE_TEXVEC_H
#define JE_TEXVEC_H

#include "Vec3d.h"
#include "jeGArray.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================
//	Typedefs/#defines
//========================================================================================
typedef struct	jeTexVec_Array			jeTexVec_Array;
					
#define jeTexVec_ArrayIndex				jeGArray_Index

#define JE_TEXVEC_ARRAY_MAX_TEXVECS		(JE_GARRAY_MAX_ELEMENTS)
#define	JE_TEXVEC_ARRAY_NULL_INDEX		(JE_GARRAY_NULL_INDEX)

//========================================================================================
//	Structure defs
//========================================================================================
typedef struct
{
	jeVec3d		VecU;
	jeVec3d		VecV;
} jeTexVec;

//Begin CyRiuS (Timothy Roff)
typedef struct Surf_TexVert
{
	float		u, v;
	float		r, g, b, a;
} Surf_TexVert;
//End CyRiuS

//========================================================================================
//	Function prototypes
//========================================================================================
jeBoolean jeTexVec_Compare(const jeTexVec *Tex1, const jeTexVec *Tex2);
jeTexVec_Array *jeTexVec_ArrayCreate(int32 StartVecs);
void jeTexVec_ArrayDestroy(jeTexVec_Array **Array);
jeBoolean jeTexVec_ArrayIndexIsValid(jeTexVec_ArrayIndex Index);
jeTexVec_ArrayIndex jeTexVec_ArrayAddTexVec(jeTexVec_Array *Array, const jeTexVec *TexVec);
jeTexVec_ArrayIndex jeTexVec_ArrayShareTexVec(jeTexVec_Array *Array, const jeTexVec *TexVec);
jeBoolean jeTexVec_ArrayRefTexVecByIndex(jeTexVec_Array *Array, jeTexVec_ArrayIndex Index);
void jeTexVec_ArrayRemoveTexVec(jeTexVec_Array *Array, jeTexVec_ArrayIndex *Index);
void jeTexVec_ArraySetTexVecByIndex(jeTexVec_Array *Array, jeTexVec_ArrayIndex Index, const jeTexVec *TexVec);
const jeTexVec *jeTexVec_ArrayGetTexVecByIndex(const jeTexVec_Array *Array, jeTexVec_ArrayIndex Index);

#ifdef __cplusplus
}
#endif

#endif
