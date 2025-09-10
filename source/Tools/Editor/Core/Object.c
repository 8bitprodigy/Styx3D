/****************************************************************************************/
/*  OBJECT.C                                                                            */
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

#include <Assert.h>
#include <string.h>	// strcpy strcat strlen 
#include <stdio.h>  // sprintf

#include "Brush.h"
#include "ObjectDef.h"
#include "Ram.h"
#include "Util.h"
#include "Light.h"
#include "ErrorLog.h"
#include "Model.h"
#include "CamObj.h"
#include "UserObj.h"
#include "Class.h"
#include "defs.h"

#define OBJECT_MAXNAMELENGTH	(31)

void Object_Free( Object ** ppObject )
{
	assert( ppObject != NULL ) ;
	assert( *ppObject != NULL ) ;

	assert( (*ppObject)->RefCnt > 0 );

	(*ppObject)->RefCnt--;
	if( (*ppObject)->RefCnt > 0 )
		return;

	if( (*ppObject)->pszName != NULL )
		jeRam_Free( (*ppObject)->pszName ) ;

	switch( (*ppObject)->ObjectKind )
	{
		case KIND_BRUSH :	Brush_Destroy( (Brush**)ppObject ) ;	break ;

		case KIND_LIGHT	:	Light_Destroy( (Light**)ppObject ) ;	break ;

		case KIND_CAMERA	:	Camera_Destroy( (Camera**)ppObject ) ;	break ;

		case KIND_USEROBJ	:	UserObj_Destroy( (UserObj**)ppObject ) ;	break ;


		case KIND_MODEL :	Model_Destroy( (Model**)ppObject ) ; break;

		case KIND_CLASS :	Class_Destroy( (Class**)ppObject ) ; break;

		default :
			assert( 0 ) ;
			break ;
	}
}// Object_Free

void	Object_AddRef( Object * pObject )
{
	pObject->RefCnt++;
}

char	*	Object_CreateDefaultName( OBJECT_KIND ObjectKind, int32 SubKind )
{
	switch( ObjectKind )
	{
		case KIND_BRUSH :
			return( Brush_CreateDefaultName( SubKind ) );	
			break ;

		case KIND_LIGHT :
			return( Light_CreateDefaultName( ) );

		case KIND_CAMERA :
			return( Camera_CreateDefaultName( ) );

		case KIND_USEROBJ :
			return( UserObj_CreateDefaultName( (jeObject*)SubKind ) );


		case KIND_CLASS:
		default :
			assert( 0 ) ;
			break ;
	}
	return( NULL );
}

char	*	Object_CreateKindName( Object * pObject )
{
	OBJECT_KIND ObjectKind;

	ObjectKind = Object_GetKind( pObject );

	switch( ObjectKind )
	{
		case KIND_BRUSH :
			return( Brush_CreateKindName( ) );	
			break ;

		case KIND_LIGHT :
			return( Light_CreateDefaultName( ) );

		case KIND_CAMERA :
			return( Camera_CreateDefaultName( ) );

		case KIND_USEROBJ :
			return( UserObj_CreateDefaultName( UserObj_GetjeObject( (UserObj*)pObject) ) );

		case KIND_MODEL:
			return( Model_CreateDefaultName( ) );

		case KIND_CLASS:
		default :
			assert( 0 ) ;
			break ;
	}
	return( NULL );
}
jeBoolean Object_Init( Object * pObject,  Group * pGroup, OBJECT_KIND ObjectKind, const char * const pszName, int32 nNumber )
{
	pObject->pszName = Util_StrDup( pszName ) ;
	if( pObject->pszName == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Object_Init:Util_StrDup" );		
		return( JE_FALSE ) ;
	}
	pObject->pGroup =pGroup ;
	pObject->nNumber = nNumber;
	pObject->RefCnt =  1;
	pObject->miscFlags = 0;
	pObject->ObjectKind = ObjectKind;
	pObject->GroupTag = BRUSH_REATTACH_GOOD;
	return( JE_TRUE );
}// Object_Init

