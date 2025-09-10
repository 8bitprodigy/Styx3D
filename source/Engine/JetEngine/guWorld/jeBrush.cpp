/****************************************************************************************/
/*  JEBRUSH.C                                                                           */
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
#ifdef WIN32
#include <windows.h>
#endif

#include <string.h>
#include <assert.h>

// Private dependents
#include "Dcommon.h"
#include "Ram.h"
#include "Errorlog.h"
#include "jeIndexPoly.h"
#include "jeVertArray.h"

#include "Engine.h"
#include "Camera.h"
#include "jeFrustum.h"
#include "jeFaceInfo.h"

#include "jePolyMgr.h"

#include "log.h"

// Public Dependents
#include "jeBrush.h"

//========================================================================================
//========================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)
												   
static jeBoolean jeBrush_WriteHeader(const jeBrush *Brush, jeVFile *VFile);
static jeBoolean jeBrush_WriteFaces(const jeBrush *Brush, jeVFile *VFile);
static jeBoolean jeBrush_ReadHeader(jeBrush *Brush, jeVFile *VFile);
static jeBoolean jeBrush_ReadFaces(jeBrush *Brush, jeVFile *VFile);

static jeBoolean DettachFaceInfoArray(jeBrush *Brush);
static jeBoolean AttachFaceInfoArray(jeBrush *Brush, jeFaceInfo_Array *Array);

static jeBoolean jeBrush_WriteArrays(const jeBrush *Brush, jeVFile *VFile, jePtrMgr *PtrMgr);
static jeBoolean jeBrush_ReadArrays(jeBrush *Brush, jeVFile *VFile, jePtrMgr *PtrMgr);

#define JE_BRUSH_HAS_FACEINFO_ARRAY	(1<<0)

//========================================================================================
//========================================================================================
// The jeBrush
typedef struct jeBrush
{
	int32					RefCount;
	jeVertArray				*VertArray;

	// Face pool
	int32					NumFaces;		// Number of faces in the brush
	jeBrush_Face			*Faces;			// Linked list of faces

	// Header data, and such
	jeXForm3d				XForm;			// XForm to go form model to worldspace
	jeXForm3d				WorldToLocked;	// XForm to go from worldspace to locked space
	jeXForm3d				LockedToWorld;	// XForm to go from locked space to world space
	jeBrush_Contents		Contents;

	// Arrays that this objects inherits from other objects
	jeFaceInfo_Array		*FaceInfoArray;

	uint16					Flags;

	#ifdef _DEBUG
		struct jeBrush		*Self;
	#endif

} jeBrush;

// The jeBrush_Face keeps track of faceinfo, poly (winding of verts, etc...)
typedef struct jeBrush_Face
{
	jeBrush					*Brush;				// Parent brush

	jeIndexPoly				*Poly;
	jeFaceInfo_ArrayIndex	FaceInfoIndex;		// Valid when there is a current FaceInfoArray on the brush
	jeFaceInfo				*FaceInfo;			// Valid when there is NOT a current FaceInfoArray on the brush
	jePlane					Plane;

	struct jeBrush_Face		*Next;
	struct jeBrush_Face		*Prev;
} jeBrush_Face;

//========================================================================================
//	jeBrush_Create
//	Create a jeBrush, with a default of n number of faces.
//	Faces may be added, or removed, by simply changing the size of the FaceArray
//========================================================================================
JETAPI jeBrush* JETCC jeBrush_Create(int32 EstimatedVerts)
{
	jeBrush		*Brush;

	Brush = JE_RAM_ALLOCATE_STRUCT(jeBrush);

	if (!Brush)
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeBrush_Create:  Out of ram for brush.", NULL);
		return NULL;
	}

	// Clear out the brush
	ZeroMem(Brush);

#ifdef _DEBUG
	Brush->Self = Brush;		// For safety checking
#endif

	jeXForm3d_SetIdentity(&Brush->XForm);
	jeXForm3d_SetIdentity(&Brush->WorldToLocked);
	jeXForm3d_SetIdentity(&Brush->LockedToWorld);

	// Create the brushes VArray
	Brush->VertArray = jeVertArray_Create(EstimatedVerts);

	if (!Brush->VertArray)
		goto ExitWithError;	

	// Create the very first ref
	if (!jeBrush_CreateRef(Brush))
		goto ExitWithError;

	return Brush;
	
	// Error, cleanup
	ExitWithError:
	{
		if (Brush)
		{
			if (Brush->VertArray)
				jeVertArray_Destroy(&Brush->VertArray);

			jeRam_Free(Brush);
		}
		return NULL;
	}
}

//========================================================================================
//	jeBrush_CreateFromFile
//========================================================================================
JETAPI jeBrush * JETCC jeBrush_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr)
{
	jeBrush		*Brush;

	if (PtrMgr)
	{
		if (!jePtrMgr_ReadPtr(PtrMgr, VFile, (void **)&Brush))
			return NULL;

		if (Brush)
		{
			if (!jeBrush_CreateRef(Brush))
				return NULL;

			return Brush;		// Ptr found in stack, return it
		}
	}

	Brush = JE_RAM_ALLOCATE_STRUCT(jeBrush);

	if (!Brush)
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeBrush_CreateFromFile:  Out of ram for brush.", NULL);
		return NULL;
	}

	// Clear out the brush
	ZeroMem(Brush);

