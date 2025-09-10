/****************************************************************************************/
/*  ENTITYTABLE.C                                                                       */
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
#include <Math.h>
#include <Stdio.h>
#include <StdLib.h>
#include <String.h>

#include "jeTypes.h"
#include "EclipseNames.h"
#include "Vec3d.h"

#include "EntityTable.h"

typedef struct tagFieldDef
{
	char		*	pszName ;
	jeSymbol_Type	Type ;
	char		*	pszDefault ;
} FieldDef ;

const static FieldDef TestFields[] = 
{
	{ "MinRadius", 	JE_SYMBOL_TYPE_INT, 	"20" },
	{ "MaxRadius", 	JE_SYMBOL_TYPE_INT, 	"40" },
	{ "FadeTime", 	JE_SYMBOL_TYPE_FLOAT, 	"0.5" },
	{ "Color", 		JE_SYMBOL_TYPE_COLOR, 	"255 255 255" },
	{ "Origin", 	JE_SYMBOL_TYPE_VEC3D, 	"5 0 5" },
	{ "Description", JE_SYMBOL_TYPE_STRING, "Hello there!" },
} ;

const static FieldDef PlayerStart[] = 
{
	{ "Origin",		JE_SYMBOL_TYPE_VEC3D,	"12 0 12" }
} ;

typedef struct tagDefaultsTypes
{
	char			*	pszName ;
	const FieldDef	*	pFields ;
	int					nFields ;
} DefaultTypes ;

const static DefaultTypes Defaults[] = 
{
	{ "Corona", TestFields, sizeof(TestFields)/sizeof(TestFields[0]) },
	{ "PlayerStart", PlayerStart, sizeof(PlayerStart)/sizeof(PlayerStart[0]) }
} ;

jeSymbol_Table * EntityTable_Create( void )
{
	return jeSymbol_TableCreate() ;
}// EntityTable_Create


void EntityTable_Destroy( jeSymbol_Table ** ppSymbols )
{
	assert( ppSymbols != NULL ) ;
	assert( *ppSymbols != NULL ) ;

	jeSymbol_TableDestroy( ppSymbols ) ;

}// EntityTable_Destroy

jeBoolean EntityTable_AddField( jeSymbol_Table * pSymbols, jeSymbol * pTypeSym, const char *Name, jeSymbol_Type Type, void *DefaultValue )
{
	jeSymbol_List *	pFieldList;
	jeSymbol *		pFieldSym;
	jeBoolean		bFound;

	bFound = jeSymbol_GetProperty
	(
		pTypeSym,
		jeEclipseNames(pSymbols, JE_ECLIPSENAMES_STRUCTUREFIELDS),
		&pFieldList, 
		sizeof(pFieldList), 
		JE_SYMBOL_TYPE_LIST
	);

	if( JE_FALSE == bFound )
	{
		pFieldList = jeSymbol_ListCreate( pSymbols );
		if( !pFieldList )
			return JE_FALSE ;
		if( jeSymbol_SetProperty( pTypeSym,
							jeEclipseNames(pSymbols, JE_ECLIPSENAMES_STRUCTUREFIELDS),
							&pFieldList, sizeof(pFieldList), JE_SYMBOL_TYPE_LIST) == JE_FALSE)
		{
			jeSymbol_ListDestroy( &pFieldList ) ;
			return JE_FALSE;
		}
	}

	pFieldSym = jeSymbol_Create(pSymbols, pTypeSym, Name, Type ) ;
	if( !pFieldSym )
		return JE_FALSE ;
	
	if( EntityTable_SetDefaultValue( pSymbols, pFieldSym, DefaultValue ) == JE_FALSE )
	{
		jeSymbol_Destroy( &pFieldSym ) ;
		return JE_FALSE;
	}

	if( jeSymbol_ListAddSymbol( pFieldList, pFieldSym ) == JE_FALSE )
	{
		jeSymbol_Destroy( &pFieldSym ) ;
		return JE_FALSE;
	}

	jeSymbol_Destroy( &pFieldSym ) ;
	jeSymbol_ListDestroy( &pFieldList ) ;

	return JE_TRUE;
}// EntityTable_AddField

