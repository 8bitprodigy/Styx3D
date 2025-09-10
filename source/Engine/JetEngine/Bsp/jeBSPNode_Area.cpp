/****************************************************************************************/
/*  JEBSPNODE_AREA.C                                                                    */
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

// Private dependents
#include "jeBSP._h"
#include "Errorlog.h"
#include "Ram.h"
#include "VisObject.h"
#include "Log.h"

// Public dependents
#include "jeBSP.h"

//=======================================================================================
//	jeBSPNode_AreaCreate
//=======================================================================================
jeBSPNode_Area *jeBSPNode_AreaCreate(void)
{
	jeBSPNode_Area		*Area;

	Area = JE_RAM_ALLOCATE_STRUCT(jeBSPNode_Area);

	if (!Area)
		return NULL;

	ZeroMem(Area);

#ifdef AREA_DRAWFACE_TEST
	LN_Null(Area);

	Area->VisObjectList = NULL;
#endif

	if (!jeBSPNode_AreaCreateRef(Area))
	{
		jeRam_Free(Area);
		return NULL;
	}

	Area->LeafArray = jeArray_Create(sizeof(jeBSPNode_AreaLeaf), 10, 10);

	if (!Area->LeafArray)
		goto ExitWithError;

	Area->ObjectChain = jeChain_Create();

	if (!Area->ObjectChain)
		goto ExitWithError;

	return Area;

	ExitWithError:
	{
		if (Area)
		{
			if (Area->LeafArray)
				jeArray_Destroy(&Area->LeafArray);

			if (Area->ObjectChain)
				jeChain_Destroy(&Area->ObjectChain);

			jeRam_Free(Area);
		}

		return NULL;
	}
}

