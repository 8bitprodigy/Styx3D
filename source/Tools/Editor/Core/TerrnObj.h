/****************************************************************************************/
/*  TERRNOBJ.H                                                                          */
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

#ifndef TERRAIN_H
#define TERRAIN_H

#include "Terrain.h"
#include "defs.h"
#include "Group.h"

typedef struct tagTerrain Terrain ;
#define TERRAIN_VERSION		(1)
#include "jeWorld.h"

#ifdef __cplusplus
extern "C" {
#endif


// CREATORS
Terrain *			Terrain_Create( jeWorld	* pWorld, Group * pGroup, const char * const pszName, int32 nNumber,  jeBitmap *HeightMap, jeBitmap * TerrainMap  ) ;
Terrain *			Terrain_Copy( Terrain *	pTerrain, int32 nNumber );
void				Terrain_Destroy( Terrain ** ppTerrain ) ;
Terrain *			Terrain_FromTemplate( char * pszName, Group * pGroup, Terrain *	pTerrain, int32 nNumber );
char  *				Terrain_CreateDefaultName( void );
Terrain *			Terrain_CreateTemplate( jeWorld	* pWorld );

// MODIFIERS
void				Terrain_Move( Terrain * pTerrain, const jeVec3d * pWorldDistance ) ;
void				Terrain_SetIndexTag( Terrain * pTerrain, const uint32 nIndex ) ;
void				Terrain_SetModified( Terrain * pTerrain ) ;
void				Terrain_Snap( Terrain * pTerrain, jeFloat fSnapSize ) ;
void				Terrain_SetXForm( Terrain * pTerrain, const jeXForm3d * XForm );
void				Terrain_UpdateBounds( Terrain * pTerrain ) ;
void				Terrain_Size( Terrain * pTerrain, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis ) ;
// ACCESSORS
void				Terrain_GetXForm( const Terrain * pTerrain, jeXForm3d * XForm );
const jeExtBox *	Terrain_GetWorldAxialBounds( const Terrain * pTerrain ) ;
jeTerrain *			Terrain_GetTerrain( const Terrain * pTerrain );
jeBoolean			Terrain_SelectClosest( Terrain * pTerrain, FindInfo	*	pFindInfo );

// IS
jeBoolean			Terrain_IsInRect( const Terrain * pTerrain, jeExtBox *pSelRect, jeBoolean bSelEncompeses );

// FILE
Terrain *			Terrain_CreateFromFile( jeVFile * pF );
jeBoolean			Terrain_WriteToFile( Terrain * pTerrain, jeVFile * pF );

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Terrain.h */
