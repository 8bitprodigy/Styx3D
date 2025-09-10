/****************************************************************************************/
/*  JEBSP_BRUSH.C                                                                       */
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
#include <memory.h>		// memset

// Private dependents
#include "jeBSP._h"
#include "Errorlog.h"
#include "Ram.h"
#include "Vec3d.h"
#include "Log.h"

// Public dependents
#include "jeBSP.h"

#define BRUSH_SIZE(s) ((sizeof(jeBSP_Brush)-sizeof(jeBSP_Side[JE_BSP_BRUSH_DEFAULT_SIDES]))+(sizeof(jeBSP_Side)*(s)));

static int32	g_ActiveBrushes = 0;
static int32	g_PeekBrushes = 0;

//=======================================================================================
//	jeBSP_BrushCreate
//=======================================================================================
jeBSP_Brush *jeBSP_BrushCreate(int32 NumSides)
{
	jeBSP_Brush		*Brush;
	int32			c;
	int32			i;

	assert(NumSides < 65535);

	// Get the size needed to accomodate the number of sides
	c = BRUSH_SIZE(NumSides);

	// Allocate the brush
	Brush = (jeBSP_Brush*)jeRam_Allocate(c);

	if (!Brush)
		return NULL;

	memset (Brush, 0, c);

	Brush->NumSides = (uint16)NumSides;

	// Default all sides to NULL PlaneIndex
	for (i=0; i<NumSides; i++)
		Brush->Sides[i].PlaneIndex = JE_PLANEARRAY_NULL_INDEX;

	g_ActiveBrushes++;

	if (g_ActiveBrushes > g_PeekBrushes)
		g_PeekBrushes = g_ActiveBrushes;

	return Brush;
}

//=======================================================================================
//	jeBSP_BrushCreateFromBox
//=======================================================================================
jeBSP_Brush *jeBSP_BrushCreateFromBox(jeBSP *BSP, const jeExtBox *Box)
{
	jeBSP_Brush	*b;
	int32		i;
	jePlane		Plane;

	b = jeBSP_BrushCreate(6);

	for (i=0 ; i<3 ; i++)
	{
		jeVec3d_Clear(&Plane.Normal);
		jeVec3d_SetElement(&Plane.Normal, i, 1.0f);
		Plane.Dist = jeVec3d_GetElement(&((jeExtBox*)Box)->Max, i)+1.0f;
		b->Sides[i].PlaneIndex = jePlaneArray_SharePlane(BSP->PlaneArray, &Plane);

		jeVec3d_SetElement(&Plane.Normal, i, -1.0f);
		Plane.Dist = -(jeVec3d_GetElement(&((jeExtBox*)Box)->Min, i)-1.0f);
		b->Sides[i+3].PlaneIndex = jePlaneArray_SharePlane(BSP->PlaneArray, &Plane);
	}

	if (!jeBSP_BrushCreatePolys(b, BSP))
	{
		jeBSP_BrushDestroy(&b);
		return NULL;
	}

	return b;
}

//=======================================================================================
//	jeBSP_BrushDestroy
//=======================================================================================
void jeBSP_BrushDestroy(jeBSP_Brush **Brush)
{
	jeBSP_Brush		*pBrush;
	int32			i;

	assert(Brush);

	pBrush = *Brush;

	assert(pBrush);

	// Free all the polys on the brush
	for (i=0 ; i<pBrush->NumSides ; i++)
	{
		if (pBrush->Sides[i].Poly)
			jePoly_Destroy(&pBrush->Sides[i].Poly);
	}

	jeRam_Free(pBrush);

	*Brush = NULL;

	g_ActiveBrushes--;
}

//=======================================================================================
//	jeBSP_BrushGetActiveCount
//=======================================================================================
int32 jeBSP_BrushGetActiveCount(void)
{
	return g_ActiveBrushes;
}

//=======================================================================================
//	jeBSP_BrushGetPeekCount
//=======================================================================================
int32 jeBSP_BrushGetPeekCount(void)
{
	return g_PeekBrushes;
}

