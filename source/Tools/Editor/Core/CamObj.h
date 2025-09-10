/****************************************************************************************/
/*  CAMOBJ.H                                                                            */
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

#ifndef CAMERA_H
#define CAMERA_H

#include "jeWorld.h"
#include "defs.h"
#include "Group.h"
#include "Descriptor.h"
#include "jwObject.h"

typedef struct tagCamera Camera ;
#define CAMERA_VERSION		(1)
#define CAMERA_INIT_ALL 0xFFFFFFFF


#ifdef __cplusplus
extern "C" {
#endif

// CREATORS
Camera *			Camera_Create( const char * const pszName, Group * pGroup, int32 nNumber) ;
Camera *			Camera_Copy( Camera *	pCamera, int32 nNumber );
void				Camera_Destroy( Camera ** ppCamera ) ;
Camera *			Camera_FromTemplate( char * pszName, Group * pGroup, Camera *	pCamera, int32 nNumber );
char  *				Camera_CreateDefaultName( void );
Camera *			Camera_CreateTemplate(  );

// MODIFIERS
jeBoolean			Camera_Move( Camera * pCamera, const jeVec3d * pWorldDistance ) ;
void				Camera_Rotate( Camera * pCamera, ORTHO_AXIS RAxis, jeFloat RadianAngle, const jeVec3d * pRotationCenter );
void				Camera_SetIndexTag( Camera * pCamera, const uint32 nIndex ) ;
void				Camera_SetModified( Camera * pCamera ) ;
void				Camera_Snap( Camera * pCamera, jeFloat fSnapSize ) ;
jeBoolean			Camera_SetXForm( Camera * pCamera, const jeXForm3d * XForm );
void				Camera_UpdateBounds( Camera * pCamera ) ;
jeBoolean			Camera_Size( Camera * pCamera, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis ) ;
void				Camera_SetIndexTag( Camera * pCamera, const uint32 nIndex ) ;
void				Camera_AddToWorld( Camera * pCamera );
jeBoolean			Camera_UpdateData( Camera * pCamera );
jeProperty_List *	Camera_BuildDescriptor( Camera * pCamera );
void				Camera_SetProperty( Camera * pCamera, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bUpdate );
jeBoolean			Camera_TranslateCurCam( Camera * pCamera, jeVec3d * Offset );
jeBoolean			Camera_RotCurCamY( Camera * pCamera, float Radians );
jeBoolean			Camera_RotCurCamX( Camera * pCamera, float Radians );
void				Camera_SetCurCamY( Camera * pCamera, float YRot );
void				Camera_SetCurCamX( Camera * pCamera, float XRot );

// ACCESSORS
void				Camera_GetXForm( const Camera * pCamera, jeXForm3d * XForm );
const jeExtBox *	Camera_GetWorldAxialBounds( const Camera * pCamera ) ;
void				Camera_GetWorldDrawBounds( const Camera * pCamera, jeExtBox *DrawBounds );
jeBoolean			Camera_SelectClosest( Camera * pCamera, FindInfo *	pFindInfo );
jeObject	*		Camera_GetjeObject( Camera * pCamera );
float				Camera_GetFOV( Camera * pCamera );
float				Camera_GetCurCamY( const Camera * pCamera ) ;
float				Camera_GetCurCamX( const Camera * pCamera ) ;

// IS
jeBoolean		Camera_IsInRect( const Camera * pCamera, jeExtBox *pSelRect, jeBoolean bSelEncompeses );

jeBoolean		Camera_WriteToFile( Camera * pCamera, jeVFile * pF, jePtrMgr *PtrMgr );
Camera *		Camera_CreateFromFile( jeVFile * pF, jePtrMgr *PtrMgr );

//PRESENTATION
void Camera_RenderOrtho( const Ortho * pOrtho, Camera *pCamera, int32 hDC, jeBoolean bColorOveride );

// CALLBACK
jeBoolean		Camera_ReattachCB( Camera * pCamera, void* lParam );

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Camera.h */
