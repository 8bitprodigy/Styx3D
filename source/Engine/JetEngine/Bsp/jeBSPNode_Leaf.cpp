/****************************************************************************************/
/*  JEBSPNODE_LEAF.C                                                                    */
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


#include "jeBSP._h"

#include "Errorlog.h"
#include "Log.h"
#include "Ram.h"

//=======================================================================================
//	jeBSPNode_LeafCreate
//=======================================================================================
jeBSPNode_Leaf *jeBSPNode_LeafCreate(jeBSP *BSP)
{
	jeBSPNode_Leaf		*Leaf;

	Leaf = JE_RAM_ALLOCATE_STRUCT(jeBSPNode_Leaf);

	if (!Leaf)
		return NULL;

	ZeroMem(Leaf);

	LN_Null(Leaf);

	return Leaf;
}

//=======================================================================================
//	jeBSPNode_LeafDestroy
//=======================================================================================
void jeBSPNode_LeafDestroy(jeBSPNode_Leaf **Leaf, jeBSP *BSP)
{
	jeBSP_Brush		*Brush, *Next;

	assert(Leaf);
	assert(*Leaf);

	for (Brush = (*Leaf)->Brushes; Brush; Brush = Next)
	{
		Next = Brush->Next;

		jeBSP_BrushDestroy(&Brush);
	}

	if ((*Leaf)->Sides)
	{
		int32		i;

		for (i=0; i< (*Leaf)->NumSides; i++)
		{
			jePlaneArray_RemovePlane(BSP->PlaneArray, &(*Leaf)->Sides[i].PlaneIndex);
		}

		jeRam_Free((*Leaf)->Sides);
	}

	if ((*Leaf)->Area)
		jeBSPNode_AreaDestroy(&(*Leaf)->Area);

	jeBSPNode_LeafDestroyDrawFaceList(*Leaf, BSP);

	jeRam_Free(*Leaf);

	*Leaf = NULL;
}

#define MAX_TEMP_LEAF_SIDES			(256)

