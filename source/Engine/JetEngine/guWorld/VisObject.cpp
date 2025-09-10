/****************************************************************************************/
/*  VISOBJECT.C                                                                         */
/*                                                                                      */
/*  Author:  Charles Bloom                                                              */
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
/******************

The render pipe here can be a little confusing, so here it is.

1. Every object is added to all the areas contained in its BBox
2. If an object is only in one area, this is flagged as a special case.
3. If an object is visible through only one portal, we render the object
	(either immediately or at the end of the scene) through that portal.
4. If an object is visible through several portals, then :
	A. if it is only in one area, we render through all the portals that see it
	B. if in more than one area, we just render with the camera

The result is that we do not currently have to special case the
	"camera is inside the BBox of the object ? -> just render once"
Because that happens automagically.

********************/

#define DO_DEBUG_INFO

#define DO_RENDER_IMMEDIATE // else delay till all vis is done

#include "Engine.h"
#include "jeFrustum.h"
#include "Object.h"
#include "List.h"
#include "VisObject.h"
#include "Ram.h"
#include "Errorlog.h"
#include "MemPool.h"
#include <assert.h>

#ifndef NDEBUG
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

#ifdef DO_DEBUG_INFO
int NumObjects = 0;
int NumObjectsVisible = 0;
int NumObjectsPortalled = 0;
#define DEBUG_INFO(x) x
#else
#define DEBUG_INFO(x)
#endif

struct jeVisObject
{
	uint32		VisFrame;	// you set these up
	jeFrustum	VisFrustum;
	jeBoolean	VisHasFrustum;	// if ! HasFrustum, use the whole camera frustum
	jeBoolean	InOneArea;
	uint32		AreaUID;

	jeVisObjectList * MyOwner;
	uint32		PrepFrame,FrustumCount;
	jeObject	Object;	
	jeVisObject * VisList;	
	HashNode	* MyHashNode;
};

struct jeVisObjectList
{
	jeVisObjectList * MySelf1;

	Hash * ObjectHash;
	int NumObjects;
	MemPool * VisObjectPool;

	uint32		VisFrame;	// this stuff was set on this frame :
	jeEngine *	Engine;
	jeCamera *	Camera;
	jeFrustum	CamFrustum;

	// jeVisObject * VisList;
	jeVisObjectList * MySelf2;
};

jeVisObjectList *	jeVisObjectList_Create(void)
{
jeVisObjectList * List;
	List = (jeVisObjectList *)jeRam_AllocateClear(sizeof(*List));
	if ( ! List )	
		return NULL;

	List->MySelf1 = List->MySelf2 = List;

	List->ObjectHash = Hash_Create();
	if ( ! List->ObjectHash )
	{
		jeVisObjectList_Destroy(&List);
		return NULL;
	}

	List->VisObjectPool = MemPool_Create(sizeof(jeVisObject),32,64);
	if ( ! List->VisObjectPool )
	{
		jeVisObjectList_Destroy(&List);
		return NULL;
	}

	List->NumObjects = 0;

	assert( Hash_NumMembers(List->ObjectHash) == List->NumObjects );

return List;
}

void jeVisObjectList_Destroy(jeVisObjectList ** pList)
{
jeVisObjectList * List;
jeVisObject * VO;

	assert(pList);
	List = *pList;
	*pList = NULL;
	if ( ! List )
		return;

	VO = jeVisObjectList_GetNext(List,NULL);
	while( VO )
	{
	jeVisObject * VONext;
		VONext = jeVisObjectList_GetNext(List,VO);
		jeVisObjectList_DestroyObject(List,VO);
		VO = VONext;
	}

	assert(List->NumObjects == 0);

	if ( List->ObjectHash )
		Hash_Destroy(List->ObjectHash);

	if ( List->VisObjectPool )
		MemPool_Destroy(&(List->VisObjectPool));

	jeRam_Free(List);
}

