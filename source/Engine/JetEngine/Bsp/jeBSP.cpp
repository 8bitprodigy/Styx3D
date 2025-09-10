/****************************************************************************************/
/*  JEBSP.C                                                                             */
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
#include <stdio.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef BUILD_BE
#define min(a,b) (((a)<(b))?(a):(b))
#endif
 
// Private dependents
#include "jeBSP._h"
#include "Ram.h"
#include "Vec3d.h"
#include "Errorlog.h"
#include "Log.h"

#include "math.h"
#include "Camera._h"

// Public dependents
#include "jeBSP.h"

#ifdef _DEBUG
	#define JE_BSP_DEBUG_OUTPUT_LEVEL		1
	//#define JE_BSP_DEBUG_OUTPUT_LEVEL		2
#else
	#define JE_BSP_DEBUG_OUTPUT_LEVEL		0
#endif

//--- Local global statics
static jeBSP_Logic			g_Logic;
static jeBSP_LogicBalance	g_LogicBalance;		// (0..10), 0 = Less splits, 10 = Balanced tree
static int32				NumNonVisNodes;

// extern data for stats purpose - Krouer
extern int32 NumMakeFaces;
extern int32 NumMergedFaces;
extern int32 NumSubdividedFaces;

//=======================================================================================
// Local Static function prototypes (ALL Static functions at BOTTOM of file)
//=======================================================================================
static jeBoolean	jeBSP_AddBSPBrush_r(jeBSP *BSP, jeBSPNode *Node, jeBSP_Brush **Brush);
static jeBoolean	jeBSP_AddBrushInternally(jeBSP *BSPTree, jeBrush *Brush, uint32 Order, jeBoolean AutoLight);
static jeBoolean	jeBSP_BuildBSPTree(	jeBSP *BSPTree, 
										jeBSP_Brush **BrushList, 
										jeBSP_Logic Logic, 
										jeBSP_LogicBalance LogicBalance);
static jeBoolean	jeBSP_BuildBSPTree_r(jeBSP *BSP, jeBSPNode *Node, jeBSP_Brush **Brushes);
static jePlane_Side jeBSP_TestBrushToPlaneIndex(jeBSP *BSP, jeBSP_Brush *Brush, jePlaneArray_Index Index, int32 *NumSplits, 
												jeBoolean *HintSplit, int32 *EpsilonBrush);

static jeBSP_Side	*jeBSP_GetSplitter(jeBSP *BSP, jeBSP_Brush *Brushes, jeBSPNode *Node);
static jeBoolean	jeBSP_CheckPlaneAgainstVolume(jeBSP *BSP, jePlaneArray_Index Index, jeBSPNode *Node);
static jeBoolean	jeBSP_CheckPlaneAgainstParents(jePlaneArray_Index Index, jeBSPNode *Node);
static jeBoolean	jeBSP_FillLeafsFromEntities(jeBSP *Tree, int32 Fill);
static jeBoolean	jeBSP_RemoveHiddenLeafs(jeBSP *Tree);
static jeBoolean	jeBSP_MarkVisibleTopSides(jeBSP *Tree);
static jeBoolean	jeBSP_CreatePortals(jeBSP *Tree, jeBoolean IncludeDetail);
static void			jeBSP_DestroyAllPortals(jeBSP *Tree);
static void			jeBSP_MarkVisibleTopSides_r(jeBSP *BSP, jeBSPNode *Node);

#define LIGHT_FRACT			8
#define CSCALE				(1.0f/195.0f)
//#define COLOR_TO_FIXED(c)	((int32)(((c)*CSCALE)*(1<<LIGHT_FRACT)))
#define COLOR_TO_FIXED(c)	((int32)((c)*(1<<LIGHT_FRACT)))

static jeBoolean	CombineDLightWithRGBMapFastLightingModel(jeBSP *BSP, int32 *LightData, jeBSPNode_Light *Light, jeBSPNode_DrawFace *Face);
static jeBoolean	CombineDLightWithRGBMap(jeBSP *BSP, int32 *LightData, jeBSPNode_Light *Light, jeBSPNode_DrawFace *Face);
static void			AddLightType1(int32 *LightDest, uint8 *LightData, int32 Size, int32 Intensity);
static void			AddLightType2(int32 *LightDest, uint8 *LightData, int32 Size, int32 Intensity);
static void	JETCC	jeBSP_SetupLightmap(jeRDriver_LMapCBInfo *Info, void *LMapCBContext);
static jeBoolean	UpdateDLights(jeBSP *BSP);
static jeBoolean	UpdateObjects(jeBSP *BSP);

static jeBoolean	JETCC ShutdownDriverCB(DRV_Driver *Driver, void *Context);
static jeBoolean	JETCC StartupDriverCB(DRV_Driver *Driver, void *Context);

static jeBoolean	jeBSP_UpdateWorldSpaceBox(jeBSP *BSP);

static jeBoolean	jeBSP_CreateInternalArrays(jeBSP *BSP);
static void			jeBSP_DestroyInternalArrays(jeBSP *BSP);
static void			jeBSP_ResetObjects(jeBSP *BSP);
static jeBoolean	jeBSP_ResetGeometry(jeBSP *BSP);
static void			jeBSP_DestroyExternalArrays(jeBSP *BSP);
static jeBoolean	jeBSP_CreateExternalArrays(jeBSP *BSP, jeFaceInfo_Array *FArray, jeMaterial_Array *MArray, jeChain *LChain, jeChain *DLChain);

static jeBoolean	jeBSP_OptimizeDrawFaceVerts(jeBSP *BSP);
static jeBoolean	jeBSP_CreateVertexBuffer(jeBSP* BSP);
static jeBoolean	jeBSP_RenderVertexBuffer(jeBSP* BSP);

//=======================================================================================
//	jeActor_SetRenderNextTime
//	An accessor the actor render flag
//=======================================================================================
extern JETAPI void JETCC jeActor_SetRenderNextTime(jeActor* Actor, jeBoolean RenderNextTime);


//=======================================================================================
//	jeBSP_Create 
//	Create an "empty" BSP tree
//=======================================================================================
jeBSP *jeBSP_Create(void)
{
	jeBSP		*BSPTree;
	jeXForm3d	IdentityXForm;

	BSPTree = JE_RAM_ALLOCATE_STRUCT(jeBSP);

	if (!BSPTree)
		return NULL;

	ZeroMem(BSPTree);

#ifdef AREA_DRAWFACE_TEST
	LN_InitList(&(BSPTree->AreaList));
	LN_InitList(&(BSPTree->LeafList));
#endif

	if (!jeBSP_CreateInternalArrays(BSPTree))
		goto ExitWithError;

	BSPTree->BSPObjectChain = jeChain_Create();

	if (!BSPTree->BSPObjectChain)
		goto ExitWithError;

	// Set default XForm
	jeXForm3d_SetIdentity(&IdentityXForm);
	jeBSP_SetXForm(BSPTree, &IdentityXForm);

	// Set other default stuff
	BSPTree->RenderMode = RenderMode_TexturedAndLit;
	BSPTree->DefaultContents = JE_BSP_CONTENTS_SOLID;

	return BSPTree;

	// Error
	ExitWithError:
	{
		if (BSPTree)
		{
			if (BSPTree->BSPObjectChain)
				jeChain_Destroy(&BSPTree->BSPObjectChain);

			jeRam_Free(BSPTree);
		}

		return NULL;
	}
}

//=======================================================================================
//	jeBSP_RebuildGeometry
//	Creates a fresh tree from a list of brushes
//=======================================================================================
jeBSP *jeBSP_RebuildGeometry(	jeBSP				*BSP,
								jeChain				*BrushChain, 
								jeBSP_Options		Options,
								jeBSP_Logic			Logic, 
								jeBSP_LogicBalance	LogicBalance)
{
	jeBSP_Brush		*BSPBrushList;

	assert(BSP);
	assert(BrushChain);

	// Destroy any geometry the BSP might have, and reset arrays
	if (!jeBSP_ResetGeometry(BSP))
		return NULL;

	BSPBrushList = NULL;

	// Set some globals
	g_Logic = Logic;
	g_LogicBalance = LogicBalance;

	BSP->NumBrushes = 0;

	// Create the TopLevel brushes, from the editor jeBrushes...
	BSP->TopBrushes = jeBSP_TopBrushCreateListFromBrushChain(BrushChain, BSP, &BSP->NumBrushes);

	if (!BSP->TopBrushes)
		goto ExitWithError;

	// Create the brushes that will get cut up in the tree
	BSPBrushList = jeBSP_BrushCreateListFromTopBrushList(BSP->TopBrushes, BSP);
	
	if (!BSPBrushList)
		goto ExitWithError;

	// CSG this list 
	if (Options & BSP_OPTIONS_CSG_BRUSHES)
	{
		BSPBrushList = jeBSP_BrushCSGList(BSPBrushList, BSP);

		if (!BSPBrushList)
			goto ExitWithError;
	}

	// Now, build the tree
	if (!jeBSP_BuildBSPTree(BSP, &BSPBrushList, Logic, LogicBalance))
		goto ExitWithError;

	// Create portals
	if (!jeBSP_CreatePortals(BSP, JE_TRUE))
		goto ExitWithError;
		
	// Remove hidden leafs
	//if (!jeBSP_RemoveHiddenLeafs(BSP))
	//	goto ExitWithError;

	// Use the tree to mark the visible sides
	if (!jeBSP_MarkVisibleTopSides(BSP))
		goto ExitWithError;

	// Merge nodes
	//jeBSPNode_MergeLeafs(BSP->RootNode);

	// Make collision hulls for leafs
	if (!jeBSPNode_UpdateLeafSides_r(BSP->RootNode, BSP))
		goto ExitWithError;

	if (Options & BSP_OPTIONS_MAKE_VIS_AREAS)
	{
		if (!jeBSPNode_MakeFaces_Callr(BSP->RootNode, BSP, JE_FALSE))
			goto ExitWithError;

		if (!jeBSPNode_MakeDrawFaceListOnLeafs_r(BSP->RootNode, BSP))
			goto ExitWithError;

		// We MUST destroy the chain of merged/split faces fragments as soon as we are done with them
		jeBSPNode_DestroyBSPFaces_r(BSP->RootNode, BSP, JE_TRUE);
		
		if (!jeBSP_MakeVisAreas(BSP))
			goto ExitWithError;

#ifdef AREA_DRAWFACE_TEST
		if ( 1 ) // @@
		{
			if (!jeBSP_MakeAreaDrawFaces(BSP))
				goto ExitWithError;
		}
#endif
	}
	else
	{
		if (!jeBSPNode_MakeFaces_Callr(BSP->RootNode, BSP, JE_TRUE))
			goto ExitWithError;
	}

#ifdef AREA_DRAWFACE_TEST
	Log_Printf("Area List Len = %d\n",LN_ListLen(&(BSP->AreaList)));
#endif

	if (!jeBSP_UpdateWorldSpaceBox(BSP))
		return NULL;

	if (!jeBSP_OptimizeDrawFaceVerts(BSP))
		return NULL;

// Krouer: put here the call to the Vertex buffer creation function
	if (BSP->Driver) 
	{
		jeDeviceCaps devcaps;
		BSP->Driver->GetDeviceCaps(&devcaps);
		if ((devcaps.SuggestedDefaultRenderFlags & JE_RENDER_FLAG_HWTRANSFORM) == JE_RENDER_FLAG_HWTRANSFORM) {
			if (!jeBSP_CreateVertexBuffer(BSP)) {
				return NULL;
			}
		}
	}
	return BSP;

	// Error
	ExitWithError:
	{
		if (BSP->RootNode)
			jeBSPNode_Destroy_r(&BSP->RootNode, BSP);

		if (BSP->TopBrushes)
			jeBSP_TopBrushDestroyList(&BSP->TopBrushes, BSP);

		if (BSPBrushList)
			jeBSP_BrushDestroyList(&BSPBrushList);

		return NULL;
	}
}

//=======================================================================================
//	jeBSP_Destroy
//=======================================================================================
void jeBSP_Destroy(jeBSP **BSPTree)
{
	assert(BSPTree);
	assert(*BSPTree);

	// Detach from the engine (if any)
	if ((*BSPTree)->Engine)
	{
		assert((*BSPTree)->ChangeDriverCB);
		jeEngine_DestroyChangeDriverCB((*BSPTree)->Engine, &(*BSPTree)->ChangeDriverCB);
		jeEngine_Free((*BSPTree)->Engine);
		(*BSPTree)->Engine = NULL;
	}
	else
	{
		assert(!(*BSPTree)->ChangeDriverCB);
		assert(!(*BSPTree)->Driver);
	}

	// First, make sure no portals are in tree
	jeBSP_DestroyAllPortals(*BSPTree);	

	// Destroy arrays, and anything that depends on them...
	jeBSP_DestroyInternalArrays(*BSPTree);

	// Destroy all externally set arrays
	jeBSP_DestroyExternalArrays(*BSPTree);

	// Destroy the BSPOBject list
	if ((*BSPTree)->BSPObjectChain)
	{
		jeChain_Link	*Link;

		for (Link = jeChain_GetFirstLink((*BSPTree)->BSPObjectChain); Link; Link = jeChain_LinkGetNext(Link))
		{
			jeBSP_Object		*BSPObject;

			BSPObject = (jeBSP_Object*)jeChain_LinkGetLinkData(Link);

			jeObject_Destroy(&BSPObject->Object);

			// [MLB-ICE]
			jeRam_Free(BSPObject);	// Icestorm: "Container" should be freed!
			// [MLB-ICE] EOB

		}

		jeChain_Destroy(&(*BSPTree)->BSPObjectChain);
	}

	// Free the bsp structure
	jeRam_Free(*BSPTree);

	// NULL out their pointer
	*BSPTree = NULL;

	// Print some degbue info...
	Log_Printf("--- jeBSP_Destroy ---\n");
	Log_Printf("Num Active BSPBrushes  : %5i\n", jeBSP_BrushGetActiveCount());
	Log_Printf("Num Peek BSPBrushes    : %5i\n", jeBSP_BrushGetPeekCount());
	Log_Printf("Num Active BSPFaces    : %5i\n", jeBSPNode_FaceGetActiveCount());
	Log_Printf("Num Peek BSPFaces      : %5i\n", jeBSPNode_FaceGetPeekCount());
	Log_Printf("Num Active Portals     : %5i\n", jeBSPNode_PortalGetActiveCount());
	Log_Printf("Num Peek Portals       : %5i\n", jeBSPNode_PortalGetPeekCount());
}

//=====================================================================================
//	jeBSP_SetArrays
//=====================================================================================
jeBoolean jeBSP_SetArrays(jeBSP *BSP, jeFaceInfo_Array *FArray, jeMaterial_Array *MArray, jeChain *LChain, jeChain *DLChain)
{
	assert(BSP);

	// Check to see if the arrays changed
	if (BSP->FaceInfoArray == FArray && 
		BSP->MaterialArray == MArray &&
		BSP->LightChain == LChain && 
		BSP->DLightChain == DLChain)
			return JE_TRUE;			// Nothing changed

	// Destroy ALL geometry, and arrays
	if (!jeBSP_ResetGeometry(BSP))
		return JE_FALSE;			

	// Destroy all externally set arrays
	jeBSP_DestroyExternalArrays(BSP);
		
	if (FArray)
		jeBSP_CreateExternalArrays(BSP, FArray, MArray, LChain, DLChain);
	else
	{
		assert(!FArray);		
		assert(!MArray);
		assert(!LChain);
		assert(!LChain);
	}

	return JE_TRUE;
}

//=====================================================================================
//	jeBSP_AddBrush
//=====================================================================================
jeBoolean jeBSP_AddBrush(jeBSP *BSPTree, jeBrush *Brush, jeBoolean AutoLight)
{
	assert(BSPTree);
	assert(Brush);

	if (!jeBSP_AddBrushInternally(BSPTree, Brush, BSPTree->NumBrushes, AutoLight))
		return JE_FALSE;

	BSPTree->NumBrushes++;

	return JE_TRUE;
}

//=====================================================================================
//	jeBSP_HasBrush
//=====================================================================================
jeBoolean jeBSP_HasBrush(jeBSP *BSPTree, jeBrush *Brush)
{
	jeBSP_TopBrush		*TopBrush;

	assert(BSPTree);
	assert(Brush);

	if (!BSPTree->RootNode)
		return JE_FALSE;

	for (TopBrush = BSPTree->TopBrushes; TopBrush; TopBrush = TopBrush->Next)
	{
		if (TopBrush->Original == Brush)
			return JE_TRUE;
	}

	return JE_FALSE;
}