Object * Object_Copy( Object * pObject, const int32 nNumber )
{
	assert( pObject != NULL ) ;

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			return (Object*)Brush_Copy( (Brush*)pObject, nNumber ) ;

		case KIND_LIGHT :
			return (Object*)Light_Copy( (Light*)pObject, nNumber ) ;

		case KIND_CAMERA:
			return (Object*)Camera_Copy( (Camera*)pObject, nNumber ) ;

		case KIND_USEROBJ:
			return (Object*)UserObj_Copy( (UserObj*)pObject, nNumber ) ;

		case KIND_MODEL:
			jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "UserObj_Copy:jeObject_Duplicate", "Object does not support clone." );
			return( NULL );
		
		case KIND_CLASS:
		default :
			assert( 0 ) ;
	}
	return NULL ;
}// Object_Copy



jeBoolean	Object_IsInRect( const Object * pObject, jeExtBox *pSelRect, jeBoolean bSelEncompeses )
{
	assert( pObject );
	assert( pSelRect );

	switch( pObject->ObjectKind )
	{
	case KIND_BRUSH :
		return Brush_IsInRect( (Brush*)pObject, pSelRect, bSelEncompeses ) ;

	case KIND_LIGHT :
		return Light_IsInRect( (Light*)pObject, pSelRect, bSelEncompeses ) ;

	case KIND_CAMERA :
		return Camera_IsInRect( (Camera*)pObject, pSelRect, bSelEncompeses ) ;

	case KIND_USEROBJ :
		return UserObj_IsInRect( (UserObj*)pObject, pSelRect, bSelEncompeses ) ;

	case KIND_MODEL :
		break;

	case KIND_CLASS:
	default :
		assert( 0 ) ;
		break ;
	}

	return JE_FALSE ;

}//Object_IsInRect

Group		*	Object_IsMemberOfLockedGroup( const Object * pObject )
{
	if( pObject->pGroup )
	{
		return( Group_FindLockedParent( pObject->pGroup ) );
	}
	return( NULL );
}

OBJECT_KIND Object_GetKind( const Object * pObject )
{
	assert( pObject != NULL );

	return( pObject->ObjectKind );
}// Object_GetKind

const char * Object_GetName( const Object * pObject )
{
	return( pObject->pszName );
}// Object_GetName

int32 Object_GetNameTag( const Object * pObject  )
{
	assert( pObject != NULL ) ;

	return pObject->nNumber ;
}// Object_GetNameTag

//Allocates name
char * Object_GetNameAndTag( const Object * pObject ) 
{
	char NumBuff[20];
	char *NameTag;

	assert( pObject );
	assert( pObject->pszName );

	sprintf( NumBuff, "%d", pObject->nNumber );
	NameTag = jeRam_Allocate( strlen( pObject->pszName ) + strlen( NumBuff ) + 1 );
	if( NameTag == NULL )
		return( NULL );
	strcpy( NameTag, pObject->pszName );
	if( pObject->nNumber )
		strcat( NameTag, NumBuff );
	return( NameTag );
}


jeBoolean Object_GetTransform( Object * pObject, jeXForm3d * pXForm )
{
	jeBoolean Result = JE_TRUE;

	assert( pObject != NULL ) ;

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			Brush_GetXForm( (Brush*)pObject, pXForm );
			break;

		case KIND_LIGHT :
			Light_GetXForm( (Light*)pObject, pXForm );
			break;

		case KIND_CAMERA :
			Camera_GetXForm( (Camera*)pObject, pXForm );
			break;

		case KIND_USEROBJ :
			Result = UserObj_GetXForm( (UserObj*)pObject, pXForm );
			break;

		case KIND_MODEL :
			Result = Model_GetXForm( (Model*)pObject, pXForm );
			break;

		
		case KIND_CLASS:
		default :
			assert( 0 ) ;
			return( JE_FALSE );
	}
	return Result ;
}// Object_GetTransform