//=====================================================================================
//	jeBSPNode_LeafInitializeSides
//=====================================================================================
jeBoolean jeBSPNode_LeafInitializeSides(jeBSPNode_Leaf *Leaf, jeBSP *BSP)
{
	jePlane				Plane;
	int32				Axis, i, Dir;
	jeBSPNode			*Node;
	jeBSPNode_Portal	*Portal;
	jeBSPNode_LeafSide	TempLeafSides[MAX_TEMP_LEAF_SIDES];
	int32				NumTempLeafSides, Side;
	jeBSPNode_LeafSide	*pSide;

	assert(Leaf);
	assert(Leaf->Node);
	assert(!Leaf->Sides);
	assert(!Leaf->NumSides);

	if (!Leaf->Node->Portals)
		return JE_TRUE;			// If no portals, then can't make sides

	Node = Leaf->Node;

	NumTempLeafSides = 0;
	pSide = TempLeafSides;

#if 1
	for (Portal = Node->Portals; Portal; Portal = Portal->Next[Side])
	{
		jeBSPNode_LeafSide	*pSide2;
		jePlaneArray_Index	PlaneIndex;

		assert(!(Portal->Nodes[0] == Node && Portal->Nodes[1] == Node));
		assert(Portal->Nodes[0] == Node || Portal->Nodes[1] == Node);

		Side = (Portal->Nodes[1] == Node);

		PlaneIndex = Portal->PlaneIndex;

		// Don't add planes already in the list
		for (pSide2 = TempLeafSides, i=0; i< NumTempLeafSides; i++, pSide2++)
		{
			if (jePlaneArray_IndexIsCoplanar(pSide2->PlaneIndex, PlaneIndex))
				break;
		}

		if (i != NumTempLeafSides)
			continue;				// Plane already in list

		// Make sure we don't overflow our TempSides array
		if (NumTempLeafSides >= MAX_TEMP_LEAF_SIDES)
		{
			//jeErrorLog_AddString(-1, "jeBSPNode_LeafInitializeSides:  NumTempLeafSides >= MAX_TEMP_LEAF_SIDES.", NULL);
			//return JE_FALSE;
			return JE_TRUE;
		}

		pSide->PlaneIndex = PlaneIndex;

		// Reverse BEFORE we ref count so it will ref the correct plane
		// NOTE - Portals without nodes are portals on the outside, these are allways facing in, flip them outwards...
		if (!Side || !Portal->OnNode)
			pSide->PlaneIndex = jePlaneArray_IndexReverse(pSide->PlaneIndex);

		// Ref the planeindex
		if (!jePlaneArray_RefPlaneByIndex(BSP->PlaneArray, pSide->PlaneIndex))
			return JE_FALSE;

		NumTempLeafSides++;
		pSide++;
	}
#endif

#if 1
	// Add any bevel planes to the sides so we can expand them for axial box collisions
	for (Axis=0 ; Axis <3 ; Axis++)
	{
		for (Dir=-1 ; Dir <= 1 ; Dir+=2)
		{
			// See if the plane is allready in the sides
			for (pSide = TempLeafSides, i=0; i< NumTempLeafSides; i++, pSide++)
			{
				const jePlane		*pPlane;
				jeFloat				FloatDir;

				pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, pSide->PlaneIndex);
				assert(pPlane);

				if (jePlaneArray_IndexSided(pSide->PlaneIndex))
					FloatDir = (jeFloat)-Dir;
				else
					FloatDir = (jeFloat)Dir;

				if (jeVec3d_GetElement(&pPlane->Normal, Axis) == FloatDir)
					break;
			}

			if (i != NumTempLeafSides)
				continue;		// Side exist, don't add

			if (NumTempLeafSides >= MAX_TEMP_LEAF_SIDES)
			{
				//jeErrorLog_AddString(-1, "jeBSPNode_LeafInitializeSides:  NumTempLeafSides >= MAX_TEMP_LEAF_SIDES.", NULL);
				//return JE_FALSE;
				return JE_TRUE;
			}

			// Add a new axial aligned side
			jeVec3d_Clear(&Plane.Normal);
			jeVec3d_SetElement(&Plane.Normal, Axis, (jeFloat)Dir);
				
			// get the mins/maxs from the gbsp brush
			if (Dir == 1)
				Plane.Dist = jeVec3d_GetElement(&Node->Box.Max, Axis);
			else
				Plane.Dist = -jeVec3d_GetElement(&Node->Box.Min, Axis);
				
			TempLeafSides[NumTempLeafSides].PlaneIndex = jePlaneArray_SharePlane(BSP->PlaneArray, &Plane);

			if (TempLeafSides[NumTempLeafSides].PlaneIndex == JE_PLANEARRAY_NULL_INDEX)
				return JE_FALSE;

			NumTempLeafSides++;
			//NumLeafBevels++;
		}
	}		
#endif

	if (NumTempLeafSides < 3)
		return JE_TRUE;

	// Allocate the sides for the leaf and copy them over
	Leaf->Sides = JE_RAM_ALLOCATE_ARRAY(jeBSPNode_LeafSide, NumTempLeafSides);

	if (!Leaf->Sides)
		return JE_FALSE;

	// Copy over the sides in the allocated sides for the leaf
	for (i=0; i< NumTempLeafSides; i++)
		Leaf->Sides[i] = TempLeafSides[i];

	Leaf->NumSides = NumTempLeafSides;

	return JE_TRUE;
}

