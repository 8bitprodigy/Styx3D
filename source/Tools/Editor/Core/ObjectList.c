/****************************************************************************************/
/*  OBJECTLIST.C                                                                        */
/*                                                                                      */
/*  Author:                                                                             */
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

#include "ObjectList.h"
#include "Util.h"

static jeBoolean ObjectList_FindCB( void *p1, void *lParam )
{
	return ( p1 == lParam ) ;
}// ObjectList_FindCB

// END STATIC

ObjectList * ObjectList_Create( void )
{
	return (ObjectList*)List_Create( ) ;
}// ObjectList_Create

void ObjectList_Destroy( ObjectList **ppList, ObjectList_DestroyCallback DestroyFcn )
{
	List_Destroy( (List**)ppList, DestroyFcn ) ;
}// ObjectList_Destroy

//
// ACCESSORS
//
int32 ObjectList_GetNumItems( ObjectList * pList )
{
	assert( pList != NULL ) ;

	return List_GetNumItems( pList ) ;
}// ObjectList_GetNumItems

Object * ObjectList_GetFirst( ObjectList * pList, ObjectIterator * Iterator )
{
	assert( pList != NULL ) ;

	return (Object*)List_GetFirst( pList, Iterator ) ;
}// ObjectList_GetFirst

Object * ObjectList_GetNext( ObjectList * pList, ObjectIterator * Iterator )
{
	assert( pList != NULL ) ;

	return (Object*)List_GetNext( pList, Iterator ) ;
}
Object * ObjectList_GetLast( ObjectList * pList )
{
	ObjectIterator	pBI ;
	assert( pList != NULL ) ;

	return (Object*)List_GetLast( pList, &pBI ) ;
}// ObjectList_GetLast

ObjectIterator ObjectList_Find( ObjectList * pList, Object * pObject )
{
	ObjectIterator	pBI ;
	Object	*		pFoundObject ;

	assert( pList != NULL ) ;

	List_Search( pList, ObjectList_FindCB, pObject, &pFoundObject, &pBI ) ;
	return pBI ;

}// ObjectList_Find

// MODIFIERS
ObjectIterator ObjectList_Append( ObjectList * pList, Object * pObject )
{
	assert( pList != NULL ) ;

	return List_Append( pList, pObject ) ;
}// ObjectList_Append

jeBoolean ObjectList_AppendNoDup( ObjectList * pList, Object * pObject )
{
	ObjectIterator	pBI ;
	jeBoolean		bFound ;
	Object	*		pFoundObject ;
	assert( pList != NULL ) ;

	bFound = List_Search( pList, ObjectList_FindCB, pObject, &pFoundObject, &pBI ) ;
	if( bFound == JE_FALSE )
	{
		if( ObjectList_Append( pList, pObject ) == NULL )
			return JE_FALSE ;
	}
	return JE_TRUE ;
}// ObjectList_AppendNoDup

// Krouer: append to a sorted list
ObjectIterator ObjectList_AppendSort( ObjectList * pList, Object * pObject, ObjectList_SortCB Callback)
{
	ObjectIterator it;
	Object* pObj;

	assert( pList != NULL ) ;

	pObj = List_GetFirst(pList, &it);
	while (pObj) {
		if (Callback(pObject, pObj)) {
			break;
		}
		pObj = List_GetNext(pList, &it);
	}
	if (it) {
		return List_InsertBefore( pList, it, pObject ) ;
	}
	return List_Append(pList, pObject);
}// ObjectList_AppendSort


void ObjectList_Remove( ObjectList * pList, Object * pObject )
{
	ObjectIterator	pBI ;
	jeBoolean		bFound ;
	Object	*		pFoundObject ;

	assert( pList != NULL ) ;

	bFound = List_Search( pList, ObjectList_FindCB, pObject, &pFoundObject, &pBI ) ;
	assert( JE_TRUE == bFound ) ;

	List_Remove( pList, pBI, NULL ) ;
}// ObjectList_Remove

// ENUMERATION

int32 ObjectList_EnumObjects( ObjectList * pList, void * pVoid, ObjectListCB Callback )
{
	assert( pList != NULL ) ;

	return List_ForEach( pList, Callback, pVoid ) ;

}// ObjectList_EnumObjectes

// CALLBACK (public)

// ObjectList_Destroy passes a ptr, not the address of a ptr
void ObjectList_DestroyCB( Object * pObject )
{
	Object * pFreeObject = pObject ;
	assert( pObject != NULL ) ;
	
	Object_Free( &pFreeObject ) ;
}//ObjectList_DestroyCB

static jeBoolean ObjectList_UnionObjectRectCB( Object * pObject, jeExtBox * pListBounds )
{

	jeExtBox  ObjectBounds;

	if( Object_GetWorldAxialBounds(pObject, &ObjectBounds) )
		Util_ExtBox_Union( pListBounds, &ObjectBounds, pListBounds ) ;
	return JE_TRUE ;
}// ObjectList_UnionObjectRectCB

void ObjectList_GetListBounds( ObjectList * pList, jeExtBox * pListBounds )
{
	assert( pList != NULL ) ;
	assert( pListBounds != NULL ) ;
	
	Util_ExtBox_SetInvalid( pListBounds );
	ObjectList_EnumObjects( pList, (void*)pListBounds, ObjectList_UnionObjectRectCB ) ;

}// ObjectList_GetListBounds

static jeBoolean ObjectList_UnionObjectDrawRectCB( Object * pObject, jeExtBox * pListBounds )
{

	jeExtBox  ObjectBounds;

	if( Object_GetWorldDrawBounds(pObject, &ObjectBounds) )
		Util_ExtBox_Union( pListBounds, &ObjectBounds, pListBounds ) ;
	return JE_TRUE ;
}// ObjectList_UnionObjectDrawRectCB

void ObjectList_GetListDrawBounds( ObjectList * pList, jeExtBox * pListBounds )
{
	assert( pList != NULL ) ;
	assert( pListBounds != NULL ) ;
	
	ObjectList_EnumObjects( pList, (void*)pListBounds, ObjectList_UnionObjectDrawRectCB ) ;

}// ObjectList_GetListDrawBounds


/* EOF: ObjectList.h */