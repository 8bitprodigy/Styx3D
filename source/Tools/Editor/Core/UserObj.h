/****************************************************************************************/
/*  USEROBJ.H                                                                           */
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

#ifndef USEROBJ_H
#define USEROBJ_H

#include "jeWorld.h"
#include "Object.h"
#include "defs.h"
#include "Group.h"
#include "jePtrMgr.h"

typedef struct tagUserObj UserObj ;

#ifdef __cplusplus
extern "C" {
#endif

// CREATORS
UserObj *			UserObj_Create( const char * const pszName, Group * pGroup, int32 nNumber, jeObject	* pgeObject) ;
UserObj *			UserObj_Copy( UserObj *	pUserObj, int32 nNumber );
void				UserObj_Destroy( UserObj ** ppUserObj ) ;
char  *				UserObj_CreateDefaultName( jeObject	* pgeObject );
char  *				UserObj_CreateKindName( );

// MODIFIERS
jeBoolean			UserObj_Move( UserObj * pUserObj, const jeVec3d * pWorldDistance ) ;
void				UserObj_SetModified( UserObj * pUserObj ) ;
void				UserObj_Snap( UserObj * pUserObj, jeFloat fSnapSize ) ;
jeBoolean			UserObj_SetXForm( UserObj * pUserObj, const jeXForm3d * XForm );
void				UserObj_UpdateBounds( UserObj * pUserObj ) ;
jeBoolean			UserObj_Size( UserObj * pUserObj, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis ) ;
jeBoolean			UserObj_RemoveFromWorld( UserObj * pUserObj, jeWorld * pWorld );
jeBoolean			UserObj_AddToWorld( UserObj * pUserObj, jeWorld * pWorld );
jeBoolean			UserObj_UpdateData( UserObj * pUserObj );
jeProperty_List *	UserObj_BuildDescriptor( UserObj * pUserObj );
void				UserObj_SetProperty( UserObj * pUserObj, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bUpdate );
jeProperty_List *	UserObj_GlobalPropertyList( const char * TypeName );
void				UserObj_SetGlobalProperty( const char * TypeName, int DataId, int DataType, jeProperty_Data * pData );

void				UserObj_Update( UserObj * pUserObj, int Update_Type );
void				UserObj_Rotate( UserObj * pUserObj, ORTHO_AXIS RAxis, jeFloat RadianAngle, const jeVec3d * pRotationCenter );
jeBoolean			UserObj_SendMessage( UserObj * pUserObj, int32 message, void * data );
int32				UserObj_GetXFormModFlag( UserObj * pUserObj );
void				UserObj_Select3d( UserObj* pUserObj, jeVec3d * Front, jeVec3d * Back, jeVec3d * Impact );
#ifdef _USE_BITMAPS
void				UserObj_ApplyMatr( UserObj* pUserObj, jeBitmap * pBitmap );
#else
void				UserObj_ApplyMatr( UserObj* pUserObj, jeMaterialSpec * pMatSpec );
#endif

// ACCESSORS
jeBoolean			UserObj_GetXForm( const UserObj * pUserObj, jeXForm3d * XForm );
jeBoolean			UserObj_GetWorldAxialBounds( const UserObj * pUserObj, jeExtBox * BBox);
jeBoolean			UserObj_GetWorldDrawBounds( const UserObj * pUserObj, jeExtBox *DrawBounds );
jeBoolean			UserObj_SelectClosest( UserObj * pUserObj, FindInfo	*	pFindInfo );
jeObject *			UserObj_GetjeObject( UserObj * pUserObj );

// IS
jeBoolean	UserObj_IsInRect( const UserObj * pUserObj, jeExtBox *pSelRect, jeBoolean bSelEncompeses );

// FILE
UserObj * UserObj_CreateFromFile( jeVFile * pF, jePtrMgr * pPtrMgr );
jeBoolean UserObj_WriteToFile( UserObj * pUserObj, jeVFile * pF, jePtrMgr * pPtrMgr );

//PRESENTATION
void UserObj_RenderOrtho( const Ortho * pOrtho, UserObj *pUserObj, int32 hDC, jeBoolean bColorOveride );


jeBoolean UserObj_AddToObject( UserObj * pUserObj, jeObject * pParent );

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: UserObj.h */
