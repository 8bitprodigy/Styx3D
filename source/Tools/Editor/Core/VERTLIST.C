/****************************************************************************************/
/*  VERTLIST.C                                                                          */
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
// VertList is a thin wrapper on List, a linked-list module.
// It provides casting and some simple validation

#include <Assert.h>

#include "VertList.h"

VertList * VertList_Create( void )
{
	return (VertList*)List_Create( ) ;
}// VertList_Create

void VertList_Destroy( VertList **ppList, VertList_DestroyCallback DestroyFcn )
{
	List_Destroy( (List**)ppList, DestroyFcn ) ;
}// VertList_Destroy

// ACCESSORS

Vert_Struct	* VertList_GetVert( VertIterator pMI )
{
	Vert_Struct	*	pVert ;
	assert( pMI != NULL ) ;

	pVert = (Vert_Struct *)List_GetData( pMI ) ;

	return pVert ;
}// VertList_GetVert


Vert_Struct	* VertList_GetFirstVert( VertList * pList, VertIterator * pMI )
{
	Vert_Struct	* pVert ;

	assert( pList != NULL ) ;
	assert( pMI != NULL ) ;

	pVert = (Vert_Struct	*)List_GetFirst( pList, pMI ) ;
	assert( pVert ) ;
	return pVert;
}// VertList_GetFirstID

Vert_Struct	* VertList_GetNextVert( VertList * pList, VertIterator * pMI )
{
	Vert_Struct	* pVert ;
	assert( pList != NULL ) ;
	assert( pMI != NULL ) ;

	pVert = (Vert_Struct	*)List_GetNext( pList, pMI ) ;
	if( pVert == NULL )
	{
		return NULL ;
	}
	return pVert ;
}// VertList_GetNextID

int32 VertList_GetNumVert( VertList * pList )
{
	return( List_GetNumItems( pList ) );
}// VirtList_GetNumVert

// MODIFIERS
VertIterator VertList_Append( VertList * pList, Vert_Struct	* pVert )
{
	assert( pList != NULL ) ;

	return List_Append( pList, (void*)pVert ) ;
}// VertList_Append

void VertList_Remove( VertList * pList, jeVertArray_Index Index, VertList_DestroyCallback Callback) 
{
	VertIterator  vI;

	if( !VertList_SearchByIndex( pList, &Index, &vI ) )
		return;

	List_Remove (pList, vI, Callback );
}

// SEARCH
jeBoolean VertList_SearchCB(void *pData, void *lParam)
{

	return( pData == lParam );
}

jeBoolean VertList_Search( VertList * pList, Vert_Struct *  pVert, VertIterator * vI )
{
	void * FoundVoid;
	
	return( List_Search ( pList, VertList_SearchCB, (void*)pVert, &FoundVoid, vI ) );
}

jeBoolean VertList_SearchByIndexCB(void *pData, void *lParam)
{
	jeVertArray_Index	*	Index = (jeVertArray_Index*)lParam;
	Vert_Struct			*	pVert = (Vert_Struct*)pData;
	assert( Index );
	assert( pVert );

	return( pVert->Index == (*Index) );
}

jeBoolean VertList_SearchByIndex( VertList * pList, jeVertArray_Index * Index, VertIterator * vI )
{
	void * FoundVoid;

	return( List_Search ( pList, VertList_SearchByIndexCB, Index, &FoundVoid, vI ) );
}
// ITERATION
int32	VertList_Enum( VertList * pList, void * pVoid, VertListCB Callback )
{
	assert( pList != NULL ) ;

	return List_ForEach( pList, Callback, pVoid ) ;

}// VertList_Enum

/* EOF: VertList.c */