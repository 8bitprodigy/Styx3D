/****************************************************************************************/
/*  TERNLIST.C                                                                          */
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

#pragma warning(disable : 4206)

#if 0 //{

/* TerrainList.c  */

// TerrainList is a thin wrapper on List, a linked-list module.
// It provides casting and some simple validation

#include <Assert.h>

#include "Terrain.h"

#include "TernList.h"

static jeBoolean TerrainList_FindCB( void *p1, void *lParam )
{
	return ( p1 == lParam ) ;
}// TerrainList_FindCB

TerrainList * TerrainList_Create( void )
{
	return (TerrainList*)List_Create( ) ;
}// TerrainList_Create

void TerrainList_Destroy( TerrainList **ppList, TerrainList_DestroyCallback DestroyFcn )
{
	List_Destroy( (List**)ppList, DestroyFcn ) ;
}// TerrainList_Destroy

// ACCESSORS

// IS

// MODIFIERS
TerrainIterator TerrainList_Append( TerrainList * pList, Terrain * pTerrain )
{
	assert( pList != NULL ) ;

	return List_Append( pList, pTerrain ) ;
}// TerrainList_Append

void TerrainList_Remove( TerrainList * pTerrainList, Terrain * pTerrain )
{
	TerrainIterator	pBI ;
	jeBoolean		bFound ;
	Terrain	*		pFoundTerrain ;

	assert( pTerrainList != NULL ) ;

	bFound = List_Search( pTerrainList, TerrainList_FindCB, pTerrain, &pFoundTerrain, &pBI ) ;
	assert( JE_TRUE == bFound ) ;

	List_Remove( pTerrainList, pBI, NULL ) ;
}// LightList_Remove

void TerrainList_Delete( TerrainList * pTerrainList, Terrain * pTerrain )
{
	TerrainList_Remove( pTerrainList, pTerrain );
	Terrain_Destroy( &pTerrain );
}// TerrainList_Delete


// ENUMERATION

int32 TerrainList_Enum( TerrainList * pTerrainList, void * pVoid, TerrainListCB Callback )
{
	assert( pTerrainList != NULL ) ;

	return List_ForEach( pTerrainList, Callback, pVoid ) ;

}// TerrainList_Enum

/* EOF: TerrainList.c */

#endif //}