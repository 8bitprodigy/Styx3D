/****************************************************************************************/
/*  TERNLIST.H                                                                          */
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
#pragma once

#ifndef TERRAINLIST_H
#define TERRAINLIST_H

#include "jeList.h"
#include "Symbol.h"
#include "TerrnObj.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef jeBoolean (*TerrainListCB)( Terrain * pTerrain, void * pVoid ) ;

typedef List TerrainList ;
typedef ListIterator TerrainIterator ;
typedef List_DestroyCallback TerrainList_DestroyCallback ;

TerrainList *	TerrainList_Create( void ) ;
void			TerrainList_Destroy( TerrainList **ppList, TerrainList_DestroyCallback DestroyFcn ) ;

// ACCESSORS

// IS

// MODIFIERS
TerrainIterator	TerrainList_Append( TerrainList * pList, Terrain * pTerrain ) ;
void			TerrainList_Delete( TerrainList * pTerrainList, Terrain * pTerrain );

// ENUMERATION
int32				TerrainList_Enum( TerrainList * pList, void * pVoid, TerrainListCB Callback ) ;

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: TerrainList.h */