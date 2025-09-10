/****************************************************************************************/
/*  LIGHTLIST.H                                                                         */
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

#ifndef LIGHTLIST_H
#define LIGHTLIST_H

#include "Light.h"
#include "jeWorld.h"
#include "jeList.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef jeBoolean (*LightListCB)( Light *pLight, void * pVoid ) ;

typedef struct		LightList		LightList;

typedef ListIterator LightIterator ;
typedef List_DestroyCallback LightList_DestroyCallback ;

LightList *		LightList_Create( jeWorld *pWorld ) ;
void			LightList_Destroy( LightList **ppList ) ;

// ACCESSORS
Light *			LightList_GetFirst( LightList * pLightList, LightIterator	*pBI );
Light *			LightList_GetNext( LightList * pLightList, LightIterator	*pBI );
int32			LightList_GetNumItems( const LightList * pList ) ;
LightIterator	LightList_Find( LightList * pList, Light * pLight ) ;

// MODIFIERS
LightIterator		LightList_Append( LightList * pList, Light * pLight ) ;
//					Remove does not _Destroy the brush
void				LightList_Remove( LightList * pList, Light * pLight ) ;
void				LightList_DeleteLight( LightList * pList, Light * pLight ) ;

// ENUMERATION
int32				LightList_EnumLights( LightList * pList, void * pVoid, LightListCB Callback ) ;

// FILE HANDLING
jeBoolean			LightList_WriteToFile( LightList * pList, jeVFile * pF, jePtrMgr * pPtrMgr ) ;
LightList *			LightList_CreateFromFile( jeVFile * pF, jeWorld  * pWorld, jePtrMgr * pPtrMgr ) ;

#ifdef __cplusplus
}
#endif






#endif // Prevent multiple inclusion
/* EOF: LightList.h */