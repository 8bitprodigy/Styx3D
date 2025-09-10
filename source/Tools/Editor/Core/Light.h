/****************************************************************************************/
/*  LIGHT.H                                                                             */
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

#ifndef LIGHT_H
#define LIGHT_H

#include "jeWorld.h"
#include "jeLight.h"
#include "defs.h"
#include "Group.h"
#include "jeProperty.h"

typedef struct tagLight Light ;
#define LIGHT_VERSION		(1)
#define LIGHT_INIT_ALL 0xFFFFFFFF

typedef enum 
{
	LIGHT_FIELD_BRIGHTNESS	= (1<<0),
	LIGHT_FIELD_COLOR		= (1<<1),
	LIGHT_FIELD_POS			= (1<<2),
	LIGHT_FIELD_RADIUS		= (1<<3)
} LIGHTINFO_FIELDS;

enum {
	LIGHT_BRIGHTNESS_FIELD = PROPERTY_LOCAL_DATATYPE_START,
	LIGHT_RADIUS_FIELD,
	LIGHT_COLOR_FIELD,
	LIGHT_RED_FIELD,
	LIGHT_GREEN_FIELD,
	LIGHT_BLUE_FIELD,
	LIGHT_PICKER_FIELD,
	LIGHT_COLOR_FIELD_END
};

typedef struct LightInfo {
	jeVec3d Pos; 
	jeVec3d Color; 
	jeFloat Radius; 
	jeFloat Brightness; 
	uint32  Flags;
} LightInfo;

typedef struct LightInfoCB_Struct {
	LightInfo *LightInfo;
	int32		FieldFlag;
} LightInfoCB_Struct;

#ifdef __cplusplus
extern "C" {
#endif

// CREATORS
Light *				Light_Create( const char * const pszName, Group * pGroup, int32 nNumber, jeWorld * pWorld) ;
Light *				Light_Copy( Light *	pLight, int32 nNumber );
void				Light_Destroy( Light ** ppLight ) ;
Light *				Light_FromTemplate( char * pszName, Group * pGroup, Light *	pLight, int32 nNumber, jeBoolean bUpdate );
char  *				Light_CreateDefaultName( void );
Light *				Light_CreateTemplate(  jeWorld * pWorld );

// MODIFIERS
jeBoolean			Light_Move( Light * pLight, const jeVec3d * pWorldDistance ) ;
void				Light_SetIndexTag( Light * pLight, const uint32 nIndex ) ;
void				Light_SetModified( Light * pLight ) ;
void				Light_Snap( Light * pLight, jeFloat fSnapSize ) ;
jeBoolean			Light_SetXForm( Light * pLight, const jeXForm3d * XForm );
void				Light_UpdateBounds( Light * pLight ) ;
jeBoolean			Light_Size( Light * pLight, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis ) ;
jeBoolean			Light_SetInfo( Light * pLight, LightInfo *pLightInfo, int32 BlankFieldFlag );
void				Light_SetIndexTag( Light * pLight, const uint32 nIndex ) ;
void				Light_RemoveFromWorld( Light * pLight );
void				Light_AddToWorld( Light * pLight );
jeBoolean			Light_UpdateData( Light * pLight );
jeProperty_List *	Light_BuildDescriptor( Light * pLight );
void				Light_SetProperty( Light * pLight, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bUpdate );
void				Light_ChangeToDLight( Light * pLight );
void				Light_ChangeFromDLight( Light * pLight );
void				Light_Update( Light * pLight, int Update_Type );

// ACCESSORS
void				Light_GetXForm( const Light * pLight, jeXForm3d * XForm );
const jeExtBox *	Light_GetWorldAxialBounds( const Light * pLight ) ;
void Light_GetWorldDrawBounds( const Light * pLight, jeExtBox *DrawBounds );
void				Light_GetInfo( const Light * pLight, LightInfo *pLightInfo, int32 *BlankFieldFlag );
jeBoolean			Light_SelectClosest( Light * pLight, FindInfo	*	pFindInfo );

// IS
jeBoolean	Light_IsInRect( const Light * pLight, jeExtBox *pSelRect, jeBoolean bSelEncompeses );

// FILE
Light * Light_CreateFromFile( jeVFile * pF, jeWorld * pWorld, jePtrMgr * pPtrMgr );
jeBoolean Light_WriteToFile( Light * pLight, jeVFile * pF, jePtrMgr * pPtrMgr );

//PRESENTATION
void Light_RenderOrtho( const Ortho * pOrtho, Light *pLight, int32 hDC, jeBoolean bColorOveride );

// CALLBACK
jeBoolean Light_ReattachCB( Light * pLight, void* lParam );

//GLOBAL PROPERTIES
jeProperty_List *	Light_GlobalPropertyList();
void				Light_SetGlobalProperty( int DataId, int DataType, jeProperty_Data * pData );

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Light.h */