#ifdef _DEBUG
	Brush->Self = Brush;		// For safety checking
#endif

	// Create the very first ref
	if (!jeBrush_CreateRef(Brush))
		goto ExitWithError;	

	if (!jeBrush_ReadHeader(Brush, VFile))
		goto ExitWithError;

	if (!jeBrush_ReadArrays(Brush, VFile, PtrMgr))
		goto ExitWithError;
		
	if (!jeBrush_ReadFaces(Brush, VFile))
		goto ExitWithError;

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, Brush))
			goto ExitWithError;
	}

	return Brush;
	
	// Error, cleanup
	ExitWithError:
	{
		if (Brush)
		{
			if (Brush->VertArray)
				jeVertArray_Destroy(&Brush->VertArray);

			jeRam_Free(Brush);
		}
		return NULL;
	}
}


//========================================================================================
//	jeBrush_WriteToFile
//========================================================================================
JETAPI jeBoolean JETCC jeBrush_WriteToFile(const jeBrush *Brush, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);
	assert(VFile);

	if (PtrMgr)
	{
		uint32		Count;

		if (!jePtrMgr_WritePtr(PtrMgr, VFile, (void*)Brush, &Count))
			return JE_FALSE;

		if (Count)
			return JE_TRUE;		// Ptr was on stack, so return
	}

	// Write the header
	if (!jeBrush_WriteHeader(Brush, VFile))
	{
		jeErrorLog_AddString(-1, "jeBrush_WriteToFile:  jeBrush_WriteHeader failed.\n", NULL);
		return JE_FALSE;
	}

	// Save the arrays out
	if (!jeBrush_WriteArrays(Brush, VFile, PtrMgr))
	{
		jeErrorLog_AddString(-1, "jeBrush_WriteToFile:  jeBrush_WriteArrays failed.\n", NULL);
		return JE_FALSE;
	}

	// Save all the faces
	if (!jeBrush_WriteFaces(Brush, VFile))
	{
		jeErrorLog_AddString(-1, "jeBrush_WriteToFile:  jeBrush_WriteFaces failed.\n", NULL);
		return JE_FALSE;
	}

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, (void*)Brush))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	jeBrush_CreateRef
//========================================================================================
JETAPI jeBoolean JETCC jeBrush_CreateRef(jeBrush *Brush)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);
	assert(Brush->RefCount >= 0);

	Brush->RefCount++;

	return JE_TRUE;
}

//========================================================================================
//	jeBrush_Destroy
//========================================================================================
JETAPI void JETCC jeBrush_Destroy(jeBrush **Brush)
{
	jeBrush_Face	*Face, *Next;

	assert(Brush);
	assert(*Brush);
	assert((*Brush)->RefCount > 0);
	assert((*Brush)->VertArray);

	(*Brush)->RefCount--;

	if ((*Brush)->RefCount == 0)
	{
		jeBoolean		Ret;

		for (Face = (*Brush)->Faces; Face; Face = Next)
		{
			Next = Face->Next;
		
			jeBrush_DestroyFace(*Brush, &Face);
		}

		assert((*Brush)->Faces == NULL);
	
		Ret = DettachFaceInfoArray(*Brush);
		assert(Ret == JE_TRUE);

		jeVertArray_Destroy(&(*Brush)->VertArray);

		jeRam_Free(*Brush);
	}

	*Brush = NULL;
}

//========================================================================================
//	jeBrush_IsValid
//========================================================================================
JETAPI jeBoolean JETCC jeBrush_IsValid(const jeBrush *Brush)
{
	jeBrush_Face	*Face;
	int32			NumFaces;

	if (!Brush)
		return JE_FALSE;

#ifdef _DEBUG
	if (Brush->Self != Brush)
		return JE_FALSE;
#endif

	NumFaces = 0;
	for (Face = Brush->Faces; Face; Face = Face->Next)
	{
		assert(Face->Poly);
		NumFaces++;
	}

	if (Brush->NumFaces != NumFaces)
		return JE_FALSE;
	
	return JE_TRUE;
}

//=======================================================================================
//	jeBrush_IsConvex
//=======================================================================================
JETAPI jeBoolean JETCC jeBrush_IsConvex(const jeBrush *Brush)
{
	jeBrush_Face		*Face1;
	jeBrush_Face		*Face2;

	for (Face1 = Brush->Faces; Face1; Face1 = Face1->Next)
	{
		if (!jeIndexPoly_IsConvex(Face1->Poly, &Face1->Plane.Normal, Face1->Brush->VertArray))
			return JE_FALSE;

		for (Face2 = Brush->Faces; Face2; Face2 = Face2->Next)
		{
			int32			i;

			// Notice how we check Face1 against itself...  This is because we want to make sure
			// that the triangles within the faces themselves are also convex...

			for (i=0; i< Face2->Poly->NumVerts; i++)
			{
				const jeVec3d	*pVert;
				jeFloat			Val;

				pVert = jeVertArray_GetVertByIndex(Brush->VertArray, Face2->Poly->Verts[i]);

				Val = jeVec3d_DotProduct(&Face1->Plane.Normal,  pVert);

				if (Val < 0)
					return JE_FALSE;
			}
		}
	}

	return JE_TRUE;			// Brush is convex
}

