/****************************************************************************************/
/*  JEBSP_TOPBRUSH.C                                                                    */
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

// The jeBSP_Top brush is the top level brush in the BSP world.  jeBSP_TopBrushes
// are created from jeBrushes.  Once this happens, no dependency exist between
// jeBSP_TopBrushes, and jeBrushes.  This was done so jeBSP's could be saved independent
// of jeBrushes.  The actual tree is built by creating a list of jeBSP_Brushes from
// jeBSP_TopBrushes.  This is done, because jeBSP_Brushes need to be as small as possible.
// The jeBSP_Brushes are then partitioned up, and the tree is born...
//		-John Pollard

#include <assert.h>
#include <stdio.h>
#include <memory.h>		// memset

// Private dependents
#include "jeBSP._h"
#include "Ram.h"
#include "Quatern.h"

// Public dependents
#include "jeBSP.h"

//=======================================================================================
//	jeBSP_TopBrushCreate
//=======================================================================================
jeBSP_TopBrush *jeBSP_TopBrushCreate(int32 NumSides, uint32 Order)
{
	jeBSP_TopBrush	*Brush;
	int32			c, i;

	assert(NumSides < 65535);

	// Get the size needed to accomodate the number of sides
	c = (sizeof(jeBSP_TopBrush)-sizeof(jeBSP_TopSide[JE_BSP_BRUSH_DEFAULT_SIDES]))+(sizeof(jeBSP_TopSide)*NumSides);

	// Allocate the brush
	Brush = (jeBSP_TopBrush*)jeRam_Allocate(c);

	if (!Brush)
		return NULL;

	memset (Brush, 0, c);

	Brush->NumSides = (uint16)NumSides;

	for (i=0; i<NumSides; i++)
	{
		Brush->TopSides[i].PlaneIndex = JE_PLANEARRAY_NULL_INDEX;
		Brush->TopSides[i].FaceInfoIndex = JE_FACEINFO_ARRAY_NULL_INDEX;
		Brush->TopSides[i].TexVecIndex = JE_TEXVEC_ARRAY_NULL_INDEX;
	}

	Brush->Order = Order;

	return Brush;
}

//=======================================================================================
//	jeBSP_TopBrushDestroy
//=======================================================================================
void jeBSP_TopBrushDestroy(jeBSP_TopBrush **Brush, jeBSP *BSP)
{
	jeBSP_TopSide		*Side;
	int32				i;

	assert(Brush);
	assert(*Brush);

	#pragma message ("Need a jeBrush_FaceDestroy here for all TopSides...")
	#pragma message ("Need a jeBrush_Destroy here...")
	
	for (Side = (*Brush)->TopSides, i=0; i<(*Brush)->NumSides; i++, Side++)
	{
		if (Side->PlaneIndex != JE_PLANEARRAY_NULL_INDEX)
			jePlaneArray_RemovePlane(BSP->PlaneArray, &Side->PlaneIndex);

		if (Side->FaceInfoIndex != JE_FACEINFO_ARRAY_NULL_INDEX)
			jeFaceInfo_ArrayRemoveFaceInfo(BSP->FaceInfoArray, &Side->FaceInfoIndex);

		if (Side->TexVecIndex != JE_TEXVEC_ARRAY_NULL_INDEX)
			jeTexVec_ArrayRemoveTexVec(BSP->TexVecArray, &Side->TexVecIndex);
	}

	jeRam_Free(*Brush);

	*Brush = NULL;
}

//=======================================================================================
//	jeBSP_TopBrushDestroyList
//=======================================================================================
void jeBSP_TopBrushDestroyList(jeBSP_TopBrush **Brushes, jeBSP *BSP)
{
	jeBSP_TopBrush	*Brush, *Next;

	for (Brush = *Brushes ; Brush ; Brush = Next)
	{
		Next = Brush->Next;

		jeBSP_TopBrushDestroy(&Brush, BSP);
	}
	
	*Brushes = NULL;		
}

