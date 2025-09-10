/****************************************************************************************/
/*  JEBSPNODE_FACE.C                                                                    */
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
#include <Assert.h>
#include <Math.h>

#include "jeBSP._h"

#include "Errorlog.h"
#include "Ram.h"

static int32 g_ActiveFaces;
static int32 g_PeekFaces;

//=======================================================================================
//	jeBSPNode_FaceCreate
//=======================================================================================
jeBSPNode_Face *jeBSPNode_FaceCreate(void)
{
	jeBSPNode_Face		*Face;

	Face = JE_RAM_ALLOCATE_STRUCT(jeBSPNode_Face);

	if (!Face)
		return NULL;

	ZeroMem(Face);

	g_ActiveFaces++;

	if (g_ActiveFaces > g_PeekFaces)
		g_PeekFaces = g_ActiveFaces;

	return Face;
}

//=======================================================================================
//	jeBSPNode_FaceCreateFromPortal
//=======================================================================================
jeBSPNode_Face *jeBSPNode_FaceCreateFromPortal(jeBSPNode_Portal *p, int32 s)
{
	jeBSPNode_Face	*Face;
	jeBSP_TopSide	*Side;
	
	assert(p);

	Side = p->Side;
	
	if (!Side)
		return NULL;	// Portal does not bridge different visible contents

	Face = jeBSPNode_FaceCreate();

	if (!Face)
		return NULL;

	assert(Side->FaceInfoIndex != JE_FACEINFO_ARRAY_NULL_INDEX);
	Face->PlaneIndex = jePlaneArray_IndexGetPositive(Side->PlaneIndex);
	Face->Portal = p;

	Face->TopSideFlags = Side->TopSideFlags;

	if (s)
	{
		Face->PlaneIndex = jePlaneArray_IndexReverse(Face->PlaneIndex);
		Face->Poly = jePoly_CreateFromPoly(p->Poly, JE_TRUE);
	}
	else
	{
		Face->Poly = jePoly_CreateFromPoly(p->Poly, JE_FALSE);
	}

	if (!Face->Poly)
	{
		jeRam_Free(Face);
		return NULL;
	}

	return Face;
}

//=======================================================================================
//	jeBSPNode_FaceDestroy
//=======================================================================================
void jeBSPNode_FaceDestroy(jeBSPNode_Face **Face)
{
	assert(Face);
	assert(*Face);

	if ((*Face)->Poly)
		jePoly_Destroy(&(*Face)->Poly);

	jeRam_Free(*Face);

	g_ActiveFaces--;

	*Face = NULL;
}

//=======================================================================================
//	jeBSPNode_FaceGetActiveCount
//=======================================================================================
int32 jeBSPNode_FaceGetActiveCount(void)
{
	return g_ActiveFaces;
}

//=======================================================================================
//	jeBSPNode_FaceGetPeekCount
//=======================================================================================
int32 jeBSPNode_FaceGetPeekCount(void)
{
	return g_PeekFaces;
}

//=======================================================================================
//	jeBSPNode_FaceMerge
//=======================================================================================
jeBoolean jeBSPNode_FaceMerge(jeBSPNode_Face *Face1, jeBSPNode_Face *Face2, jeBSP *BSP, jeBSPNode_Face **Out)
{
	jePoly			*NewPoly;
	jeBSPNode_Face	*NewFace;
	const jePlane	*pPlane;
	jeVec3d			Normal;

	assert(Face1);
	assert(Face2);
	assert(Face1->Portal);
	assert(Face1->Portal->Side);
	assert(Face2->Portal);
	assert(Face2->Portal->Side);
	assert(Out);
	assert(Face1 != *Out);
	assert(Face2 != *Out);

	*Out = NULL;

	if (!jePlaneArray_IndexIsCoplanarAndFacing(Face1->PlaneIndex, Face2->PlaneIndex))
		return JE_TRUE;			// Planes don't match

	if ( Face1->Contents != Face2->Contents )
		return JE_TRUE;			// Don't merge faces across different contents

	if (Face1->Portal->Side->FaceInfoIndex != Face2->Portal->Side->FaceInfoIndex)
		return JE_TRUE;			// Different faceinfo

	if (Face1->TopSideFlags != Face2->TopSideFlags)
		return JE_TRUE;			// Different TopSideFlags

	#if 1
	//if (!(g_Options & JE_BSP_OPTIONS_MERJE_ACROSS_MULTIPLE_BRUSH_FACES))
	{
		// Don't merge across multiple brush faces
		if (Face1->Portal->Side->jeBrushFace != Face2->Portal->Side->jeBrushFace)
			return JE_TRUE;
	}
	#endif

	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Face1->PlaneIndex);

	Normal = pPlane->Normal;

	if (jePlaneArray_IndexSided(Face1->PlaneIndex))
		jeVec3d_Inverse(&Normal);

	if (!jePoly_Merge(Face1->Poly, Face2->Poly, &Normal, &NewPoly))
	{
		jeErrorLog_AddString(-1, "jeBSPNode_FaceMerge:  jePoly_Merge failed.", NULL);
		return JE_FALSE;		 
	}

	if (!NewPoly)
		return JE_TRUE;		// Can't merge

	NewFace = jeBSPNode_FaceCreate();

	if (!NewFace)
	{
		jePoly_Destroy(&NewPoly);
		return JE_FALSE;
	}

	*NewFace = *Face1;		// Copy face

	NewFace->Flags = 0;

	// Mark this face as a face in the chain, so we can find the originals
	//	(Original faces won't have the BSPFACE_MERGED_SPLIT flag)
	NewFace->Flags |= BSPFACE_MERGED_SPLIT;	

	NewFace->Poly = NewPoly;

	// Let face1, and face2 know who they merged with
	Face1->Merged = NewFace;
	Face2->Merged = NewFace;

	if (Face1->Portal->Side->jeBrushFace != Face2->Portal->Side->jeBrushFace)
		NewFace->Flags |= BSPFACE_SPAN_MULTIPLE_BRUSH_FACES;

	*Out = NewFace;			// This is the new face

	return JE_TRUE;
}

