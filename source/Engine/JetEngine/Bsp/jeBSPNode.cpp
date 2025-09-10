/****************************************************************************************/
/*  JEBSPNODE.C                                                                         */
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
#include <math.h>

#include "Dcommon.h"
#include "jeBSP._h"
#include "jeBSP.h"

#include "Errorlog.h"
#include "Log.h"
#include "Ram.h"
#include "Quatern.h"


// Added by chrisjp : Engine now looks at driver preferences when creating lightmaps.
extern jePixelFormat bestSupportedLightmapPixelFormat;

void DetermineSupportedLightmapFormat(jePixelFormat goalFormat, DRV_Driver *Driver);
// End added by chrisjp

//****** NOTES!!!
//
//	Functions that end in _r, will be recursively re-entered.
//	Functions that end in _Callr, will call the _r function of the SAME TYPE ONLY
//	Example:
//		jeBSP_MakeBSPFaces(Node);		// This will make the faces
//		jeBSP_MakeBSPFaces_r(Node);		// This will call jeBSP_MakeBSPFaces
//		jeBSP_MakeBSPFaces_Callr(Node);	// This will call jeBSP_MakeFaces_r
//

//
//	Ok, thus far, this is how the jeBSPNode_Portal, jeBSPNode_Face, and jeBSPNode_DrawFace system works on the nodes.
//
//	First, an explanation on how the BSP is created/maintained.  The BSP is basically a bunch of partitioned
//	jeBSP_Brush's.  They are partitioned to constantly mantain a tree of convex pieces.  These peices form the
//	jeBSPNode_Leaf's.  
//
//	During the life of the tree, it maintains list of jeBSPNode_Portal's that are passages
//	from one leaf to another.  At the same time, it keeps a list of jeBSPNode_Faces on portals that seperate
//	visible passages (i.e. walls, etc...).  
//
//	The jeBSPNode_Face is kind of tricky though.  When it's created on the portal, it adds itself to the list
//	of faces on the node that created the portal.  Then, it goes through all the jeBSPNode_Face's and merges them
//	till they can no longer merge (they remain convex).  Lastly, they are split up to prepare the face to have a 
//	lightmap.  During this entire process, these new jeBSPNode_Face's are maintained in the same list, but either
//	set their "merged" member, or the 2 "split" members in the face.  These new faces, will have the 
//	BSPFACE_MERGED_SPLIT flag set.  This simply means that these faces were a result from a merge or split.
//	
//	After the merge/split precess has been preformed, all the faces that don't have the "merged", or "split" member
//	set are then converted to a list of jeBSPNode_DrawFace's on the node the the face's portal was on.
//	The main reason that we keep this chain, is so leafs can find out what faces are looking into them.  
//	We just simply have to start on the portals, then go out to the jeBSPNode_Face's on that portal, then skip
//	all faces that have the "merged" or "split" member set, and we now have the list of faces that look into that leaf.
//
//	After everything is done with the face creation process, all the faces that were not originally created by portals
//	are destroyed (face with the BSPFACE_MERGED_SPLIT flag are destroyed).  The originals are kept on the portals.
//	
//	When a new brush is added, it is partitioned down to the leafs.  It then takes the leafs it lands in,
//	and paritions that leaf by the brush that landed in it.  It then takes all the portals that wre on that leaf,
//	and partitions them down to the new leafs.  BEFORE it partitions the portals though, it first takes the nodes
//	that the portals were on and destroys all drawfaces, then destroys all the jeBSPNode_Face's on all the portals
//	that bound the new leaf that is going to get cut up.
//
//		-John Pollard

#define	OUTSIDE_PADDING			128.0f		// Space between world box, and outside leaf

#define ADD_PORTAL_TO_NODES(p, n1, n2) do {  jeBSPNode_AddPortal(n1, p, 0); jeBSPNode_AddPortal(n2, p, 1);} while (0)

//#define NODE_USE_JE_RAM
int TotalNumberCollisions=0;

//=======================================================================================
//	jeBSPNode_Create
//=======================================================================================
jeBSPNode *jeBSPNode_Create(jeBSP *BSP)
{
	jeBSPNode		*Node;

#ifdef NODE_USE_JE_RAM
	Node = JE_RAM_ALLOCATE_STRUCT(jeBSPNode);
#else
	Node = (jeBSPNode *)jeArray_GetNewElement(BSP->NodeArray);
#endif
	if (!Node)
		return NULL;

	ZeroMem(Node);

	Node->PlaneIndex = JE_PLANEARRAY_NULL_INDEX;

	return Node;
}

//=======================================================================================
//	jeBSPNode_Destroy
//=======================================================================================
void jeBSPNode_Destroy_r(jeBSPNode **Node, jeBSP *BSP)
{
	jeBSPNode		*Node2;

	assert(Node);
	assert(jeBSPNode_IsValid(*Node) == JE_TRUE);

	Node2 = *Node;

	if (!(Node2->Flags & NODE_LEAF))
	{
		assert(!Node2->Leaf);
		
		jeBSPNode_Destroy_r(&Node2->Children[NODE_FRONT], BSP);
		jeBSPNode_Destroy_r(&Node2->Children[NODE_BACK], BSP);

		// Remove the reference to the plane that this node was using
		jePlaneArray_RemovePlane(BSP->PlaneArray, &Node2->PlaneIndex);

		// Destroy any draw faces on node
		jeBSPNode_DestroyDrawFaces(Node2, BSP);
	}
	else
	{
		assert(Node2->Leaf);
		assert(Node2->Leaf->Node);
		assert(Node2->Leaf->Node == Node2);
		assert(!Node2->DrawFaces);
		assert(!Node2->NumDrawFaces);
		assert(Node2->PlaneIndex == JE_PLANEARRAY_NULL_INDEX);
		jeBSPNode_LeafDestroy(&Node2->Leaf, BSP);
	}

	// Free all the portals on this node
	jeBSPNode_DestroyPortals(Node2, BSP);
	// Free all the faces
	jeBSPNode_DestroyBSPFaces(Node2, BSP, JE_FALSE);

#ifdef NODE_USE_JE_RAM
	jeRam_Free(*Node);
#else
	{
		jeBoolean	Ret;
		Ret = jeArray_FreeElement(BSP->NodeArray, *Node);
		assert(Ret == JE_TRUE);
	}
#endif

	*Node = NULL;
}