//========================================================================================
//	jeBrush_SetContents
//========================================================================================
JETAPI void JETCC jeBrush_SetContents(jeBrush *Brush, jeBrush_Contents Contents)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);

	Brush->Contents = Contents;
}

//========================================================================================
//	jeBrush_GetContents
//========================================================================================
JETAPI jeBrush_Contents JETCC jeBrush_GetContents(const jeBrush *Brush)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);

	return Brush->Contents;
}

//========================================================================================
//	jeBrush_SetXForm
//========================================================================================
JETAPI void JETCC jeBrush_SetXForm(jeBrush *Brush, const jeXForm3d *XForm, jeBoolean Locked)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);

	if (Locked)
	{
	#if 1	
		{
		jeXForm3d		Inverse;
		jeXForm3d		xCopy = *XForm;

		xCopy.Flags = 0;
		jeXForm3d_Orthonormalize(&xCopy);

		// Get the Inverse of the input orthonormalise XForm
		jeXForm3d_GetInverse(&xCopy, &Inverse);
		
		// XForm the current xform by it
		Brush->XForm.Flags = 0;
		jeXForm3d_Orthonormalize(&Brush->XForm);

		jeXForm3d_Multiply(&Brush->XForm, &Inverse, &Inverse);

		// XForm the current locked xform by this resuling xform
		jeXForm3d_Multiply(&Brush->WorldToLocked, &Inverse, &Brush->WorldToLocked);

		jeXForm3d_GetInverse(&Brush->WorldToLocked, &Brush->LockedToWorld);
		}
	#else		// Test code, that does not save rotation or scaling
		{
		jeVec3d			Diff, Save;
		Save = Brush->WorldToLocked.Translation;

		jeXForm3d_SetIdentity(&Brush->WorldToLocked);
		jeXForm3d_SetIdentity(&Brush->LockedToWorld);

		jeVec3d_Subtract(&XForm->Translation, &Brush->XForm.Translation, &Diff);

		jeVec3d_Subtract(&Save, &Diff, &Brush->WorldToLocked.Translation);

		Brush->LockedToWorld.Translation = Brush->WorldToLocked.Translation;
		jeVec3d_Inverse(&Brush->LockedToWorld.Translation);
		}
	#endif

	}
	else
	{
		jeXForm3d_SetIdentity(&Brush->WorldToLocked);
		jeXForm3d_SetIdentity(&Brush->LockedToWorld);
	}
	
	Brush->XForm = *XForm;
}

//========================================================================================
//	jeBrush_GetXForm
//========================================================================================
JETAPI const jeXForm3d * JETCC jeBrush_GetXForm(jeBrush *Brush)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);

	return &Brush->XForm;
}

//========================================================================================
//	jeBrush_GetWorldToLockedXForm
//========================================================================================
JETAPI const jeXForm3d * JETCC jeBrush_GetWorldToLockedXForm(jeBrush *Brush)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);

	return &Brush->WorldToLocked;
}

//========================================================================================
//	jeBrush_GetLockedToWorldXForm
//========================================================================================
JETAPI const jeXForm3d * JETCC jeBrush_GetLockedToWorldXForm(jeBrush *Brush)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);

	return &Brush->LockedToWorld;
}

//========================================================================================
//	jeBrush_GetVertArray
//========================================================================================
JETAPI jeVertArray * JETCC jeBrush_GetVertArray(const jeBrush *Brush)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);

	return Brush->VertArray;
}

//========================================================================================
//	jeBrush_SetFaceInfoArray
//========================================================================================
JETAPI jeBoolean JETCC jeBrush_SetFaceInfoArray(jeBrush *Brush, jeFaceInfo_Array *Array)
{
	assert(jeFaceInfo_ArrayIsValid(Array));

	if (!DettachFaceInfoArray(Brush))
		return JE_FALSE;

	assert(jeFaceInfo_ArrayIsValid(Array));

	if (!AttachFaceInfoArray(Brush, Array))
		return JE_FALSE;
	
	return JE_TRUE;
}

#define JE_BRUSH_LINK_FACE(b, f) {	\
			f->Next = b->Faces;		\
			if (b->Faces)			\
				b->Faces->Prev = f;	\
			b->Faces = f;			\
									\
			b->NumFaces++;			\
									\
			f->Brush = b;			\
			}

