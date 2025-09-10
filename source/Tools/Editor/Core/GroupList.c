/****************************************************************************************/
/*  GROUPLIST.C                                                                         */
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

// GroupList is a thin wrapper on List, a linked-list module.
// It provides casting and some simple validation

#include <Assert.h>

#include "Group.h"
#include "Errorlog.h"

#include "GroupList.h"

#define GROUP_VERSION	(1)

GroupList * GroupList_Create( void )
{
	return (GroupList*)List_Create( ) ;
}// GroupList_Create

void GroupList_Destroy( GroupList **ppList, GroupList_DestroyCallback DestroyFcn )
{
	List_Destroy( (List**)ppList, DestroyFcn ) ;
}// GroupList_Destroy

// ACCESSORS

int32 GroupList_GetNumItems( const GroupList * pList )
{
	assert( pList != NULL ) ;

	return List_GetNumItems( pList ) ;
}// GroupList_GetNumItems

Group * GroupList_GetGroup( GroupIterator pGI )
{
	Group	*	pGroup ;
	assert( pGI != NULL ) ;

	pGroup = List_GetData( pGI ) ;
	assert( JE_TRUE == Group_IsValid( pGroup ) ) ;

	return pGroup ;
}// GroupList_GetGroup


Group * GroupList_GetFirst( GroupList * pList, GroupIterator * pGI )
{
	Group	* pGroup ;

	assert( pList != NULL ) ;
	assert( pGI != NULL ) ;

	pGroup = (Group*)List_GetFirst( pList, pGI ) ;
	assert( pGroup ) ;
	return  pGroup  ;
}// GroupList_GetFirstID

Group * GroupList_GetNext( GroupList * pList, GroupIterator * pGI )
{
	Group	* pGroup ;
	assert( pList != NULL ) ;
	assert( pGI != NULL ) ;

	pGroup = (Group*)List_GetNext( pList, pGI ) ;
	return  pGroup  ;
}// GroupList_GetNextID


// IS
jeBoolean GroupList_IsGroupVisible( GroupIterator pGI )
{
	assert( pGI != NULL ) ;

	return Group_IsVisible( (Group*)List_GetData( pGI ) ) ;
}// GroupList_IsVisible

// MODIFIERS
GroupIterator GroupList_Append( GroupList * pList, Group * pGroup )
{
	assert( pList != NULL ) ;
	assert( JE_TRUE == Group_IsValid( pGroup ) ) ;

	return List_Append( pList, pGroup ) ;
}// GroupList_Append

static jeBoolean GroupList_ReattachObjectCB( Group * pGroup, void * lParam )
{
	Object * pObject = ( Object*)lParam ;

	assert( pGroup );
	assert( lParam );

	if( Object_GetGroupTag( pObject ) == Group_GetIndexTag( pGroup ) )
	{
		Object_SetGroupTag( pObject, BRUSH_REATTACH_GOOD );
		Group_AddObject( pGroup, pObject );
		Object_SetGroup( pObject, pGroup );
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

jeBoolean	GroupList_ReattachObject( GroupList * pList, Object * pObject )
{
	return( !GroupList_EnumGroups( pList, pObject, GroupList_ReattachObjectCB ));
}

// ENUMERATION

int32 GroupList_EnumGroups( GroupList * pGroupList, void * pVoid, GroupListCB Callback )
{
	assert( pGroupList != NULL ) ;

	return List_ForEach( pGroupList, Callback, pVoid ) ;

}// GroupList_EnumLights

// FILE HANDLING

GroupList *			GroupList_CreateFromFile( jeVFile * pF  )
{
	GroupList	*	pGroupList = NULL ;
	int32			i ;
	int32			nItems ;
	int32			nVersion ;
	Group		*	pGroup;
	assert( jeVFile_IsValid( pF ) ) ;

	if( !jeVFile_Read( pF, &nVersion, sizeof nVersion ) )
		return NULL ;
	if( nVersion != GROUP_VERSION )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "GroupList_CreateFromFile Version.\n", NULL);
		return NULL ;
	}

	if( !jeVFile_Read( pF, &nItems, sizeof nItems ) )
		return NULL ;

	pGroupList = GroupList_Create( ) ;
	if( pGroupList == NULL )
	{
		jeErrorLog_AddString(JE_ERR_INTERNAL_RESOURCE, "GroupList_CreateFromFile\n", NULL);
		return NULL ;
	}

	for( i=0; i<nItems; i++ )
	{
		pGroup = Group_CreateFromFile( pF ) ;
		if( pGroup == NULL )
		{
			jeErrorLog_AddString(JE_ERR_INTERNAL_RESOURCE, "Grouplist_CreateFromFile\n", NULL);
			return NULL ;
		}
		if( GroupList_Append( pGroupList, pGroup ) == NULL )
		{	
			Group_Destroy( &pGroup ) ;
			return NULL ;
		}
	}
	return pGroupList ;


}// GroupList_CreateFromFile


static jeBoolean GroupList_WriteCB( Group *pGroup, void* lParam )
{
	return Group_WriteToFile( pGroup, (jeVFile*)lParam ) ;
}// GroupList_WriteCB

jeBoolean	GroupList_WriteToFile( GroupList * pList, jeVFile * pF )
{
	int32	nVersion ;
	int32	nItems ;
	assert( pList != NULL ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	nVersion = GROUP_VERSION ;
	if( jeVFile_Write( pF, &nVersion, sizeof nVersion ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "GroupList_WriteToFile:jeVFile_Write", NULL);
		return JE_FALSE;
	}
	
	nItems = GroupList_GetNumItems( pList ) ;
	if( jeVFile_Write( pF, &nItems, sizeof nItems ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "GroupList_WriteToFile:jeVFile_Writev", NULL);
		return JE_FALSE;
	}

	return GroupList_EnumGroups( pList, pF, GroupList_WriteCB ) ;

}// LightList_WriteToFile


/* EOF: GroupList.c */