jeVisObject * jeVisObjectList_CreateObject(	jeVisObjectList * List,jeObject *Obj)
{
jeVisObject * VO;

	assert( Obj);
	assert( jeVisObjectList_IsValid(List) );

	if ( Hash_Get(List->ObjectHash,(uint32)Obj->Instance,NULL) )
	{
		jeErrorLog_AddString(-1,"VisObjectList_CreateObject : Object already in list !",NULL);
		return NULL;
	}

	VO = (jeVisObject *)MemPool_GetHunk(List->VisObjectPool);
	if ( ! VO )
		return NULL;

	VO->Object = *Obj;
	jeObject_CreateRef(Obj);

	VO->VisFrame = -1;

	assert( Hash_NumMembers(List->ObjectHash) == List->NumObjects );

	VO->MyHashNode = Hash_Add(List->ObjectHash,(uint32)Obj->Instance,(uint32)VO);

	assert( HashNode_Data(VO->MyHashNode) == (uint32)VO );

	List->NumObjects ++;

	assert( Hash_NumMembers(List->ObjectHash) == List->NumObjects );

	DEBUG_INFO(NumObjects++);

	VO->MyOwner = List;

return VO;
}

jeVisObject * jeVisObjectList_FindObject(	const jeVisObjectList * List,const jeObject *Obj)
{
jeVisObject * VO;

	assert( jeVisObjectList_IsValid(List) );

	if ( ! Hash_Get(List->ObjectHash,(uint32)Obj->Instance,(uint32 *)&VO) )
		return NULL;

return VO;
}

void jeVisObjectList_DestroyObject(jeVisObjectList * List,jeVisObject *VO)
{
	assert( VO );
	assert( jeVisObjectList_IsValid(List) );

	assert( HashNode_Data(VO->MyHashNode) == (uint32)VO );
	assert( Hash_NumMembers(List->ObjectHash) == List->NumObjects );
	assert( List->NumObjects > 0 );

#ifdef _DEBUG
	{
	jeVisObject * VO2;
	HashNode * hn;

		if ( ! (hn = Hash_Get(List->ObjectHash,(uint32)(VO->Object.Instance),(uint32 *)&VO2)) )
			assert("object not in hash!" == NULL);

		assert(hn == VO->MyHashNode);
		assert(VO == VO2);
		assert(VO->MyOwner == List);
	}
#endif

	Hash_DeleteNode(List->ObjectHash,VO->MyHashNode);

	jeObject_Free(&(VO->Object));

	List->NumObjects --;

	assert( Hash_NumMembers(List->ObjectHash) == List->NumObjects );
	
	DEBUG_INFO(NumObjects--);
}

void jeVisObjectList_RenderStart(jeVisObjectList * List, const jeEngine *Engine, 
							const jeCamera *Camera, uint32 VisFrame)
{
	assert( jeVisObjectList_IsValid(List) );

	DEBUG_INFO(NumObjectsVisible = NumObjectsPortalled = 0);

	List->Engine = (jeEngine *)Engine;
	List->Camera = (jeCamera *)Camera;
	jeFrustum_SetWorldSpaceFromCamera(&(List->CamFrustum),Camera);
	List->VisFrame = VisFrame;
}

void jeVisObject_RenderInternal(const jeVisObjectList * List,const jeVisObject * VO,uint32 VisFrame)
{
	assert(List->VisFrame == VisFrame);
	if ( VisFrame == VO->VisFrame )
	{
		if ( VO->PrepFrame != VisFrame )
		{
			DEBUG_INFO(NumObjectsVisible++);

			((jeVisObject *)VO)->PrepFrame = VisFrame;

			//jeObject_RenderPrep(&(VO->Object),List->Camera);
		}
	
		if ( VO->VisHasFrustum )
		{
			//jeObject_RenderThroughFrustum(&(VO->Object),List->Engine,&(VO->VisFrustum),VisFrame);
			DEBUG_INFO(NumObjectsPortalled++);
		}
		else
		{
			//jeObject_RenderThroughFrustum(&(VO->Object),List->Engine,&(List->CamFrustum),VisFrame);
		}
		
		DEBUG( ((jeVisObject *)VO)->VisFrame = -1 );
	}
}