//========================================================================================
//	jeBrush_CreateFace
//========================================================================================
JETAPI jeBrush_Face * JETCC jeBrush_CreateFace(jeBrush *Brush, int32 NumVerts)
{
	jeBrush_Face		*Face;

	assert(Brush);

	Face = JE_RAM_ALLOCATE_STRUCT(jeBrush_Face);

	if (!Face)
		return NULL;

	ZeroMem(Face);

	// Create the poly for the face
	Face->Poly = jeIndexPoly_Create((jeVertArray_Index)NumVerts);

	if (!Face->Poly)
		goto ExitWithError;

	// Initialize the FaceInfoIndex to NULL
	Face->FaceInfoIndex = JE_FACEINFO_ARRAY_NULL_INDEX;

	// Create the FaceInfo.  We use this till we have an array to stuff it in
	Face->FaceInfo = JE_RAM_ALLOCATE_STRUCT(jeFaceInfo);

	// Set the FaceInfo defaults
	jeFaceInfo_SetDefaults(Face->FaceInfo);

	if (!Face->FaceInfo)
		goto ExitWithError;

	JE_BRUSH_LINK_FACE(Brush, Face);

	return Face;
	
	ExitWithError:
	{
		if (Face)
		{
			if (Face->Poly)
				jeIndexPoly_Destroy(&Face->Poly);

			if (Face->FaceInfo)
				jeRam_Free(Face->FaceInfo);

			jeRam_Free(Face);
		}

		return NULL;
	}
}

//========================================================================================
//	jeBrush_CreateFaceFromFile
//========================================================================================
JETAPI jeBrush_Face * JETCC jeBrush_CreateFaceFromFile(jeBrush *Brush, jeVFile *VFile)
{
	jeBrush_Face		*Face;

	Face = JE_RAM_ALLOCATE_STRUCT(jeBrush_Face);

	if (!Face)
		return NULL;

	ZeroMem(Face);

	if (Brush->FaceInfoArray)
	{
		// Read the FaceInfoIndex
		if (!jeVFile_Read(VFile, &Face->FaceInfoIndex, sizeof(Face->FaceInfoIndex)))
			goto ExitWithError;

		// Ref our copy of the FaceInfoIndex
		if (!jeFaceInfo_ArrayRefFaceInfoIndex(Brush->FaceInfoArray, Face->FaceInfoIndex))
			goto ExitWithError;
	}
	else
	{
		Face->FaceInfo = JE_RAM_ALLOCATE_STRUCT(jeFaceInfo);

		if (!Face->FaceInfo)
			goto ExitWithError;

		// Read in the FaceInfo
		if (!jeVFile_Read(VFile, Face->FaceInfo, sizeof(jeFaceInfo)))
			goto ExitWithError;
	}
	
	// Create this face's IndexPoly
	Face->Poly = jeIndexPoly_CreateFromFile(VFile);

	if (!Face->Poly)
		goto ExitWithError;

	JE_BRUSH_LINK_FACE(Brush, Face);

	return Face;

	ExitWithError:
	{
		if (Face)
		{
			if (Face->Poly)
				jeIndexPoly_Destroy(&Face->Poly);

			if (Face->FaceInfo)
				jeRam_Free(Face->FaceInfo);

			jeRam_Free(Face);
		}

		return NULL;
	}
}

//========================================================================================
//	jeBrush_WriteFaceToFile
//========================================================================================
JETAPI jeBoolean JETCC jeBrush_WriteFaceToFile(const jeBrush *Brush, const jeBrush_Face *Face, jeVFile *VFile)
{
	if (Brush->FaceInfoArray)
	{
		assert(!Face->FaceInfo);

		// Write out the FaceInfoIndex
		if (!jeVFile_Write(VFile, &Face->FaceInfoIndex, sizeof(Face->FaceInfoIndex)))
			return JE_FALSE;
	}
	else
	{
		assert(Face->FaceInfo);

		// Write out the FaceInfo
		if (!jeVFile_Write(VFile, Face->FaceInfo, sizeof(jeFaceInfo)))
			return JE_FALSE;
	}

	// Write out this face's poly
	if (!jeIndexPoly_WriteToFile(Face->Poly, VFile))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeBrush_DestroyFace
//========================================================================================
JETAPI void JETCC jeBrush_DestroyFace(jeBrush *Brush, jeBrush_Face **Face)
{
	jeBrush_Face		*pFace;
	int32				v;

	assert(Face);
	assert(*Face);
	assert((*Face)->Brush == Brush);
	assert((*Face)->Poly);

	assert(Brush->NumFaces > 0);

	pFace = *Face;

	if (pFace->Next)
		pFace->Next->Prev = pFace->Prev;

	if (Brush->Faces == pFace)
	{
		assert(pFace->Prev == NULL);
		Brush->Faces = pFace->Next;
	}
	else
	{
		assert(pFace->Prev != NULL);
		pFace->Prev->Next = pFace->Next;
	}

	for (v=0; v< pFace->Poly->NumVerts; v++)
		if (pFace->Poly->Verts[v] != JE_VERTARRAY_NULL_INDEX)
			jeVertArray_RemoveVert(Brush->VertArray, &pFace->Poly->Verts[v]);

	jeIndexPoly_Destroy(&pFace->Poly);

	if (pFace->FaceInfo)
	{
		assert(pFace->FaceInfoIndex == JE_FACEINFO_ARRAY_NULL_INDEX);
		jeRam_Free(pFace->FaceInfo);
	}
	else
	{
		assert(pFace->FaceInfoIndex != JE_FACEINFO_ARRAY_NULL_INDEX);
		assert(Brush->FaceInfoArray);

		jeFaceInfo_ArrayRemoveFaceInfo(Brush->FaceInfoArray, &pFace->FaceInfoIndex);
	}

	jeRam_Free(pFace);

	*Face = NULL;
			
	Brush->NumFaces--;
}

//========================================================================================
//	jeBrush_GetFaceCount
//========================================================================================
JETAPI int32 JETCC jeBrush_GetFaceCount(const jeBrush *Brush)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);

	return Brush->NumFaces;
}

