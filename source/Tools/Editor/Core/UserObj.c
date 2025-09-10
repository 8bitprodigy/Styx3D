/****************************************************************************************/
/*  USEROBJ.C                                                                           */
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
#include <Memory.h>
#include <String.h>
#include <float.h>

#include "ErrorLog.h"
#include "jet.h"
#include "Ram.h"
#include "Transform.h"
#include "Util.h"
#include "../resource.h"
#include "Point.h"
#include "Brush.h"
#include "EditMsg.h"

#pragma warning(disable : 4201 4214 4115)
#include <Windows.h>
#include <Windowsx.h>
#pragma warning(default : 4201 4214 4115; disable : 4514)

#include "UserObj.h"
#include "ObjectDef.h"
#define SIGNATURE			'UOBJ'

#define USEROBJ_BOX_MIN				-0.1f
#define USEROBJ_BOX_MAX				0.1f
#define USEROBJ_DRAW_MIN				-3.0f
#define USEROBJ_DRAW_MAX				3.0f
#define USEROBJ_FLAG_DIRTY			0x0001
#define USEROBJ_FLAG_WBOUNDSDIRTY		0x0002
#define USEROBJ_FLAG_DIRTYALL			USEROBJ_FLAG_DIRTY | USEROBJ_FLAG_WBOUNDSDIRTY
#define USEROBJ_MAXNAMELENGTH	(31)

typedef struct tagUserObj
{
	Object				ObjectData ;
#ifdef _DEBUG
	int					nSignature ;
#endif
	jeObject			*	pgeObject;
	Brush				*   pDrawBrush;
} UserObj ;

//STATIC FUNCTIONS

jeBoolean UserObj_AddToObject( UserObj * pUserObj, jeObject * pParent )
{
	assert( pUserObj );
	assert( pUserObj->pgeObject );
	assert( pParent );

	return( jeObject_AddChild( pParent, pUserObj->pgeObject) );
}

static jeBoolean UserObj_SizeEdge( UserObj * pUserObj, const jeVec3d * pStillEdge, const jeFloat fScale, ORTHO_AXIS Axis )
{
	float	fTemp;
	jeXForm3d	XForm;

	jeVec3d		Scale ;
	jeVec3d		Temp ;
	int			ModFlags;


	if( !UserObj_GetXForm( pUserObj, &XForm ) )
		return( JE_TRUE );

	ModFlags = jeObject_GetXFormModFlags( pUserObj->pgeObject );

	if( ModFlags & JE_OBJECT_XFORM_SCALE )
	{
		jeVec3d_Set( &Scale, 1.0f, 1.0f, 1.0f ) ;
		jeVec3d_SetElement( &Scale, Axis, fScale ) ;
		Temp = XForm.Translation ;	
		jeVec3d_Clear( &XForm.Translation ) ;
		jeXForm3d_Scale( &XForm, Scale.X, Scale.Y, Scale.Z ) ;
		XForm.Translation = Temp ;
	}

	
	if( ModFlags & JE_OBJECT_XFORM_TRANSLATE )
	{
		fTemp = jeVec3d_GetElement( &XForm.Translation, Axis ) - jeVec3d_GetElement( pStillEdge, Axis ) ;
		fTemp = fTemp * fScale ;
		fTemp = fTemp + jeVec3d_GetElement( pStillEdge, Axis ) ;
		jeVec3d_SetElement( &XForm.Translation, Axis, fTemp ) ;
	}
	UserObj_SetXForm( pUserObj, &XForm );
	return( JE_TRUE );
}

