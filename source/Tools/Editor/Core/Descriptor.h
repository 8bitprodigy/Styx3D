/****************************************************************************************/
/*  DESCRIPTOR.H                                                                        */
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
#pragma once
#include "Vec3d.h"
#define DESCRIPTOR_LOCAL_DATATYPE_START 1000
#define DESCRIPTOR_DATA_INVALID			0
typedef union FieldData {
	char * String;
	int   Bool;
	float Float;
	int	  Int;
	jeVec3d Vector;
} FieldData;

typedef enum {
	STRING_TYPE,
	INT_STRING_TYPE,
	FLOAT_STRING_TYPE,
	CHECK_TYPE,
	RADIO_TYPE,
	GROUP_TYPE,
	VEC3D_GROUP_TYPE,
	COLOR_GROUP_TYPE,
	GROUP_END_TYPE,
	COLOR_PICKER_TYPE
} DESCRIPTOR_FIELD_TYPE;

typedef struct Descriptor {
	char * FieldName;
	DESCRIPTOR_FIELD_TYPE Type;
	int	DataSize;
	FieldData Data;
	int		  DataId;
	float	  Min;
	float	  Max;
	float	  Increment;
	jeBoolean bDisabled;
} Descriptor;

typedef struct DescriptorArray {
	int	DescriptorN;
	Descriptor * pDescriptor;
} DescriptorArray;

DescriptorArray * DescriptorArray_Create( int FieldN );
DescriptorArray * DescriptorArray_Merge( DescriptorArray *pArray, DescriptorArray *pArray2, int bSameType );
Descriptor *DescriptorArray_FindByDataId(  DescriptorArray *pArray, int FieldId );
void DescriptorArray_Destroy( DescriptorArray *pArray );
void Descriptor_FillGroup( Descriptor *pDescriptor, int NameId, int FieldId );
void Descriptor_FillCheck( Descriptor *pDescriptor, int NameId, int Value, int FieldId );
void Descriptor_FillRaido( Descriptor *pDescriptor, int NameId, int Value, int FieldId );
void Descriptor_FillVec3dGroup( Descriptor *pDescriptor, int NameId, const jeVec3d *Vector, int FieldId );
void Descriptor_FillColorGroup( Descriptor *pDescriptor, int NameId, const jeVec3d *Vector, int FieldId );
void Descriptor_FillFloat( Descriptor *pDescriptor, int NameId, float Float, int FieldId, float Min, float Max, float Increment );
void Descriptor_FillInt( Descriptor *pDescriptor, int NameId, int Int, int FieldId, float Min, float Max, float Increment );
void Descriptor_FillString( Descriptor *pDescriptor, int NameId, char *String, int FieldId );
void Descriptor_FillGroupEnd( Descriptor *pDescriptor, int FieldId);
void Descriptor_FillColorPicker( Descriptor *pDescriptor, int NameId,  jeVec3d *Vector, int FieldId );
void Descriptor_SetDataInvalid( Descriptor *pDescriptor );
void Descriptor_SetDisabled( Descriptor *pDescriptor, jeBoolean bDisable );