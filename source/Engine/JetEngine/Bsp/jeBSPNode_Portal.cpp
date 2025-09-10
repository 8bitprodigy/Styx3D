/****************************************************************************************/
/*  JEBSPNODE_PORTAL.C                                                                  */
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

static		int32 ActivePortals;
static		int32 PeekPortals;

//=====================================================================================
//	jeBSPNode_PortalCreate
//=====================================================================================
jeBSPNode_Portal *jeBSPNode_PortalCreate(jePoly *Poly, jeBSP *BSP)
{
	jeBSPNode_Portal	*Portal;

	Portal = JE_RAM_ALLOCATE_STRUCT(jeBSPNode_Portal);

	if (!Portal)
		return NULL;

	memset(Portal, 0, sizeof(jeBSPNode_Portal));

	Portal->Poly = Poly;

	ActivePortals++;

	if (ActivePortals > PeekPortals)
		PeekPortals++;

	BSP->DebugInfo.NumPortals++;

	return Portal;
}

//=====================================================================================
//	jeBSPNode_PortalDestroy
//=====================================================================================
void jeBSPNode_PortalDestroy(jeBSPNode_Portal **Portal, jeBSP *BSP)
{
	assert(Portal);
	assert(*Portal);

	if ((*Portal)->Poly)
		jePoly_Destroy(&(*Portal)->Poly);

	//jeBSPNode_PortalResetTopSide(*Portal);

	jeRam_Free(*Portal);

	ActivePortals--;

	BSP->DebugInfo.NumPortals--;

	*Portal = NULL;
}

//=====================================================================================
//	jeBSPNode_PortalIsValid
//=====================================================================================
jeBoolean jeBSPNode_PortalIsValid(const jeBSPNode_Portal *Portal)
{
	jePoly		*Poly;
	jeVec3d		*Verts;
	int32		i, k;
	jeFloat		Val;

	Poly = Portal->Poly;
	Verts = Poly->Verts;

	if (Poly->NumVerts < 3)
		return JE_FALSE;

	for (i=0; i< Poly->NumVerts; i++)
	{
		for (k=0; k<3; k++)
		{
			Val = jeVec3d_GetElement(&Verts[i], k);

			if (Val >= JE_BSP_MINMAX_BOUNDS)
				return JE_FALSE;
		
			if (Val <= -JE_BSP_MINMAX_BOUNDS)
				return JE_FALSE;
		}
	}

	return JE_TRUE;
}

//=====================================================================================
//	jeBSPNode_PortalFindTopSide
//	Examines the brushes on each side of the portal, and finde the side best suited to use
//	for the portal
//=====================================================================================
void jeBSPNode_PortalFindTopSide(jeBSPNode_Portal *Portal, jeBSP *BSP, int32 s)
{
	jeBSP_TopBrush		*TopBrush;

	assert(Portal);
	assert(Portal->Nodes[0]->Leaf);		// Portals SHOULD seperate leafs!
	assert(Portal->Nodes[1]->Leaf);
	assert(Portal->OnNode);

	if (Portal->Flags & PORTAL_SIDE_FOUND)
		return;		// Don't check portals more than once, sice each side checks both leafs

	assert(!Portal->Side);		// There should be no side set yet

	jeBSPNode_GetTopSideSeperatingLeafs(BSP, Portal->Nodes[0]->Leaf, Portal->Nodes[1]->Leaf, Portal->OnNode->PlaneIndex, &TopBrush, &Portal->Side);

	if (Portal->Side)
	{
		Portal->Flags |= PORTAL_SIDE_FOUND;

		if (Portal->Side->FaceInfoIndex == JE_FACEINFO_ARRAY_NULL_INDEX)
			Portal->Side = NULL;	// Cancel out side for this portal, if it has no FaceInfo
	}
}

//=====================================================================================
//	jeBSPNode_PortalResetTopSide
//=====================================================================================
void jeBSPNode_PortalResetTopSide(jeBSPNode_Portal *Portal)
{
	if (Portal->Side)
	{
		assert(Portal->Flags & PORTAL_SIDE_FOUND);
		Portal->Side = NULL;		// Reset the portal side...
	}

	Portal->Flags &= ~PORTAL_SIDE_FOUND;
}

//=====================================================================================
//	jeBSPNode_PortalGetActiveCount
//=====================================================================================
int32 jeBSPNode_PortalGetActiveCount(void)
{
	return ActivePortals;
}

//=====================================================================================
//	jeBSPNode_PortalGetPeekCount
//=====================================================================================
int32 jeBSPNode_PortalGetPeekCount(void)
{
	return PeekPortals;
}