// CREATORS
UserObj *	UserObj_Create( const char * const pszName, Group * pGroup, int32 nNumber, jeObject	* pgeObject )
{
	UserObj	*	pUserObj;
	char * CombName;
	jeBrush * pgeBrush;

	assert( pszName );
	pUserObj = JE_RAM_ALLOCATE_STRUCT_CLEAR( UserObj );
	if( pUserObj == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate UserObj" );
		return( NULL );
	}
	assert( (pUserObj->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	if( !Object_Init( &pUserObj->ObjectData, pGroup, KIND_USEROBJ, pszName, nNumber ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeRam_Free( pUserObj );
		return( NULL );
	}

	pUserObj->pgeObject = pgeObject;
	if( pUserObj->pgeObject == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "UserObj_Create:jeUserObj_Create" );
		jeRam_Free( pUserObj );
		return( NULL );
	}


	if( jeObject_SendMessage( pUserObj->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
	{
		pUserObj->pDrawBrush = Brush_Create( pszName, NULL, 0 );
	}
	else
		pUserObj->pDrawBrush = NULL;
	CombName = Object_GetNameAndTag( &pUserObj->ObjectData );

	jeObject_SetName( pgeObject, CombName );
	jeRam_Free( CombName );

	return( pUserObj );
}// UserObj_Create


char  *	 UserObj_CreateKindName( )
{
	return( Util_LoadLocalRcString( IDS_USEROBJ ) );
}



UserObj *	UserObj_Copy( UserObj *	pUserObj, int32 nNumber )
{
	UserObj *pNewUserObj;
	jeObject *pgeObject;


	
	assert( pUserObj );
	assert( SIGNATURE == pUserObj->nSignature ) ;

	pgeObject = jeObject_Duplicate( pUserObj->pgeObject );
	if( pgeObject == NULL )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "UserObj_Copy:jeObject_Duplicate", "Object does not support clone." );
		return( NULL );
	}


	pNewUserObj = UserObj_Create( pUserObj->ObjectData.pszName, pUserObj->ObjectData.pGroup, nNumber, pgeObject );
	if( pNewUserObj == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( NULL );
	}

	return( pNewUserObj );
}



char *	UserObj_CreateDefaultName( jeObject	* pgeObject )
{
	return( Util_StrDup( jeObject_GetTypeName(pgeObject)));
}

void UserObj_Destroy( UserObj ** ppUserObj ) 
{
	assert( ppUserObj );
	assert( *ppUserObj );


	if( (*ppUserObj)->pgeObject  != NULL )
	{
		jeObject_Destroy( &(*ppUserObj)->pgeObject );
	}
	if( (*ppUserObj)->pDrawBrush )
	{
		Brush_SetGeBrush( (*ppUserObj)->pDrawBrush, KIND_BRUSH, NULL );
		Brush_Destroy( &(*ppUserObj)->pDrawBrush );
	}
	jeRam_Free( (*ppUserObj) );
}// UserObj_Destroy


// MODIFIERS
jeBoolean UserObj_Move( UserObj * pUserObj, const jeVec3d * pWorldDistance )
{
	jeXForm3d XF;
	assert( pUserObj != NULL ) ;
	assert( SIGNATURE == pUserObj->nSignature ) ;

	UserObj_SetModified( pUserObj );
	jeObject_GetXForm(pUserObj->pgeObject,&XF);
	jeVec3d_Add( &XF.Translation, pWorldDistance, &XF.Translation );
	jeObject_SetXForm(pUserObj->pgeObject,&XF);
	return( JE_TRUE );

}// UserObj_Move

jeBoolean UserObj_Size( UserObj * pUserObj, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis )
{
	jeBoolean bResult = JE_TRUE;

	assert( pUserObj != NULL ) ;
	assert( SIGNATURE == pUserObj->nSignature ) ;

	// The Z axis is flipped (down is positive)
	// We just determine the edge and call _SizeEdge once or twice to keep this
	// as simple as possible
	switch( eSizeType )
	{
	case Select_Top :
		if( Ortho_Axis_Z == VAxis )
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Max, vScale, VAxis ) ;
		else
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Min, vScale, VAxis ) ;
		break ;

	case Select_Bottom :
		if( Ortho_Axis_Z == VAxis )
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Max, vScale, VAxis ) ;
		break ;

	case Select_Left :
		bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Max, hScale, HAxis ) ;
		break ;

	case Select_Right :
		bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_TopLeft :
		if( Ortho_Axis_Z == VAxis )
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Max, vScale, VAxis ) ;
		else
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Min, vScale, VAxis ) ;
		bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Max, hScale, HAxis ) ;
		break ;		

	case Select_TopRight :
		if( Ortho_Axis_Z == VAxis )
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Max, vScale, VAxis ) ;
		else
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Min, vScale, VAxis ) ;
		bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_BottomLeft :
		if( Ortho_Axis_Z == VAxis )
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Max, vScale, VAxis ) ;
		bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Max, hScale, HAxis ) ;
		break ;
	
	case Select_BottomRight :
		if( Ortho_Axis_Z == VAxis )
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Max, vScale, VAxis ) ;
		bResult = UserObj_SizeEdge( pUserObj, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;
	}
	UserObj_SetModified( pUserObj ) ;
	if( bResult == JE_FALSE )
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "UserObj_Size:UserObj_SizeEdge" );

	return( bResult );
}// UserObj_Size