jeBoolean Object_GetWorldAxialBounds( Object * pObject, jeExtBox *ObjectBounds)
{
	const jeExtBox *TempBounds;

	assert( pObject != NULL ) ;
	assert( ObjectBounds != NULL ) ;

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
				TempBounds = Brush_GetWorldAxialBounds( (Brush *)pObject );
				*ObjectBounds = *TempBounds;
			return( JE_TRUE ) ;

		case KIND_LIGHT :
				TempBounds = Light_GetWorldAxialBounds( (Light *)pObject );
				*ObjectBounds = *TempBounds;
			return( JE_TRUE ) ;

		case KIND_CAMERA :
				TempBounds = Camera_GetWorldAxialBounds( (Camera *)pObject );
				*ObjectBounds = *TempBounds;
			return( JE_TRUE ) ;

		case KIND_USEROBJ :
			return( UserObj_GetWorldAxialBounds( (UserObj *)pObject, ObjectBounds ) ) ;

		case KIND_MODEL:
			return( Model_GetWorldAxialBounds( (Model *)pObject, ObjectBounds ) ) ;

		case KIND_CLASS:
			break;

		default :
			assert( 0 ) ;
	}
	return JE_FALSE ;
}

jeBoolean Object_GetWorldDrawBounds( Object * pObject, jeExtBox *ObjectBounds)
{
	const jeExtBox *TempBounds;

	assert( pObject != NULL ) ;
	assert( ObjectBounds != NULL ) ;

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
				TempBounds = Brush_GetWorldAxialBounds( (Brush *)pObject );
				*ObjectBounds = *TempBounds;
			return( JE_TRUE ) ;

		case KIND_LIGHT :
				Light_GetWorldDrawBounds( (Light *)pObject, ObjectBounds);
			return( JE_TRUE ) ;

		case KIND_CAMERA :
				Camera_GetWorldDrawBounds( (Camera *)pObject, ObjectBounds);
			return( JE_TRUE ) ;

		case KIND_USEROBJ :
				UserObj_GetWorldDrawBounds( (UserObj *)pObject, ObjectBounds);
			return( JE_TRUE ) ;

		case KIND_MODEL:
			return( Model_GetWorldAxialBounds( (Model *)pObject, ObjectBounds ) ) ;

		case KIND_CLASS:
			break;

		default :
			assert( 0 ) ;
	}
	return JE_FALSE ;
}
uint32 Object_GetMiscFlags( const Object * pObject )
{
	assert( pObject != NULL ) ;

	return pObject->miscFlags ;
}// Object_GetMiscFlags

Group	*  Object_GetGroup( const Object * pObject )
{
	assert( pObject != NULL ) ;
	return( pObject->pGroup );
}

uint32	Object_GetGroupTag( const Object * pObject )
{
	assert( pObject != NULL ) ;
	return( pObject->GroupTag );
}

jeBoolean Object_IsInLevel( const Object * pObject )
{
	assert( pObject != NULL ) ;
	return( pObject->miscFlags &  OBJECT_INLEVEL );
}

jeBoolean Object_SelectClosest(  Object * pObject, FindInfo	*	pFindInfo )
{
	int32 Kind = Object_GetKind( pObject ) ;
	if( Kind & pFindInfo->eSelKind )
	{
		switch( Kind )
		{
			case KIND_BRUSH :
				Brush_SelectClosest( (Brush *) pObject, pFindInfo );
				break;

			case KIND_LIGHT :
				Light_SelectClosest( (Light *) pObject, pFindInfo );
				break;

			case KIND_CAMERA :
				Camera_SelectClosest( (Camera *) pObject, pFindInfo );
				break;

			case KIND_USEROBJ :
				UserObj_SelectClosest( (UserObj *) pObject, pFindInfo );
				break;

			case KIND_MODEL:
				break;

			case KIND_CLASS:
			default:
				assert(0 );
		}
	}//Selection kind matches mask
	return JE_TRUE ;

}