//========================================================================================
//	jeBrush_GetNextFace
//========================================================================================
JETAPI jeBrush_Face * JETCC jeBrush_GetNextFace(const jeBrush *Brush, const jeBrush_Face *Start)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);

	if (!Start)
		return Brush->Faces;

	return Start->Next;
}

//========================================================================================
//	jeBrush_GetPrevFace
//	Note that GetPrev face CANNOT take NULL as start.  Must be valid face...
//========================================================================================
JETAPI jeBrush_Face * JETCC jeBrush_GetPrevFace(const jeBrush *Brush, const jeBrush_Face *Start)
{
	assert(jeBrush_IsValid(Brush) == JE_TRUE);
	assert(Start);

	return Start->Prev;
}

//========================================================================================
//	jeBrush_GetFaceByIndex
//========================================================================================
JETAPI jeBrush_Face * JETCC jeBrush_GetFaceByIndex(const jeBrush *Brush, int32 Index)
{
	jeBrush_Face	*Face;

	assert(jeBrush_IsValid(Brush) == JE_TRUE);
	assert(Index >= 0 && Index < Brush->NumFaces);

	// Jump to index in linked list
	for (Face = Brush->Faces; Face && Index > 0; Index--, Face=Face->Next);

	return Face;
}

//========================================================================================
//	jeBrush_FaceGetBrush
//========================================================================================
JETAPI jeBrush * JETCC jeBrush_FaceGetBrush(const jeBrush_Face *Face)
{
	assert(Face);
	assert(Face->Poly);

	return Face->Brush;
}

//========================================================================================
//	jeBrush_FaceGetVertCount
//========================================================================================
JETAPI int32 JETCC jeBrush_FaceGetVertCount(const jeBrush_Face *Face)
{
	assert(Face);
	assert(Face->Poly);

	return Face->Poly->NumVerts;
}

//========================================================================================
//	jeBrush_FaceSetVertByIndex
//========================================================================================
JETAPI void JETCC jeBrush_FaceSetVertByIndex(jeBrush_Face *Face, int32 Index, const jeVec3d *Vert)
{
	jeVertArray_Index	*VIndex;

	assert(Face);
	assert(Index < JE_INDEXPOLY_MAX_VERTS);

	VIndex = &Face->Poly->Verts[Index];

	if (*VIndex != JE_VERTARRAY_NULL_INDEX)
		jeVertArray_RemoveVert(Face->Brush->VertArray, VIndex);

	*VIndex = jeVertArray_ShareVert(Face->Brush->VertArray, Vert);

	jeBrush_FaceCalcPlane(Face);
}

//========================================================================================
//	jeBrush_FaceMoveVertByIndex
//========================================================================================
JETAPI void JETCC jeBrush_FaceMoveVertByIndex(jeBrush_Face *Face, int32 Index, const jeVec3d *Vert)
{
	jeVertArray_Index	VIndex;

	assert(Face);
	assert(Index < JE_INDEXPOLY_MAX_VERTS);

	VIndex = Face->Poly->Verts[Index];

	jeVertArray_SetVertByIndex(Face->Brush->VertArray, VIndex, Vert);

	jeBrush_FaceCalcPlane(Face);
}

//========================================================================================
//	jeBrush_FaceGetVertByIndex
//========================================================================================
JETAPI const jeVec3d * JETCC jeBrush_FaceGetVertByIndex(const jeBrush_Face *Face, int32 Index)
{
	assert(Face);
	assert(Index < JE_INDEXPOLY_MAX_VERTS);

	return jeVertArray_GetVertByIndex(Face->Brush->VertArray, Face->Poly->Verts[Index]);
}

//========================================================================================
//	jeBrush_FaceGetWorldSpaceVertByIndex
//========================================================================================
JETAPI jeVec3d JETCC jeBrush_FaceGetWorldSpaceVertByIndex(const jeBrush_Face *Face, int32 Index)
{
	jeVec3d		Vert;

	assert(Face);
	assert(Index < JE_INDEXPOLY_MAX_VERTS);

	Vert = *jeVertArray_GetVertByIndex(Face->Brush->VertArray, Face->Poly->Verts[Index]);

	jeXForm3d_Transform(&Face->Brush->XForm, &Vert, &Vert);

	return Vert;
}