void UserObj_Rotate( UserObj * pUserObj, ORTHO_AXIS RAxis, jeFloat RadianAngle, const jeVec3d * pRotationCenter )
{
	jeXForm3d	XForm ;
	jeXForm3d	OrgXForm;
	int			ModFlags;
	assert( pUserObj != NULL ) ;
	assert( SIGNATURE == pUserObj->nSignature ) ;
	
	if( !UserObj_GetXForm( pUserObj, &OrgXForm ) )
		return;

	XForm = OrgXForm;
	ModFlags = jeObject_GetXFormModFlags( pUserObj->pgeObject );

	//If it cant be translated or rotated return.
	if( (ModFlags & ( JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_ROTATE)) == 0 )
		return;
	jeXForm3d_Translate( &XForm, -pRotationCenter->X, -pRotationCenter->Y, -pRotationCenter->Z ) ;
	switch( RAxis )
	{
	case Ortho_Axis_X :
		jeXForm3d_RotateX( &XForm, RadianAngle ) ;	break ;
	case Ortho_Axis_Y :
		jeXForm3d_RotateY( &XForm, RadianAngle ) ;	break ;
	case Ortho_Axis_Z :
		jeXForm3d_RotateZ( &XForm, RadianAngle ) ;	break ;
	}
	jeXForm3d_Translate( &XForm, pRotationCenter->X, pRotationCenter->Y, pRotationCenter->Z ) ; 

	// If cant be rotated then just translate it.
	if( !(ModFlags & JE_OBJECT_XFORM_ROTATE ) )
	{
		OrgXForm.Translation = XForm.Translation;
		XForm = OrgXForm;
	}
	UserObj_SetXForm( pUserObj, &XForm) ;
	UserObj_SetModified( pUserObj ) ;


}// UserObj_Rotate

jeBoolean UserObj_SendMessage( UserObj * pUserObj, int32 message, void * data )
{
	assert( pUserObj );
	assert( SIGNATURE == pUserObj->nSignature ) ;
	assert( pUserObj->pgeObject );

	return( jeObject_SendMessage(pUserObj->pgeObject, message, data ) );
}

int32 UserObj_GetXFormModFlag( UserObj * pUserObj )
{
	assert( pUserObj );
	assert( SIGNATURE == pUserObj->nSignature ) ;
	assert( pUserObj->pgeObject );

	return( jeObject_GetXFormModFlags(pUserObj->pgeObject ) );
}

void UserObj_Select3d( UserObj* pUserObj, jeVec3d * Front, jeVec3d * Back, jeVec3d * Impact )
{
	Select3dContextDef Context;

	assert( pUserObj );
	assert( Front );
	assert( Back );
	assert( pUserObj->pgeObject );


	Context.Front = *Front;
	Context.Back = *Back;
	Context.Impact = *Impact;

	jeObject_SendMessage( pUserObj->pgeObject, JETEDITOR_SELECT3D,	&Context );
}

#ifdef _USE_BITMAPS
void UserObj_ApplyMatr( UserObj* pUserObj, jeBitmap * pBitmap )
{
	assert( pUserObj );
	assert( pBitmap );
	assert( pUserObj->pgeObject );


	jeObject_SendMessage( pUserObj->pgeObject, JETEDITOR_APPLYMATERIAL,	pBitmap );
}
#else
void UserObj_ApplyMatr( UserObj* pUserObj, jeMaterialSpec * pMatSpec )
{
	assert( pUserObj );
	assert( pMatSpec );
	assert( pUserObj->pgeObject );

	jeObject_SendMessage( pUserObj->pgeObject, JETEDITOR_APPLYMATERIALSPEC,	pMatSpec );
}
#endif