#define SHEET_BRUSH_THICKNESS	5.0f

//=======================================================================================
//	jeBSP_TopBrushCreateFromBrushSheet
//	Create a special TopBrush that is a sheet
//=======================================================================================
static jeBSP_TopBrush *jeBSP_TopBrushCreateFromBrushSheet(jeBrush *Brush, jeBSP *BSP, uint32 Order)
{
	jeBrush_Face		*Face;
	jeBSP_TopSide		*Side;
	int32				i, v, NumFaces, NumVerts;
	jeVec3d				Tri[3];
	jePlane				Plane;
	jeBSP_TopBrush		*TopBrush;
	jeBrush_Contents	Contents;
//	jeFaceInfo	      FaceInfo;

	NumFaces = jeBrush_GetFaceCount(Brush);

	if (!NumFaces)
		return NULL;

	assert(NumFaces == 1);	// Only one face in a sheet brush

	Contents = jeBrush_GetContents(Brush);

	Face = jeBrush_GetNextFace(Brush, NULL);		// Get the sheet face
	assert(Face);
	
	NumVerts = jeBrush_FaceGetVertCount(Face);
	assert(NumVerts >= 3);

	// Create the top brush
	NumFaces = NumVerts+2;	// Make enough faces to extrude the sheet (2 faces + poly edges)

	TopBrush = jeBSP_TopBrushCreate(NumFaces, Order);

	if (!TopBrush)
		return NULL;

	TopBrush->Original = Brush;

	//
	// Create the first side
	//
	Side = TopBrush->TopSides;

	// Create a plane from the polys verts
	for (v=0; v< 3; v++)
	{
		Tri[v] = jeBrush_FaceGetWorldSpaceVertByIndex(Face, v);
	}

	// Create the plane
	jePlane_SetFromVerts(&Plane, &Tri[0], &Tri[1], &Tri[2]);

	Side->PlaneIndex = jePlaneArray_SharePlane(BSP->PlaneArray, &Plane);
	assert(Side->PlaneIndex != JE_PLANEARRAY_NULL_INDEX);

	if (Side->PlaneIndex == JE_PLANEARRAY_NULL_INDEX)
		return NULL;

	// Assign the faceinfo index
	if (!jeBSP_TopBrushSideCalcFaceInfo(TopBrush, BSP, Side, Face))
		return NULL;

	// This side is the actual sheet, so it's visible
	Side->Flags = SIDE_VISIBLE;

	Side->Flags |= SIDE_SHEET;	

	// KROUER: try to change the flag for the VIS Bug
    //jeBrush_FaceGetFaceInfo(Face, &FaceInfo);
   	//if (FaceInfo.Flags & FACEINFO_VIS_PORTAL)
   	{
		Side->Flags |= (SIDE_VIS_PORTAL|SIDE_HINT);
		//Side->Flags |= SIDE_VIS_PORTAL;
		//Side->Flags |= SIDE_HINT;
   	}

	Side++;

	//
	// Extrude and flip the sheet to the other side
	//
	for (v=0; v< 3; v++)
		jeVec3d_AddScaled(&Tri[v], &Plane.Normal, -SHEET_BRUSH_THICKNESS, &Tri[v]);

	// Create a reversed plane
	jePlane_SetFromVerts(&Plane, &Tri[0], &Tri[1], &Tri[2]);
	jePlane_Inverse(&Plane);

	Side->PlaneIndex = jePlaneArray_SharePlane(BSP->PlaneArray, &Plane);
	assert(Side->PlaneIndex != JE_PLANEARRAY_NULL_INDEX);
	Side->Flags = 0;		// Side is not visible
	Side->FaceInfoIndex = JE_FACEINFO_ARRAY_NULL_INDEX;
	Side++;

	// Create the brush sides from the poly edges
	for (i=0; i< NumVerts; i++, Side++)
	{
		int32				i2;
		jeVec3d				Vert1, Vert2, Vec;
		jePlane				EdgePlane;
		jeFloat				Length;

		i2 = (i+1 < NumVerts) ? (i+1): 0;

		Vert1 = jeBrush_FaceGetWorldSpaceVertByIndex(Face, i);
		Vert2 = jeBrush_FaceGetWorldSpaceVertByIndex(Face, i2);

		jeVec3d_Subtract(&Vert1, &Vert2, &Vec);
		Length = jeVec3d_Normalize(&Vec);

		assert(Length > 0.1);

		memset(&EdgePlane, 0, sizeof(jePlane));
		jeVec3d_CrossProduct(&Plane.Normal, &Vec, &EdgePlane.Normal);
		EdgePlane.Dist = jeVec3d_DotProduct(&Vert1, &EdgePlane.Normal);
		
		Side->PlaneIndex = jePlaneArray_SharePlane(BSP->PlaneArray, &EdgePlane);
		assert(Side->PlaneIndex != JE_PLANEARRAY_NULL_INDEX);
		Side->Flags = 0;		// Side is not visible
		Side->FaceInfoIndex = JE_FACEINFO_ARRAY_NULL_INDEX;
	}

	// Just in case they used other contents, just force it to be sheet contents.
	//	The only requirement is that they set it as sheet on the brush
	TopBrush->Contents = JE_BSP_CONTENTS_SHEET;

	assert(TopBrush->Contents & JE_BSP_CONTENTS_SHEET);

	#pragma message ("Need a jeBrush_CreateRef here...")

	return TopBrush;
}