void jeVisObject_Render(jeVisObject *VO,const jeFrustum *Frustum,uint32 VisFrame)
{

#ifdef DO_RENDER_IMMEDIATE
	if ( VO->InOneArea )
	{
		assert(VO->MyOwner->VisFrame == VisFrame);

		DEBUG_INFO(NumObjectsPortalled++);

		if ( VO->PrepFrame != VisFrame )
		{
			DEBUG_INFO(NumObjectsVisible++);
			VO->PrepFrame = VisFrame;
			VO->FrustumCount = 0;

			//jeObject_RenderPrep(&(VO->Object),VO->MyOwner->Camera);
		}

		//jeObject_RenderThroughFrustum(&(VO->Object),VO->MyOwner->Engine,Frustum,VisFrame ^ VO->FrustumCount);

		VO->FrustumCount += 256;
		VO->VisFrame = -1; // won't be rendered again

		return;
	}

	// else in more that one area
#endif

	if ( VO->VisFrame == VisFrame )
	{
		// already seen this object this frame from a different frustum
		VO->VisHasFrustum = JE_FALSE;
	}
	else
	{
		VO->VisFrame = VisFrame;
		VO->VisHasFrustum = JE_TRUE;
		VO->VisFrustum = *Frustum;
	}
}

void jeVisObject_MarkVis(jeVisObject *VO,uint32 VisFrame)
{
	VO->VisFrame = VisFrame;
	VO->VisHasFrustum = JE_FALSE;
}

void jeVisObjectList_RenderAll(const jeVisObjectList * List,uint32 VisFrame)
{
HashNode * hn;
DEBUG(int ObjsWalked=0);

	assert(List->VisFrame == VisFrame);
	assert( jeVisObjectList_IsValid(List) );

	hn = NULL;

	assert( Hash_NumMembers(List->ObjectHash) == List->NumObjects );

	while( (hn = Hash_WalkNext(List->ObjectHash,hn)) )
	{
	jeVisObject * VO;
		VO = (jeVisObject *)HashNode_Data(hn);

		assert( hn == VO->MyHashNode );
		assert(VO->MyOwner == List);

		jeVisObject_RenderInternal(List,VO,VisFrame);
		
		DEBUG(ObjsWalked++);
		assert(ObjsWalked <= List->NumObjects );
	}
	assert(ObjsWalked == List->NumObjects );
}

jeVisObject * jeVisObjectList_GetNext(const jeVisObjectList * List,jeVisObject *VO)
{
HashNode * hn;
	assert( jeVisObjectList_IsValid(List) );

	if ( VO )
		hn = VO->MyHashNode;
	else
		hn = NULL;

	hn = Hash_WalkNext(List->ObjectHash,hn);
	
	if ( ! hn )
		return NULL;
			
	VO = (jeVisObject *)HashNode_Data(hn);

	assert( ! VO || VO->MyOwner == List);

return VO;
}

jeBoolean jeVisObjectList_IsValid(const jeVisObjectList * List)
{
	if ( ! List) return JE_FALSE;
	if ( ! (List->MySelf1 == List) ) return JE_FALSE;
	if ( ! (List->MySelf2 == List) ) return JE_FALSE;

	if ( List->NumObjects < 0 )
		return JE_FALSE;

	if ( ! List->ObjectHash )
		return JE_FALSE;

	if ( ! MemPool_IsValid(List->VisObjectPool) )
		return JE_FALSE;

	if ( Hash_NumMembers(List->ObjectHash) != List->NumObjects )
		return JE_FALSE;

return JE_TRUE;
}

const jeObject *	jeVisObject_Object(const jeVisObject *VO)
{
	assert(VO);
	return &(VO->Object);
}

void				jeVisObject_AddArea(jeVisObject *VO,uint32 AreaUID)
{
	if ( AreaUID == 0 )
	{
		VO->InOneArea = JE_TRUE;
	}
	else if ( VO->AreaUID == 0 )
	{
		VO->InOneArea = JE_TRUE;
	}
	else if ( VO->AreaUID != AreaUID )
	{
		VO->InOneArea = JE_FALSE;
	}
	VO->AreaUID = AreaUID;
}