//========================================================================================
//	jeBrush_FaceCalcPlane
//========================================================================================
JETAPI void JETCC jeBrush_FaceCalcPlane(jeBrush_Face *Face)
{
	const jeVec3d	*pVerts[3];
	int32			i;

	for (i=0; i<3; i++)
	{
		if (Face->Poly->Verts[i] == JE_VERTARRAY_NULL_INDEX)
			break;

		pVerts[i] = jeVertArray_GetVertByIndex(Face->Brush->VertArray, Face->Poly->Verts[i]);
		assert(pVerts[i]);
	}

	if (i != 3)
	{
		memset(&Face->Plane, 0, sizeof(jePlane));
		return;
	}
	
	jePlane_SetFromVerts(&Face->Plane, pVerts[0], pVerts[1], pVerts[2]);
}

//========================================================================================
//	jeBrush_FaceGetFaceInfoIndex
//========================================================================================
JETAPI jeFaceInfo_ArrayIndex JETCC jeBrush_FaceGetFaceInfoIndex(jeBrush_Face *Face)
{
	return Face->FaceInfoIndex;
}

//========================================================================================
//	jeBrush_FaceGetFaceInfo
//========================================================================================
JETAPI jeBoolean JETCC jeBrush_FaceGetFaceInfo(jeBrush_Face *Face, jeFaceInfo *FaceInfo)
{
	if (Face->FaceInfoIndex == JE_FACEINFO_ARRAY_NULL_INDEX)
	{
		assert(Face->FaceInfo);
		assert(!Face->Brush->FaceInfoArray);

		*FaceInfo = *Face->FaceInfo;
	}
	else
	{
		const jeFaceInfo *pFaceInfo;

		assert(!Face->FaceInfo);
		assert(Face->Brush->FaceInfoArray);

		pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(Face->Brush->FaceInfoArray, Face->FaceInfoIndex);

		if (!pFaceInfo)
			return JE_FALSE;

		*FaceInfo = *pFaceInfo;
	}

	return JE_TRUE;
}

//========================================================================================
//	jeBrush_FaceSetFaceInfo
//========================================================================================
JETAPI jeBoolean JETCC jeBrush_FaceSetFaceInfo(jeBrush_Face *Face, const jeFaceInfo *FaceInfo)
{
	if (Face->FaceInfoIndex == JE_FACEINFO_ARRAY_NULL_INDEX)
	{
		assert(Face->FaceInfo);
		assert(!Face->Brush->FaceInfoArray);

		*Face->FaceInfo = *FaceInfo;
	}
	else
	{
		assert(!Face->FaceInfo);
		assert(Face->Brush->FaceInfoArray);

		// Remove any old faceinfo 
		jeFaceInfo_ArrayRemoveFaceInfo(Face->Brush->FaceInfoArray, &Face->FaceInfoIndex);
		
		Face->FaceInfoIndex = jeFaceInfo_ArrayShareFaceInfo(Face->Brush->FaceInfoArray, FaceInfo);

		if (Face->FaceInfoIndex == JE_FACEINFO_ARRAY_NULL_INDEX)
			return JE_FALSE;
	}

	return JE_TRUE;
}

static jeBoolean jeBrush_FaceRender(const jeBrush_Face *Face, const DRV_Driver *Driver, const jeCamera *Camera, const jeFrustum *Frustum);

//========================================================================================
//	jeBrush_Render
//========================================================================================
JETAPI jeBoolean JETCC jeBrush_Render(const jeBrush *Brush, const jeEngine *Engine, const jeCamera *Camera)
{
	jeBrush_Face		*Face;
	const DRV_Driver			*Driver;
	jeFrustum			Frustum, WorldSpaceFrustum;

	Driver = jeEngine_GetDriver(Engine);

	assert(Driver);

	jeFrustum_SetFromCamera(&Frustum, Camera);
	jeFrustum_TransformToWorldSpace(&Frustum, Camera, &WorldSpaceFrustum);
	
	for (Face = Brush->Faces; Face; Face = Face->Next)
	{
		if (!jeBrush_FaceRender(Face, Driver, Camera, &WorldSpaceFrustum))
			return JE_FALSE;
	}

	return JE_TRUE;
}

#define MAX_TEMP_VERTS		64

//extern jePolyMgr		*HackPolyMgr;

