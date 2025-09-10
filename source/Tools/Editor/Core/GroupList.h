/****************************************************************************************/
/*  GROUPLIST.H                                                                         */
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
#ifndef GROUPLIST_H
#define GROUPLIST_H

#include "Group.h"
#include "jeList.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef List GroupList ;
typedef ListIterator GroupIterator ;
typedef List_DestroyCallback GroupList_DestroyCallback ;
typedef jeBoolean (*GroupListCB)( Group *pGroup, void * pVoid ) ;

GroupList *		GroupList_Create( void ) ;
void			GroupList_Destroy( GroupList **ppList, GroupList_DestroyCallback DestroyFcn ) ;

// ACCESSORS
int32			GroupList_GetNumItems( const GroupList * pList );
Group *			GroupList_GetGroup( GroupIterator pGI ) ;
Group *			GroupList_GetFirst( GroupList * pList, GroupIterator * pGI ) ;
Group *			GroupList_GetNext( GroupList * pList, GroupIterator * pGI ) ;

// IS
jeBoolean		GroupList_IsGroupVisible( GroupIterator pGI ) ;

// MODIFIERS
GroupIterator	GroupList_Append( GroupList * pList, Group * pGroup ) ;
jeBoolean		GroupList_ReattachObject( GroupList * pList, Object * pObject );

// ENUMERATION
int32 GroupList_EnumGroups( GroupList * pGroupList, void * pVoid, GroupListCB Callback );

// FILE HANDLING

GroupList *			GroupList_CreateFromFile( jeVFile * pF  ) ;
jeBoolean			GroupList_WriteToFile( GroupList * pList, jeVFile * pF ) ;

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: GroupList.h */