//=======================================================================================
//	jeBSPNode_IsValid
//=======================================================================================
jeBoolean jeBSPNode_IsValid(const jeBSPNode *Node)
{
	if (!Node)
		return JE_FALSE;

	if (Node->Leaf)
	{
		if (!(Node->Flags & NODE_LEAF))
		{
			jeErrorLog_AddString(-1, "jeBSPNode_IsValid:  !(Node->Flags & NODE_LEAF)", NULL);
			return JE_FALSE;
		}

		if (Node->Leaf->Node != Node)		// Make sure they still point to each other
		{
			jeErrorLog_AddString(-1, "jeBSPNode_IsValid:  Node->Leaf->Node != Node", NULL);
			return JE_FALSE;
		}

	#if 0	// Sometimes portals get clipped away before they recurse to the leafs.  
		if (!Node->Portals)					// Leafs should have portals
		{
			jeErrorLog_AddString(-1, "jeBSPNode_IsValid:  !Node->Portals", NULL);
			return JE_FALSE;
		}
	#endif
	}
	else
	{
		if (Node->Flags & NODE_LEAF)
		{
			jeErrorLog_AddString(-1, "jeBSPNode_IsValid:  Node->Flags & NODE_LEAF", NULL);
			return JE_FALSE;
		}

		if (Node->Portals)					// Nodes should NOT have portals
		{
			jeErrorLog_AddString(-1, "jeBSPNode_IsValid:  Node->Portals", NULL);
			return JE_FALSE;
		}
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_DestroyPortals_r
//=======================================================================================
void jeBSPNode_DestroyPortals_r(jeBSPNode *Node, jeBSP *BSP)
{
	assert(Node);

	if (!(Node->Flags & NODE_LEAF))		// Recurse to leafs
	{
		assert(!Node->Leaf);
		jeBSPNode_DestroyPortals_r(Node->Children[NODE_FRONT], BSP);
		jeBSPNode_DestroyPortals_r(Node->Children[NODE_BACK], BSP);

		return;
	}
	assert(Node->Leaf);

	// Destroy the portals only
	jeBSPNode_DestroyPortals(Node, BSP);
}

//=======================================================================================
//	jeBSPNode_DestroyBSPFaces_r
//=======================================================================================
void jeBSPNode_DestroyBSPFaces_r(jeBSPNode *Node, jeBSP *BSP, jeBoolean KeepOriginal)
{
	assert(Node);

	if (Node->Flags & NODE_LEAF)
	{
		assert(Node->Leaf);
		return;
	}

	assert(!Node->Leaf);

	jeBSPNode_DestroyBSPFaces_r(Node->Children[NODE_FRONT], BSP, KeepOriginal);
	jeBSPNode_DestroyBSPFaces_r(Node->Children[NODE_BACK], BSP, KeepOriginal);

	// Destroy the faces only
	jeBSPNode_DestroyBSPFaces(Node, BSP, KeepOriginal);
}

//=======================================================================================
//	jeBSPNode_DestroyBSPFaces
//	Destroys ALL BSP faces, unless KeepOriginal is set...
//=======================================================================================
void jeBSPNode_DestroyBSPFaces(jeBSPNode *Node, jeBSP *BSP, jeBoolean KeepOriginal)
{
	jeBSPNode_Face		*Face, *NextFace, *OriginalFaces;

	OriginalFaces = NULL;
	
	// Destroy node faces
	for (Face = Node->Faces; Face; Face = NextFace)
	{
		NextFace = Face->Next;

		if (!(Face->Flags & BSPFACE_MERGED_SPLIT) && KeepOriginal)
		{
			// This face no longer has a merge target or split target...
			Face->Split[0] = NULL;
			Face->Split[1] = NULL;
			Face->Merged = NULL;

			Face->Next = OriginalFaces;		// Make new list of original faces
			OriginalFaces = Face;
			continue;
		}
		
		jeBSPNode_FaceDestroy(&Face);
	}

	Node->Faces = OriginalFaces;
}

//=======================================================================================
//	PropogateFaceInfoToDrawFaces_r
//=======================================================================================
static jeBoolean PropogateFaceInfoToDrawFaces_r(jeBSPNode_Face *Face, jeBSP *BSP, jeBSP_TopSide *Side)
{
	assert(Face);
	assert(Side);
	assert(!Face->Merged);		// Faces should only be split once we get passed the merged chain

	if (Face->Split[0])
	{
		if (!PropogateFaceInfoToDrawFaces_r(Face->Split[0], BSP, Side))
			return JE_FALSE;
		if (!PropogateFaceInfoToDrawFaces_r(Face->Split[1], BSP, Side))
			return JE_FALSE;

		return JE_TRUE;
	}

	if (Face->DrawFace)
	{
		jeBSPNode_DrawFace	*DFace = Face->DrawFace;

		DFace->TopSideFlags = Side->TopSideFlags;
		
		jeBSPNode_DrawFaceSetFaceInfoIndex(DFace, BSP, Side->FaceInfoIndex);

		// We need to get smarter about when we destroy the faces lightmap
		//	We should only destroy the lightmap if it depends on data that just changed in faceinfo...
		if (DFace->Lightmap)
			jeBSPNode_LightmapDestroy(&DFace->Lightmap, BSP);
	}

	return JE_TRUE;
}

//=======================================================================================
//	PropogateFaceInfoToDrawFaces
//=======================================================================================
static jeBoolean PropogateFaceInfoToDrawFaces(jeBSPNode_Face *Face, jeBSP *BSP, jeBSP_TopSide *Side)
{
	assert(Face);
	assert(Side);

	while (Face->Merged)
	{
		if (Face->Portal->Side->jeBrushFace != Face->Merged->Portal->Side->jeBrushFace)
			return JE_FALSE;	// Can't propogate, face merged accros multiple brush faces
	}

	if (!PropogateFaceInfoToDrawFaces_r(Face, BSP, Side))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_DiryNodes_r
//=======================================================================================
jeBoolean jeBSPNode_DirtyNodes_r(jeBSPNode *Node, jeBSP *BSP, jeBSP_TopSide *Side)
{
	int32				PSide;
	jeBSPNode_Portal	*Portal;

	assert(jeBSPNode_IsValid(Node) == JE_TRUE);
	assert(Side);

	if (!Node->Leaf)		// Recurse to leafs
	{
		if (!jeBSPNode_DirtyNodes_r(Node->Children[NODE_FRONT], BSP, Side))
			return JE_FALSE;
		if (!jeBSPNode_DirtyNodes_r(Node->Children[NODE_BACK], BSP, Side))
			return JE_FALSE;

		return JE_TRUE;
	}

	// At leaf, find portals that use this side
	for (Portal = Node->Portals; Portal; Portal = Portal->Next[PSide])
	{
		PSide = (Portal->Nodes[1] == Node);
		
		if (Portal->Side == Side)
		{
			assert(Portal->OnNode);

			#if 1
				Portal->OnNode->Flags |= NODE_REBUILD_FACES;		// Flag node to rebuild face
			#else
			{
				int32		i;

				// First, try to propogate this info to the drawfaces
				//	If DrawFaces merged across multiple brush faces, then we must rebuild the node faces
				for (i=0; i<2; i++)
				{
					if (!Portal->Face[i])
						continue;

					if (!PropogateFaceInfoToDrawFaces(Portal->Face[i], BSP, Side))
					{
						Portal->OnNode->Flags |= NODE_REBUILD_FACES;		// Flag node to rebuild face
						break;
					}
					// At least one face got propogated, so update lights on node
					Portal->OnNode->Flags |= NODE_UPDATELIGHTS;
				}
			}
			#endif
		}

	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_PropogateTopSideFlags
//=======================================================================================
jeBoolean jeBSPNode_PropogateTopSideFlags_r(jeBSPNode *Node, jeBSP *BSP, jeBSP_TopSide *Side, int32 *NumPropogated)
{
	int32		i;

	assert(jeBSPNode_IsValid(Node) == JE_TRUE);
	assert(Side);

	if (Node->Leaf)
		return JE_TRUE;

	for (i=0; i< Node->NumDrawFaces; i++)
	{
		jeBSPNode_DrawFace		*DFace;

		DFace = Node->DrawFaces[i];

		if (DFace->jeBrushFace == Side->jeBrushFace)
		{
			(*NumPropogated)++;
			DFace->TopSideFlags = Side->TopSideFlags;
		}
	}

	if (!jeBSPNode_PropogateTopSideFlags_r(Node->Children[NODE_FRONT], BSP, Side, NumPropogated))
		return JE_FALSE;
	if (!jeBSPNode_PropogateTopSideFlags_r(Node->Children[NODE_BACK], BSP, Side, NumPropogated))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_DestroyDrawFaces
//=======================================================================================
void jeBSPNode_DestroyDrawFaces(jeBSPNode *Node, jeBSP *BSP)
{
	int32				i;

	assert(Node);
	assert(!(Node->Flags & NODE_LEAF));
	assert(!Node->Leaf);

	if (!Node->DrawFaces)
	{
		assert(Node->NumDrawFaces == 0);
		return;
	}

	for (i=0; i<Node->NumDrawFaces; i++)
	{
		jeBSPNode_DrawFace	*DFace;
		
		DFace = Node->DrawFaces[i];

		assert(DFace->Poly);		// DFace is bad if no poly
		jeBSPNode_DrawFaceDestroy(&DFace, BSP);
	}

	jeRam_Free(Node->DrawFaces);

	Node->DrawFaces = NULL;
	Node->NumDrawFaces = 0;
}

//=======================================================================================
//	jeBSPNode_DestroyPortals
//=======================================================================================
void jeBSPNode_DestroyPortals(jeBSPNode *Node, jeBSP *BSP)
{
	jeBSPNode_Portal	*Portal, *NextPortal;
	int32				Side;

	assert(Node);

	// Free all the portals on this node
	for (Portal = Node->Portals; Portal; Portal = NextPortal)
	{
		assert(!(Portal->Nodes[0] == Node && Portal->Nodes[1] == Node));	// Should be on one side or the other (not both)
		assert(Portal->Nodes[0] == Node || Portal->Nodes[1] == Node);		// Should be on at least one side

		Side = (Portal->Nodes[1] == Node);

		NextPortal = Portal->Next[Side];

		// Remove the portal from BOTH nodes that it seperates...
		jeBSPNode_RemovePortal(Portal->Nodes[0], Portal);
		jeBSPNode_RemovePortal(Portal->Nodes[1], Portal);

		jeBSPNode_PortalDestroy(&Portal, BSP);
	}

	assert(Node->Portals == NULL);		// Above code SHOULD have removed all portals
}

//=======================================================================================
//	jeBSPNode_InitializeRootPortals
//	Create portals on nodes box, and points front side of portals to node, back side to OutsideNode
//=======================================================================================
jeBSPNode *jeBSPNode_InitializeRootPortals(jeBSPNode *Node, jeBSP *BSP)
{
	int32				k, i, Index;
	jePlane				Planes[6];
	jeBSPNode_Portal	*Portals[6];
	jeVec3d				Mins, Maxs;
	jeBSPNode			*OutsideNode;

	OutsideNode = jeBSPNode_Create(BSP);

	if (!OutsideNode)
		return NULL;

	OutsideNode->Leaf = jeBSPNode_LeafCreate(BSP);

	if (!OutsideNode->Leaf)
		goto ExitWithError;

	// The outside node
	OutsideNode->Flags = NODE_LEAF | NODE_OUTSIDE;
	OutsideNode->Leaf->Contents = JE_BSP_CONTENTS_SOLID;
	OutsideNode->Leaf->Node = OutsideNode;

	memset(Planes, 0, 6*sizeof(jePlane));
	memset(Portals, 0, 6*sizeof(jeBSPNode_Portal*));

	// Get extents of node
	assert(jeExtBox_IsValid(&Node->Box));

	Mins = Node->Box.Min;
	Maxs = Node->Box.Max;

#if 0
	// Get the box of the node, and expand it a little
	for (k=0; k< 3; k++)
	{
		float	Min, Max;

		Min = jeVec3d_GetElement(&Mins, k) - OUTSIDE_PADDING;
		Max = jeVec3d_GetElement(&Maxs, k) + OUTSIDE_PADDING;

		if (Min <= -JE_BSP_MINMAX_BOUNDS)
			goto ExitWithError;	// Not enough room to expand
		if (Max >= JE_BSP_MINMAX_BOUNDS)
			goto ExitWithError;	// Not enough room to expand

		jeVec3d_SetElement(&Mins, k, Min);
		jeVec3d_SetElement(&Maxs, k, Max);
	}
#else
	Mins.X = (-JE_BSP_MINMAX_BOUNDS)+1.0f;
	Mins.Y = (-JE_BSP_MINMAX_BOUNDS)+1.0f;
	Mins.Z = (-JE_BSP_MINMAX_BOUNDS)+1.0f;
	
	Maxs.X =  JE_BSP_MINMAX_BOUNDS-1.0f;
	Maxs.Y =  JE_BSP_MINMAX_BOUNDS-1.0f;
	Maxs.Z =  JE_BSP_MINMAX_BOUNDS-1.0f;
#endif
	// Create 6 portals on this box, and point to the outsidenode and the Node
	for (i=0; i<3; i++)
	{
		for (k=0; k<2; k++)
		{
			jePoly				*Poly;
			jeBSPNode_Portal	*Portal;

			Index = k*3 + i;

			jeVec3d_Clear(&Planes[Index].Normal);

			if (k == 0)
			{
				jeVec3d_SetElement(&Planes[Index].Normal, i, 1.0f);
				Planes[Index].Dist = jeVec3d_GetElement(&Mins, i);
			}
			else
			{
				jeVec3d_SetElement(&Planes[Index].Normal, i, -1.0f);
				Planes[Index].Dist = -jeVec3d_GetElement(&Maxs, i);
			}
			
			Planes[Index].Type = Type_Any;	// Not found yet

			Poly = jePoly_CreateFromPlane(&Planes[Index], JE_BSP_MINMAX_BOUNDS);

			if (!Poly)
				goto ExitWithError;

			Portal = jeBSPNode_PortalCreate(Poly, BSP);

			if (!Portal)
				goto ExitWithError;
			
			Portal->PlaneIndex = jePlaneArray_SharePlane(BSP->PlaneArray, &Planes[Index]);

			if (Portal->PlaneIndex == JE_PLANEARRAY_NULL_INDEX)
				goto ExitWithError;

			if (!jePlaneArray_IndexSided(Portal->PlaneIndex))
				ADD_PORTAL_TO_NODES(Portal, Node, OutsideNode);
			else
				ADD_PORTAL_TO_NODES(Portal, OutsideNode, Node);

			Portals[Index] = Portal;
		}
	}
							  
	// Clip the portals against each other, to form a perfect skin of the box
	for (i=0; i< 6; i++)
	{
		for (k=0; k< 6; k++)
		{
			if (k == i)
				continue;

			if (!jePoly_ClipEpsilon(&Portals[i]->Poly, JE_BSP_PLANESIDE_EPSILON, &Planes[k], JE_FALSE))
			{
				jeErrorLog_AddString(-1, "jeBSPNode_InitializeRootPortals:  jePoly_ClipEpsilon failed.", NULL);
				goto ExitWithError;
			}

			if (!Portals[i]->Poly)
			{
				jeErrorLog_AddString(-1, "jeBSPNode_InitializeRootPortals:  Portal was clipped away.", NULL);
				goto ExitWithError;
			}
		}
	}

	return OutsideNode;

	ExitWithError:
	{
		if (OutsideNode)	// Don't destroy leaf, node will do that for us...
			jeBSPNode_Destroy_r(&OutsideNode, BSP);

		for (i=0; i<6; i++)
			if (Portals[i])
				jeBSPNode_PortalDestroy(&Portals[i], BSP);

		return NULL;
	}
}

//=======================================================================================
//	jeBSPNode_CalcBoundsFromPortals
//	Calcs bounds for nodes, and leafs
//=======================================================================================
void jeBSPNode_CalcBoundsFromPortals(jeBSPNode *Node)
{
	jeBSPNode_Portal	*p;
	jeBoolean			Set;
	int32				s, i;

	assert(Node);

	Set = JE_FALSE;

	for (p=Node->Portals; p; p = p->Next[s])
	{
		s = (p->Nodes[1] == Node);

		for (i=0; i<p->Poly->NumVerts; i++)
		{
			if (!Set)
				jeExtBox_SetToPoint(&Node->Box, &p->Poly->Verts[i]);
			else
				jeExtBox_ExtendToEnclose(&Node->Box, &p->Poly->Verts[i]);

			Set = JE_TRUE;
		}
	}
}

//=======================================================================================
//	jeBSPNode_PartitionPortals_r
//	Calcs the bounds for node by taking bounds of the current portal set
//	Then takes the portals on a node, and distributes them to the nodes children
//=======================================================================================
jeBoolean jeBSPNode_PartitionPortals_r(jeBSPNode *Node, jeBSP *BSP, jeBoolean IncludeDetail)
{
	jeBSPNode_CalcBoundsFromPortals(Node);	// Calcs bounds for nodes and leafs
	
	if (Node->Flags & NODE_LEAF)			// At leaf, no more recursing
		return JE_TRUE;

	if (!IncludeDetail && (Node->Flags & NODE_DETAIL))	// Stop at detail, if told to do so
		return JE_TRUE;

	// Initialize the portal on this node
	if (!jeBSPNode_InitializePortal(Node, BSP))
		return JE_FALSE;

	// Distribute the portals to this nodes children
	if (!jeBSPNode_DistributePortalsToChildren(Node, BSP))
		return JE_FALSE;

	// Take the portals on the 2 children, and ditribute those as well
	if (!jeBSPNode_PartitionPortals_r(Node->Children[NODE_FRONT], BSP, IncludeDetail))
		return JE_FALSE;

	if (!jeBSPNode_PartitionPortals_r(Node->Children[NODE_BACK], BSP, IncludeDetail))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_CreatePoly
//	Creates a huge poly on the node, and clips it against all the nodes parents...
//	Poly can come back NULL, and not be an error.  The poly just got clipped away...
//=======================================================================================
jeBoolean jeBSPNode_CreatePoly(const jeBSPNode *Node, jeBSP *BSP, jePoly **PolyOut)
{
	jePoly			*Poly;
	const jePlane	*Plane;
	jeBSPNode		*n;

	Plane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
	
	Poly = jePoly_CreateFromPlane(Plane, JE_BSP_MINMAX_BOUNDS);

	if (!Poly)
		return JE_FALSE;

	// Clip this poly by all the parents of this node
	for (n = Node->Parent ; n && Poly ; )
	{
		jeBoolean	Side;

		Plane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, n->PlaneIndex);

		Side = (n->Children[0] == Node) ? JE_FALSE : JE_TRUE;
		
		if (!jePoly_ClipEpsilon(&Poly, 0.001f, Plane, Side))
			return JE_FALSE;

		Node = n;
		n = n->Parent;
	}

	*PolyOut = Poly;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_InitializePortal
//	Create a portal on the node.  Clips it by all nodes parents, and current portals.
//	Then points the portal to the nodes 2 children
//=======================================================================================
jeBoolean jeBSPNode_InitializePortal(jeBSPNode *Node, jeBSP *BSP)
{
	jePoly				*Poly;
	jeBSPNode_Portal	*Portal;
	int32				Side;

	// Create a new portal
	if (!jeBSPNode_CreatePoly(Node, BSP, &Poly))
		return JE_FALSE;

	// Clip it against all other portals attached to this node
	for (Portal = Node->Portals; Portal && Poly; Portal = Portal->Next[Side])
	{
		const jePlane	*pPlane;

		assert(Portal->Nodes[0] == Node || Portal->Nodes[1] == Node);
		assert(!(Portal->Nodes[0] == Node && Portal->Nodes[1] == Node));
		
		Side = (Portal->Nodes[1] == Node);

		pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Portal->PlaneIndex);

		if (!jePoly_ClipEpsilon(&Poly, 0.001f, pPlane, Side))
			return JE_FALSE;

		if (!Poly)
		{
			break;
		}
	}
	
	if (Poly && jePoly_IsTiny(Poly))
		jePoly_Destroy(&Poly);

	if (Poly)
	{
		Portal = jeBSPNode_PortalCreate(Poly, BSP);

		if (!Portal)
			return JE_FALSE;

		Portal->PlaneIndex = Node->PlaneIndex;
		Portal->OnNode = Node;

		if (!jeBSPNode_PortalIsValid(Portal))
		{
			jeErrorLog_AddString(-1, "jeBSPNode_InitializePortal:  jeBSPNode_PortalIsValidGeometry failed.\n", NULL);
			return JE_FALSE;
		}
		
		// Make the portal look at nodes children
		ADD_PORTAL_TO_NODES(Portal, Node->Children[0], Node->Children[1]);
	}
	else
	{
		Log_Printf("jeBSPNode_InitializePortal:  Portal was cut away.\n");
	}


	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_DistributePortalsToChildren
//	Take all the portals that look at this node, and distributes to the nodes children
//=======================================================================================
jeBoolean jeBSPNode_DistributePortalsToChildren(jeBSPNode *Node, jeBSP *BSP)
{
	jeBSPNode_Portal	*Portal, *Next;
	const jePlane		*pPlane;
	jeBSPNode			*Front, *Back;
	
	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
	assert(pPlane);

	Front = Node->Children[0];
	Back = Node->Children[1];

	// Partition all portals by this node
	for (Portal = Node->Portals; Portal; Portal = Next)
	{
		int32				Side;
		jePoly				*FPoly, *BPoly;
		jeBSPNode_Portal	*NewPortal;
		jeBSPNode			*OppositeNode;

		assert(Portal->Nodes[0] == Node || Portal->Nodes[1] == Node);
		assert(!(Portal->Nodes[0] == Node && Portal->Nodes[1] == Node));
		
		Side = (Portal->Nodes[1] == Node);

		Next = Portal->Next[Side];
		
		// Remember the node on the opposite side
		OppositeNode = Portal->Nodes[!Side];

		// Remove both nodes from this portal, split it, then add the split portals back to the nodes
		// that they truly look at
		jeBSPNode_RemovePortal(Portal->Nodes[0], Portal);
		jeBSPNode_RemovePortal(Portal->Nodes[1], Portal);

		if (!jePoly_SplitEpsilon(&Portal->Poly, 0.001f, pPlane, JE_FALSE, &FPoly, &BPoly))
		{
			jeErrorLog_AddString(-1, "jeBSPNode_DistributePortalsToChildren:  jePoly_SplitEpsilon failed.\n", NULL);
			return JE_FALSE;
		}
		
		if (FPoly && jePoly_IsTiny(FPoly))
			jePoly_Destroy(&FPoly);

		if (BPoly && jePoly_IsTiny(BPoly))
			jePoly_Destroy(&BPoly);
		
		if (!FPoly && !BPoly)			// Both tiny, or clipped away
			continue;
		
		if (!FPoly)						// On back side
		{
			Portal->Poly = BPoly;
			if (Side)
				ADD_PORTAL_TO_NODES(Portal, OppositeNode, Back);
			else
				ADD_PORTAL_TO_NODES(Portal, Back, OppositeNode);
			continue;
		}

		if (!BPoly)						// On front side
		{
			Portal->Poly = FPoly;
			if (Side)
				ADD_PORTAL_TO_NODES(Portal, OppositeNode, Front);
			else
				ADD_PORTAL_TO_NODES(Portal, Front, OppositeNode);
			continue;
		}

		// Portal was split
		NewPortal = jeBSPNode_PortalCreate(BPoly, BSP);
		
		if (!NewPortal)
			return JE_FALSE;

		*NewPortal = *Portal;
		NewPortal->Poly = BPoly;

		Portal->Poly = FPoly;
		
		if (Side)
		{
			ADD_PORTAL_TO_NODES(Portal, OppositeNode, Front);
			ADD_PORTAL_TO_NODES(NewPortal, OppositeNode, Back);
		}
		else
		{
			ADD_PORTAL_TO_NODES(Portal, Front, OppositeNode);
			ADD_PORTAL_TO_NODES(NewPortal, Back, OppositeNode);
		}
	}

	assert(Node->Portals == NULL);	// All portals SHOULD have been removed, and distributed to the nodes children!!!

	return JE_TRUE;
}

//=====================================================================================
//	jeBSPNode_AddPortal
//=====================================================================================
void jeBSPNode_AddPortal(jeBSPNode *Node, jeBSPNode_Portal *Portal, int32 Side)
{
	assert(!Portal->Nodes[Side]);

	Portal->Nodes[Side] = Node;
	Portal->Next[Side] = Node->Portals;
	Node->Portals = Portal;
}

//=====================================================================================
//	jeBSPNode_RemovePortal
//	Finds a portal on the node, and removes it from the list of portals on the node
//=====================================================================================
void jeBSPNode_RemovePortal(jeBSPNode *Node, jeBSPNode_Portal *Portal)
{
	int32				Side;
	jeBSPNode_Portal	*p, **p2;
	
	assert(Node->Portals);		// Better have some portals on this node
	assert(!(Portal->Nodes[0] == Node && Portal->Nodes[1] == Node));
	assert(Portal->Nodes[0] == Node || Portal->Nodes[1] == Node);	

	jeBSPNode_PortalResetTopSide(Portal);

	// Find the portal on this node
	for (p2 = &Node->Portals, p = *p2; p; p2 = &p->Next[Side], p = *p2)
	{
		assert(!(p->Nodes[0] == Node && p->Nodes[1] == Node));
		assert(p->Nodes[0] == Node || p->Nodes[1] == Node);

		Side = (p->Nodes[1] == Node);	// Get the side of the portal that this node is on

		if (p == Portal)
			break;			// Got it
	}
	 
	assert(p && p2 && *p2);

	Side = (Portal->Nodes[1] == Node);	// Get the side of the portal that the node was on
	
	*p2 = Portal->Next[Side];
	Portal->Nodes[Side] = NULL;
}


//=======================================================================================
//	jeBSPNode_BuildContents
//=======================================================================================
jeBoolean jeBSPNode_BuildContents(jeBSPNode *Node, jeBSP *BSP)
{
	jeBSPNode_Leaf	*Leaf;
	jeBSP_Brush		*Brush;
	uint32			BestOrder;

	assert(jeBSPNode_IsValid(Node) == JE_TRUE);

	Leaf = Node->Leaf;

	// Reset the leaf contents
	Leaf->BrushContents = 0;
	Leaf->Contents = BSP->DefaultContents;

	BestOrder = 0;

	// Find the highest cut order
	for (Brush = Leaf->Brushes; Brush; Brush = Brush->Next)
	{
		jeBSP_TopBrush		*TopBrush;

		TopBrush = Brush->Original;

		if (TopBrush->Contents & JE_BSP_CONTENTS_AIR)
		{
			if (TopBrush->Order >= BestOrder)			// Remember the cut brush with the highest order
				BestOrder = TopBrush->Order;

			Leaf->Contents &= ~JE_BSP_CONTENTS_SOLID;	// Since there is a cut, remove all solid from default contents
		}
	}

	// "or" together all the contents of the brushes that make up this leaf
	for (Brush = Leaf->Brushes; Brush; Brush = Brush->Next)
	{
		jeBSP_TopBrush		*TopBrush;
		uint32				Contents;

		TopBrush = Brush->Original;
		Contents = TopBrush->Contents;

		// Only keep contents if it is higher than the highest cut contents
		if (TopBrush->Order >= BestOrder || (Contents & JE_BSP_CONTENTS_EMPTY))
		{
			Leaf->Contents |= Contents;
			Leaf->BrushContents |= Contents;
		}
	}

	// Solid overrides everything
	if ((Leaf->BrushContents|Leaf->Contents) & JE_BSP_CONTENTS_SOLID)
		Leaf->Contents = JE_BSP_CONTENTS_SOLID;

	return JE_TRUE;
}

int32	NumAirLeafs = 0;
int32	NumSolidLeafs = 0;

//=======================================================================================
//	jeBSPNode_InitializeLeaf
//	Converts a node into a leaf, and create the contents
//=======================================================================================
jeBoolean jeBSPNode_InitializeLeaf(jeBSPNode *Node, jeBSP *BSP, jeBSP_Brush *Brushes)
{
	jeBSPNode_Leaf	*Leaf;

	assert(!Node->Leaf);

	Leaf = jeBSPNode_LeafCreate(BSP);

	if (!Leaf)
		return JE_FALSE;

	Node->Flags = NODE_LEAF;	// Node is a leaf
	Node->PlaneIndex = JE_PLANEARRAY_NULL_INDEX;
	Node->Side = NULL;

	Node->Leaf = Leaf;
	Leaf->Node = Node;
	Leaf->Brushes = Brushes;		// Remember the list

	// Build the contents of this leaf
	jeBSPNode_BuildContents(Node, BSP);

	if (Leaf->Contents & JE_BSP_CONTENTS_AIR)
		NumAirLeafs++;
	if (Leaf->Contents & JE_BSP_CONTENTS_SOLID)
		NumSolidLeafs++;

#if 0
	// Create the leaf sides from the first brush
	if (!jeBSPNode_LeafInitializeSides(Leaf))
	{
		jeBSPNode_LeafDestroy(&Node->Leaf);
		return JE_FALSE;
	}
#endif

	// Free up some memory
	{
		jeBSP_Brush	*Brush;
		int32		i;

		// Don't need to keep polygons anymore...
		for (Brush = Brushes; Brush; Brush = Brush->Next)
		{
			for (i=0; i< Brush->NumSides; i++)
			{
				if (Brush->Sides[i].Poly)
					jePoly_Destroy(&Brush->Sides[i].Poly);
			}

			Brush->Flags &= ~BSPBRUSH_FORCEBOTH;		// Remove FORCEBOTH flag...
		}
	}

	return JE_TRUE;
}

static int32 NumMergedLeafs;

//=======================================================================================
//	jeBSPNode_MergeLeafs_r
//=======================================================================================
void jeBSPNode_MergeLeafs_r(jeBSPNode *Node, jeBSP *BSP)
{
	jeBSP_Brush		*b, *Next;
	jeBSPNode_Leaf	*lf, *lb;

	if (Node->Flags & NODE_LEAF)		
	{
		assert(Node->Leaf);
		return;
	}
	
	assert(!Node->Leaf);

	// Recurse to leafs, then start merging back up the stack...
	jeBSPNode_MergeLeafs_r(Node->Children[0], BSP);
	jeBSPNode_MergeLeafs_r(Node->Children[1], BSP);

	lf = Node->Children[0]->Leaf;
	lb = Node->Children[1]->Leaf;

	// If there are leafs on both sides, and...
	// If contents are the same on both sides (or solid on both), nodes can be merged
	if ((lf && lb) && ((lf->Contents == lb->Contents) || 
		((lf->Contents & JE_BSP_CONTENTS_SOLID) && (lb->Contents & JE_BSP_CONTENTS_SOLID))) )
	{
		jeBSPNode_Leaf	*NewLeaf;			

		assert(!Node->Faces);				// Same contents should NOT have faces
		assert(!Node->Children[0]->Faces);
		assert(!Node->Children[1]->Faces);

		// Create a new leaf
		NewLeaf = jeBSPNode_LeafCreate(BSP);

		// Remove the reference to this plane the node was using
		jePlaneArray_RemovePlane(BSP->PlaneArray, &Node->PlaneIndex);

		Node->Flags = NODE_LEAF;
		Node->Leaf = NewLeaf;			

		NewLeaf->Node = Node;
		NewLeaf->Contents = lb->Contents;
		NewLeaf->Brushes = lb->Brushes;

		// Combine front brushlist with back
		for (b=lf->Brushes ; b ; b=Next)
		{
			Next = b->Next;
			b->Next = NewLeaf->Brushes;
			NewLeaf->Brushes = b;
		}

		// front back leaf no longer contain brushes, since this new leaf took them
		lf->Brushes = NULL;
		lb->Brushes = NULL;

		// Destroy the nodes children
		jeBSPNode_Destroy_r(&Node->Children[0], BSP);
		jeBSPNode_Destroy_r(&Node->Children[1], BSP);

		NumMergedLeafs++;
	}
}

//=======================================================================================
//	jeBSPNode_MergeLeafs
//=======================================================================================
void jeBSPNode_MergeLeafs(jeBSPNode *Node, jeBSP *BSP)
{
	NumMergedLeafs = 0;

	Log_Printf("--- jeBSPNode_MergeLeafs ---\n");

	jeBSPNode_MergeLeafs_r(Node, BSP);

	Log_Printf("Num Merged Leafs       : %5i\n", NumMergedLeafs);
}

//=====================================================================================
//	jeBSPNode_FindLeaf
//=====================================================================================
jeBSPNode_Leaf *jeBSPNode_FindLeaf(const jeBSPNode *Node, jeBSP *BSP, const jeVec3d *Pos)
{
	assert(jeBSPNode_IsValid(Node) == JE_TRUE);
	assert(Pos);

	while(!(Node->Flags & NODE_LEAF))		// Go to leafs
	{
		const jePlane	*Plane;
		jeFloat			Dist;

		Plane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
		
		Dist = jePlane_PointDistanceFast(Plane, Pos);	// We can use fast check, since node planes face positive

		if (Dist > 0)
			Node = Node->Children[0];
		else
			Node = Node->Children[1];
	}

	return Node->Leaf;
}

int32 NumFilledLeafs;

//=====================================================================================
//	jeBSPNode_FillUnTouchedLeafs_r
//=====================================================================================
void jeBSPNode_FillUnTouchedLeafs_r(jeBSPNode *Node, int32 Fill)
{
	if (!(Node->Flags & NODE_LEAF))		// Recurse to leafs
	{
		assert(!Node->Leaf);
		jeBSPNode_FillUnTouchedLeafs_r(Node->Children[0], Fill);
		jeBSPNode_FillUnTouchedLeafs_r(Node->Children[1], Fill);
		return;
	}
	
	assert(Node->Leaf);

	if ((Node->Leaf->Contents & JE_BSP_CONTENTS_SOLID))
		return;		// Already solid or removed...

	if (Node->Leaf->CurrentFrame != Fill)
	{
		// Fill er in with solid so it does not show up...(Preserve user contents)
		Node->Leaf->Contents &= (0xffff0000);
		Node->Leaf->Contents |= JE_BSP_CONTENTS_SOLID;
		NumFilledLeafs++;
	}
}

int32 NumMakeFaces;
int32 NumMergedFaces;
int32 NumSubdividedFaces;

//=======================================================================================
//	jeBSPNode_SubdivideFaces
//=======================================================================================
jeBoolean jeBSPNode_SubdivideFaces(jeBSPNode *Node, jeBSP *BSP)
{
	jeBSPNode_Face	*Face;
	int32			NumSubdivided;

	NumSubdivided = 0;

	for (Face = Node->Faces; Face; Face = Face->Next)
	{
		if (!jeBSPNode_FaceSubdivide(Face, BSP, Node, 230.0f, &NumSubdivided))
			return JE_FALSE;
	}

	NumSubdividedFaces += NumSubdivided;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_MakeFaces
//=======================================================================================
jeBoolean jeBSPNode_MakeFaces(jeBSPNode *Node, jeBSP *BSP, jeBoolean FreeMergedSplit)
{
	if (!(Node->Flags & NODE_REBUILD_FACES))
		return JE_TRUE;		// Node is not marked to rebuild any faces
		
	// Destroy all but the original faces created off the portals on this node
	jeBSPNode_DestroyBSPFaces(Node, BSP, JE_TRUE);

	// Destroy any previous DrawFaces on the node
	jeBSPNode_DestroyDrawFaces(Node, BSP);

	// Merge list
	if (Node->Faces)
	{
		int32		NumMerged = 0;

		#if 1
		{
			// Merge faces 
			if (!jeBSPNode_FaceMergeList(Node->Faces, BSP, &NumMerged))
				return JE_FALSE;
			}
		#endif

		NumMergedFaces += NumMerged;

		// Subdivide them up for lightmaps... 
		if (!jeBSPNode_SubdivideFaces(Node, BSP))
			return JE_FALSE;
	}

	// Make the new DrawFaces on this node from the final BSP faces
	if (!jeBSPNode_MakeDrawFaces(Node, BSP))
		return JE_FALSE;

	if (FreeMergedSplit)
	{
		// Keep only originals
		jeBSPNode_DestroyBSPFaces(Node, BSP, JE_TRUE);
	}

	Node->Flags &= ~NODE_REBUILD_FACES;		// This node is up to date with the faces
	Node->Flags |= NODE_UPDATELIGHTS;		// This node needs light

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_MakeFaces_r
//	Examines all portals on leafs, and checks to see if any portal needs a face...
//=======================================================================================
jeBoolean jeBSPNode_MakeFaces_r(jeBSPNode *Node, jeBSP *BSP, jeBoolean FreeMergedSplit)
{
	jeBSPNode_Portal	*p;
	int32				s;

	// Recurse down to leafs
	if (!(Node->Flags & NODE_LEAF))
	{
		assert(!Node->Leaf);
		assert(!Node->Portals);		// At this point, portals should be at leafs ONLY

		if (!jeBSPNode_MakeFaces_r(Node->Children[NODE_FRONT], BSP, FreeMergedSplit))
			return JE_FALSE;

		if (!jeBSPNode_MakeFaces_r(Node->Children[NODE_BACK], BSP, FreeMergedSplit))
			return JE_FALSE;
		
		if (!jeBSPNode_MakeFaces(Node, BSP, FreeMergedSplit))
			return JE_FALSE;

		return JE_TRUE;
	}
	assert(Node->Leaf);

	// Solid leafs never have visible faces
	if (Node->Leaf->Contents & JE_BSP_CONTENTS_SOLID)
		return JE_TRUE;

	// See which portals are valid, and need faces
	// If any portals needs faces, create the face on the side of the portal that
	// looks into this leaf...
	for (p = Node->Portals; p ; p = p->Next[s])
	{
		s = (p->Nodes[1] == Node);

		if (!p->Side)		// Portal does not seperate visible contents
		{
			assert(!p->Face[0] && !p->Face[1]);	// portal should have NO faces in this case
			continue;
		}

		if (p->Face[s])		// Portal already has a face that looks into this leaf
		{
			// Keep topsideflags up to date
			p->Face[s]->TopSideFlags = p->Side->TopSideFlags;
			continue;
		}

		// Create a face on the portal that looks into this leaf
		p->Face[s] = jeBSPNode_FaceCreateFromPortal(p, s);

		if (!p->Face[s])
			return JE_FALSE;
		
		// Record the contents that the face looks into (this leaf)
		p->Face[s]->Contents = Node->Leaf->Contents;			// Front side contents is this leaf

		// Add the face to the list of faces on the node that originaly created the portal
		p->Face[s]->Next = p->OnNode->Faces;
		p->OnNode->Faces = p->Face[s];

		// When we recurse back up the stack, nodes need to know if they need
		// an update.  IOW, we don't want to try to merge the list, if NO new faces were
		// created on the node...
		p->OnNode->Flags |= NODE_REBUILD_FACES;		// Flag the node that faces need to be rebuilt

		NumMakeFaces++;
	}

	return JE_TRUE;
}


//=======================================================================================
//	jeBSPNode_MakeFaces_Callr
//=======================================================================================
jeBoolean jeBSPNode_MakeFaces_Callr(jeBSPNode *Node, jeBSP *BSP, jeBoolean FreeMergedSplit)
{
	Log_Printf("--- jeBSPNode_MakeFaces	---\n");

	NumMergedFaces = 0;
	NumSubdividedFaces = 0;
	NumMakeFaces = 0;

	if (!jeBSPNode_MakeFaces_r(Node, BSP, FreeMergedSplit))
		return JE_FALSE;

	Log_Printf("TotalFaces             : %5i\n", NumMakeFaces);
	Log_Printf("Merged Faces           : %5i\n", NumMergedFaces);
	Log_Printf("Subdivided Faces       : %5i\n", NumSubdividedFaces);
	Log_Printf("FinalFaces             : %5i\n", (NumMakeFaces-NumMergedFaces)+NumSubdividedFaces);
	
	BSP->DebugInfo.NumMakeFaces = NumMakeFaces;
	BSP->DebugInfo.NumMergedFaces = NumMergedFaces;
	BSP->DebugInfo.NumSubdividedDrawFaces = NumSubdividedFaces;
	BSP->DebugInfo.NumDrawFaces = (NumMakeFaces-NumMergedFaces)+NumSubdividedFaces;

	return JE_TRUE;
}

jeVec3d VecTable[6] = { 
	{-1.0f, 0.0f, 0.0f, 0.0f},		// Left Wall
	{ 1.0f, 0.0f, 0.0f, 0.0f},		// Right Wall
	{ 0.0f, 0.0f,-1.0f, 0.0f},		// Back Wall
	{ 0.0f, 0.0f, 1.0f, 0.0f},		// Front Wall
	{ 0.0f, 1.0f, 0.0f, 0.0f},		// Ceiling
	{ 0.0f,-1.0f, 0.0f, 0.0f}		// Floor
};

//=======================================================================================
//	BuildDrawFaceXForm
//=======================================================================================
static jeBoolean BuildDrawFaceXForm(jeBSPNode_DrawFace *DFace, jeBSP *BSP)
{
	jeFloat		BestDot;
	int32		i, BestIndex;
	jeVec3d		Left, Up, In;
	jePlane		Plane;

	assert(DFace);
	assert(DFace->PortalXForm);
	assert(DFace->jeBrushFace);

	BestDot = -1.0f;
	BestIndex = -1;

	Plane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, DFace->PlaneIndex);

	if (!jePlaneArray_IndexSided(DFace->PlaneIndex))
		jePlane_Inverse(&Plane);

	for (i=0; i<6; i++)
	{
		jeFloat		Dot;

		Dot = jeVec3d_DotProduct(&Plane.Normal, &VecTable[i]);

		if (Dot > BestDot)
		{
			BestDot = Dot;
			BestIndex = i;
		}

	}

	switch (BestIndex)
	{
		case 0:				// Left Wall
			jeVec3d_Set(&Left, 0.0f, 0.0f, 1.0f);
			jeVec3d_Set(&Up, 0.0f, 1.0f, 0.0f);
			break;
		case 1:				// Right Wall
			jeVec3d_Set(&Left, 0.0f, 0.0f,-1.0f);
			jeVec3d_Set(&Up, 0.0f, 1.0f, 0.0f);
			break;
		case 2:				// Back Wall
			jeVec3d_Set(&Left, -1.0f, 0.0f, 0.0f);
			jeVec3d_Set(&Up, 0.0f, 1.0f, 0.0f);
			break;
		case 3:				// Front Wall
			jeVec3d_Set(&Left,  1.0f, 0.0f, 0.0f);
			jeVec3d_Set(&Up, 0.0f, 1.0f, 0.0f);
			break;
		case 4:				// Ceiling
			jeVec3d_Set(&Left,  -1.0f, 0.0f, 0.0f);
			jeVec3d_Set(&Up, 0.0f, 0.0f, 1.0f);
			break;
		case 5:				// Floor
			jeVec3d_Set(&Left,  -1.0f, 0.0f, 0.0f);
			jeVec3d_Set(&Up, 0.0f, 0.0f,-1.0f);
			break;
		default:
			assert(0);
	}
	
	jeVec3d_Copy(&Plane.Normal, &In);

	jeVec3d_CrossProduct(&Up, &In, &Left);
	jeVec3d_Normalize(&Left);

	jeVec3d_CrossProduct(&In, &Left, &Up);
	jeVec3d_Normalize(&Up);

	jeXForm3d_SetFromLeftUpIn(DFace->PortalXForm, &Left, &Up, &In);

	// Calculate center of original brush face, and use that as reference point
	{
		jeVec3d		Center, Vert;
		int32		NumVerts;

		NumVerts = jeBrush_FaceGetVertCount(DFace->jeBrushFace);

		jeVec3d_Clear(&Center);

		for (i=0; i< NumVerts; i++)
		{
			Vert = jeBrush_FaceGetWorldSpaceVertByIndex(DFace->jeBrushFace, i);

			jeVec3d_Add(&Center, &Vert, &Center);
		}

		jeVec3d_Scale(&Center, 1.0f/(jeFloat)NumVerts, &Center);

		jeXForm3d_Translate(DFace->PortalXForm, Center.X, Center.Y, Center.Z);

	}

	// Rotate, and translate the reference point by using texture rotate/shift info
	#if 0
	{
		jeFloat				ShiftU, ShiftV;
		const jeFaceInfo	*pFaceInfo;
		jeXForm3d			XForm;
		jeQuaternion		Quat;

		pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(BSP->FaceInfoArray, DFace->FaceInfoIndex);
		assert(pFaceInfo);

		ShiftU = -pFaceInfo->ShiftU;
		ShiftV = pFaceInfo->ShiftV;

		jeXForm3d_Translate(DFace->PortalXForm, Left.X*ShiftU, Left.Y*ShiftU, Left.Z*ShiftU);
		jeXForm3d_Translate(DFace->PortalXForm, Up.X*ShiftV, Up.Y*ShiftV, Up.Z*ShiftV);

		jeQuaternion_SetFromAxisAngle(&Quat, &In, (pFaceInfo->Rotate/180.0f)*JE_PI);
		jeQuaternion_ToMatrix(&Quat, &XForm);

		jeXForm3d_Multiply(&XForm, DFace->PortalXForm, DFace->PortalXForm);
	}
	#endif

	return JE_TRUE;
}

static	int32	NumDrawFaces;
static	int32	RGBHack = 0;

//=======================================================================================
//	jeBSPNode_MakeDrawFaces
//=======================================================================================
jeBoolean jeBSPNode_MakeDrawFaces(jeBSPNode *Node, jeBSP *BSP)
{
	jeBSPNode_Face		*Face;
	int32				NumFaces;
	jeBSPNode_DrawFace	*DFace;

	assert(Node);
	assert(!(Node->Flags&NODE_LEAF));
	assert(!Node->Leaf);

	// Free old draw faces... (if any)
	jeBSPNode_DestroyDrawFaces(Node, BSP);

	// Count the bsp faces
	NumFaces = 0;
	for (Face = Node->Faces; Face; Face = Face->Next)
	{
		if (Face->Merged || Face->Split[0] || Face->Split[1])
			continue;		// Only interested in final faces

		NumFaces++;		
	}

	// Create the draw faces (array of pointers to drawfaces, in the DrawFace Array)
	assert(NumFaces < JE_BSP_MAX_DRAW_FACES);
	
	Node->DrawFaces = JE_RAM_ALLOCATE_ARRAY(jepBSPNode_DrawFace, NumFaces);

	if (!Node->DrawFaces)
	{
		jeErrorLog_AddString(-1, "jeBSPNode_MakeDrawFaces:  Out of memory for DrawFaces.", NULL);
		return JE_FALSE;
	}

	// Clear out entire array of pointers
	ZeroMemArray(Node->DrawFaces, NumFaces);

	// Now, build the draw faces from the BSP faces
	NumFaces = 0;
	for (Face = Node->Faces; Face; Face = Face->Next)
	{
		int32					v;
		const jeFaceInfo		*pFaceInfo;
		jeVec3d					*pVert;
		jeExtBox				Box;
		jeFaceInfo_ArrayIndex	FaceInfoIndex;
		jeBSP_TopSide			*TopSide;

		assert(Face->Portal);
		assert(Face->Portal->Side);

		if (Face->Merged || Face->Split[0] || Face->Split[1])
			continue;		// Only interested in final faces

		// Grab the TopSide
		TopSide = Face->Portal->Side;

		FaceInfoIndex = TopSide->FaceInfoIndex;

		pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(BSP->FaceInfoArray, FaceInfoIndex);
		assert(pFaceInfo);

	#if 0
		if (pFaceInfo->Flags & FACEINFO_NO_DRAWFACE || (TopSide->Flags & SIDE_VIS_PORTAL))
			continue;		// Don't create draw faces for vis portal faces...
	#endif

		Node->DrawFaces[NumFaces] = jeBSPNode_DrawFaceCreate(BSP);
		DFace = Node->DrawFaces[NumFaces];

		if (!DFace)
			return JE_FALSE;

		NumFaces++;

		// If face does not span multiple brush faces, mark the brush face it used
		if (!(Face->Flags & BSPFACE_SPAN_MULTIPLE_BRUSH_FACES))
			DFace->jeBrushFace = TopSide->jeBrushFace;

		DFace->Contents = Face->Contents;

		// Point them to each other
		Face->DrawFace = DFace;
		DFace->TopSideFlags = Face->TopSideFlags;

		DFace->Poly = jeIndexPoly_Create(Face->Poly->NumVerts);

		if (!DFace->Poly)
			return JE_FALSE;

		pVert = &Face->Poly->Verts[0];
		jeExtBox_SetToPoint(&Box, pVert);

		for (v=0; v< DFace->Poly->NumVerts; v++, pVert++)
		{
			//DFace->Poly->Verts[v] = jeVertArray_ShareVert(BSP->VertArray, pVert);
			DFace->Poly->Verts[v] = jeVertArray_AddVert(BSP->VertArray, pVert);

			jeExtBox_ExtendToEnclose(&Box, pVert);
		}

		DFace->Radius = jeVec3d_DistanceBetween(&Box.Max, &Box.Min) * 0.5f;
		jeVec3d_Add(&Box.Max, &Box.Min, &DFace->Center);
		jeVec3d_Scale(&DFace->Center, 0.5f, &DFace->Center);
		
		if (jePlaneArray_IndexSided(Face->PlaneIndex))
			DFace->NodeSide = 1;
		else
			DFace->NodeSide = 0;
						   
		// Assign faceinfo index
		if (!jeBSPNode_DrawFaceSetFaceInfoIndex(DFace, BSP, FaceInfoIndex))
			return JE_FALSE;

		// Assign PlaneIndex
		DFace->PlaneIndex = Face->PlaneIndex;
		// Ref plane index so we can use it outside of this scope safely
		if (!jePlaneArray_RefPlaneByIndex(BSP->PlaneArray, DFace->PlaneIndex))
			return JE_FALSE;

		// Get the TexVec index
		DFace->TexVecIndex = TopSide->TexVecIndex;
		// Ref the TexVec index, since we will carry it out of this scope
		if (!jeTexVec_ArrayRefTexVecByIndex(BSP->TexVecArray, DFace->TexVecIndex))
			return JE_FALSE;

		// Create the texture uv's
		if (!jeBSPNode_DrawFaceCreateUVInfo(DFace, BSP))
			return JE_FALSE;

		// Set up the faces portal
		if (pFaceInfo->PortalCamera)
		{
			DFace->PortalObject = pFaceInfo->PortalCamera;
			
			jeObject_CreateRef(DFace->PortalObject);
			
			DFace->PortalXForm = JE_RAM_ALLOCATE_STRUCT(jeXForm3d);

			if (!DFace->PortalXForm)
				return JE_FALSE;

			BuildDrawFaceXForm(DFace, BSP);
		}

		Node->NumDrawFaces++;
		DFace++;
	}

	assert(Node->NumDrawFaces == NumFaces);

	NumDrawFaces += NumFaces;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_MakeDrawFaces_r
//=======================================================================================
jeBoolean jeBSPNode_MakeDrawFaces_r(jeBSPNode *Node, jeBSP *BSP)
{
	assert(Node);

	if (Node->Flags & NODE_LEAF)
	{
		assert(Node->Leaf);
		return JE_TRUE;
	}

	assert(!Node->Leaf);

	if (!jeBSPNode_MakeDrawFaces(Node, BSP))
	{
		jeErrorLog_AddString(-1, "jeBSPNode_MakeDrawFaces_r:  jeBSPNode_MakeDrawFaces failed.", NULL);
		return JE_FALSE;
	}

	if (!jeBSPNode_MakeDrawFaces_r(Node->Children[NODE_FRONT], BSP))
		return JE_FALSE;
	if (!jeBSPNode_MakeDrawFaces_r(Node->Children[NODE_BACK], BSP))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_MakeDrawFaces_Callr
//=======================================================================================
jeBoolean jeBSPNode_MakeDrawFaces_Callr(jeBSPNode *Node, jeBSP *BSP)
{
	assert(Node);

	Log_Printf("--- jeBSPNode_MakeDrawFaces_Callr --- \n");

	NumDrawFaces = 0;

	if (!jeBSPNode_MakeDrawFaces_r(Node, BSP))
	{
		jeErrorLog_AddString(-1, "jeBSPNode_MakeDrawFaces_Callr:  jeBSPNode_MakeDrawFaces_r failed.", NULL);
		return JE_FALSE;
	}

	Log_Printf("Num DrawFaces          : %5i\n", NumDrawFaces);

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_UpdateLeafSides_r
//=======================================================================================
jeBoolean jeBSPNode_UpdateLeafSides_r(jeBSPNode *Node, jeBSP *BSP)
{
	assert(jeBSPNode_IsValid(Node));

	if (Node->Leaf)
	{
		if (Node->Leaf->Contents & JE_BSP_CONTENTS_SOLID)
		{
			if (!jeBSPNode_LeafInitializeSides(Node->Leaf, BSP))
				return JE_FALSE;
		}

		return JE_TRUE;
	}

	if (!jeBSPNode_UpdateLeafSides_r(Node->Children[NODE_FRONT], BSP))
		return JE_FALSE;
	if (!jeBSPNode_UpdateLeafSides_r(Node->Children[NODE_BACK], BSP))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_RemoveRecursionBit_r
//=======================================================================================
static void jeBSPNode_RemoveRecursionBit_r(jeBSPNode *Node, uint32 RecursionBit)
{
	int32		i;

	// Check recursion bit
	if (!(Node->RecursionBits & RecursionBit))
		return;

	g_WorldDebugInfo.NumNodes++;

	// Remove the recursion bit
	Node->RecursionBits ^= RecursionBit;

	if (Node->Leaf)		// At leaf, start recursing back up stack
	{
		g_WorldDebugInfo.NumLeaves++;
		return;
	}

	// Render polys on node
	for (i=0; i< Node->NumDrawFaces; i++)
	{
		jeBSPNode_DrawFace	*DFace;

		DFace = Node->DrawFaces[i];

		// Remove recursion bit from face
		DFace->RecursionBits ^= RecursionBit;
	}

	jeBSPNode_RemoveRecursionBit_r(Node->Children[NODE_FRONT], RecursionBit);
	jeBSPNode_RemoveRecursionBit_r(Node->Children[NODE_BACK], RecursionBit);
}

//=======================================================================================
//	jeBSPNode_RenderFrontToBack_r
//		Traverses down the side the camera is on to the opposite side the camera is on
//=======================================================================================
void jeBSPNode_RenderFrontToBack_r(jeBSPNode *Node, jeBSP *BSP, jeBSPNode_SceneInfo *SceneInfo, uint32 ClipFlags)
{
	jeFloat			Dist;
	int32			i, Side;
	const jePlane	*pPlane;

	assert(jeBSPNode_IsValid(Node) == JE_TRUE);

	// Check recursion bit
	if (SceneInfo->RecursionBit)
	{
		if (!(Node->RecursionBits & SceneInfo->RecursionBit))
		{
			return;
		}
	}

	// Remove the recursion bit
	Node->RecursionBits ^= SceneInfo->RecursionBit;

	g_WorldDebugInfo.NumNodes++;

	if (Node->Leaf)		// At leaf, start recursing back up stack
	{
		g_WorldDebugInfo.NumLeaves++;
		return;
	}

#if 1
	if (ClipFlags)
	{
		jeFloat		*MinMaxs;
		int32		*Index, p;
		jeFloat		Dist2;
		jeVec3d		Pnt;
		jePlane		*pPlane;
		jeFrustum	*Frustum;

		Frustum = SceneInfo->Frustum;

		MinMaxs = (float*)&Node->Box.Min;

		for (pPlane = Frustum->Planes, p=0; p< Frustum->NumPlanes; p++, pPlane++)
		{
			if (!(ClipFlags & (1<<p)) )
				continue;

			Index = Frustum->pFrustumBBoxIndexes[p];

			Pnt.X = MinMaxs[Index[0]];
			Pnt.Y = MinMaxs[Index[1]];
			Pnt.Z = MinMaxs[Index[2]];
			
			Dist2 = jeVec3d_DotProduct(&Pnt, &pPlane->Normal);
			Dist2 -= pPlane->Dist;

			if (Dist2 <= 0)
			{
				// We have no more visible nodes from this POV, 
				//	so just traverse to leafs to remove the recursion bit
				if (SceneInfo->RecursionBit)
				{
					jeBSPNode_RemoveRecursionBit_r(Node->Children[NODE_FRONT], SceneInfo->RecursionBit);
					jeBSPNode_RemoveRecursionBit_r(Node->Children[NODE_BACK], SceneInfo->RecursionBit);
				}

				return;
			}

			Pnt.X = MinMaxs[Index[3+0]];
			Pnt.Y = MinMaxs[Index[3+1]];
			Pnt.Z = MinMaxs[Index[3+2]];

			Dist2 = jeVec3d_DotProduct(&Pnt, &pPlane->Normal);
			Dist2 -= pPlane->Dist;

			if (Dist2 >= 0)		
				ClipFlags ^= (1<<p);		// Don't need to clip to this plane anymore
		}
	}			
#endif

	// Get the distance that the eye is from this plane
	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
	Dist = jePlane_PointDistanceFast(pPlane, &SceneInfo->POV);

	if (Dist < 0)		
		Side = 1;	// On the back side, go BACK->FRONT
	else
		Side = 0;	// On the front side, go FRONT->BACK

	jeBSPNode_RenderFrontToBack_r(Node->Children[Side], BSP, SceneInfo, ClipFlags);

	// Render polys on node
	for (i=0; i< Node->NumDrawFaces; i++)
	{
		jeDeviceCaps devcaps;
		jeBSPNode_DrawFace	*DFace;

		DFace = Node->DrawFaces[i];

		if (!(DFace->RecursionBits & SceneInfo->RecursionBit) && SceneInfo->RecursionBit)
			continue;

		// Remove recursion bit from face
		DFace->RecursionBits ^= SceneInfo->RecursionBit;

		if (DFace->NodeSide != Side)
			continue;		// Backfaced from node dir

		BSP->Driver->GetDeviceCaps(&devcaps);
		if ((devcaps.SuggestedDefaultRenderFlags & JE_RENDER_FLAG_HWTRANSFORM) == 0) {
			jeBSPNode_DrawFaceRender(DFace, BSP, SceneInfo, ClipFlags);
		} else {
			jeBSPNode_DrawFaceRenderPortal(DFace, BSP, SceneInfo, ClipFlags);
		}
		NumMakeFaces++;
	}

	jeBSPNode_RenderFrontToBack_r(Node->Children[!Side], BSP, SceneInfo, ClipFlags);
}

//=======================================================================================
//	jeBSPNode_RemoveTopBrush_r
//=======================================================================================
jeBoolean jeBSPNode_RemoveTopBrush_r(jeBSPNode *Node, jeBSP *BSP, jeBSP_TopBrush *TopBrush)
{
	jeBSP_Brush		*BSPBrush, *Next, *NewBrushes;
	jeBoolean		Rebuild;

	assert(jeBSPNode_IsValid(Node) == JE_TRUE);

	if (!(Node->Flags & NODE_LEAF))		// Recurse to leafs
	{
		if (!jeBSPNode_RemoveTopBrush_r(Node->Children[NODE_FRONT], BSP, TopBrush))
			return JE_FALSE;
		if (!jeBSPNode_RemoveTopBrush_r(Node->Children[NODE_BACK], BSP, TopBrush))
			return JE_FALSE;

		return JE_TRUE;
	}

	jeBSPNode_LeafDestroyDrawFaceList(Node->Leaf, BSP);

	// Find the brushes that point to the top brush, and remove them
	NewBrushes = NULL;
	Rebuild = JE_FALSE;

	for (BSPBrush = Node->Leaf->Brushes; BSPBrush; BSPBrush = Next)
	{
		Next = BSPBrush->Next;

		if (BSPBrush->Original == TopBrush)
		{
			Rebuild = JE_TRUE;
			jeBSP_BrushDestroy(&BSPBrush);
			continue;
		}

		BSPBrush->Next = NewBrushes;
		NewBrushes = BSPBrush;
	}

	Node->Leaf->Brushes = NewBrushes;

	if (Rebuild)		// If we removed any brushes, rebuild the contents in this leaf, and call for an update on faces
	{
		jeBSPNode_Portal	*p;
		int32				Side;

		// Rebuild the contents for this node...
		jeBSPNode_BuildContents(Node, BSP);

		// Go through all the portals that look into this leaf, and remove their polys
		// from the nodes they were created on
		for (p = Node->Portals; p; p = p->Next[Side])
		{
			Side = (p->Nodes[1] == Node);

			assert(!(p->Nodes[0] == Node && p->Nodes[1] == Node));
			assert(p->Nodes[0] == Node || p->Nodes[1] == Node);

			if (p->OnNode)
			{
				int32		i;

				for (i=0; i<2; i++)
				{
					if (p->Face[i])
					{
						assert(p->Face[i]->Portal == p);
						p->OnNode->Faces = jeBSPNode_FaceCullList(p->OnNode->Faces, &p->Face[i]);
					}
				}

				// Force an update on the node that the portal was on
				p->OnNode->Flags |= NODE_REBUILD_FACES;
			}

			jeBSPNode_PortalResetTopSide(p);
		}
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_RebuildFaces_r
//=======================================================================================
void jeBSPNode_RebuildFaces_r(jeBSPNode *Node, jeBSP *BSP)
{
	jeBSPNode_Portal	*p;
	int32				Side;

	assert(Node);

	if (!(Node->Flags & NODE_LEAF))
	{
		assert(!Node->Leaf);
		
		jeBSPNode_RebuildFaces_r(Node->Children[NODE_FRONT], BSP);
		jeBSPNode_RebuildFaces_r(Node->Children[NODE_BACK], BSP);

		// Destroy any draw faces on node
		jeBSPNode_DestroyDrawFaces(Node, BSP);
	}
	else
	{
		assert(Node->Leaf);
		assert(Node->Leaf->Node);
		assert(Node->Leaf->Node == Node);
		assert(!Node->DrawFaces);
		assert(!Node->NumDrawFaces);
		
		// Reset portal sides
		for (p=Node->Portals; p; p = p->Next[Side])
		{
			Side = (p->Nodes[1] == Node);

			assert(!(p->Nodes[0] == Node && p->Nodes[1] == Node));
			assert(p->Nodes[0] == Node || p->Nodes[1] == Node);

			jeBSPNode_PortalResetTopSide(p);

			p->Face[0] = NULL;
			p->Face[1] = NULL;
		}
	}

	assert(!Node->DrawFaces);
	assert(!Node->NumDrawFaces);

	// Free all the faces
	jeBSPNode_DestroyBSPFaces(Node, BSP, JE_FALSE);
}

//=======================================================================================
//	jeBSPNode_SplitBrushList
//=======================================================================================
jeBoolean jeBSPNode_SplitBrushList(jeBSPNode *Node, jeBSP *BSP, jeBSP_Brush *Brushes, jeBSP_Brush **Front, jeBSP_Brush **Back)
{
	jeBSP_Brush			*Brush, *NewBrush, *NewBrush2, *Next;
	jeBSP_Side			*Side;
	jePlane_Side		Sides;
	int32				i;
	
	*Front = *Back = NULL;

	for (Brush = Brushes ; Brush ; Brush = Next)
	{
		Next = Brush->Next;

		Sides = Brush->Side;

		if (Sides == PSIDE_BOTH)
		{	
			// Split into two brushes
			if (!jeBSP_BrushSplit(Brush, BSP, Node->PlaneIndex, SIDE_NODE, &NewBrush, &NewBrush2))
				return JE_FALSE;

			if (NewBrush)
			{
				NewBrush->Next = *Front;
				*Front = NewBrush;
			}
			if (NewBrush2)
			{
				NewBrush2->Next = *Back;
				*Back = NewBrush2;
			}

			BSP->DebugInfo.NumSplits++;

			continue;
		}
		
		// Copy the brush
		NewBrush = jeBSP_BrushCreateFromBSPBrush(Brush);

		// If the planenum is actualy a part of the brush
		// find the plane and flag it as used so it won't be tried
		// as a splitter again
		if (Sides & PSIDE_FACING)
		{
			for (Side = NewBrush->Sides, i=0 ; i<NewBrush->NumSides ; i++, Side++)
			{
				if (jePlaneArray_IndexIsCoplanar(Side->PlaneIndex, Node->PlaneIndex))
					Side->Flags |= SIDE_NODE;
			}
		}

		if (Sides & PSIDE_FRONT)
		{
			NewBrush->Next = *Front;
			*Front = NewBrush;
			continue;
		}
		if (Sides & PSIDE_BACK)
		{
			NewBrush->Next = *Back;
			*Back = NewBrush;
			continue;
		}
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_MakeAreas_r
//=======================================================================================
jeBoolean jeBSPNode_MakeAreas_r(jeBSPNode *Node, jeBSP *BSP, jeChain *AreaChain)
{
	assert(Node);

	// Recurse to leafs
	while (!(Node->Flags & NODE_LEAF))
	{
		assert(!Node->Leaf);

		if (!jeBSPNode_MakeAreas_r(Node->Children[0], BSP, AreaChain))
			return JE_FALSE;

		Node = Node->Children[1];
	}
	
	assert(Node->Leaf);

	// Find all VIS_PORTALS
	if (!jeBSPNode_LeafMakeAreas(Node->Leaf, BSP, AreaChain))
		return JE_FALSE;

	return JE_TRUE;
}

#ifdef AREA_DRAWFACE_TEST
//=======================================================================================
//	jeBSPNode_MakeLeafList_r
//=======================================================================================
jeBoolean jeBSPNode_MakeLeafList_r(jeBSPNode *Node,LinkNode *pList)
{
	assert(Node);

	// Recurse to leafs
	while (!(Node->Flags & NODE_LEAF))
	{
		assert(!Node->Leaf);

		if (!jeBSPNode_MakeLeafList_r(Node->Children[0],pList))
			return JE_FALSE;

		Node = Node->Children[1];
	}
	
	assert(Node->Leaf);

	assert(LN_ListLen(&(Node->Leaf->LN))==0);
	LN_AddTail(pList,Node->Leaf);

	return JE_TRUE;
}
#endif

//=======================================================================================
//	jeBSPNode_CountAreaVisPortals_r
//=======================================================================================
jeBoolean jeBSPNode_CountAreaVisPortals_r(jeBSPNode *Node)
{
	assert(Node);
	
	// recurse to leafs
	if (!(Node->Flags & NODE_LEAF))
	{
		assert(!Node->Leaf);
		if (!jeBSPNode_CountAreaVisPortals_r(Node->Children[NODE_FRONT]))
			return JE_FALSE;
		if (!jeBSPNode_CountAreaVisPortals_r(Node->Children[NODE_BACK]))
			return JE_FALSE;

		return JE_TRUE;
	}
	
	assert(Node->Leaf);

	if (!jeBSPNode_LeafCountAreaVisPortals(Node->Leaf))
		return JE_FALSE;
	
	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_MakeAreaVisPortals_r
//=======================================================================================
jeBoolean jeBSPNode_MakeAreaVisPortals_r(jeBSPNode *Node, jeBSP *BSP)
{
	assert(Node);
	
	// recurse to leafs
	if (!(Node->Flags & NODE_LEAF))
	{
		assert(!Node->Leaf);
		if (!jeBSPNode_MakeAreaVisPortals_r(Node->Children[NODE_FRONT], BSP))
			return JE_FALSE;
		if (!jeBSPNode_MakeAreaVisPortals_r(Node->Children[NODE_BACK], BSP))
			return JE_FALSE;

		return JE_TRUE;
	}
	
	assert(Node->Leaf);

	if (!jeBSPNode_LeafMakeAreaVisPortals(Node->Leaf, BSP))
		return JE_FALSE;
	
	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_DestroyAreas_r
//=======================================================================================
jeBoolean jeBSPNode_DestroyAreas_r(jeBSPNode *Node)
{
	assert(Node);

	// Recurse to leafs
	if (!(Node->Flags & NODE_LEAF))
	{
		assert(!Node->Leaf);

		if (!jeBSPNode_DestroyAreas_r(Node->Children[NODE_FRONT]))
			return JE_FALSE;
		if (!jeBSPNode_DestroyAreas_r(Node->Children[NODE_BACK]))
			return JE_FALSE;

		return JE_TRUE;
	}
	
	assert(Node->Leaf);

	if (Node->Leaf->Area)
		jeBSPNode_AreaDestroy(&Node->Leaf->Area);

	return JE_TRUE;
}

//=====================================================================================
//	jeBSPNode_DoAllAreasInRadius
//=====================================================================================
void jeBSPNode_DoAllAreasInRadius(jeBSPNode *Node, jeBSP *BSP, jeVec3d *Pos,jeFloat Radius,
												jeBSP_DoAreaFunc CB,void * Context)
{
	const jePlane	*Plane;
	jeFloat			Dist;

	assert(Node);
	assert(CB);
	assert(Pos);

	while(!(Node->Flags & NODE_LEAF))		// Go to leafs
	{
		assert(!Node->Leaf);

		Plane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
		
		Dist = jeVec3d_DotProduct(Pos, &(Plane->Normal)) - Plane->Dist;

		if (Dist > Radius)
		{
			Node = Node->Children[0];
		}
		else if ( Dist < (- Radius) )
		{
			Node = Node->Children[1];
		}
		else
		{
			// must go down both

			jeBSPNode_DoAllAreasInRadius(Node->Children[0], BSP, Pos,Radius,CB,Context);
												
			Node = Node->Children[1];
		}

		assert(Node);
	}

	assert(Node->Leaf);

	if ( Node->Leaf->Area )
	{
		CB(Node->Leaf->Area,Context);
	}
}

//=====================================================================================
//	jeBSPNode_DoAllAreasInBox
//=====================================================================================
jeBoolean jeBSPNode_DoAllAreasInBox(jeBSPNode *Node, jeBSP *BSP, jeExtBox *BBox,jeBSP_DoAreaFunc CB,void * Context)
{
	jeFloat R;
	jeVec3d Center;

	R = jeVec3d_DistanceBetween(&(BBox->Max),&(BBox->Min)) * 0.5f;
	jeVec3d_Add(&(BBox->Max),&(BBox->Min),&Center);
	jeVec3d_Scale(&Center,0.5f,&Center);

	// <> approximate, fast !
   	jeBSPNode_DoAllAreasInRadius(Node,BSP,&Center,R,CB,Context);

	return JE_TRUE;
}

//=====================================================================================
//	jeBSPNode_FindArea
//=====================================================================================
jeBSPNode_Area *jeBSPNode_FindArea(jeBSPNode *Node, jeBSP *BSP, const jeVec3d *Pos)
{

	assert(jeBSPNode_IsValid(Node) == JE_TRUE);
	assert(Pos);

	while(!(Node->Flags & NODE_LEAF))		// Go to leafs
	{
		const jePlane	*Plane;
		jeFloat			Dist;

		assert(!Node->Leaf);

		Plane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
		
		Dist = jePlane_PointDistanceFast(Plane, Pos);	// We can use fast check, since node planes face positive

		if (Dist > 0)
			Node = Node->Children[0];
		else
			Node = Node->Children[1];

		assert(Node);
	}

	assert(Node->Leaf);

	return Node->Leaf->Area;
}

// not really the closest; it's the first non-solid leaf we
//	can find, walking backwards up the tree
jeBSPNode_Area *jeBSPNode_FindClosestArea(jeBSPNode *Node, jeBSP *BSP, const jeVec3d *Pos)
{
	assert(jeBSPNode_IsValid(Node) == JE_TRUE);
	assert(Pos);

	while(!(Node->Flags & NODE_LEAF))		// Go to leafs
	{
		const jePlane	*Plane;
		jeFloat			Dist;
		jeBSPNode_Area	*Area;
		int32			Side;

		assert(!Node->Leaf);

		Plane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
		
		Dist = jePlane_PointDistanceFast(Plane, Pos);	// We can use fast check, since node planes face positive

		Side = (Dist > 0) ? 0 : 1;

		Area = jeBSPNode_FindClosestArea(Node->Children[Side], BSP, Pos);

		if ( Area )
			return Area;

		Node = Node->Children[!Side];

		assert(Node);
	}

	return Node->Leaf->Area;
}

//====================================================================================
//	jeBSPNode_RayIntersects_r
//====================================================================================
#define NODE_CLIPPLANE_EPSILON 0.0001f  // DarkRift/Incarnadine
jeBoolean jeBSPNode_RayIntersects_r(const jeBSPNode *Node, jeBSP *BSP, const jeVec3d *Front, const jeVec3d *Back)
{
    jeFloat			Fd, Bd, Dist;
    int32			Side;
    jeVec3d			I;
	const jePlane	*pPlane;

	assert(jeBSPNode_IsValid(Node) == JE_TRUE);

	if (Node->Leaf)
	{
		if (Node->Leaf->Contents & JE_BSP_CONTENTS_SOLID)
			return JE_TRUE;						// Ray collided with solid space
		else 
			return JE_FALSE;					// Ray collided with empty space
	}

	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);

    Fd = jePlane_PointDistanceFast(pPlane, Front);
    Bd = jePlane_PointDistanceFast(pPlane, Back);

    //if (Fd >= -1.0f && Bd >= -1.0f) 
	if (Fd >= -NODE_CLIPPLANE_EPSILON && Bd >= -NODE_CLIPPLANE_EPSILON) //DarkRift/Incarnadine
        return(jeBSPNode_RayIntersects_r(Node->Children[NODE_FRONT], BSP, Front, Back));
    //if (Fd < 1.0f && Bd < 1.0f) 
	if (Fd < NODE_CLIPPLANE_EPSILON && Bd < NODE_CLIPPLANE_EPSILON) //DarkRift/Incarnadine
        return(jeBSPNode_RayIntersects_r(Node->Children[NODE_BACK], BSP, Front, Back));

    Side = (Fd < 0);
    Dist = Fd / (Fd - Bd);

    I.X = Front->X + Dist * (Back->X - Front->X);
    I.Y = Front->Y + Dist * (Back->Y - Front->Y);
    I.Z = Front->Z + Dist * (Back->Z - Front->Z);

	if (jeBSPNode_RayIntersects_r(Node->Children[Side], BSP, Front, &I))
        return JE_TRUE;
    else if (jeBSPNode_RayIntersects_r(Node->Children[!Side], BSP, &I, Back))
		return JE_TRUE;

	return JE_FALSE;
}

//====================================================================================
//	jeBSPNode_CollisionExact_r
//====================================================================================
jeBoolean jeBSPNode_CollisionExact_r(const jeBSPNode *Node, jeBSP *BSP, const jeVec3d *Front, const jeVec3d *Back, jeBSPNode_CollisionInfo *Info)
{
    jeFloat			Fd, Bd, Dist;
    int32			Side;
    jeVec3d			I;
	const jePlane	*pPlane;

	assert(jeBSPNode_IsValid(Node) == JE_TRUE);

	if (Node->Leaf)
	{
		if (Node->Leaf->Contents & JE_BSP_CONTENTS_SOLID )
			return JE_TRUE;						// Ray collided with solid space
		else 
			return JE_FALSE;					// Ray collided with empty space
	}

	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);

    Fd = jePlane_PointDistanceFast(pPlane, Front);

    Bd = jePlane_PointDistanceFast(pPlane, Back);

    //if (Fd >= -1.0f && Bd >= -1.0f)
	if (Fd >= -NODE_CLIPPLANE_EPSILON && Bd >= -NODE_CLIPPLANE_EPSILON) //DarkRift/Incarnadine
        return(jeBSPNode_CollisionExact_r(Node->Children[NODE_FRONT], BSP, Front, Back, Info));
    //if (Fd < 1.0f && Bd < 1.0f)
	if (Fd < NODE_CLIPPLANE_EPSILON && Bd < NODE_CLIPPLANE_EPSILON) //DarkRift/Incarnadine
        return(jeBSPNode_CollisionExact_r(Node->Children[NODE_BACK], BSP, Front, Back, Info));

    Side = (Fd < 0);
    Dist = Fd / (Fd - Bd);

    I.X = Front->X + Dist * (Back->X - Front->X);
    I.Y = Front->Y + Dist * (Back->Y - Front->Y);
    I.Z = Front->Z + Dist * (Back->Z - Front->Z);

	if (jeBSPNode_CollisionExact_r(Node->Children[Side], BSP, Front, &I, Info))
	{
		if (Info && !Info->HitSet) // Icestorm
		{
			*(Info->Plane) = *pPlane;
			
			if (Side)
				jePlane_Inverse(Info->Plane);

			*(Info->Impact) = I;
			Info->HitSet = JE_TRUE;
		}
		TotalNumberCollisions++;
        return JE_TRUE;
	}
    else if (jeBSPNode_CollisionExact_r(Node->Children[!Side], BSP, &I, Back, Info))
	{
		if (Info && !Info->HitSet) // Icestorm
		{
			*(Info->Plane) = *pPlane;
			
			if (Side)
				jePlane_Inverse(Info->Plane);

			*(Info->Impact) = I;
			Info->HitSet = JE_TRUE;
		}
		TotalNumberCollisions++;
		return JE_TRUE;
	}

	return JE_FALSE;
}

//=====================================================================================
//	jeBSPNode_CollisionBBox_r
//=====================================================================================
jeBoolean jeBSPNode_CollisionBBox_r(const jeBSPNode *Node, jeBSP *BSP, const jeExtBox *Box1, const jeExtBox *Box2, const jeVec3d *Front, const jeVec3d *Back, jeBSPNode_CollisionInfo2 *Info)
{
	jePlane_Side	Side;
	const jePlane	*pPlane;

	assert(jeBSPNode_IsValid(Node));

	if (Node->Leaf)
	{
		jeBSPNode_CollisionInfo2 DummyInfo;
		if (!(Node->Leaf->Contents & JE_BSP_CONTENTS_SOLID))
			return JE_FALSE;
		
		if (!Node->Leaf->NumSides)
			return JE_FALSE;

		if (!Info) // Icestorm: Ignore precise details
		{
			Info=&DummyInfo;
			Info->HitSet = JE_FALSE;
			Info->HitLeaf = JE_TRUE;	
		} else
			Info->HitLeaf = JE_FALSE;

		jeBSPNode_LeafCollision_r(Node->Leaf, BSP, 0, 1, Box2, Front, Back, Info);

		return (Info->HitSet);
	}

	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
	assert(pPlane);

	Side = jePlane_BoxSide(pPlane, Box1, 0.01f);

	if (Info)
	{
		// Go down the sides that the box lands in
		if (Side & PSIDE_FRONT)
			jeBSPNode_CollisionBBox_r(Node->Children[NODE_FRONT], BSP, Box1, Box2, Front, Back, Info);
		if (Side & PSIDE_BACK)
			jeBSPNode_CollisionBBox_r(Node->Children[NODE_BACK], BSP, Box1, Box2, Front, Back, Info);
		return (Info->HitSet);
	} else
	{
		// Go down the sides that the box lands in
		if (Side & PSIDE_FRONT)
			if (jeBSPNode_CollisionBBox_r(Node->Children[NODE_FRONT], BSP, Box1, Box2, Front, Back, Info))
				return JE_TRUE;
		if (Side & PSIDE_BACK)
			if (jeBSPNode_CollisionBBox_r(Node->Children[NODE_BACK], BSP, Box1, Box2, Front, Back, Info))
				return JE_TRUE;
		return JE_FALSE;
	}
}

// Added by Icestorm
//=====================================================================================
//	jeBSPNode_ChangeBoxCollisionBBox_r
//=====================================================================================
jeBoolean jeBSPNode_ChangeBoxCollisionBBox_r(const jeBSPNode *Node, jeBSP *BSP, const jeExtBox *Box1, const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeBSPNode_CollisionInfo3 *Info)
{
	jePlane_Side	Side;
	const jePlane	*pPlane;

	assert(jeBSPNode_IsValid(Node));

	if (Node->Leaf)
	{
		jeBSPNode_CollisionInfo3 DummyInfo;
		if (!(Node->Leaf->Contents & JE_BSP_CONTENTS_SOLID))
			return JE_FALSE;
		
		if (!Node->Leaf->NumSides)
			return JE_FALSE;

		if (!Info) // Icestorm: Ignore precise details
		{
			Info=&DummyInfo;
			Info->HitLeaf = JE_TRUE;	
			Info->HitSet  = JE_FALSE;
		} else
			Info->HitLeaf = JE_FALSE;

		jeBSPNode_LeafChangeBoxCollision_r(Node->Leaf, BSP, 0, 1, Pos, FrontBox, BackBox, Info);

		return (Info->HitSet);
	}

	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
	assert(pPlane);

	Side = jePlane_BoxSide(pPlane, Box1, 0.01f);

	if (Info)
	{
		// Go down the sides that the box lands in
		if (Side & PSIDE_FRONT)
			jeBSPNode_ChangeBoxCollisionBBox_r(Node->Children[NODE_FRONT], BSP, Box1, Pos, FrontBox, BackBox, Info);
		if (Side & PSIDE_BACK)
			jeBSPNode_ChangeBoxCollisionBBox_r(Node->Children[NODE_BACK], BSP, Box1, Pos, FrontBox, BackBox, Info);
		return (Info->HitSet);
	} else
	{
		// Go down the sides that the box lands in
		if (Side & PSIDE_FRONT)
			if (jeBSPNode_ChangeBoxCollisionBBox_r(Node->Children[NODE_FRONT], BSP, Box1, Pos, FrontBox, BackBox, Info))
				return JE_TRUE;
		if (Side & PSIDE_BACK)
			if (jeBSPNode_ChangeBoxCollisionBBox_r(Node->Children[NODE_BACK], BSP, Box1, Pos, FrontBox, BackBox, Info))
				return JE_TRUE;
		return JE_FALSE;
	}
}

//=======================================================================================
//	jeBSPNode_MakeDrawFaceListOnLeafs_r
//=======================================================================================
jeBoolean jeBSPNode_MakeDrawFaceListOnLeafs_r(jeBSPNode *Node, jeBSP *BSP)
{
	if (!(Node->Flags & NODE_LEAF))		// Recurse to leafs
	{
		if (!jeBSPNode_MakeDrawFaceListOnLeafs_r(Node->Children[NODE_FRONT], BSP))
			return JE_FALSE;
		if (!jeBSPNode_MakeDrawFaceListOnLeafs_r(Node->Children[NODE_BACK], BSP))
			return JE_FALSE;

		return JE_TRUE;
	}

	if (!jeBSPNode_LeafMakeDrawFaceList(Node->Leaf, BSP))
		return JE_FALSE;

	return JE_TRUE;
}

//=====================================================================================
//	jeBSPNode_VisibleContents
//=====================================================================================
uint32 jeBSPNode_VisibleContents(uint32 Contents)
{
	int32		i;

	Contents &= JE_BSP_VISIBLE_CONTENTS;

	if (!Contents)
		return 0;		// Early out

	for (i=0; i<32; i++)
	{
		uint32	Bit = (1<<i);

		if (Contents & Bit)
			return Bit;
	}

	return 0;		// No visible contents
}

//=====================================================================================
//	jeBSPNode_GetTopSideSeperatingLeafs
//=====================================================================================
void jeBSPNode_GetTopSideSeperatingLeafs(jeBSP *BSP, const jeBSPNode_Leaf *Leaf1, const jeBSPNode_Leaf *Leaf2, jePlaneArray_Index PlaneIndex, jeBSP_TopBrush **DstTopBrush, jeBSP_TopSide **DstTopSide)
{
	uint32					Contents, MajorContents, BestOrder;
	jeBSP_Brush				*Brush;
	int32					i,j;
	jeBSP_TopSide			*Side, *BestSide, *ExactSide;
	jeBSP_TopBrush			*ExactBrush, *BestBrush;
	jeFloat					Dot, BestDot;
	const jePlane			*p1;
	jeBrush_Contents		c1, c2;
	const jeBSPNode_Leaf	*Leafs[2];

	assert(Leaf1);
	assert(Leaf2);
	assert(DstTopBrush);
	assert(DstTopSide);

	*DstTopBrush = NULL;
	*DstTopSide = NULL;

	// Portal only visible, if it is seperating different contents (or either is sheet)
	c1 = Leaf1->Contents;
	c2 = Leaf2->Contents;

	Contents = c1^c2;

	if ((c1&c2)&JE_BSP_CONTENTS_SHEET)
		Contents |= JE_BSP_CONTENTS_SHEET;	// If sheet was on both side, put it back in

	// There must be a visible contents on at least one side of the portal...
	//MajorContents = jeBSPNode_VisibleContents(Contents);
	MajorContents = Contents & JE_BSP_VISIBLE_CONTENTS;

	if (MajorContents & (JE_BSP_CONTENTS_SOLID | JE_BSP_CONTENTS_AIR))
		MajorContents &= ~JE_BSP_CONTENTS_EMPTY;	// Solid/Cut overides empty when picking textures

	if (!MajorContents)		
		return;				// Portal doea not seperate visible contents

	p1 = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, PlaneIndex);

	BestOrder = 0;
	ExactSide = BestSide = NULL;
	ExactBrush = BestBrush = NULL;
	BestDot = 0.0f;

	Leafs[0] = Leaf1;
	Leafs[1] = Leaf2;

	for (j=0 ; j<2 ; j++)
	{
		const jeBSPNode_Leaf	*Leaf;

		Leaf = Leafs[j];

		// First try and find an exact match
		for (Brush= Leaf->Brushes ; Brush; Brush=Brush->Next)
		{
			jeBSP_TopBrush		*TopBrush;

			TopBrush = Brush->Original;

			// Only use the brush that contains a major contents (solid)
			if (!(TopBrush->Contents & MajorContents))
				continue;

			if (TopBrush->Order < BestOrder)
				continue;

		#if 0
			for (i=0 ; i< Brush->NumSides ; i++)
			{
				Side = &Brush->Sides[i];
			
				if (!(Side->Flags & SIDE_SPLIT))
					break;
			}

			if (i == Brush->NumSides)
				continue;		// Brush was split on all sides, just a contents changer
		#endif

			for (i=0 ; i<TopBrush->NumSides ; i++)
			{
				const jePlane		*p2;

				Side = &TopBrush->TopSides[i];
			
				if (Side->Flags & SIDE_NODE)
					continue;		

				if (Side->Flags & SIDE_SPLIT)
					continue;

				if (jePlaneArray_IndexIsCoplanar(Side->PlaneIndex,PlaneIndex))
				{	
					// Exact match
					ExactSide = &TopBrush->TopSides[i];
					ExactBrush = TopBrush;
					BestOrder = TopBrush->Order;
				}
				
				if (!(Contents & JE_BSP_CONTENTS_SHEET) && !ExactSide)
				{
					// Until we find an exact match, 
					// keep looking for the closest match just in case
					p2 = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Side->PlaneIndex);
					Dot = jeVec3d_DotProduct(&p1->Normal, &p2->Normal);

					if (Dot > BestDot)
					{
						BestDot = Dot;
						BestSide = Side;
						BestOrder = TopBrush->Order;
						BestBrush = TopBrush;
					}
				}
			}
		}
	}
	
	if (ExactSide)
	{
		// ExactSide/Brush overides BestSide
		*DstTopBrush = ExactBrush;
		*DstTopSide = ExactSide;
	}
	else
	{
		// If no exact, just take the best
		*DstTopBrush = BestBrush;
		*DstTopSide = BestSide;
	}
}

#define NODESTACK_MAX_NODES		1024

typedef struct
{
	int32			NumNodes;
	const jeBSPNode	*Nodes[NODESTACK_MAX_NODES];
} NodeStack;

//====================================================================================
//	jeBSPNode_FillNodeStack_r
//	Fills a node stack using the segment from front to back of segment
//====================================================================================
static jeBoolean jeBSPNode_FillNodeStack_r(const jeBSPNode *Node, jeBSP *BSP, const jeVec3d *Front, const jeVec3d *Back, NodeStack *Stack)
{
    jeFloat			Fd, Bd, Dist;
    int32			Side;
    jeVec3d			I;
	const jePlane	*pPlane;

	assert(jeBSPNode_IsValid(Node) == JE_TRUE);
	assert(Front);
	assert(Back);

	if (Node->Leaf)
	{
		if (Stack->NumNodes >= NODESTACK_MAX_NODES)
			return JE_FALSE;		// Oh well...

		Stack->Nodes[Stack->NumNodes++] = Node;

		return JE_TRUE;
	}

	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);

    Fd = jePlane_PointDistanceFast(pPlane, Front);
    Bd = jePlane_PointDistanceFast(pPlane, Back);

    if (Fd >= 0.0f && Bd >= 0.0f) 
        return(jeBSPNode_FillNodeStack_r(Node->Children[NODE_FRONT], BSP, Front, Back, Stack));
    if (Fd <= 0.0f && Bd <= 0.0f)
        return(jeBSPNode_FillNodeStack_r(Node->Children[NODE_BACK], BSP, Front, Back, Stack));

    // The segment is split
	Side = (Fd < 0);
    Dist = Fd / (Fd - Bd);

    I.X = Front->X + Dist * (Back->X - Front->X);
    I.Y = Front->Y + Dist * (Back->Y - Front->Y);
    I.Z = Front->Z + Dist * (Back->Z - Front->Z);

	// Traverse form front of segment to back
	if (!jeBSPNode_FillNodeStack_r(Node->Children[Side], BSP, Front, &I, Stack))
		return JE_FALSE;

	if (Stack->NumNodes >= NODESTACK_MAX_NODES)
		return JE_FALSE;		// Oh well...

	Stack->Nodes[Stack->NumNodes++] = Node;

	if (!jeBSPNode_FillNodeStack_r(Node->Children[!Side], BSP, &I, Back, Stack))
		return JE_FALSE;

	return JE_TRUE;
}

//====================================================================================
//	jeBSPNode_RayIntersectsBrushes
//====================================================================================
jeBoolean jeBSPNode_RayIntersectsBrushes(const jeBSPNode *Node, jeBSP *BSP, const jeVec3d *Front, const jeVec3d *Back, jeBrushRayInfo *Info)
{
	NodeStack		Stack;
	int32			i;

	assert(Node);
	assert(Front);
	assert(Back);
	assert(Info);

	ZeroMem(Info);

	Stack.NumNodes = 0;

	if (!jeBSPNode_FillNodeStack_r(Node, BSP, Front, Back, &Stack))
		return JE_FALSE;		// Node stack must have filled up, just act like there was no collision

	for (i=0; i< Stack.NumNodes-2; i += 2)
	{
		const jeBSPNode		*Node1, *Node2, *Node3;

		Node1 = Stack.Nodes[i+0];
		Node2 = Stack.Nodes[i+1];
		Node3 = Stack.Nodes[i+2];

		assert(Node1->Leaf);			// The left leaf
		assert(!Node2->Leaf);
		assert(Node3->Leaf);

		if (!(Node1->Leaf->Contents & JE_BSP_CONTENTS_SOLID))
		{
			jeBSP_TopBrush		*TopBrush;
			jeBSP_TopSide		*TopSide;

			jeBSPNode_GetTopSideSeperatingLeafs(BSP, Node1->Leaf, Node3->Leaf, Node2->PlaneIndex, &TopBrush, &TopSide);

			if (TopBrush && TopSide)
			{
				jeFloat			Fd, Bd, Ratio;

				Info->Brush = TopBrush->Original;
				Info->BrushFace = TopSide->jeBrushFace;

				Info->Plane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node2->PlaneIndex);

				Fd = jePlane_PointDistanceFast(&Info->Plane, Front);
				Bd = jePlane_PointDistanceFast(&Info->Plane, Back);

				Ratio = Fd / (Fd - Bd);

				Info->Impact.X = Front->X + Ratio * (Back->X - Front->X);
				Info->Impact.Y = Front->Y + Ratio * (Back->Y - Front->Y);
				Info->Impact.Z = Front->Z + Ratio * (Back->Z - Front->Z);

				Info->c1 = Node1->Leaf->BrushContents;
				Info->c2 = Node3->Leaf->BrushContents;

				return JE_TRUE;
			}
		}
	}

	return JE_FALSE;
}

//====================================================================================
//	jeBSPNode_SetDLight_r
//====================================================================================
jeBoolean jeBSPNode_SetDLight_r(jeBSPNode *Node, jeBSP *BSP, jeBSPNode_Light *Light, uint32 LBit, uint32 DLightVisFrame)
{
	const jePlane	*pPlane;
	int32			i;
	jeFloat			Dist;

	assert(jeBSPNode_IsValid(Node));

	if (Node->Leaf)
		return JE_TRUE;			// At leaf no more recursing

	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex);
	assert(pPlane);

	Dist = jePlane_PointDistanceFast(pPlane, &Light->Pos);

	if (Dist > Light->Radius)			// Only on front
		return jeBSPNode_SetDLight_r(Node->Children[NODE_FRONT], BSP, Light, LBit, DLightVisFrame);
	else if (Dist < -Light->Radius)		// Only on back
		return jeBSPNode_SetDLight_r(Node->Children[NODE_BACK], BSP, Light, LBit, DLightVisFrame);

	// Light is touching node, find faces that touch light on the node
	for (i=0; i< Node->NumDrawFaces; i++)
	{
		jeBSPNode_DrawFace	*DFace;

		DFace = Node->DrawFaces[i];

		if (jeVec3d_DistanceBetween(&DFace->Center, &Light->Pos) > Light->Radius + DFace->Radius)
			continue;		// Light is NOT in radius of face

		if (DFace->DLightVisFrame != DLightVisFrame)
		{
			// Reset the face if this is the first light to touch it this frame...
			DFace->DLights = 0;
			DFace->DLightVisFrame = DLightVisFrame;
		}

		// This face will be lit by the light, so mark it
		DFace->DLights |= LBit;
	}

	// Go down both
	if (!jeBSPNode_SetDLight_r(Node->Children[NODE_FRONT], BSP, Light, LBit, DLightVisFrame))
		return JE_FALSE;

	if (!jeBSPNode_SetDLight_r(Node->Children[NODE_BACK], BSP, Light, LBit, DLightVisFrame))
		return JE_FALSE;

	return JE_TRUE;
}

//====================================================================================
//	jeBSPNode_CreateLightmapTHandles_r
//====================================================================================
jeBoolean jeBSPNode_CreateLightmapTHandles_r(jeBSPNode *Node, DRV_Driver *Driver)
{
	int32		i;

	assert(jeBSPNode_IsValid(Node));
	assert(Driver);

	if (Node->Leaf)
		return JE_TRUE;		// At leaf, start recursing back up

	for (i=0; i< Node->NumDrawFaces; i++)
	{
		jeBSPNode_DrawFace		*pDFace;
		int32					Width, Height;
		jeRDriver_PixelFormat	PixelFormat;
		jeBSPNode_Lightmap		*Lightmap;

		pDFace = Node->DrawFaces[i];

		Lightmap = pDFace->Lightmap;

		if (!Lightmap)
			continue;

		assert(!Lightmap->THandle);

		// Create the lightmap THandle 
		Width = Lightmap->Width;
		Height = Lightmap->Height;

		// Modified by chrisjp.
		if(bestSupportedLightmapPixelFormat == 0)
			DetermineSupportedLightmapFormat(JE_PIXELFORMAT_16BIT_565_RGB , Driver);

		PixelFormat.Flags = RDRIVER_PF_LIGHTMAP;
		PixelFormat.PixelFormat = bestSupportedLightmapPixelFormat;

		Lightmap->THandle = Driver->THandle_Create(Width, Height, 1, &PixelFormat);

		if (!Lightmap->THandle)
			return JE_FALSE;
	}

	if (!jeBSPNode_CreateLightmapTHandles_r(Node->Children[NODE_FRONT], Driver))
		return JE_FALSE;
	if (!jeBSPNode_CreateLightmapTHandles_r(Node->Children[NODE_BACK], Driver))
		return JE_FALSE;

	return JE_TRUE;
}

//====================================================================================
//	jeBSPNode_DestroyLightmapTHandles_r
//====================================================================================
jeBoolean jeBSPNode_DestroyLightmapTHandles_r(jeBSPNode *Node, DRV_Driver *Driver)
{
	int32		i;

	assert(jeBSPNode_IsValid(Node));
	assert(Driver);

	if (Node->Leaf)
		return JE_TRUE;		// At leaf, start recursing back up

	for (i=0; i< Node->NumDrawFaces; i++)
	{
		jeBSPNode_DrawFace		*pDFace;
		jeBSPNode_Lightmap		*Lightmap;

		pDFace = Node->DrawFaces[i];

		Lightmap = pDFace->Lightmap;

		if (!Lightmap)
			continue;

		assert(Lightmap->THandle);

		Driver->THandle_Destroy(Lightmap->THandle);
		Lightmap->THandle = NULL;
	}

	if (!jeBSPNode_DestroyLightmapTHandles_r(Node->Children[NODE_FRONT], Driver))
		return JE_FALSE;
	if (!jeBSPNode_DestroyLightmapTHandles_r(Node->Children[NODE_BACK], Driver))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_WeldDrawFaceVerts_r
//=======================================================================================
jeBoolean jeBSPNode_WeldDrawFaceVerts_r(jeBSPNode *Node, jeBSP *BSP, jeVertArray_Optimizer *Optimizer)
{
	int32		i;

	assert(jeBSPNode_IsValid(Node));

	if (Node->Leaf)
		return JE_TRUE;		// At leaf, return

	for (i=0; i< Node->NumDrawFaces; i++)
	{
		jeBSPNode_DrawFace	*pDrawFace;
		jeVertArray_Index	*pIVert;
		int32				v;

		pDrawFace = Node->DrawFaces[i];
		assert(pDrawFace);
		
		for (pIVert = pDrawFace->Poly->Verts, v=0; v< pDrawFace->Poly->NumVerts; v++, pIVert++)
		{
			(*pIVert) = jeVertArray_GetOptimizedIndex(BSP->VertArray, Optimizer, (*pIVert));
			assert((*pIVert) != JE_VERTARRAY_NULL_INDEX);
		}
	}

	if (!jeBSPNode_WeldDrawFaceVerts_r(Node->Children[NODE_FRONT], BSP, Optimizer))
		return JE_FALSE;

	if (!jeBSPNode_WeldDrawFaceVerts_r(Node->Children[NODE_BACK], BSP, Optimizer))
		return JE_FALSE;

	return JE_TRUE;
}

//=====================================================================================
//	TJunct code (put most of this in jeBSPNode_DrawFace???)
//=====================================================================================

// FIXME:	Dynamically allocate these arrays

#define OFF_EPSILON				(0.05f)
#define	MAX_TEMP_INDEX_VERTS	(1024)

static int32					NumTempIndexVerts;
static jeVertArray_Index		TempIndexVerts[MAX_TEMP_INDEX_VERTS];

static int32					NumEdgeVerts;
static jeVertArray_Index		EdgeVerts[JE_VERTARRAY_MAX_VERTS];

static const jeVec3d			*pEdgeV1;
static const jeVec3d			*pEdgeV2;
static jeVec3d					EdgeDir;
static jeBSP					*g_BSP;

//=====================================================================================
//	FinalizeFace
//=====================================================================================
static jeBoolean FinalizeFace(jeBSPNode_DrawFace *Face, int32 Base, jeBSP *BSP)
{
	int32				i;
	jeIndexPoly			*Poly;
	const jeFaceInfo	*pFaceInfo;

	if (NumTempIndexVerts == Face->Poly->NumVerts)
		return JE_TRUE;			// No TJunctions were added, leave face alone...

	// Grab the faceinfo
	pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(BSP->FaceInfoArray, Face->FaceInfoIndex);
	assert(pFaceInfo);
	
	if (pFaceInfo->PortalCamera)
		return JE_TRUE;				// Don't fix tjuncts for faces that use portals, it just complicates things...

	// Create a new poly with the new number of vertices
	Poly = jeIndexPoly_Create((jeIndexPoly_NumVertType)NumTempIndexVerts);

	if (!Poly)
		return JE_FALSE;
		
	// Copy the new vertices over
	for (i=0; i< NumTempIndexVerts; i++)
	{
		// Assign the index to the new poly
		Poly->Verts[i] = TempIndexVerts[(i+Base)%NumTempIndexVerts];
		// Ref the index
		if (!jeVertArray_RefVertByIndex(BSP->VertArray, Poly->Verts[i]))
			return JE_FALSE;
	}

	// De-ref the verts in the old poly
	for (i=0; i< Face->Poly->NumVerts; i++)
		jeVertArray_RemoveVert(BSP->VertArray, &Face->Poly->Verts[i]);

	// Destroy the old poly on the face
	jeIndexPoly_Destroy(&Face->Poly);

	// Assign the new poly (with fixed TJuncts)
	Face->Poly = Poly;

	// Create the texture uv's
	if (!jeBSPNode_DrawFaceCreateUVInfo(Face, BSP))
		return JE_FALSE;

	return JE_TRUE;
}

//=====================================================================================
//	TestEdge_r
//=====================================================================================
static jeBoolean TestEdge_r(jeFloat Start, jeFloat End, jeVertArray_Index p1, jeVertArray_Index p2, int32 StartVert)
{
	int32		k;

	if (p1 == p2)
		return JE_TRUE;			// Degenerate edge

	for (k=StartVert ; k<NumEdgeVerts ; k++)
	{
		const jeVec3d		*pVert;
		jeVertArray_Index	j;
		jeFloat				Dist;
		jeVec3d				Vert2;
		jeVec3d				Exact;
		jeVec3d				Off;
		jeFloat				Error;

		j = EdgeVerts[k];

		if (j == p1 || j == p2)
			continue;

		pVert = jeVertArray_GetVertByIndex(g_BSP->VertArray, j);
		assert(pVert);

		// Translate the point by the amount it would take to put the edge at the origin...
		jeVec3d_Subtract(pVert, pEdgeV1, &Vert2);
		Dist = jeVec3d_DotProduct(&Vert2, &EdgeDir);
		
		if (Dist <= Start || Dist >= End)
			continue;						// Point does not lie in between edge end points
		
		// Put the point along the edge by projecting it along the edge dir by the amount of the dist perpendicular from the start of the edge
		jeVec3d_AddScaled(pEdgeV1, &EdgeDir, Dist, &Exact);
		jeVec3d_Subtract(pVert, &Exact, &Off);
		Error = jeVec3d_Length(&Off);

		if (fabs(Error) > OFF_EPSILON)
			continue;		// Point does NOT lie on the edge

		// break the edge
		//NumTJunctions++;

		// Recursively go down the back side, re-constructing the poly while preserving the winding order...
		if (!TestEdge_r(Start, Dist, p1, j, k+1))
			return JE_FALSE;

		// Now go down the front side, and add the TJunct point...
		if (!TestEdge_r(Dist, End, j, p2, k+1))
			return JE_FALSE;

		return JE_TRUE;
	}
							
	if (NumTempIndexVerts >= MAX_TEMP_INDEX_VERTS)
		return JE_FALSE;		// Whoops...

	// Insert the index in the temp index array, the poly will use this to re-construct itself
	TempIndexVerts[NumTempIndexVerts++] = p1;

	return JE_TRUE;
}

//=====================================================================================
//	FixFaceTJunctions
//=====================================================================================
static jeBoolean FixFaceTJunctions(jeBSPNode *Node, jeBSP *BSP, jeBSPNode_DrawFace *Face, jeVertArray_Optimizer *Optimizer)
{
	int32				i, NumVerts;
	jeVertArray_Index	v1, v2;
	int32				Start[MAX_TEMP_INDEX_VERTS];
	int32				Count[MAX_TEMP_INDEX_VERTS];
	jeFloat				Len;
	int32				Base;

	NumTempIndexVerts = 0;
	
	NumVerts = Face->Poly->NumVerts;

	g_BSP = BSP;

	for (i=0; i< NumVerts; i++)
	{
		v1 = Face->Poly->Verts[i];
		v2 = Face->Poly->Verts[(i+1)%NumVerts];

		// Get the 2 verts that define the edge
		pEdgeV1 = jeVertArray_GetVertByIndex(BSP->VertArray, v1);
		pEdgeV2 = jeVertArray_GetVertByIndex(BSP->VertArray, v2);

		// Find all the verts that lie inside the box formed by the edge
		jeVertArray_GetEdgeVerts(Optimizer, pEdgeV1, pEdgeV2, EdgeVerts, &NumEdgeVerts, JE_VERTARRAY_MAX_VERTS);

		// Get the edge dir
		jeVec3d_Subtract(pEdgeV2, pEdgeV1, &EdgeDir);
		// Get the length of the edge, and normalize the edge dir
		Len = jeVec3d_Normalize(&EdgeDir);

		Start[i] = NumTempIndexVerts;

		// Recursively start constructing the poly while inserting TJuncts, and preserve winding order
		if (!TestEdge_r(0.0f, Len, v1, v2, 0))
			return JE_FALSE;

		Count[i] = NumTempIndexVerts - Start[i];
	}

	if (NumTempIndexVerts < 3)
	{
		//jeIndexPoly_Destroy(&Face->Poly);
		return JE_TRUE;				// Face callapsed, mark it to be removed (by setting the poly to null)
	}

	// Try to find an edge that does not have any TJuncts on it, and use that as the first edge
	for (i=0; i< NumVerts; i++)
	{
		if (Count[i] == 1 && Count[(i+NumVerts-1)%NumVerts] == 1)
			break;
	}

	if (i == NumVerts)
		Base = 0;
	else
		Base = Start[i];

	if (!FinalizeFace(Face, Base, BSP))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_FixDrawFaceTJuncts_r
//=======================================================================================
jeBoolean jeBSPNode_FixDrawFaceTJuncts_r(jeBSPNode *Node, jeBSP *BSP, jeVertArray_Optimizer *Optimizer)
{
	int32				i, GoodFaces;
	jeBSPNode_DrawFace	*pFace;

	if (Node->Leaf)
		return JE_TRUE;

	GoodFaces = 0;

	// Go through each face, and fix TJuncts...
	for (i=0; i< Node->NumDrawFaces; i++)
	{
		pFace = Node->DrawFaces[i];

		if (!FixFaceTJunctions(Node, BSP, pFace, Optimizer))
			return JE_FALSE;

		if (pFace->Poly)
			GoodFaces++;
	}

	assert(GoodFaces <= Node->NumDrawFaces);		// NumFaces should have ONLY gotten smaller, NOT bigger

	// If the number of faces changed (faces got removed...), allocate a new array, and re-assign faces to the node
	if (GoodFaces != Node->NumDrawFaces)
	{
		jepBSPNode_DrawFace	*NewDrawFaces;
		int32				NumNewDrawFaces;

		NumNewDrawFaces = 0;

		NewDrawFaces = JE_RAM_ALLOCATE_ARRAY(jepBSPNode_DrawFace, GoodFaces);

		if (!NewDrawFaces)
			return JE_FALSE;

		// Faces got removed, reflect that change in the nodes DrawFace array
		for (i=0; i< Node->NumDrawFaces; i++)
		{
			pFace = Node->DrawFaces[i];

			if (!pFace->Poly)
			{
				jeBSPNode_DrawFaceDestroy(&pFace, BSP);
				continue;
			}

			NewDrawFaces[NumNewDrawFaces++] = pFace;
		}
		
		assert(NumNewDrawFaces == GoodFaces);		// They should be the same or something went wrong...

		jeRam_Free(Node->DrawFaces);
		Node->DrawFaces = NewDrawFaces;
	}	

	if (!jeBSPNode_FixDrawFaceTJuncts_r(Node->Children[NODE_FRONT], BSP, Optimizer))
		return JE_FALSE;

	if (!jeBSPNode_FixDrawFaceTJuncts_r(Node->Children[NODE_BACK], BSP, Optimizer))
		return JE_FALSE;

	return JE_TRUE;
}