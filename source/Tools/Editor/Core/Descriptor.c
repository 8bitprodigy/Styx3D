/****************************************************************************************/
/*  DESCRIPTOR.C                                                                        */
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

#include "Descriptor.h"
#include "ram.h"
#include "errorlog.h"
#include "assert.h"
#include "util.h"
#include <string.h>

DescriptorArray * DescriptorArray_Create( int FieldN )
{
	DescriptorArray * pArray = NULL;
	Descriptor *pDescriptor = NULL;

	pArray = JE_RAM_ALLOCATE_STRUCT( DescriptorArray );
	if( pArray == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "DescriptorArray" );
		return( NULL );
	}

	pArray->DescriptorN =  FieldN;

	pDescriptor = JE_RAM_ALLOCATE_ARRAY_CLEAR( Descriptor, FieldN );
	if( pDescriptor == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Descriptor" );
		jeRam_Free( pArray );
		return( NULL );
	}
	pArray->pDescriptor = pDescriptor;

	return( pArray );
}

void Descriptor_FillCheck( Descriptor *pDescriptor, int NameId, int Value, int FieldId )
{
	pDescriptor->FieldName = Util_LoadLocalRcString( NameId );
	pDescriptor->Type = CHECK_TYPE;
	pDescriptor->Data.Bool = Value;
	pDescriptor->DataSize = sizeof( int );
	pDescriptor->DataId = FieldId;
}

void Descriptor_FillRaido( Descriptor *pDescriptor, int NameId, int Value, int FieldId )
{
	pDescriptor->FieldName = Util_LoadLocalRcString( NameId );
	pDescriptor->Type = RADIO_TYPE;
	pDescriptor->Data.Bool = Value;
	pDescriptor->DataSize = sizeof( int );
	pDescriptor->DataId = FieldId;
}

void Descriptor_FillVec3dGroup( Descriptor *pDescriptor, int NameId, const jeVec3d *Vector, int FieldId )
{
	pDescriptor->FieldName = Util_LoadLocalRcString( NameId );
	pDescriptor->Type = VEC3D_GROUP_TYPE;
	pDescriptor->Data.Vector = *Vector;
	pDescriptor->DataSize = sizeof( jeVec3d );
	pDescriptor->DataId = FieldId;
}

void Descriptor_FillColorGroup( Descriptor *pDescriptor, int NameId, const jeVec3d *Vector, int FieldId )
{
	pDescriptor->FieldName = Util_LoadLocalRcString( NameId );
	pDescriptor->Type = COLOR_GROUP_TYPE;
	pDescriptor->Data.Vector = *Vector;
	pDescriptor->DataSize = sizeof( jeVec3d );
	pDescriptor->DataId = FieldId;
}

void Descriptor_FillFloat( Descriptor *pDescriptor, int NameId, float Float, int FieldId, float Min, float Max, float Increment )
{
	pDescriptor->FieldName = Util_LoadLocalRcString( NameId );
	pDescriptor->Type = FLOAT_STRING_TYPE;
	pDescriptor->Data.Float = Float;
	pDescriptor->DataSize = sizeof( float );
	pDescriptor->DataId = FieldId;
	pDescriptor->Min = Min;
	pDescriptor->Max = Max;
	pDescriptor->Increment = Increment;
}

void Descriptor_FillInt( Descriptor *pDescriptor, int NameId, int Int, int FieldId, float Min, float Max, float Increment )
{
	pDescriptor->FieldName = Util_LoadLocalRcString( NameId );
	pDescriptor->Type = INT_STRING_TYPE;
	pDescriptor->Data.Int = Int;
	pDescriptor->DataSize = sizeof( int );
	pDescriptor->DataId = FieldId;
	pDescriptor->Min = Min;
	pDescriptor->Max = Max;
	pDescriptor->Increment = Increment;
}

void Descriptor_FillString( Descriptor *pDescriptor, int NameId, char *String, int FieldId )
{
	pDescriptor->FieldName = Util_LoadLocalRcString( NameId );
	pDescriptor->Type = STRING_TYPE;
	pDescriptor->Data.String = String;
	pDescriptor->DataSize = sizeof( char* );
	pDescriptor->DataId = FieldId;
}

void Descriptor_FillGroupEnd( Descriptor *pDescriptor, int FieldId )
{
	pDescriptor->FieldName = NULL;
	pDescriptor->Type = GROUP_END_TYPE;
	pDescriptor->DataSize = DESCRIPTOR_DATA_INVALID;
	pDescriptor->DataId = FieldId;
}

void Descriptor_FillColorPicker( Descriptor *pDescriptor, int NameId,  jeVec3d *Vector, int FieldId )
{
	pDescriptor->FieldName = Util_LoadLocalRcString( NameId );
	pDescriptor->Type = COLOR_PICKER_TYPE;
	pDescriptor->Data.Vector = *Vector;
	pDescriptor->DataSize = sizeof( jeVec3d );
	pDescriptor->DataId = FieldId;
}

