/****************************************************************************************/
/*  FACELIST.C                                                                          */
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

// FaceList is a thin wrapper on List, a linked-list module.
// It provides casting and some simple validation

#include <Assert.h>

#include "FaceList.h"

FaceList * FaceList_Create( void )
{
	return (FaceList*)List_Create( ) ;
}// FaceList_Create

void FaceList_Destroy( FaceList **ppList, FaceList_DestroyCallback DestroyFcn )
{
	List_Destroy( (List**)ppList, DestroyFcn ) ;
}// FaceList_Destroy

// ACCESSORS

jeBrush_Face * FaceList_GetFace( FaceIterator pMI )
{
	jeBrush_Face	*	pFace ;
	assert( pMI != NULL ) ;

	pFace = List_GetData( pMI ) ;

	return pFace ;
}// FaceList_GetFace


jeBrush_Face* FaceList_GetFirstFace( FaceList * pList, FaceIterator * pMI )
{
	jeBrush_Face	* pFace ;

	assert( pList != NULL ) ;
	assert( pMI != NULL ) ;

	pFace = (jeBrush_Face*)List_GetFirst( pList, pMI ) ;
	return pFace;
}// FaceList_GetFirstID

jeBrush_Face* FaceList_GetNextFace( FaceList * pList, FaceIterator * pMI )
{
	jeBrush_Face	* pFace ;
	assert( pList != NULL ) ;
	assert( pMI != NULL ) ;

	pFace = (jeBrush_Face*)List_GetNext( pList, pMI ) ;
	if( pFace == NULL )
	{
		return NULL ;
	}
	return pFace ;
}// FaceList_GetNextID

int32 FaceList_GetNumFace( FaceList * pList )
{
	return( List_GetNumItems( pList ) );
}// FaceList_GetNumFace

// MODIFIERS
FaceIterator FaceList_Append( FaceList * pList, jeBrush_Face * pFace )
{
	assert( pList != NULL ) ;

	return List_Append( pList, pFace ) ;
}// FaceList_Append


void	FaceList_Remove( FaceList * pList, jeBrush_Face * pFace ) 
{
	FaceIterator  fI;

	if( !FaceList_Search( pList, pFace, &fI ) )
		return;

	List_Remove (pList, fI, NULL );
}

// SEARCH
jeBoolean FaceList_SearchCB(void *pData, void *lParam)
{

	return( pData == lParam );
}

jeBoolean FaceList_Search( FaceList * pList, jeBrush_Face*  pFace, FaceIterator * fI )
{
	void * FoundVoid;

	return( List_Search ( pList, FaceList_SearchCB, (void*)pFace, &FoundVoid, fI ) );
}

// ITERATION
int FaceList_Enum(FaceList * pList, FaceListCB CallbackFcn, void *lParam)
{
	return( List_ForEach( pList, CallbackFcn, lParam ) );
}
/* EOF: FaceList.c */