int32 Object_GetXFormModFlags( Object * pObject )
{
	int32 Kind = Object_GetKind( pObject ) ;
	int ModFlag = 0;
	if( Kind )
	{
		switch( Kind )
		{
			case KIND_BRUSH :
				ModFlag = JE_OBJECT_XFORM_ALL;
				break;

			case KIND_LIGHT :
				ModFlag = JE_OBJECT_XFORM_TRANSLATE;
				break;

			case KIND_CAMERA :
				ModFlag = JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_ROTATE;
				break;

			case KIND_USEROBJ :
				ModFlag = UserObj_GetXFormModFlag( (UserObj *) pObject );
				break;

			case KIND_MODEL:
				ModFlag = JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_ROTATE;
				break;

			case KIND_CLASS:
				ModFlag = 0;
				break;

			default:
				assert(0 );
		}
	}//Selection kind matches mask
	return ModFlag ;
}
jeObject *	Object_GetjeObject( Object * pObject )
{
	int32 Kind = Object_GetKind( pObject ) ;
	jeObject * pgeObject = NULL;

	if( Kind )
	{
		switch( Kind )
		{
			case KIND_BRUSH :
			case KIND_LIGHT :
			case KIND_CLASS:
				break;

			case KIND_CAMERA :
				pgeObject = Camera_GetjeObject( (Camera *)pObject );
				break;

			case KIND_MODEL:
				pgeObject = Model_GetjeObject( (Model *)pObject );
				break;

			case KIND_USEROBJ :
				pgeObject = UserObj_GetjeObject( (UserObj *)pObject );
				break;

			default:
				assert(0 );
		}
	}
	return pgeObject ;
}

void Object_ClearMiscFlags( Object * pObject, uint32 nMiscFlags )
{
	assert( pObject != NULL ) ;
	
	pObject->miscFlags &= ~nMiscFlags ;
}// Object_ClearMiscFlags

void Object_SetMiscFlags( Object * pObject, uint32 nMiscFlags )
{
	assert( pObject != NULL ) ;
	
	pObject->miscFlags |= nMiscFlags ;
}// Object_SetMiscFlags

jeBoolean Object_Move(  Object * pObject, const jeVec3d * pWorldDistance )
{
	assert( pObject != NULL ) ;
	assert( pWorldDistance != NULL );

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			Brush_Move( (Brush *)pObject,  pWorldDistance);
			return( JE_TRUE ) ;

		case KIND_LIGHT :
			Light_Move( (Light *)pObject,  pWorldDistance);
			return( JE_TRUE ) ;

		case KIND_CAMERA :
			Camera_Move( (Camera *)pObject,  pWorldDistance);
			return( JE_TRUE ) ;

		case KIND_USEROBJ :
			UserObj_Move( (UserObj *)pObject,  pWorldDistance);
			return( JE_TRUE ) ;

		case KIND_MODEL :
			Model_Move( (Model *)pObject,  pWorldDistance);
			return( JE_TRUE ) ;

		case KIND_CLASS:
		default :
			assert( 0 ) ;
	}
	Object_Update( pObject, OBJECT_UPDATE_REALTIME, JE_FALSE );
	return JE_FALSE ;
}

jeBoolean Object_Rotate( Object * pObject, ORTHO_AXIS RAxis, jeFloat RadianAngle, const jeVec3d * pRotationCenter )
{
	assert( pObject != NULL ) ;
	assert( pRotationCenter != NULL );

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			Brush_Rotate( (Brush *)pObject,  RAxis, RadianAngle, pRotationCenter  );
			return( JE_TRUE ) ;

		case KIND_CAMERA :
			Camera_Rotate( (Camera *)pObject,  RAxis, RadianAngle, pRotationCenter  );
			return( JE_TRUE ) ;

		case KIND_USEROBJ :
			UserObj_Rotate( (UserObj *)pObject,  RAxis, RadianAngle, pRotationCenter  );
			return( JE_TRUE ) ;

		case KIND_MODEL :
			Model_Rotate( (Model *)pObject,  RAxis, RadianAngle, pRotationCenter  );
			return( JE_TRUE );

		case KIND_LIGHT :
			return( JE_TRUE );

		case KIND_CLASS:
		default :
			assert( 0 ) ;
	}
	Object_Update( pObject, OBJECT_UPDATE_REALTIME, JE_FALSE );
	return JE_FALSE ;
}// Object_Rotate