#define DEGENERATE_EPSILON		0.001f

//====================================================================================
//	jeBSPNode_FaceIsValidGeometry
//====================================================================================
jeBoolean jeBSPNode_FaceIsValidGeometry(const jeBSPNode_Face *Face, jeBSP *BSP)
{
	int32		i, j, NumVerts;
	jeVec3d		Vect1, *Verts, *V1, *V2, EdgeNormal;
	jePoly		*Poly;
	jeFloat		Dist, EdgeDist;
	jePlane		Plane;
	
	Poly = Face->Poly;
	Verts = Poly->Verts;
	NumVerts = Poly->NumVerts;

	if (NumVerts < 3)
		return JE_FALSE;
	
	Plane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Face->PlaneIndex);

	if (jePlaneArray_IndexSided(Face->PlaneIndex))
		jePlane_Inverse(&Plane);

	//	Check for degenerate edges, convexity, and make sure it's planar
	for (i=0; i< NumVerts; i++)
	{
		V1 = &Verts[i];
		V2 = &Verts[(i+1)%NumVerts];

		//	Check for degenreate edge
		jeVec3d_Subtract(V2, V1, &Vect1);
		Dist = jeVec3d_Length(&Vect1);
	
		if (fabs(Dist) < DEGENERATE_EPSILON)
			return JE_FALSE;

		// Check for planar
		Dist = jeVec3d_DotProduct(V1, &Plane.Normal) - Plane.Dist;

		if (Dist > JE_BSP_ON_EPSILON || Dist <-JE_BSP_ON_EPSILON)
			return JE_FALSE;

		jeVec3d_CrossProduct(&Plane.Normal, &Vect1, &EdgeNormal);
		jeVec3d_Normalize(&EdgeNormal);
		EdgeDist = jeVec3d_DotProduct(V1, &EdgeNormal);
		
		// Check for convexity
		for (j=0; j< NumVerts; j++)
		{
			Dist = jeVec3d_DotProduct(&Verts[j], &EdgeNormal) - EdgeDist;
			
			if (Dist > JE_BSP_ON_EPSILON)
				return JE_FALSE;
		}
	}

	return JE_TRUE;
}

//====================================================================================
//	jeBSPNode_FaceMergeList
//====================================================================================
jeBoolean jeBSPNode_FaceMergeList(jeBSPNode_Face *Faces, jeBSP *BSP, int32 *NumMerged)
{
	jeBSPNode_Face	*Face1, *Face2, *End, *Merged;
	
	assert(Faces);
	assert(NumMerged);

	(*NumMerged) = 0;

	for (Face1 = Faces ; Face1 ; Face1 = Face1->Next)
	{
		if (Face1->Merged || Face1->Split[0] || Face1->Split[1])
			continue;

		for (Face2 = Faces ; Face2 != Face1 ; Face2 = Face2->Next)
		{
			if (Face2->Merged || Face2->Split[0] || Face2->Split[1])
				continue;
			
			Merged = NULL;
			if (!jeBSPNode_FaceMerge(Face1, Face2, BSP, &Merged))
				return JE_FALSE;

			if (!Merged)
				continue;

			if (!jePoly_RemoveDegenerateEdges(Merged->Poly, 0.001f))
				return JE_FALSE;
			
			if (!jeBSPNode_FaceIsValidGeometry(Merged, BSP))
			{
				jeBSPNode_FaceDestroy(&Merged);
				Face1->Merged = NULL;		// Cancel out merge
				Face2->Merged = NULL;
				continue;
			}

			(*NumMerged)++;

			// Add the Merged to the end of the face list 
			// so it will be checked against all the faces again
			for (End = Faces ; End->Next ; End = End->Next);
				
			Merged->Next = NULL;
			End->Next = Merged;
			break;
		}
	}

	return JE_TRUE;
}