//========================================================================================
//	jeBrush_FaceRender
//========================================================================================
jeBoolean jeBrush_FaceRender(const jeBrush_Face *Face, const DRV_Driver *Driver, const jeCamera *Camera, const jeFrustum *Frustum)
{
	int32				NumVerts;
	jeLVertex			LVerts1[MAX_TEMP_VERTS], LVerts2[MAX_TEMP_VERTS];
	jeLVertex			*pLVert;
	jeTLVertex			TLVerts[MAX_TEMP_VERTS];
	int32				i;
	jeFrustum_LClipInfo	ClipInfo;

	pLVert = LVerts1;				// Fill in this array with x,y,z,u,v,r,g,b...

	NumVerts = Face->Poly->NumVerts;
	assert(NumVerts+4 < MAX_TEMP_VERTS);

	// Copy the index verts, and the uvrgb's into a jeLVertex structure
	for (i=0; i<NumVerts; i++)
	{
		jeVec3d	pSrcVert;

		pSrcVert = jeBrush_FaceGetWorldSpaceVertByIndex(Face, i);

		pLVert->X = pSrcVert.X;
		pLVert->Y = pSrcVert.Y;
		pLVert->Z = pSrcVert.Z;

		pLVert++;
	}
	
	// Fill LVerts2 with clipped LVerts1
	ClipInfo.SrcVerts = LVerts1;
	ClipInfo.NumSrcVerts = NumVerts;
	ClipInfo.Work1 = LVerts1;
	ClipInfo.Work2 = LVerts2;
	ClipInfo.ClipFlags = 0xffffffff;

	// Clip the verts against the frustum
	if (!jeFrustum_ClipLVertsXYZUV(Frustum, &ClipInfo))
		return JE_TRUE;		// Poly was clipped away

	for (pLVert = ClipInfo.DstVerts, i=0; i<ClipInfo.NumDstVerts; i++, pLVert++)
	{
		pLVert->r = pLVert->g = 255.0f;
		pLVert->b = 0.0f;
		pLVert->a = 110.0f;
	}

	// Transform and project the point
	jeCamera_TransformAndProjectAndClampLArray(Camera, ClipInfo.DstVerts, TLVerts, ClipInfo.NumDstVerts);

	Driver->RenderGouraudPoly(TLVerts, ClipInfo.NumDstVerts, JE_RENDER_FLAG_ALPHA);

	return JE_TRUE;
}