//=====================================================================================
//	jeBSP_RemoveBrush
//=====================================================================================
jeBoolean jeBSP_RemoveBrush(jeBSP *BSPTree, jeBrush *Brush)
{
	jeBSP_TopBrush		*TopBrush, *NewList, *Next;
	jeBoolean			Found;

	assert(BSPTree);
	assert(Brush);
	assert(jeBSP_HasBrush(BSPTree, Brush));

	if (BSPTree->AreaChain)
	{
		if (!jeBSP_DestroyVisAreas(BSPTree))
			return JE_FALSE;
	}

	if (!BSPTree->RootNode)
		return JE_FALSE;

	NewList = NULL;
	Found = JE_FALSE;
	
	// Find all the topbrushes that are a apart of the editor brush, and remove them
	for (TopBrush = BSPTree->TopBrushes; TopBrush; TopBrush = Next)
	{
		Next = TopBrush->Next;

		if (TopBrush->Original == Brush)
		{
			Found = JE_TRUE;
			
			if (!jeBSPNode_RemoveTopBrush_r(BSPTree->RootNode, BSPTree, TopBrush))
				return JE_FALSE;
			
			jeBSP_TopBrushDestroy(&TopBrush, BSPTree);
			continue;
		}

		TopBrush->Next = NewList;
		NewList = TopBrush;
	}

	BSPTree->TopBrushes = NewList;
   	
	if (!Found)
		return JE_FALSE;		// Brush not in tree!!

	BSPTree->UpdateFlags |= BSP_UPDATE_FACES;
	BSPTree->UpdateFlags |= BSP_UPDATE_LIGHTS;

	BSPTree->DebugInfo.NumBrushes--;

	return JE_TRUE;
}

