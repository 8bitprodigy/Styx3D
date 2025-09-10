/****************************************************************************************/
/*  JEINDEXPOLY.C                                                                       */
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
#include <assert.h>
#include <stdio.h>

// Public dependents
#include "jeIndexPoly.h"

// Private dependents
#include "Ram.h"

//=======================================================================================
//	jeIndexPoly_Create
//=======================================================================================
jeIndexPoly *jeIndexPoly_Create(jeIndexPoly_NumVertType NumVerts)
{
	jeIndexPoly	*Poly;
	int32		i;

	assert(NumVerts < JE_INDEXPOLY_MAX_VERTS);

	Poly = JE_RAM_ALLOCATE_STRUCT(jeIndexPoly);

	if (!Poly)
		return NULL;

	Poly->Verts = JE_RAM_ALLOCATE_ARRAY(jeVertArray_Index, NumVerts);

	if (!Poly->Verts)
	{
		jeRam_Free(Poly);
		return NULL;
	}

	// Invalidate all the indexes
	for (i=0; i<NumVerts; i++)
	{
		Poly->Verts[i] = JE_VERTARRAY_NULL_INDEX;
	}
	
	Poly->NumVerts = NumVerts;

	return Poly;
}

//=======================================================================================
//	jeIndexPoly_CreateFromFile
//=======================================================================================
jeIndexPoly *jeIndexPoly_CreateFromFile(jeVFile *VFile)
{
	jeIndexPoly	*Poly;

	Poly = JE_RAM_ALLOCATE_STRUCT(jeIndexPoly);

	if (!Poly)
		return NULL;

	// Read the number of verts
	if (!jeVFile_Read(VFile, &Poly->NumVerts, sizeof(Poly->NumVerts)))
		return NULL;

	// Allocate the verts
	Poly->Verts = JE_RAM_ALLOCATE_ARRAY(jeVertArray_Index, Poly->NumVerts);

	if (!Poly->Verts)
	{
		jeRam_Free(Poly);
		return NULL;
	}

	// Read the verts
	if (!jeVFile_Read(VFile, Poly->Verts, sizeof(Poly->Verts[0])*Poly->NumVerts))
		return NULL;

	return Poly;
}

//=======================================================================================
//	jeIndexPoly_WriteToFile
//=======================================================================================
jeBoolean jeIndexPoly_WriteToFile(const jeIndexPoly *Poly, jeVFile *VFile)
{
	// Write out the number of verts
	if (!jeVFile_Write(VFile, &Poly->NumVerts, sizeof(Poly->NumVerts)))
		return JE_FALSE;

	// Write out the verts
	if (!jeVFile_Write(VFile, Poly->Verts, sizeof(Poly->Verts[0])*Poly->NumVerts))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeIndexPoly_Destroy
//=======================================================================================
void jeIndexPoly_Destroy(jeIndexPoly **Poly)
{
	jeIndexPoly		*pPoly;

	assert(Poly);
	pPoly = *Poly;
	assert(pPoly);

	if (pPoly->Verts)
	{
		assert(pPoly->NumVerts > 0);
		jeRam_Free(pPoly->Verts);
	}
	else
	{
		assert(pPoly->NumVerts == 0);
	}

	jeRam_Free(pPoly);

	*Poly = NULL;
}

//=======================================================================================
//	jeIndexPoly_IsConvex
//=======================================================================================
jeBoolean jeIndexPoly_IsConvex(const jeIndexPoly *Poly, const jeVec3d *Normal, const jeVertArray *Array)
{
	int32		i;

	for (i=0; i<Poly->NumVerts; i++)
	{
		jeVec3d			Edge, EdgeNormal;
		const jeVec3d	*v1, *v2;
		int32			j, i2;

		i2 = (i == Poly->NumVerts - 1) ? 0 : i+1;

		v1 = jeVertArray_GetVertByIndex(Array, (jeVertArray_Index)i);
		v2 = jeVertArray_GetVertByIndex(Array, (jeVertArray_Index)i2);

		jeVec3d_Subtract(v1, v2, &Edge);

		jeVec3d_CrossProduct(&Edge, Normal, &EdgeNormal);
		jeVec3d_Normalize(&EdgeNormal);

		for (j=0; j<Poly->NumVerts; j++)
		{
			jeFloat			Val;
			const jeVec3d	*v3;

			if (j == i || j == i2)
				continue;

			v3 = jeVertArray_GetVertByIndex(Array, (jeVertArray_Index)j);

			Val = jeVec3d_DotProduct(v2, &EdgeNormal);

			if (Val < 0)			// Point behind edge, poly is non-convex
				return JE_FALSE;
		}
	}

	return JE_TRUE;
}