jeBoolean EntityTable_AddFieldToInstances( jeSymbol_Table * pSymbols, jeSymbol * pDef, const char * pszName, jeSymbol_Type Type, void * DefaultValue )
{
	jeSymbol_List	*	pList ;
	jeSymbol		*	pEntity ;
	jeBoolean			bContinue ;
	int					iIndex ;
	assert( pSymbols != NULL ) ;
	assert( pDef != NULL ) ;
	assert( pszName != NULL ) ;
	assert( strlen(pszName) < ENTITY_MAXNAMELENGTH ) ;
	assert( DefaultValue != NULL ) ;

	pList = jeSymbol_TableGetQualifiedSymbolList( pSymbols, jeSymbol_GetQualifier( pDef ) ) ;
	if( pList == NULL )
		return JE_FALSE ;

	iIndex = 0 ;
	bContinue = JE_TRUE ;
	while( bContinue && (pEntity = jeSymbol_ListGetSymbol( pList, iIndex )) != NULL )
	{
		iIndex++ ;
		if( jeSymbol_Compare( pEntity, pDef ) == JE_FALSE )
		{
			bContinue = EntityTable_AddField( pSymbols, pDef, pszName, Type, DefaultValue ) ;
		}
	}
	jeSymbol_ListDestroy( &pList ) ;

	return bContinue ;
}// EntityTable_AddFieldToInstances

jeSymbol * EntityTable_CreateType( jeSymbol_Table * pSymbols, const char * pszName )
{
	jeSymbol		*	pTypeSym ;
	jeSymbol		*	pQualifier ;
	jeSymbol_List	*	pDefTypeList ;
	jeSymbol		*	pGlobalTypesSymbol ;
	jeSymbol		*	pDefinitionsProperty ;
	jeBoolean			bSuccess ;

	pGlobalTypesSymbol = jeEclipseNames( pSymbols, JE_ECLIPSENAMES_TYPES ) ;
	pDefinitionsProperty = jeEclipseNames( pSymbols, JE_ECLIPSENAMES_TYPEDEFINITIONS ) ;
	if( pGlobalTypesSymbol == NULL || pDefinitionsProperty == NULL )
		return NULL ;
	
	// Can't do it if the symbol already exists.
	if( jeSymbol_TableFindSymbol( pSymbols, NULL, pszName ) )
		return NULL ;

	// Create a package for the type, and intern the type symbol in that package
	pQualifier = jeSymbol_Create( pSymbols, NULL, pszName, JE_SYMBOL_TYPE_VOID ) ;
	if( !pQualifier )
		return NULL ;

	pTypeSym = jeSymbol_Create( pSymbols, pQualifier, pszName, JE_SYMBOL_TYPE_VOID ) ;
	jeSymbol_Destroy( &pQualifier ) ;

	if( jeSymbol_GetProperty( pGlobalTypesSymbol, pDefinitionsProperty, &pDefTypeList, sizeof pDefTypeList, JE_SYMBOL_TYPE_LIST ) == JE_FALSE )
	{
		// First time thru--create list and set it's property
		pDefTypeList = jeSymbol_ListCreate( pSymbols ) ;
		bSuccess = JE_FALSE ;
		if( pDefTypeList != NULL )
		{
			bSuccess = jeSymbol_SetProperty( pGlobalTypesSymbol, pDefinitionsProperty, &pDefTypeList, sizeof( pDefTypeList ), JE_SYMBOL_TYPE_LIST ) ;
		}

		if( pDefTypeList == NULL || bSuccess == JE_FALSE )
		{
			jeSymbol_TableRemoveSymbol( pSymbols, pTypeSym ) ;
			jeSymbol_Destroy( &pTypeSym ) ;
			return NULL ;
		}
	}
	
	if( jeSymbol_ListAddSymbol( pDefTypeList, pTypeSym ) == JE_FALSE )
	{
		jeSymbol_TableRemoveSymbol( pSymbols, pTypeSym ) ;
		jeSymbol_Destroy( &pTypeSym ) ;
		return NULL ;
	}

	jeSymbol_ListDestroy( &pDefTypeList ) ;

	return pTypeSym ;
}// EntityTable_CreateType