//=======================================================================================
//	AllocateAreaPortals
//=======================================================================================
static jeBoolean AllocateAreaPortals(jeBSP *BSP)
{
	jeChain_Link		*Link;

	for (Link = jeChain_GetFirstLink(BSP->AreaChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeBSPNode_Area	*Area;

		Area = (jeBSPNode_Area*)jeChain_LinkGetLinkData(Link);

		// If array has not been allocated, then allocate it now...
		assert(!Area->AreaPortals);
		assert(!Area->NumWorkAreaPortals);

		if (!Area->NumAreaPortals)
			continue;

		Area->AreaPortals = JE_RAM_ALLOCATE_ARRAY(jeBSPNode_AreaPortal, Area->NumAreaPortals);

		if (!Area->AreaPortals)
			return JE_FALSE;

		ZeroMemArray(Area->AreaPortals, Area->NumAreaPortals);
	}

	return JE_TRUE;
}

//=======================================================================================
//	CheckAreaPortals
//=======================================================================================
static void CheckAreaPortals(jeBSP *BSP)
{
	jeChain_Link		*Link;

	for (Link = jeChain_GetFirstLink(BSP->AreaChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeBSPNode_Area	*Area;

		Area = (jeBSPNode_Area*)jeChain_LinkGetLinkData(Link);

		assert(Area->NumAreaPortals == Area->NumWorkAreaPortals);
	}
}

//=======================================================================================
//	jeBSP_MakeVisAreas
//=======================================================================================
jeBoolean jeBSP_MakeVisAreas(jeBSP *BSPTree)
{
	assert(BSPTree);

	if (!BSPTree->RootNode)
		return JE_TRUE;

	Log_Printf(" --- jeBSP_MakeVisAreas ---\n");

	BSPTree->DebugInfo.NumAreas = 0;

	assert(!BSPTree->AreaChain);

	BSPTree->AreaChain = jeChain_Create();

	if (!BSPTree->AreaChain)
		return JE_FALSE;

	// Destroy any existing areas
	if (!jeBSPNode_DestroyAreas_r(BSPTree->RootNode))
		return JE_FALSE;

	// Make the AreaChain
	if (!jeBSPNode_MakeAreas_r(BSPTree->RootNode, BSPTree, BSPTree->AreaChain))
		return JE_FALSE;

	// First, count the portals on the areas
	if (!jeBSPNode_CountAreaVisPortals_r(BSPTree->RootNode))
		return JE_FALSE;

	// Allocate portals arrays on the areas
	if (!AllocateAreaPortals(BSPTree))
		return JE_FALSE;

	// Then, make the portals on the areas
	if (!jeBSPNode_MakeAreaVisPortals_r(BSPTree->RootNode, BSPTree))
		return JE_FALSE;

	// Finally, verify that counts match what was made (This code only asserts, so no error checking)
	CheckAreaPortals(BSPTree);

	Log_Printf("Num Vis Areas      : %5i\n", BSPTree->DebugInfo.NumAreas);

#ifdef AREA_DRAWFACE_TEST
	assert( BSPTree->DebugInfo.NumAreas == LN_ListLen(&(BSPTree->AreaList)));
#endif

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_DestroyVisAreas
//=======================================================================================
jeBoolean jeBSP_DestroyVisAreas(jeBSP *BSP)
{
	jeChain_Link		*Link;

	if (!BSP->AreaChain)
		return JE_TRUE;

	assert(BSP->RootNode);

	// Reset the BSP object list (they depend on the areas being valid)
	jeBSP_ResetObjects(BSP);

	if (!jeBSPNode_DestroyAreas_r(BSP->RootNode))
		return JE_FALSE;

	for (Link = jeChain_GetFirstLink(BSP->AreaChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeBSPNode_Area	*Area;

		Area = (jeBSPNode_Area*)jeChain_LinkGetLinkData(Link);

		assert(Area->RefCount == 1);

		jeBSPNode_AreaDestroy(&Area);
	}

	jeChain_Destroy(&BSP->AreaChain);

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_AddObject
//=======================================================================================
jeBoolean jeBSP_AddObject(jeBSP *BSP, jeObject *Object)
{
	jeBSP_Object	*BSPObject;
	jeXForm3d		XForm;

	assert(!jeBSP_HasObject(BSP, Object));

	BSPObject = JE_RAM_ALLOCATE_STRUCT(jeBSP_Object);

	if (!BSPObject)
		return JE_FALSE;

	ZeroMem(BSPObject);

	BSPObject->Object = Object;

	// For now, just simply use the translation of the object to get an area
	//	we will need to eventually use the box, and possibly occupy more than one
	//	area, but this will do for now
	jeObject_GetXForm(BSPObject->Object, &XForm);

	BSPObject->Area = jeBSP_FindArea(BSP, &XForm.Translation);

	if (BSPObject->Area)
	{
		// Add the object to the list of objects in the area
		assert(!jeChain_FindLink(BSPObject->Area->ObjectChain, Object));
		jeChain_AddLinkData(BSPObject->Area->ObjectChain, Object);
	}

	jeChain_AddLinkData(BSP->BSPObjectChain, BSPObject);

	jeObject_CreateRef(Object);

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_RemoveObject
//=======================================================================================
jeBoolean jeBSP_RemoveObject(jeBSP *BSP, jeObject *Object)
{
	jeChain_Link	*Link;

	assert(jeBSP_HasObject(BSP, Object));

	for (Link = jeChain_GetFirstLink(BSP->BSPObjectChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeBSP_Object		*BSPObject;

		BSPObject = (jeBSP_Object*)jeChain_LinkGetLinkData(Link);

		if (BSPObject->Object != Object)
			continue;

		// If the Object has an area, remove it from the areas list
		if (BSPObject->Area)
		{
			assert(jeChain_FindLink(BSPObject->Area->ObjectChain, BSPObject->Object));
			jeChain_RemoveLinkData(BSPObject->Area->ObjectChain, BSPObject->Object);
			BSPObject->Area = NULL;
		}

		// De-ref the object
		jeObject_Destroy(&BSPObject->Object);

		// Remove the object from the chain
		assert(jeChain_FindLink(BSP->BSPObjectChain, BSPObject));
		jeChain_RemoveLinkData(BSP->BSPObjectChain, BSPObject);

		// Free the bsp object
		jeRam_Free(BSPObject);

		return JE_TRUE;
	}
	
	assert(0);
	return JE_FALSE;		
}

//=======================================================================================
//	jeBSP_HasObject
//=======================================================================================
jeBoolean jeBSP_HasObject(jeBSP *BSP, jeObject *Object)
{
	jeChain_Link	*Link;

	for (Link = jeChain_GetFirstLink(BSP->BSPObjectChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeBSP_Object		*BSPObject;

		BSPObject = (jeBSP_Object*)jeChain_LinkGetLinkData(Link);

		if (BSPObject->Object == Object)
			return JE_TRUE;
	}

	return JE_FALSE;
}

//=======================================================================================
//	Rendering/Vis
//=======================================================================================

//=======================================================================================
//	jeBSP_VisFrame
//=======================================================================================
jeBoolean jeBSP_VisFrame(jeBSP *BSPTree, const jeCamera *Camera, const jeFrustum *ModelSpaceFrustum)
{
	jeBSPNode_Area		*Area;
	jeVec3d				POV;

	assert(BSPTree);

	if (!BSPTree->RootNode)
		return JE_TRUE;

	if (!BSPTree->AreaChain)
		return JE_TRUE;

	// Get the POV by taking WorldSpace camera pos, and rotaing into model space
	jeXForm3d_Transform(&BSPTree->WorldToModelXForm, jeCamera_GetPov(Camera), &POV);

	Area = jeBSPNode_FindArea(BSPTree->RootNode, BSPTree, &POV);

	if (!Area)
	{
		// Not in a valid vis area
		// find *something* to mark as vis!
		Area = jeBSPNode_FindClosestArea(BSPTree->RootNode, BSPTree, &POV);

		if (!Area) // no areas in the whole BSP !
			return JE_TRUE;
	}

	assert(BSPTree->AreaChain);

	if (!jeBSPNode_AreaVisFlood_r(Area, &POV, ModelSpaceFrustum, (1<<BSPTree->RenderRecursion), Area))
	{
	
//		return JE_FALSE;
	}
	
	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_RenderFrontToBack
//=======================================================================================
jeBoolean jeBSP_RenderFrontToBack(jeBSP *Tree, jeCamera *Camera, jeFrustum *CameraSpaceFrustum, jeFrustum *ModelSpaceFrustum, jeXForm3d *ModelToCameraXForm)
{
	uint32				ClipFlags;
	jeBSPNode_SceneInfo	SceneInfo;
	DRV_Driver			*Driver;

	assert(Tree);
	assert(Tree->Engine);
	assert(Camera);
	assert(ModelSpaceFrustum);

	if (!Tree->RootNode)
		return JE_TRUE;

	if (!Tree->Engine)
		return JE_TRUE;

#if (JE_BSP_DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("BEGIN jeBSP_RenderFrontToBack\n");
#endif

	Driver = jeEngine_GetDriver(Tree->Engine);
	assert(Driver);
	Driver->SetupLightmap = jeBSP_SetupLightmap;

	// Fill in the SceneInfo struct
	SceneInfo.Camera = Camera;
	
	// Get ModelToCameraXForm by combining ModelToWorld, and WorldToCamera together
	SceneInfo.ModelToCameraXForm = *ModelToCameraXForm;

	// Get the POV by taking WorldSpace camera pos, and rotaing into model space
	jeXForm3d_Transform(&Tree->WorldToModelXForm, jeCamera_GetPov(Camera), &SceneInfo.POV);

	SceneInfo.Frustum = ModelSpaceFrustum;

	if (Tree->AreaChain)
		SceneInfo.RecursionBit = (1<<Tree->RenderRecursion);
	else
		SceneInfo.RecursionBit = 0;


	jeEngine_GetDefaultRenderFlags(Tree->Engine, &SceneInfo.DefaultRenderFlags);

	// Setup clipflags
	ClipFlags = (1<<ModelSpaceFrustum->NumPlanes)-1;

	Tree->RenderRecursion++;

	NumMakeFaces = 0;

	jeBSPNode_RenderFrontToBack_r(Tree->RootNode, Tree, &SceneInfo, ClipFlags);

	assert(Tree->RenderRecursion > 0);
	Tree->RenderRecursion--;

	if (Tree->AreaChain)
	{
		jeChain_Link	*Link;

		for (Link = jeChain_GetFirstLink(Tree->AreaChain); Link; Link = jeChain_LinkGetNext(Link))
		{
			jeBSPNode_Area		*Area;
			jeChain_Link		*Link2;

			Area = (jeBSPNode_Area*)jeChain_LinkGetLinkData(Link);
			
			if (!(Area->RecursionBits & SceneInfo.RecursionBit))
				continue;

			// Krouer: I can render the area here
			{
				jeDeviceCaps devcaps;
				Tree->Driver->GetDeviceCaps(&devcaps);
				if ((devcaps.SuggestedDefaultRenderFlags & JE_RENDER_FLAG_HWTRANSFORM) == JE_RENDER_FLAG_HWTRANSFORM) {
					jeBSPNode_AreaRenderVertexBuffer(Area, Tree);
				}
			}

			// The Area is visible, render all objects inside the area
			//	NOTE - For now, objects only live in one area at a time, so we
			//	always render them.  As soon as they are allowed to live in more than
			//	one area, we will have to take this into account, and make sure we only render
			//	them once...
			for (Link2 = jeChain_GetFirstLink(Area->ObjectChain); Link2; Link2 = jeChain_LinkGetNext(Link2))
			{
				jeObject		*Object;
				jeObject_Type	objectType;

				Object = (jeObject*)jeChain_LinkGetLinkData(Link2);
				objectType = jeObject_GetType(Object);

				if (JE_OBJECT_TYPE_ACTOR==objectType || JE_OBJECT_TYPE_TERRAIN==objectType) {
					jeObject_SetRenderNextPass(Object, JE_TRUE);
					//jeActor_SetRenderNextTime((jeActor*) Object->Instance, JE_TRUE);
				} else {
					// Render the object
					if (!jeObject_Render(Object, Tree->World, Tree->Engine, Camera, CameraSpaceFrustum, 0))
						return JE_FALSE;
				}
			}

			// Remove the vis recursion bit
			Area->RecursionBits ^= SceneInfo.RecursionBit;
		}
	}
	else		// Render every object, since there is no areas...
	{
		jeChain_Link	*Link;

		OutputDebugString("BSP: before render Objects - no AreaChain\n");
		{
			jeDeviceCaps devcaps;
			Tree->Driver->GetDeviceCaps(&devcaps);
			if ((devcaps.SuggestedDefaultRenderFlags & JE_RENDER_FLAG_HWTRANSFORM) == JE_RENDER_FLAG_HWTRANSFORM) {
				jeBSP_RenderVertexBuffer(Tree);
			}
		}

		for (Link = jeChain_GetFirstLink(Tree->BSPObjectChain); Link; Link = jeChain_LinkGetNext(Link))
		{
			//jeObject		*Object;
			jeBSP_Object		*BSPObject;

			BSPObject = (jeBSP_Object*)jeChain_LinkGetLinkData(Link);
		
			// Render the object
			if (!jeObject_Render(BSPObject->Object, Tree->World, Tree->Engine, Camera, CameraSpaceFrustum, 0))
				return JE_FALSE;
		}
	}

#if (JE_BSP_DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("END jeBSP_RenderFrontToBack\n");
#endif

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_RenderAndVis
//=======================================================================================
jeBoolean jeBSP_RenderAndVis(jeBSP *Tree, jeCamera *Camera, jeFrustum *Frustum)
{
	jeFrustum		ModelSpaceFrustum;
	jeXForm3d		ModelToCameraXForm, CameraToModelXForm;

	Tree->DebugInfo.NumVisibleAreas = 0;

	jeXForm3d_Multiply(jeCamera_XForm(Camera), &Tree->ModelToWorldXForm, &ModelToCameraXForm);
	jeXForm3d_GetTranspose(&ModelToCameraXForm, &CameraToModelXForm);

	// Transform Frustum from camera space to model space
	jeFrustum_Transform(Frustum, &CameraToModelXForm, &ModelSpaceFrustum);

	if (!jeBSP_UpdateAll(Tree))
		return JE_FALSE;

	if (!jeBSP_VisFrame(Tree, Camera, &ModelSpaceFrustum))
		return JE_FALSE;

	if (!UpdateDLights(Tree))
		return JE_FALSE;

	if (!UpdateObjects(Tree))
		return JE_FALSE;

	// Render the models BSP tree
	if (!jeBSP_RenderFrontToBack(Tree, Camera, Frustum, &ModelSpaceFrustum, &ModelToCameraXForm))
		return JE_FALSE;

#if 0
	if (!Tree->AreaChain) // @@
	{
		// Render the models BSP tree
		if (!jeBSP_RenderFrontToBack(Tree, Camera, &ModelSpaceFrustum))
			return JE_FALSE;

	}
	else
	{
		if (!jeBSP_RenderAreas(Tree, Camera, Frustum))
			return JE_FALSE;
	}
#endif

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_RenderAreas
//=======================================================================================
jeBoolean jeBSP_RenderAreas(jeBSP *Tree, jeCamera *Camera, jeFrustum *CameraSpaceFrustum, jeFrustum *ModelSpaceFrustum, jeXForm3d *ModelToCameraXForm)
{
#ifdef AREA_DRAWFACE_TEST
	DRV_Driver		*Driver;

	jeBSPNode_Area *Area;

	assert(Tree);
	assert(Tree->Engine);
	assert(Camera);
	assert(Frustum);

	if (!Tree->RootNode)
		return JE_TRUE;

	Driver = jeEngine_GetDriver(Tree->Engine);
	assert(Driver);
	Driver->SetupLightmap = jeBSP_SetupLightmap;
		
	Area = jeBSPNode_FindArea(Tree->RootNode,Tree, jeCamera_GetPov(Camera));
	if ( ! Area )
	{
		Area = jeBSPNode_FindClosestArea(Tree->RootNode,Tree, jeCamera_GetPov(Camera));
		if ( ! Area )
			return JE_FALSE;
	}

	if (! jeBSPNode_AreaRenderFlood_r(Area,Tree, Camera,Frustum, Tree->RenderRecursion ,NULL) )
		return JE_FALSE;
#endif

	return JE_TRUE;
}

#ifdef AREA_DRAWFACE_TEST
//=======================================================================================
//	jeBSP_MakeAreaDrawFaces
//=======================================================================================
jeBoolean jeBSP_MakeAreaDrawFaces(jeBSP *BSP)
{
	jeBSPNode_Area * pArea;
	LinkNode *CurNode=NULL;

	jeChain_Link	*Link;

	assert(BSP);

//	LN_Walk(pArea,&(BSP->AreaList))
	for (Link = jeChain_GetFirstLink(BSP->AreaChain); Link; Link = jeChain_LinkGetNext(Link))

	{
		pArea = (jeBSPNode_Area*)jeChain_LinkGetLinkData(Link);
		if ( ! jeBSPNode_AreaMakeDrawFaces(pArea) )
			return JE_FALSE;
	}

   return JE_TRUE;
}
#endif

//=======================================================================================
//	Update lights/faces/brushes
//=======================================================================================

//=====================================================================================
//	jeBSP_UpdateBrush
//	Update the geometry of a brush, autolights if told to do so
//=====================================================================================
jeBoolean jeBSP_UpdateBrush(jeBSP *BSPTree, jeBrush *Brush, jeBoolean AutoLight)
{
	jeBSP_TopBrush		*TopBrush, *NewList, *Next;
	jeBoolean			Found;
	uint32				Order;

	assert(BSPTree);
	assert(Brush);

	if (BSPTree->AreaChain)
	{
		if (!jeBSP_DestroyVisAreas(BSPTree))
			return JE_FALSE;
	}

	NewList = NULL;
	Found = JE_FALSE;

	if (BSPTree->RootNode)
	{
		// Find all the topbrushes that are a part of the editor brush, and remove them
		for (TopBrush = BSPTree->TopBrushes; TopBrush; TopBrush = Next)
		{
			Next = TopBrush->Next;

			if (TopBrush->Original == Brush)
			{
				if (!Found)
				{
					Order = TopBrush->Order;
					Found = JE_TRUE;
				}
				else
				{
					// All topbrushes resulting from the same jeBrush should have the same order number!!
					assert(Order == TopBrush->Order);
				}
		
				if (!jeBSPNode_RemoveTopBrush_r(BSPTree->RootNode, BSPTree, TopBrush))
					return JE_FALSE;
		
				jeBSP_TopBrushDestroy(&TopBrush, BSPTree);
				continue;
			}

			TopBrush->Next = NewList;
			NewList = TopBrush;
		}

		BSPTree->TopBrushes = NewList;
	}

	if (!Found)
		Order = BSPTree->NumBrushes++;
   	
	// Add the brush back into the tree with the same order number it had before
	if (!jeBSP_AddBrushInternally(BSPTree, Brush, Order, AutoLight))
		return JE_FALSE;

	BSPTree->UpdateFlags |= BSP_UPDATE_FACES;
	BSPTree->UpdateFlags |= BSP_UPDATE_LIGHTS;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_UpdateBrushFace
//	Update the properties of a face, autolights if told to do so
//=======================================================================================
jeBoolean jeBSP_UpdateBrushFace(jeBSP *BSP, const jeBrush_Face *Face, jeBoolean AutoLight)
{
	jeBSP_TopBrush	*TopBrush;
	jeBrush			*Original;

	assert(BSP);

	if (BSP->AreaChain)
	{
		if (!jeBSP_DestroyVisAreas(BSP))
			return JE_FALSE;
	}

	if (!BSP->RootNode)
		return JE_TRUE;

	Original = jeBrush_FaceGetBrush(Face);

	// Find the TopBrush that contains the pointer to the jeBrushFace
	//	This is kind of slow, but we could use hash tables if it got out of hand...
	for (TopBrush = BSP->TopBrushes; TopBrush; TopBrush = TopBrush->Next)
	{
		int32			i;
		jeBSP_TopSide	*Side;

		if (TopBrush->Original != Original)
			continue;

		for (Side = TopBrush->TopSides, i=0; i< TopBrush->NumSides; i++, Side++)
		{
			if (Side->jeBrushFace != Face)
				continue;

			if (AutoLight)
				Side->TopSideFlags |= TOPSIDE_UPDATE_LIGHT;
			else
				Side->TopSideFlags &= ~TOPSIDE_UPDATE_LIGHT;

			if (!jeBSP_TopBrushSideCalcFaceInfo(TopBrush, BSP, Side, Face))
				return JE_FALSE;

			// Mark all nodes that touch this side to update their faces
			if (!jeBSPNode_DirtyNodes_r(BSP->RootNode, BSP, Side))
				return JE_FALSE;
					
			BSP->UpdateFlags |= BSP_UPDATE_FACES;
			BSP->UpdateFlags |= BSP_UPDATE_LIGHTS;
		}
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_PatchLights
//=======================================================================================
jeBoolean jeBSP_PatchLighting(jeBSP *Tree)
{
	if (!Tree->RootNode)
		return JE_TRUE;

	if (!jeBSPNode_LightPatch_r(Tree->RootNode, Tree, Tree->RootNode))
		return JE_FALSE;

	{
		jeBSP_TopBrush		*TopBrush;

		for (TopBrush = Tree->TopBrushes; TopBrush; TopBrush = TopBrush->Next)
		{
			int32		i;

			for (i=0; i< TopBrush->NumSides; i++)
			{
				TopBrush->TopSides[i].TopSideFlags |= TOPSIDE_UPDATE_LIGHT;
			}
		}
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_RebuildLights
//=======================================================================================
jeBoolean jeBSP_RebuildLights(jeBSP *Tree)
{
	if (!Tree->RootNode)
		return JE_TRUE;

	if (!jeBSPNode_LightUpdate_r(Tree->RootNode, Tree, Tree->RootNode, JE_TRUE))
		return JE_FALSE;

	{
		jeBSP_TopBrush		*TopBrush;

		for (TopBrush = Tree->TopBrushes; TopBrush; TopBrush = TopBrush->Next)
		{
			int32		i;

			for (i=0; i< TopBrush->NumSides; i++)
			{
				TopBrush->TopSides[i].TopSideFlags |= TOPSIDE_UPDATE_LIGHT;
			}
		}
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_RebuildLightsFromPoint
//=======================================================================================
jeBoolean jeBSP_RebuildLightsFromPoint(jeBSP *Tree, const jeVec3d *Pos, jeFloat Radius)
{
	jeVec3d		NewPos;

	if (!Tree->RootNode)
		return JE_TRUE;

	jeXForm3d_Transform(&Tree->WorldToModelXForm, Pos, &NewPos);
	
	if (!jeBSPNode_LightUpdateFromPoint_r(Tree->RootNode, Tree, Tree->RootNode, &NewPos, Radius))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_UpdateAll
//=======================================================================================
jeBoolean jeBSP_UpdateAll(jeBSP *BSP)
{
	assert(BSP);

#if (JE_BSP_DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("BEGIN jeBSP_UpdateAll\n");
#endif

	if (!BSP->RootNode)
		return JE_TRUE;

	if (BSP->UpdateFlags & BSP_UPDATE_FACES)
	{
		if (!jeBSP_MarkVisibleTopSides(BSP))
			return JE_FALSE;

		NumMakeFaces = 0;
		NumMergedFaces = 0;
		NumSubdividedFaces = 0;

		if (!jeBSPNode_MakeFaces_r(BSP->RootNode, BSP, JE_TRUE))
			return JE_FALSE;

		BSP->UpdateFlags &= ~BSP_UPDATE_FACES;

		if (BSP->Driver) 
		{
			jeDeviceCaps devcaps;
			BSP->Driver->GetDeviceCaps(&devcaps);
			if ((devcaps.SuggestedDefaultRenderFlags & JE_RENDER_FLAG_HWTRANSFORM) == JE_RENDER_FLAG_HWTRANSFORM) {
				jeBSP_CreateVertexBuffer(BSP);
			}
		}
	}

	if (BSP->UpdateFlags & BSP_UPDATE_LIGHTS)
	{
		if (!jeBSPNode_LightUpdate_r(BSP->RootNode, BSP, BSP->RootNode, JE_FALSE))
			return JE_FALSE;

		BSP->UpdateFlags &= ~BSP_UPDATE_LIGHTS;
	}

	{
		jeDeviceCaps devcaps;
		BSP->Driver->GetDeviceCaps(&devcaps);
		if ((devcaps.SuggestedDefaultRenderFlags & JE_RENDER_FLAG_HWTRANSFORM) == JE_RENDER_FLAG_HWTRANSFORM && BSP->pVertexBuffer == NULL) {
			jeBSP_CreateVertexBuffer(BSP);
		}
	}

#if (JE_BSP_DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("END jeBSP_UpdateAll\n");
#endif

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_RebuildFaces
//=======================================================================================
jeBoolean jeBSP_RebuildFaces(jeBSP *BSPTree)
{
	if (!BSPTree->RootNode)
		return JE_TRUE;

	jeBSPNode_RebuildFaces_r(BSPTree->RootNode, BSPTree);

	// Use the tree to mark the visible sides
	if (!jeBSP_MarkVisibleTopSides(BSPTree))
		return JE_FALSE;

	if (!jeBSPNode_MakeFaces_Callr(BSPTree->RootNode, BSPTree, JE_TRUE))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_SetBrushFaceCB
//=======================================================================================
jeBoolean jeBSP_SetBrushFaceCB(jeBSP *BSP, jeBSPNode_DrawFaceCB *CB, void *Context)
{
	BSP->DrawFaceCB = CB;
	BSP->DrawFaceCBContext = Context;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_SetBrushFaceCBOnOff
//=======================================================================================
jeBoolean jeBSP_SetBrushFaceCBOnOff(jeBSP *BSP, const jeBrush_Face *Face, jeBoolean OnOff)
{
	jeBSP_TopBrush	*TopBrush;
	jeBrush			*Original;

	assert(BSP);

	if (!BSP->RootNode)
		return JE_TRUE;

	Original = jeBrush_FaceGetBrush(Face);

	// Find the TopBrush that contains the pointer to the jeBrushFace
	//	This is kind of slow, but we could use hash tables if it got out of hand...
	for (TopBrush = BSP->TopBrushes; TopBrush; TopBrush = TopBrush->Next)
	{
		int32			i;
		jeBSP_TopSide	*Side;

		if (TopBrush->Original != Original)
			continue;

		for (Side = TopBrush->TopSides, i=0; i< TopBrush->NumSides; i++, Side++)
		{
			int32		Num;

			if (Side->jeBrushFace != Face)
				continue;

			if (OnOff)
				Side->TopSideFlags |= TOPSIDE_CALL_CB;
			else
				Side->TopSideFlags &= ~TOPSIDE_CALL_CB;

			Num = 0;
			if (!jeBSPNode_PropogateTopSideFlags_r(BSP->RootNode, BSP, Side, &Num))
				return JE_FALSE;

			if (!Num)
			{
				if (!jeBSPNode_DirtyNodes_r(BSP->RootNode, BSP, Side))
					return JE_FALSE;
			}

			// Update faces and lights just in case anything got destroyed
			BSP->UpdateFlags |= BSP_UPDATE_FACES;
			BSP->UpdateFlags |= BSP_UPDATE_LIGHTS;
		}
	}

	return JE_TRUE;
}

//================================================================================================
//	Misc
//================================================================================================

//=======================================================================================
//	jeBSP_SetXForm
//=======================================================================================
jeBoolean jeBSP_SetXForm(jeBSP *BSP, const jeXForm3d *XForm)
{
	// Save off ModelToWorld XForm
	BSP->ModelToWorldXForm = *XForm;

	// Get the Transpose of that to get WorldToModelXForm
	jeXForm3d_GetTranspose(&BSP->ModelToWorldXForm, &BSP->WorldToModelXForm);

	// Maintain the bounding WorldSpace ExtBox of the bsp
	if (!jeBSP_UpdateWorldSpaceBox(BSP))
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_SetEngine
//=======================================================================================
jeBoolean jeBSP_SetEngine(jeBSP *Tree, jeEngine *Engine)
{
	assert(Tree);
	if (Engine == Tree->Engine) {
		return JE_TRUE;
	}
	
	if (Tree->Engine)
	{
		assert(Tree->ChangeDriverCB);
		jeEngine_DestroyChangeDriverCB(Tree->Engine, &Tree->ChangeDriverCB);
		jeEngine_Free(Tree->Engine);
		Tree->Engine = NULL;
	}
	else
	{
		assert(!Tree->ChangeDriverCB);
		assert(!Tree->Driver);
	}

	Tree->Engine = Engine;

	if (Engine)
	{
		Tree->ChangeDriverCB = jeEngine_CreateChangeDriverCB(Engine, ShutdownDriverCB, StartupDriverCB, Tree);

		if (!Tree->ChangeDriverCB)
			return JE_FALSE;

		jeEngine_CreateRef(Engine);
		jeEngine_SetRenderMode(Engine, Tree->RenderMode);
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_SetWorld
//=======================================================================================
jeBoolean jeBSP_SetWorld(jeBSP *Tree, jeWorld *World)
{
	assert(Tree);
	
	if (Tree->World)
	{
		jeWorld_Destroy(&Tree->World);
		Tree->World = NULL;
	}

	Tree->World = World;

	if (World)
	{
		jeWorld_CreateRef(World);
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_SetRenderMode
//=======================================================================================
jeBoolean jeBSP_SetRenderMode(jeBSP *BSP, jeBSP_RenderMode RenderMode)
{
	BSP->RenderMode = RenderMode;
   jeEngine_SetRenderMode(BSP->Engine, RenderMode);

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_SetDefaultContents
//=======================================================================================
jeBoolean jeBSP_SetDefaultContents(jeBSP *BSP, jeBrush_Contents DefaultContents)
{
	BSP->DefaultContents = DefaultContents;

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_FindArea
//=======================================================================================
jeBSPNode_Area *jeBSP_FindArea(jeBSP *BSP, const jeVec3d *Pos)
{
	if (!BSP->RootNode)
		return NULL;

	return jeBSPNode_FindArea(BSP->RootNode,BSP, Pos);
}

//=======================================================================================
//	jeBSP_DoAllAreasInBox
//=======================================================================================
jeBoolean jeBSP_DoAllAreasInBox(jeBSP *BSP,jeExtBox *BBox,jeBSP_DoAreaFunc CB,void * Context)
{
	return jeBSPNode_DoAllAreasInBox(BSP->RootNode,BSP, BBox,CB,Context);
}

//=====================================================================================
//	jeBSP_GetModelSpaceBox
//=====================================================================================
jeBoolean jeBSP_GetModelSpaceBox(const jeBSP *BSP, jeExtBox *Box)
{
	*Box = BSP->Box;

	return JE_TRUE;
}

//=====================================================================================
//	jeBSP_GetWorldSpaceBox
//=====================================================================================
jeBoolean jeBSP_GetWorldSpaceBox(const jeBSP *BSP, jeExtBox *Box)
{
	*Box = BSP->WorldSpaceBox;

	return JE_TRUE;
}

//=====================================================================================
//	GetSegmentBox
//	Creates a box around the entire segment
//=====================================================================================
static void GetSegmentBox(const jeVec3d *Mins, const jeVec3d *Maxs, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *SMins, jeVec3d *SMaxs)
{
	int32		i;

	assert(Mins);
	assert(Maxs);
	assert(Front);
	assert(Back);
	
	for (i=0 ; i<3 ; i++)
	{
		if (jeVec3d_GetElement(Back, i) > jeVec3d_GetElement(Front, i))
		{
			jeVec3d_SetElement(SMins, i, jeVec3d_GetElement(Front, i) + jeVec3d_GetElement(Mins, i) - 1.0f);
			jeVec3d_SetElement(SMaxs, i, jeVec3d_GetElement(Back, i) + jeVec3d_GetElement(Maxs, i) + 1.0f);
		}
		else
		{
			jeVec3d_SetElement(SMins, i, jeVec3d_GetElement(Back, i) + jeVec3d_GetElement(Mins, i) - 1.0f);
			jeVec3d_SetElement(SMaxs, i, jeVec3d_GetElement(Front, i) + jeVec3d_GetElement(Maxs, i) + 1.0f);
		}
	}
}

//=======================================================================================
//	jeBSP_Collision
//	Returns JE_TRUE if there was a collision, JE_FALSE otherwise
//=======================================================================================
jeBoolean jeBSP_Collision(	const jeBSP		*BSP, 
							const jeExtBox	*Box, 
							const jeVec3d	*Front, 
							const jeVec3d	*Back, 
							jeVec3d			*Impact, 
							jePlane			*Plane)
{
	int32			i;
	jeVec3d			SegmentMins, SegmentMaxs, Front2, Back2, vPath;
	jeExtBox		FakeBox;
	const jeExtBox	*RejectBox;

	// The code below will act as if there was a box, so if box is NULL, pretend like there is a box
	if (Box)
	{
		RejectBox = Box;
	}
	else
	{
		jeVec3d_Set(&FakeBox.Min, -1.0f, -1.0f, -1.0f);
		jeVec3d_Set(&FakeBox.Max,  1.0f,  1.0f,  1.0f);
		RejectBox = &FakeBox;
	}

	// Get a box that enscribes the entire segment
	GetSegmentBox(&RejectBox->Min, &RejectBox->Max, Front, Back, &SegmentMins, &SegmentMaxs);

	// Do some trivial rejection stuff here
	for (i=0; i<3; i++)
	{
		if (jeVec3d_GetElement(&SegmentMaxs, i) < jeVec3d_GetElement(&BSP->WorldSpaceBox.Min, i))
			break;
		if (jeVec3d_GetElement(&SegmentMins, i) > jeVec3d_GetElement(&BSP->WorldSpaceBox.Max, i))
			break;
	}

	if (i != 3)
		return JE_FALSE;		// Segment does not collide with mnode's dynamic box (box is already transformed into world space)


	// Transform the ray into the bsptree
	jeXForm3d_Transform(&BSP->WorldToModelXForm, Front, &Front2);
	jeXForm3d_Transform(&BSP->WorldToModelXForm, Back , &Back2);

	if (!Box)
	{
		// Icestorm: Check only on collision, no further details
		if (!Plane || !Impact)
			return (jeBSPNode_CollisionExact_r(BSP->RootNode, (jeBSP*)BSP, &Front2, &Back2, NULL));
		else
		{
			jeBSPNode_CollisionInfo			Info;

			Info.HitSet = JE_FALSE;
			Info.Impact = Impact;
			Info.Plane = Plane;

			if (jeBSPNode_CollisionExact_r(BSP->RootNode, (jeBSP*)BSP, &Front2, &Back2, &Info))
			{
	            jeXForm3d_Transform(&BSP->ModelToWorldXForm, Info.Impact, Info.Impact);
				jePlane_Transform(Plane, &BSP->ModelToWorldXForm, Plane);	// Transform Plane into world space
				return JE_TRUE;
			}
		}
	}
	else
	{
		jeExtBox					SegmentBox;

		GetSegmentBox(&Box->Min, &Box->Max, &Front2, &Back2, &SegmentBox.Min, &SegmentBox.Max);

		if (!Plane || !Impact)
			return jeBSPNode_CollisionBBox_r(BSP->RootNode, (jeBSP*)BSP, &SegmentBox, Box, &Front2, &Back2, NULL);
		else
		{
			jeBSPNode_CollisionInfo2	Info;
			Info.HitSet = JE_FALSE;
			Info.Front = &Front2;
			Info.Plane = Plane;
			Info.Impact = Impact;
			Info.BestDist = 9999999.0f;

			jeBSPNode_CollisionBBox_r(BSP->RootNode, (jeBSP*)BSP, &SegmentBox, Box, &Front2, &Back2, &Info);

			if (Info.HitSet)
			{
				jePlane_Transform(Plane, &BSP->ModelToWorldXForm, Plane);	// Transform Plane into world space
				jeXForm3d_Transform(&BSP->ModelToWorldXForm, Info.Impact, Info.Impact);

				jeVec3d_Subtract(Front, Back, &vPath); 
				jeVec3d_Normalize(&vPath); 
				jeVec3d_AddScaled(Info.Impact, &vPath, 1.5f, Info.Impact); 
				Plane->Dist = jeVec3d_DotProduct(&vPath, Info.Impact); 
				Plane->Type = Type_Any; 

				return JE_TRUE;
			}
		}
	}

	return JE_FALSE;			
}

// Added by Icestorm
//=======================================================================================
//	jeBSP_ChangeBoxCollision
//	Returns JE_TRUE if there was a collision, while box changes, JE_FALSE otherwise
//=======================================================================================
jeBoolean jeBSP_ChangeBoxCollision(	const jeBSP		*BSP, 
									const jeVec3d	*Pos,
									const jeExtBox	*FrontBox, 
									const jeExtBox	*BackBox, 
									jeExtBox		*ImpactBox, 
									jePlane			*Plane)
{
	int32			i;
	jeVec3d			SegmentMins, SegmentMaxs, Pos2;
	jeExtBox		RejectBox;

	assert(jeExtBox_IsValid(FrontBox)&&jeExtBox_IsValid(BackBox));
	
	// Get a box that enscribes the entire segment
	jeExtBox_Union(FrontBox, BackBox, &RejectBox);
	// extend to a safe distance
	RejectBox.Min.X-=1.0f;RejectBox.Min.Y-=1.0f;RejectBox.Min.Z-=1.0f;
	RejectBox.Max.X+=1.0f;RejectBox.Max.Y+=1.0f;RejectBox.Max.Z+=1.0f;
	jeExtBox_SetTranslation(&RejectBox,Pos);
	SegmentMins=RejectBox.Min;SegmentMaxs=RejectBox.Max;

	// Do some trivial rejection stuff here
	for (i=0; i<3; i++)
	{
		if (jeVec3d_GetElement(&SegmentMaxs, i) < jeVec3d_GetElement(&BSP->WorldSpaceBox.Min, i))
			break;
		if (jeVec3d_GetElement(&SegmentMins, i) > jeVec3d_GetElement(&BSP->WorldSpaceBox.Max, i))
			break;
	}

	if (i != 3)
		return JE_FALSE;		// Segment does not collide with node's dynamic box (box is already transformed into world space)


	// Transform the ray into the bsptree
	jeXForm3d_Transform(&BSP->WorldToModelXForm, Pos, &Pos2);

	{
		jeExtBox_SetTranslation(&RejectBox,&Pos2);
		
		if (!Plane || !ImpactBox)
			return jeBSPNode_ChangeBoxCollisionBBox_r(BSP->RootNode, (jeBSP*)BSP, &RejectBox, &Pos2,
					FrontBox, BackBox, NULL);
		else
		{
			jeBSPNode_CollisionInfo3	Info;

			Info.HitSet = JE_FALSE;
			Info.FrontBox = FrontBox;
			Info.Plane = Plane;
			Info.ImpactBox = ImpactBox;
			Info.BestDist = 9999999.0f;

			jeBSPNode_ChangeBoxCollisionBBox_r(BSP->RootNode, (jeBSP*)BSP, &RejectBox, &Pos2, FrontBox, BackBox, &Info);

			if (Info.HitSet)
			{
				jePlane_Transform(Plane, &BSP->ModelToWorldXForm, Plane);	// Transform Plane into world space
				Plane->Dist += 0.1f;
				return JE_TRUE;
			}
		}
	}

	return JE_FALSE;			
}

//========================================================================================
//	jeBSP_RayIntersectsBrushes
//========================================================================================
jeBoolean jeBSP_RayIntersectsBrushes(const jeBSP *BSP, const jeVec3d *Front, const jeVec3d *Back, jeBrushRayInfo *Info)
{
	jeVec3d		Front2, Back2;

	assert(BSP);

	if (!BSP->RootNode)
		return JE_FALSE;

	// Transform the ray into the bsptree
	jeXForm3d_Transform(&BSP->WorldToModelXForm, Front, &Front2);
	jeXForm3d_Transform(&BSP->WorldToModelXForm, Back , &Back2);

	if (jeBSPNode_RayIntersectsBrushes(((jeBSP*)BSP)->RootNode, (jeBSP*)BSP, &Front2, &Back2, Info))
	{
		jePlane_Transform(&Info->Plane, &BSP->ModelToWorldXForm, &Info->Plane);	// Transform Plane into world space
		return JE_TRUE;
	}

	return JE_FALSE;
}

//=======================================================================================
//	jeBSP_GetDebugInfo
//=======================================================================================
const jeBSP_DebugInfo *jeBSP_GetDebugInfo(const jeBSP *BSPTree)
{
	assert(BSPTree);

	return &BSPTree->DebugInfo;
}

//=======================================================================================
//	 Local static functions
//=======================================================================================

//=====================================================================================
//	jeBSP_AddBSPBrush_r
//	Takes the brush, and distributes it to the leafs, where it then takes the leafs
//	the brush lands in, and partitions up the resulting brushes to make more leafs...
//=====================================================================================
static jeBoolean jeBSP_AddBSPBrush_r(jeBSP *BSP, jeBSPNode *Node, jeBSP_Brush **Brush)
{
	jeBSP_Brush		*Front, *Back, *Brush2;
	int32			Splits,  EpsilonBrush;
	int HintSplit;

	assert(Node);

	Brush2 = *Brush;

	if (!Brush2)				// Brush was clipped away...
		return JE_TRUE;

	if (Node->Flags & NODE_LEAF)
	{
		// At leaf...
		// Convert this leaf into a node, and take the brush fragment that landed in this partition, and 
		// partition it into new leafs starting from the new node (the converted leaf)
		jeBSPNode_Portal	*p;
		int32				Side;
		jeBSP_Brush			*BSPBrush;

		assert(Node->Leaf);
		assert(Node->Portals);

		jeBSPNode_LeafDestroyDrawFaceList(Node->Leaf, BSP);

	#if 1
		for (BSPBrush = Node->Leaf->Brushes; BSPBrush; BSPBrush=BSPBrush->Next)
		{
			// Since this brush is "inside" of this leaf, we can force the brushes of this leaf
			// down both sides (unless the leaf brush has a plane that shares a plane in this brush, 
			// then we send it down the correct side)
			BSPBrush->Flags = BSPBRUSH_FORCEBOTH;
			//jeBSP_BrushCreatePolys(BSPBrush);
		}
	#endif

		Brush2->Next = Node->Leaf->Brushes;	// Add brush to front of leafs list of brushes
		Node->Leaf->Brushes = NULL;			// So jeBSPNode_LeafDestroy won't destroy them
		Node->Flags &= ~NODE_LEAF;			// This node is no longer a leaf

		jeBSPNode_LeafDestroy(&Node->Leaf, BSP);

		BSP->DebugInfo.NumLeafs--;			// Take a leaf away from the num total leafs in tree

		// Go through all the portals that look into this leaf, and remove their polys
		// from the nodes they were created on
		for (p = Node->Portals; p; p = p->Next[Side])
		{
			Side = (p->Nodes[1] == Node);

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
				// Force an update on nodes that share this portal
				p->OnNode->Flags |= NODE_REBUILD_FACES;
			}

			jeBSPNode_PortalResetTopSide(p);
		}

		// Start from this list, and start building the tree from here to form the new leafs
		if (!jeBSP_BuildBSPTree_r(BSP, Node, Brush))
		{
			jeErrorLog_AddString(-1, "jeBSP_AddBSPBrush_r:  jeBSPNode_PartitionPortals_r failed.", NULL);
			return JE_FALSE;
		}

		// Take the portals that were on this old leaf, and distribute them to the new leafs...
		if (!jeBSPNode_PartitionPortals_r(Node, BSP, JE_TRUE))
		{
			jeErrorLog_AddString(-1, "jeBSP_AddBSPBrush_r:  jeBSPNode_PartitionPortals_r failed.", NULL);
			return JE_FALSE;
		}

		// Update the leaf sides
		if (!jeBSPNode_UpdateLeafSides_r(Node, BSP))
		{
			jeErrorLog_AddString(-1, "jeBSP_AddBSPBrush_r:  jeBSPNode_UpdateLeafSides_r failed.", NULL);
			return JE_FALSE;
		}

		return JE_TRUE;
	}

	assert(!Node->Leaf);

	// Set the brush side, jeBSP_BrushSplitListByNode uses it as a first test
	Brush2->Side = jeBSP_TestBrushToPlaneIndex(BSP, Brush2, Node->PlaneIndex, &Splits, &HintSplit, &EpsilonBrush);

	if (!jeBSPNode_SplitBrushList(Node, BSP, Brush2, &Front, &Back))
		return JE_FALSE;

	jeBSP_BrushDestroy(Brush);		// Don't need this sucka anymore...

	if (!jeBSP_AddBSPBrush_r(BSP, Node->Children[NODE_FRONT], &Front))
		return JE_FALSE;
	if (!jeBSP_AddBSPBrush_r(BSP, Node->Children[NODE_BACK], &Back))
		return JE_FALSE;

	return JE_TRUE;
}

//=====================================================================================
//	jeBSP_AddBrushInternally
//=====================================================================================
static jeBoolean jeBSP_AddBrushInternally(jeBSP *BSPTree, jeBrush *Brush, uint32 Order, jeBoolean AutoLight)
{
	jeBSP_TopBrush	*TopBrush = NULL;
	jeBSP_Brush		*BSPBrush = NULL;
	jeBoolean		Ret;

	assert(BSPTree);
	assert(Brush);
	assert(!jeBSP_HasBrush(BSPTree, Brush));

	if (BSPTree->AreaChain)
	{
		if (!jeBSP_DestroyVisAreas(BSPTree))
			return JE_FALSE;
	}

	// Create a Top level Brush from the editor brush
	TopBrush = jeBSP_TopBrushCreateFromBrush(Brush, BSPTree, Order);

	if (!TopBrush)
		goto ExitWithError;

	if (AutoLight)
	{
		jeBSP_TopSide		*TopSide;
		int32				i;

		for (TopSide = TopBrush->TopSides, i = 0; i< TopBrush->NumSides; i++, TopSide++)
			TopSide->TopSideFlags |= TOPSIDE_UPDATE_LIGHT;
	}

	// Add the top brush to the list of top brushes in the tree
	TopBrush->Next = BSPTree->TopBrushes;
	BSPTree->TopBrushes = TopBrush;

	// Create a bsp brush
	BSPBrush = jeBSP_BrushCreateFromTopBrush(TopBrush, BSPTree);

	if (!BSPBrush)
		goto ExitWithError;

	Ret = JE_TRUE;

	// Start at the top of the tree, and filter the BSPBrush down...
	if (!BSPTree->RootNode)
	{
		if (!jeBSP_BuildBSPTree(BSPTree, &BSPBrush, Logic_Lazy, 0))
			goto ExitWithError;
			
		if (!jeBSP_CreatePortals(BSPTree, JE_TRUE))
			goto ExitWithError;
	}
	else
	{
		// Update the bsptrees bbox
		jeExtBox_Union(&BSPTree->Box, &BSPBrush->Box, &BSPTree->Box);

		if (!jeBSP_AddBSPBrush_r(BSPTree, BSPTree->RootNode, &BSPBrush))		// Add brush to existing tree
		{
			goto ExitWithError;
		}
	}

	BSPTree->UpdateFlags |= BSP_UPDATE_FACES;
	BSPTree->UpdateFlags |= BSP_UPDATE_LIGHTS;

	if (!jeBSP_UpdateWorldSpaceBox(BSPTree))
		return JE_FALSE;

	BSPTree->DebugInfo.NumBrushes++;

	return JE_TRUE;

	// Error
	ExitWithError:
	{
		if (TopBrush)
			BSPTree->TopBrushes = jeBSP_TopBrushCullList(BSPTree->TopBrushes, BSPTree, TopBrush);

		if (BSPBrush)
			jeBSP_BrushDestroy(&BSPBrush);

		return JE_FALSE;
	}
}

// FIXME: Remove!!!!
extern int32	NumAirLeafs;
extern int32	NumSolidLeafs;

//=======================================================================================
//	jeBSP_BuildBSPTree
//=======================================================================================
static jeBoolean jeBSP_BuildBSPTree(jeBSP *BSPTree, 
									jeBSP_Brush **BrushList, 
									jeBSP_Logic Logic, 
									jeBSP_LogicBalance LogicBalance)
{
	jeBSP_Brush	*b;
	int32		NumNonVisFaces;
	int32		i;
	jeFloat		Volume;
	jeBoolean	Set;

	assert(BSPTree->RootNode == NULL);

	Log_Printf("--- jeBSP_BuildBSPTree ---\n");

	// Set static globals
	g_Logic = Logic;
	g_LogicBalance = LogicBalance;

	NumNonVisFaces = 0;

	Set = JE_FALSE;

	for (b=*BrushList ; b ; b=b->Next)
	{
		BSPTree->DebugInfo.NumBrushes++;

		Volume = jeBSP_BrushVolume(b, BSPTree);

		if (Volume < JE_BSP_TINY_VOLUME)
			Log_Printf("**WARNING** jeBSP_BuildBSPTree: Brush with NULL volume\n");
		
		for (i=0 ; i<b->NumSides ; i++)
		{
			if (!b->Sides[i].Poly)
				continue;

			BSPTree->DebugInfo.NumTotalBrushFaces++;

			if (b->Sides[i].Flags & SIDE_NODE)
				continue;

			if (b->Sides[i].Flags & SIDE_VISIBLE)
				BSPTree->DebugInfo.NumVisibleBrushFaces++;
		}

		if (!Set)
		{
			BSPTree->Box = b->Box;
			Set = JE_TRUE;
		}
		else
		{
			jeExtBox_Union(&BSPTree->Box, &b->Box, &BSPTree->Box);
		}
	}
	
	Log_Printf("Total Brushes          : %5i\n", BSPTree->DebugInfo.NumBrushes);
	Log_Printf("Visible Faces          : %5i\n", BSPTree->DebugInfo.NumVisibleBrushFaces);
	Log_Printf("Total Faces            : %5i\n", BSPTree->DebugInfo.NumTotalBrushFaces);

	NumNonVisNodes = 0;
	
	// Create the very first node
	BSPTree->RootNode = jeBSPNode_Create(BSPTree);

	BSPTree->RootNode->Box = BSPTree->Box;

	if (Logic >= Logic_Smart)
	{
		BSPTree->RootNode->Volume = jeBSP_BrushCreateFromBox(BSPTree, &BSPTree->Box);

		if (jeBSP_BrushVolume(BSPTree->RootNode->Volume, BSPTree) < JE_BSP_TINY_VOLUME)
			Log_Printf("**WARNING** jeBSP_BuildBSPTree: BAD Tree Volume.\n");
	}
	else
		BSPTree->RootNode->Volume = NULL;

	if (!jeBSP_BuildBSPTree_r(BSPTree, BSPTree->RootNode, BrushList))
		return JE_FALSE;

	Log_Printf("Total Nodes            : %5i\n", BSPTree->DebugInfo.NumNodes);
	Log_Printf("Nodes Removed          : %5i\n", NumNonVisNodes);
	Log_Printf("Total Leafs            : %5i\n", BSPTree->DebugInfo.NumLeafs);

	//Log_Printf("Num Air Leafs          : %5i\n", NumAirLeafs);
	//Log_Printf("Num Solid Leafs        : %5i\n", NumSolidLeafs);

#ifdef AREA_DRAWFACE_TEST
	LN_InitList(&(BSPTree->LeafList));

	if (!jeBSPNode_MakeLeafList_r(BSPTree->RootNode,&(BSPTree->LeafList)))
		return JE_FALSE;

	Log_Printf("Leaf List Len = %d\n",LN_ListLen(&(BSPTree->LeafList)));
	assert( ((BSPTree->DebugInfo.NumNodes+1)/2) == LN_ListLen(&(BSPTree->LeafList)));
#endif

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_BuildBSPTree_r
//=======================================================================================
static jeBoolean jeBSP_BuildBSPTree_r(jeBSP *BSP, jeBSPNode *Node, jeBSP_Brush **Brushes)
{
	jeBSPNode	*NewNode;
	jeBSP_Side	*BestSide;
	int32		i;
	jeBSP_Brush	*Children[2];

	// find the best plane to use as a splitter
	BestSide = jeBSP_GetSplitter(BSP, *Brushes, Node);
	
	if (!BestSide)
	{
		BSP->DebugInfo.NumLeafs++;

		if (!jeBSPNode_InitializeLeaf(Node, BSP, *Brushes))
			return JE_FALSE;

		if (Node->Volume)
		{
			assert(g_Logic >= Logic_Smart);
			jeBSP_BrushDestroy(&Node->Volume);
		}

		return JE_TRUE;
	}

	BSP->DebugInfo.NumNodes++;

	// This is a splitplane node
	Node->Side = BestSide;
	// Nodes are ALWAYS positive facing
	Node->PlaneIndex = jePlaneArray_IndexGetPositive(BestSide->PlaneIndex);

	// Ref the plane
	if (!jePlaneArray_RefPlaneByIndex(BSP->PlaneArray, Node->PlaneIndex))
		return JE_FALSE;

	// Split the brushes on this node
	if (!jeBSPNode_SplitBrushList(Node, BSP, *Brushes, &Children[0], &Children[1]))
   {
		return JE_FALSE;
   }

	// Don't need this list anymore
	jeBSP_BrushDestroyList(Brushes);
	
   // Set pointer to zero
   Node->Children[0] = NULL;
   Node->Children[1] = NULL;
	// Allocate children before recursing
	for (i=0 ; i<2 ; i++)
	{
		NewNode = jeBSPNode_Create(BSP);

		if (!NewNode)
			goto ExitError;

		NewNode->Parent = Node;				// And a child is born...
		Node->Children[i] = NewNode;
	}
	
	if (Node->Volume)
	{
		assert(g_Logic >= Logic_Smart);

		// Distribute this nodes volume to its children
		jeBSP_BrushSplit(Node->Volume, BSP, Node->PlaneIndex, SIDE_NODE, &Node->Children[0]->Volume, &Node->Children[1]->Volume);

		if (!Node->Children[0]->Volume || !Node->Children[1]->Volume)
			Log_Printf("*WARNING* jeBSP_BuildBSPTree_r:  Volume was not split on both sides...\n");
	
		jeBSP_BrushDestroy(&Node->Volume);
	}

	// Recursively process children
	for (i=0 ; i<2 ; i++)
	{
      if (!jeBSP_BuildBSPTree_r(BSP, Node->Children[i], &Children[i]))
      {
			goto ExitError;
      }
	}

	return JE_TRUE;

ExitError:
	for (i=0 ; i<2 ; i++)
	{
      if (Node->Children[i]) {
         jeBSPNode_Destroy_r(&Node->Children[i], BSP);
      }
   }
   return JE_FALSE;
}

//=======================================================================================
//	jeBSP_TestBrushToPlaneIndex
//=======================================================================================
static jePlane_Side jeBSP_TestBrushToPlaneIndex(jeBSP *BSP, jeBSP_Brush *Brush, jePlaneArray_Index Index, int32 *NumSplits, 
												jeBoolean *HintSplit, int32 *EpsilonBrush)
{
	int32			i, j;
	const jePlane	*pPlane;
	jePlane_Side	s;
	jePoly			*p;
	jeFloat			d, d_front, d_back;
	int32			Front, Back;
	jeBSP_Side		*pSide;

	*NumSplits = 0;
	*HintSplit = JE_FALSE;

#if 0		// Experimental
	// Once a brushes sides has all been picked as nodes, we can assume that it totally
	//	encapsulates all brushes under it in the tree.  In this case, we can start forcing
	//	down both sides of it's children
	if (!(Brush->Flags & BSPBRUSH_FORCEBOTH))
	{
		for (pSide = Brush->Sides, i=0 ; i<Brush->NumSides ; i++, pSide++)
		{
			if (!(pSide->Flags & SIDE_NODE))
				break;
		}

		if (i == Brush->NumSides)
		{
			Brush->Flags |= BSPBRUSH_FORCEBOTH;

			for (pSide = Brush->Sides, i=0 ; i<Brush->NumSides ; i++, pSide++)
				jePoly_Destroy(&pSide->Poly);

			Brush->NumSides = 0;
		}
	}
#endif

	// If the brush uses this planenum, then send it down the side it's oppositely facing
	for (pSide = Brush->Sides, i=0 ; i<Brush->NumSides ; i++, pSide++)
	{
		if (jePlaneArray_IndexIsCoplanarAndFacing(Index, pSide->PlaneIndex))
			return PSIDE_BACK|PSIDE_FACING;
		if (jePlaneArray_IndexIsCoplanarAndNotFacing(Index, pSide->PlaneIndex))
			return PSIDE_FRONT|PSIDE_FACING;
	}
		
	if (Brush->Flags & BSPBRUSH_FORCEBOTH)
		return PSIDE_BOTH;			// If not facing, check to see if forceboth is set...

	// Box on plane side
	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Index);

	assert(pPlane);

	s = jePlane_BoxSide(pPlane, &Brush->Box, JE_BSP_PLANESIDE_EPSILON);

	if (s != PSIDE_BOTH)
		return s;
					   	
	// If on both sides, count the visible faces split
	d_front = d_back = 0.0f;

	for (pSide = Brush->Sides, i=0 ; i<Brush->NumSides ; i++, pSide++)
	{
		jeVec3d	*pVert;

		if (pSide->Flags & SIDE_NODE)
			continue;	
	#if 1
		if (!(pSide->Flags & SIDE_VISIBLE))
			continue;		
	#endif

		p = pSide->Poly;

		if (!p)
			continue;

		Front = Back = 0;

		for (pVert = p->Verts, j=0 ; j<p->NumVerts; j++, pVert++)
		{
		#if 1
			d = jePlane_PointDistanceFast(pPlane, pVert);
		#else
			d = DotProduct (pVert, &Plane->Normal) - Plane->Dist;
		#endif

			if (d > d_front)
				d_front = d;
			else if (d < d_back)
				d_back = d;

			if (d > 0.1) 
				Front = 1;
			else if (d < -0.1) 
				Back = 1;
		}

		if (Front && Back)
		{
			if (!(pSide->Flags & SIDE_SKIP))
			{
				(*NumSplits)++;
				if (pSide->Flags & SIDE_HINT)
					*HintSplit = JE_TRUE;
			}
		}
	}

	if ( (d_front > 0.0 && d_front < 1.0) || (d_back < 0.0 && d_back > -1.0) )
		(*EpsilonBrush)++;

#if 0
	if (*NumSplits == 0)
	{	
		if (Front)
			s = PSIDE_FRONT;
		else if (Back)
			s = PSIDE_BACK;
		else
			s = 0;
	}
#endif

	return s;
}

//=======================================================================================
//	jeBSP_CheckPlaneAgainstParents
//	Makes sure no plane gets used twice in the tree, from the children up.  
//=======================================================================================
static jeBoolean jeBSP_CheckPlaneAgainstParents(jePlaneArray_Index Index, jeBSPNode *Node)
{
	jeBSPNode	*p;

	for (p=Node->Parent ; p ; p=p->Parent)
	{
		if (jePlaneArray_IndexIsCoplanar(p->PlaneIndex, Index))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_CheckPlaneAgainstVolume
//	Makes sure that a potential splitter does not make tiny volumes from the parent volume
//=======================================================================================
static jeBoolean jeBSP_CheckPlaneAgainstVolume(jeBSP *BSP, jePlaneArray_Index Index, jeBSPNode *Node)
{
	jeBSP_Brush	*Front, *Back;
	jeBoolean	Good;

	jeBSP_BrushSplit(Node->Volume, BSP, Index, SIDE_NODE, &Front, &Back);

	Good = (Front && Back);

	if (Front)
		jeBSP_BrushDestroy(&Front);
	if (Back)
		jeBSP_BrushDestroy(&Back);

	return Good;
}

//=======================================================================================
//	jeBSP_GetSplitter
//=======================================================================================
static jeBSP_Side *jeBSP_GetSplitter(jeBSP *BSP, jeBSP_Brush *Brushes, jeBSPNode *Node)
{
	int32			Value, BestValue;
	jeBSP_Brush		*Brush, *Test;
	jeBSP_Side		*Side, *BestSide;
	int32			i, j, Pass, NumPasses;
	jePlane_Side	s;
	int32			Front, Back, Both, Facing, Splits;
	int32			BSplits;
	int32			BestSplits;
	int32			EpsilonBrush;
	jeBoolean		HintSplit;

	if (!Brushes)
		return NULL;

	BestSide	= NULL;
	BestValue	= -999999;
	BestSplits	= 0;

	// The search order goes: 
	//		Visible-Structural, Visible-Detail,
	//		NonVisible-Structural, NonVisible-Detail.
	// If any valid plane is available in a pass, no further passes will be tried.
	NumPasses = 4;
	for (Pass = 0 ; Pass < NumPasses ; Pass++)
	{
		for (Brush = Brushes ; Brush ; Brush=Brush->Next)
		{
			const jePlane		*pPlane;

		#ifdef USE_DETAIL
			if ( (Pass & 1) && !(Brush->Original->Contents & JE_BSP_CONTENTS_DETAIL))
				continue;
			if ( !(Pass & 1) && (Brush->Original->Contents & JE_BSP_CONTENTS_DETAIL))
				continue;
		#endif
			
			for (i=0 ; i<Brush->NumSides ; i++)
			{
				jePlaneArray_Index		Index;

				Side = &Brush->Sides[i];
				
				if (!Side->Poly)
					continue;	// No Poly, so it can't split
				if (Side->Flags & (SIDE_NODE|SIDE_TESTED|SIDE_SPLIT))
					continue;	// Already a node splitter/Or already tested
				if (Side->Flags & SIDE_SKIP)
					continue;
				if (!(Side->Flags & SIDE_VISIBLE) && Pass<2)
					continue;	// Only check visible faces on first pass

				// Treat index as a node (always positive)
				Index = jePlaneArray_IndexGetPositive(Side->PlaneIndex);

				assert(jeBSP_CheckPlaneAgainstParents(Index, Node) == JE_TRUE);
				
				if (Node->Volume)
				{
					assert(g_Logic >= Logic_Smart);

					if (!jeBSP_CheckPlaneAgainstVolume(BSP, Index, Node))
						continue;	// Would produce a tiny volume
				}
				
				Front = 0;
				Back = 0;
				Both = 0;
				Facing = 0;
				Splits = 0;
				EpsilonBrush = 0;

				for (Test = Brushes ; Test ; Test=Test->Next)
				{
					s = jeBSP_TestBrushToPlaneIndex(BSP, Test, Index, &BSplits, &HintSplit, &EpsilonBrush);

					Splits += BSplits;

					if (BSplits && (s&PSIDE_FACING) )
					{
						jeErrorLog_AddString(-1, "jeBSP_GetSplitter:  Brush non-convex.", NULL);
						return NULL;
					}

					Test->TestSide = s;

					// If the brush shares this face, don't bother
					// testing that facenum as a splitter again
					if (s & PSIDE_FACING)
					{
						Facing++;
						for (j=0 ; j<Test->NumSides ; j++)
						{
							if (jePlaneArray_IndexIsCoplanar(Test->Sides[j].PlaneIndex, Index))
								Test->Sides[j].Flags |= SIDE_TESTED;
						}
					}

					if (s & PSIDE_FRONT)
						Front++;
					if (s & PSIDE_BACK)
						Back++;
					if (s == PSIDE_BOTH)
						Both++;
				}

				// Give a value estimate for using this plane
				Value = 10*Facing - ((10-g_LogicBalance)*Splits) - (g_LogicBalance*abs(Front-Back));
				//Value = 5*Facing - 5*Splits - abs(Front-Back);
				//Value = 0 - ((10-g_LogicBalance)*Splits) - (g_LogicBalance*abs(Front-Back));
				//Value = -10*Splits;
				
				pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Index);

				if (pPlane->Type < 3)
					Value+=10;					// Try to get axial aligned planes at top of tree
				
				Value -= EpsilonBrush*1000;		// Try to avoid splits that will cause tiny leafs

				// Never split a hint side except with another hint
				if (HintSplit && !(Side->Flags & SIDE_HINT) )
					Value = -999999;

				// Save off the side test so we don't need
				// to recalculate it when we actually seperate
				// the brushes
				if (Value > BestValue)
				{
					BestValue = Value;
					BestSide = Side;
					BestSplits = Splits;

					for (Test = Brushes ; Test ; Test=Test->Next)
						Test->Side = Test->TestSide;

					//if (g_Logic == Logic_Lazy)
					//	break;
				}
			}

			if (g_Logic == Logic_Lazy)
			{
				if (BestSide)		// Just take the first side in lazy mode
					break;
			}
		}

		// Bail out now, if we found a good side
		if (BestSide)
		{
			if (Pass > 1)
			{
				//BSP->DebugInfo.NumNodes--;
				NumNonVisNodes++;
			}
			
			if (Pass > 0)						// The node is not visible, or detail, so just mark it detail...
				Node->Flags |= NODE_DETAIL;	
			
			break;
		}
	}

	// Clear all the tested flags we set
	for (Brush = Brushes ; Brush ; Brush=Brush->Next)
	{
		for (i=0, Side = Brush->Sides ; i<Brush->NumSides ; i++, Side++)
			Side->Flags &= ~SIDE_TESTED;
	}

	return BestSide;
}

//=====================================================================================
//	jeBSP_FillLeafsFromEntities
//=====================================================================================
static jeBoolean jeBSP_FillLeafsFromEntities(jeBSP *Tree, int32 Fill)
{
	jeBoolean	Empty;
	int32		i;

	assert(Tree);

	Empty = JE_FALSE;
	//Entity = NULL;

	//while (Entity = jeEntity_SetGetNextEntity(EntitySet, Entity))
	for (i=0; i<1; i++)
	{
		jeVec3d			Pos;
		jeBSPNode_Leaf	*Leaf;

		//jeEntity_GetPosition(Entity, &Pos);
		jeVec3d_Set(&Pos, 0.0f, 0.0f, 0.0f);

		// Get the leaf to fill from
		Leaf = jeBSPNode_FindLeaf(Tree->RootNode, Tree, &Pos);
		assert(Leaf);

		if (Leaf->Contents & JE_BSP_CONTENTS_SOLID)
		{
			//Log_Printf("jeBSP_FillLeafsFromEntities:  Entity in solid space.\n");
			continue;
		}

		Empty = JE_TRUE;

		// Fill from this leaf
		jeBSPNode_LeafFill_r(Leaf, Fill);
	}

	if (!Empty)
	{
		jeErrorLog_AddString(-1, "jeBSP_FillLeafsFromEntities:  No entities in empty space.\n", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

extern int32 NumFilledLeafs;

//=====================================================================================
//	jeBSP_RemoveHiddenLeafs
//=====================================================================================
static jeBoolean jeBSP_RemoveHiddenLeafs(jeBSP *Tree)
{
	assert(Tree);

	Log_Printf("--- jeBSP_RemoveHiddenLeafs --- \n");

	if (!jeBSP_FillLeafsFromEntities(Tree, 1))
		return JE_FALSE;

	jeBSPNode_FillUnTouchedLeafs_r(Tree->RootNode, 1);

	Log_Printf("Num Filled Leafs       : %5i\n", NumFilledLeafs);

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_MarkVisibleTopSides
//=======================================================================================
static jeBoolean jeBSP_MarkVisibleTopSides(jeBSP *Tree)
{
	int32			j;
	jeBSP_TopBrush	*TopBrush;

	Log_Printf("--- jeBSP_MarkVisibleTopSides--- \n");

	// Clear all the visible flags
	for (TopBrush = Tree->TopBrushes; TopBrush; TopBrush = TopBrush->Next)
	{
		for (j=0 ; j<TopBrush->NumSides ; j++)
			TopBrush->TopSides[j].Flags &= ~SIDE_VISIBLE;
	}
	
	// Set visible flags on the sides that are used by portals
	jeBSP_MarkVisibleTopSides_r (Tree, Tree->RootNode);

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_MarkVisibleTopSides_r
//=======================================================================================
static void jeBSP_MarkVisibleTopSides_r(jeBSP *BSP, jeBSPNode *Node)
{
	jeBSPNode_Portal	*p;
	int32				s;

	// Recurse to leafs 
	if (!(Node->Flags & NODE_LEAF))
	{
		assert(!Node->Leaf);
		jeBSP_MarkVisibleTopSides_r(BSP, Node->Children[0]);
		jeBSP_MarkVisibleTopSides_r(BSP, Node->Children[1]);
		return;
	}
	assert(Node->Leaf);

	if (!Node->Leaf->Contents)
		return;

	for (p=Node->Portals ; p ; p = p->Next[s])
	{
		s = (p->Nodes[1] == Node);

		if (!p->OnNode)
		{
			assert(p->Nodes[!s]->Flags & NODE_OUTSIDE);	// Only outside nodes don't set onnode for portals
			assert(p->Nodes[!s]->Flags & NODE_LEAF);
			assert(p->Nodes[!s]->Leaf);
			assert(p->Nodes[!s]->Leaf->Node == p->Nodes[!s]);
			continue;
		}

		// For all sides not found for portals, try to find a side
		if (!(p->Flags & PORTAL_SIDE_FOUND))
			jeBSPNode_PortalFindTopSide(p, BSP, s);

		// If the portal has a side, mark the side as visible
		if (p->Side)
			p->Side->Flags |= SIDE_VISIBLE;		
	}

}

//=======================================================================================
//	jeBSP_CreatePortals
//=======================================================================================
static jeBoolean jeBSP_CreatePortals(jeBSP *Tree, jeBoolean IncludeDetail)
{
	assert(Tree);

	jeBSP_DestroyAllPortals(Tree);		// First, make sure no portals are in tree

	// Create portals
	Tree->OutsideNode = jeBSPNode_InitializeRootPortals(Tree->RootNode, Tree);

	if (!Tree->OutsideNode)
	{
		jeErrorLog_AddString(-1, "jeBSP_CreatePortals:  jeBSPNode_InitializeRootPortals failed.", NULL);
		return JE_FALSE;
	}

	if (!jeBSPNode_PartitionPortals_r(Tree->RootNode, Tree, IncludeDetail))
	{
		jeErrorLog_AddString(-1, "jeBSP_CreatePortals:  jeBSPNode_PartitionPortals_r failed.", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeBSP_DestroyAllPortals
//=======================================================================================
static void jeBSP_DestroyAllPortals(jeBSP *Tree)
{
	assert(Tree);

	if (!Tree->RootNode)
	{
		assert(!Tree->OutsideNode);
		return;
	}

	jeBSPNode_DestroyPortals_r(Tree->RootNode, Tree);

	if (Tree->OutsideNode)
		jeBSPNode_Destroy_r(&Tree->OutsideNode, Tree);
}

//=====================================================================================
//	Dynamic Lights
//=====================================================================================

//=====================================================================================
//	CombineDLightWithRGBMapFastLightingModel
//=====================================================================================
static jeBoolean CombineDLightWithRGBMapFastLightingModel(jeBSP *BSP, int32 *LightData, jeBSPNode_Light *Light, jeBSPNode_DrawFace *Face)
{
	jeBoolean			Hit;
	jeFloat				Radius, Dist;
	const jePlane		*pPlane;
	const jeTexVec		*pTexVec;
	int32				Sx, Sy, x, y, u, v, Val;
	int32				ColorR, ColorG, ColorB, Radius2, Dist2;
	int32				FixedX, FixedY, XStep, YStep;
	jeBSPNode_Lightmap	*Lightmap;
	
	assert(LightData);
	assert(Light);
	assert(Face);
	assert(Face->Lightmap);

	Lightmap = Face->Lightmap;

	pTexVec = jeTexVec_ArrayGetTexVecByIndex(BSP->TexVecArray, Face->TexVecIndex);

	pPlane = jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Face->PlaneIndex);
	assert(pPlane);

	Radius = Light->Radius;

	Dist = jePlane_PointDistanceFast(pPlane, &Light->Pos);

	// Subtract dist from radius, so we can project light on surface and calculate in 2d
	Radius -= (float)fabs(Dist);

	if (Radius <= 0.0f)		// We can leave now if the dist is > radius
		return JE_FALSE;

	// Calculate where light is projected onto the 2d-plane
	Sx = (int32)(jeVec3d_DotProduct(&Light->Pos, &pTexVec->VecU));
	Sy = (int32)(jeVec3d_DotProduct(&Light->Pos, &pTexVec->VecV));

	// Align with upper-left of lightmap in 2d space
	Sx -= (int32)(Lightmap->StartU+Face->FixShiftU);
	Sy -= (int32)(Lightmap->StartV+Face->FixShiftV);

	// Scale by the texture scaling (1:21:10 fixed)
	Sx *= Lightmap->XScale;
	Sy *= Lightmap->YScale;

	Hit = JE_FALSE;
	
	ColorR = Light->R;
	ColorG = Light->G;
	ColorB = Light->B;

	Radius2 = (int32)Radius;
	
	XStep = Lightmap->XStep;
	YStep = Lightmap->YStep;

	FixedY = Sy;

	for (v=0; v< Lightmap->Height; v++)
	{
		y = FixedY >> 10;

		if (y < 0)
			y = -y;

		FixedX = Sx;
		
		for (u=0; u< Lightmap->Width; u++)
		{
			x = FixedX >> 10;

			if (x<0)
				x = -x;

			if (x > y)
				Dist2 = (x + (y>>1));
			else
				Dist2 = (y + (x>>1));
			
			if (Dist2 < Radius2)
			{
				Hit = JE_TRUE;
				
				Val = (Radius2 - Dist2);

				*LightData++ += (int32)(Val * ColorR);
				*LightData++ += (int32)(Val * ColorG);
				*LightData++ += (int32)(Val * ColorB);
			}
			else
				LightData+=3;

			FixedX -= XStep;
		}

		FixedY -= YStep;
	}

	return Hit;
}

//=======================================================================================
//	CombineDLightWithRGBMap
//=======================================================================================
static jeBoolean CombineDLightWithRGBMap(jeBSP *BSP, int32 *LightData, jeBSPNode_Light *Light, jeBSPNode_DrawFace *Face)
{
	int32				i, p, NumPoints;
	jeVec3d				*Points, *pPoint;	
	jeVec3d				RGB[MAX_LIGHTMAP_WH*MAX_LIGHTMAP_WH], *pRGB;		// 5k 
	jeBSPNode_Lightmap	*Lightmap;
	jePlane				Plane;
	jeFloat				Dist;
	jeBoolean			DoubleSided;

	Lightmap = Face->Lightmap;
	assert(Lightmap);

	NumPoints = Lightmap->Width*Lightmap->Height;

	assert(NumPoints < MAX_LIGHTMAP_WH*MAX_LIGHTMAP_WH);

	Points = Lightmap->Points;

	if (!Points)
		return JE_FALSE;

	Plane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Face->PlaneIndex);

	if (jePlaneArray_IndexSided(Face->PlaneIndex))
		jePlane_Inverse(&Plane);

	DoubleSided = (Face->Contents & JE_BSP_CONTENTS_EMPTY);

	Dist = jePlane_PointDistance(&Plane, &Light->Pos);

	if (!DoubleSided)
	{
		if (Dist < 0.001f)
			return JE_FALSE;		// Light behind surface
	}

	if (jeVec3d_DistanceBetween(&Face->Center, &Light->Pos) > Face->Radius + Light->Radius)
		return JE_FALSE;		// Light is NOT in radius of face

	pPoint = Points;
	pRGB = RGB;

	for (p=0; p< NumPoints; p++, pPoint++, pRGB++)
	{
		jeVec3d		Vect;
		jeFloat		Dist, Angle, Val;

		jeVec3d_Subtract(&Light->Pos, pPoint, &Vect);
		Dist = jeVec3d_Normalize(&Vect);

		Angle = jeVec3d_DotProduct(&Vect, &Plane.Normal);

		if (Angle <= 0.001f)							
		{
			if (DoubleSided)
				Angle = (jeFloat)fabs(Angle);
			else 
				goto Skip;
		}
			
		Val = (Light->Radius - Dist) * Angle;

		if (Val <= 0.0f)
			goto Skip;	// Light out of radius for this point

		if (BSP->RootNode)
		{
			if (jeBSPNode_RayIntersects_r(BSP->RootNode, BSP, pPoint, &Light->Pos))
				goto Skip;	// Ray is in shadow
		}

		// Add this lights color to the lightmap data
		jeVec3d_Scale(&Light->Color, Val, pRGB);
		continue;

		Skip:
		{
			jeVec3d_Clear(pRGB);
		}
	}

	pRGB = RGB;

	// Put the light into the lightmaps data, scaling, then clamping to MaxLight
	for (p=0; p< NumPoints; p++, pRGB++)
	{
	#if 0
		jeFloat		Max, Max2;

		// Clamp light to a min of 1.0f, and find max
		Max = 0.0f;
		for (i=0; i<3; i++)
		{
			jeFloat		Val;

			Val = jeVec3d_GetElement(pRGB, i);
			if (Val < 1.0f)
				jeVec3d_SetElement(pRGB, i, 1.0f);
			if (Val > Max)
				Max = Val;
		}

		Max2 = min(Max, MaxLight);
			
		// Copy work RGB into real lightmap, clamping to MaxLight
		for (i=0; i<3; i++)
			*LightData++ += (uint32)(jeVec3d_GetElement(pRGB, i)*Max2/Max)<<LIGHT_FRACT;
	#else	
		for (i=0; i<3; i++, LightData++)
		{
			jeFloat	Val;
			
			Val = min(jeVec3d_GetElement(pRGB, i), MaxLight);

			(*LightData) += (uint32)(Val*(1<<LIGHT_FRACT));
		}
	#endif
	}

	return JE_TRUE;
}

//=====================================================================================
//	AddLightType1
//=====================================================================================
static void AddLightType1(int32 *LightDest, uint8 *LightData, int32 Size, int32 Intensity)
{	
	int32	h;

	assert(LightDest != NULL);
	assert(LightData != NULL);

	for (h = 0; h < Size; h++)
	{
		*LightDest++ = *LightData++ * Intensity;
		*LightDest++ = *LightData++ * Intensity;
		*LightDest++ = *LightData++ * Intensity;
	}
}

//=====================================================================================
//	AddLightType2
//=====================================================================================
static void AddLightType2(int32 *LightDest, uint8 *LightData, int32 Size, int32 Intensity)
{	
	int32	h;

	assert(LightDest != NULL);
	assert(LightData != NULL);

	for (h = 0; h < Size; h++)
	{
		*LightDest++ += *LightData++ * Intensity;
		*LightDest++ += *LightData++ * Intensity;
		*LightDest++ += *LightData++ * Intensity;
	}
}

DRV_RGB		BlankLight[MAX_LIGHTMAP_WH*MAX_LIGHTMAP_WH];
int32		TempRGB32[MAX_LIGHTMAP_WH*MAX_LIGHTMAP_WH*3];
DRV_RGB		FinalRGB[MAX_LIGHTMAP_WH*MAX_LIGHTMAP_WH];

//=======================================================================================
//	jeBSP_SetupLightmap
//=======================================================================================
static void JETCC jeBSP_SetupLightmap(jeRDriver_LMapCBInfo *Info, void *LMapCBContext)
{
	int32				Intensity;
	jeBSPNode_DrawFace	*pFace;
	int32				i;
	jeBSP				*BSP;
	int32				Size;
	jeBSPNode_Lightmap	*pLightmap;
	int32				*pRGB1;
	DRV_RGB				*pRGB2;

	pFace = (jeBSPNode_DrawFace *)LMapCBContext;
	assert( pFace->Lightmap );
	BSP = pFace->BSP;
	assert(BSP);

	pLightmap = pFace->Lightmap;

	assert(pLightmap->Width <= MAX_LIGHTMAP_WH);
	assert(pLightmap->Height <= MAX_LIGHTMAP_WH);

	Info->Dynamic = (jeBoolean)pFace->Lightmap->Dynamic;
	pFace->Lightmap->Dynamic = 0;
	
	if (pFace->DLightVisFrame != BSP->DLightVisFrame /* && Lightmap->NumStyles == 1*/)
	{
		// If there is no dynamic light and only 1 layer, then we can short-curcuit the layering/clamping process
		if (pFace->Lightmap->RGBData[0])
			Info->RGBLight[0] = (DRV_RGB*)pLightmap->RGBData[0];		// Style 0
		else
			Info->RGBLight[0] = BlankLight;

		goto FogOnly;
	}

	// Get the lightmap size
	Size = pLightmap->Width*pLightmap->Height;

	// Layer all the Styles together
	//for (i=0; i< Lightmap->NumStyles; i++)
	for (i=0; i< 1; i++)
	{
		if (!i)
			AddLightType1(TempRGB32, pLightmap->RGBData[i], Size, 1<<LIGHT_FRACT);	// Set for first time
		else	
			AddLightType2(TempRGB32, pLightmap->RGBData[i], Size, 1<<LIGHT_FRACT);	// Merge in with light
	}

	// Merge in the dynamic lights
	if (pFace->DLightVisFrame == BSP->DLightVisFrame)			// Face has some dlights
	{
		for (i=0; i< BSP->NumDLights; i++)
		{
			jeBSPNode_Light		*Light;

			if (!(pFace->DLights & (1<<i)))
				continue;		// Light was not touching the face

			Light = &BSP->DLights[i];

			if (Light->Flags & JE_LIGHT_FLAG_FAST_LIGHTING_MODEL)
			{
				if (CombineDLightWithRGBMapFastLightingModel(BSP, TempRGB32, Light, pFace))
					Info->Dynamic = JE_TRUE;
			}
			else 
			{
				if (CombineDLightWithRGBMap(BSP, TempRGB32, Light, pFace))
					Info->Dynamic = JE_TRUE;
			}
		}
	}

	// Clamp and Copy 32-bit data over to 8-bit data
	pRGB1 = TempRGB32;
	pRGB2 = FinalRGB;
	for (i=0; i< Size; i++)
	{
		Intensity = (*pRGB1++) >> LIGHT_FRACT;
		if (Intensity > 255)
			Intensity = 255;
		else if (Intensity < 0 )
			Intensity = 0;

		pRGB2->r = (uint8)Intensity;

		Intensity = (*pRGB1++) >> LIGHT_FRACT;
		if (Intensity > 255)
			Intensity = 255;
		else if (Intensity < 0 )
			Intensity = 0;

		pRGB2->g = (uint8)Intensity;

		Intensity = (*pRGB1++) >> LIGHT_FRACT;
		if (Intensity > 255)
			Intensity = 255;
		else if (Intensity < 0 )
			Intensity = 0;

		pRGB2->b = (uint8)Intensity;

		pRGB2++;
	}

	// Point the lightmap to the 8-bit data
	Info->RGBLight[0] = FinalRGB;

	FogOnly:
	
	// Set fog to NULL for now
	//Lamex
	Info->RGBLight[1] = NULL;
	//Info->RGBLight[1] = Info->RGBLight[1];
}

//========================================================================================
//	UpdateDLights
//========================================================================================
static jeBoolean UpdateDLights(jeBSP *BSP)
{
	jeChain_Link		*Link;

	assert(BSP);

	if (!BSP->RootNode)
	{
		BSP->NumDLights = 0;
		return JE_TRUE;
	}

	if (BSP->RenderRecursion)		
		return JE_TRUE;

	BSP->NumDLights = 0;

	BSP->DLightVisFrame++;

	for (Link = jeChain_GetFirstLink(BSP->DLightChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeLight			*Light;
		jeBSPNode_Light	*BSPLight;
		jeFloat			Brightness;

		Light = (jeLight*)jeChain_LinkGetLinkData(Link);
		BSPLight = &BSP->DLights[BSP->NumDLights];

		if (!jeLight_GetAttributes(Light, &BSPLight->Pos, &BSPLight->Color, &BSPLight->Radius, &Brightness, &BSPLight->Flags))
			return JE_FALSE;

		// Rotate the light into the BSP, so we can send it down the tree
		jeXForm3d_Transform(&BSP->WorldToModelXForm, &BSPLight->Pos, &BSPLight->Pos);

		if (BSPLight->Color.X > 1.0f || BSPLight->Color.Y > 1.0f || BSPLight->Color.Z > 1.0f)
			jeVec3d_Scale(&BSPLight->Color, 1.0f/255.0f, &BSPLight->Color);

		jeVec3d_Scale(&BSPLight->Color, Brightness, &BSPLight->Color);

		BSPLight->R = COLOR_TO_FIXED(BSPLight->Color.X);
		BSPLight->G = COLOR_TO_FIXED(BSPLight->Color.Y);
		BSPLight->B = COLOR_TO_FIXED(BSPLight->Color.Z);

		if (!jeBSPNode_SetDLight_r(BSP->RootNode, BSP, BSPLight, 1<<BSP->NumDLights, BSP->DLightVisFrame))
			return JE_FALSE;
		
		BSP->NumDLights++;

		if (BSP->NumDLights >= MAX_VISIBLE_DLIGHTS)
			break;
	}

	return JE_TRUE;
}

//=======================================================================================
//	UpdateObjects
//=======================================================================================
static jeBoolean UpdateObjects(jeBSP *BSP)
{
	jeChain_Link	*Link;

	for (Link = jeChain_GetFirstLink(BSP->BSPObjectChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeBSP_Object		*BSPObject;
		jeBSPNode_Area		*Area;
		jeXForm3d			XForm;

		BSPObject = (jeBSP_Object*)jeChain_LinkGetLinkData(Link);
		
		jeObject_GetXForm(BSPObject->Object, &XForm);

		Area = jeBSP_FindArea(BSP, &XForm.Translation);

		// KROUER: use a render flag
		if (Area != NULL) { // only if a valid area exist
			if (BSPObject->Object->Methods->Type == JE_OBJECT_TYPE_ACTOR || BSPObject->Object->Methods->Type == JE_OBJECT_TYPE_TERRAIN) {
				jeObject_SetRenderNextPass(BSPObject->Object, JE_FALSE);
				//jeActor_SetRenderNextTime((jeActor*) BSPObject->Object->Instance, JE_FALSE);
			}
		}

		if (BSPObject->Area == Area)
			continue;		// Area has not changed, so do nothing

		// Remove the object from the old area (if any)
		if (BSPObject->Area)
		{
			assert(jeChain_FindLink(BSPObject->Area->ObjectChain, BSPObject->Object));
			jeChain_RemoveLinkData(BSPObject->Area->ObjectChain, BSPObject->Object);
		}

		// Then, add it to the new one
		if (Area)
		{
			assert(!jeChain_FindLink(Area->ObjectChain, BSPObject->Object));
			jeChain_AddLinkData(Area->ObjectChain, BSPObject->Object);
		}

		BSPObject->Area = Area;		// Remember the area
	}

	return JE_TRUE;
}

//=======================================================================================
//	ShutdownDriverCB
//=======================================================================================
static jeBoolean JETCC ShutdownDriverCB(DRV_Driver *Driver, void *Context)
{
	jeBSP	*Tree = (jeBSP*)Context;

	if (Tree->RootNode)
	{
		if (!jeBSPNode_DestroyLightmapTHandles_r(Tree->RootNode, Driver))
			return JE_FALSE;
	}

	Tree->Driver = NULL;

	return JE_TRUE;
}

//=======================================================================================
//	StartupDriverCB
//=======================================================================================
static jeBoolean JETCC StartupDriverCB(DRV_Driver *Driver, void *Context)
{
	jeBSP	*Tree = (jeBSP*)Context;

	if (Tree->RootNode)
	{
		if (!jeBSPNode_CreateLightmapTHandles_r(Tree->RootNode, Driver))
			return JE_FALSE;
	}

	Tree->Driver = Driver;		// This is the current render driver (used to manage lightmaps, rendering, etc)

	return JE_TRUE;
}

//========================================================================================
//	jeBSP_UpdateWorldSpaceBox
//========================================================================================
static jeBoolean jeBSP_UpdateWorldSpaceBox(jeBSP *BSP)

{
	jeExtBox	*Box, *WorldSpaceBox;
	jeVec3d		AxisVecs[3], Center;
	int32		i;

	Box = &BSP->Box;

	WorldSpaceBox = &BSP->WorldSpaceBox;

	// First, caclulate center of bsp
	jeVec3d_Add(&Box->Min, &Box->Max, &Center);
	jeVec3d_Scale(&Center, 0.5f, &Center);

	// Then build some vectors along the box's 3 axis so we can rotate them and get new box
	for(i=0;i < 3;i++)
	{
		jeVec3d_SetElement(&AxisVecs[i], i, jeVec3d_GetElement(&Box->Max, i) - jeVec3d_GetElement(&Center, i));
		jeXForm3d_Rotate(&BSP->ModelToWorldXForm, &AxisVecs[i], &AxisVecs[i]);
	}
		
	// Mask off sign bits
#if 0
	for(i=0;i < 3; i++)
	{
		AxisVecs[i].X = fabs(AxisVecs[i].X);
		AxisVecs[i].Y = fabs(AxisVecs[i].Y);
		AxisVecs[i].Z = fabs(AxisVecs[i].Z);
	}
#else
	for(i=0;i < 3;i++)
	{
		int32		j;

		for(j=0;j < 3;j++)
		{
			*((int32*)(&AxisVecs[i])+j)	= *((int32*)(&AxisVecs[i])+j) & 0x7fffffff;
		}
	}
#endif

	// Get new box by summing up the 3 vectors
	for(i=0;i < 3;i++)
	{
		jeVec3d_SetElement(&WorldSpaceBox->Max, i,
			  jeVec3d_GetElement(&AxisVecs[0], i)
			+ jeVec3d_GetElement(&AxisVecs[1], i)
			+ jeVec3d_GetElement(&AxisVecs[2], i));
	}

	// Inverse max to get min
	WorldSpaceBox->Min = WorldSpaceBox->Max;
	jeVec3d_Inverse(&WorldSpaceBox->Min);

	// Transform to world space
	jeVec3d_Add(&BSP->ModelToWorldXForm.Translation, &Center, &Center);
	jeVec3d_Add(&WorldSpaceBox->Min, &Center, &WorldSpaceBox->Min);
	jeVec3d_Add(&WorldSpaceBox->Max, &Center, &WorldSpaceBox->Max);
		
	// Add some epsilon back in
	for(i=0;i < 3;i++)
	{
		jeVec3d_SetElement(&WorldSpaceBox->Min, i, jeVec3d_GetElement(&WorldSpaceBox->Min, i) - 1.0f);
		jeVec3d_SetElement(&WorldSpaceBox->Max, i, jeVec3d_GetElement(&WorldSpaceBox->Max, i) + 1.0f);
	}

	return JE_TRUE;
}

//=====================================================================================
//	jeBSP_CreateInternalArrays
//=====================================================================================
static jeBoolean jeBSP_CreateInternalArrays(jeBSP *BSP)
{
	BSP->PlaneArray = jePlaneArray_Create(128);		// 128 planes should be good to start with

	if (!BSP->PlaneArray)
		goto ExitWithError;

	BSP->VertArray = jeVertArray_Create(128);		// 128 verts should be good to start with

	if (!BSP->VertArray)
		goto ExitWithError;

	BSP->TexVecArray = jeTexVec_ArrayCreate(6);

	if (!BSP->TexVecArray)
		goto ExitWithError;

	BSP->NodeArray = jeArray_Create(sizeof(jeBSPNode), 128, 128);

	if (!BSP->NodeArray)
		goto ExitWithError;
	
	BSP->DrawFaceArray = jeArray_Create(sizeof(jeBSPNode_DrawFace), 128, 128);

	if (!BSP->DrawFaceArray)
		goto ExitWithError;
		
	return JE_TRUE;
			
	ExitWithError:
	{
		if (BSP->PlaneArray)
			jePlaneArray_Destroy(&BSP->PlaneArray);

		if (BSP->VertArray)
			jeVertArray_Destroy(&BSP->VertArray);

		if (BSP->TexVecArray)
			jeTexVec_ArrayDestroy(&BSP->TexVecArray);

		if (BSP->NodeArray)
			jeArray_Destroy(&BSP->NodeArray);

		if (BSP->DrawFaceArray)
			jeArray_Destroy(&BSP->DrawFaceArray);

		return JE_FALSE;
	}
}

//=====================================================================================
//	jeBSP_DestroyInternalArrays
//		NOTE - This will actually destroy anything that depends on the arrays as well...
//=====================================================================================
static void jeBSP_DestroyInternalArrays(jeBSP *BSP)
{
	assert(BSP);

	// Destroy all vis areas in the BSP
	jeBSP_DestroyVisAreas(BSP);

	// Destroy the stuff the depends on the arrays FIRST...
	if (BSP->RootNode)
		jeBSPNode_Destroy_r(&BSP->RootNode, BSP);

	if (BSP->TopBrushes)
		jeBSP_TopBrushDestroyList(&BSP->TopBrushes, BSP);

	// Then destroy thr actuall arrays
	if (BSP->TexVecArray)
		jeTexVec_ArrayDestroy(&BSP->TexVecArray);

	if (BSP->NodeArray)
		jeArray_Destroy(&BSP->NodeArray);

	if (BSP->DrawFaceArray)
		jeArray_Destroy(&BSP->DrawFaceArray);

	if (BSP->PlaneArray)
		jePlaneArray_Destroy(&BSP->PlaneArray);

	if (BSP->VertArray)
		jeVertArray_Destroy(&BSP->VertArray);
}

//=====================================================================================
//	jeBSP_ResetObjects
//=====================================================================================
static void jeBSP_ResetObjects(jeBSP *BSP)
{
	jeChain_Link	*Link;

	for (Link = jeChain_GetFirstLink(BSP->BSPObjectChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeBSP_Object		*BSPObject;

		BSPObject = (jeBSP_Object*)jeChain_LinkGetLinkData(Link);

		// If the Object has an area, remove it from the areas list
		if (BSPObject->Area)
		{
			assert(jeChain_FindLink(BSPObject->Area->ObjectChain, BSPObject->Object));
			jeChain_RemoveLinkData(BSPObject->Area->ObjectChain, BSPObject->Object);
			BSPObject->Area = NULL;
		}
	}
}

//=====================================================================================
//	jeBSP_ResetGeometry
//		Destroy ALL geometry, TopBrushes, and any internal arrays that the geometry uses
//=====================================================================================
static jeBoolean jeBSP_ResetGeometry(jeBSP *BSP)
{
	assert(BSP);

	assert(BSP->RenderRecursion == 0);

	// Reset the BSP object list
	jeBSP_ResetObjects(BSP);
	// Make sure no portals are in tree
	jeBSP_DestroyAllPortals(BSP);
	// Destroy the current set of internal arrays
	jeBSP_DestroyInternalArrays(BSP);
	// Create new internal arrays
	jeBSP_CreateInternalArrays(BSP);

	// Clear the BSP's Box
	jeVec3d_Clear(&BSP->Box.Min);	
	jeVec3d_Clear(&BSP->Box.Max);	
	jeVec3d_Clear(&BSP->WorldSpaceBox.Min);
	jeVec3d_Clear(&BSP->WorldSpaceBox.Max);

	// Clear other various stuff
	BSP->UpdateFlags = 0;

	BSP->NumDLights = 0;
	BSP->DLightVisFrame = 0;

	BSP->NumBrushes = 0;
	memset(&BSP->DebugInfo, 0, sizeof(BSP->DebugInfo));

	return JE_TRUE;
}

//=====================================================================================
//	jeBSP_DestroyExternalArrays
//=====================================================================================
static void jeBSP_DestroyExternalArrays(jeBSP *BSP)
{
	assert(BSP);

	// Un-ref old arrays (if any)
	if (BSP->FaceInfoArray)
	{
		// If one array is set, then they ALL MUST BE SET
		assert(BSP->FaceInfoArray);
		assert(BSP->MaterialArray);
		assert(BSP->LightChain);
		assert(BSP->DLightChain);

		jeFaceInfo_ArrayDestroy(&BSP->FaceInfoArray);
		jeMaterial_ArrayDestroy(&BSP->MaterialArray);
		jeChain_Destroy(&BSP->LightChain);
		jeChain_Destroy(&BSP->DLightChain);
	}
}

//=====================================================================================
//	jeBSP_CreateExternalArrays
//=====================================================================================
static jeBoolean jeBSP_CreateExternalArrays(jeBSP *BSP, jeFaceInfo_Array *FArray, jeMaterial_Array *MArray, jeChain *LChain, jeChain *DLChain)
{
	// If one array is set, they ALL must be set
	assert(FArray);		
	assert(MArray);
	assert(LChain);
	assert(DLChain);

	// There should be no arrays at this point in the bsp
	assert(!BSP->FaceInfoArray);
	assert(!BSP->MaterialArray);
	assert(!BSP->LightChain);
	assert(!BSP->DLightChain);

	// Increase refs on new incoming arrays
	if (!jeFaceInfo_ArrayCreateRef(FArray))
		goto ExitWithError;

	if (!jeMaterial_ArrayCreateRef(MArray))
		goto ExitWithError;

	if (!jeChain_CreateRef(LChain))
		goto ExitWithError;

	if (!jeChain_CreateRef(DLChain))
		goto ExitWithError;

	// Save off the arrays
	BSP->FaceInfoArray = FArray;
	BSP->MaterialArray = MArray;
	BSP->LightChain = LChain;
	BSP->DLightChain = DLChain;

	return JE_TRUE;

	ExitWithError:
	{
		if (BSP->FaceInfoArray)
			jeFaceInfo_ArrayDestroy(&BSP->FaceInfoArray);
		if (BSP->MaterialArray)
			jeMaterial_ArrayDestroy(&BSP->MaterialArray);
		if (BSP->LightChain)
			jeChain_Destroy(&BSP->LightChain);
		if (BSP->DLightChain)
			jeChain_Destroy(&BSP->DLightChain);

		return JE_FALSE;
	}
}

//=====================================================================================
//	jeBSP_OptimizeDrawFaceVerts
//=====================================================================================
static jeBoolean jeBSP_OptimizeDrawFaceVerts(jeBSP *BSP)
{
	jeVertArray_Optimizer	*Optimizer;

	if (!BSP->RootNode)
		return JE_TRUE;

	Optimizer = jeVertArray_CreateOptimizer(BSP->VertArray);

	if (!Optimizer)
		return JE_FALSE;

	if (!jeBSPNode_WeldDrawFaceVerts_r(BSP->RootNode, BSP, Optimizer))
	{
		jeVertArray_DestroyOptimizer(BSP->VertArray, &Optimizer);
		return JE_FALSE;
	}
	
	if (!jeBSPNode_FixDrawFaceTJuncts_r(BSP->RootNode, BSP, Optimizer))
	{
		jeVertArray_DestroyOptimizer(BSP->VertArray, &Optimizer);
		return JE_FALSE;
	}

	jeVertArray_DestroyOptimizer(BSP->VertArray, &Optimizer);
	
	return JE_TRUE;
}


// Krouer : VertexBuffer sort functions

typedef struct
{
	jeMaterialSpec	*pSpec;
	jeChain			*pFaces;
} jeFacesPerMaterial;

jeChain_Link* jeBSP_FindFacesByMaterial(jeChain* Chain, const jeMaterialSpec* pMatSpec, jeFacesPerMaterial** ppSearch)
{
	jeChain_Link* pLink;
	jeFacesPerMaterial* pSearch;

	pSearch = NULL;

	for (pLink = jeChain_GetFirstLink(Chain); pLink; pLink = jeChain_LinkGetNext(pLink))
	{
		pSearch = (jeFacesPerMaterial*) jeChain_LinkGetLinkData(pLink);
		if (pSearch->pSpec == pMatSpec) {
			*ppSearch = pSearch;
			return pLink;
		}
	}

	return NULL;
}

jeChain_Link* jeBSP_FindRenderSection(jeChain* Chain, const jeMaterialSpec* pMatSpec, jeRenderSectionData** ppSearch)
{
	jeChain_Link* pLink;
	jeRenderSectionData* pSearch;

	pSearch = NULL;

	for (pLink = jeChain_GetFirstLink(Chain); pLink; pLink = jeChain_LinkGetNext(pLink))
	{
		pSearch = (jeRenderSectionData*) jeChain_LinkGetLinkData(pLink);
		if (pSearch->Material == pMatSpec) {
			*ppSearch = pSearch;
			return pLink;
		}
	}

	return NULL;
}

static jeBoolean jeBSP_CreateVertexBuffer(jeBSP* Tree)
{
	unsigned long nVertexCount;
	unsigned long nIndexCount;
	unsigned long nIndexOffset;
	unsigned long nVertexOffset;

	jeFacesPerMaterial* pFacesSort;
	jeRenderSectionData* pSectionData;

	jeChain_Link* pDataLink;
	jeChain_Link* pSortedLink;

	// Vertex data
	jeTexVert			*pTexVerts;
	jeVertex			pTLVerts[MAX_TEMP_VERTS];
	jeVertex			*pCurVerts;
	jeIndexPoly			*pPoly;
	jeVertArray_Index	*pIdxVerts;

	jeVertex* pVertexData;
	uint16* pIndexData;
	const jeVec3d			*pVert;

	// Matertial Data
	const jeMaterial		*pMaterial;
	const jeMaterialSpec	*pMatSpec;
	const jeFaceInfo		*pFaceInfo;

	jeBSPNode_DrawFace* pDrawFace;
	jeBSPNode_Leaf* pLeaf;

	nVertexCount = 0;
	nIndexCount = 0;
	nIndexOffset = 0;
	nVertexOffset = 0;

	if (Tree->AreaChain) {
		// Parse all AreaChain
		jeChain_Link* pAreaLink;
		jeBSPNode_Area* pArea;
		jeBSPNode_AreaLeaf* AreaLeaf;

		for (pAreaLink = jeChain_GetFirstLink(Tree->AreaChain); pAreaLink; pAreaLink = jeChain_LinkGetNext(pAreaLink))
		{
			int f;			

			AreaLeaf = NULL;
			// Parse all faces of the Area
			pArea = (jeBSPNode_Area*) jeChain_LinkGetLinkData(pAreaLink);
			// Sort faces per Material
			while (AreaLeaf = (jeBSPNode_AreaLeaf*)jeArray_GetNextElement(pArea->LeafArray, AreaLeaf))
			{
				pLeaf = AreaLeaf->Leaf;
				for(f=0;f<pLeaf->NumDrawFaces;f++)
				{
					pDrawFace = pLeaf->DrawFaces[f];
					pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(Tree->FaceInfoArray, pDrawFace->FaceInfoIndex);
					pMaterial = jeMaterial_ArrayGetMaterialByIndex(Tree->MaterialArray, pFaceInfo->MaterialIndex);
					pMatSpec = jeMaterial_GetMaterialSpec(pMaterial);

					pDataLink = jeBSP_FindFacesByMaterial(pArea->FacesPerMaterialList, pMatSpec, &pFacesSort);
					if (pDataLink)
					{
						// add the face to the material list
						jeChain_AddLinkData(pFacesSort->pFaces, pDrawFace);
					}
					else 
					{
						// Create a new items
						pFacesSort = JE_RAM_ALLOCATE_STRUCT(jeFacesPerMaterial);
						pFacesSort->pSpec = (jeMaterialSpec*) pMatSpec;
						pFacesSort->pFaces = jeChain_Create();

						// Add the face to the material list
						jeChain_AddLinkData(pFacesSort->pFaces, pDrawFace);

						// Add the material key to the Area list
						jeChain_AddLinkData(pArea->FacesPerMaterialList, pFacesSort);
					}

					// Add Portal of Faces has children of the BSP and the Area
					if (pDrawFace->PortalObject)
					{
						jeChain_AddLinkData(pArea->ObjectChain, pDrawFace->PortalObject);
						jeObject_CreateRef(pDrawFace->PortalObject);
					}

					nVertexCount += pDrawFace->Poly->NumVerts; //jeBrush_FaceGetVertCount(pDrawFace->jeBrushFace);
					nIndexCount += pDrawFace->Poly->NumVerts;
				}
			}
		}

		// now all faces are sorted
		//if (Tree->pVertexBuffer) {
		//	jeEngine_DestroyVertexBuffer(Tree->Engine, &Tree->pVertexBuffer);
		//}
		//if (Tree->pIndexBuffer) {
		//	jeEngine_DestroyIndexBuffer(Tree->Engine, &Tree->pIndexBuffer);
		//}
		// create the 2 Buffers
		//Tree->pVertexBuffer = jeEngine_CreateVertexBuffer(Tree->Engine, nVertexCount);
		//Tree->pIndexBuffer = jeEngine_CreateIndexBuffer(Tree->Engine, nIndexCount);

		// lock the 2 buffers
		//jeEngine_LockVertexBuffer(Tree->Engine, Tree->pVertexBuffer, 0, nVertexCount, (void**)&pVertexData);
		//jeEngine_LockIndexBuffer(Tree->Engine, Tree->pIndexBuffer, 0, nIndexCount, (void**)&pIndexData);

		// reparse the areas and fill the vertex and index buffer
		for (pAreaLink = jeChain_GetFirstLink(Tree->AreaChain); pAreaLink; pAreaLink = jeChain_LinkGetNext(pAreaLink))
		{
			pArea = (jeBSPNode_Area*) jeChain_LinkGetLinkData(pAreaLink);

			for (pSortedLink = jeChain_GetFirstLink(pArea->FacesPerMaterialList); pSortedLink; pSortedLink = jeChain_LinkGetNext(pSortedLink))
			{
				pFacesSort = (jeFacesPerMaterial*) jeChain_LinkGetLinkData(pSortedLink);

				pSectionData = JE_RAM_ALLOCATE_STRUCT(jeRenderSectionData);
				pSectionData->StartIndex = nIndexOffset;
				pSectionData->StartVertex = nIndexOffset;
				pSectionData->VertexCount = 0;
				pSectionData->IndexCount = 0;
				pSectionData->Material = pFacesSort->pSpec;

				for (pDataLink = jeChain_GetFirstLink(pFacesSort->pFaces); pDataLink; pDataLink = jeChain_LinkGetNext(pDataLink))
				{
					long IdxVert = 0;
					pDrawFace = (jeBSPNode_DrawFace*) jeChain_LinkGetLinkData(pDataLink);
					//pBrushFace = pDrawFace->jeBrushFace;

					// Fill the TexturedVertex Array
					pPoly = pDrawFace->Poly;
					pIdxVerts = pPoly->Verts;
					pCurVerts = pTLVerts;
					ZeroMemArray(pTLVerts, MAX_TEMP_VERTS);
					pTexVerts = pDrawFace->TVerts;

					for (IdxVert=0; IdxVert<pPoly->NumVerts;IdxVert++)
					{
						pVert = jeVertArray_GetVertByIndex(Tree->VertArray, *pIdxVerts);

						pCurVerts->World.X = pVert->X;
						pCurVerts->World.Y = pVert->Y;
						pCurVerts->World.Z = pVert->Z;

						pCurVerts->Color.r = 255;
						pCurVerts->Color.g = 255;
						pCurVerts->Color.b = 255;
						pCurVerts->Color.a = 255;
						
						pCurVerts->u = pTexVerts->u;
						pCurVerts->v = pTexVerts->v;

						// now offset the index of the value already set
						// now copy the index into the index buffer
						pIndexData[nIndexOffset+IdxVert] = (uint16) (nVertexOffset+IdxVert);
	
						pCurVerts++;
						pTexVerts++;
						pIdxVerts++;
					}

					pSectionData->IndexCount += pPoly->NumVerts;

					// now copy the buffer into the vertex buffer - must be formatted to driver wish
					memcpy(pVertexData+nVertexOffset, pTLVerts, pPoly->NumVerts*sizeof(jeVertex));
					pSectionData->VertexCount += pPoly->NumVerts;

					// update the render section
					nVertexOffset += pPoly->NumVerts;
				}
			}

			jeChain_Destroy(&pArea->FacesPerMaterialList);
		}

		// unlock the buffer
		//jeEngine_UnlockIndexBuffer(Tree->Engine, Tree->pIndexBuffer);
		//jeEngine_UnlockVertexBuffer(Tree->Engine, Tree->pVertexBuffer);
	} else {
		pDrawFace = NULL;
		// Parse all Faces
		while (pDrawFace = (jeBSPNode_DrawFace*) jeArray_GetNextElement(Tree->DrawFaceArray, pDrawFace))
		{
			// Sort faces per Material
			pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(Tree->FaceInfoArray, pDrawFace->FaceInfoIndex);
			pMaterial = jeMaterial_ArrayGetMaterialByIndex(Tree->MaterialArray, pFaceInfo->MaterialIndex);
			pMatSpec = jeMaterial_GetMaterialSpec(pMaterial);

			pDataLink = jeBSP_FindFacesByMaterial(Tree->FacesPerMaterialList, pMatSpec, &pFacesSort);
			if (pDataLink)
			{
				// add the face to the material list
				jeChain_AddLinkData(pFacesSort->pFaces, pDrawFace);
			}
			else
			{
				// Create a new items
				pFacesSort = JE_RAM_ALLOCATE_STRUCT(jeFacesPerMaterial);
				pFacesSort->pSpec = (jeMaterialSpec*) pMatSpec;
				pFacesSort->pFaces = jeChain_Create();

				// Add the face to the material list
				jeChain_AddLinkData(pFacesSort->pFaces, pDrawFace);

				// Add the material key to the Area list
				jeChain_AddLinkData(Tree->FacesPerMaterialList, pFacesSort);
			}

			// Add Portal of Faces has children of the BSP
			if (pDrawFace->PortalObject)
			{
				jeBSP_AddObject(Tree, pDrawFace->PortalObject);
			}

			nVertexCount += pDrawFace->Poly->NumVerts; //jeBrush_FaceGetVertCount(pDrawFace->jeBrushFace);
			nIndexCount += pDrawFace->Poly->NumVerts;
		}

		// lock the 2 buffers
		//if (Tree->pVertexBuffer) {
		//	jeEngine_DestroyVertexBuffer(Tree->Engine, &Tree->pVertexBuffer);
		//}
		//if (Tree->pIndexBuffer) {
		//	jeEngine_DestroyIndexBuffer(Tree->Engine, &Tree->pIndexBuffer);
		//}
		// create the 2 Buffers
		//Tree->pVertexBuffer = jeEngine_CreateVertexBuffer(Tree->Engine, nVertexCount);
		//Tree->pIndexBuffer = jeEngine_CreateIndexBuffer(Tree->Engine, nIndexCount);

		//jeEngine_LockVertexBuffer(Tree->Engine, Tree->pVertexBuffer, 0, nVertexCount, (void**)&pVertexData);
		//jeEngine_LockIndexBuffer(Tree->Engine, Tree->pIndexBuffer, 0, nIndexCount, (void**)&pIndexData);
		
		// Fill the buffers
		for (pSortedLink = jeChain_GetFirstLink(Tree->FacesPerMaterialList); pSortedLink; pSortedLink = jeChain_LinkGetNext(pSortedLink))
		{
			pFacesSort = (jeFacesPerMaterial*) jeChain_LinkGetLinkData(pSortedLink);

			pSectionData = JE_RAM_ALLOCATE_STRUCT(jeRenderSectionData);
			pSectionData->StartIndex = 0;
			pSectionData->StartVertex = 0;
			pSectionData->VertexCount = 0;
			pSectionData->IndexCount = 0;
			pSectionData->Material = pFacesSort->pSpec;

			for (pDataLink = jeChain_GetFirstLink(pFacesSort->pFaces); pDataLink; pDataLink = jeChain_LinkGetNext(pDataLink))
			{
				long IdxVert = 0;
				pDrawFace = (jeBSPNode_DrawFace*) jeChain_LinkGetLinkData(pDataLink);

				// Fill the TexturedVertex Array
				pPoly = pDrawFace->Poly;
				pIdxVerts = pPoly->Verts;
				pCurVerts = pTLVerts;
				ZeroMemArray(pTLVerts, MAX_TEMP_VERTS);
				pTexVerts = pDrawFace->TVerts;

				for (IdxVert=0; IdxVert<pPoly->NumVerts;IdxVert++)
				{
					pVert = jeVertArray_GetVertByIndex(Tree->VertArray, *pIdxVerts);

					pCurVerts->World.X = pVert->X;
					pCurVerts->World.Y = pVert->Y;
					pCurVerts->World.Z = pVert->Z;

					pCurVerts->Color.r = 255;
					pCurVerts->Color.g = 255;
					pCurVerts->Color.b = 255;
					pCurVerts->Color.a = 255;
					
					pCurVerts->u = pTexVerts->u;
					pCurVerts->v = pTexVerts->v;

					// now offset the index of the value already set
					// now copy the index into the index buffer
					pIndexData[nIndexOffset+IdxVert] = (uint16) (nVertexOffset+IdxVert);

					pCurVerts++;
					pTexVerts++;
					pIdxVerts++;
				}

				pSectionData->IndexCount += pPoly->NumVerts;

				// now copy the buffer into the vertex buffer - must be formatted to driver wish
				memcpy(pVertexData+nVertexOffset, pTLVerts, pPoly->NumVerts*sizeof(jeVertex));
				pSectionData->VertexCount += pPoly->NumVerts;

				// update the render section
				nVertexOffset += pPoly->NumVerts;
			}
		}

		// unlock the buffer
		//jeEngine_UnlockIndexBuffer(Tree->Engine, Tree->pIndexBuffer);
		//jeEngine_UnlockVertexBuffer(Tree->Engine, Tree->pVertexBuffer);

		jeChain_Destroy(&Tree->FacesPerMaterialList);
	}
	return JE_TRUE;
}

static jeBoolean jeBSP_RenderVertexBuffer(jeBSP* Tree)
{
	jeChain_Link* pLink;
	
	// Enumerate all sections
	for (pLink = jeChain_GetFirstLink(Tree->RenderDataList); pLink; pLink = jeChain_LinkGetNext(pLink))
	{
		jeRenderSectionData* pSection;
		pSection = (jeRenderSectionData*) jeChain_LinkGetLinkData(pLink);

		// Render the current section
		//Tree->Driver->Render(Tree->pVertexBuffer, Tree->pIndexBuffer, pSection); // TBD
	}
	return JE_TRUE;
}