//=======================================================================================
//	jeBSPNode_AreaCreateRef
//=======================================================================================
jeBoolean jeBSPNode_AreaCreateRef(jeBSPNode_Area *Area)
{
	assert(Area);
	assert(Area->RefCount >= 0);		// 0 because this could be the first ref

	if (Area->RefCount >= (~(uint32)(0)))	// Good God!  Help them if they need more than this...
		return JE_FALSE;

	Area->RefCount++;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSPNode_AreaDestroy
//=======================================================================================
void jeBSPNode_AreaDestroy(jeBSPNode_Area **pArea)
{
	jeBSPNode_Area	*Area;

	assert(pArea);

	Area = *pArea;

	if (!Area)
		return;

	assert(Area->RefCount > 0);

	Area->RefCount--;

	if (Area->RefCount == 0)
	{
		if (Area->AreaPortals)
		{
			assert(Area->NumAreaPortals > 0);
			assert(Area->NumAreaPortals == Area->NumWorkAreaPortals);
		
			jeRam_Free(Area->AreaPortals);

			Area->AreaPortals = NULL;
			Area->NumAreaPortals = 0;
			Area->NumWorkAreaPortals = 0;
		}

		assert(Area->LeafArray);
		jeArray_Destroy(&(Area->LeafArray));

		assert(Area->ObjectChain);
		jeChain_Destroy(&Area->ObjectChain);

#ifdef AREA_DRAWFACE_TEST
		if ( Area->VisObjectList )
			List_Destroy(Area->VisObjectList);

		if ( Area->DrawFaces )
			jeRam_Free(Area->DrawFaces);
#endif
		jeRam_Free(Area);
	}

	*pArea = NULL;
}

#define MAX_PORTAL_VERTS	(JE_FRUSTUM_MAX_PLANES*2)

//=======================================================================================
//	jeBSPNode_AreaVisFlood
//=======================================================================================
jeBoolean jeBSPNode_AreaVisFlood_r(jeBSPNode_Area *Area, const jeVec3d *Pos, const jeFrustum *Frustum, uint32 RecursionBit, jeBSPNode_Area *FromArea)
{
	jeBSPNode_AreaPortal	*pPortal;
	int32					i;
	jeFrustum_ClipInfo		ClipInfo;
	jeVec3d					Work1[MAX_PORTAL_VERTS];		// Add padding
	jeVec3d					Work2[MAX_PORTAL_VERTS];
	jeBSPNode_AreaLeaf		*AreaLeaf;

	if (!(Area->RecursionBits & RecursionBit))
	{
		// Mark all leafs in the area to the current RecursionBit
		AreaLeaf = NULL;
		while (AreaLeaf = (jeBSPNode_AreaLeaf*)jeArray_GetNextElement(Area->LeafArray, AreaLeaf))
		{
			// Mark the leaf as visible
			jeBSPNode_LeafSetRecursionBit(AreaLeaf->Leaf, RecursionBit);
		}

		g_WorldDebugInfo.NumVisibleAreas++;	
	
	#if 0
		// Mark jeObjects as Vis
		if (Area->VisObjectList)
		{
			List			*Node;
			jeVisObject		*VO;

			for(Node = List_Next(Area->VisObjectList); Node != Area->VisObjectList; Node = List_Next(Node) )
			{
				assert(Node);

				VO = (jeVisObject *)List_NodeData(Node);
				assert(VO);

				//jeVisObject_SetRecursionBit(VO,Frustum,RecursionBit);
			}
		}
	#endif

		// Mark this area with the current RecursionBit
		Area->RecursionBits |= RecursionBit;
	}

	// Setup some of the clip info that won't change
	ClipInfo.ClipFlags = 0xFFFFFF;
	ClipInfo.Work1 = Work1;
	ClipInfo.Work2 = Work2;

	pPortal = Area->AreaPortals;

	for (i=0; i< Area->NumAreaPortals; i++, pPortal++)
	{
		jeFrustum			NewFrustum;
		jeBSPNode_Area		*OtherArea;
		float				Dist;

		OtherArea = (jeBSPNode_Area*)pPortal->Target;

		assert(OtherArea);
		assert(OtherArea != Area);

		if ( OtherArea == FromArea ) // <> don't go back to the source
			continue;

		Dist = jeVec3d_DotProduct(Pos, &pPortal->Plane.Normal) - pPortal->Plane.Dist;

		if (Dist >= 0)	
			continue;			// Don't go out front facing portals
		
		assert(pPortal->Poly->NumVerts <= JE_FRUSTUM_MAX_PLANES);

		ClipInfo.NumSrcVerts = pPortal->Poly->NumVerts;
		ClipInfo.SrcVerts = pPortal->Poly->Verts;

		// Clip the portal by all the frustum planes
		if (!jeFrustum_ClipVerts(Frustum, &ClipInfo))
			continue;		// Portal was clipped away

		// Create a new frustum from this new portal, and flood through it...
		jeFrustum_SetFromVerts(&NewFrustum, Pos, ClipInfo.DstVerts, ClipInfo.NumDstVerts);

		// Flow through the portals target (the area on the other side of the portal)
		if (!jeBSPNode_AreaVisFlood_r(OtherArea, Pos, &NewFrustum, RecursionBit, Area))
			return JE_FALSE;
	}

	return JE_TRUE;
}

#ifdef AREA_DRAWFACE_TEST
//=======================================================================================
//	jeBSPNode_AreaAddObject
//=======================================================================================
void jeBSPNode_AreaAddObject(jeBSPNode_Area *Area,jeVisObject *VO)
{

	if ( ! Area->VisObjectList )
	{
		Area->VisObjectList = List_Create();
		assert(Area->VisObjectList);
	}
	else
	{
		if ( List_Find(Area->VisObjectList,(uint32)VO) )
			return; // already have it !
	}

	jeVisObject_AddArea(VO,(uint32)Area);

	#ifdef _DEBUG
	{
		int l1,l2;

		l1 = List_Length(Area->VisObjectList);

		List_AddTail(Area->VisObjectList,(uint32)VO);

		l2 = List_Length(Area->VisObjectList);
		assert( l2 == (l1 + 1) );
	}
	#else
		List_AddTail(Area->VisObjectList,(uint32)VO);
	#endif
}

//=======================================================================================
//	jeBSPNode_AreaRemoveObject
//=======================================================================================
jeBoolean jeBSPNode_AreaRemoveObject(jeBSPNode_Area *Area,jeVisObject *VO)
{
List * VONode;

	if ( Area->VisObjectList == NULL )
		return JE_FALSE;

	#ifdef _DEBUG
	{
		int l1,l2;

		l1 = List_Length(Area->VisObjectList);
		assert( l1 > 0 );

		VONode = List_Find(Area->VisObjectList,(uint32)VO);
	
		if ( ! VONode )
			return JE_FALSE;

		assert(List_NodeData(VONode) == (uint32)VO);

		List_DeleteNode(VONode);

		l2 = List_Length(Area->VisObjectList);
		assert( l2 == (l1 - 1) );
	}
	#else
		VONode = List_Find(Area->VisObjectList,(uint32)VO);

		if ( ! VONode )
			return JE_FALSE;

		List_DeleteNode(VONode);
	#endif

return JE_TRUE;
}


//=======================================================================================
//	jeBSPNode_AreaMakeDrawFaces
//=======================================================================================
jeBoolean jeBSPNode_AreaMakeDrawFaces(jeBSPNode_Area *Area)
{
	jeBSPNode_AreaLeaf	*AreaLeaf;
	jeBSPNode_Leaf		*pLeaf;
	Hash				*DrawFaceHash;
	HashNode			*hn;
	jeBSPNode_DrawFace	*pFace;
	int					di;

	DrawFaceHash = Hash_Create();
	if (  ! DrawFaceHash )
		return JE_FALSE;

	assert(Area->NumDrawFaces == 0);
	assert(Area->DrawFaces == NULL);

	AreaLeaf = NULL;
	while( (AreaLeaf = (jeBSPNode_AreaLeaf *)jeArray_GetNextElement(Area->LeafArray,AreaLeaf) ) != NULL )
	{
	int f;
		pLeaf = AreaLeaf->Leaf;
		for(f=0;f<pLeaf->NumDrawFaces;f++)
		{
			pFace = pLeaf->DrawFaces[f];
			if ( ! Hash_Get(DrawFaceHash,(uint32)pFace,NULL) )
			{
				Hash_Add(DrawFaceHash,(uint32)pFace,(uint32)pFace);
				Area->NumDrawFaces ++;
			}
		}
	}

	Area->DrawFaces = jeRam_Allocate(Area->NumDrawFaces*sizeof(void *));
	if ( ! Area->DrawFaces )
	{
		Hash_Destroy(DrawFaceHash);
		return JE_FALSE;
	}

	hn = NULL;
	di = 0;
	while(hn = Hash_WalkNext(DrawFaceHash,hn) )
	{
		pFace = (jeBSPNode_DrawFace *) HashNode_Data(hn);
		assert( di < Area->NumDrawFaces );
		Area->DrawFaces[di++] = pFace;
	}

	Hash_Destroy(DrawFaceHash);

	Log_Printf("Area Drawfaces : %d\n",Area->NumDrawFaces);

	return JE_TRUE;
}
#endif

#ifdef AREA_DRAWFACE_TEST

//=======================================================================================
//	jeBSPNode_AreaVisFlood
//=======================================================================================
jeBoolean jeBSPNode_AreaRenderFlood_r(jeBSPNode_Area *Area, jeBSP *BSP, const jeCamera *Camera, const jeFrustum *Frustum, uint32 RecursionBit, jeBSPNode_Area *FromArea)
{
	jeBSPNode_AreaPortal	*pPortal;
	int32					i;
	jeFrustum_ClipInfo		ClipInfo;
	jeVec3d					Work1[MAX_PORTAL_VERTS];		// Add padding
	jeVec3d					Work2[MAX_PORTAL_VERTS];
	uint32					ClipFlags;
	jeBSPNode_SceneInfo		SceneInfo;

	assert(0);		// This stuff is not ready for prime time...

	if (!(Area->RecursionBits & RecursionBit))
		g_WorldDebugInfo.NumVisibleAreas++;	

	// Mark this area with the current RecursionBit
	Area->RecursionBits |= RecursionBit;

	// Render it !
	SceneInfo.Camera = (jeCamera*)Camera;
	SceneInfo.Frustum = (jeFrustum*)Frustum;
	SceneInfo.RecursionBit = RecursionBit;

	//@@ set up area clip flags!
	ClipFlags = (1UL<<Frustum->NumPlanes)-1;

	for(i=0;i<Area->NumDrawFaces;i++)
	{
		/**

		disadvantages :
			1. we don't have the heirarchical clipflags
			2. backfacing the drawfaces

		***/
		// @@ do ExtBox clipflags thing for each draw face !!
		//	jeFrustum_SetClipFlagsFromExtBox()
		//	faster than doing the frustum clips cuz we avoid the memcpys of all the verts
		jeBSPNode_DrawFaceRender(Area->DrawFaces[i], BSP, &SceneInfo, ClipFlags);
	}

	// Render jeObjects
	if ( Area->VisObjectList )
	{
		List		*Node;
		jeVisObject	*VO;
	
		for(Node = List_Next(Area->VisObjectList); Node != Area->VisObjectList; Node = List_Next(Node) )
		{
			assert(Node);

			VO = (jeVisObject *)List_NodeData(Node);
			assert(VO);

			jeVisObject_Render(VO, Frustum, RecursionBit);
		}
	}

	// Setup some of the clip info that won't change
	ClipInfo.ClipFlags = 0xFFFFFF;
	ClipInfo.Work1 = Work1;
	ClipInfo.Work2 = Work2;

	pPortal = Area->AreaPortals;

	for (i=0; i< Area->NumAreaPortals; i++, pPortal++)
	{
		jeFrustum			NewFrustum;
		jeBSPNode_Area		*OtherArea;
		float				Dist;

		OtherArea = (jeBSPNode_Area*)pPortal->Target;

		assert(OtherArea);
		assert(OtherArea != Area);

		if ( OtherArea == FromArea ) // don't go back to the source
			continue;

		Dist = jeVec3d_DotProduct(jeCamera_GetPov(Camera), &pPortal->Plane.Normal) - pPortal->Plane.Dist;

		if (Dist >= 0)	
			continue;			// Don't go out front facing portals
		
		assert(pPortal->Poly->NumVerts <= MAX_PORTAL_VERTS);

		ClipInfo.NumSrcVerts = pPortal->Poly->NumVerts;
		ClipInfo.SrcVerts = pPortal->Poly->Verts;

		// Clip the portal by all the frustum planes
		if (!jeFrustum_ClipVerts(Frustum, &ClipInfo))
			continue;		// Portal was clipped away

		// Create a new frustum from this new portal, and flood though it...
		jeFrustum_SetFromVerts(&NewFrustum, jeCamera_GetPov(Camera), ClipInfo.DstVerts, ClipInfo.NumDstVerts);

		// Flow through the portals target (the area on the other side of the portal)
		if (!jeBSPNode_AreaRenderFlood_r(OtherArea, BSP, Camera, &NewFrustum, RecursionBit, Area))
			return JE_FALSE;
	}

	return JE_TRUE;
}
#endif

jeBoolean jeBSPNode_AreaRenderVertexBuffer(jeBSPNode_Area* Area, jeBSP* Tree)
{
	jeChain_Link* pLink;
	
	// Enumerate all sections
	for (pLink = jeChain_GetFirstLink(Area->RenderDataList); pLink; pLink = jeChain_LinkGetNext(pLink))
	{
		jeRenderSectionData* pSection;
		pSection = (jeRenderSectionData*) jeChain_LinkGetLinkData(pLink);

		// Render the current section
		//Tree->Driver->Render(Tree->pVertexBuffer, Tree->pIndexBuffer, pSection); // TBD
	}
	return JE_TRUE;
}