jeBoolean	Object_Size( Object * pObject, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis )
{
	assert( pObject != NULL ) ;
	assert( pSelectedBounds != NULL );

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			Brush_Size( (Brush *)pObject,  pSelectedBounds, hScale, vScale, eSizeType, HAxis, VAxis );
			return( JE_TRUE ) ;

		case KIND_LIGHT :
			Light_Size( (Light *)pObject,  pSelectedBounds, hScale, vScale, eSizeType, HAxis, VAxis );
			return( JE_TRUE ) ;

		case KIND_CAMERA :
			Camera_Size( (Camera *)pObject,  pSelectedBounds, hScale, vScale, eSizeType, HAxis, VAxis );
			return( JE_TRUE ) ;

		case KIND_USEROBJ :
			UserObj_Size( (UserObj *)pObject,  pSelectedBounds, hScale, vScale, eSizeType, HAxis, VAxis );
			return( JE_TRUE ) ;

		case KIND_MODEL :
			return( JE_TRUE ) ;


		case KIND_CLASS:
		default :
			assert( 0 ) ;
	}
	Object_Update( pObject, OBJECT_UPDATE_REALTIME, JE_FALSE );
	return JE_FALSE ;
}

jeBoolean Object_SetTransform( Object * pObject, jeXForm3d * pXForm )
{
	assert( pObject != NULL ) ;

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			Brush_SetXForm( (Brush*)pObject, pXForm );
			return JE_TRUE ;

		case KIND_LIGHT :
			Light_SetXForm( (Light*)pObject, pXForm );
			return( JE_TRUE );

		case KIND_CAMERA :
			Camera_SetXForm( (Camera*)pObject, pXForm );
			return( JE_TRUE );

		case KIND_USEROBJ :
			UserObj_SetXForm( (UserObj*)pObject, pXForm );
			return( JE_TRUE );

		case KIND_MODEL :
			Model_SetXForm( (Model*)pObject, pXForm );
			return( JE_TRUE );

		case KIND_CLASS:
		default :
			assert( 0 ) ;
	}
	Object_Update( pObject, OBJECT_UPDATE_REALTIME, JE_FALSE );
	return JE_FALSE ;
}// Object_SetTransform

jeBoolean Object_SetName( Object * pObject, const char * Name, int32 nNumber )
{
	pObject->nNumber = nNumber;
	if( pObject->pszName )
		jeRam_Free(pObject->pszName );
	pObject->pszName = Util_StrDup( Name );
	if( pObject->pszName == NULL )
		return( JE_FALSE );
	return( JE_TRUE );
}//Object_SetName

void Object_Shear( Object * pObject, const jeVec3d * pWorldDistance, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, const jeExtBox * pSelectedBounds)
{
	assert( pObject != NULL ) ;

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			Brush_Shear( (Brush*)pObject, pWorldDistance, eSizeType, HAxis, VAxis, pSelectedBounds );
			break;

		case KIND_LIGHT :
		case KIND_CAMERA :
		case KIND_MODEL:
		case KIND_USEROBJ :
		case KIND_CLASS:
			break;

		default :
			assert( 0 ) ;
	}
	Object_Update( pObject, OBJECT_UPDATE_REALTIME, JE_FALSE );

}//Object_Shear

void	Object_SetGroupTag( Object * pObject, uint32 Tag )
{
	assert( pObject );

	pObject->GroupTag = Tag;
}

void Object_SetGroup( Object * pObject, Group * pGroup )
{
	assert( pObject );
	assert( pGroup );

	pObject->pGroup = pGroup;
}


//File

