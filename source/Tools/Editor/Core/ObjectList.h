/****************************************************************************************/
/*  OBJECTLIST.H                                                                        */
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

#ifndef OBJECTLIST_H
#define OBJECTLIST_H

#include "jeList.h"
#include "jwObject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef jeBoolean (*ObjectListCB)( Object *pObject, void * pVoid );

// Krouer: allow to add to the list with a sort
// return JE_TRUE if pObj1 is before pObj2
// return JE_FALSE if pObj1 is after pObj2
typedef jeBoolean (*ObjectList_SortCB)(Object *pObj1, Object *pObj2);

typedef List					ObjectList ;
typedef ListIterator			ObjectIterator ;
typedef List_DestroyCallback	ObjectList_DestroyCallback ;

ObjectList *	ObjectList_Create( void ) ;
void			ObjectList_Destroy( ObjectList **ppList, ObjectList_DestroyCallback DestroyFcn ) ;

// ACCESSORS
Object *		ObjectList_GetFirst( ObjectList * pList, ObjectIterator * Iterator ) ;
Object *		ObjectList_GetNext( ObjectList * pList, ObjectIterator * Iterator ) ;
Object *		ObjectList_GetLast( ObjectList * pList );
int32			ObjectList_GetNumItems( ObjectList * pList ) ;
ObjectIterator	ObjectList_Find( ObjectList * pList, Object * pObject ) ;
void ObjectList_GetListBounds( ObjectList * pList, jeExtBox * pListBounds );
void ObjectList_GetListDrawBounds( ObjectList * pList, jeExtBox * pListBounds );

// MODIFIERS
ObjectIterator	ObjectList_Append( ObjectList * pList, Object * pObject ) ;
jeBoolean		ObjectList_AppendNoDup( ObjectList * pList, Object * pObject ) ;
void			ObjectList_Remove( ObjectList * pList, Object * pObject ) ;

// Krouer: allow to add sorted
ObjectIterator	ObjectList_AppendSort( ObjectList * pList, Object * pObject, ObjectList_SortCB Callback ) ;

// ENUMERATION
int32			ObjectList_EnumObjects( ObjectList * pList, void * pVoid, ObjectListCB Callback ) ;

// CALLBACK
void			ObjectList_DestroyCB( Object * pObject ) ;

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion

/* EOF: ObjectList.h */