jeBoolean UserObj_SetXForm( UserObj * pUserObj, const jeXForm3d * XForm )
{

	assert( pUserObj );
	assert( SIGNATURE == pUserObj->nSignature ) ;
	assert( pUserObj->pgeObject );
	assert( XForm );

	jeObject_SetXForm(pUserObj->pgeObject, XForm);
	UserObj_SetModified( pUserObj );
	return( JE_TRUE );
}// UserObj_SetXForm

 
jeBoolean UserObj_RemoveFromWorld( UserObj * pUserObj, jeWorld * pWorld)
{
	assert( pUserObj );
	assert( pUserObj->pgeObject );
	assert( pWorld );

	return( jeWorld_RemoveObject( pWorld, pUserObj->pgeObject) );
}

jeBoolean UserObj_AddToWorld( UserObj * pUserObj, jeWorld * pWorld )
{
	assert( pUserObj );
	assert( pUserObj->pgeObject );
	assert( pWorld );

	return( jeWorld_AddObject( pWorld, pUserObj->pgeObject) );
}

void UserObj_SetModified( UserObj * pUserObj )
{
	assert( pUserObj != NULL ) ;
	assert( SIGNATURE == pUserObj->nSignature ) ;
	
	Object_Dirty( &pUserObj->ObjectData );
}// UserObj_SetModified


// ACCESSORS
jeBoolean UserObj_GetXForm( const UserObj * pUserObj, jeXForm3d * XForm )
{
	return( jeObject_GetXForm(pUserObj->pgeObject,XForm) );

}

jeBoolean UserObj_GetWorldAxialBounds( const UserObj * pUserObj, jeExtBox * BBox)
{
	assert( pUserObj != NULL ) ;
	assert( SIGNATURE == pUserObj->nSignature ) ;
	
	return( jeObject_GetExtBox	( pUserObj->pgeObject, BBox) );

}// UserObj_GetWorldAxialBounds