int32 EntityTable_ListGetNumItems( jeSymbol_List * pList )
{
	jeSymbol *	p ;
	int32		nCount ;
	assert( pList != NULL ) ;

	nCount = 0 ;
	while( (p = jeSymbol_ListGetSymbol( pList, nCount )) != NULL )
	{
		nCount++ ;
	}
	return nCount ;
}// EntityTable_ListGetNumItems

jeBoolean EntityTable_EnumDefinitions( jeSymbol_Table * pSymbols, void * pVoid, EntityTable_ForEachCallback Callback )
{
	jeSymbol_List	*	pDefTypeList ;
	jeSymbol		*	pGlobalTypesSymbol ;
	jeSymbol		*	pDefinitionsProperty ;
	jeSymbol		*	pSymbol ;
	int					Index = 0 ;
	jeBoolean			bContinue ;

	assert( pSymbols != NULL ) ;

	pGlobalTypesSymbol = jeEclipseNames( pSymbols, JE_ECLIPSENAMES_TYPES ) ;
	pDefinitionsProperty = jeEclipseNames( pSymbols, JE_ECLIPSENAMES_TYPEDEFINITIONS ) ;
	if( pGlobalTypesSymbol == NULL || pDefinitionsProperty == NULL )
		return JE_FALSE ;

	jeSymbol_GetProperty( pGlobalTypesSymbol, pDefinitionsProperty, &pDefTypeList, sizeof pDefTypeList, JE_SYMBOL_TYPE_LIST ) ;
	assert( pDefTypeList != NULL ) ;

	bContinue = JE_TRUE ;
	while( bContinue && (pSymbol = jeSymbol_ListGetSymbol( pDefTypeList, Index )) != NULL )
	{
		Index++ ;
		bContinue = Callback( pSymbol, pVoid ) ;
	}

	return JE_TRUE ;

}// EntityTable_Enum

jeBoolean EntityTable_EnumFields( jeSymbol_Table * pST, const char * pszType, void * pVoid, EntityTable_ForEachCallback Callback )
{
	jeSymbol		*	pTypeSym ;
	jeSymbol_List	*	pFieldList ;
	jeSymbol		*	pFieldSym ;
	jeSymbol		*	pEntityDef ;
	int					iField ;
	jeBoolean			b ;

	assert( pST != NULL ) ;

	pTypeSym = jeSymbol_TableFindSymbol( pST, NULL, pszType ) ;
	assert( pTypeSym != NULL ) ;

	pEntityDef = jeSymbol_TableFindSymbol( pST, pTypeSym, jeSymbol_GetName( pTypeSym ) ) ;	// TYPE::TYPE

	b = jeSymbol_GetProperty
	(
		pEntityDef,
		jeEclipseNames(pST, JE_ECLIPSENAMES_STRUCTUREFIELDS),
		&pFieldList, 
		sizeof(pFieldList), 
		JE_SYMBOL_TYPE_LIST
	);
	assert( b ) ;
	if( JE_FALSE == b )
		return JE_FALSE ;
	
	iField = 0 ;
	while( b && (pFieldSym = jeSymbol_ListGetSymbol( pFieldList, iField )) != NULL )
	{
		b = Callback( pFieldSym, pVoid ) ;
		iField++ ;
	}
	jeSymbol_ListDestroy( &pFieldList ) ;

	return b ;

}// EntityTable_EnumFields

