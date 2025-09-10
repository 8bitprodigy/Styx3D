/****************************************************************************************/
/*  JEFACEINFO.C                                                                        */
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
#include <math.h>

// Public dependents
#include "jeFaceInfo.h"

// Private dependents
#include "Ram.h"
#include "Errorlog.h"

//=======================================================================================
//	jeFaceInfo_SetDefaults
//=======================================================================================
JETAPI void JETCC jeFaceInfo_SetDefaults(jeFaceInfo *FaceInfo)
{
	assert(FaceInfo);

	memset(FaceInfo, 0, sizeof(*FaceInfo));

	FaceInfo->Alpha = 100.0f;

	FaceInfo->DrawScaleU = 1.0f;
	FaceInfo->DrawScaleV = 1.0f;

	FaceInfo->LMapScaleU = 1.0f;
	FaceInfo->LMapScaleV = 1.0f;

	FaceInfo->MaterialIndex = JE_MATERIAL_ARRAY_NULL_INDEX;
}

#define FLOAT_COMPARE(a, b) (fabs((b)-(a)) < 0.01f)
//=======================================================================================
//	jeFaceInfo_Compare
//=======================================================================================
JETAPI jeBoolean JETCC jeFaceInfo_Compare(const jeFaceInfo *Face1, const jeFaceInfo *Face2)
{
	// FIXME:  Try to test the ones that are going to most likely fail first...
	if (Face1->Flags != Face2->Flags)
		return JE_FALSE;
	if (Face1->Alpha != Face2->Alpha)
		return JE_FALSE;

	if (!FLOAT_COMPARE(Face1->ShiftU, Face2->ShiftU))
		return JE_FALSE;

	if (!FLOAT_COMPARE(Face1->ShiftV, Face2->ShiftV))
		return JE_FALSE;

	if (!FLOAT_COMPARE(Face1->Rotate, Face2->Rotate))
		return JE_FALSE;

	if (Face1->DrawScaleU != Face2->DrawScaleU)
		return JE_FALSE;
	if (Face1->DrawScaleV != Face2->DrawScaleV)
		return JE_FALSE;
	if (Face1->LMapScaleU != Face2->LMapScaleU)
		return JE_FALSE;
	if (Face1->LMapScaleV != Face2->LMapScaleV)
		return JE_FALSE;
	if (Face1->MaterialIndex != Face2->MaterialIndex)
		return JE_FALSE;

	if (Face1->PortalCamera != Face2->PortalCamera)
		return JE_FALSE;

	return JE_TRUE;
}