//=====================================================================================
//	jeBSPNode_LeafFill_r
//=====================================================================================
jeBoolean jeBSPNode_LeafFill_r(jeBSPNode_Leaf *Leaf, int32 Fill)
{
	jeBSPNode_Portal	*Portal;
	int32				Side;
	jeBSPNode			*Node;

	assert(Leaf);
	assert(Leaf->Node);		

	if (Leaf->Node->Flags & NODE_OUTSIDE)
		Log_Printf("**** Leak ****\n");

	if (Leaf->Contents & JE_BSP_CONTENTS_SOLID)
		return JE_TRUE;

	if (Leaf->CurrentFrame == Fill)					// Only visit leafs once
		return JE_TRUE;

	Leaf->CurrentFrame = Fill;						// Mark the leaf as visitied

	Node = Leaf->Node;		// Get node that this leaf is on

	for (Portal = Node->Portals; Portal; Portal = Portal->Next[Side])
	{
		jeBSPNode_Leaf	*LeafTo;

		assert(!(Portal->Nodes[0] == Node && Portal->Nodes[1] == Node));
		assert(Portal->Nodes[0] == Node || Portal->Nodes[1] == Node);

		Side = (Portal->Nodes[1] == Node);

		LeafTo = Portal->Nodes[!Side]->Leaf;
		assert(LeafTo);

		// Flood to the leaf on the other side of the portal (!side)
		if (!jeBSPNode_LeafFill_r(LeafTo, Fill))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_LeafFloodAreas_r
//		Marks the current leaf with the current area, and adds the leaf to the areas LeafArray.
//		It then proceeds to keep flooding till it hits solid wall, or another vis portal
//=======================================================================================
jeBoolean jeBSPNode_LeafFloodAreas_r(jeBSPNode_Leaf *Leaf, jeBSPNode_Area *Area)
{
	jeBSPNode			*Node;
	jeBSPNode_Portal	*p;
	int32				Side;
	jeBSPNode_AreaLeaf	*AreaLeaf;

	assert(Leaf);
	assert(Area);
	
	if (Leaf->Area)
		return JE_TRUE;		// Area already found

	if (Leaf->Contents & JE_BSP_CONTENTS_SOLID)
		return JE_TRUE;		// Solid, can't cross portal

	// Add this leaf to the list of leafs on the Areas LeafArray...
	AreaLeaf = (jeBSPNode_AreaLeaf*)jeArray_GetNewElement(Area->LeafArray);
	AreaLeaf->Leaf = Leaf;

	// This leaf now has an area
	Leaf->Area = Area;

	// Ref the area (Area should have at least as many refs, as the number of leafs touching it)
	if (!jeBSPNode_AreaCreateRef(Area))
		return JE_FALSE;

	// Grab the node of the leaf
	Node = Leaf->Node;
	assert(Node);

	for (p = Node->Portals; p; p = p->Next[Side])
	{
		Side = (p->Nodes[1] == Node);

		if (p->Side && p->Side->Flags & SIDE_VIS_PORTAL)
			continue;		// Don't flood out vis portals

		// Flood to the leaf on the opposite side of the portal
		if (!jeBSPNode_LeafFloodAreas_r(p->Nodes[!Side]->Leaf, Area))
			return JE_FALSE;
	}
	
	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_LeafMakeAreas
//		Starts at the current leaf, and floods out (jeBSPNode_LeafFloodAreas_r) and marks 
//		all areas touched with the same area.  
//=======================================================================================
jeBoolean jeBSPNode_LeafMakeAreas(jeBSPNode_Leaf *Leaf, jeBSP *BSP, jeChain *AreaChain)
{
	jeBSPNode		*Node;
	jeBSPNode_Area	*Area;

	assert(Leaf);
	assert(AreaChain);

	Node = Leaf->Node;
	assert(Node);

	if (Leaf->Contents & JE_BSP_CONTENTS_SOLID)
		return JE_TRUE;		// Solids can't have areas

	if (Leaf->Area)
		return JE_TRUE;		// Area already set
	
	BSP->DebugInfo.NumAreas++;

	// Create an Area, and start flooding out of the current leaf using jeBSPNode_LeafFloodAreas_r
	Area = jeBSPNode_AreaCreate();

	if (!Area)
		return JE_FALSE;

	if (!jeChain_AddLinkData(AreaChain, Area))
		return JE_FALSE;
		
	// Flood from this leaf, setting all touched leafs to this area
	if (!jeBSPNode_LeafFloodAreas_r(Leaf, Area))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_LeafCountAreaVisPortals
//=======================================================================================
jeBoolean jeBSPNode_LeafCountAreaVisPortals(jeBSPNode_Leaf *Leaf)
{
	jeBSPNode			*Node;
	jeBSPNode_Portal	*p;
	int32				Side;

	assert(Leaf);

	if (Leaf->Contents & JE_BSP_CONTENTS_SOLID)
		return JE_TRUE;

	Node = Leaf->Node;
	assert(Node);

	for (p = Node->Portals; p; p = p->Next[Side])
	{
		jeBSPNode_Area			*Area, *OtherArea;
		jeBSPNode_Leaf			*OtherLeaf;

		Side = (p->Nodes[1] == Node);

		if (!p->Side)		
			continue;		// Portal didn't have side, not visible

		if (!(p->Side->Flags & SIDE_VIS_PORTAL))
			continue;		// Only interested in VIS_PORTALS's

		Area = p->Nodes[Side]->Leaf->Area;
		assert(Area);
		assert(Area == Leaf->Area);

		OtherLeaf = p->Nodes[!Side]->Leaf;
		assert(OtherLeaf);
		assert(OtherLeaf != Leaf);

		if (OtherLeaf->Contents & JE_BSP_CONTENTS_SOLID)
			continue;		// Ignore VisPortals that flood into solid

		OtherArea = OtherLeaf->Area;
		assert(OtherArea);

		if (Area == OtherArea)		
			continue;		// Invalid vis portal! Seperates same area...
		
		Area->NumAreaPortals++;
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_LeafMakeAreaVisPortals
//=======================================================================================
jeBoolean jeBSPNode_LeafMakeAreaVisPortals(jeBSPNode_Leaf *Leaf, jeBSP *BSP)
{
	jeBSPNode			*Node;
	jeBSPNode_Portal	*p;
	int32				Side;

	assert(Leaf);

	if (Leaf->Contents & JE_BSP_CONTENTS_SOLID)
		return JE_TRUE;

	Node = Leaf->Node;
	assert(Node);

	for (p = Node->Portals; p; p = p->Next[Side])
	{
		jeBSPNode_AreaPortal	*AreaPortal;
		jeBSPNode_Area			*Area, *OtherArea;
		jeBSPNode_Leaf			*OtherLeaf;

		Side = (p->Nodes[1] == Node);

		if (!p->Side)		
			continue;		

		if (!(p->Side->Flags & SIDE_VIS_PORTAL))
			continue;		// Only interested in VIS_PORTALS's

		Area = p->Nodes[Side]->Leaf->Area;
		assert(Area);
		assert(Area == Leaf->Area);

		OtherLeaf = p->Nodes[!Side]->Leaf;
		assert(OtherLeaf);
		assert(OtherLeaf != Leaf);

		if (OtherLeaf->Contents & JE_BSP_CONTENTS_SOLID)
			continue;		// Ignore VisPortals that flood into solid

		OtherArea = OtherLeaf->Area;
		assert(OtherArea);

		if (Area == OtherArea)		
			continue;		// Invalid vis portal! Seperates same area...

		assert(Area->NumWorkAreaPortals < Area->NumAreaPortals);
		assert(Area->AreaPortals);
	
		AreaPortal = &Area->AreaPortals[Area->NumWorkAreaPortals++];

		AreaPortal->Poly = jePoly_CreateFromPoly(p->Poly, !Side);

		if (!AreaPortal->Poly)
			return JE_FALSE;

		AreaPortal->Target = OtherArea;		// This portal looks into the other area

		AreaPortal->Plane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, p->OnNode->PlaneIndex);

		if (!Side)
			jePlane_Inverse(&AreaPortal->Plane);
	}

	return JE_TRUE;
}

//=======================================================================================
//	CountFaces_r
//=======================================================================================
static void CountLeafFaces_r(jeBSPNode_Leaf *Leaf, jeBSPNode_Face *Face)
{
	assert(Face);

	while (Face->Merged)
		Face = Face->Merged;

	if (Face->Split[0])
	{
		CountLeafFaces_r(Leaf, Face->Split[0]);
		CountLeafFaces_r(Leaf, Face->Split[1]);
		return;
	}

	if ( Face->DrawFace )
	{
		Leaf->NumDrawFaces++;
	}
}

//=======================================================================================
//	GetLeafFaces_r
//=======================================================================================
static void GetLeafFaces_r(jeBSPNode_Leaf *Leaf, jeBSPNode_Face *Face)
{
	assert(Face);

	while (Face->Merged)
	{
		assert(!Face->DrawFace);
		Face = Face->Merged;
	}

	if (Face->Split[0])
	{
		assert(!Face->DrawFace);
		GetLeafFaces_r(Leaf, Face->Split[0]);
		GetLeafFaces_r(Leaf, Face->Split[1]);
		return;
	}

//	assert(Face->DrawFace); // portal Faces don't face DrawFaces !

	if ( Face->DrawFace )
	{
		Leaf->DrawFaces[Leaf->NumDrawFaces++] = Face->DrawFace;
	}
}

//=======================================================================================
//	jeBSPNode_LeafMakeDrawFaceList
//=======================================================================================
jeBoolean jeBSPNode_LeafMakeDrawFaceList(jeBSPNode_Leaf *Leaf, jeBSP *BSP)
{
	int32				s;
	jeBSPNode_Portal	*p;
	jeBSPNode			*Node;

	assert(Leaf);

	Node = Leaf->Node;

	assert(Node);
	assert(Node->Leaf == Leaf);

	// Destroy any previous list of drawfaces on this leaf
	jeBSPNode_LeafDestroyDrawFaceList(Leaf, BSP);

	// Count number of faces on this leaf
	Leaf->NumDrawFaces = 0;
	for (p = Node->Portals; p; p = p->Next[s])
	{
		s = (p->Nodes[1] == Node);

		if (!p->Face[s])
			continue;

		CountLeafFaces_r(Leaf, p->Face[s]);
	}

	if (!Leaf->NumDrawFaces)			// Don't bother making anything
		return JE_TRUE;

	// Allocate space for the array of pointers to the faces
	Leaf->DrawFaces = JE_RAM_ALLOCATE_ARRAY(jepBSPNode_DrawFace, Leaf->NumDrawFaces);

	Leaf->NumDrawFaces = 0;
	for (p = Node->Portals; p; p = p->Next[s])
	{
		s = (p->Nodes[1] == Node);

		if (!p->Face[s])
			continue;

		GetLeafFaces_r(Leaf, p->Face[s]);
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_LeafDestroyDrawFaceList
//=======================================================================================
void jeBSPNode_LeafDestroyDrawFaceList(jeBSPNode_Leaf *Leaf, jeBSP *BSP)
{
	assert(Leaf);

	if (Leaf->DrawFaces)
	{
		assert(Leaf->NumDrawFaces);
		jeRam_Free(Leaf->DrawFaces);
		Leaf->DrawFaces = NULL;
		Leaf->NumDrawFaces = 0;
	}
	else
	{
		assert(!Leaf->NumDrawFaces);
	}
}

//=======================================================================================
//	jeBSPNode_BubbleRecursionBit
//=======================================================================================
void jeBSPNode_BubbleRecursionBit(jeBSPNode *Node, uint32 RecursionBit)
{
	assert(jeBSPNode_IsValid(Node));

	while (Node)
	{
		Node->RecursionBits |= RecursionBit;
		Node = Node->Parent;
	}
}

//=======================================================================================
//	jeBSPNode_LeafSetRecursionBit
//=======================================================================================
void jeBSPNode_LeafSetRecursionBit(jeBSPNode_Leaf *Leaf, uint32 RecursionBit)
{
	int32		i;

	assert(Leaf);

	if (Leaf->Node->RecursionBits & RecursionBit)
		return;		// Don't bother messing with it again...

	Leaf->Node->RecursionBits |= RecursionBit;

	// Set all faces on the leaf as visible...
	for (i=0; i< Leaf->NumDrawFaces; i++)
	{
		Leaf->DrawFaces[i]->RecursionBits |= RecursionBit;
	}

	// Bubble vis info up to parents
	jeBSPNode_BubbleRecursionBit(Leaf->Node->Parent, RecursionBit);
}

//#define LEAF_COLLISION_EPSILON	(0.1f)
//#define LEAF_COLLISION_EPSILON	(0.0001f)  // DarkRift/Incarnadine
#define LEAF_COLLISION_EPSILON	(0.01f)  // Incarnadine

//=====================================================================================
//	ExpandPlaneForBox
//	Pushes a plane out by the side of the box it is looking at
//=====================================================================================
static void ExpandPlaneForBox(jePlane *Plane, const jeVec3d *Mins, const jeVec3d *Maxs)
{
	jeVec3d		*Normal;

	Normal = &Plane->Normal;
	
	if (Normal->X > 0)
		Plane->Dist -= Normal->X * Mins->X;
	else	 
		Plane->Dist -= Normal->X * Maxs->X;
	
	if (Normal->Y > 0)
		Plane->Dist -= Normal->Y * Mins->Y;
	else
		Plane->Dist -= Normal->Y * Maxs->Y;

	if (Normal->Z > 0)
		Plane->Dist -= Normal->Z * Mins->Z;
	else							 
		Plane->Dist -= Normal->Z * Maxs->Z;
}

//=====================================================================================
//	jeBSPNode_LeafCollision_r
//=====================================================================================
jeBoolean jeBSPNode_LeafCollision_r(const jeBSPNode_Leaf *Leaf, jeBSP *BSP, int32 Side, int32 PSide, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeBSPNode_CollisionInfo2 *Info)
{
	jeFloat				Fd, Bd, Dist;
	jeBSPNode_LeafSide	*pSide;
	jePlane				Plane;
	int32				Side2;
	jeVec3d				I;

	assert(Leaf);
	assert(Box);
	assert(Front);
	assert(Back);
	assert(Info);
	assert(Leaf->Sides);

	if (!PSide)
		return JE_FALSE;		// Ray was on front side, not a collision when dealing with convex hulls

	if (!(Leaf->Contents & JE_BSP_CONTENTS_SOLID)) // Icestorm/Incarnadine
		return JE_FALSE;

	if (Side >= Leaf->NumSides)
	{
		// We did the solid test above already, so we already know the answer. - Icestorm
		return JE_TRUE;

		/*
		if (Leaf->Contents & JE_BSP_CONTENTS_SOLID)
			return JE_TRUE;		// If it lands behind all sides, it is inside
		else
			return JE_FALSE;
		*/
	}

	pSide = &Leaf->Sides[Side];

	Plane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, pSide->PlaneIndex);
	Plane.Type = Type_Any;
	
	if (jePlaneArray_IndexSided(pSide->PlaneIndex))
		jePlane_Inverse(&Plane);
	
	// Simulate the point having a box, by pushing the plane out by the box size
	ExpandPlaneForBox(&Plane, &Box->Min, &Box->Max);

	Fd = jePlane_PointDistanceFast(&Plane, Front);
	Bd = jePlane_PointDistanceFast(&Plane, Back);

	if (Fd >= 0 && Bd >= 0)	// Leaf sides are convex hulls, so front side is totally outside
		return JE_FALSE;

	if (Fd < 0 && Bd < 0)
		return jeBSPNode_LeafCollision_r(Leaf, BSP, Side+1, 1, Box, Front, Back, Info);

	// We have an intersection
    Side2 = Fd < 0;
	
	/* Icestorm/Incarnadine	
	if (Fd < 0)
		Dist = (Fd + LEAF_COLLISION_EPSILON)/(Fd-Bd);
	else
		Dist = (Fd - LEAF_COLLISION_EPSILON)/(Fd-Bd);
	*/

	Dist = (Fd - LEAF_COLLISION_EPSILON)/(Fd-Bd);

	if (Dist < 0.0f)
		Dist = 0.0f;
	
	if (Dist > 1.0f)
		Dist = 1.0f;

    I.X = Front->X + Dist * (Back->X - Front->X);
    I.Y = Front->Y + Dist * (Back->Y - Front->Y);
    I.Z = Front->Z + Dist * (Back->Z - Front->Z);

	// Only go down the back side, since the front side is empty in a convex tree
	if (jeBSPNode_LeafCollision_r(Leaf, BSP, Side+1, Side2, Box, Front, &I, Info))
	{
		// No collision info needs to be set here or the wrong impact
		// point could be returned.  - Icestorm/Incarnadine
		return JE_TRUE;
	}
	else if (jeBSPNode_LeafCollision_r(Leaf, BSP, Side+1, !Side2, Box, &I, Back, Info))
	{
		// Record the intersection closest to the start of ray
		if (!Info->HitLeaf)
		{
			jeFloat		Dist;

			Dist = jeVec3d_DistanceBetween(Info->Front, &I);

			if (Dist < Info->BestDist)
			{
				Info->BestDist = Dist;
				*(Info->Impact) = I;
				*(Info->Plane) = Plane;				
			}	
			Info->HitLeaf = JE_TRUE;	// Icestorm: should prevent unnecessary tests
		}
		Info->HitSet = JE_TRUE;	// For collisioncheck only
		return JE_TRUE;
	}
	
	return JE_FALSE;	
}

// Added by Icestorm
//=====================================================================================
//	GetChangeBoxPlaneImpact
//	Pushes a plane out by the side of the box it is looking at
//  and get this point
//=====================================================================================
static void GetChangeBoxPlaneImpact(jePlane *Plane, const jeExtBox *Box, const jeVec3d *Pos)
{
	jeVec3d Impact;

	if (Plane->Normal.X > 0)
		Impact.X=Box->Min.X+Pos->X;
	else	 
		Impact.X=Box->Max.X+Pos->X;	

	if (Plane->Normal.Y > 0)
		Impact.Y=Box->Min.Y+Pos->Y;
	else
		Impact.Y=Box->Max.Y+Pos->Y;

	if (Plane->Normal.Z > 0)
		Impact.Z=Box->Min.Z+Pos->Z;
	else				
		Impact.Z=Box->Max.Z+Pos->Z;

	Plane->Dist = Plane->Normal.X * Impact.X;
	Plane->Dist += Plane->Normal.Y * Impact.Y;
	Plane->Dist += Plane->Normal.Z * Impact.Z;
}

//=====================================================================================
//	jeBSPNode_LeafChangeBoxCollision_r
//=====================================================================================
jeBoolean jeBSPNode_LeafChangeBoxCollision_r(const jeBSPNode_Leaf *Leaf, jeBSP *BSP, int32 Side, int32 PSide, const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeBSPNode_CollisionInfo3 *Info)
{
	jeFloat				Fd, Bd,Fd2,Bd2, Dist;
	jeBSPNode_LeafSide	*pSide;
	jePlane				Plane,FPlane,FPlane2,BPlane,BPlane2;
	int32				Side2;
	jeExtBox			IBox;

	assert(Leaf);
	assert(Pos);
	assert(FrontBox);
	assert(BackBox);
	assert(Info);
	assert(Leaf->Sides);

	if (!PSide)
		return JE_FALSE;		// Box was on front side, not a collision when dealing with convex hulls

	if (!(Leaf->Contents & JE_BSP_CONTENTS_SOLID))
		return JE_FALSE;

	// We did the solid test above already, so we already know the answer.
	if (Side >= Leaf->NumSides)
		return JE_TRUE;

	pSide = &Leaf->Sides[Side];

	Plane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, pSide->PlaneIndex);
	Plane.Type = Type_Any;
	
	if (jePlaneArray_IndexSided(pSide->PlaneIndex))
		jePlane_Inverse(&Plane);
	
	FPlane=BPlane=FPlane2=BPlane2=Plane;
	// Push the plane to all four corners (front/back and biggest/smallest dist.)
	// and get the dists.
	ExpandPlaneForBox(&FPlane,  &FrontBox->Min, &FrontBox->Max);
	ExpandPlaneForBox(&FPlane2, &FrontBox->Max, &FrontBox->Min);

	Fd  = jePlane_PointDistanceFast(&FPlane,  Pos);
	Fd2 = jePlane_PointDistanceFast(&FPlane2, Pos);

	ExpandPlaneForBox(&BPlane,  &BackBox->Min, &BackBox->Max);
	ExpandPlaneForBox(&BPlane2, &BackBox->Max, &BackBox->Min);

	Bd  = jePlane_PointDistanceFast(&BPlane,  Pos);
	Bd2 = jePlane_PointDistanceFast(&BPlane2, Pos);

	if (Fd >= 0 && Bd >= 0 && Fd2>=0 && Bd2>=0)	// Leaf sides are convex hulls, so boxes are totally outside
		return JE_FALSE;

	if (Fd < 0 && Bd < 0 && Fd2<0 && Bd2<0)  // all are inside
		return jeBSPNode_LeafChangeBoxCollision_r(Leaf, BSP, Side+1, 1, Pos, FrontBox, BackBox, Info);

	// both are in- and outside: test rest of planes
	// same as Frontbox inside,except there is no IBox 
	if ((Fd < 0 && Fd2 >= 0) || (Fd >= 0 && Fd2 < 0)) 
		return jeBSPNode_LeafChangeBoxCollision_r(Leaf, BSP, Side+1, 1, Pos, FrontBox, BackBox, Info);

	// We have an intersection
    Side2 = Fd < 0;
	
	if (Side2) { Fd=Fd2;Bd=Bd2; } // Get side of collision

	Dist = (Fd - LEAF_COLLISION_EPSILON)/(Fd-Bd);

	if (Dist < 0.0f)
		Dist = 0.0f;
	
	if (Dist > 1.0f)
		Dist = 1.0f;

	// Calc. new changed box, a little bit outside
    IBox.Min.X = FrontBox->Min.X + Dist * (BackBox->Min.X - FrontBox->Min.X);
    IBox.Min.Y = FrontBox->Min.Y + Dist * (BackBox->Min.Y - FrontBox->Min.Y);
    IBox.Min.Z = FrontBox->Min.Z + Dist * (BackBox->Min.Z - FrontBox->Min.Z);

	IBox.Max.X = FrontBox->Max.X + Dist * (BackBox->Max.X - FrontBox->Max.X);
    IBox.Max.Y = FrontBox->Max.Y + Dist * (BackBox->Max.Y - FrontBox->Max.Y);
    IBox.Max.Z = FrontBox->Max.Z + Dist * (BackBox->Max.Z - FrontBox->Max.Z);

	// Only go down the back side, since the front side is empty in a convex tree
	if (jeBSPNode_LeafChangeBoxCollision_r(Leaf, BSP, Side+1, Side2, Pos, FrontBox, &IBox, Info))
	{
		// No collision info needs to be set here or the wrong impact
		// box could be returned.
		return JE_TRUE;
	}
	else if (jeBSPNode_LeafChangeBoxCollision_r(Leaf, BSP, Side+1, !Side2, Pos, &IBox, BackBox, Info))
	{
		// Record the intersection closest to the start of ray
		if (!Info->HitLeaf)
		{
			jeFloat		Dist;

			GetChangeBoxPlaneImpact(&Plane,&IBox,Pos);
			Dist = jeVec3d_DistanceBetween(&Info->FrontBox->Min, &IBox.Min);

			if (Dist < Info->BestDist)
			{
				Info->BestDist = Dist;
				*(Info->ImpactBox) = IBox;
				*(Info->Plane) = Plane;
			}	
			
			Info->HitLeaf = JE_TRUE;
						
		}
		Info->HitSet = JE_TRUE;		// Icestorm: For collisioncheck only
		return JE_TRUE;
	}
	
	return JE_FALSE;	
}