//====================================================================================
//	NewFaceFromFaceForSplit
//====================================================================================
static jeBSPNode_Face *NewFaceFromFaceForSplit(jeBSPNode_Face *f)
{
	jeBSPNode_Face	*Newf;

	Newf = jeBSPNode_FaceCreate();

	if (!Newf)
		return NULL;

	*Newf = *f;
	
	// Clear this faces split pointer/poly
	Newf->Split[0] = Newf->Split[1] = NULL;
	Newf->Poly = NULL;

	// Make sure that we remember that this is NOT an original face.
	// Mark it as a face in the chain, so we can destroy it once the drawfaces
	// are created from it!!!!!
	Newf->Flags |= BSPFACE_MERGED_SPLIT;
	
	return Newf;
}

//====================================================================================
//	jeBSPNode_FaceSubdivide
//====================================================================================
jeBoolean jeBSPNode_FaceSubdivide(jeBSPNode_Face *Face, jeBSP *BSP, jeBSPNode *Node, jeFloat SubdivideSize, int32 *NumSubdivided)
{
	jeFloat				Mins, Maxs, v;
	int32				Axis, i;
	jeVec3d				Temp;
	jeFloat				Dist;
	jePoly				*p, *Frontp, *Backp;
	jePlane				Plane;
	const jeFaceInfo	*pFaceInfo;
	const jeTexVec		*pTexVec;

	assert(Face);
	assert(Node);
	assert(Face->Portal);
	assert(Face->Portal->Side);

	if (Face->Merged)
		return JE_TRUE;
	
	pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(BSP->FaceInfoArray, Face->Portal->Side->FaceInfoIndex);
	assert(pFaceInfo);

	// Faces that don't have lightmaps DON"T need to be subdivided
	if (!jeFaceInfo_NeedsLightmap(pFaceInfo))
		return JE_TRUE;

	pTexVec = jeTexVec_ArrayGetTexVecByIndex(BSP->TexVecArray, Face->Portal->Side->TexVecIndex);
	assert(pTexVec);

	for (Axis = 0 ; Axis < 2 ; Axis++)
	{
		while (1)
		{
			Mins = 999999.0f;
			Maxs = -999999.0f;
			
			if (!Axis)
				jeVec3d_Copy(&pTexVec->VecU, &Temp);
			else
				jeVec3d_Copy(&pTexVec->VecV, &Temp);
			
			for (i=0 ; i<Face->Poly->NumVerts ; i++)
			{
				v = jeVec3d_DotProduct(&Face->Poly->Verts[i], &Temp);
				if (v < Mins)
					Mins = v;
				if (v > Maxs)
					Maxs = v;
			}
			
			if (Maxs - Mins <= SubdivideSize)
				break;
			
			// Split it
			(*NumSubdivided)++;
			
			v = jeVec3d_Normalize(&Temp);	

			Dist = (Mins + SubdivideSize - 16.0f)/v;

			Plane.Normal = Temp;
			Plane.Dist = Dist;
			Plane.Type = Type_Any;
			
			p = jePoly_CreateFromPoly(Face->Poly, JE_FALSE);

			if (!p)
				return JE_FALSE;

			if (!jePoly_SplitEpsilon(&p, 0.0f, &Plane, JE_FALSE, &Frontp, &Backp))
				return JE_FALSE;

			assert(Frontp || Backp);

			Face->Split[0] = NewFaceFromFaceForSplit(Face);
			Face->Split[0]->Poly = Frontp;
			Face->Split[0]->Next = Node->Faces;
			Node->Faces = Face->Split[0];

			Face->Split[1] = NewFaceFromFaceForSplit(Face);
			Face->Split[1]->Poly = Backp;
			Face->Split[1]->Next = Node->Faces;
			Node->Faces = Face->Split[1];

			if (!jeBSPNode_FaceSubdivide(Face->Split[0], BSP, Node, SubdivideSize, NumSubdivided))
				return JE_FALSE;

			if (!jeBSPNode_FaceSubdivide(Face->Split[1], BSP, Node, SubdivideSize, NumSubdivided))
				return JE_FALSE;

			return JE_TRUE;
		}
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_FaceCullList
//=======================================================================================
jeBSPNode_Face *jeBSPNode_FaceCullList(jeBSPNode_Face *List, jeBSPNode_Face **Skip1)
{
	jeBSPNode_Face	*NewList;
	jeBSPNode_Face	*Next;

	NewList = NULL;

	for ( ; List ; List = Next)
	{
		Next = List->Next;

		if (List == *Skip1)
		{
			jeBSPNode_FaceDestroy(Skip1);
			continue;
		}

		List->Next = NewList;
		NewList = List;
	}

	return NewList;
}