jeBoolean UserObj_GetWorldDrawBounds( const UserObj * pUserObj, jeExtBox *DrawBounds )
{
	jeBrush *pgeBrush;
	assert( pUserObj != NULL ) ;
	assert( SIGNATURE == pUserObj->nSignature ) ;
	
	if( pUserObj->pDrawBrush )
	{
		if( !jeObject_SendMessage( pUserObj->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
			return( JE_FALSE );
		Brush_SetGeBrush( pUserObj->pDrawBrush, 0, pgeBrush );
		*DrawBounds = *Brush_GetWorldAxialBounds( pUserObj->pDrawBrush );
		return( JE_TRUE );
	}
	return( jeObject_GetExtBox	( pUserObj->pgeObject, DrawBounds) );

}// UserObj_GetWorldDrawBounds


jeBoolean UserObj_SelectClosest( UserObj * pUserObj, FindInfo	*	pFindInfo )
{
	Point				pt1;
	Point				pt2;
	jeFloat				DistSq ;
	jeVec3d			Vert1 ;
	jeVec3d			Vert2 ;
	jeExtBox		Bounds;
	int32			y ;
	jeBrush		*	pgeBrush;

	assert( pUserObj != NULL );
	assert( pFindInfo != NULL );
	assert( pFindInfo->pOrtho  != NULL ) ;
			
	
	if( pUserObj->pDrawBrush )
	{
		if( !jeObject_SendMessage( pUserObj->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
			return( JE_FALSE );
		Brush_SetGeBrush( pUserObj->pDrawBrush, 0, pgeBrush );
		Brush_SelectClosest( pUserObj->pDrawBrush, pFindInfo );
		if( pFindInfo->pObject == (Object*)pUserObj->pDrawBrush )
			pFindInfo->pObject = (Object*)pUserObj;
		return( JE_TRUE );
	}



	if( !UserObj_GetWorldDrawBounds( pUserObj, &Bounds )  )
		return( JE_TRUE );
	Vert1 = Bounds.Min ;
	Vert2 = Bounds.Max ;
	Ortho_WorldToView( pFindInfo->pOrtho, &Vert1, &pt1 ) ;
	Ortho_WorldToView( pFindInfo->pOrtho, &Vert2, &pt2 ) ;
	DistSq = Util_PointToLineDistanceSquared( &pt1, &pt2, pFindInfo->pViewPt ) ;
	if( DistSq < pFindInfo->fMinDistance )
	{
		pFindInfo->fMinDistance = DistSq ;
		pFindInfo->pObject = (Object*)pUserObj ;
		pFindInfo->nFace = 0 ;
		pFindInfo->nFaceEdge = 0;
	}

	y = pt1.Y ;
	pt1.Y = pt2.Y ;
	pt2.Y = y ;

	DistSq = Util_PointToLineDistanceSquared( &pt1, &pt2, pFindInfo->pViewPt ) ;
	if( DistSq < pFindInfo->fMinDistance )
	{
		pFindInfo->fMinDistance = DistSq ;
		pFindInfo->pObject = (Object*)pUserObj ;
		pFindInfo->nFace = 0 ;
		pFindInfo->nFaceEdge = 0;
	}
	return( JE_TRUE );
}

jeObject * UserObj_GetjeObject( UserObj * pUserObj )
{
	assert( pUserObj != NULL );
	assert( SIGNATURE == pUserObj->nSignature ) ;

	return( pUserObj->pgeObject );
}

jeBoolean UserObj_FillPositionDescriptor( UserObj * pUserObj, jeProperty_List * pArray )
{
	jeXForm3d XForm;
	char * Name;

	jeProperty Property;
	if( !UserObj_GetXForm( pUserObj, &XForm ) )
		return( JE_TRUE );

	Name = Util_LoadLocalRcString( IDS_POSITION_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillVec3dGroup( &Property, Name, &XForm.Translation,	OBJECT_POSITION_FIELD  );
	if( !jeProperty_Append( pArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONX_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat(  &Property, Name, XForm.Translation.X, OBJECT_POSITION_FIELDX, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONY_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat(  &Property, Name, XForm.Translation.Y,	OBJECT_POSITION_FIELDY, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONZ_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, XForm.Translation.Z, OBJECT_POSITION_FIELDZ, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	jeProperty_FillGroupEnd( &Property, OBJECT_POSITION_FIELD_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

jeProperty_List *	UserObj_BuildDescriptor( UserObj * pUserObj )
{
	jeProperty_List * pPropertyArray = NULL;
	jeProperty_List * pObjectArray;
	jeProperty_List * pArray = NULL;
	jeProperty		  Property;
	char		*	  Name;


	pObjectArray = jeProperty_ListCreateEmpty();

	Name = Util_LoadLocalRcString( IDS_NAME_FIELD );
	if( Name == NULL )
		goto UOBD_ERROR;
	jeProperty_FillString( &Property, Name, pUserObj->ObjectData.pszName, OBJECT_NAME_FIELD );
	jeRam_Free( Name );
	if( !jeProperty_Append( pObjectArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( NULL );
	}
	
	if( !UserObj_FillPositionDescriptor( pUserObj, pObjectArray ) )
		goto UOBD_ERROR;

	if( !jeObject_GetPropertyList(pUserObj->pgeObject, &pPropertyArray) )
		goto UOBD_ERROR;


	 pArray = jeProperty_ListConCat( pObjectArray, pPropertyArray );
	 if( pArray == NULL )
		 goto UOBD_ERROR;

	jeProperty_ListDestroy( &pObjectArray );
	jeProperty_ListDestroy( &pPropertyArray );

	 return( pArray );
UOBD_ERROR:
	 if( pObjectArray )
		 jeProperty_ListDestroy( &pObjectArray );

	 if( pPropertyArray )
		 jeProperty_ListDestroy( &pPropertyArray );

	 if( pArray )
		 jeProperty_ListDestroy( &pArray );
	 return( NULL );
}

jeProperty_List *	UserObj_GlobalPropertyList( const char * TypeName )
{
	jeProperty_List * pPropertyArray = NULL;

	assert( TypeName );

	if( ! jeObject_GetRegisteredPropertyList( TypeName, &pPropertyArray ) )
		return( NULL );

	return( pPropertyArray );
}

void UserObj_SetGlobalProperty( const char * TypeName, int DataId, int DataType, jeProperty_Data * pData )
{
	jeObject_SetRegisteredProperty( TypeName, DataId, DataType, pData );
}


void UserObj_SetProperty( UserObj * pUserObj, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bUpdate )
{
	char * CombName;

	assert( pUserObj );
	assert( pData );

	if( DataId == OBJECT_NAME_FIELD )
	{
		CombName = Object_GetNameAndTag( &pUserObj->ObjectData );

		jeObject_SetName( pUserObj->pgeObject, CombName );
		jeRam_Free( CombName );
		return;
	}

	jeObject_SetProperty( pUserObj->pgeObject, DataId, DataType, (jeProperty_Data*)pData );
	bUpdate;
}


void UserObj_Update( UserObj * pUserObj, int Update_Type )
{
	pUserObj;
	Update_Type;
}

//IS
jeBoolean	UserObj_IsInRect( const UserObj * pUserObj, jeExtBox *pSelRect, jeBoolean bSelEncompeses )
{
	jeExtBox WorldBounds;
	jeExtBox		Result;

	assert( pUserObj );
	assert( pSelRect );
	
	if( !UserObj_GetWorldDrawBounds( pUserObj, &WorldBounds)  )
		return( JE_FALSE );
	if( bSelEncompeses )
	{
		if( pSelRect->Max.X >= WorldBounds.Max.X &&
			pSelRect->Max.Y >= WorldBounds.Max.Y &&
			pSelRect->Max.Z >= WorldBounds.Max.Z &&
			pSelRect->Min.X <= WorldBounds.Min.X &&
			pSelRect->Min.Y <= WorldBounds.Min.Y &&
			pSelRect->Min.Z <= WorldBounds.Min.Z )
			 return( JE_TRUE );
	}
	else
	{
		return( Util_geExtBox_Intersection ( pSelRect, &WorldBounds, &Result	) );
	}
	return( JE_FALSE );
}//UserObj_IsInRect


//FILE
UserObj * UserObj_CreateFromFile( jeVFile * pF, jePtrMgr * pPtrMgr )
{
	UserObj	*	pUserObj = NULL ;
	jeBrush	*	pgeBrush;

	assert( jeVFile_IsValid( pF ) ) ;

	pUserObj = JE_RAM_ALLOCATE_STRUCT( UserObj );
	if( pUserObj == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate UserObj" );
		return( NULL );
	}
	memset( pUserObj, 0, sizeof( UserObj ) );
	assert( (pUserObj->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN

	if( !Object_InitFromFile( pF , &pUserObj->ObjectData ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Object_InitFromFile.", NULL);
		return NULL;
	}

	pUserObj->pgeObject = jeObject_CreateFromFile( pF, pPtrMgr );
	if( pUserObj->pgeObject == NULL )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "jeObject_CreateFromFile.", NULL);
		return NULL;
	}
	if( jeObject_SendMessage( pUserObj->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
	{
		pUserObj->pDrawBrush = Brush_Create( pUserObj->ObjectData.pszName, NULL, 0 );
	}
	else
		pUserObj->pDrawBrush = NULL;
	Object_SetInLevel( (Object*)pUserObj, JE_TRUE );
	return( pUserObj );
}




jeBoolean UserObj_WriteToFile( UserObj * pUserObj, jeVFile * pF, jePtrMgr * pPtrMgr )
{
	assert( pUserObj != NULL ) ;
	assert( SIGNATURE == pUserObj->nSignature ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	if( !Object_WriteToFile( &pUserObj->ObjectData, pF ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.", NULL);
		return JE_FALSE;
	}
	if( jeObject_WriteToFile( pUserObj->pgeObject, pF, pPtrMgr ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "jeUserObj_WriteToFile.", NULL);
		return JE_FALSE;
	}
	return JE_TRUE ;

}// UserObj_WriteToFile


//DISPLAY
#define USEROBJ_MAXPOINTSPERFACE (64)
void UserObj_RenderOrtho( const Ortho * pOrtho, UserObj *pUserObj, int32 hDC, jeBoolean bColorOveride )
{
	Point			points[USEROBJ_MAXPOINTSPERFACE];
	jeVec3d			Vert1 ;
	jeVec3d			Vert2 ;
	jeExtBox  Bounds;
	int32			y ;
	jeXForm3d		XF;
	HPEN				hOldPen = NULL ;
	HPEN				hPen ;
	jeBrush *			pgeBrush;

	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pUserObj->nSignature ) ;
	assert( pUserObj != NULL ) ;

	if( pUserObj->pDrawBrush )
	{
		if( !bColorOveride )
		{
			hPen = CreatePen( PS_SOLID, 1, RGB( 255, 0, 128 ) ) ;		// Selected objects
			hOldPen = SelectPen( (HDC)hDC, hPen ) ;
		}
		if( !jeObject_SendMessage( pUserObj->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
			return;
		Brush_SetGeBrush( pUserObj->pDrawBrush, 0, pgeBrush );
		Brush_RenderOrthoFaces(  pUserObj->pDrawBrush, pOrtho, hDC, JE_FALSE, JE_FALSE, JE_TRUE );
		if( !bColorOveride )
		{
			hPen = SelectPen( (HDC)hDC, hOldPen ) ;
			DeletePen( hPen ) ;
		}
		return;
	}

	if( !UserObj_GetWorldDrawBounds( pUserObj, &Bounds )  )
		return;

	if( !UserObj_GetXForm( pUserObj, &XF ) )
		return;
/*
	hPen = CreatePen( PS_SOLID, 1, RGB( 255, 255, 255 ) ) ;		// Selected objects
	hOldPen = SelectPen( (HDC)hDC, hPen ) ;
	Vert1 = XF.Translation ;
	jeXForm3d_GetIn( &XF, &Vert2 );
	jeVec3d_Scale( &Vert2, 8.0f, &Vert2 );
	jeVec3d_Add( &Vert2, &Vert1, &Vert2 );
	Ortho_WorldToView( pOrtho, &Vert1, &points[0] ) ;
	Ortho_WorldToView( pOrtho, &Vert2, &points[1] ) ;
	Pen_Polyline( hDC, points, 2 ) ;
	hPen = SelectPen( (HDC)hDC, hOldPen ) ;
	DeletePen( hPen ) ;
	//TextOut( (HDC)hDC, points[1].X, points[1].Y, "Z", 1 ) ;

	hPen = CreatePen( PS_SOLID, 1, RGB( 255, 0, 0 ) ) ;		// Selected objects
	hOldPen = SelectPen( (HDC)hDC, hPen ) ;
	Vert1 = XF.Translation ;
	jeXForm3d_GetUp( &XF, &Vert2 );
	jeVec3d_Scale( &Vert2, 8.0f, &Vert2 );
	jeVec3d_Add( &Vert2, &Vert1, &Vert2 );
	Ortho_WorldToView( pOrtho, &Vert1, &points[0] ) ;
	Ortho_WorldToView( pOrtho, &Vert2, &points[1] ) ;
	Pen_Polyline( hDC, points, 2 ) ;
	hPen = SelectPen( (HDC)hDC, hOldPen ) ;
	DeletePen( hPen ) ;
	//TextOut( (HDC)hDC, points[1].X, points[1].Y, "Y", 1 ) ;

	hPen = CreatePen( PS_SOLID, 1, RGB( 0, 255, 0 ) ) ;		// Selected objects
	hOldPen = SelectPen( (HDC)hDC, hPen ) ;
	Vert1 = XF.Translation ;
	jeXForm3d_GetLeft( &XF, &Vert2 );
	jeVec3d_Scale( &Vert2, 8.0f, &Vert2 );
	jeVec3d_Add( &Vert2, &Vert1, &Vert2 );
	Ortho_WorldToView( pOrtho, &Vert1, &points[0] ) ;
	Ortho_WorldToView( pOrtho, &Vert2, &points[1] ) ;
	Pen_Polyline( hDC, points, 2 ) ;
	hPen = SelectPen( (HDC)hDC, hOldPen ) ;
	DeletePen( hPen ) ;
	//TextOut( (HDC)hDC, points[1].X, points[1].Y, "X", 1 ) ;
*/
  
	if( !bColorOveride )
	{
		hPen = CreatePen( PS_SOLID, 1, RGB( 255, 0, 128 ) ) ;		// Selected objects
		hOldPen = SelectPen( (HDC)hDC, hPen ) ;
	}
	 Vert1 = Bounds.Min ;
	Vert2 = Bounds.Max ;
	Ortho_WorldToView( pOrtho, &Vert1, &points[0] ) ;
	Ortho_WorldToView( pOrtho, &Vert2, &points[1] ) ;

	// It would appear we just let GDI clip...
	Pen_Polyline( hDC, points, 2 ) ;
	y = points[0].Y ;
	points[0].Y = points[1].Y ;
	points[1].Y = y ;
	Pen_Polyline( hDC, points, 2 ) ;
	if( !bColorOveride )
	{
		hPen = SelectPen( (HDC)hDC, hOldPen ) ;
		DeletePen( hPen ) ;
	}


}// UserObj_RenderOrtho