//=======================================================================================
//	jeBSP_BrushDestroyList
//=======================================================================================
void jeBSP_BrushDestroyList(jeBSP_Brush **Brushes)
{
	jeBSP_Brush	*Brush, *Next;

	for (Brush = *Brushes ; Brush ; Brush = Next)
	{
		Next = Brush->Next;

		jeBSP_BrushDestroy(&Brush);
	}
	
	*Brushes = NULL;		
}

//=======================================================================================
//	jeBSP_BrushCullList
//=======================================================================================
jeBSP_Brush *jeBSP_BrushCullList(jeBSP_Brush *List, jeBSP_Brush *Skip1)
{
	jeBSP_Brush	*NewList;
	jeBSP_Brush	*Next;

	NewList = NULL;

	for ( ; List ; List = Next)
	{
		Next = List->Next;

		if (List == Skip1)
		{
			jeBSP_BrushDestroy(&List);
			continue;
		}

		List->Next = NewList;
		NewList = List;
	}

	return NewList;
}

//=======================================================================================
//	jeBSP_BrushCreateFromBSPBrush
//	Copys a BSPBrush
//=======================================================================================
jeBSP_Brush *jeBSP_BrushCreateFromBSPBrush(const jeBSP_Brush *Brush)
{
	jeBSP_Brush *NewBrush;
	int32		Size;
	int32		i;
	
	assert(Brush);
	assert(Brush->NumSides > 0);

	Size = BRUSH_SIZE(Brush->NumSides);

	NewBrush = jeBSP_BrushCreate(Brush->NumSides);

	if (!NewBrush)
		return NULL;

	// Copy entire brush over
	memcpy (NewBrush, Brush, Size);

	// Copy all the polys over
	for (i=0 ; i<Brush->NumSides ; i++)
	{
		if (!Brush->Sides[i].Poly)
			continue;

		assert(Brush->Sides[i].Poly->NumVerts > 0);
		assert(Brush->Sides[i].Poly->NumVerts < 255);
		
		NewBrush->Sides[i].Poly = jePoly_CreateFromPoly(Brush->Sides[i].Poly, JE_FALSE);
		
		if (!NewBrush->Sides[i].Poly)
			return NULL;
	}

	return NewBrush;
}

//=======================================================================================
//	jeBSP_SideSetFromTopSide
//=======================================================================================
void jeBSP_SideSetFromTopSide(jeBSP_Side *Side, const jeBSP_TopSide *TopSide)
{
	assert(Side);
	assert(TopSide);
	assert(TopSide->PlaneIndex != JE_PLANEARRAY_NULL_INDEX);

	Side->PlaneIndex = TopSide->PlaneIndex;
	//Side->FaceInfoIndex = TopSide->FaceInfoIndex;
	Side->Flags = TopSide->Flags;

	if (TopSide->Flags & SIDE_HINT)
		Side->Flags |= SIDE_VISIBLE;		// Hint allways visible
}

//=======================================================================================
//	jeBSP_BrushCreateFromTopBrush
//=======================================================================================
jeBSP_Brush *jeBSP_BrushCreateFromTopBrush(jeBSP_TopBrush *TopBrush, jeBSP *BSP)
{
	jeBSP_Brush		*BSPBrush;
	int32			i;

	assert(TopBrush);

	// Create a BSPBrush with the same number of sides as the top brush
	BSPBrush = jeBSP_BrushCreate(TopBrush->NumSides);

	if (!BSPBrush)
		return NULL;

	// Copy the sides over
	for (i=0; i< TopBrush->NumSides; i++)
	{
		jeBSP_TopSide	*TopSide;
		jeBSP_Side		*Side;

		TopSide = &TopBrush->TopSides[i];
		Side = &BSPBrush->Sides[i];

		// Copy the side
		jeBSP_SideSetFromTopSide(Side, TopSide);
	}

	// Create the polys out of the sides on the BSPBrush.  It should be a perfect skin of the brush...
	if (!jeBSP_BrushCreatePolys(BSPBrush, BSP))
		goto ExitWithError;

	BSPBrush->Original = TopBrush;		// Remember the top brush that created this brush

	return BSPBrush;

	// Error
	ExitWithError:
	{
		if (BSPBrush)
			jeBSP_BrushDestroy(&BSPBrush);

		return NULL;
	}
}

