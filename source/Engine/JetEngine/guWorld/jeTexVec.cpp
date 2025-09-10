/****************************************************************************************/
/*  JETEXVEC.C                                                                          */
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
#include <memory.h>

// Public dependents
#include "jeTexVec.h"

// Private dependents
#include "jeGArray.h"
#include "Ram.h"
#include "Errorlog.h"

//=======================================================================================
//	jeTexVec_Compare
//=======================================================================================
jeBoolean jeTexVec_Compare(const jeTexVec *Tex1, const jeTexVec *Tex2)
{
	if (!jeVec3d_Compare(&Tex1->VecU, &Tex2->VecU, 0.01f))
		return JE_FALSE;
	if (!jeVec3d_Compare(&Tex1->VecV, &Tex2->VecV, 0.01f))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeTexVec_ArrayCreate
//=======================================================================================
jeTexVec_Array *jeTexVec_ArrayCreate(int32 StartVecs)
{
	return (jeTexVec_Array*)jeGArray_Create(StartVecs, sizeof(jeTexVec));
}

//=======================================================================================
//	jeTexVec_ArrayDestroy
//=======================================================================================
void jeTexVec_ArrayDestroy(jeTexVec_Array **Array)
{
	jeGArray_Destroy((jeGArray**)Array);
}

//=======================================================================================
//	jeTexVec_ArrayIndexIsValid
//=======================================================================================
jeBoolean jeTexVec_ArrayIndexIsValid(jeTexVec_ArrayIndex Index)
{
	if (Index == JE_GARRAY_NULL_INDEX)
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeTexVec_ArrayAddTexVec
//=======================================================================================
jeTexVec_ArrayIndex jeTexVec_ArrayAddTexVec(jeTexVec_Array *Array, const jeTexVec *TexVec)
{
	jeGArray_Index	Index;

	Index = jeGArray_AddElement((jeGArray*)Array, (jeGArray_Element*)TexVec);

	return (jeTexVec_ArrayIndex)Index;
}

//=======================================================================================
//	jeTexVec_ArrayShareTexVec
//=======================================================================================
jeTexVec_ArrayIndex jeTexVec_ArrayShareTexVec(jeTexVec_Array *Array, const jeTexVec *TexVec)
{
	jeTexVec			*pTexVec;
	jeGArray_RefType	*pRef;
	int32				i;

	pTexVec = (jeTexVec*)jeGArray_GetElements((jeGArray*)Array);
	pRef = jeGArray_GetRefCounts((jeGArray*)Array);

	for (i=0; i< jeGArray_GetSize((jeGArray*)Array); i++, pRef++, *pTexVec++)
	{
		if (*pRef == 0 || *pRef >= JE_GARRAY_MAX_ELEMENT_REFCOUNT)
			continue;					// If it's empty, or totally full, we can't share with it...

		if (jeTexVec_Compare(pTexVec, TexVec))
		{
			assert(*pRef < JE_GARRAY_MAX_ELEMENT_REFCOUNT);
			(*pRef)++;
			return (jeTexVec_ArrayIndex)i;
		}
	}

	return jeTexVec_ArrayAddTexVec(Array, TexVec);
}

//=======================================================================================
//	jeTexVec_ArrayRefTexVecByIndex
//=======================================================================================
jeBoolean jeTexVec_ArrayRefTexVecByIndex(jeTexVec_Array *Array, jeTexVec_ArrayIndex Index)
{
	return jeGArray_RefElement((jeGArray*)Array, Index);
}

//=======================================================================================
//	jeTexVec_ArrayRemoveTexVec
//=======================================================================================
void jeTexVec_ArrayRemoveTexVec(jeTexVec_Array *Array, jeTexVec_ArrayIndex *Index)
{
	jeGArray_RemoveElement((jeGArray*)Array, (jeGArray_Index*)Index);
}

//=======================================================================================
//	jeTexVec_ArraySetTexVecByIndex
//=======================================================================================
void jeTexVec_ArraySetTexVecByIndex(jeTexVec_Array *Array, jeTexVec_ArrayIndex Index, const jeTexVec *TexVec)
{
	jeGArray_SetElementByIndex((jeGArray*)Array, (jeGArray_Index)Index, (jeGArray_Element*)TexVec);
}

//=======================================================================================
//	jeTexVec_ArrayGetTexVecByIndex
//=======================================================================================
const jeTexVec *jeTexVec_ArrayGetTexVecByIndex(const jeTexVec_Array *Array, jeTexVec_ArrayIndex Index)
{
	return (jeTexVec*)jeGArray_GetElementByIndex((jeGArray*)Array, (jeGArray_Index)Index);
}

