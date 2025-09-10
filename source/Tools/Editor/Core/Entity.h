/****************************************************************************************/
/*  ENTITY.H                                                                            */
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

#ifndef ENTITY_H
#define ENTITY_H

#include "Symbol.h"
#include "Vec3d.h"
#include "XForm3d.h"
#include "Extbox.h"
#include "Defs.h"
#include "Group.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENTITY_MAXNAMELENGTH	(31)
#define ENTITY_MAXSTRINGLENGTH	(253)
#define ENTITY_MAXNUMBERLENGTH	(10)

#define ENTITY_VERSION			(1)

typedef struct tagEntity Entity ;

// CREATORS
Entity *			Entity_Create( jeSymbol_Table * pSymbols, Group * pGroup, const char * pszType, const char * pszName, const int32 nNumber );
Entity *			Entity_Copy( Entity *	pEntity, int32 nNumber );
void				Entity_Destroy( Entity ** ppEntity ) ;
Entity *			Entity_FromTemplate( const char * pszName, Group * pGroup, const Entity *	pEntity, int32 nNumber );
Entity *			Entity_CreateTemplate(  const char * const pszType, jeSymbol_Table * pSymbols );

// MODIFIERS
void				Entity_Move( Entity * pEntity, const jeVec3d * pWorldDistance ) ;
void				Entity_SetModified( Entity * pEntity ) ;
void				Entity_Snap( Entity * pEntity, jeFloat fSnapSize ) ;
void				Entity_SetXForm( Entity * pEntity, const jeXForm3d * XForm );
void				Entity_UpdateBounds( Entity * pEntity ) ;
void				Entity_Size( Entity * pEntity, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis ) ;
void				Enity_SetField( const Entity * pEntity , jeSymbol *FieldSymbol, void *pData, int32 DataSize );


// ACCESSORS
void				Entity_GetXForm( const Entity * pEntity, jeXForm3d * XForm );
const jeExtBox	*	Entity_GetWorldAxialBounds( const Entity * pEntity ) ;
void				Entity_GetWorldDrawBounds( const Entity * pEntity, jeExtBox *DrawBounds );
const char		*	Entity_GetType( const Entity * pEntity );
jeBoolean			Entity_GetField( const Entity * pEntity , jeSymbol *FieldSymbol, void *pData, int32 DataSize, jeBoolean *pDataInited );
jeBoolean			Enity_SelectClosest(  Entity * pEntity, FindInfo	*	pFindInfo );

// IS
jeBoolean			Entity_IsInRect( const Entity * pEntity, jeExtBox *pSelRect, jeBoolean bSelEncompeses );

// FILE
Entity *			Entity_CreateFromFile( jeVFile * pF, const int32 nVersion, jeSymbol_Table * pEntities ) ;
jeBoolean			Entity_WriteToFile( Entity * pEntity, jeVFile * pF );
jeBoolean			Entity_Reattach( Entity * pEntity ) ;

#ifdef __cplusplus
}
#endif


#endif // Prevent multiple inclusion
/* EOF: Entity.h */