jeBoolean Object_WriteToFile( Object * pObject, jeVFile * pF )
{
	assert( jeVFile_IsValid( pF ) ) ;

	pObject->GroupTag = Group_GetIndexTag( pObject->pGroup );

	if( jeVFile_Write( pF, pObject->pszName, strlen( pObject->pszName )+1 ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	if( jeVFile_Write( pF, &pObject->ObjectKind, sizeof pObject->ObjectKind ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pObject->nNumber, sizeof pObject->nNumber ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pObject->GroupTag, sizeof pObject->GroupTag ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	return( JE_TRUE );
}

jeBoolean		Object_InitFromFile( jeVFile * pF , Object * pObject )
{

	char		szName[ OBJECT_MAXNAMELENGTH ] ;

	assert( jeVFile_IsValid( pF ) ) ;

	if( !Util_geVFile_ReadString( pF, szName, OBJECT_MAXNAMELENGTH ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Object_InitFromFile.\n", NULL);
		return JE_FALSE;
	}
	pObject->pszName = Util_StrDup(szName);
	pObject->RefCnt = 1;
	if( !jeVFile_Read( pF, &pObject->ObjectKind, sizeof pObject->ObjectKind ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Object_InitFromFile.\n", NULL);
		return JE_FALSE;
	}

	if( !jeVFile_Read( pF, &pObject->nNumber, sizeof pObject->nNumber ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Object_InitFromFile.\n", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pObject->GroupTag, sizeof pObject->GroupTag ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "Object_InitFromFile.\n", NULL);
		return JE_FALSE;
	}
	return( JE_TRUE );
}

jeProperty_List * Object_BuildDescriptor( Object * pObject )
{
	jeProperty_List *pArray = NULL;
	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			pArray = Brush_BuildDescriptor( (Brush*)pObject );
			break;

		case KIND_LIGHT :
			pArray = Light_BuildDescriptor( (Light*)pObject );
			break;

		case KIND_CAMERA :
			pArray = Camera_BuildDescriptor( (Camera*)pObject );
			break;

		case KIND_USEROBJ :
			pArray = UserObj_BuildDescriptor( (UserObj*)pObject );
			break;

		case KIND_MODEL :
			pArray = Model_BuildDescriptor( (Model*)pObject );
			break;

		case KIND_CLASS:
			pArray = Class_BuildDescriptor( (Class*)pObject );
			break;
		default :
			assert( 0 ) ;
	}
	return( pArray );
}

void Object_SetProperty( Object * pObject, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bLightUpdate, jeBoolean bBrushUpdate, jeBoolean bBrushLighting )
{
	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			Brush_SetProperty( (Brush*)pObject, DataId, DataType, pData, bBrushUpdate, bBrushLighting );
			break;

		case KIND_LIGHT :
			Light_SetProperty( (Light*)pObject, DataId, DataType, pData,  bLightUpdate );
			break;

		case KIND_USEROBJ :
			UserObj_SetProperty( (UserObj*)pObject, DataId, DataType, pData,  bLightUpdate );
			break;

		case KIND_CAMERA:
			Camera_SetProperty( (Camera*)pObject, DataId, DataType, pData,  bLightUpdate );
			break;

		case KIND_CLASS:
			Class_SetProperty( (Class*)pObject, DataId, DataType, pData,  bLightUpdate );
			break;

		default :
			assert( 0 ) ;
	}
}

void Object_Update( Object *pObject, int Update_Type, jeBoolean bOverideDirty )
{

	switch( pObject->ObjectKind )
	{
		case KIND_BRUSH :
			Brush_Update( (Brush*)pObject, Update_Type );
			break;

		case KIND_LIGHT :
			Light_Update( (Light*)pObject, Update_Type );
			break;

		case KIND_USEROBJ :
			UserObj_Update( (UserObj*)pObject, Update_Type );
			break;

		case KIND_CAMERA:
			break;

		case KIND_MODEL:
		case KIND_CLASS:
			break;

		default :
			assert( 0 ) ;
	}
	bOverideDirty;
}

void Object_Dirty( Object *pObject )
{
	pObject->miscFlags |= OBJECT_DIRTY;
}

void Object_SetInLevel( Object *pObject, jeBoolean bInLevel )
{
	pObject->miscFlags |= OBJECT_DIRTY;
	if( bInLevel )
		pObject->miscFlags |= OBJECT_INLEVEL;
	else
		pObject->miscFlags &= ~OBJECT_INLEVEL;
}

jeBoolean Object_SendMessage( Object *pObject, int32 message, void * data )
{
	//Later some editor objects might respond to generic messages.
	if( pObject->ObjectKind != KIND_USEROBJ )
		return( JE_FALSE );

	return( UserObj_SendMessage( (UserObj*)pObject, message, data ) );
}
