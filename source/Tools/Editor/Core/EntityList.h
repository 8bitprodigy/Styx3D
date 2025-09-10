/****************************************************************************************/
/*  ENTITYLIST.H                                                                        */
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

#ifndef ENTITYLIST_H
#define ENTITYLIST_H

#include "jeList.h"
#include "Symbol.h"
#include "Entity.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef jeBoolean (*EntityListCB)( Entity * pEntity, void * pVoid ) ;

typedef List EntityList ;
typedef ListIterator EntityIterator ;
typedef List_DestroyCallback EntityList_DestroyCallback ;

EntityList *	EntityList_Create( void ) ;
void			EntityList_Destroy( EntityList **ppList, EntityList_DestroyCallback DestroyFcn ) ;

// ACCESSORS
int32			EntityList_GetNumItems( const EntityList * pList ) ;
// IS

// MODIFIERS
EntityIterator	EntityList_Append( EntityList * pList, Entity * pEntity ) ;
void			EntityList_Delete( EntityList * pEntityList, Entity * pEntity );

// ENUMERATION
int32			EntityList_Enum( EntityList * pList, void * pVoid, EntityListCB Callback ) ;

// FILE HANDLING

EntityList *	EntityList_CreateFromFile( jeVFile * pF, jeSymbol_Table * pEntities ) ;
jeBoolean		EntityList_WriteToFile( EntityList * pList, jeVFile * pF ) ;
jeBoolean		EntityList_Reattach( EntityList * pList ) ;

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: EntityList.h */