void Descriptor_FillGroup( Descriptor *pDescriptor, int NameId, int FieldId )
{
	pDescriptor->FieldName = Util_LoadLocalRcString( NameId );
	pDescriptor->Type = GROUP_TYPE;
	pDescriptor->DataSize = DESCRIPTOR_DATA_INVALID;
	pDescriptor->DataId = FieldId;
}

static jeBoolean Descriptor_DataEqual( Descriptor *pDescriptor, Descriptor *pDescriptor2 )
{
	jeBoolean Result = JE_FALSE;

	if( pDescriptor->Type != pDescriptor2->Type )
		return( JE_FALSE );

	if( pDescriptor->DataSize == DESCRIPTOR_DATA_INVALID )
		return( JE_FALSE );

	if( pDescriptor2->DataSize == DESCRIPTOR_DATA_INVALID )
		return( JE_FALSE );

	switch( pDescriptor->Type )
	{
	case STRING_TYPE:
		Result = !strcmp( pDescriptor->Data.String, pDescriptor2->Data.String);
		break;

	case INT_STRING_TYPE:
		Result = (pDescriptor->Data.Int == pDescriptor2->Data.Int);
		break;

	case FLOAT_STRING_TYPE:
		Result = (pDescriptor->Data.Float == pDescriptor2->Data.Float);
		break;

	case CHECK_TYPE:
	case RADIO_TYPE:
		Result = (pDescriptor->Data.Bool == pDescriptor2->Data.Bool);
		break;

	case VEC3D_GROUP_TYPE:
	case COLOR_GROUP_TYPE:
		Result = jeVec3d_Compare( &pDescriptor->Data.Vector, &pDescriptor->Data.Vector, 0.0);
		break;

	case GROUP_TYPE:
	case GROUP_END_TYPE:
	case COLOR_PICKER_TYPE:
		Result = JE_FALSE;
		break;

	default:
		assert( 0);
	}
	return( Result );
}

void DescriptorArray_Destroy( DescriptorArray *pArray )
{
	int i;
	if( pArray->pDescriptor != NULL )
	{
		for( i = 0; i < pArray->DescriptorN; i++ )
		{
			if( pArray->pDescriptor[i].FieldName != NULL )
				jeRam_Free( pArray->pDescriptor[i].FieldName );
		}
		jeRam_Free( pArray->pDescriptor );
	}
	jeRam_Free( pArray );
}

DescriptorArray *DescriptorArray_Merge( DescriptorArray *pArray, DescriptorArray *pArray2, int bSameType )
{
	int i, j;
	int NewArrayCnt = 0;
	int NewArraySize;
	DescriptorArray *pNewArray;
	Descriptor *pDescriptor;
	Descriptor *pDescriptor2;
	Descriptor *pNewDescriptor;

	if( pArray->DescriptorN > pArray2->DescriptorN )
		NewArraySize =  pArray->DescriptorN;
	else
		NewArraySize =  pArray2->DescriptorN;

	pNewArray = DescriptorArray_Create( NewArraySize );
	if( pNewArray == NULL )
		return( NULL );

	pDescriptor  = pArray->pDescriptor;
	pDescriptor2 = pArray2->pDescriptor;
	pNewDescriptor = pNewArray->pDescriptor;
	for( i = 0;i < pArray->DescriptorN ; i++ )
	{
		for( j = 0; j <  pArray2->DescriptorN ; j++ )
		{
			if( pDescriptor[i].DataId == pDescriptor2[j].DataId )
			{
				if( !bSameType && (pDescriptor[i].DataId >= DESCRIPTOR_LOCAL_DATATYPE_START) )
					break;
				pNewDescriptor[NewArrayCnt] = pDescriptor[i];
				if( pDescriptor[i].FieldName  != NULL )
					pNewDescriptor[NewArrayCnt].FieldName = Util_StrDup( pDescriptor[i].FieldName );
				if( !Descriptor_DataEqual( &pDescriptor[i], &pDescriptor2[j] ) )
					pNewDescriptor[NewArrayCnt].DataSize = DESCRIPTOR_DATA_INVALID;
				NewArrayCnt++;
				break;
			}
		}
	}
	pNewArray->DescriptorN = NewArrayCnt;
	DescriptorArray_Destroy( pArray );
	DescriptorArray_Destroy( pArray2 );
	return( pNewArray );

}

Descriptor *DescriptorArray_FindByDataId(  DescriptorArray *pArray, int FieldId )
{
	int i;

	for( i = 0; i < pArray->DescriptorN; i++ )
	{
		if( pArray->pDescriptor[i].DataId == FieldId )
			return( &pArray->pDescriptor[i] );
	}
	return( NULL );
}

void Descriptor_SetDataInvalid( Descriptor *pDescriptor )
{
	pDescriptor->DataSize = DESCRIPTOR_DATA_INVALID;
}

void Descriptor_SetDisabled( Descriptor *pDescriptor, jeBoolean bDisable )
{
	pDescriptor->bDisabled = bDisable;
}