//=======================================================================================
//	jeBSP_BrushCreateListFromTopBrushList
//=======================================================================================
jeBSP_Brush *jeBSP_BrushCreateListFromTopBrushList(jeBSP_TopBrush *TopBrushList, jeBSP *BSP)
{
	jeBSP_TopBrush	*TopBrush;
	jeBSP_Brush		*BSPBrushList;

	BSPBrushList = NULL;

	for (TopBrush = TopBrushList; TopBrush; TopBrush = TopBrush->Next)
	{
		jeBSP_Brush		*BSPBrush;

		BSPBrush = jeBSP_BrushCreateFromTopBrush(TopBrush, BSP);

		if (!BSPBrush)
		{
			jeBSP_BrushDestroyList(&BSPBrushList);
			return NULL;
		}

		assert(BSPBrush->Next == NULL);

		BSPBrush->Next = BSPBrushList;
		BSPBrushList = BSPBrush;
	}

	return BSPBrushList;
}

//=======================================================================================
//	jeBSP_BrushCalcBounds
//=======================================================================================
jeBoolean jeBSP_BrushCalcBounds(jeBSP_Brush *BSPBrush)
{
	int32		i;
	jeBoolean	Set;

	Set = JE_FALSE;

	// Go through all the brush sides
	for (i=0; i< BSPBrush->NumSides; i++)
	{
		jePoly	*Poly;
		int32	v;

		Poly = BSPBrush->Sides[i].Poly;

		if (!Poly)
			continue;

		if (!Set)		// Take first valid point as box default
		{
			jeExtBox_SetToPoint(&BSPBrush->Box, &BSPBrush->Sides[i].Poly->Verts[0]);
			Set = JE_TRUE;
		}

		// Extend the box
		for (v=0; v< Poly->NumVerts; v++)
			jeExtBox_ExtendToEnclose(&BSPBrush->Box, &Poly->Verts[v]);
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_BrushIsValidGeometry
//=======================================================================================
jeBoolean jeBSP_BrushIsValidGeometry(jeBSP_Brush *BSPBrush)
{
	int32		j;

	if (BSPBrush->NumSides < 3)
	{
		jeErrorLog_AddString(-1, "BSPBrush NumSides < 3", NULL);
		return JE_FALSE;
	}

	for (j=0 ; j<3 ; j++)
	{
		if (jeVec3d_GetElement(&BSPBrush->Box.Min, j) >= jeVec3d_GetElement(&BSPBrush->Box.Max, j))
			return JE_FALSE;

		if (jeVec3d_GetElement(&BSPBrush->Box.Min, j) <= -JE_BSP_MINMAX_BOUNDS)
			return JE_FALSE;

		if (jeVec3d_GetElement(&BSPBrush->Box.Max, j) >= JE_BSP_MINMAX_BOUNDS)
			return JE_FALSE;
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeExtBox_Intersects
//=======================================================================================
static jeBoolean jeExtBox_Intersects(  const jeExtBox *B1,  const jeExtBox *B2 )
{
	assert ( jeExtBox_IsValid (B1) != JE_FALSE );
	assert ( jeExtBox_IsValid (B2) != JE_FALSE );

	if ((B1->Min.X >= B2->Max.X) || (B1->Max.X <= B2->Min.X)) return JE_FALSE;
	if ((B1->Min.Y >= B2->Max.Y) || (B1->Max.Y <= B2->Min.Y)) return JE_FALSE;
	if ((B1->Min.Z >= B2->Max.Z) || (B1->Max.Z <= B2->Min.Z)) return JE_FALSE;
	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_BrushDisjoint
//	Returns JE_TRUE is 2 brushes are NOT overlapping
//=======================================================================================
jeBoolean jeBSP_BrushDisjoint(const jeBSP_Brush *b1, const jeBSP_Brush *b2)
{
	int32		i, j;

	// Check bounding boxes
	if (!jeExtBox_Intersects(&b1->Box, &b2->Box))
		return JE_TRUE;		// Bounding boxes don't overlap
	
	// Check for opposing planes
	for (i=0 ; i<b1->NumSides ; i++)
	{
		jePlaneArray_Index	i1, i2;
		
		i1 = b1->Sides[i].PlaneIndex;
		
		for (j=0 ; j<b2->NumSides ; j++)
		{
			i2 = b2->Sides[j].PlaneIndex;

			if (jePlaneArray_IndexIsCoplanarAndNotFacing(i1, i2))
			{
				//Log_Printf("Disjoint by opposing planes...\n");
				return JE_TRUE;	// Opposite/Coplanar planes, so not overlapping
			}
		}
	}
	
	return JE_FALSE;	// Might intersect
}

//=======================================================================================
//	jeBSP_BrushCanBite
//=======================================================================================
jeBoolean jeBSP_BrushCanBite(const jeBSP_Brush *b1, const jeBSP_Brush *b2)
{
	jeBrush_Contents	c1, c2;
	jeBSP_TopBrush		*t1, *t2;

	t1 = b1->Original;
	t2 = b2->Original;

	if (t1->Order < t2->Order)
		return JE_FALSE;

	c1 = t1->Contents;
	c2 = t2->Contents;

#ifdef USE_DETAIL
	// Detail brushes never bite structural brushes
	if ( (c1 & JE_BSP_CONTENTS_DETAIL) && !(c2 & JE_BSP_CONTENTS_DETAIL) )
		return JE_FALSE;
#endif

	//if (c1 & BSP_CONTENTS_FLOCKING)
	//	return JE_FALSE;
	//if (c2 & JE_BSP_CONTENTS_FLOCK)		// Nothing cuts a flock brush (they override)
	//	return JE_FALSE;

	//if (c1 & JE_BSP_CONTENTS_SOLID)	
	//	return JE_TRUE;

	if (c1 == c2)
		return JE_TRUE;

	return JE_FALSE;
}

//=======================================================================================
//	jeBSP_BrushVolume
//=======================================================================================
float jeBSP_BrushVolume(jeBSP_Brush *Brush, jeBSP *BSP)
{
	int32		i;
	jePoly		*p;
	jeVec3d		Corner;
	float		d, Area, Volume;
	jeBSP_Side	*pSide;

	assert(Brush);

	if (!Brush)
		return 0.0f;

	// Grab the first valid point as the corner
	for (p=NULL, i=0 ; i<Brush->NumSides ; i++)
	{
		p = Brush->Sides[i].Poly;
		if (p)
			break;
	}

	if (!p)				// No polys on the brush
		return 0.0f;

	jeVec3d_Copy(&p->Verts[0], &Corner);

	// Make tetrahedrons to all other faces
	for (Volume = 0.0f, pSide = Brush->Sides; i<Brush->NumSides ; i++, pSide++)
	{
		const jePlane	*pPlane;

		p = pSide->Poly;

		if (!p)
			continue;

		pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, pSide->PlaneIndex);

		d = jePlane_PointDistanceFast(pPlane, &Corner);

		if (jePlaneArray_IndexSided(pSide->PlaneIndex))
			d = -d;

		Area = jePoly_Area(p);
		Volume += -d*Area;
	}

	Volume /= 3.0f;

	return Volume;
}

//=======================================================================================
//	jeBSP_BrushMostlyOnPlaneSide
//=======================================================================================
jePlane_Side jeBSP_BrushMostlyOnPlaneSide(const jeBSP_Brush *Brush, const jePlane *Plane)
{
	int32			i, j;
	jePoly			*p;
	float			d, Max;
	jePlane_Side	Side;

	Max = 0.0f;
	Side = PSIDE_FRONT;		// Default to front side

	for (i=0 ; i<Brush->NumSides ; i++)
	{
		p = Brush->Sides[i].Poly;

		if (!p)
			continue;

		for (j=0 ; j<p->NumVerts ; j++)
		{
			d = jeVec3d_DotProduct(&p->Verts[j], &Plane->Normal) - Plane->Dist;

			if (d > Max)
			{
				Max = d;
				Side = PSIDE_FRONT;
			}
			if (-d > Max)
			{
				Max = -d;
				Side = PSIDE_BACK;
			}
		}
	}

	return Side;
}

//=======================================================================================
//	jeBSP_BrushCountList
//=======================================================================================
int32 jeBSP_BrushCountList(const jeBSP_Brush *Brushes)
{
	int32	c;

	for (c=0 ; Brushes ; Brushes = Brushes->Next)
		c++;

	return c;
}

//=======================================================================================
//	jeBSP_BrushCreatePolys
//=======================================================================================
jeBoolean jeBSP_BrushCreatePolys(jeBSP_Brush *Brush, jeBSP *BSP)
{
	int32			i, j;

	for (i=0 ; i<Brush->NumSides ; i++)
	{
		jePoly			*p;
		jeBSP_Side		*Side;
		jePlane			Plane;
		const jePlane	*pPlane;

		Side = &Brush->Sides[i];

		if (Side->Poly)
			jePoly_Destroy(&Side->Poly);		// Destroy all old polys

		Plane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Side->PlaneIndex);
		Plane.Type = Type_Any;

		if (jePlaneArray_IndexSided(Side->PlaneIndex))
			jePlane_Inverse(&Plane);

		p = jePoly_CreateFromPlane(&Plane, JE_BSP_MINMAX_BOUNDS);

		if (!p)
			return JE_FALSE;

		for (j=0 ; j<Brush->NumSides && p; j++)
		{
			if (i == j)
				continue;

			pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Brush->Sides[j].PlaneIndex);

			if (!jePoly_ClipEpsilon(&p, 0.0f, pPlane, !jePlaneArray_IndexSided(Brush->Sides[j].PlaneIndex)))
				return JE_FALSE;
		}

		Side->Poly = p;
		
		if (!Side->Poly)					// If no poly on side, then make is invisible
			Side->Flags &= ~SIDE_VISIBLE;
	}

	// Calculate the bounds of the brush
	if (!jeBSP_BrushCalcBounds(Brush))
		return JE_FALSE;

	// Make sure it's valid
	if (!jeBSP_BrushIsValidGeometry(Brush))
	{
		jeErrorLog_AddString(-1, "jeBSP_BrushCreatePolys:  jeBSP_BrushIsValidGeometry failed.", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_BrushListAddToTail
//=======================================================================================
jeBSP_Brush *jeBSP_BrushListAddToTail(jeBSP_Brush *List, jeBSP_Brush *Tail)
{
	jeBSP_Brush	*Walk, *Next;

	for (Walk=List ; Walk ; Walk=Next)
	{	// Add to end of list
		Next = Walk->Next;
		Walk->Next = NULL;
		Tail->Next = Walk;
		Tail = Walk;
	}

	return Tail;
}

//=======================================================================================
//	jeBSP_BrushSplit
//	Splits brush into 2 brushes.  The input brush is NOT freed!  
//	Back and/or Front CAN be NULL if the brush was not split, or tiny...
//=======================================================================================
jeBoolean jeBSP_BrushSplit(jeBSP_Brush *Brush, jeBSP *BSP, jePlaneArray_Index Index, jeBSP_SideFlag MidFlags, jeBSP_Brush **Front, jeBSP_Brush **Back)
{
	jeBSP_Brush		*b[2];
	int32			i, j;
	jePoly			*p, *MidPoly;
	jePlane			Plane;
	const jePlane	*pPlane;
	jeBSP_Side		*cs, *pSide;
	float			d, FrontD, BackD;

	if (Brush->Flags & BSPBRUSH_FORCEBOTH)		// Force it down both sides
	{
		*Front = jeBSP_BrushCreateFromBSPBrush(Brush);
		*Back = jeBSP_BrushCreateFromBSPBrush(Brush);
		return JE_TRUE;
	}

	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Index);

	Plane = *pPlane;		// Copy the plane into something we can modify
	Plane.Type = Type_Any;

	if (jePlaneArray_IndexSided(Index))
		jePlane_Inverse(&Plane);

	*Front = *Back = NULL;

	// Check all points
	FrontD = BackD = 0.0f;

	for (i=0 ; i<Brush->NumSides ; i++)
	{
		jeVec3d	*pVert;

		p = Brush->Sides[i].Poly;

		if (!p)
			continue;

		for (pVert = p->Verts, j=0 ; j<p->NumVerts ; j++, pVert++)
		{
		#if 1
			d = jePlane_PointDistanceFast(pPlane, pVert);

			if (jePlaneArray_IndexSided(Index))
				d = -d;
		#else
			d = jeVec3d_DotProduct (pVert, &Plane.Normal) - Plane.Dist;
		#endif

			if (d > FrontD)
				FrontD = d;
			if (d < BackD)
				BackD = d;
		}
	}
	
	if (FrontD < 0.1f)			// Only on back
	{	
		*Back = jeBSP_BrushCreateFromBSPBrush(Brush);
		return JE_TRUE;
	}

	if (BackD > -0.1f)			// Only on front
	{	
		*Front = jeBSP_BrushCreateFromBSPBrush(Brush);
		return JE_TRUE;
	}

	// create a new poly from the split plane
	p = jePoly_CreateFromPlane(&Plane, JE_BSP_MINMAX_BOUNDS);

	if (!p)
		return JE_FALSE;
	
	// Clip the poly by all the planes of the brush being split
	for (pSide = Brush->Sides, i=0 ; i<Brush->NumSides && p ; i++, pSide++)
	{
		const jePlane		*pPlane2;

		pPlane2 = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, pSide->PlaneIndex);
		
		// Keep the back side
		if (!jePoly_ClipEpsilon(&p, 0.0f, pPlane2, !jePlaneArray_IndexSided(pSide->PlaneIndex)))
			return JE_FALSE;
	}

	if (!p || jePoly_IsTiny(p))
	{	
		// The brush isn't really split
		jePlane_Side		Side;

		//Log_Printf("Dropping poly...\n");

		Side = jeBSP_BrushMostlyOnPlaneSide(Brush, &Plane);
		
		if (Side == PSIDE_FRONT)
			*Front = jeBSP_BrushCreateFromBSPBrush(Brush);
		if (Side == PSIDE_BACK)
			*Back = jeBSP_BrushCreateFromBSPBrush(Brush);

		if (!(*Front) && !(*Back))
			return JE_FALSE;

		return JE_TRUE;
	}

	// Split it for real
	MidPoly = p;					// Store the mid poly

	// Create 2 brushes
	for (i=0 ; i<2 ; i++)
	{
		b[i] = jeBSP_BrushCreate(Brush->NumSides+1);
		
		if (!b[i])		
			return JE_FALSE;		// FIXME:  Free brushes

		// Reset sides, so we can fill it with exact number of sides, after split
		b[i]->NumSides = 0;	
		b[i]->Original = Brush->Original;		// Save original
	}

	// Split all the current polys of the brush being split, 
	// and distribute them to the other 2 brushes
	for (i=0 ; i<Brush->NumSides ; i++)
	{
		jeBSP_Side		*pDestSide, *pSrcSide;
		jePoly			*pPolys[2];

		pSrcSide = &Brush->Sides[i];
		
		if (!pSrcSide->Poly)	// Don't copy sides that don't have polys
			continue;

		p = jePoly_CreateFromPoly(pSrcSide->Poly, JE_FALSE);		// Copy the poly
		
		if (!p)
			return JE_FALSE;		// FIXME:  Free stuff

		// Split the poly on the split plane
		if (!jePoly_SplitEpsilon(&p, 0.0f, &Plane, JE_FALSE, &pPolys[0], &pPolys[1]))
			return JE_FALSE;

		// Distribute the polys to the split brushes
		for (j=0 ; j<2 ; j++)
		{
			if (!pPolys[j])
				continue;
		#if 1
			if (jePoly_IsTiny(pPolys[j]))
			{
				jePoly_Destroy(&pPolys[j]);
				continue;
			}
		#endif
			
			pDestSide = &b[j]->Sides[b[j]->NumSides];
			b[j]->NumSides++;
			
			*pDestSide = *pSrcSide;					// Copy the side
			
			pDestSide->Poly = pPolys[j];
			pDestSide->Flags &= ~SIDE_TESTED;		// Remove the tested flag
		}
	}

	// See if we have valid polygons on both sides
	for (i=0 ; i<2 ; i++)
	{
		if (!jeBSP_BrushCalcBounds(b[i]))
		{
			jeErrorLog_AddString(-1, "jeBSP_BrushSplit:  jeBSP_BrushCalcBounds failed.", NULL);
			return JE_FALSE;
		}
		//assert(jeBSP_BrushIsValidGeometry(b[i]) == JE_TRUE);
		
		if (!jeBSP_BrushIsValidGeometry(b[i]))
		{
			jeBSP_BrushDestroy(&b[i]);
			//return JE_FALSE;
		}
		
	}


	if (!(b[0] && b[1]) )		// If either brush got removed
	{
		if (!b[0] && !b[1])		// If both brushes got removed
			Log_Printf("jeBSP_BrushSplit: Split removed brush\n");
		else
			Log_Printf("jeBSP_BrushSplit: Split not on both sides\n");
		
		if (b[0])
		{
			jeBSP_BrushDestroy(&b[0]);
			*Front = jeBSP_BrushCreateFromBSPBrush(Brush);
		}
		if (b[1])
		{
			jeBSP_BrushDestroy(&b[1]);
			*Back = jeBSP_BrushCreateFromBSPBrush(Brush);
		}
		return JE_TRUE;
	}

	// Add the midpoly to both sides
	for (i=0 ; i<2 ; i++)
	{
		cs = &b[i]->Sides[b[i]->NumSides];
		b[i]->NumSides++;

		cs->PlaneIndex = Index;		// The MidPolys plane will be the splitplane index
		cs->Flags = MidFlags;		// Store the mid flags
		cs->Flags |= SIDE_SPLIT;	// Remember that this side was a result of a split

		if (!i)		// Reverse MidPoly on front side of split plane
		{
			cs->PlaneIndex = jePlaneArray_IndexReverse(cs->PlaneIndex);
			cs->Poly = jePoly_CreateFromPoly(MidPoly, JE_TRUE);
		}
		else
		{
			cs->Poly = MidPoly;
		}

		if (!cs->Poly)
			return JE_FALSE;
	}

	// Check the brushes for tiny volumes
	for (i=0 ; i<2 ; i++)
	{
		jeFloat		Val;

		Val = jeBSP_BrushVolume(b[i], BSP);

		if (Val < JE_BSP_TINY_VOLUME)
			jeBSP_BrushDestroy(&b[i]);
	}
	
	if (!b[0] && !b[1])
		Log_Printf("jeBSP_BrushSplit: Split removed brush (2)\n");
		//return JE_FALSE;		// The above code should surely take care of bad brushes
	
	*Front = b[0];
	*Back = b[1];

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_BrushSubtract
//	Frees a+b, returns new brush.  New brush might be unique, a, b, or NULL.
//=======================================================================================
jeBoolean jeBSP_BrushSubtract(jeBSP_Brush *a, jeBSP_Brush *b, jeBSP *BSP, jeBSP_Brush **Result)
{	
	int32		i;
	jeBSP_Brush	*Front, *Back;
	jeBSP_Brush	*Out, *In;
	jeBSP_Side	*Side;

	In = a;
	Out = NULL;

	Side = b->Sides;

	for (i=0 ; i<b->NumSides && In ; i++, Side++)
	{
		if (!jeBSP_BrushSplit(In, BSP, Side->PlaneIndex, SIDE_NODE, &Front, &Back))
		{
			*Result = NULL;
			return JE_FALSE;
		}

		if (In != a)
			jeBSP_BrushDestroy(&In);

		if (Front)				// Keep front side
		{	
			Front->Next = Out;
			Out = Front;
		}
		In = Back;				// Keep splitting back side
	}
	if (In)
		jeBSP_BrushDestroy(&In);
	else
	{	// didn't really intersect
		jeBSP_BrushDestroyList(&Out);
		*Result = a;
		return JE_TRUE;
	}

	*Result = Out;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_BrushCSGList
//	Input is freed, only keeps output...
//=======================================================================================
jeBSP_Brush *jeBSP_BrushCSGList(jeBSP_Brush *List, jeBSP *BSP)
{
	jeBSP_Brush	*b1, *b2, *Next;
	jeBSP_Brush	*Tail;
	jeBSP_Brush	*Keep;
	jeBSP_Brush	*Sub, *Sub2;
	int32		c1, c2;

	Log_Printf("--- jeBSP_BrushCSGList --- \n");
	Log_Printf("Num brushes before CSG : %5i\n", jeBSP_BrushCountList(List));
	
	Keep = NULL;

	NewList:

	// Find tail
	if (!List)
		return NULL;

	for (Tail=List ; Tail->Next ; Tail=Tail->Next);

	for (b1=List ; b1 ; b1=Next)
	{
		Next = b1->Next;
		
		for (b2=b1->Next ; b2 ; b2 = b2->Next)
		{
			if (jeBSP_BrushDisjoint(b1, b2))
				continue;

			Sub = NULL;
			Sub2 = NULL;
			c1 = 999999;
			c2 = 999999;

			if (jeBSP_BrushCanBite(b2, b1) )
			{
				if (!jeBSP_BrushSubtract(b1, b2, BSP, &Sub))
					return NULL;

				if (Sub == b1)
					continue;		// Didn't really intersect

				if (!Sub)
				{	
					// b1 is swallowed by b2
					List = jeBSP_BrushCullList(b1, b1);
					goto NewList;
				}

				c1 = jeBSP_BrushCountList(Sub);
			}

			if ( jeBSP_BrushCanBite(b1, b2) )
			{
				if (!jeBSP_BrushSubtract(b2, b1, BSP, &Sub2))
					return NULL;

				if (Sub2 == b2)
					continue;		// didn't really intersect

				if (!Sub2)
				{	
					// b2 is swallowed by b1
					jeBSP_BrushDestroyList(&Sub);
					List = jeBSP_BrushCullList (b1, b2);
					goto NewList;
				}
				c2 = jeBSP_BrushCountList(Sub2);
			}

			if (!Sub && !Sub2)
				continue;		// neither one can bite

			// only accept if it didn't fragment
			// (commenting this out allows full fragmentation)
		#if 0
			if (c1 > 4 && c2 > 4)
			{
				if (Sub2)
					jeBSP_BrushDestroyList(&Sub2);
				if (Sub)
					jeBSP_BrushDestroyList(&Sub);

				continue;
			}
		#endif
			
			if (c1 < c2)
			{
				if (Sub2)
					jeBSP_BrushDestroyList(&Sub2);
				Tail = jeBSP_BrushListAddToTail(Sub, Tail);
				List = jeBSP_BrushCullList(b1, b1);
				goto NewList;
			}
			else
			{
				if (Sub)
					jeBSP_BrushDestroyList(&Sub);
				Tail = jeBSP_BrushListAddToTail (Sub2, Tail);
				List = jeBSP_BrushCullList(b1, b2);
				goto NewList;
			}
			
			
		}

		if (!b2)
		{	// b1 is no longer intersecting anything, so keep it
			b1->Next = Keep;
			Keep = b1;
		}
	}

	Log_Printf("Num brushes after CSG  : %5i\n", jeBSP_BrushCountList(Keep));

	return Keep;
}

