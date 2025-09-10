/****************************************************************************************/
/*  LIGHT.C                                                                             */
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

#include "Light.h"
#include "ObjectDef.h"
#define SIGNATURE			'LITE'

#define LIGHT_BOX_MIN				-0.1f
#define LIGHT_BOX_MAX				0.1f
#define LIGHT_DRAW_MIN				-3.0f
#define LIGHT_DRAW_MAX				3.0f
#define LIGHT_FLAG_DIRTY			0x0001
#define LIGHT_FLAG_WBOUNDSDIRTY		0x0002
#define LIGHT_FLAG_DIRTYALL			LIGHT_FLAG_DIRTY | LIGHT_FLAG_WBOUNDSDIRTY
#define LIGHT_MAXNAMELENGTH	(31)

static int gLight_Update = OBJECT_UPDATE_REALTIME;


typedef enum
{
	LIGHT_GLOBAL_UPDATEGROUP_ID = PROPERTY_LOCAL_DATATYPE_START,
	LIGHT_GLOBAL_UPDATE_MANUEL_ID,
	LIGHT_GLOBAL_UPDATE_CHANGE_ID,
	LIGHT_GLOBAL_UPDATE_REALTIME_ID,
	LIGHT_GLOBAL_UPDATEGROUP_END_ID,
	LIGHT_GLOBAL_MAINTAIN_LIGHING_ID
};

typedef struct tagLight
{
	Object				ObjectData ;
	uint32				nIndexTag ;		// Used only during load
#ifdef _DEBUG
	int					nSignature ;
#endif
	int32				Flags;
	jeExtBox			WorldBounds ;
	LightInfo			LightData ;
	jeLight			*	pgeLight;
	jeWorld			*	pWorld; //Array that owns this light
	jeBoolean			bInWorld;
	jeBoolean			bDLight;
} Light ;

//STATIC FUNCTIONS

static jeBoolean Light_SetData( Light * pLight )
{
	if( pLight->pgeLight != NULL )
	{
		if( !jeLight_SetAttributes(	pLight->pgeLight,
									&pLight->LightData.Pos, 
									&pLight->LightData.Color, 
									pLight->LightData.Radius, 
									pLight->LightData.Brightness, 
									pLight->LightData.Flags) )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_SetData:jeLight_SetAttributes" );
			return( JE_FALSE );
		}
		Object_Dirty( &pLight->ObjectData );
	}
	return( JE_TRUE );
}

jeBoolean Light_UpdateData( Light * pLight )
{
	if( pLight->pgeLight != NULL )
	{
		if( !jeLight_SetAttributes(	pLight->pgeLight,
									&pLight->LightData.Pos, 
									&pLight->LightData.Color, 
									pLight->LightData.Radius, 
									pLight->LightData.Brightness, 
									pLight->LightData.Flags) )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_SetData:jeLight_SetAttributes" );
			return( JE_FALSE );
		}

	}
	return( JE_TRUE );
}