//========================================================================================
//	****** local static functions ********
//========================================================================================

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
		((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

#define JE_BRUSH_TAG				MAKEFOURCC('G', 'E', 'B', 'F')		// 'GE' 'B'rush 'F'ile
#define JE_BRUSH_VERSION			0x0000

//========================================================================================
//	jeBrush_WriteHeader
//========================================================================================
static jeBoolean jeBrush_WriteHeader(const jeBrush *Brush, jeVFile *VFile)
{
	uint32		Tag;
	uint16		Version;

	assert(Brush);
	assert(VFile);

	// Write TAG
	Tag = JE_BRUSH_TAG;

	if (!jeVFile_Write(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	// Write version
	Version = JE_BRUSH_VERSION;

	if (!jeVFile_Write(VFile, &Version, sizeof(Version)))
		return JE_FALSE;

	// Write out the Brush flags
	if (!jeVFile_Write(VFile, &Brush->Flags, sizeof(Brush->Flags)))
		return JE_FALSE;

	// Write out misc data
	if (!jeVFile_Write(VFile, &Brush->XForm, sizeof(Brush->XForm)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &Brush->WorldToLocked, sizeof(Brush->WorldToLocked)))
		return JE_FALSE;

	if (!jeVFile_Write(VFile, &Brush->Contents, sizeof(Brush->Contents)))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeBrush_WriteArrays
//========================================================================================
static jeBoolean jeBrush_WriteArrays(const jeBrush *Brush, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	if (!jeVertArray_WriteToFile(Brush->VertArray, VFile))
		return JE_FALSE;

	if (Brush->FaceInfoArray)
	{
		assert(Brush->Flags & JE_BRUSH_HAS_FACEINFO_ARRAY);

		if (!jeFaceInfo_ArrayWriteToFile(Brush->FaceInfoArray, VFile, NULL, NULL, PtrMgr))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
// jeBrush_WriteFaces
//========================================================================================
static jeBoolean jeBrush_WriteFaces(const jeBrush *Brush, jeVFile *VFile)
{
	jeBrush_Face		*Face;
	int32				NumFaces;

	// Count faces
	NumFaces = 0;
	for (Face = Brush->Faces; Face; Face = Face->Next)
		NumFaces++;

	// Write out number of faces
	if (!jeVFile_Write(VFile, &NumFaces, sizeof(NumFaces)))
		return JE_FALSE;

	for (Face = Brush->Faces; Face; Face = Face->Next)
	{
		if (!jeBrush_WriteFaceToFile(Brush, Face, VFile))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	jeBrush_ReadHeader
//========================================================================================
static jeBoolean jeBrush_ReadHeader(jeBrush *Brush, jeVFile *VFile)
{
	uint32		Tag;
	uint16		Version;

	assert(Brush);
	assert(VFile);

	if (!jeVFile_Read(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	if (Tag != JE_BRUSH_TAG)
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &Version, sizeof(Version)))
		return JE_FALSE;

	if (Version != JE_BRUSH_VERSION)
		return JE_FALSE;

	// Read the Brush flags
	if (!jeVFile_Read(VFile, &Brush->Flags, sizeof(Brush->Flags)))
		return JE_FALSE;

	// Read misc data
	if (!jeVFile_Read(VFile, &Brush->XForm, sizeof(Brush->XForm)))
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &Brush->WorldToLocked, sizeof(Brush->WorldToLocked)))
		return JE_FALSE;

	// Get the LockedToWorld XForm off the WorldToLocked XForm
	jeXForm3d_GetInverse(&Brush->WorldToLocked, &Brush->LockedToWorld);

	if (!jeVFile_Read(VFile, &Brush->Contents, sizeof(Brush->Contents)))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeBrush_ReadArrays
//========================================================================================
static jeBoolean jeBrush_ReadArrays(jeBrush *Brush, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	assert(Brush);
	assert(VFile);

	Brush->VertArray = jeVertArray_CreateFromFile(VFile);

	if (!Brush->VertArray)
		goto ExitWithError;

	if (Brush->Flags & JE_BRUSH_HAS_FACEINFO_ARRAY)
	{
		Brush->FaceInfoArray = jeFaceInfo_ArrayCreateFromFile(VFile, NULL, NULL, PtrMgr);

		if (!Brush->FaceInfoArray)
			goto ExitWithError;
	}

	return JE_TRUE;

	ExitWithError:
	{
		if (Brush->VertArray)
			jeVertArray_Destroy(&Brush->VertArray);
		if (Brush->FaceInfoArray)
			jeFaceInfo_ArrayDestroy(&Brush->FaceInfoArray);

		return JE_FALSE;
	}
}

//========================================================================================
//	jeBrush_ReadFaces
//========================================================================================
static jeBoolean jeBrush_ReadFaces(jeBrush *Brush, jeVFile *VFile)
{
	int32	NumFaces, i;

	assert(Brush);
	assert(VFile);

	// Read number of faces
	if (!jeVFile_Read(VFile, &NumFaces, sizeof(NumFaces)))
		return JE_FALSE;

	for (i=0; i< NumFaces; i++)
	{
		jeBrush_Face	*Face;

		Face = jeBrush_CreateFaceFromFile(Brush, VFile);

		if (!Face)
			return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	DettachFaceInfoArray
//========================================================================================
static jeBoolean DettachFaceInfoArray(jeBrush *Brush)
{
	jeBrush_Face	*Face;

	if (!Brush->FaceInfoArray)
	{
		assert(!(Brush->Flags & JE_BRUSH_HAS_FACEINFO_ARRAY));
		return JE_TRUE;				// Nothing to dettach
	}

	assert(Brush->Flags & JE_BRUSH_HAS_FACEINFO_ARRAY);

	// Save off the FaceInfo in a copy, and remove the index from the array for each face
	for (Face = Brush->Faces; Face; Face = Face->Next)
	{
		assert(!Face->FaceInfo);

		Face->FaceInfo = JE_RAM_ALLOCATE_STRUCT(jeFaceInfo);

		if (!Face->FaceInfo)
			return JE_FALSE;

		// Save off the FaceInfo
		*(Face->FaceInfo) = *jeFaceInfo_ArrayGetFaceInfoByIndex(Brush->FaceInfoArray, Face->FaceInfoIndex);

		// Remove the index from the current Array
		jeFaceInfo_ArrayRemoveFaceInfo(Brush->FaceInfoArray, &Face->FaceInfoIndex);
	}

	// Destroy the ref on the array
	jeFaceInfo_ArrayDestroy(&Brush->FaceInfoArray);

	Brush->FaceInfoArray = NULL;
	Brush->Flags &= ~JE_BRUSH_HAS_FACEINFO_ARRAY;

	return JE_TRUE;
}

//========================================================================================
//	AttachFaceInfoArray
//========================================================================================
static jeBoolean AttachFaceInfoArray(jeBrush *Brush, jeFaceInfo_Array *Array)
{
	jeBrush_Face	*Face;

	if (!Array)
		return JE_TRUE;

	assert(!Brush->FaceInfoArray);
	assert(!(Brush->Flags & JE_BRUSH_HAS_FACEINFO_ARRAY));

	for (Face = Brush->Faces; Face; Face = Face->Next)
	{
		assert(Face->FaceInfo);

		// Get the index fromt he faceinfo
		Face->FaceInfoIndex = jeFaceInfo_ArrayShareFaceInfo(Array, Face->FaceInfo);

		if (Face->FaceInfoIndex == JE_FACEINFO_ARRAY_NULL_INDEX)
			return JE_FALSE;

		// Free the faceinfo
		jeRam_Free(Face->FaceInfo);
		Face->FaceInfo = NULL;
	}

	// Ref new array
	if (!jeFaceInfo_ArrayCreateRef(Array))
		return JE_FALSE;

	Brush->FaceInfoArray = Array;
	Brush->Flags |= JE_BRUSH_HAS_FACEINFO_ARRAY;

	return JE_TRUE;
}
