/****************************************************************************************/
/*  MODELLIST.C                                                                         */
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

// ModelList is a thin wrapper on List, a linked-list module.
// It provides casting and some simple validation

#include <Assert.h>

#include "ErrorLog.h"
#include "jeModel.h"

#include "ModelList.h"



// STATIC 
static void ModelList_DestroyModelCB( void *p1 )
{
	Model * pModel = (Model*)p1 ;
	assert( pModel != NULL ) ;

	Object_Free( (Object**)&pModel ) ;
}// ModelList_DestroyModelCB

static jeBoolean ModelList_WriteCB( Model * pModel, void* lParam )
{
	return Model_WriteToFile( pModel, (Brush_WriteInfo*)lParam) ;
}// ModelList_WriteCB


// END STATIC

ModelList * ModelList_Create( void )
{
	return (ModelList*)List_Create( ) ;
}// ModelList_Create

void ModelList_Destroy( ModelList **ppList, ModelList_DestroyCallback DestroyFcn )
{
	List_Destroy( (List**)ppList, DestroyFcn ) ;
}// ModelList_Destroy

// ACCESSORS

int32 ModelList_GetNumItems( const ModelList * pList )
{
	assert( pList != NULL ) ;

	return List_GetNumItems( pList ) ;
}// ModelList_GetNumItems

Model * ModelList_GetModel( ModelList * pList, ModelIterator * pMI )
{
//	Model	*	pModel ;
	assert( pMI != NULL ) ;

//	pModel = List_GetData( pMI ) ;
//	assert( JE_TRUE == Model_IsValid( pModel ) ) ;
	return (Model*)List_GetFirst( (List*)pList, pMI ) ;

}// ModelList_GetModel


Model * ModelList_GetFirst( ModelList * pList, ModelIterator * pMI )
{
	Model	* pModel ;

	assert( pList != NULL ) ;
	assert( pMI != NULL ) ;

	pModel = (Model*)List_GetFirst( pList, pMI ) ;
	return pModel ;
}// ModelList_GetFirst

Model * ModelList_GetNext( ModelList * pList, ModelIterator * pMI )
{
	Model	* pModel ;
	assert( pList != NULL ) ;
	assert( pMI != NULL ) ;

	pModel = (Model*)List_GetNext( pList, pMI ) ;
	return pModel ;
}// ModelList_GetNextID


// MODIFIERS
ModelIterator ModelList_Append( ModelList * pList, Model * pModel )
{
	assert( pList != NULL ) ;
	assert( JE_TRUE == Model_IsValid( pModel ) ) ;

	Object_AddRef( (Object*)pModel );
	return List_Append( pList, pModel ) ;
}// ModelList_Append

static jeBoolean ModelList_FindCB( void *p1, void *lParam )
{
	return ( p1 == lParam ) ;
}// ObjectList_FindCB

void ModelList_Remove( ModelList * pList, Model * pModel ) 
{
	ModelIterator	pMI ;
	jeBoolean		bFound ;
	Model	*		pFoundModel ;

	assert( pList != NULL ) ;

	bFound = List_Search( pList, ModelList_FindCB, pModel, &pFoundModel, &pMI ) ;
	assert( JE_TRUE == bFound ) ;

	List_Remove( pList, pMI, NULL ) ;
	Object_Free( (Object**)&pModel );

}

// ENUMERATION

int32 ModelList_EnumModels( ModelList * pList, void * pVoid, ModelListCB Callback )
{
	assert( pList != NULL ) ;

	return List_ForEach( pList, Callback, pVoid ) ;

}// ModelList_EnumModels


typedef struct BrushEnumData {
	void* pVoid;
	BrushListCB Callback;
} BrushEnumData;

static jeBoolean ModelList_BrushEnumCB( Model* pModel, void * pVoid )
{
	BrushEnumData *pData = (BrushEnumData*)pVoid;

	assert( pModel );
	assert( pData );

	return( Model_EnumBrushes( pModel, pData->pVoid, pData->Callback ) );
}

