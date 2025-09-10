/****************************************************************************************/
/*  JEPROPERTY.H                                                                        */
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
// jeProperty.h : header file
//
//

#ifndef JE_PROPERTY_H
#define JE_PROPERTY_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include "Vec3d.h"
#define PROPERTY_LOCAL_DATATYPE_START 1000
#define PROPERTY_DATA_INVALID			0
typedef union jeProperty_Data {
	char * String;
	int   Bool;
	float Float;
	int	  Int;
	void * Ptr;
	jeVec3d Vector;
} jeProperty_Data;

typedef enum {
	PROPERTY_STRING_TYPE,	//Uses no Type Info
	PROPERTY_INT_TYPE,		//Uses NumInfo
	PROPERTY_FLOAT_TYPE,	//Uses NumIn
	PROPERTY_CHECK_TYPE,	//Uses no Type Info
	PROPERTY_RADIO_TYPE,	//Uses no Type Info
	PROPERTY_GROUP_TYPE,	//Uses no Type Info
	PROPERTY_VEC3D_GROUP_TYPE,	//Uses no Type Info
	PROPERTY_COLOR_GROUP_TYPE,	//Uses no Type Info
	PROPERTY_GROUP_END_TYPE,	//Uses no Type Info
	PROPERTY_COLOR_PICKER_TYPE,	//Uses no Type Info
	PROPERTY_BUTTON_TYPE,		//Uses no Type Info
	PROPERTY_COMBO_TYPE	,		//Uses ComboInfo
	PROPERTY_TIME_GROUP_TYPE,	//Uses no Type Info
	PROPERTY_CHANNEL_POS_TYPE,		//Uses no Type Info
	PROPERTY_CHANNEL_EVENT_TYPE,	//Uses no Type Info
	PROPERTY_CHANNEL_ROT_TYPE,		//Uses no Type Info
	PROPERTY_CURTIME_TYPE,			//Uses no Type Info
	PROPERTY_STATIC_INT_TYPE,		//Uses no Type Info
	PROPERTY_LAST
} PROPERTY_FIELD_TYPE;

typedef struct jeProperty_NumInfo {
	float	  Min;
	float	  Max;
	float	  Increment;
} jeProperty_NumInfo;

typedef struct jeProperty_ComboInfo {
	int StringN;
	char ** StringList;
} jeProperty_ComboInfo;

typedef union jeProperty_TypeInfo {
	jeProperty_NumInfo NumInfo;
	jeProperty_ComboInfo ComboInfo;
} jeProperty_TypeInfo;

typedef struct jeProperty {
	char * FieldName;
	PROPERTY_FIELD_TYPE Type;
	int	DataSize;
	jeProperty_Data Data;
	int		  DataId;
	jeBoolean bDisabled;
	jeProperty_TypeInfo TypeInfo;
} jeProperty;

typedef struct jeProperty_List {
	int	jePropertyN;
	jeProperty * pjeProperty;
	jeBoolean	bDirty;			//This is used to comunicate that the list has been changed.
								//It is used by the editor when a property list is built to update data.
								//if the the bDirty is set it rebuild dialog instead of just updating data
} jeProperty_List;

//========================================================================================================
//========================================================================================================
JETAPI jeProperty_List * JETCC jeProperty_ListCreate( int FieldN );
JETAPI jeProperty_List * JETCC jeProperty_ListCreateEmpty();
JETAPI jeProperty_List * JETCC jeProperty_ListCopy( jeProperty_List * pArray);
JETAPI jeProperty_List * JETCC jeProperty_ListConCat( jeProperty_List * pArray, jeProperty_List *pArray2 );
JETAPI jeBoolean JETCC jeProperty_Append( jeProperty_List *pArray, jeProperty *pjeProperty );
JETAPI jeProperty_List * JETCC jeProperty_ListMerge( jeProperty_List *pArray, jeProperty_List *pArray2, int bSameType );
JETAPI jeProperty * JETCC jeProperty_ListFindByDataId(  jeProperty_List *pArray, int FieldId );
JETAPI void JETCC jeProperty_ListDestroy( jeProperty_List **pArray );
JETAPI jeBoolean JETCC jeProperty_FillGroup( jeProperty *pjeProperty, char *Name, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillButton( jeProperty *pjeProperty, char *Name, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillCheck( jeProperty *pjeProperty, char *Name, int Value, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillRadio( jeProperty *pjeProperty, char *Name, int Value, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillVec3dGroup( jeProperty *pjeProperty, char *Name, const jeVec3d *Vector, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillColorGroup( jeProperty *pjeProperty, char *Name, const jeVec3d *Vector, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillTimeGroup( jeProperty *pjeProperty, char *Name, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillFloat( jeProperty *pjeProperty, char *Name, float Float, int FieldId, float Min, float Max, float Increment );
JETAPI jeBoolean JETCC jeProperty_FillInt( jeProperty *pjeProperty, char *Name, int Int, int FieldId, float Min, float Max, float Increment );
JETAPI jeBoolean JETCC jeProperty_FillStaticInt( jeProperty *pjeProperty, char *Name, int Int, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillString( jeProperty *pjeProperty, char *Name, char *String, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillVoid( jeProperty *pjeProperty, PROPERTY_FIELD_TYPE Type, void *Pointer, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillGroupEnd( jeProperty *pjeProperty, int FieldId);
JETAPI jeBoolean JETCC jeProperty_FillColorPicker( jeProperty *pjeProperty, char *Name,  jeVec3d *Vector, int FieldId );
JETAPI jeBoolean JETCC jeProperty_FillCombo( jeProperty *pjeProperty, char *Name,  char * Select, int FieldId, int StringN, char **StringList );
JETAPI jeBoolean JETCC jeProperty_FillCurTime( jeProperty *pjeProperty, float Time, int FieldId );
JETAPI void JETCC jeProperty_SetDataInvalid( jeProperty *pjeProperty );
JETAPI void JETCC jeProperty_SetDisabled( jeProperty *pjeProperty, jeBoolean bDisable );

#ifdef __cplusplus
}
#endif

#endif 
