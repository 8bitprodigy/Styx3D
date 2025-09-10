/****************************************************************************************/
/*  JEPROPERTY.C                                                                        */
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

#include <assert.h>
#include <string.h>

#include "jeProperty.h"
#include "Ram.h"
#include "Errorlog.h"
#include "Util.h"


//========================================================================================================
//	jeProperty_ListCreate
//========================================================================================================
JETAPI jeProperty_List * JETCC jeProperty_ListCreate( int FieldN )
{
	jeProperty_List * pArray = NULL;
	jeProperty *pjeProperty = NULL;

	pArray = JE_RAM_ALLOCATE_STRUCT( jeProperty_List );
	if( pArray == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "jeProperty_List" );
		return( NULL );
	}

	pArray->pjeProperty = JE_RAM_ALLOCATE_ARRAY_CLEAR( jeProperty, FieldN );
	if( pArray->pjeProperty == NULL )
		return( NULL );
	pArray->jePropertyN =  FieldN;
	pArray->bDirty = JE_FALSE;

	return( pArray );
}

//========================================================================================================
//	jeProperty_ListCreate
//========================================================================================================
JETAPI jeProperty_List * JETCC jeProperty_ListCreateEmpty( )
{
	jeProperty_List * pArray = NULL;
	jeProperty *pjeProperty = NULL;

	pArray = JE_RAM_ALLOCATE_STRUCT( jeProperty_List );
	if( pArray == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "jeProperty_List" );
		return( NULL );
	}

	pArray->jePropertyN =  0;
	pArray->pjeProperty = NULL;
	pArray->bDirty = JE_FALSE;

	return( pArray );
}

//========================================================================================================
//	jeProperty_ListCopy
//========================================================================================================
JETAPI jeProperty_List * JETCC jeProperty_ListCopy( jeProperty_List * pArray)
{
	jeProperty_List * pNewArray;
	int i;

	pNewArray = jeProperty_ListCreate( pArray->jePropertyN );
	if( pNewArray == NULL )
		return( NULL );

	for( i = 0; i < pArray->jePropertyN; i++ )
	{
		pNewArray->pjeProperty[i] = pArray->pjeProperty[i];
		if( pArray->pjeProperty[i].FieldName != NULL )
		{
			pNewArray->pjeProperty[i].FieldName = Util_StrDup( pArray->pjeProperty[i].FieldName );
			if( pNewArray->pjeProperty[i].FieldName == NULL )
				goto PLC_ERROR;
		}
		pNewArray->jePropertyN = i+1;
	}
	pNewArray->bDirty = pArray->bDirty;

	return( pNewArray );
PLC_ERROR:
	jeProperty_ListDestroy( &pNewArray );
	return( NULL );
}

