/****************************************************************************************/
/*  ENTITY.C                                                                            */
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
#include <String.h>

#include "ErrorLog.h"
#include "ObjectDef.h"	// Should be a private header file!
#include "Ram.h"
#include "Util.h"
#include "jet.h"

#include "Entity.h"
#include "EntityTable.h"

#define ENTITY_BOX_MIN				-0.1f
#define ENTITY_BOX_MAX				0.1f
#define ENTITY_DRAW_MIN				-3.0f
#define ENTITY_DRAW_MAX				3.0f
#define SIGNATURE					'ENTY'
#define ENTITY_FLAG_DIRTY			0x0001
#define ENTITY_FLAG_WBOUNDSDIRTY	0x0002
#define ENTITY_FLAG_DIRTYALL		(ENTITY_FLAG_DIRTY | ENTITY_FLAG_WBOUNDSDIRTY)

typedef struct tagEntity
{
	Object			ObjectData ;
#ifdef _DEBUG
	int				nSignature ;
#endif
	jeSymbol_Table * pSymbolTable ;
	int32			Flags;
	jeExtBox		WorldBounds ;
	jeVec3d			Origin	;
	jeSymbol	*	pSymbol ;
	char		*	pszType;
} Entity ;

//STATIC FUNCTIONS


static void Entity_GetOrigin( Entity * pEntity )
{
	jeSymbol *FieldSymbol;

	if( pEntity->pSymbol == NULL )
		return;
	FieldSymbol = EntityTable_GetField( pEntity->pSymbolTable, pEntity->pSymbol, "Origin" ) ;

	if( FieldSymbol  == NULL )
		return;

	jeSymbol_GetProperty( pEntity->pSymbol, FieldSymbol, &pEntity->Origin, 
		sizeof( pEntity->Origin ), JE_SYMBOL_TYPE_VEC3D );
}

static void Entity_SetOrigin( Entity * pEntity )
{
	jeSymbol *FieldSymbol;

	if( pEntity->pSymbol == NULL )
		return;
	FieldSymbol = EntityTable_GetField( pEntity->pSymbolTable, pEntity->pSymbol, "Origin" ) ;

	if( FieldSymbol  == NULL )
		return;

	jeSymbol_SetProperty( pEntity->pSymbol, FieldSymbol, &pEntity->Origin, 
		sizeof( pEntity->Origin ), JE_SYMBOL_TYPE_VEC3D );
}

static void Entity_SizeEdge( Entity * pEntity, const jeVec3d * pStillEdge, const jeFloat fScale, ORTHO_AXIS Axis )
{
	float	fTemp;


	fTemp = jeVec3d_GetElement( &pEntity->Origin, Axis ) - jeVec3d_GetElement( pStillEdge, Axis ) ;
	fTemp = fTemp * fScale ;
	fTemp = fTemp + jeVec3d_GetElement( pStillEdge, Axis ) ;
	jeVec3d_SetElement( &pEntity->Origin, Axis, fTemp ) ;
	Entity_SetOrigin( pEntity );
}

static char * Entity_AllocateNameWithNumber( const Entity * pEntity )
{
	char	*	pszNameAndNumber ;

	pszNameAndNumber = jeRam_Allocate( strlen( pEntity->ObjectData.pszName ) + ENTITY_MAXNUMBERLENGTH ) ;
	if( pszNameAndNumber != NULL )
	{
		sprintf( pszNameAndNumber, "%s %d", pEntity->ObjectData.pszName, pEntity->ObjectData.nNumber ) ;
	}

	return pszNameAndNumber ;
}// Entity_AllocateNameWithNumber