static jeBoolean Light_SizeEdge( Light * pLight, const jeVec3d * pStillEdge, const jeFloat fScale, ORTHO_AXIS Axis )
{
	float	fTemp;


	fTemp = jeVec3d_GetElement( &pLight->LightData.Pos, Axis ) - jeVec3d_GetElement( pStillEdge, Axis ) ;
	fTemp = fTemp * fScale ;
	fTemp = fTemp + jeVec3d_GetElement( pStillEdge, Axis ) ;
	jeVec3d_SetElement( &pLight->LightData.Pos, Axis, fTemp ) ;
	if( !Light_SetData( pLight ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_SizeEdge:Light_SetData" );
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

// CREATORS
Light *	Light_Create( const char * const pszName, Group * pGroup, int32 nNumber,jeWorld	* pWorld )
{
	Light	*	pLight;
	assert( pszName );
	assert( pWorld );

	pLight = JE_RAM_ALLOCATE_STRUCT( Light );
	if( pLight == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Light" );
		return( NULL );
	}
	memset( pLight, 0, sizeof( Light ) );
	assert( (pLight->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	if( !Object_Init( &pLight->ObjectData, pGroup, KIND_LIGHT, pszName, nNumber ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeRam_Free( pLight );
		return( NULL );
	}

	pLight->pgeLight = jeLight_Create();
	if( pLight->pgeLight == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_Create:jeLight_Create" );
		jeRam_Free( pLight );
		return( NULL );
	}
	pLight->pWorld = pWorld;
	pLight->bInWorld = JE_FALSE;
	pLight->bDLight = JE_FALSE;

	if( !jeLight_GetAttributes(	pLight->pgeLight, 
									&pLight->LightData.Pos, 
									&pLight->LightData.Color, 
									&pLight->LightData.Radius, 
									&pLight->LightData.Brightness, 
									&pLight->LightData.Flags ) 
	  )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_Create:jeLight_GetAttributes" );
		jeRam_Free( pLight );
		return( NULL );
	}
	jeExtBox_Set( &pLight->WorldBounds, LIGHT_BOX_MIN, LIGHT_BOX_MIN, LIGHT_BOX_MIN,
										LIGHT_BOX_MAX, LIGHT_BOX_MAX, LIGHT_BOX_MAX );
	return( pLight );
}// Light_Create


Light *	Light_Copy( Light *	pLight, int32 nNumber )
{
	Light *pNewLight;

	
	assert( pLight );
	assert( SIGNATURE == pLight->nSignature ) ;

	pNewLight = Light_Create( pLight->ObjectData.pszName, pLight->ObjectData.pGroup, nNumber,pLight->pWorld );
	if( pNewLight == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( NULL );
	}
	pNewLight->LightData = pLight->LightData;
	if( !Light_UpdateData( pNewLight ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_Copy::Light_SetData" );
		return( NULL );
	}

	return( pNewLight );
}

Light *	Light_FromTemplate( char * pszName, Group * pGroup, Light *	pLight, int32 nNumber, jeBoolean bUpdate )
{
	Light *pNewLight;

	
	assert( pszName );
	assert( pLight );
	assert( SIGNATURE == pLight->nSignature ) ;

	pNewLight = Light_Copy( pLight, nNumber );
	if( pNewLight == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( NULL );
	}
	if( pNewLight->ObjectData.pszName != NULL )
	{
		jeRam_Free( pNewLight->ObjectData.pszName );
	}
	 pNewLight->ObjectData.pszName = pszName;
	 pNewLight->ObjectData.pGroup = pGroup ;

	Light_UpdateBounds( pNewLight );
	if( !jeWorld_AddLight(pNewLight->pWorld, pNewLight->pgeLight, bUpdate ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_SetData:jeWorld_AddLight" );
		return NULL;
	}
	pNewLight->bInWorld = JE_TRUE;
	return( pNewLight );
}// Light_FromTemplate


char *	Light_CreateDefaultName(  )
{
	return( Util_LoadLocalRcString( IDS_LIGHT ) );
}

void Light_Destroy( Light ** ppLight ) 
{
	assert( ppLight );
	assert( *ppLight );
	assert( (*ppLight)->pWorld );


	if( (*ppLight)->pgeLight  != NULL )
	{
		if( (*ppLight)->bInWorld )
		{
			jeWorld_RemoveLight((*ppLight)->pWorld, (*ppLight)->pgeLight, JE_TRUE );
			(*ppLight)->bInWorld = JE_FALSE;
		}
		jeLight_Destroy( &(*ppLight)->pgeLight );
	}

	// [MLB-ICE] Comment: Same as in Brush/Level/... ;)
	if( (*ppLight)->ObjectData.pszName != NULL )
		jeRam_Free( (*ppLight)->ObjectData.pszName );
	// [MLB-ICE] EOB

	jeRam_Free( (*ppLight) );
}// Light_Destroy

Light *	Light_CreateTemplate(  jeWorld * pWorld )
{
	Light * pLight;
	assert( pWorld );

	pLight = JE_RAM_ALLOCATE_STRUCT( Light );
	if( pLight == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Light" );
		return( NULL );
	}
	memset( pLight, 0, sizeof( Light) );
	assert( (pLight->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	Object_Init( (Object*)pLight, NULL, KIND_LIGHT, "Template", 0 );
	pLight->pWorld = pWorld;
	pLight->LightData.Brightness = 3.0f;
	jeVec3d_Set( &pLight->LightData.Color, 255.0f, 255.0f, 255.0f );
	jeVec3d_Set( &pLight->LightData.Pos, 0.0f, 0.0f, 0.0f );
	pLight->LightData.Radius = 200.0f;
	Light_UpdateBounds( pLight );
	pLight->pgeLight = NULL;
	jeExtBox_Set( &pLight->WorldBounds, LIGHT_BOX_MIN, LIGHT_BOX_MIN, LIGHT_BOX_MIN,
										LIGHT_BOX_MAX, LIGHT_BOX_MAX, LIGHT_BOX_MAX );
	return( pLight );
}

// MODIFIERS
jeBoolean Light_Move( Light * pLight, const jeVec3d * pWorldDistance )
{
	assert( pLight != NULL ) ;
	assert( SIGNATURE == pLight->nSignature ) ;

	Light_SetModified( pLight );
	jeVec3d_Add( &pLight->LightData.Pos, pWorldDistance, &pLight->LightData.Pos );
	Light_SetData( pLight );
	return( JE_TRUE );

}// Light_Move

jeBoolean Light_Size( Light * pLight, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis )
{
	jeBoolean bResult = JE_TRUE;

	assert( pLight != NULL ) ;
	assert( SIGNATURE == pLight->nSignature ) ;

	// The Z axis is flipped (down is positive)
	// We just determine the edge and call _SizeEdge once or twice to keep this
	// as simple as possible
	switch( eSizeType )
	{
	case Select_Top :
		if( Ortho_Axis_Z == VAxis )
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Min, vScale, VAxis ) ;
		break ;

	case Select_Bottom :
		if( Ortho_Axis_Z == VAxis )
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		break ;

	case Select_Left :
		bResult = Light_SizeEdge( pLight, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;

	case Select_Right :
		bResult = Light_SizeEdge( pLight, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_TopLeft :
		if( Ortho_Axis_Z == VAxis )
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Min, vScale, VAxis ) ;
		bResult = Light_SizeEdge( pLight, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;		

	case Select_TopRight :
		if( Ortho_Axis_Z == VAxis )
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Min, vScale, VAxis ) ;
		bResult = Light_SizeEdge( pLight, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_BottomLeft :
		if( Ortho_Axis_Z == VAxis )
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		bResult = Light_SizeEdge( pLight, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;
	
	case Select_BottomRight :
		if( Ortho_Axis_Z == VAxis )
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			bResult = Light_SizeEdge( pLight, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		bResult = Light_SizeEdge( pLight, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;
	}
	Light_SetModified( pLight ) ;
	if( bResult == JE_FALSE )
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_Size:Light_SizeEdge" );

	Light_SetData( pLight );
	return( bResult );
}// Light_Size

jeBoolean Light_SetXForm( Light * pLight, const jeXForm3d * XForm )
{

	assert( pLight );
	assert( SIGNATURE == pLight->nSignature ) ;
	assert( XForm );

	pLight->LightData.Pos = XForm->Translation;
	Light_SetModified( pLight );
	return( Light_SetData( pLight ) );
}// Light_SetXForm


void Light_UpdateBounds( Light * pLight )
{

	assert( pLight );

	jeExtBox_SetTranslation ( &pLight->WorldBounds, &pLight->LightData.Pos );

}

void Light_SetModified( Light * pLight )
{
	assert( pLight != NULL ) ;
	assert( SIGNATURE == pLight->nSignature ) ;
	
	pLight->Flags |= LIGHT_FLAG_DIRTYALL ;	
}// Light_SetModified

jeBoolean Light_SetInfo( Light * pLight, LightInfo *pLightInfo, int32 BlankFieldFlag )
{
	assert( pLight );
	assert( pLightInfo );

	if( BlankFieldFlag  & LIGHT_FIELD_POS )
	{
		pLight->LightData.Pos = pLightInfo->Pos;
		Light_SetModified( pLight );
	}
	if( BlankFieldFlag  & LIGHT_FIELD_BRIGHTNESS )
		pLight->LightData.Brightness = pLightInfo->Brightness;
	if( BlankFieldFlag  & LIGHT_FIELD_RADIUS )
		pLight->LightData.Radius = pLightInfo->Radius;
	if( BlankFieldFlag  & LIGHT_FIELD_COLOR )
		pLight->LightData.Color = pLightInfo->Color;
	return( Light_SetData( pLight ) );
}//Light_SetInfo

void Light_SetIndexTag( Light * pLight, const uint32 nIndex ) 
{
	pLight->nIndexTag = nIndex;
	//jeLight_SetIndexTAG( pLight->pgeLight, nIndex );
}//Light_SetIndexTag

void Light_RemoveFromWorld( Light * pLight )
{
	jeWorld_RemoveLight( pLight->pWorld, pLight->pgeLight, JE_TRUE );
	pLight->bInWorld = JE_FALSE;
}

void Light_AddToWorld( Light * pLight )
{
	jeWorld_AddLight(pLight->pWorld, pLight->pgeLight, JE_TRUE);
	pLight->bInWorld = JE_TRUE;
}

// ACCESSORS
void Light_GetXForm( const Light * pLight, jeXForm3d * XForm )
{
	jeXForm3d_SetIdentity( XForm );
	XForm->Translation = pLight->LightData.Pos;
}

const jeExtBox * Light_GetWorldAxialBounds( const Light * pLight )
{
	assert( pLight != NULL ) ;
	assert( SIGNATURE == pLight->nSignature ) ;
	
	if( pLight->Flags & LIGHT_FLAG_WBOUNDSDIRTY )
	{	
		Light * pEvalLight = (Light*)pLight ;			// Lazy Evaluation requires removing the const
		Light_UpdateBounds( pEvalLight ) ;
	}

	return &pLight->WorldBounds ;

}// Light_GetWorldAxialBounds

void Light_GetWorldDrawBounds( const Light * pLight, jeExtBox *DrawBounds )
{
	jeVec3d Center;
	assert( pLight != NULL ) ;
	assert( SIGNATURE == pLight->nSignature ) ;
	
	if( pLight->Flags & LIGHT_FLAG_WBOUNDSDIRTY )
	{	
		Light * pEvalLight = (Light*)pLight ;			// Lazy Evaluation requires removing the const
		Light_UpdateBounds( pEvalLight ) ;
	}
	jeExtBox_GetTranslation ( &pLight->WorldBounds, &Center );
	jeExtBox_Set (  DrawBounds,
				  LIGHT_DRAW_MIN,	  LIGHT_DRAW_MIN,	  LIGHT_DRAW_MIN,
				  LIGHT_DRAW_MAX,	  LIGHT_DRAW_MAX,	  LIGHT_DRAW_MAX );
	jeExtBox_SetTranslation ( DrawBounds, &Center );

}// Light_GetWorldDrawBounds

void Light_GetInfo( const Light * pLight, LightInfo *pLightInfo, int32 *BlankFieldFlag )
{
	assert( pLight );
	assert( pLightInfo );
	assert( BlankFieldFlag );

	if( *BlankFieldFlag == LIGHT_INIT_ALL )
	{
		*(pLightInfo) = pLight->LightData;
		*BlankFieldFlag = 0;
		return;
	}
	if( !jeVec3d_Compare( &pLightInfo->Pos,&pLight->LightData.Pos, 0.0f) )
		*BlankFieldFlag |= LIGHT_FIELD_POS;

	if( pLightInfo->Brightness != pLight->LightData.Brightness )
		*BlankFieldFlag |= LIGHT_FIELD_BRIGHTNESS;

	if( pLightInfo->Radius != pLight->LightData.Radius )
		*BlankFieldFlag |= LIGHT_FIELD_RADIUS;

	if( !jeVec3d_Compare( &pLightInfo->Color,&pLight->LightData.Color, 0.0f) )
		*BlankFieldFlag |= LIGHT_FIELD_COLOR;
	return;
}//Light_GetInfo

jeBoolean Light_SelectClosest( Light * pLight, FindInfo	*	pFindInfo )
{
	Point				pt1;
	Point				pt2;
	jeFloat				DistSq ;
	jeVec3d			Vert1 ;
	jeVec3d			Vert2 ;
	jeExtBox		Bounds;
	int32			y ;

	assert( pLight != NULL );
	assert( pFindInfo != NULL );
	assert( pFindInfo->pOrtho  != NULL ) ;
			
	


	Light_GetWorldDrawBounds( pLight, &Bounds ) ;
	Vert1 = Bounds.Min ;
	Vert2 = Bounds.Max ;
	Ortho_WorldToView( pFindInfo->pOrtho, &Vert1, &pt1 ) ;
	Ortho_WorldToView( pFindInfo->pOrtho, &Vert2, &pt2 ) ;
	DistSq = Util_PointToLineDistanceSquared( &pt1, &pt2, pFindInfo->pViewPt ) ;
	if( DistSq < pFindInfo->fMinDistance )
	{
		pFindInfo->fMinDistance = DistSq ;
		pFindInfo->pObject = (Object*)pLight ;
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
		pFindInfo->pObject = (Object*)pLight ;
		pFindInfo->nFace = 0 ;
		pFindInfo->nFaceEdge = 0;
	}
	return( JE_TRUE );
}


jeBoolean Light_FillPositionDescriptor( Light * pLight, jeProperty_List * pArray )
{
	char * Name;
	jeProperty Property;

	Name = Util_LoadLocalRcString( IDS_POSITION_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillVec3dGroup( &Property, Name, &pLight->LightData.Pos,	OBJECT_POSITION_FIELD  );
	if( !jeProperty_Append( pArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONX_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat(  &Property, Name, pLight->LightData.Pos.X, OBJECT_POSITION_FIELDX, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONY_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat(  &Property, Name, pLight->LightData.Pos.Y,	OBJECT_POSITION_FIELDY, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONZ_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, pLight->LightData.Pos.Z, OBJECT_POSITION_FIELDZ, -FLT_MAX, FLT_MAX, 1.0f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}

	jeProperty_FillGroupEnd( &Property, OBJECT_POSITION_FIELD_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

jeBoolean Light_FillRGBDescriptor( Light * pLight, jeProperty_List * pArray )
{
	char * Name;
	jeProperty Property;

	Name = Util_LoadLocalRcString( IDS_COLOR_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillColorGroup( &Property, Name, &pLight->LightData.Color,	LIGHT_COLOR_FIELD  );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}

	Name = Util_LoadLocalRcString( IDS_RED_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, pLight->LightData.Color.X,	LIGHT_RED_FIELD, 0, 255.0f, 2.0f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}

	Name = Util_LoadLocalRcString( IDS_GREEN_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, pLight->LightData.Color.Y,	LIGHT_GREEN_FIELD, 0, 255.0f, 1.0f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}

	Name = Util_LoadLocalRcString( IDS_BLUE_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, pLight->LightData.Color.Z,	LIGHT_BLUE_FIELD, 0, 255.0, 1.0f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	Name = Util_LoadLocalRcString( IDS_PICKER_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillColorPicker( &Property, Name, &pLight->LightData.Color,	LIGHT_PICKER_FIELD );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}

	jeProperty_FillGroupEnd( &Property, LIGHT_COLOR_FIELD_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

jeProperty_List *	Light_BuildDescriptor( Light * pLight )
{
	jeProperty_List * pArray = NULL;
	char * Name;
	jeProperty Property;


	assert( pLight );

	pArray = jeProperty_ListCreateEmpty();
	if( pArray == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "DescriptorArray" );
		return NULL;
	}

	Name = Util_LoadLocalRcString( IDS_NAME_FIELD );
	if( Name == NULL )
		goto LBD_ERROR;
	jeProperty_FillString( &Property, Name, pLight->ObjectData.pszName, OBJECT_NAME_FIELD );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
		goto LBD_ERROR;

	Light_FillPositionDescriptor( pLight, pArray );

	Name = Util_LoadLocalRcString( IDS_BRIGHTNESS_FIELD );
	if( Name == NULL )
		goto LBD_ERROR;
	jeProperty_FillFloat( &Property, Name, pLight->LightData.Brightness,	LIGHT_BRIGHTNESS_FIELD, 0, FLT_MAX, 1.0f);
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
		goto LBD_ERROR;

	Name = Util_LoadLocalRcString( IDS_RADIUS_FIELD );
	if( Name == NULL )
		goto LBD_ERROR;
	jeProperty_FillFloat( &Property, Name, pLight->LightData.Radius, LIGHT_RADIUS_FIELD, 1, FLT_MAX, 1.0f );
	Light_FillRGBDescriptor( pLight, pArray );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
		goto LBD_ERROR;
	
	return( pArray );

LBD_ERROR:
	jeProperty_ListDestroy( &pArray );
	return( NULL );
}

void Light_SetProperty( Light * pLight, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bUpdate )
{
	DataType;
	switch( DataId )
	{
	case LIGHT_BRIGHTNESS_FIELD:
		pLight->LightData.Brightness = pData->Float;
		break;

	case LIGHT_RADIUS_FIELD:
		pLight->LightData.Radius = pData->Float;
		break;
	case LIGHT_RED_FIELD:
		pLight->LightData.Color.X = pData->Float;
		break;

	case LIGHT_GREEN_FIELD:
		pLight->LightData.Color.Y = pData->Float;
		break;

	case LIGHT_BLUE_FIELD:
		pLight->LightData.Color.Z = pData->Float;
		break;

	case LIGHT_PICKER_FIELD:
		pLight->LightData.Color = pData->Vector;
		break;
	}
	Light_SetData( pLight );
	if( bUpdate )
		Light_Update( pLight, OBJECT_UPDATE_CHANGE );
}

void Light_ChangeToDLight( Light * pLight )
{
	Light_RemoveFromWorld( pLight );
	jeWorld_AddDLight(pLight->pWorld, pLight->pgeLight );
	pLight->bDLight = JE_TRUE;
}

void Light_ChangeFromDLight( Light * pLight )
{
	jeWorld_RemoveDLight( pLight->pWorld, pLight->pgeLight);
	Light_AddToWorld( pLight );
	pLight->bDLight = JE_FALSE;
}

void Light_Update( Light * pLight, int Update_Type )
{
#pragma message( "Temporary Add and Remove from world to update lights." )

	if( Update_Type >= OBJECT_UPDATE_CHANGE )
		Object_Dirty( (Object*)pLight );

	if( Update_Type > gLight_Update )
		return;
	if( !(pLight->ObjectData.miscFlags & OBJECT_DIRTY  ) )
		return;

	if( Update_Type == OBJECT_UPDATE_REALTIME && pLight->bDLight == JE_FALSE )
	{
		Light_ChangeToDLight( pLight );
	}

	if( Update_Type == OBJECT_UPDATE_CHANGE && pLight->bDLight )
	{
		Light_ChangeFromDLight( pLight );
	}

	if( pLight->bInWorld )
	{
		jeWorld_RemoveLight(pLight->pWorld, pLight->pgeLight, JE_TRUE );
		pLight->bInWorld = JE_FALSE;
		if( !jeWorld_AddLight(pLight->pWorld, pLight->pgeLight, JE_TRUE ) )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_SetData:jeWorld_AddLight" );
			return;
		}
		pLight->bInWorld = JE_TRUE;
	}
	pLight->ObjectData.miscFlags &= ~OBJECT_DIRTY;
}

//IS
jeBoolean	Light_IsInRect( const Light * pLight, jeExtBox *pSelRect, jeBoolean bSelEncompeses )
{
	const jeExtBox *pWorldBounds;
	jeExtBox		Result;

	assert( pLight );
	assert( pSelRect );
	
	pWorldBounds = Light_GetWorldAxialBounds( pLight ) ;
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
}//Light_IsInRect


//FILE
Light * Light_CreateFromFile( jeVFile * pF, jeWorld * pWorld, jePtrMgr * pPtrMgr )
{
	Light	*	pLight = NULL ;

	assert( jeVFile_IsValid( pF ) ) ;

	pLight = JE_RAM_ALLOCATE_STRUCT( Light );
	if( pLight == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Light" );
		return( NULL );
	}
	memset( pLight, 0, sizeof( Light ) );
	assert( (pLight->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN

	if( !Object_InitFromFile( pF , &pLight->ObjectData ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Object_InitFromFile.\n", NULL);
		return NULL;
	}

	if( !jeVFile_Read(  pF, &pLight->nIndexTag, sizeof pLight->nIndexTag ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Light_ReadFromFile.\n", NULL);
		return NULL;
	}
	pLight->pgeLight = jeLight_CreateFromFile(pF, pPtrMgr);
	if( pLight->pgeLight == NULL )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "jeLight_CreateFromFile.\n", NULL);
		return NULL;
	}
	if( !jeLight_GetAttributes(	pLight->pgeLight, 
									&pLight->LightData.Pos, 
									&pLight->LightData.Color, 
									&pLight->LightData.Radius, 
									&pLight->LightData.Brightness, 
									&pLight->LightData.Flags ) 
	  )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Light_ReattachCB:jeLight_GetAttributes" );
		return NULL;
	}

	pLight->pWorld = pWorld;
	jeExtBox_Set( &pLight->WorldBounds, LIGHT_BOX_MIN, LIGHT_BOX_MIN, LIGHT_BOX_MIN,
											LIGHT_BOX_MAX, LIGHT_BOX_MAX, LIGHT_BOX_MAX );
	Light_UpdateBounds( pLight );

	pLight->Flags |= LIGHT_FLAG_DIRTYALL ;	
	Object_SetInLevel( (Object*)pLight, JE_TRUE );
	pLight->bInWorld = JE_TRUE;

	return( pLight );
}


jeBoolean Light_WriteToFile( Light * pLight, jeVFile * pF, jePtrMgr * pPtrMgr )
{
	assert( pLight != NULL ) ;
	assert( SIGNATURE == pLight->nSignature ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	if( !Object_WriteToFile( &pLight->ObjectData, pF ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pLight->nIndexTag, sizeof pLight->nIndexTag ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Light_WriteToFile.", NULL);
		return JE_FALSE;
	}
	if( jeLight_WriteToFile( pLight->pgeLight, pF, pPtrMgr) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "jeLight_WriteToFile.", NULL);
		return JE_FALSE;
	}
	return JE_TRUE ;

}// Light_WriteToFile

//DISPLAY
#define LIGHT_MAXPOINTSPERFACE (64)
void Light_RenderOrtho( const Ortho * pOrtho, Light *pLight, int32 hDC, jeBoolean bColorOveride )
{
	Point			points[LIGHT_MAXPOINTSPERFACE];
	jeVec3d			Vert1 ;
	jeVec3d			Vert2 ;
	jeExtBox  Bounds;
	int32			y ;

	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pLight->nSignature ) ;
	assert( pLight != NULL ) ;

	 Light_GetWorldDrawBounds( pLight, &Bounds ) ;
	
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
	bColorOveride;

}// Light_RenderOrtho


jeProperty_List *	Light_GlobalPropertyList()
{
	jeProperty_List * pList;
	jeProperty	Property;
	char *	Name;
	jeBoolean bCheck;

	pList  =  jeProperty_ListCreate(0);
	if( pList == NULL )
		return( NULL );
	
	Name = Util_LoadLocalRcString( IDS_UPDATE ) ;
	jeProperty_FillGroup( &Property, Name, LIGHT_GLOBAL_UPDATEGROUP_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Light_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}

	Name = Util_LoadLocalRcString( IDS_UPDATE_MANUEL ) ;
	bCheck = (gLight_Update == OBJECT_UPDATE_MANUEL );
	jeProperty_FillRadio( &Property, Name, bCheck, LIGHT_GLOBAL_UPDATE_MANUEL_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Light_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}
	
	Name = Util_LoadLocalRcString( IDS_UPDATE_CHANGE ) ;
	bCheck = (gLight_Update == OBJECT_UPDATE_CHANGE );
	jeProperty_FillRadio( &Property, Name, bCheck, LIGHT_GLOBAL_UPDATE_CHANGE_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Light_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}

	Name = Util_LoadLocalRcString( IDS_UPDATE_REALTIME ) ;
	bCheck = (gLight_Update == OBJECT_UPDATE_REALTIME );
	jeProperty_FillRadio( &Property, Name, bCheck, LIGHT_GLOBAL_UPDATE_REALTIME_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Light_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}

	jeProperty_FillGroupEnd( &Property, LIGHT_GLOBAL_UPDATEGROUP_END_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Light_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}

	
	return( pList );
}

void Light_SetGlobalProperty( int DataId, int DataType, jeProperty_Data * pData )
{
	switch( DataId )
	{
		case LIGHT_GLOBAL_UPDATE_MANUEL_ID:
			if( pData->Bool )
			{
				gLight_Update = OBJECT_UPDATE_MANUEL;
			}
			break;

		case LIGHT_GLOBAL_UPDATE_CHANGE_ID:
			if( pData->Bool )
			{
				gLight_Update = OBJECT_UPDATE_CHANGE;
			}
			break;

		case LIGHT_GLOBAL_UPDATE_REALTIME_ID:
			if( pData->Bool )
			{
				gLight_Update = OBJECT_UPDATE_REALTIME;
			}
			break;

	}
	DataType;
}