jeBoolean EntityTable_InitDefault( jeSymbol_Table * pSymbols )
{
	int					i ;
	int					j ;
	int					nFields ;
	jeSymbol		*	pDef ;
	assert( pSymbols != NULL ) ;

	for( i=0; i<sizeof(Defaults)/sizeof(Defaults[0]); i++ )
	{
		pDef = EntityTable_CreateType( pSymbols, Defaults[i].pszName ) ;
		if( pDef != NULL )
		{
			nFields = Defaults[i].nFields ;
			for( j=0; j<nFields; j++ )
			{
				EntityTable_AddField
				( 
					pSymbols, 
					pDef, 
					Defaults[i].pFields[j].pszName,
					Defaults[i].pFields[j].Type,
					Defaults[i].pFields[j].pszDefault
				) ;
			}
		}
	}
	
	return JE_TRUE ;
}// EntityTable_InitDefault

void EntityTable_RemoveDefaultEntityField( jeSymbol_Table * pST, jeSymbol * pSymbol )
{
	jeSymbol		*	pEntityDef ;
	jeSymbol_List	*	pFieldList ;
	jeBoolean			b ;
	assert( pST != NULL ) ;
	assert( pSymbol != NULL ) ;
	
	// Incoming ENTITY::ENTITY::FIELD
	pEntityDef = jeSymbol_GetQualifier( pSymbol ) ;	// ENTITY::ENTITY

	// Remove from structure fields
	b = jeSymbol_GetProperty
	(
		pEntityDef,
		jeEclipseNames(pST, JE_ECLIPSENAMES_STRUCTUREFIELDS),
		&pFieldList, 
		sizeof(pFieldList), 
		JE_SYMBOL_TYPE_LIST
	);
	if( b )
	{
		jeSymbol_ListRemoveSymbol( pFieldList, pSymbol ) ;
		jeSymbol_ListDestroy( &pFieldList ) ;
	}

	jeSymbol_TableRemoveSymbol( pST, pSymbol ) ;

}// EntityTable_RemoveDefaultEntityField

void EntityTable_RemoveEntityAndInstances( jeSymbol_Table * pST, jeSymbol * pEntityDef )
{
	jeSymbol_List	*	pList ;
	jeSymbol		*	p ;
	int32				nIndex ;

	assert( pST != NULL ) ;
	assert( pEntityDef != NULL ) ;

	// List will have the "definition" and all instances
	pList = jeSymbol_TableGetQualifiedSymbolList( pST, jeSymbol_GetQualifier(pEntityDef) ) ;
	assert( pList != NULL ) ;
	nIndex = 0 ;
	while( (p = jeSymbol_ListGetSymbol( pList, nIndex )) != NULL )
	{
		nIndex++ ;
		jeSymbol_TableRemoveSymbol( pST, p ) ;
		jeSymbol_Destroy( &p ) ;
	}
	jeSymbol_ListDestroy( &pList ) ;

//	EntityTable_RemoveDefaultEntityField( pST, pEntityDef ) ;
//	jeSymbol_Destroy( &pEntityDef ) ;

}// EntityTable_RemoveEntityAndInstances