int32 ModelList_EnumBrushes( ModelList * pList, void * pVoid, BrushListCB Callback )
{
	BrushEnumData Data;

	Data.pVoid = pVoid;
	Data.Callback = Callback;

	return( ModelList_EnumModels( pList, &Data, ModelList_BrushEnumCB ) );
}

// CALLBACKS

jeBoolean ModelList_NumberModelsCB( Model * pModel, void * lParam )
{
	int32 *Counter = (int32*)lParam;

	*Counter += 1;
	return JE_TRUE ;
	pModel;
}// ModelList_NumberModelsCB


// FILE HANDLING
ModelList * ModelList_CreateFromFile( jeVFile * pF, jePtrMgr * pPtrMgr )
{
	ModelList	*	pList = NULL ;
	Model		*	pModel ;
	int32			i ;
	int32			nItems ;
	int32			nVersion ;
	assert( jeVFile_IsValid( pF ) ) ;

	if( !jeVFile_Read( pF, &nVersion, sizeof nVersion ) )
		return NULL ;
	if( nVersion != MODEL_VERSION )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "ModelList_CreateFromFile Version.\n", NULL);
		return NULL ;
	}

	if( !jeVFile_Read( pF, &nItems, sizeof nItems ) )
		return NULL ;

	pList = ModelList_Create( ) ;
	if( pList == NULL )
		goto MLCFF_FAILURE ;

	for( i=0; i<nItems; i++ )
	{
		pModel = Model_CreateFromFile( pF, nVersion, pPtrMgr ) ;
		if( pModel == NULL )
			goto MLCFF_FAILURE ;
		
		if( ModelList_Append( pList, pModel ) == NULL )
		{	
			Model_Destroy( &pModel ) ;
			goto MLCFF_FAILURE ;
		}
		Object_Free((Object**) &pModel ); // Icestorm: We created the model, send it to pList, then we MUST DESTROY it
	}
	return pList ;

MLCFF_FAILURE :
	if( pList != NULL )
		ModelList_Destroy( &pList, ModelList_DestroyModelCB ) ;
	return NULL ;

}// ModelList_CreateFromFile


jeBoolean ModelList_WriteToFile( ModelList * pList, jeVFile * pF, jePtrMgr * pPtrMgr )
{
	int32	nVersion ;
	int32	nItems ;
	Brush_WriteInfo WriteInfo;

	// !!FRANK - CREATE A PTRMGR HERE?

	assert( pList != NULL ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	nVersion = MODEL_VERSION ;
	if( jeVFile_Write( pF, &nVersion, sizeof nVersion ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "ModelList_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	
	nItems = ModelList_GetNumItems( pList ) ;
	if( jeVFile_Write( pF, &nItems, sizeof nItems ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "ModelList_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	
	WriteInfo.pF = pF;
	WriteInfo.pPtrMgr = pPtrMgr;

	return ModelList_EnumModels( pList, &WriteInfo, ModelList_WriteCB ) ;

}// ModelList_WriteToFile

jeBoolean ModelList_Reattach( ModelList * pList, jeWorld * pWorld )
{
	Model * pModel;
	ListIterator pli;
	ModelReattachInfo	mri ;
	assert( pList != NULL ) ;
	assert( pWorld != NULL ) ;
	
	pModel = (Model*)List_GetFirst (pList, &pli );
	mri.pWorld = pWorld;
	while( pModel != NULL )
	{
		mri.pModel = Model_GetguModel( pModel ) ;
	//	mri.IndexTag = jeModel_GetIndexTag( pModel ) ;
		ModelList_EnumModels( pList, &mri, Model_ReattachCB ) ;
		assert( mri.IndexTag == MODEL_REATTACH_GOOD ) ;
		pModel = (Model*)List_GetNext(pList, &pli );
	}
	return JE_TRUE ;
}// ModelList_Reattach

/* EOF: ModelList.c */