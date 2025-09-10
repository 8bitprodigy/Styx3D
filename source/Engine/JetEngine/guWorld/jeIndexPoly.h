/****************************************************************************************/
/*  JEINDEXPOLY.H                                                                       */
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

#ifndef GEINDEXPOLY_H
#define GEINDEXPOLY_H

#include "jeVertArray.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================
//	Typedefs/#defines
//========================================================================================
typedef uint16	jeIndexPoly_NumVertType;		// This must be an unsigned!!!

#define JE_INDEXPOLY_MAX_VERTS	(0xffff)

//========================================================================================
//	Structure defs
//========================================================================================
typedef struct
{
	jeIndexPoly_NumVertType	NumVerts;
	jeVertArray_Index		*Verts;	
} jeIndexPoly;

//========================================================================================
//	Function prototypes
//========================================================================================
jeIndexPoly *jeIndexPoly_Create(jeIndexPoly_NumVertType NumVerts);
jeIndexPoly *jeIndexPoly_CreateFromFile(jeVFile *VFile);
jeBoolean	jeIndexPoly_WriteToFile(const jeIndexPoly *Poly, jeVFile *VFile);
void		jeIndexPoly_Destroy(jeIndexPoly **Poly);
jeBoolean	jeIndexPoly_IsConvex(const jeIndexPoly *Poly, const jeVec3d *Normal, const jeVertArray *Array);

#ifdef __cplusplus
}
#endif

#endif
