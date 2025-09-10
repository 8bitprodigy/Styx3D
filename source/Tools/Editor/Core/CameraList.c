/****************************************************************************************/
/*  CAMERALIST.C                                                                        */
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

// CameraList contains a list to all the lights of a level as
// well a conatainer of all data needed to manupulate the lights.
// It also holds the Templete Camera which is Dummy light structure
// which has not been allocated to the world light array.

#include <Assert.h>

#include "ErrorLog.h"
#include "Util.h"
#include "ram.h"

#include "CameraList.h"

typedef struct CameraList{
	List			*	pList;
} CameraList;

static void CameraList_DestroyCameraCB( void *p1 )
{
	Camera * pCamera = (Camera*)p1 ;
	assert( pCamera != NULL ) ;

	Object_Free( (Object**)&pCamera ) ;
}// CameraList_DestroyCameraCB

static jeBoolean CameraList_FindCB( void *p1, void *lParam )
{
	return ( p1 == lParam ) ;
}// CameraList_FindCB



CameraList * CameraList_Create(  )
{
	CameraList *pCameraList;

	pCameraList = JE_RAM_ALLOCATE_STRUCT( CameraList );
	if( pCameraList == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate CameraList" );
		return( NULL );
	}
	pCameraList->pList = List_Create( );
	if( pCameraList->pList == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeRam_Free( pCameraList );
		return( NULL );
	}

	return(pCameraList) ;
}// CameraList_Create


void CameraList_Destroy( CameraList **ppCameraList )
{
	List_Destroy( &(*ppCameraList)->pList, CameraList_DestroyCameraCB ) ;
	jeRam_Free( (*ppCameraList) );
}// CameraList_Destroy

//
// ACCESSORS
//
int32 CameraList_GetNumItems( const CameraList * pCameraList )
{
	assert( pCameraList != NULL ) ;

	return List_GetNumItems( pCameraList->pList ) ;
}// CameraList_GetNumItems

Camera * CameraList_GetFirst( CameraList * pCameraList, CameraIterator	*pBI )
{
	 ;
	assert( pCameraList != NULL ) ;

	return (Camera*)List_GetFirst( pCameraList->pList, pBI ) ;
}// CameraList_GetFirst

Camera * CameraList_GetNext( CameraList * pCameraList, CameraIterator	*pBI )
{
	 ;
	assert( pCameraList != NULL ) ;

	return (Camera*)List_GetNext( pCameraList->pList, pBI ) ;
}// CameraList_GetFirst


CameraIterator CameraList_Find( CameraList * pCameraList, Camera * pCamera )
{
	CameraIterator	pBI ;
	Camera	*		pFoundCamera ;

	assert( pCameraList != NULL ) ;

	List_Search( pCameraList->pList, CameraList_FindCB, pCamera, &pFoundCamera, &pBI ) ;
	return pBI ;

}// CameraList_Find

// MODIFIERS
CameraIterator CameraList_Append( CameraList * pCameraList, Camera * pCamera )
{
	assert( pCameraList != NULL ) ;

	Object_AddRef( (Object*)pCamera );
	return List_Append( pCameraList->pList, pCamera ) ;
}// CameraList_Append

void CameraList_Remove( CameraList * pCameraList, Camera * pCamera )
{
	CameraIterator	pBI ;
	jeBoolean		bFound ;
	Camera	*		pFoundCamera ;

	assert( pCameraList != NULL ) ;

	bFound = List_Search( pCameraList->pList, CameraList_FindCB, pCamera, &pFoundCamera, &pBI ) ;
	assert( JE_TRUE == bFound ) ;

	List_Remove( pCameraList->pList, pBI, NULL ) ;
}// CameraList_Remove

void CameraList_DeleteCamera( CameraList * pCameraList, Camera * pCamera )
{
	CameraList_Remove( pCameraList, pCamera );
	Object_Free( (Object**)&pCamera );
}// CameraList_DeleteCamera

// ENUMERATION

int32 CameraList_EnumCameras( CameraList * pCameraList, void * pVoid, CameraListCB Callback )
{
	assert( pCameraList != NULL ) ;

	return List_ForEach( pCameraList->pList, Callback, pVoid ) ;

}// CameraList_EnumCameraes



CameraList * CameraList_CreateFromFile( jeVFile * pF, jePtrMgr *pPtrMgr  )
{
	CameraList	*	pCameraList = NULL ;
	Camera		*	pCamera ;
	int32			i ;
	int32			nItems ;
	int32			nVersion ;
	assert( jeVFile_IsValid( pF ) ) ;

	if( !jeVFile_Read( pF, &nVersion, sizeof nVersion ) )
		return NULL ;
	if( nVersion != CAMERA_VERSION )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushList_CreateFromFile Version.\n", NULL);
		return NULL ;
	}

	if( !jeVFile_Read( pF, &nItems, sizeof nItems ) )
		return NULL ;

	pCameraList = CameraList_Create(  ) ;
	if( pCameraList == NULL )
	{
		jeErrorLog_AddString(JE_ERR_INTERNAL_RESOURCE, "Cameraist_CreateFromFile\n", NULL);
		return NULL ;
	}

	for( i=0; i<nItems; i++ )
	{
		pCamera = Camera_CreateFromFile( pF, pPtrMgr ) ;
		if( pCamera == NULL )
		{
			jeErrorLog_AddString(JE_ERR_INTERNAL_RESOURCE, "Cameraist_CreateFromFile\n", NULL);
			return NULL ;
		}
		if( CameraList_Append( pCameraList, pCamera ) == NULL )
		{	
			Camera_Destroy( &pCamera ) ;
			return NULL ;
		}
		Object_Free( (Object**)&pCamera );
	}
	return pCameraList ;

}// CameraList_CreateFromFile



jeBoolean CameraList_WriteToFile( CameraList * pList, jeVFile * pF, jePtrMgr *pPtrMgr )

{
	int32	nVersion ;
	int32	nItems ;
	Camera	* pCamera;
	ListIterator pli;

	assert( pList != NULL ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	nVersion = CAMERA_VERSION ;
	if( jeVFile_Write( pF, &nVersion, sizeof nVersion ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushList_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	
	nItems = CameraList_GetNumItems( pList ) ;
	if( jeVFile_Write( pF, &nItems, sizeof nItems ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushList_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	
	pCamera = (Camera	*)List_GetFirst (pList->pList, &pli);
	while( pCamera )
	{
		if( !Camera_WriteToFile( pCamera, pF, pPtrMgr ) )
		{
			jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Camera_WriteToFile.\n", NULL);
			return JE_FALSE;
		}

		pCamera = (Camera*)List_GetNext( pList->pList, &pli );
	}
	return  JE_TRUE;

}// CameraList_WriteToFile


/* EOF: CameraList.h */