JETAPI jeProperty_List * JETCC jeProperty_ListConCat( jeProperty_List * pArray, jeProperty_List *pArray2 )
{
	jeProperty_List * pNewArray;
	int i;
	int Array1End;

	pNewArray = jeProperty_ListCreate( pArray->jePropertyN + pArray2->jePropertyN );
	if( pNewArray == NULL )
		return( NULL );
	pNewArray->jePropertyN = 0;
	for( i = 0; i < pArray->jePropertyN; i++ )
	{
		pNewArray->pjeProperty[i] = pArray->pjeProperty[i];
		if( pArray->pjeProperty[i].FieldName != NULL )
		{
			pNewArray->pjeProperty[i].FieldName = Util_StrDup( pArray->pjeProperty[i].FieldName );
			if( pNewArray->pjeProperty[i].FieldName == NULL )
				goto PLC_ERROR;
		}
		pNewArray->jePropertyN++;
	}
	Array1End = i;
	for( i = 0; i < pArray2->jePropertyN; i++ )
	{
		pNewArray->pjeProperty[Array1End+i] = pArray2->pjeProperty[i];
		if( pArray2->pjeProperty[i].FieldName != NULL )
		{
			pNewArray->pjeProperty[Array1End+i].FieldName = Util_StrDup( pArray2->pjeProperty[i].FieldName );
			if( pNewArray->pjeProperty[Array1End+i].FieldName == NULL )
				goto PLC_ERROR;
		}
		pNewArray->jePropertyN++;
	}
	pNewArray->bDirty = pArray->bDirty | pArray2->bDirty ;
	return( pNewArray );
PLC_ERROR:
	jeProperty_ListDestroy( &pNewArray );
	return( NULL );
}
//========================================================================================================
//	jeProperty_Append
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_Append( jeProperty_List *pArray, jeProperty *pjeProperty )
{
	jeProperty *pjePropertyArray = NULL;

	if( pArray->pjeProperty == NULL )
	{
		pArray->pjeProperty = JE_RAM_ALLOCATE_STRUCT( jeProperty );
		if( pArray->pjeProperty == NULL )
			return( JE_FALSE );
		pArray->jePropertyN = 0;
	}
	else
	{
		pjePropertyArray = JE_RAM_REALLOC_ARRAY( pArray->pjeProperty, jeProperty, pArray->jePropertyN+1);
		if( pjePropertyArray == NULL )
			return( JE_FALSE );
		pArray->pjeProperty = pjePropertyArray;
	}
	pArray->pjeProperty[pArray->jePropertyN] = *pjeProperty;
	pArray->jePropertyN += 1;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillCheck
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillCheck( jeProperty * pjeProperty, char *Name, int Value, int FieldId )
{
	assert( pjeProperty != NULL );
	assert( Name != NULL );
	
	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_CHECK_TYPE;
	pjeProperty->Data.Bool = Value;
	pjeProperty->DataSize = sizeof( int );
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillRadio
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillRadio( jeProperty * pjeProperty, char *Name, int Value, int FieldId )
{
	assert( pjeProperty != NULL );
	assert( Name != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_RADIO_TYPE;
	pjeProperty->Data.Bool = Value;
	pjeProperty->DataSize = sizeof( int );
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillVec3dGroup
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillVec3dGroup( jeProperty * pjeProperty, char *Name, const jeVec3d *Vector, int FieldId )
{

	assert( pjeProperty != NULL );
	assert( Name != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_VEC3D_GROUP_TYPE;
	pjeProperty->Data.Vector = *Vector;
	pjeProperty->DataSize = sizeof( jeVec3d );
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillColorGroup
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillColorGroup( jeProperty * pjeProperty, char *Name, const jeVec3d *Vector, int FieldId )
{

	assert( pjeProperty != NULL );
	assert( Name != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_COLOR_GROUP_TYPE;
	pjeProperty->Data.Vector = *Vector;
	pjeProperty->DataSize = sizeof( jeVec3d );
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillFloat
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillFloat( jeProperty * pjeProperty, char *Name, float Float, int FieldId, float Min, float Max, float Increment )
{

	assert( pjeProperty != NULL );
	assert( Name != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_FLOAT_TYPE;
	pjeProperty->Data.Float = Float;
	pjeProperty->DataSize = sizeof( float );
	pjeProperty->DataId = FieldId;
	pjeProperty->TypeInfo.NumInfo.Min = Min;
	pjeProperty->TypeInfo.NumInfo.Max = Max;
	pjeProperty->TypeInfo.NumInfo.Increment = Increment;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillInt
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillInt( jeProperty * pjeProperty, char *Name, int Int, int FieldId, float Min, float Max, float Increment )
{
	assert( pjeProperty != NULL );
	assert( Name != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_INT_TYPE;
	pjeProperty->Data.Int = Int;
	pjeProperty->DataSize = sizeof( int );
	pjeProperty->DataId = FieldId;
	pjeProperty->TypeInfo.NumInfo.Min = Min;
	pjeProperty->TypeInfo.NumInfo.Max = Max;
	pjeProperty->TypeInfo.NumInfo.Increment = Increment;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillInt
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillStaticInt( jeProperty * pjeProperty, char *Name, int Int, int FieldId )
{
	assert( pjeProperty != NULL );
	assert( Name != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_STATIC_INT_TYPE;
	pjeProperty->Data.Int = Int;
	pjeProperty->DataSize = sizeof( int );
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillString
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillString( jeProperty * pjeProperty, char *Name, char *String, int FieldId )
{
	assert( pjeProperty != NULL );
	assert( Name != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_STRING_TYPE;
	pjeProperty->Data.String = String;
	pjeProperty->DataSize = sizeof( char* );
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}


//========================================================================================================
//	jeProperty_FillGroupEnd
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillGroupEnd( jeProperty * pjeProperty, int FieldId )
{
	assert( pjeProperty != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = NULL;
	pjeProperty->Type = PROPERTY_GROUP_END_TYPE;
	pjeProperty->DataSize = PROPERTY_DATA_INVALID;
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillColorPicker
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillColorPicker( jeProperty * pjeProperty, char *Name,  jeVec3d *Vector, int FieldId )
{
	assert( Name != NULL );
	assert( pjeProperty != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_COLOR_PICKER_TYPE;
	pjeProperty->Data.Vector = *Vector;
	pjeProperty->DataSize = sizeof( jeVec3d );
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}
//========================================================================================================
//	jeProperty_FillCombo
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillCombo( jeProperty *pjeProperty, char *Name,  char * Select, int FieldId, int StringN, char **StringList )
{
	assert( Name != NULL );
	assert( pjeProperty != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = 	PROPERTY_COMBO_TYPE	;
	pjeProperty->Data.String = Select;
	pjeProperty->DataSize = sizeof( int );
	pjeProperty->DataId = FieldId;
	pjeProperty->TypeInfo.ComboInfo.StringN = StringN;
	pjeProperty->TypeInfo.ComboInfo.StringList = StringList;

	return( JE_TRUE );
}
//========================================================================================================
//	jeProperty_FillGroup
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillGroup( jeProperty * pjeProperty, char *Name, int FieldId )
{

	assert( Name != NULL );
	assert( pjeProperty != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_GROUP_TYPE;
	pjeProperty->DataSize = PROPERTY_DATA_INVALID;
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillTimeGroup
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillTimeGroup( jeProperty *pjeProperty, char *Name, int FieldId )
{

	assert( Name != NULL );
	assert( pjeProperty != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_TIME_GROUP_TYPE;
	pjeProperty->DataSize = PROPERTY_DATA_INVALID;
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillButton
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillButton( jeProperty *pjeProperty, char *Name, int FieldId )
{

	assert( Name != NULL );
	assert( pjeProperty != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->FieldName = Util_StrDup( Name );
	pjeProperty->Type = PROPERTY_BUTTON_TYPE;
	pjeProperty->DataSize = PROPERTY_DATA_INVALID;
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillVoid
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillVoid( jeProperty *pjeProperty, PROPERTY_FIELD_TYPE Type, void *Pointer, int FieldId )
{
	assert( pjeProperty != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->Type = Type;
	pjeProperty->Data.Ptr = Pointer;
	pjeProperty->DataSize = sizeof( void * );
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}

//========================================================================================================
//	jeProperty_FillCurTime
//========================================================================================================
JETAPI jeBoolean JETCC jeProperty_FillCurTime( jeProperty *pjeProperty, float Time, int FieldId )
{
	assert( pjeProperty != NULL );

	memset( pjeProperty, 0, sizeof( jeProperty ) );
	pjeProperty->Type = PROPERTY_CURTIME_TYPE;
	pjeProperty->Data.Float = Time;
	pjeProperty->DataSize = sizeof( float );
	pjeProperty->DataId = FieldId;
	return( JE_TRUE );
}


//========================================================================================================
//	jeProperty_DataEqual
//========================================================================================================
static jeBoolean jeProperty_DataEqual( jeProperty *pjeProperty, jeProperty *pjeProperty2 )
{
	jeBoolean Result = JE_FALSE;

	if( pjeProperty->Type != pjeProperty2->Type )
		return( JE_FALSE );

	if( pjeProperty->DataSize == PROPERTY_DATA_INVALID )
		return( JE_FALSE );

	if( pjeProperty2->DataSize == PROPERTY_DATA_INVALID )
		return( JE_FALSE );

	switch( pjeProperty->Type )
	{
	case PROPERTY_STRING_TYPE:
	case PROPERTY_COMBO_TYPE:
		Result = !strcmp( pjeProperty->Data.String, pjeProperty2->Data.String);
		break;

	case PROPERTY_INT_TYPE:
		Result = (pjeProperty->Data.Int == pjeProperty2->Data.Int);
		break;

	case PROPERTY_FLOAT_TYPE:
		Result = (pjeProperty->Data.Float == pjeProperty2->Data.Float);
		break;

	case PROPERTY_CHECK_TYPE:
	case PROPERTY_RADIO_TYPE:
		Result = (pjeProperty->Data.Bool == pjeProperty2->Data.Bool);
		break;

	case PROPERTY_VEC3D_GROUP_TYPE:
	case PROPERTY_COLOR_GROUP_TYPE:
		Result = jeVec3d_Compare( &pjeProperty->Data.Vector, &pjeProperty->Data.Vector, 0.0);
		break;

	case PROPERTY_GROUP_TYPE:
	case PROPERTY_GROUP_END_TYPE:
	case PROPERTY_COLOR_PICKER_TYPE:
	case PROPERTY_TIME_GROUP_TYPE:
	case PROPERTY_CHANNEL_POS_TYPE:
	case PROPERTY_CHANNEL_EVENT_TYPE:
	case PROPERTY_CHANNEL_ROT_TYPE:
	case PROPERTY_CURTIME_TYPE:
	case PROPERTY_STATIC_INT_TYPE:
		Result = JE_FALSE;
		break;

	default:
		assert( 0);
	}
	return( Result );
}

//========================================================================================================
//	jeProperty_ListDestroy
//========================================================================================================
JETAPI void JETCC jeProperty_ListDestroy( jeProperty_List **pArray )
{
	int i;

	assert(pArray);
	assert(*pArray);

	if((*pArray)->pjeProperty != NULL )
	{
		for( i = 0; i < (*pArray)->jePropertyN; i++ )
		{
			if((*pArray)->pjeProperty[i].FieldName != NULL )
				jeRam_Free((*pArray)->pjeProperty[i].FieldName );
		}
		jeRam_Free((*pArray)->pjeProperty );
	}

	jeRam_Free(*pArray);

	*pArray = NULL;
}

//========================================================================================================
//	jeProperty_ListMerge
//========================================================================================================
JETAPI jeProperty_List * JETCC jeProperty_ListMerge( jeProperty_List *pArray, jeProperty_List *pArray2, int bSameType )
{
	int i, j;
	int NewArrayCnt = 0;
	int NewArraySize;
	jeProperty_List *pNewArray;
	jeProperty *pjeProperty;
	jeProperty *pjeProperty2;
	jeProperty *pNewjeProperty;

	if( pArray->jePropertyN > pArray2->jePropertyN )
		NewArraySize =  pArray->jePropertyN;
	else
		NewArraySize =  pArray2->jePropertyN;

	pNewArray = jeProperty_ListCreate( NewArraySize );
	if( pNewArray == NULL )
		return( NULL );

	pjeProperty  = pArray->pjeProperty;
	pjeProperty2 = pArray2->pjeProperty;
	pNewjeProperty = pNewArray->pjeProperty;
	for( i = 0;i < pArray->jePropertyN ; i++ )
	{
		for( j = 0; j <  pArray2->jePropertyN ; j++ )
		{
			if( pjeProperty[i].DataId == pjeProperty2[j].DataId )
			{
				if( !bSameType && (pjeProperty[i].DataId >= PROPERTY_LOCAL_DATATYPE_START) )
					break;
				pNewjeProperty[NewArrayCnt] = pjeProperty[i];
				if( pjeProperty[i].FieldName  != NULL )
					pNewjeProperty[NewArrayCnt].FieldName = Util_StrDup( pjeProperty[i].FieldName );
				if( !jeProperty_DataEqual( &pjeProperty[i], &pjeProperty2[j] ) )
					pNewjeProperty[NewArrayCnt].DataSize = PROPERTY_DATA_INVALID;
				NewArrayCnt++;
				break;
			}
		}
	}
	pNewArray->jePropertyN = NewArrayCnt;
	pNewArray->bDirty = pArray->bDirty | pArray2->bDirty ;
	return( pNewArray );

}

//========================================================================================================
//	jeProperty_ListFindByDataId
//========================================================================================================
JETAPI jeProperty * JETCC jeProperty_ListFindByDataId(  jeProperty_List *pArray, int FieldId )
{
	int i;

	for( i = 0; i < pArray->jePropertyN; i++ )
	{
		if( pArray->pjeProperty[i].DataId == FieldId )
			return( &pArray->pjeProperty[i] );
	}
	return( NULL );
}

//========================================================================================================
//	jeProperty_SetDataInvalid
//========================================================================================================
JETAPI void JETCC jeProperty_SetDataInvalid( jeProperty *pjeProperty )
{
	pjeProperty->DataSize = PROPERTY_DATA_INVALID;
}

//========================================================================================================
//	jeProperty_SetDisabled
//========================================================================================================
JETAPI void JETCC jeProperty_SetDisabled( jeProperty *pjeProperty, jeBoolean bDisable )
{
	pjeProperty->bDisabled = bDisable;
}
