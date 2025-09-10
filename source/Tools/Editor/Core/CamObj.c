/****************************************************************************************/
/*  CAMOBJ.C                                                                            */
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
#include "Ram.h"
#include "Errorlog.h"
#include "Util.h"
#include "../resource.h"
#include "XForm3d.h"
#include "CamObj.h"
#include "ObjectDef.h"
#include "units.h"
#define SIGNATURE			'CAMR'
#include "Brush.h"
#include "EditMsg.h"
#include "CamFieldID.h"


typedef struct tagCamera
{
	Object				ObjectData ;
#ifdef _DEBUG
	int					nSignature ;
#endif
	float				XRotation;
	float				YRotation;
	int32				Flags;
	Brush				* pCamBrush;
	jeObject			*pgeObject;
	jeExtBox			WorldBounds;
} Camera ;

#define CAMERA_SIZE	 16
#define CAMERA_FLAG_DIRTY			0x0001
#define CAMERA_FLAG_WBOUNDSDIRTY		0x0002
#define CAMERA_FLAG_DIRTYALL			CAMERA_FLAG_DIRTY | CAMERA_FLAG_WBOUNDSDIRTY
#define CAMERA_MAXNAMELENGTH	(31)

Camera * Camera_Create( const char * const pszName, Group * pGroup, int32 nNumber) 
{
	Camera	*	pCamera;
	jeBrush	* pgeBrush;
	char * Name;

	assert( pszName );
	pCamera = JE_RAM_ALLOCATE_STRUCT( Camera );
	if( pCamera == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Camera" );
		return( NULL );
	}
	memset( pCamera, 0, sizeof( Camera ) );
	assert( (pCamera->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	if( !Object_Init( &pCamera->ObjectData, pGroup, KIND_CAMERA, pszName, nNumber ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Camera_Create:Object_Init" );
		jeRam_Free( pCamera );
		return( NULL );
	}
	pCamera->pgeObject = jeObject_Create( "Camera" );
	if( pCamera->pgeObject  == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Camera_Create:jeObject_Create" );
		jeRam_Free( pCamera );
		return( NULL );
	}
	if( jeObject_SendMessage( pCamera->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
		pCamera->pCamBrush = Brush_Create( pszName, NULL, 0 );

	Name = Object_GetNameAndTag( &pCamera->ObjectData );
	if( Name )
	{
		jeObject_SetName( pCamera->pgeObject, Name );
		jeRam_Free( Name );
	}
	pCamera->XRotation = 0.0f;
	pCamera->YRotation = 0.0f;

	pCamera->Flags |= CAMERA_FLAG_WBOUNDSDIRTY;

	return( pCamera );
}// Camera_Create

Camera *	Camera_Copy( Camera *	pCamera, int32 nNumber )
{
	Camera *pNewCamera;
	jeBrush	* pgeBrush;
	char * Name;

	pNewCamera = JE_RAM_ALLOCATE_STRUCT( Camera );
	if( pNewCamera == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Camera" );
		return( NULL );
	}
	memset( pNewCamera, 0, sizeof( Camera ) );
	assert( (pNewCamera->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	if( !Object_Init( &pNewCamera->ObjectData, pCamera->ObjectData.pGroup, KIND_CAMERA, pCamera->ObjectData.pszName, nNumber ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Camera_Copy:Object_Init" );
		jeRam_Free( pCamera );
		return( NULL );
	}
	pNewCamera->pgeObject = jeObject_Duplicate( pCamera->pgeObject );
	if( pNewCamera->pgeObject  == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Camera_Copy:jeObject_Create" );
		jeRam_Free( pCamera );
		return( NULL );
	}
	if( jeObject_SendMessage( pNewCamera->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
		pNewCamera->pCamBrush = Brush_Create( pCamera->ObjectData.pszName, NULL, 0 );
	Name = Object_GetNameAndTag( &pCamera->ObjectData );
	if( Name )
	{
		jeObject_SetName( pCamera->pgeObject, Name );
		jeRam_Free( Name );
	}
	pNewCamera->Flags |= CAMERA_FLAG_WBOUNDSDIRTY;
	pNewCamera->XRotation = pCamera->XRotation;
	pNewCamera->YRotation = pCamera->YRotation;
	return( pNewCamera );
}


char *	Camera_CreateDefaultName(  )
{
	return( Util_LoadLocalRcString( IDS_CAMERA ) );
}

void Camera_Destroy( Camera ** ppCamera ) 
{
	assert( ppCamera );
	assert( *ppCamera );
	if( (*ppCamera)->pgeObject )
		jeObject_Destroy( &(*ppCamera)->pgeObject );
	if( (*ppCamera)->pCamBrush )
	{
		Brush_SetGeBrush( (*ppCamera)->pCamBrush, KIND_BRUSH, NULL );
		Brush_Destroy( &(*ppCamera)->pCamBrush );
	}
	jeRam_Free( (*ppCamera) );
}// Camera_Destroy


// MODIFIERS
jeBoolean Camera_Move( Camera * pCamera, const jeVec3d * pWorldDistance )
{
	jeXForm3d XF;
	assert( pCamera != NULL ) ;
	assert( pCamera->pgeObject );
	assert( SIGNATURE == pCamera->nSignature ) ;

	jeObject_GetXForm( pCamera->pgeObject, &XF );

	Camera_SetModified( pCamera );
	jeVec3d_Add( &XF.Translation, pWorldDistance, &XF.Translation );
	jeObject_SetXForm( pCamera->pgeObject, &XF );

	return( JE_TRUE );

}// Camera_Move

void Camera_Rotate( Camera * pCamera, ORTHO_AXIS RAxis, jeFloat RadianAngle, const jeVec3d * pRotationCenter )
{
	jeXForm3d	XForm ;
	jeXForm3d	XRot_XForm ;
	jeXForm3d	CamXForm;
	assert( pCamera != NULL ) ;
	assert( SIGNATURE == pCamera->nSignature ) ;
	
	switch( RAxis )
	{
	case Ortho_Axis_X :
		pCamera->XRotation -=  RadianAngle;
		pCamera->XRotation = (float)fmod( pCamera->XRotation, (2*M_PI));
		break;

	case Ortho_Axis_Z :
		pCamera->XRotation +=  RadianAngle;
		pCamera->XRotation = (float)fmod( pCamera->XRotation, (2*M_PI));
		break;

	case Ortho_Axis_Y :
		pCamera->YRotation +=  RadianAngle;
		pCamera->YRotation = (float)fmod( pCamera->YRotation, (2*M_PI));
		break ;
	}
	jeXForm3d_SetYRotation( &XForm, pCamera->YRotation );
	jeXForm3d_SetXRotation( &XRot_XForm, pCamera->XRotation );
	jeXForm3d_Multiply( &XForm, &XRot_XForm, &XForm );
	Camera_GetXForm( pCamera, &CamXForm );
	jeXForm3d_Translate( &XForm, CamXForm.Translation.X, CamXForm.Translation.Y, CamXForm.Translation.Z ) ; 

	Camera_SetXForm( pCamera, &XForm) ;
	pRotationCenter;
}// Camera_Rotate

static jeBoolean Camera_SizeEdge( Camera * pCamera, const jeVec3d * pStillEdge, const jeFloat fScale, ORTHO_AXIS Axis )
{
	float	fTemp;
	jeXForm3d	CamXForm;

	Camera_GetXForm( pCamera, &CamXForm );
	fTemp = jeVec3d_GetElement( &CamXForm.Translation, Axis ) - jeVec3d_GetElement( pStillEdge, Axis ) ;
	fTemp = fTemp * fScale ;
	fTemp = fTemp + jeVec3d_GetElement( pStillEdge, Axis ) ;
	jeVec3d_SetElement( &CamXForm.Translation, Axis, fTemp ) ;
	Camera_SetXForm( pCamera, &CamXForm );
	return( JE_TRUE );
}

jeBoolean Camera_Size( Camera * pCamera, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis )
{
	jeBoolean bResult = JE_TRUE;

	assert( pCamera != NULL ) ;
	assert( SIGNATURE == pCamera->nSignature ) ;

	// The Z axis is flipped (down is positive)
	// We just determine the edge and call _SizeEdge once or twice to keep this
	// as simple as possible
	switch( eSizeType )
	{
	case Select_Top :
		if( Ortho_Axis_Z == VAxis )
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Min, vScale, VAxis ) ;
		break ;

	case Select_Bottom :
		if( Ortho_Axis_Z == VAxis )
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		break ;

	case Select_Left :
		bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;

	case Select_Right :
		bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_TopLeft :
		if( Ortho_Axis_Z == VAxis )
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Min, vScale, VAxis ) ;
		bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;		

	case Select_TopRight :
		if( Ortho_Axis_Z == VAxis )
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Min, vScale, VAxis ) ;
		bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_BottomLeft :
		if( Ortho_Axis_Z == VAxis )
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;
	
	case Select_BottomRight :
		if( Ortho_Axis_Z == VAxis )
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		bResult = Camera_SizeEdge( pCamera, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;
	}
	Camera_SetModified( pCamera ) ;
	if( bResult == JE_FALSE )
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Camera_Size:Camera_SizeEdge" );

	return( bResult );
}// Camera_Size

jeBoolean Camera_SetXForm( Camera * pCamera, const jeXForm3d * XForm )
{

	assert( pCamera );
	assert( SIGNATURE == pCamera->nSignature ) ;
	assert( XForm );

	jeObject_SetXForm( pCamera->pgeObject, XForm );
	Camera_SetModified( pCamera );
	return( JE_TRUE );
}// Camera_SetXForm


void Camera_UpdateBounds( Camera * pCamera )
{

	assert( pCamera );

	jeObject_GetExtBox( pCamera->pgeObject, &pCamera->WorldBounds );

}

void Camera_SetModified( Camera * pCamera )
{
	assert( pCamera != NULL ) ;
	assert( SIGNATURE == pCamera->nSignature ) ;
	
	pCamera->Flags |= CAMERA_FLAG_DIRTYALL ;	
}// Camera_SetModified




// ACCESSORS
void Camera_GetXForm( const Camera * pCamera, jeXForm3d * XForm )
{
	jeObject_GetXForm( pCamera->pgeObject, XForm );
}

const jeExtBox * Camera_GetWorldAxialBounds( const Camera * pCamera )
{
	assert( pCamera != NULL ) ;
	assert( SIGNATURE == pCamera->nSignature ) ;
	
	if( pCamera->Flags & CAMERA_FLAG_WBOUNDSDIRTY )
	{	
		Camera * pEvalCamera = (Camera*)pCamera ;			// Lazy Evaluation requires removing the const
		Camera_UpdateBounds( pEvalCamera ) ;
	}

	return &pCamera->WorldBounds ;

}// Camera_GetWorldAxialBounds

void Camera_GetWorldDrawBounds( const Camera * pCamera, jeExtBox *DrawBounds )
{
	assert( pCamera != NULL ) ;
	assert( SIGNATURE == pCamera->nSignature ) ;
	
	if( pCamera->Flags & CAMERA_FLAG_WBOUNDSDIRTY )
	{	
		Camera * pEvalCamera = (Camera*)pCamera ;			// Lazy Evaluation requires removing the const
		Camera_UpdateBounds( pEvalCamera ) ;
	}
	*DrawBounds = pCamera->WorldBounds;

}// Camera_GetWorldDrawBounds


jeBoolean Camera_SelectClosest( Camera * pCamera, FindInfo *	pFindInfo )
{
	jeBrush *pgeBrush;
	if( pCamera->pCamBrush )
	{
		if( !jeObject_SendMessage( pCamera->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
			return( JE_TRUE );
		Brush_SetGeBrush( pCamera->pCamBrush, 0, pgeBrush );
		Brush_SelectClosest( pCamera->pCamBrush, pFindInfo );
		if( pFindInfo->pObject == (Object*)pCamera->pCamBrush )
			pFindInfo->pObject = (Object*)pCamera;
	}
	return(  JE_TRUE );
}

jeBoolean Camera_FillPositionDescriptor( Camera * pCamera, jeProperty_List * pArray )
{
	jeXForm3d XForm;
	char * Name;

	jeProperty Property;
	Camera_GetXForm( pCamera, &XForm );

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

jeProperty_List *	Camera_BuildDescriptor( Camera * pCamera )
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
	jeProperty_FillString( &Property, Name, pCamera->ObjectData.pszName, OBJECT_POSITION_FIELD );
	jeRam_Free( Name );
	if( !jeProperty_Append( pObjectArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( NULL );
	}
	
	if( !Camera_FillPositionDescriptor( pCamera, pObjectArray ) )
		goto UOBD_ERROR;

	if( !jeObject_GetPropertyList(pCamera->pgeObject, &pPropertyArray) )
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


//IS
jeBoolean	Camera_IsInRect( const Camera * pCamera, jeExtBox *pSelRect, jeBoolean bSelEncompeses )
{
	const jeExtBox *pWorldBounds;
	jeExtBox		Result;

	assert( pCamera );
	assert( pSelRect );
	
	pWorldBounds = Camera_GetWorldAxialBounds( pCamera ) ;
	if( bSelEncompeses )
	{
		if( pSelRect->Max.X >= pWorldBounds->Max.X &&
			pSelRect->Max.Y >= pWorldBounds->Max.Y &&
			pSelRect->Max.Z >= pWorldBounds->Max.Z &&
			pSelRect->Min.X <= pWorldBounds->Min.X &&
			pSelRect->Min.Y <= pWorldBounds->Min.Y &&
			pSelRect->Min.Z <= pWorldBounds->Min.Z )
			 return( JE_TRUE );
	}
	else
	{
		return( Util_geExtBox_Intersection ( pSelRect, pWorldBounds, &Result	) );
	}
	return( JE_FALSE );
}//Camera_IsInRect

jeBoolean Camera_TranslateCurCam( Camera * pCamera, jeVec3d * Offset )
{
	jeXForm3d	XForm ;
	jeXForm3d	CamXForm;

	Camera_GetXForm( pCamera, &CamXForm );
	XForm = CamXForm;
	jeVec3d_Set( &XForm.Translation, 0.0f, 0.0f, 0.0f );

	jeXForm3d_Transform(&XForm, Offset, Offset );
	jeVec3d_Add( Offset, &CamXForm.Translation, &XForm.Translation );

	Camera_SetXForm( pCamera, &XForm) ;
	Camera_SetModified( pCamera ) ;
	return( JE_TRUE );
}

jeBoolean Camera_RotCurCamY( Camera * pCamera, float Radians )
{
	jeXForm3d	CamXForm;
	jeXForm3d	XForm ;
	jeXForm3d	XRot_XForm ;
	assert( pCamera != NULL ) ;
	assert( SIGNATURE == pCamera->nSignature ) ;
	
	Camera_GetXForm( pCamera, &CamXForm );
	pCamera->YRotation += Radians;
	jeXForm3d_SetYRotation( &XForm, pCamera->YRotation );
	jeXForm3d_SetXRotation( &XRot_XForm, pCamera->XRotation );
	jeXForm3d_Multiply( &XForm, &XRot_XForm, &XForm );
	jeXForm3d_Translate( &XForm, CamXForm.Translation.X, CamXForm.Translation.Y, CamXForm.Translation.Z ) ; 

	Camera_SetXForm( pCamera, &XForm) ;
	Camera_SetModified( pCamera ) ;
	return( JE_TRUE );
}

jeBoolean Camera_RotCurCamX( Camera * pCamera, float Radians )
{
	jeXForm3d	XForm ;
	jeXForm3d	XRot_XForm ;
	jeXForm3d	CamXForm;

	assert( pCamera != NULL ) ;
	assert( SIGNATURE == pCamera->nSignature ) ;
	
	Camera_GetXForm( pCamera, &CamXForm );
	pCamera->XRotation += Radians;
	jeXForm3d_SetYRotation( &XForm, pCamera->YRotation );
	jeXForm3d_SetXRotation( &XRot_XForm, pCamera->XRotation );
	jeXForm3d_Multiply( &XForm, &XRot_XForm, &XForm );
	jeXForm3d_Translate( &XForm, CamXForm.Translation.X, CamXForm.Translation.Y, CamXForm.Translation.Z ) ; 

	Camera_SetXForm( pCamera, &XForm) ;
	Camera_SetModified( pCamera ) ;
	return( JE_TRUE );
}


Camera * Camera_CreateFromFile( jeVFile * pF, jePtrMgr *PtrMgr )
{
	Camera	*	pCamera = NULL ;
	jeBrush * pgeBrush;

	assert( jeVFile_IsValid( pF ) ) ;

	pCamera = JE_RAM_ALLOCATE_STRUCT( Camera );
	if( pCamera == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Camera" );
		return( NULL );
	}
	memset( pCamera, 0, sizeof( Camera ) );
	assert( (pCamera->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN

	if( !Object_InitFromFile( pF , &pCamera->ObjectData ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Object_InitFromFile.\n", NULL);
		return NULL;
	}

	pCamera->pgeObject = jeObject_CreateFromFile( pF, PtrMgr );
	if( !pCamera->pgeObject )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Camera_ReadFromFile.\n", NULL);
		return NULL;
	}

	if( !jeVFile_Read(	pF, &pCamera->XRotation, sizeof( pCamera->XRotation) ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "CreateFromFile:XRotation" );
		return NULL;
	}
	if( !jeVFile_Read(	pF, &pCamera->YRotation, sizeof( pCamera->YRotation) ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "CreateFromFile:YRotation" );
		return NULL;
	}

	pCamera->Flags |= CAMERA_FLAG_DIRTYALL ;
	if( jeObject_SendMessage( pCamera->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
		pCamera->pCamBrush = Brush_Create( pCamera->ObjectData.pszName, NULL, 0 ) ;
	Camera_UpdateBounds( pCamera );
	Object_SetInLevel( (Object*)pCamera, JE_TRUE );
	return( pCamera );
}


jeBoolean Camera_WriteToFile( Camera * pCamera, jeVFile * pF, jePtrMgr *PtrMgr )
{
	assert( pCamera != NULL ) ;
	assert( SIGNATURE == pCamera->nSignature ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	if( !Object_WriteToFile( &pCamera->ObjectData, pF ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.", NULL);
		return JE_FALSE;
	}

	if( !jeObject_WriteToFile( pCamera->pgeObject, pF, PtrMgr ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.", NULL);
		return JE_FALSE;
	}

	if( !jeVFile_Write(	pF, &pCamera->XRotation, sizeof( pCamera->XRotation) ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Camera_WriteToFile:XRotation" );
		return( JE_FALSE );
	}
	if( !jeVFile_Write(	pF, &pCamera->YRotation, sizeof( pCamera->YRotation) ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Camera_WriteToFile:YRotation" );
		return( JE_FALSE );
	}

	return JE_TRUE ;

}// Camera_WriteToFile


//PRESENTATION
void Camera_RenderOrtho( const Ortho * pOrtho, Camera *pCamera, int32 hDC, jeBoolean bColorOveride)
{
/*	jeBrush * pgeBrush;
	jwePen  * pPen = NULL;

	if( ! bColorOveride )
	{
		pPen = Pen_SelectColor( hDC, 0, 255, 0);
	}
	if( !pCamera->pCamBrush )
		return;
	if( !jeObject_SendMessage( pCamera->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush ) )
		return;
	Brush_SetGeBrush( pCamera->pCamBrush, 0, pgeBrush );

	Brush_RenderOrthoFaces(  pCamera->pCamBrush, pOrtho, hDC, JE_FALSE, JE_FALSE, JE_TRUE );
	if( pPen )
		Pen_Release( pPen, hDC );
*/

	//	by TOM
	jeBrush * pgeBrush = NULL;
	jwePen  * pPen = NULL;

	if( ! bColorOveride )
	{
		pPen = Pen_SelectColor( hDC, 0, 255, 0);
	}
	if( !pCamera->pCamBrush )
		return;
	jeObject_SendMessage( pCamera->pgeObject, JETEDITOR_GET_JEBRUSH, &pgeBrush );

	if (!pgeBrush)
	{
		return;
	}
	Brush_SetGeBrush( pCamera->pCamBrush, 0, pgeBrush );

	Brush_RenderOrthoFaces(  pCamera->pCamBrush, pOrtho, hDC, JE_FALSE, JE_FALSE, JE_TRUE );
	if( pPen )
		Pen_Release( pPen, hDC );
}

jeObject *	Camera_GetjeObject( Camera * pCamera )
{
	assert( pCamera );

	return( pCamera->pgeObject );
}

float	Camera_GetFOV( Camera * pCamera )
{
	float FOV = 1.0f;

	assert( pCamera );
	assert( pCamera->pgeObject );

	jeObject_GetProperty( pCamera->pgeObject, CAMREA_FOV_ID, PROPERTY_FLOAT_TYPE, (jeProperty_Data*)&FOV );
	return( FOV );
}

float Camera_GetCurCamY( const Camera * pCamera )
{
	assert( pCamera );

	return( pCamera->YRotation );
}

float Camera_GetCurCamX( const Camera * pCamera )
{
	assert( pCamera );

	return( pCamera->XRotation );
}

void Camera_SetCurCamY( Camera * pCamera, float YRot )
{
	assert( pCamera );

	pCamera->YRotation = YRot;
}

void Camera_SetCurCamX( Camera * pCamera, float XRot )
{
	assert( pCamera );

	pCamera->XRotation  = XRot;
}

void	Camera_SetProperty( Camera * pCamera, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bUpdate )
{
	assert( pCamera );
	assert( pCamera->pgeObject );

	bUpdate;
	jeObject_SetProperty( pCamera->pgeObject, DataId, DataType,pData );
}

