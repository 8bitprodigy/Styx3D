/****************************************************************************************/
/*  CAMERALIST.H                                                                        */
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

#ifndef CAMERALIST_H
#define CAMERALIST_H

#include "CamObj.h"
#include "jeWorld.h"
#include "jeList.h"
#include "VFile.h"
#include "jePtrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef jeBoolean (*CameraListCB)( Camera *pCamera, void * pVoid ) ;

typedef struct		CameraList		CameraList;

typedef ListIterator CameraIterator ;
typedef List_DestroyCallback CameraList_DestroyCallback ;

CameraList *		CameraList_Create(  ) ;
void			CameraList_Destroy( CameraList **ppList ) ;

// ACCESSORS
Camera *			CameraList_GetFirst( CameraList * pCameraList, CameraIterator	*pBI );
Camera *			CameraList_GetNext( CameraList * pCameraList, CameraIterator	*pBI );
int32			CameraList_GetNumItems( const CameraList * pList ) ;
CameraIterator	CameraList_Find( CameraList * pList, Camera * pCamera ) ;

// MODIFIERS
CameraIterator		CameraList_Append( CameraList * pList, Camera * pCamera ) ;
//					Remove does not _Destroy the brush
void				CameraList_Remove( CameraList * pList, Camera * pCamera ) ;
void				CameraList_DeleteCamera( CameraList * pList, Camera * pCamera ) ;

// ENUMERATION
int32				CameraList_EnumCameras( CameraList * pList, void * pVoid, CameraListCB Callback ) ;

// FILE HANDLING
CameraList *			CameraList_CreateFromFile( jeVFile * pF, jePtrMgr *pPtrMgr  ) ;
jeBoolean			CameraList_WriteToFile( CameraList * pList, jeVFile * pF, jePtrMgr *pPtrMgr ) ;

#ifdef __cplusplus
}
#endif






#endif // Prevent multiple inclusion
/* EOF: CameraList.h */