Entity * Entity_Create( jeSymbol_Table * pSymbols, Group * pGroup, const char * pszType, const char * pszName, const int32 nNumber )
{
	Entity	*	pEntity ;
	char	*	pszNameAndNumber ;
	assert( pszName != NULL ) ;
	assert( strlen( pszName ) < ENTITY_MAXNAMELENGTH ) ;

	pEntity = JE_RAM_ALLOCATE_STRUCT( Entity ) ;
	if( pEntity == NULL )
		return NULL ;

	memset( pEntity, 0, sizeof *pEntity ) ;
	assert( (pEntity->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN

	if( !Object_Init( &pEntity->ObjectData, pGroup, KIND_ENTITY, pszName, nNumber )  )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Entity_Create:Object_Init" );
		goto EC_FAILURE ;
	}

	pEntity->pSymbolTable = pSymbols;
	pEntity->pszType = Util_StrDup( pszType );
	if( pEntity->pszType == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Entity_Create:Util_StrDup" );
		goto EC_FAILURE ;
	}

	pszNameAndNumber = Entity_AllocateNameWithNumber( pEntity ) ;
	if( pszNameAndNumber == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Entity_Create:Entity_AllocateNameWithNumber" );
		goto EC_FAILURE ;
	}
	
	pEntity->pSymbol = EntityTable_AddEntity( pSymbols, pszType, pszNameAndNumber ) ;
	jeRam_Free( pszNameAndNumber ) ;
	if( pEntity->pSymbol == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Entity_Create Object_Init" );
		goto EC_FAILURE ;
	}
	jeExtBox_Set( &pEntity->WorldBounds, ENTITY_BOX_MIN, ENTITY_BOX_MIN, ENTITY_BOX_MIN,
										ENTITY_BOX_MAX, ENTITY_BOX_MAX, ENTITY_BOX_MAX );
	Entity_GetOrigin( pEntity );

	return( pEntity );

EC_FAILURE :
	Entity_Destroy( &pEntity ) ;
	return NULL ;

}// Entity_Create

Entity * Entity_Copy(  Entity *	pEntity, int32 nNumber )
{
	Entity * pNewEntity;
	char	*	pszNameAndNumber ;

	pNewEntity = Entity_Create( pEntity->pSymbolTable, pEntity->ObjectData.pGroup, pEntity->pszType, pEntity->ObjectData.pszName, nNumber );
	if( pNewEntity == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Entity_Copy:Entity_Create" );
		return( NULL );
	}
	
	//Detroy the default symbol
	if( pNewEntity->pSymbol != NULL )
		jeSymbol_TableRemoveSymbol( pNewEntity->pSymbolTable, pNewEntity->pSymbol ) ;

	pszNameAndNumber = jeRam_Allocate( strlen( pEntity->ObjectData.pszName ) + ENTITY_MAXNUMBERLENGTH ) ;
	if( pszNameAndNumber == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Entity_Copy:pszNameAndNumber" );
		goto ECP_FAILURE ;
	}
	sprintf( pszNameAndNumber, "%s %d", pEntity->ObjectData.pszName, nNumber ) ;

	//Create a copy of the old symbol
	pNewEntity->pSymbol = EntityTable_CopyEntity( pEntity->pSymbolTable, pEntity->pSymbol, pszNameAndNumber ) ;

	jeRam_Free( pszNameAndNumber ) ;

	
	pNewEntity->Origin = pEntity->Origin;
	Entity_SetOrigin( pNewEntity );
	Entity_UpdateBounds( pNewEntity );

	return( pNewEntity );

ECP_FAILURE :
	Entity_Destroy( &pNewEntity ) ;
	return NULL ;
}

void Entity_Destroy( Entity ** ppEntity )
{
	assert( ppEntity != NULL ) ;
	assert( (*ppEntity)->nSignature == SIGNATURE ) ;

	if( (*ppEntity)->pSymbol != NULL )
		jeSymbol_TableRemoveSymbol( (*ppEntity)->pSymbolTable, (*ppEntity)->pSymbol ) ;

	assert( ((*ppEntity)->nSignature = 0) == 0 ) ;	// CLEAR
	(*ppEntity)->ObjectData.ObjectKind = KIND_INVALID ;

	jeRam_Free( *ppEntity ) ;
}// Entity_Destroy

Entity * Entity_CreateTemplate( const char * const pszType, jeSymbol_Table * pSymbols )
{
	Entity  * pEntity;

	assert( pszType != NULL ) ;
	assert( strlen( pszType ) < ENTITY_MAXNAMELENGTH ) ;

	pEntity = JE_RAM_ALLOCATE_STRUCT( Entity ) ;
	if( pEntity == NULL )
		return NULL ;

	memset( pEntity, 0, sizeof *pEntity ) ;
	assert( (pEntity->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN

	if( !Object_Init( &pEntity->ObjectData, NULL, KIND_ENTITY, pszType, 0 )  )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Entity_CreateTemplate:Object_Init" );
		goto EC_FAILURE ;
	}

	pEntity->pSymbolTable = pSymbols;
	pEntity->pszType = Util_StrDup( pszType );
	if( pEntity->pszType == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Entity_CreateTemplate:pszType" );
		goto EC_FAILURE ;
	}
	jeExtBox_Set( &pEntity->WorldBounds, ENTITY_BOX_MIN, ENTITY_BOX_MIN, ENTITY_BOX_MIN,
										ENTITY_BOX_MAX, ENTITY_BOX_MAX, ENTITY_BOX_MAX );
	return( pEntity );

EC_FAILURE :
	Entity_Destroy( &pEntity ) ;
	return NULL ;
}// Entity_CreateTemplate

Entity * Entity_FromTemplate( const char * pszName, Group * pGroup, const Entity *	pEntity, int32 nNumber )
{
	Entity * pNewEntity;

	pNewEntity = Entity_Create( pEntity->pSymbolTable, pGroup, pEntity->pszType, pszName, nNumber );
	if( pNewEntity == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Entity_FromTemplate:Entity_Create" );
		return( NULL );
	}
	pNewEntity->Origin = pEntity->Origin;
	Entity_SetOrigin( pNewEntity );
	Entity_UpdateBounds( pNewEntity );

	return( pNewEntity );
}
// MODIFIERS
void Entity_Move( Entity * pEntity, const jeVec3d * pWorldDistance )
{
	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;

	Entity_SetModified( pEntity );
	jeVec3d_Add( &pEntity->Origin, pWorldDistance, &pEntity->Origin );
	Entity_SetOrigin( pEntity );

}// Entity_Move

void Entity_Size( Entity * pEntity, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis )
{
	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;

	// The Z axis is flipped (down is positive)
	// We just determine the edge and call _SizeEdge once or twice to keep this
	// as simple as possible
	switch( eSizeType )
	{
	case Select_Top :
		if( Ortho_Axis_Z == VAxis )
			Entity_SizeEdge( pEntity, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			Entity_SizeEdge( pEntity, &pSelectedBounds->Min, vScale, VAxis ) ;
		break ;

	case Select_Bottom :
		if( Ortho_Axis_Z == VAxis )
			Entity_SizeEdge( pEntity, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			Entity_SizeEdge( pEntity, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		break ;

	case Select_Left :
		Entity_SizeEdge( pEntity, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;

	case Select_Right :
		Entity_SizeEdge( pEntity, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_TopLeft :
		if( Ortho_Axis_Z == VAxis )
			Entity_SizeEdge( pEntity, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			Entity_SizeEdge( pEntity, &pSelectedBounds->Min, vScale, VAxis ) ;
		Entity_SizeEdge( pEntity, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;		

	case Select_TopRight :
		if( Ortho_Axis_Z == VAxis )
			Entity_SizeEdge( pEntity, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			Entity_SizeEdge( pEntity, &pSelectedBounds->Min, vScale, VAxis ) ;
		Entity_SizeEdge( pEntity, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_BottomLeft :
		if( Ortho_Axis_Z == VAxis )
			Entity_SizeEdge( pEntity, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			Entity_SizeEdge( pEntity, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		Entity_SizeEdge( pEntity, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;
	
	case Select_BottomRight :
		if( Ortho_Axis_Z == VAxis )
			Entity_SizeEdge( pEntity, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			Entity_SizeEdge( pEntity, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		Entity_SizeEdge( pEntity, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;
	}
	Entity_SetModified( pEntity ) ;

}// Entity_Size

void Enity_SetField( const Entity * pEntity , jeSymbol *FieldSymbol, void *pData, int32 DataSize )
{
	jeSymbol_Type Type;
	
	assert( pEntity );
	assert( FieldSymbol );
	assert( pData );


	Type = jeSymbol_GetType( FieldSymbol );

	jeSymbol_SetProperty( pEntity->pSymbol, FieldSymbol, pData, 
		DataSize, Type );
}

void Entity_SetXForm( Entity * pEntity, const jeXForm3d * XForm )
{
	assert( pEntity );
	assert( SIGNATURE == pEntity->nSignature ) ;
	assert( XForm );

	pEntity->Origin = XForm->Translation;
	Entity_SetOrigin( pEntity );
	Entity_SetModified( pEntity );

}// Entity_SetXForm


void Entity_UpdateBounds( Entity * pEntity )
{
	assert( pEntity );

	jeExtBox_SetTranslation ( &pEntity->WorldBounds, &pEntity->Origin );

}

void Entity_SetModified( Entity * pEntity )
{
	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;
	
	pEntity->Flags |= ENTITY_FLAG_DIRTYALL ;	
}// Entity_SetModified


// ACCESSORS
void Entity_GetXForm( const Entity * pEntity, jeXForm3d * XForm )
{
	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;
	
	jeXForm3d_SetIdentity( XForm );
	XForm->Translation = pEntity->Origin;
}

const jeExtBox * Entity_GetWorldAxialBounds( const Entity * pEntity )
{
	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;
	
	if( pEntity->Flags & ENTITY_FLAG_WBOUNDSDIRTY )
	{	
		Entity * pEvalEntity = (Entity*)pEntity ;			// Lazy Evaluation requires removing the const
		Entity_UpdateBounds( pEvalEntity ) ;
	}

	return &pEntity->WorldBounds ;

}// Entity_GetWorldAxialBounds

void Entity_GetWorldDrawBounds( const Entity * pEntity, jeExtBox *DrawBounds )
{
	jeVec3d Center;
	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;
	
	if( pEntity->Flags & ENTITY_FLAG_WBOUNDSDIRTY )
	{	
		Entity * pEvalEntity = (Entity*)pEntity ;			// Lazy Evaluation requires removing the const
		Entity_UpdateBounds( pEvalEntity ) ;
	}
	jeExtBox_GetTranslation ( &pEntity->WorldBounds, &Center );
	jeExtBox_Set (  DrawBounds,
				  ENTITY_DRAW_MIN,	  ENTITY_DRAW_MIN,	  ENTITY_DRAW_MIN,
				  ENTITY_DRAW_MAX,	  ENTITY_DRAW_MAX,	  ENTITY_DRAW_MAX );
	jeExtBox_SetTranslation ( DrawBounds, &Center );

}// Entity_GetWorldDrawBounds


const char * Entity_GetType( const Entity * pEntity )
{
	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;
	
	return( pEntity->pszType );
} //Entity_GetType

//IS
jeBoolean	Entity_IsInRect( const Entity * pEntity, jeExtBox *pSelRect, jeBoolean bSelEncompeses )
{
	const jeExtBox *pWorldBounds;
	jeExtBox		Result;

	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;
	assert( pSelRect );
	
	pWorldBounds = Entity_GetWorldAxialBounds( pEntity ) ;
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
}//Entity_IsInRect

jeBoolean	Entity_GetField( const Entity * pEntity , jeSymbol *FieldSymbol, void *pData, int32 DataSize, jeBoolean *pDataInited )
{
	jeSymbol_Type Type;
	
	assert( pEntity );
	assert( FieldSymbol );
	assert( pData );
	assert( pDataInited );

	if( pEntity->pSymbol == NULL )
		return JE_TRUE ;

	Type = jeSymbol_GetType( FieldSymbol );

	if( !(*pDataInited) )
	{
		jeSymbol_GetProperty( pEntity->pSymbol, FieldSymbol, pData, 
			DataSize, Type );
		*pDataInited = JE_TRUE;
		return( JE_TRUE );
	}
	switch( Type )
	{
		int		Integer;
		int		*OldInteger;
		jeFloat	Float;
		jeFloat	*OldFloat;
		jeVec3d	Vector;
		jeVec3d	*OldVector;
		JE_RGBA	Color;
		JE_RGBA	*OldColor;
		char	*String;
		char	*OldString;

	case JE_SYMBOL_TYPE_INT:
		assert( DataSize == sizeof(Integer) );
		OldInteger = (int*)pData;
		jeSymbol_GetProperty(pEntity->pSymbol,
								  FieldSymbol,
								  &Integer,
								  sizeof(Integer),
								  JE_SYMBOL_TYPE_INT);
		return( Integer == *OldInteger );

	case JE_SYMBOL_TYPE_FLOAT:
		assert( DataSize == sizeof(jeFloat) );
		OldFloat = (jeFloat*)pData;
		jeSymbol_GetProperty(pEntity->pSymbol,
								  FieldSymbol,
								  &Float,
								  sizeof(jeFloat),
								  JE_SYMBOL_TYPE_FLOAT);
		return( Float == *OldFloat );

	case JE_SYMBOL_TYPE_COLOR:
		assert( DataSize == sizeof(JE_RGBA) );
		OldColor = (JE_RGBA*)pData;
		jeSymbol_GetProperty(pEntity->pSymbol,
								  FieldSymbol,
								  &Color,
								  sizeof(JE_RGBA),
								  JE_SYMBOL_TYPE_COLOR);
		return( Color.r == OldColor->r &&
				Color.g	== OldColor->g &&
				Color.b	== OldColor->b &&
				Color.a	== OldColor->a 	);

	case JE_SYMBOL_TYPE_VEC3D:
		assert( DataSize == sizeof(jeVec3d) );
		OldVector = (jeVec3d*)pData;
		jeSymbol_GetProperty(pEntity->pSymbol,
								  FieldSymbol,
								  &Vector,
								  sizeof(jeVec3d),
								  JE_SYMBOL_TYPE_VEC3D);
		return( jeVec3d_Compare( &Vector, OldVector, 0.0f ) );

	case JE_SYMBOL_TYPE_STRING:
		assert( DataSize == sizeof(char	*) );
		OldString = (char	*)pData;
		jeSymbol_GetProperty(pEntity->pSymbol,
								  FieldSymbol,
								  &String,
								  sizeof(char	*),
								  JE_SYMBOL_TYPE_STRING);
		return( !strcmp( String, OldString ) );

	default:
		assert(!"Not finished here");
		return JE_FALSE;
	}

	return JE_FALSE;
}


jeBoolean Enity_SelectClosest(  Entity * pEntity, FindInfo	*	pFindInfo )
{
	Point				pt;
	jeVec3d				wpt ;
	jeFloat				DistSq ;
	jeXForm3d			wXForm;

	assert( pEntity != NULL );
	assert( pFindInfo != NULL );
	assert( pFindInfo->pOrtho  != NULL ) ;
			

	Entity_GetXForm( pEntity, &wXForm );
	wpt = wXForm.Translation;
	Ortho_WorldToView( pFindInfo->pOrtho, &wpt, &pt ) ;
	DistSq = Util_PointDistanceSquared( &pt, pFindInfo->pViewPt );
	if( DistSq < pFindInfo->fMinDistance )
	{
		pFindInfo->fMinDistance = DistSq ;
		pFindInfo->pObject = (Object*)pEntity ;
		pFindInfo->nFace = 0 ;
		pFindInfo->nFaceEdge = 0;
	}
	return( JE_TRUE );
}

// FILE HANDLING

Entity * Entity_CreateFromFile( jeVFile * pF, const int32 nVersion, jeSymbol_Table * pEntities )
{
	Entity	*	pEntity = NULL ;
	char		szType[ ENTITY_MAXNAMELENGTH ] ;
	assert( jeVFile_IsValid( pF ) ) ;
	assert( nVersion <= ENTITY_VERSION ) ;
	
	if( ENTITY_VERSION != nVersion )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "Entity_CreateFromFile.\n", NULL);
		return NULL ;
	}

	if( !Util_geVFile_ReadString( pF, szType, ENTITY_MAXNAMELENGTH ) )
		goto ECFF_FAILURE ;

	pEntity = Entity_CreateTemplate( szType, pEntities ) ;
	if( pEntity == NULL )
		goto ECFF_FAILURE ;

	jeRam_Free( pEntity->ObjectData.pszName ) ;
	if( !Object_InitFromFile( pF , &pEntity->ObjectData ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Object_InitFromFile.\n", NULL);
		goto ECFF_FAILURE ;
	}
	if( !jeVFile_Read( pF, &pEntity->Flags, sizeof pEntity->Flags ) )
		goto ECFF_FAILURE ;

	if( !jeVFile_Read( pF, &pEntity->WorldBounds, sizeof pEntity->WorldBounds ) )
		goto ECFF_FAILURE ;

	if( !jeVFile_Read( pF, &pEntity->Origin, sizeof pEntity->Origin ) )
		goto ECFF_FAILURE ;

	return pEntity ;

ECFF_FAILURE :
	if( pEntity != NULL )
		Object_Free( (Object**)pEntity ) ;

	jeErrorLog_AddString(JE_ERR_FILEIO_READ, "Entity_CreateFromFile.\n", NULL);
	return NULL ;

}// Entity_CreateFromFile


jeBoolean Entity_WriteToFile( Entity * pEntity, jeVFile * pF )
{
	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	if( jeVFile_Write( pF, pEntity->pszType, strlen( pEntity->pszType )+1 ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Entity_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	if( !Object_WriteToFile( &pEntity->ObjectData, pF ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	if( jeVFile_Write( pF, &pEntity->Flags, sizeof pEntity->Flags ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Entity_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	if( jeVFile_Write( pF, &pEntity->WorldBounds, sizeof pEntity->WorldBounds ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Entity_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	if( jeVFile_Write( pF, &pEntity->Origin, sizeof pEntity->Origin ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Entity_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	return JE_TRUE ;

}// Entity_WriteToFile

jeBoolean Entity_Reattach( Entity * pEntity )
{
	char * pszNameAndNumber ;
	assert( pEntity != NULL ) ;
	assert( SIGNATURE == pEntity->nSignature ) ;
	assert( pEntity->pSymbol == NULL ) ;
	assert( pEntity->pSymbolTable != NULL ) ;

	pszNameAndNumber = Entity_AllocateNameWithNumber( pEntity ) ;
	if( pszNameAndNumber == NULL )
		return JE_FALSE ;

	pEntity->pSymbol = EntityTable_FindSymbol( pEntity->pSymbolTable, pEntity->pszType, pszNameAndNumber ) ;
	jeRam_Free( pszNameAndNumber ) ;
	return ( pEntity->pSymbol == NULL ) ? JE_FALSE : JE_TRUE ;

}// Entity_Reattach

/* EOF: Entity.c */