//====================================================================================
//	jeFaceInfo_NeedsLightmap
//====================================================================================
JETAPI jeBoolean JETCC jeFaceInfo_NeedsLightmap(const jeFaceInfo *pFaceInfo)
{
	if (pFaceInfo->Flags & FACEINFO_NO_LIGHTMAP)
		return JE_FALSE;

	//if (pFaceInfo->PortalCamera)
	//	return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeFaceInfo_ArrayCreate
//=======================================================================================
JETAPI jeFaceInfo_Array * JETCC jeFaceInfo_ArrayCreate(int32 StartFaces)
{
	return (jeFaceInfo_Array*)jeGArray_Create(StartFaces, sizeof(jeFaceInfo));
}

//=======================================================================================
//	jeFaceInfo_ArrayCreateFromFile
//=======================================================================================
JETAPI jeFaceInfo_Array * JETCC jeFaceInfo_ArrayCreateFromFile(jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr)
{
	return (jeFaceInfo_Array*)jeGArray_CreateFromFile(VFile, IOFunc, IOFuncContext, PtrMgr);
}

//=======================================================================================
//	jeFaceInfo_ArrayWriteToFile
//=======================================================================================
JETAPI jeBoolean JETCC jeFaceInfo_ArrayWriteToFile(const jeFaceInfo_Array *Array, jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr)
{
	return jeGArray_WriteToFile((jeGArray*)Array, VFile, IOFunc, IOFuncContext, PtrMgr);
}

//=======================================================================================
//	jeFaceInfo_ArrayCreateRef
//=======================================================================================
JETAPI jeBoolean JETCC jeFaceInfo_ArrayCreateRef(jeFaceInfo_Array *Array)
{
	return jeGArray_CreateRef((jeGArray*)Array);
}

//=======================================================================================
//	jeFaceInfo_ArrayDestroy
//=======================================================================================
JETAPI void JETCC jeFaceInfo_ArrayDestroy(jeFaceInfo_Array **Array)
{
	jeGArray_Destroy((jeGArray**)Array);
}

//=======================================================================================
//	jeFaceInfo_ArrayIndexIsValid
//=======================================================================================
JETAPI jeBoolean JETCC jeFaceInfo_ArrayIndexIsValid(jeFaceInfo_ArrayIndex Index)
{
	if (Index == JE_GARRAY_NULL_INDEX)
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeFaceInfo_ArrayIsValid
//=======================================================================================
JETAPI jeBoolean JETCC jeFaceInfo_ArrayIsValid(const jeFaceInfo_Array* Array)
{
	return (jeGArray_IsValid((jeGArray*)Array));
}

//=======================================================================================
//	jeFaceInfo_ArrayAddFaceInfo
//=======================================================================================
JETAPI jeFaceInfo_ArrayIndex JETCC jeFaceInfo_ArrayAddFaceInfo(jeFaceInfo_Array *Array, const jeFaceInfo *FaceInfo)
{
	jeGArray_Index	Index;

	Index = jeGArray_AddElement((jeGArray*)Array, (jeGArray_Element*)FaceInfo);

	return (jeFaceInfo_ArrayIndex)Index;
}

//=======================================================================================
//	jeFaceInfo_ArrayShareFaceInfo
//=======================================================================================
JETAPI jeFaceInfo_ArrayIndex JETCC jeFaceInfo_ArrayShareFaceInfo(jeFaceInfo_Array *Array, const jeFaceInfo *FaceInfo)
{
	jeFaceInfo			*pFaceInfo;
	jeGArray_RefType	*pRef;
	int32				i;

	pFaceInfo = (jeFaceInfo*)jeGArray_GetElements((jeGArray*)Array);
	pRef = jeGArray_GetRefCounts((jeGArray*)Array);

	for (i=0; i< jeGArray_GetSize((jeGArray*)Array); i++, pRef++, *pFaceInfo++)
	{
		if (*pRef == 0 || *pRef >= JE_GARRAY_MAX_ELEMENT_REFCOUNT)
			continue;					// If it's empty, or totally full, we can't share with it...

		if (jeFaceInfo_Compare(pFaceInfo, FaceInfo))
		{
			assert(*pRef < JE_GARRAY_MAX_ELEMENT_REFCOUNT);
			(*pRef)++;
			return (jeFaceInfo_ArrayIndex)i;
		}
	}

	return jeFaceInfo_ArrayAddFaceInfo(Array, FaceInfo);
}

//=======================================================================================
//	jeFaceInfo_ArrayRefFaceInfoIndex
//=======================================================================================
JETAPI jeBoolean JETCC jeFaceInfo_ArrayRefFaceInfoIndex(jeFaceInfo_Array *Array, jeFaceInfo_ArrayIndex Index)
{
	return jeGArray_RefElement((jeGArray*)Array, Index);
}

//=======================================================================================
//	jeFaceInfo_ArrayRemoveFaceInfo
//=======================================================================================
JETAPI void JETCC jeFaceInfo_ArrayRemoveFaceInfo(jeFaceInfo_Array *Array, jeFaceInfo_ArrayIndex *Index)
{
	jeGArray_RemoveElement((jeGArray*)Array, (jeGArray_Index*)Index);
}

//=======================================================================================
//	jeFaceInfo_ArraySetFaceInfoByIndex
//=======================================================================================
JETAPI void JETCC jeFaceInfo_ArraySetFaceInfoByIndex(jeFaceInfo_Array *Array, jeFaceInfo_ArrayIndex Index, const jeFaceInfo *FaceInfo)
{
	jeGArray_SetElementByIndex((jeGArray*)Array, (jeGArray_Index)Index, (jeGArray_Element*)FaceInfo);
}

//=======================================================================================
//	jeFaceInfo_ArrayGetFaceInfoByIndex
//=======================================================================================
JETAPI const jeFaceInfo * JETCC jeFaceInfo_ArrayGetFaceInfoByIndex(const jeFaceInfo_Array *Array, jeFaceInfo_ArrayIndex Index)
{
	return (jeFaceInfo*)jeGArray_GetElementByIndex((jeGArray*)Array, (jeGArray_Index)Index);
}