jeBoolean EntityTable_SetDefaultValue( jeSymbol_Table *pST, jeSymbol *pFieldSym, void *DefaultValue )
{
	jeSymbol_Type	Type;
	jeSymbol *		pDefaultValueSym;

	pDefaultValueSym = pFieldSym ;

//	DefaultValueSym = jeEclipseNames(ST, JE_ECLIPSENAMES_FIELDDEFAULTVALUE);
//	if	(!DefaultValueSym)
//		return JE_FALSE;

	Type = jeSymbol_GetType( pFieldSym ) ;
	switch( Type )
	{
		int		Integer;
		jeFloat	Float;
		jeVec3d	Vector;
		JE_RGBA	Color;

	case JE_SYMBOL_TYPE_INT:
		Integer = atoi(DefaultValue);
		return jeSymbol_SetProperty(pFieldSym,
								  pDefaultValueSym,
								  &Integer,
								  sizeof(Integer),
								  JE_SYMBOL_TYPE_INT);

	case JE_SYMBOL_TYPE_FLOAT:
		Float = (jeFloat)atof(DefaultValue);
		return jeSymbol_SetProperty(pFieldSym, 
								pDefaultValueSym,
								&Float, sizeof(Float), JE_SYMBOL_TYPE_FLOAT);

	case JE_SYMBOL_TYPE_COLOR:
		sscanf( DefaultValue, "%f %f %f", &Color.r, &Color.g, &Color.b);
		return jeSymbol_SetProperty( pFieldSym, 
								pDefaultValueSym,
								&Color, sizeof(Color), JE_SYMBOL_TYPE_COLOR);

	case JE_SYMBOL_TYPE_VEC3D:
		sscanf(DefaultValue, "%f %f %f", &Vector.X, &Vector.Y, &Vector.Z);
		return jeSymbol_SetProperty(pFieldSym, 
								pDefaultValueSym,
								&Vector, sizeof(Vector), JE_SYMBOL_TYPE_VEC3D);
		break;

	case JE_SYMBOL_TYPE_STRING:
		return jeSymbol_SetProperty(pFieldSym, 
								pDefaultValueSym,
								DefaultValue, sizeof(DefaultValue), JE_SYMBOL_TYPE_STRING);
		break;

	default:
		assert(!"Not finished here");
		return JE_FALSE;
	}

	assert(!"Shouldn't get here");
	pST;
}// EntityTable_SetDefaultValue

//jeSymbol * Entity_TableGetType( 

jeSymbol * EntityTable_GetField( jeSymbol_Table * pST, jeSymbol * pEntity, const char * pszName )
{
	jeSymbol * pQualifier ;
	jeSymbol * pEntityDef ;
	jeSymbol * pField = NULL ;

	assert( pST != NULL ) ;
	assert( pEntity != NULL ) ;
	assert( pszName != NULL ) ;
	assert( strlen( pszName ) < ENTITY_MAXNAMELENGTH ) ;

	pQualifier = jeSymbol_GetQualifier( pEntity ) ;	// TYPE:
	pEntityDef = jeSymbol_TableFindSymbol( pST, pQualifier, jeSymbol_GetName( pQualifier ) ) ;	// TYPE::TYPE
	if( pEntityDef != NULL )
		pField = jeSymbol_TableFindSymbol( pST, pEntityDef, pszName ) ;	// TYPE::TYPE::FIELD

	return pField ;

}// EntityTable_GetField

jeSymbol * EntityTable_AddEntity( jeSymbol_Table * pST, const char * pszType, const char * pszName )
{
	jeSymbol		*	pType ;
	jeSymbol		*	pEntityDef ;
	jeSymbol		*	pEntity ;
	jeSymbol		*	pProperty ;
	jeSymbol_List	*	pFieldList ;
	int					iProperty ;
	jeBoolean			b ;

	assert( pST != NULL ) ;
	assert( pszType != NULL ) ;
	assert( strlen( pszType ) < ENTITY_MAXNAMELENGTH ) ;
	assert( pszName != NULL ) ;
	assert( strlen( pszName ) < ENTITY_MAXNAMELENGTH ) ;

	pType = jeSymbol_TableFindSymbol( pST, NULL, pszType ) ;
	assert( pType != NULL ) ;

	pEntityDef = jeSymbol_Create( pST, pType/*jeSymbol_GetQualifier( pType )*/, pszType, JE_SYMBOL_TYPE_SYMBOL ) ;
	if( pEntityDef == NULL )
		return NULL ;

	pEntity = jeSymbol_Create( pST, pType, pszName, JE_SYMBOL_TYPE_SYMBOL ) ;
	if( pEntity == NULL )
		return NULL ;

	// Get the list of fields for this Entity
	b = jeSymbol_GetProperty
	( 
		pEntityDef, 
		jeEclipseNames( pST, JE_ECLIPSENAMES_STRUCTUREFIELDS), 
		&pFieldList, 
		sizeof pFieldList, 
		JE_SYMBOL_TYPE_LIST
	) ;
	if( b == JE_FALSE )
		return pEntity ;

	// For each field, set the default value from the def entity
	iProperty = 0 ;
	b = JE_TRUE ;
	while( b && (pProperty = jeSymbol_ListGetSymbol( pFieldList, iProperty )) != NULL )
	{
		b = jeSymbol_CopyProperty( pEntity, pProperty, pProperty, pProperty ) ;
		if( b == JE_FALSE )
		{
			jeSymbol_Destroy( &pEntity ) ;
			pEntity = NULL ;		// Just to be sure
			break ;
		}
		iProperty++ ;
	}

	jeSymbol_ListDestroy( &pFieldList ) ;

	return pEntity ;
}// EntityTable_AddEntity


