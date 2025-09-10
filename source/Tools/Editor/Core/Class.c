/****************************************************************************************/
/*  CLASS.C                                                                             */
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
#include "Class.h"
#include "ObjectDef.h"
#include "ErrorLog.h"
#include "Ram.h"
#include "assert.h"
#include "brush.h"
#include "light.h"
#include "UserObj.h"

typedef struct tagClass{
	Object ObjectData;
	int ClassKind;
}Class;

Class *			Class_Create( const char * const pszName, int Kind)
{
	Class * pClass;

	assert( pszName );

	pClass = JE_RAM_ALLOCATE_STRUCT_CLEAR( Class );
	if( pClass == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Class_Create" );
		return( NULL );
	}
	if( !Object_Init( &pClass->ObjectData, NULL, KIND_CLASS, pszName, 0 ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Class_Create:Object_Init" );
		return( NULL );
	}
	pClass->ClassKind = Kind;
	return( pClass );
}

void Class_Destroy( Class ** ppClass ) 
{
	jeRam_Free( *ppClass );
}
int	Class_GetClassKind( Class * pClass )
{
	return( pClass->ClassKind );
}

jeProperty_List *	Class_BuildDescriptor( Class * pClass )
{
	assert( pClass );

	switch( pClass->ClassKind )
	{
		case KIND_BRUSH:
			return( Brush_GlobalPropertyList() );

		case KIND_LIGHT:
			return( Light_GlobalPropertyList() );

		case KIND_USEROBJ:
			return( UserObj_GlobalPropertyList( pClass->ObjectData.pszName ) );

	}
	return( NULL );
}

void Class_SetProperty( Class * pClass, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bUpdate )
{
	assert( pClass );
	switch( pClass->ClassKind )
	{
		case KIND_BRUSH:
			Brush_SetGlobalProperty( DataId, DataType, pData );
			break;

		case KIND_LIGHT:
			Light_SetGlobalProperty( DataId, DataType, pData );
			break;

	}

	bUpdate;
}
