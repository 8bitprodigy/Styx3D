/****************************************************************************************/
/*  MATRARRAY.H                                                                         */
/*                                                                                      */
/*  Author:                                                                             */
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
#pragma once
#ifndef MATRARRAY_H
#define MATRARRAY_H

#include "jeList.h"
#include "MatrIdx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MatrArray_Struct MatrArray_Struct ;
typedef ListIterator MatrIdxIterator ;

MatrArray_Struct *		MatrArray_Create( jeMaterial_Array * pMatlArray ) ;
void			MatrArray_Destroy( MatrArray_Struct **ppList ) ;

// ACCESSORS
MatrIdx_Struct *	MatrArray_GetMatrIdx(		MatrArray_Struct * pList, MatrIdxIterator pMI ) ;
MatrIdx_Struct *	MatrArray_GetFirstMatrIdx(	MatrArray_Struct * pList, MatrIdxIterator * pMI ) ;
MatrIdx_Struct *	MatrArray_GetNextMatrIdx(	MatrArray_Struct * pList, MatrIdxIterator * pMI ) ;

MatrIdx_Struct *	MatrArray_GetCurMatrIdx( MatrArray_Struct* MatrArray );
void				MatrArray_SetCurMatrIdx( MatrArray_Struct* MatrArray, MatrIdx_Struct* MatrIdx );

// MODIFIERS
MatrIdx_Struct *	MatrArray_Add( MatrArray_Struct * pList, jeBitmap * pBitmap, const char * Name );

// SEARCH
MatrIdx_Struct *	MatrArray_SearchByName( MatrArray_Struct* MatrArray, MatrIdxIterator * pMI, const char* Name );

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: MatrArray.h */
