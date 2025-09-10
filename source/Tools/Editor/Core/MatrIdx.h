/****************************************************************************************/
/*  MATRIDX.H                                                                           */
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
#ifndef MATRIDX_H
#define MATRIDX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bitmap.h"
#include "jeMaterial.h"

typedef struct		MatrIdx_Struct		MatrIdx_Struct;

MatrIdx_Struct  *		MatrIdx_Create( jeMaterial_Array * pMatlArray, jeBitmap * pBitMap, const char * Name );
const char		*		MatrIdx_GetName( MatrIdx_Struct* MatrIdx );
jeBitmap		*		MatrIdx_GetBitmap( MatrIdx_Struct* MatrIdx );
void					MatrIdx_AddRef( MatrIdx_Struct* MatrIdx );
jeMaterial_ArrayIndex	MatrIdx_GetIndex( MatrIdx_Struct* pMatrIdx );
//returns JE_TRUE if object was truly destroyed
//returns JE_FALSE if only RefCnt was decremented
jeBoolean MatrIdx_Destroy( MatrIdx_Struct** hMatrIdx );

#ifdef __cplusplus
}
#endif

#endif //Prevent multiple inclusion
/* EOF: MatrIdxs.h */