//=======================================================================================
//	jeBSP_TopBrushCreateFromBrush
//	This function will return a list of convex jeBSP_TopBrushes, if jeBrush is NOT convex...
//	It is ok that a list is returned, because jeBSP_TopBrush is ONLY used by jeBSP
//	module and friends.  It is NOT publicized that there is a next pointer in the jeBSP_TopBrush...
//=======================================================================================
jeBSP_TopBrush *jeBSP_TopBrushCreateFromBrush(jeBrush *Brush, jeBSP *BSP, uint32 Order)
{
	jeBSP_TopBrush	*TopBrush;
	jeBSP_TopSide	*Side;
	int32			NumFaces, NeededFaces;
	jeBrush_Face	*Face;

	assert(Brush);

	NumFaces = jeBrush_GetFaceCount(Brush);

	if (!NumFaces)
		return NULL;

	// If this is a sheet brush, send it down a different pipeline
	if (jeBrush_GetContents(Brush) & JE_BSP_CONTENTS_SHEET)
		return jeBSP_TopBrushCreateFromBrushSheet(Brush, BSP, Order);

	// Count how many faces we'll need to create this brush
	Face = NULL;
	NeededFaces = 0;
	while (Face = jeBrush_GetNextFace(Brush, Face))
	{
		NeededFaces += jeBrush_FaceGetVertCount(Face) - 2;
	}

	TopBrush = jeBSP_TopBrushCreate(NeededFaces, Order);

	if (!TopBrush)
		return NULL;

	TopBrush->Original = Brush;

	Face = NULL;
	Side = TopBrush->TopSides;

	// Reset number of TopSides, so we can increment it as we actually create them, so it is accurate
	TopBrush->NumSides = 0;

	// Copy the polys over
	while (Face = jeBrush_GetNextFace(Brush, Face))
	{
		int32				NumVerts, f;
		const jeFaceInfo	*pFaceInfo;

		NumVerts = jeBrush_FaceGetVertCount(Face);
		assert(NumVerts >= 3);

		// Triangulate the face out just in case it's not planar
		for (f = 0; f< NumVerts-2; f++)
		{
			int32				j;
			jePlane				Plane;
			jeVec3d				Tri[3];

			// Create a plane from the current tri we are on in the face
			Tri[0] = jeBrush_FaceGetWorldSpaceVertByIndex(Face, 0);
			Tri[1] = jeBrush_FaceGetWorldSpaceVertByIndex(Face, f+1);
			Tri[2] = jeBrush_FaceGetWorldSpaceVertByIndex(Face, f+2);

			// Create the plane
			jePlane_SetFromVerts(&Plane, &Tri[0], &Tri[1], &Tri[2]);

			// Find the plane index
			Side->PlaneIndex = jePlaneArray_SharePlane(BSP->PlaneArray, &Plane);
			assert(Side->PlaneIndex != JE_PLANEARRAY_NULL_INDEX);

			if (Side->PlaneIndex == JE_PLANEARRAY_NULL_INDEX)
				return NULL;

			// Check to see if the plane already exist in the brush, if so, remove it from the array, and do not
			//	create this face on the top brush...
			for (j=0; j< TopBrush->NumSides; j++)
			{
				if (jePlaneArray_IndexIsCoplanar(Side->PlaneIndex, TopBrush->TopSides[j].PlaneIndex))
				{
					// Remove this plane
					jePlaneArray_RemovePlane(BSP->PlaneArray, &Side->PlaneIndex);
					break;
				}
			}

			if (j != TopBrush->NumSides)
				continue;	// Plane already in list

			if (!jeBSP_TopBrushSideCalcFaceInfo(TopBrush, BSP, Side, Face))
				return NULL;

			pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(BSP->FaceInfoArray, Side->FaceInfoIndex);
			assert(pFaceInfo);

			// All TopSides default to being visible 
			Side->Flags = SIDE_VISIBLE;

			// KROUER: try to change the flag for the VIS Bug
         	if (pFaceInfo->Flags & FACEINFO_VIS_PORTAL) 
        	{
				Side->Flags |= (SIDE_VIS_PORTAL|SIDE_HINT);
				//Side->Flags |= SIDE_VIS_PORTAL;
            	//Side->Flags |= SIDE_HINT;
         	}
			if (pFaceInfo->PortalCamera)
				Side->Flags |= SIDE_HINT;

			Side++;
			TopBrush->NumSides++;

			assert(TopBrush->NumSides <= NeededFaces);
		}
	}

	// Get the contents from the editor Brush
	TopBrush->Contents = jeBrush_GetContents(Brush);

	#pragma message ("Need a jeBrush_CreateRef here...")

	return TopBrush;
}