jeSymbol * EntityTable_CopyEntity( jeSymbol_Table * pST, jeSymbol * pEntity, const char * pszName )
{
	jeSymbol		*	pType ;
	jeSymbol		*	pEntityDef ;
	jeSymbol		*	pNewEntity ;
	jeSymbol		*	pProperty ;
	jeSymbol_List	*	pFieldList ;
	int					iProperty ;
	jeBoolean			b ;

	assert( pST != NULL ) ;
	assert( pszName != NULL ) ;
	assert( strlen( pszName ) < ENTITY_MAXNAMELENGTH ) ;

	pType = jeSymbol_GetQualifier( pEntity ) ;	// TYPE:
	assert( pType != NULL ) ;

	pEntityDef = jeSymbol_TableFindSymbol( pST, pType, jeSymbol_GetName( pType ) ) ;
	if( pEntityDef == NULL )
		return NULL ;

	pNewEntity = jeSymbol_Create( pST, pType, pszName, JE_SYMBOL_TYPE_SYMBOL ) ;
	if( pNewEntity == NULL )
		return NULL ;

	// Get the list of fields for this Entity
	b = jeSymbol_GetProperty
	( 
		pEntityDef, 
		jeEclipseNames( pST, JE_ECLIPSENAMES_STRUCTUREFIELDS), 
		&pFieldList, 
		sizeof pFieldList, 
		JE_SYMBOL_TYPE_LIST
	) ;
	if( b == JE_FALSE )
		return pNewEntity ;

	// For each field, set the default value from the def entity
	iProperty = 0 ;
	b = JE_TRUE ;
	while( b && (pProperty = jeSymbol_ListGetSymbol( pFieldList, iProperty )) != NULL )
	{
		b = jeSymbol_CopyProperty( pNewEntity, pProperty, pEntity, pProperty ) ;
		if( b == JE_FALSE )
		{
			jeSymbol_Destroy( &pNewEntity ) ;
			pNewEntity = NULL ;		// Just to be sure
			break ;
		}
		iProperty++ ;
	}

	jeSymbol_ListDestroy( &pFieldList ) ;

	return pNewEntity ;
}// EntityTable_CopyEntity


jeSymbol * EntityTable_FindSymbol( jeSymbol_Table * pST, const char * pszType, const char * pszName )
{
	jeSymbol * pTypeSym ;
	jeSymbol * pEntity ;

	assert( pST != NULL ) ;
	assert( pszType != NULL ) ;
	assert( strlen(pszType) < ENTITY_MAXNAMELENGTH ) ;
	assert( pszName != NULL ) ;
	assert( strlen(pszName) < ENTITY_MAXNAMELENGTH ) ;

	pTypeSym = jeSymbol_TableFindSymbol( pST, NULL, pszType ) ;
	assert( pTypeSym != NULL ) ;

	pEntity = jeSymbol_TableFindSymbol( pST, pTypeSym, pszName ) ;
	assert( pEntity != NULL ) ;

	return pEntity ;

}// EntityTable_FindSymbol


/* EOF: EntityTable.c */