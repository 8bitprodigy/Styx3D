/****************************************************************************************/
/*  ENTITYLIST.C                                                                        */
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

// EntityList is a thin wrapper on List, a linked-list module.
// It provides casting and some simple validation

#include <Assert.h>

#include "Entity.h"
#include "ErrorLog.h"

#include "EntityList.h"

#define ENTITY_VERSION	(1)

static void EntityList_DestroyEntityCB( void *p1 )
{
	Entity * pEntity = (Entity*)p1 ;
	assert( pEntity != NULL ) ;

	Entity_Destroy( &pEntity ) ;
}// EntityList_DestroyEntityCB

static jeBoolean EntityList_FindCB( void *p1, void *lParam )
{
	return ( p1 == lParam ) ;
}// EntityList_FindCB

static jeBoolean EntityList_ReattachCB( Entity *pEntity, void* lParam )
{
	return Entity_Reattach( pEntity ) ;
	lParam;
}// EntityList_ReattachCB

static jeBoolean EntityList_WriteCB( Entity *pEntity, void* lParam )
{
	return Entity_WriteToFile( pEntity, (jeVFile*)lParam ) ;
}// EntityList_WriteCB


EntityList * EntityList_Create( void )
{
	return (EntityList*)List_Create( ) ;
}// EntityList_Create

void EntityList_Destroy( EntityList **ppList, EntityList_DestroyCallback DestroyFcn )
{
	List_Destroy( (List**)ppList, DestroyFcn ) ;
}// EntityList_Destroy

// ACCESSORS
int32 EntityList_GetNumItems( const EntityList * pList )
{
	assert( pList != NULL ) ;

	return List_GetNumItems( pList ) ;
}// EntityList_GetNumItems

// IS

// MODIFIERS
EntityIterator EntityList_Append( EntityList * pList, Entity * pEntity )
{
	assert( pList != NULL ) ;

	return List_Append( pList, pEntity ) ;
}// EntityList_Append

void EntityList_Remove( EntityList * pEntityList, Entity * pEntity )
{
	EntityIterator	pBI ;
	jeBoolean		bFound ;
	Entity	*		pFoundEntity ;

	assert( pEntityList != NULL ) ;

	bFound = List_Search( pEntityList, EntityList_FindCB, pEntity, &pFoundEntity, &pBI ) ;
	assert( JE_TRUE == bFound ) ;

	List_Remove( pEntityList, pBI, NULL ) ;
}// LightList_Remove

void EntityList_Delete( EntityList * pEntityList, Entity * pEntity )
{
	EntityList_Remove( pEntityList, pEntity );
	Entity_Destroy( &pEntity );
}// EntityList_Delete


// ENUMERATION

int32 EntityList_Enum( EntityList * pEntityList, void * pVoid, EntityListCB Callback )
{
	assert( pEntityList != NULL ) ;

	return List_ForEach( pEntityList, Callback, pVoid ) ;

}// EntityList_Enum

// FILE HANDLING

EntityList * EntityList_CreateFromFile( jeVFile * pF, jeSymbol_Table * pEntities )
{
	EntityList	*	pList = NULL ;
	Entity		*	pEntity ;
	int32			i ;
	int32			nItems ;
	int32			nVersion ;

	assert( jeVFile_IsValid( pF ) ) ;

	if( !jeVFile_Read( pF, &nVersion, sizeof nVersion ) )
		return NULL ;
	if( nVersion != ENTITY_VERSION )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "EntityList_CreateFromFile.\n", NULL);
		return NULL ;
	}

	if( !jeVFile_Read( pF, &nItems, sizeof nItems ) )
		return NULL ;

	pList = EntityList_Create( ) ;
	if( pList == NULL )
		goto ELCFF_FAILURE ;

	for( i=0; i<nItems; i++ )
	{
		pEntity = Entity_CreateFromFile( pF, nVersion, pEntities ) ;
		if( pEntity == NULL )
			goto ELCFF_FAILURE ;
		if( EntityList_Append( pList, pEntity ) == NULL )
		{	
			Entity_Destroy( &pEntity ) ;
			goto ELCFF_FAILURE ;
		}
	}
	return pList ;

ELCFF_FAILURE :
	if( pList != NULL )
		EntityList_Destroy( &pList, EntityList_DestroyEntityCB ) ;
	return NULL ;

}// EntityList_CreateFromFile


jeBoolean EntityList_WriteToFile( EntityList * pList, jeVFile * pF )
{
	int32	nVersion ;
	int32	nItems ;
	assert( pList != NULL ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	nVersion = ENTITY_VERSION ;
	if( jeVFile_Write( pF, &nVersion, sizeof nVersion ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "EntityList_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	
	nItems = EntityList_GetNumItems( pList ) ;
	if( jeVFile_Write( pF, &nItems, sizeof nItems ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "EntityList_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	return EntityList_Enum( pList, pF, EntityList_WriteCB ) ;

}// EntityList_WriteToFile

jeBoolean EntityList_Reattach( EntityList * pList )
{
	assert( pList != NULL ) ;

	// For each entity, find it's symbol
	return EntityList_Enum( pList, NULL, EntityList_ReattachCB ) ;

}// EntityList_Enum

/* EOF: EntityList.c */