//=======================================================================================
//	jeBSP_TopBrushCreateListFromBrushChain
//=======================================================================================
jeBSP_TopBrush *jeBSP_TopBrushCreateListFromBrushChain(jeChain *BrushChain, jeBSP *BSP, uint32 *Order)
{
	jeBSP_TopBrush	*TopBrushList;
	int32			BadBrushes;
	jeBrush			*Brush;

	assert(BrushChain);

	BadBrushes = 0;
	TopBrushList = NULL;

	Brush = NULL;

	// Create the BSPBrushList
	while (Brush = (jeBrush*)jeChain_GetNextLinkData(BrushChain, Brush))
	{
		jeBSP_TopBrush	*TopBrush;

		TopBrush = jeBSP_TopBrushCreateFromBrush(Brush, BSP, (*Order));
		
		if (!TopBrush)
		{
			BadBrushes++;
			continue;
		}

		assert(TopBrush->Next == NULL);	// Only handle one brush at a time for now...

		// Add the BSPBrush to the beginning of the list
		TopBrush->Next = TopBrushList;
		TopBrushList = TopBrush;

		(*Order)++;
	}

	return TopBrushList;
}

//=======================================================================================
//	jeBSP_TopBrushSideCalcFaceInfo
//=======================================================================================
jeBoolean jeBSP_TopBrushSideCalcFaceInfo(jeBSP_TopBrush *TopBrush, jeBSP *BSP, jeBSP_TopSide *Side, const jeBrush_Face *jeFace)
{
	jePlane				Plane;
	const jeFaceInfo	*pFaceInfo;
	
	assert(BSP->FaceInfoArray);
	assert(BSP->TexVecArray);
	assert(BSP->PlaneArray);
	
	// Remove any old FaceInfo/TexVec index data from the arrays
	if (Side->FaceInfoIndex != JE_FACEINFO_ARRAY_NULL_INDEX)
		jeFaceInfo_ArrayRemoveFaceInfo(BSP->FaceInfoArray, &Side->FaceInfoIndex);
	
	if (Side->TexVecIndex != JE_TEXVEC_ARRAY_NULL_INDEX)
		jeTexVec_ArrayRemoveTexVec(BSP->TexVecArray, &Side->TexVecIndex);
	
#pragma message ("Need jeBrush_FaceCreateRef here...")
	Side->jeBrushFace = (jeBrush_Face*)jeFace;
	
	assert(Side->PlaneIndex != JE_PLANEARRAY_NULL_INDEX);
	
	Plane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Side->PlaneIndex);
	
	// inverse the plane
	if (jePlaneArray_IndexSided(Side->PlaneIndex)) {
		jePlane_Inverse(&Plane);
	}
	
	// compute all face UV vertex coords 
	{
		jeVec3d					LockedTranslation;
		const jeXForm3d			*XForm, *WorldToLocked, *LockedToWorld;
		jeTexVec				TexVec1, TexVec2;
		jeFaceInfo				FaceInfo;
		jeFaceInfo_ArrayIndex	FaceInfoIndex;
		jeFloat					AddU, AddV, Len1, Len2;
		
		FaceInfoIndex = jeBrush_FaceGetFaceInfoIndex((jeBrush_Face*)jeFace);
		
		// Get the faceinfo so we can access it
		pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(BSP->FaceInfoArray, FaceInfoIndex);
		assert(pFaceInfo);
		
		FaceInfo = *pFaceInfo;
		
		// Get the ModelToWorld, WorldToLocked, and LockedToWorld XForms
		XForm = jeBrush_GetXForm(TopBrush->Original);
		assert(XForm);
		WorldToLocked = jeBrush_GetWorldToLockedXForm(TopBrush->Original);
		assert(WorldToLocked);
		LockedToWorld = jeBrush_GetLockedToWorldXForm(TopBrush->Original);
		assert(LockedToWorld);
		
		// Put the normal into locked space
		jeXForm3d_Rotate(WorldToLocked, &Plane.Normal, &Plane.Normal);
		jeVec3d_Normalize(&Plane.Normal);
		
		// Get the locked texture vectors from the locked normal
		jePlane_GetAAVectors(&Plane, &TexVec1.VecU, &TexVec1.VecV);
		
		// Scale and rotate the texture vectors
		assert(FaceInfo.DrawScaleU);
		assert(FaceInfo.DrawScaleV);
		
		assert(FaceInfo.LMapScaleU);
		assert(FaceInfo.LMapScaleV);
		
		jeVec3d_Scale(&TexVec1.VecU, 1.0f/FaceInfo.LMapScaleU, &TexVec1.VecU);
		jeVec3d_Scale(&TexVec1.VecV, 1.0f/FaceInfo.LMapScaleV, &TexVec1.VecV);
		
		// Rotate the texture vectors
		{
			jeVec3d			Axis;
			jeXForm3d		RotXForm;
			jeQuaternion	Quat;
			
			jeVec3d_CrossProduct(&TexVec1.VecU, &TexVec1.VecV, &Axis);
			
			jeVec3d_Normalize(&Axis);
			
			jeQuaternion_SetFromAxisAngle(&Quat, &Axis, (FaceInfo.Rotate/180.0f)*JE_PI);
			jeQuaternion_ToMatrix(&Quat, &RotXForm);
			
			jeXForm3d_Transform(&RotXForm, &TexVec1.VecU, &TexVec1.VecU);
			jeXForm3d_Transform(&RotXForm, &TexVec1.VecV, &TexVec1.VecV);
		}
		
		
		// Rotate the locked texture vectors into world space
		jeXForm3d_Rotate(LockedToWorld, &TexVec1.VecU, &TexVec2.VecU);
		jeXForm3d_Rotate(LockedToWorld, &TexVec1.VecV, &TexVec2.VecV);
		
		// Inverse the scaling in the rotation
		//	This is done by first setting the length to the original locked length,
		//	then scaling this length by the ratio of the old length over the new length
		Len1 = jeVec3d_Length(&TexVec1.VecU);
		Len2 = jeVec3d_Length(&TexVec2.VecU);
		jeVec3d_Normalize(&TexVec2.VecU);
		jeVec3d_Scale(&TexVec2.VecU, Len1, &TexVec2.VecU);
		jeVec3d_Scale(&TexVec2.VecU, Len1/Len2, &TexVec2.VecU);
		
		Len1 = jeVec3d_Length(&TexVec1.VecV);
		Len2 = jeVec3d_Length(&TexVec2.VecV);
		jeVec3d_Normalize(&TexVec2.VecV);
		jeVec3d_Scale(&TexVec2.VecV, Len1, &TexVec2.VecV);
		jeVec3d_Scale(&TexVec2.VecV, Len1/Len2, &TexVec2.VecV);
		
		// Get the locked translation
		jeXForm3d_Transform(WorldToLocked, &XForm->Translation, &LockedTranslation);
		
		// Get the difference in ShiftU/ShiftV, so we can correct for it
		//	We do this by projecting the translation of the face onto the corresponding new and locked texture vectors
		//	Then we take the difference...
		AddU = jeVec3d_DotProduct(&TexVec2.VecU, &XForm->Translation) - jeVec3d_DotProduct(&TexVec1.VecU, &LockedTranslation);
		AddV = jeVec3d_DotProduct(&TexVec2.VecV, &XForm->Translation) - jeVec3d_DotProduct(&TexVec1.VecV, &LockedTranslation);
		
		// Add it in 
		//	We must interpret the Shifts the same way the drivers do. 
		//	So we must scale the shift the same way the driver will scale, etc...
		FaceInfo.ShiftU -= AddU/(FaceInfo.DrawScaleU/FaceInfo.LMapScaleU);
		FaceInfo.ShiftV -= AddV/(FaceInfo.DrawScaleV/FaceInfo.LMapScaleV);
		
		// Obtain the FaceInfoIndex for this side, by re-sharing the FaceInfo into the array
		//	If nothing changed, then we should get the same faceinfo, otherwise, it will try to share with something else...
		Side->FaceInfoIndex = jeFaceInfo_ArrayShareFaceInfo(BSP->FaceInfoArray, &FaceInfo);
		
		if (Side->FaceInfoIndex == JE_FACEINFO_ARRAY_NULL_INDEX)
			return JE_FALSE;
		
		// Get the texvec index by sharing the texture vectors into the texvec array
		Side->TexVecIndex = jeTexVec_ArrayShareTexVec(BSP->TexVecArray, &TexVec2);
		
		if (Side->TexVecIndex == JE_TEXVEC_ARRAY_NULL_INDEX)
			return JE_FALSE;
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_TopBrushCullList
//=======================================================================================
jeBSP_TopBrush *jeBSP_TopBrushCullList(jeBSP_TopBrush *List, jeBSP *BSP, jeBSP_TopBrush *Skip1)
{
	jeBSP_TopBrush	*NewList;
	jeBSP_TopBrush	*Next;

	NewList = NULL;

	for ( ; List ; List = Next)
	{
		Next = List->Next;

		if (List == Skip1)
		{
			jeBSP_TopBrushDestroy(&List, BSP);
			continue;
		}

		List->Next = NewList;
		NewList = List;
	}

	return NewList;
}
