/****************************************************************************************/
/*  MODEL.C                                                                             */
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

#include <Assert.h>
#include <Memory.h>
#include <String.h>
#include <float.h>

#include "ErrorLog.h"
#include "Ram.h"
#include "Util.h"

#include "Model.h"
#include "ObjectDef.h"
#include "BrushList.h"
#include "ModelInstance.h"
#include "../resource.h"

#define MODEL_SIGNATURE	(0xECA89BDF)

typedef struct tagModel
{
	Object			ObjectData ;
	BrushList	*	pBrushes ;
#ifdef _DEBUG
	int				nSignature ;
#endif
	jeBoolean		bVisible ;
	jeBoolean		bLocked;
	uint32			nIndexTag ;		/* Used only during load */
	ModelID			nID ;
	jeObject	*	pgeObject;
	ModelInstance		*	pInstance ;
    jeXForm3d       XForm;  /* Krouer: copy the XForm here to improve perf */
} Model ;


Model * Model_Create( Group * pGroup, const char * const pszName, int32 nNumber)
{
	Model* pModel = NULL;
	char* NameTag = NULL;
	pModel = JE_RAM_ALLOCATE_STRUCT( Model ) ;
	if( pModel == NULL )
		goto MC_FAILURE ;

    jeXForm3d_SetIdentity(&pModel->XForm);

	memset( pModel, 0, sizeof *pModel ) ;
	assert( MODEL_SIGNATURE == (pModel->nSignature = MODEL_SIGNATURE) ) ;	// ASSIGN

	Object_Init( &pModel->ObjectData, pGroup, KIND_MODEL, pszName, nNumber );
	pModel->pBrushes	= BrushList_Create( ) ;
	pModel->pgeObject = jeObject_Create( "Model" );
	if( pModel->pgeObject == NULL )
		goto MC_FAILURE ;

	NameTag = Object_GetNameAndTag( &pModel->ObjectData );
	if( NameTag == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Model_Create:Object_GetNameAndTag");
		goto MC_FAILURE;
	}
	jeObject_SetName( pModel->pgeObject, NameTag );
	jeRam_Free( NameTag );
	pModel->pInstance = jeObject_GetInstance(pModel->pgeObject) ;
	if( pModel->pInstance == NULL )
		goto MC_FAILURE ;
	jeObject_CreateInstanceRef(pModel->pgeObject);
    jeObject_GetXForm(pModel->pgeObject, &pModel->XForm);

	return pModel ;
	
MC_FAILURE :
	if( pModel->pInstance != NULL && pModel->pgeObject != NULL )
		jeObject_Free( pModel->pgeObject ) ;
	if( pModel->pgeObject != NULL )
		jeObject_Destroy( &pModel->pgeObject ) ;
	if( (pModel)->pBrushes != NULL )
		BrushList_Destroy( &(pModel)->pBrushes, NULL ) ;

	if( pModel != NULL )
		jeRam_Free( pModel );
	return NULL ;
}// Model_Create

static void Model_DestroyBrushCB( void *p1 )
{
	Brush * pBrush = (Brush*)p1 ;
	assert( pBrush != NULL ) ;

	Brush_SetModel( pBrush, NULL ) ;
	Object_SetInLevel( (Object*)pBrush, JE_FALSE );
	Object_Free( (Object**)&pBrush ) ;
}// DestroyBrushCB

void Model_FreeBrushList( Model * pModel )
{
	Brush *	pBrush;
	BrushIterator Interator;

	pBrush = BrushList_GetFirst( pModel->pBrushes, &Interator );
	while( pBrush )
	{
		Brush_SetModel( pBrush, NULL ) ;
		Object_SetInLevel( (Object*)pBrush, JE_FALSE );
		pBrush = BrushList_GetNext( pModel->pBrushes, &Interator );
	}
}

void	Model_RestoreBrush( Model * pModel)
{
	Brush *	pBrush;
	BrushIterator Interator;

	pBrush = BrushList_GetFirst( pModel->pBrushes, &Interator );
	while( pBrush )
	{
		Brush_SetModel( pBrush, pModel ) ;
		Object_SetInLevel( (Object*)pBrush, JE_TRUE );
		pBrush = BrushList_GetNext( pModel->pBrushes, &Interator );
	}
}


void Model_Destroy( Model ** ppModel )
{
	Model * pModel ;
	assert( ppModel != NULL ) ;
	pModel = *ppModel ;
	assert( pModel != NULL ) ;
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;


	if( (*ppModel)->pBrushes != NULL )
		BrushList_Destroy( &(*ppModel)->pBrushes, Model_DestroyBrushCB ) ;

	if( (*ppModel)->pInstance != NULL && (*ppModel)->pgeObject != NULL )
		jeObject_Free( (*ppModel)->pgeObject ) ;

	if( (*ppModel)->pgeObject != NULL )
		jeObject_Destroy( &(*ppModel)->pgeObject ) ;

	assert( ((*ppModel)->nSignature = 0) == 0 ) ;	// CLEAR
	

	jeRam_Free( *ppModel ) ;

}// Model_Destroy

char  *			Model_CreateDefaultName( void )
{
		return( Util_LoadLocalRcString( IDS_TAB_MODELS ) );
}
// ACCESSORS

jeModel * Model_GetguModel( Model * pModel )
{
	assert( pModel != NULL ) ;
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;
	assert( pModel->pInstance != NULL ) ;

	return pModel->pInstance->pModel ;
}// Model_GetguModel

Brush *	Model_FindBrush( Model * pModel, jeBrush *pgeBrush )
{
	BrushIterator Interator;

	return( BrushList_FindByGeBrush( pModel->pBrushes, &Interator, pgeBrush ) );
}

jeBoolean Model_IsLocked( Model * pModel )
{
	assert( pModel != NULL ) ;
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;

	return( pModel->bLocked );
}

ModelID Model_GetID( const Model * pModel )
{
	assert( pModel != NULL ) ;
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;

	return pModel->nID ;
}// Model_GetID



// IS 

// MODIFIERS
jeBoolean Model_AddBrush( Model * pModel, Brush * pBrush )
{
	jeXForm3d XF;
	jeXForm3d ModelXF;

	assert( pModel != NULL ) ;
	assert( pModel->pBrushes != NULL );
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;
	assert( Brush_IsValid( pBrush ) ) ;	
	assert( !Brush_IsInModel( pBrush ) );

	if( BrushList_GetNumItems( pModel->pBrushes) == 0 )
	{
		Brush_GetXForm( pBrush, &XF );
		jeXForm3d_SetIdentity( &ModelXF );
		ModelXF.Translation = XF.Translation;
		Model_SetXForm( pModel, &ModelXF );
	}
	BrushList_Append( pModel->pBrushes, pBrush ) ;
	return JE_TRUE ;
}// Model_AddBrush

void Model_RemoveBrush( Model * pModel, Brush * pBrush )
{
	jeXForm3d XF;

	assert( pModel != NULL ) ;
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;
	assert( Brush_IsValid( pBrush ) ) ;	
	assert( Brush_IsInModel( pBrush ) ) ;
	
	Brush_GetXForm( pBrush, &XF );
	BrushList_Remove( pModel->pBrushes, pBrush ) ;
	Brush_SetModel( pBrush, NULL ) ;
	Brush_SetXForm( pBrush, &XF );
	jeModel_RemoveBrush( pModel->pInstance->pModel, Brush_GetjeBrush(pBrush), JE_TRUE ) ;

}// Model_RemoveBrush

jeBoolean Model_AddBrushWorld( Model * pModel, Brush * pBrush, jeBoolean bBrushUpdate, jeBoolean bLightUpdate )
{
	jeBoolean	b ;
	jeXForm3d XF;
	assert( pModel != NULL ) ;
	assert( pModel->pBrushes != NULL );
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;
	assert( Brush_IsValid( pBrush ) ) ;	

	Brush_GetXForm( pBrush, &XF );
	Brush_SetModel( pBrush, pModel ) ;	// 	Also sets inmodel flag
	Brush_SetXForm( pBrush, &XF );
	b = jeModel_AddBrush( pModel->pInstance->pModel, Brush_GetjeBrush(pBrush), bBrushUpdate, bLightUpdate ) ;
	return b ;
}// Model_AddBrush

void Model_RemoveBrushWorld( Model * pModel, Brush * pBrush )
{
	assert( pModel != NULL ) ;
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;
	assert( Brush_IsValid( pBrush ) ) ;	
	assert( Brush_IsInModel( pBrush ) ) ;
	
	jeModel_RemoveBrush( pModel->pInstance->pModel, Brush_GetjeBrush(pBrush), JE_TRUE ) ;

}// Model_RemoveBrush


void Model_Move( Model * pModel, const jeVec3d * pWorldDistance )
{
	jeXForm3d XF;

	assert( pModel );
	assert( pWorldDistance );
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;

	Model_GetXForm(pModel, &XF);
	jeVec3d_Add( &XF.Translation, pWorldDistance, &XF.Translation );
	Model_SetXForm(pModel, &XF);
}

void Model_Rotate( Model * pModel, ORTHO_AXIS RAxis, jeFloat RadianAngle, const jeVec3d * pRotationCenter )
{
	jeXForm3d	XForm ;
	jeXForm3d	OrgXForm;
	int			ModFlags;
	assert( pModel != NULL ) ;
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;
	
	if( !Model_GetXForm( pModel, &OrgXForm ) )
		return;

	XForm = OrgXForm;
	ModFlags = jeObject_GetXFormModFlags( pModel->pgeObject );

	//If it cant be translated or rotated return.
	if( (ModFlags & ( JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_ROTATE)) == 0 )
		return;
	jeXForm3d_Translate( &XForm, -pRotationCenter->X, -pRotationCenter->Y, -pRotationCenter->Z ) ;
	switch( RAxis )
	{
	case Ortho_Axis_X :
		jeXForm3d_RotateX( &XForm, RadianAngle ) ;	break ;
	case Ortho_Axis_Y :
		jeXForm3d_RotateY( &XForm, RadianAngle ) ;	break ;
	case Ortho_Axis_Z :
		jeXForm3d_RotateZ( &XForm, RadianAngle ) ;	break ;
	}
	jeXForm3d_Translate( &XForm, pRotationCenter->X, pRotationCenter->Y, pRotationCenter->Z ) ; 

	// If cant be rotated then just translate it.
	if( !(ModFlags & JE_OBJECT_XFORM_ROTATE ) )
	{
		OrgXForm.Translation = XForm.Translation;
		XForm = OrgXForm;
	}
	Model_SetXForm( pModel, &XForm) ;


}// Model_Rotate

jeBoolean Model_DirtyBrushBoundsCB( Brush * pBrush, void * Context )
{
	assert( pBrush );
	Context;

	Brush_SetModified( pBrush );
	return( JE_TRUE );
}

void Model_SetXForm( Model * pModel, const jeXForm3d * pXForm )
{
	assert( pModel );
	assert( pXForm );
	assert( pModel->pgeObject );
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;

    pModel->XForm = *pXForm;
	jeObject_SetXForm(pModel->pgeObject, pXForm);
	BrushList_EnumBrushes( pModel->pBrushes, NULL, Model_DirtyBrushBoundsCB );
}
void Model_SetLocked( Model * pModel, jeBoolean bLocked )
{
	pModel->bLocked = bLocked;
}

jeBoolean	Model_GetXForm( const Model * pModel, jeXForm3d * pXForm )
{
	assert( pModel );
	assert( pXForm );
	//assert( pModel->pgeObject );
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;

    *pXForm = pModel->XForm;
    return JE_TRUE;
	
	//return( jeObject_GetXForm(pModel->pgeObject, pXForm) );
}

// CALLBACK
jeBoolean Model_ReattachCB( Model * pModel, void * lParam )
{
	ModelReattachInfo * pmri = (ModelReattachInfo*)lParam ;

	//if( pModel->nIndexTag == pmri->IndexTag )
	{
		pmri->IndexTag = MODEL_REATTACH_GOOD ;

		BrushList_Reattach( pModel->pBrushes, pModel, pmri->pWorld ) ;

		// Icestorm: Workaround
		//  Make sure the models will be attached to the world,
		//  or external arrays could be NULL => Assertion errors...
		//  (This isn't a good solution, but where can I get its ModelObject?
		//  It seems that jeModels are saved/loaded and not ModelObjects!)
		{
			jeFaceInfo_Array	*FArray;
			jeMaterial_Array	*MArray;
			jeChain				*LChain;
			jeChain				*DLChain;

			FArray = jeWorld_GetFaceInfoArray(pmri->pWorld);
			assert(FArray);

			MArray = jeWorld_GetMaterialArray(pmri->pWorld);
			assert(MArray);

			LChain = jeWorld_GetLightChain(pmri->pWorld);
			assert(LChain);

			DLChain = jeWorld_GetDLightChain(pmri->pWorld);
			assert(DLChain);

			jeModel_SetArrays(Model_GetguModel(pModel), FArray, MArray, LChain, DLChain);
		} // EOF Icestorm

	}
	return JE_TRUE ;
}// Model_ReattachCB


void Model_Size( Model * pModel, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis )
{
	return;
	pModel;
	pSelectedBounds;
	hScale;
	vScale;
	eSizeType;
	HAxis;
	VAxis;
}

jeBoolean Model_GetWorldAxialBounds( const Model * pModel, jeExtBox *pExtBox )
{
	assert( pModel );
	assert( pExtBox );
	assert( pModel->pgeObject );
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;

	return( jeObject_GetExtBox( pModel->pgeObject, pExtBox ) );
}

jeObject	*	Model_GetjeObject( const Model * pModel )
{
	assert( pModel );
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;

	return( pModel->pgeObject );
}

jeBoolean Model_FillPositionDescriptor( Model * pModel, jeProperty_List * pArray )
{
	jeXForm3d XForm;
	char * Name;

	jeProperty Property;
	if( !Model_GetXForm( pModel, &XForm ) )
		return( JE_TRUE );

	Name = Util_LoadLocalRcString( IDS_POSITION_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillVec3dGroup( &Property, Name, &XForm.Translation,	OBJECT_POSITION_FIELD  );
	if( !jeProperty_Append( pArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONX_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat(  &Property, Name, XForm.Translation.X, OBJECT_POSITION_FIELDX, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONY_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat(  &Property, Name, XForm.Translation.Y,	OBJECT_POSITION_FIELDY, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONZ_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, XForm.Translation.Z, OBJECT_POSITION_FIELDZ, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	jeProperty_FillGroupEnd( &Property, OBJECT_POSITION_FIELD_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

BrushList	*	Model_GetBrushList( Model * pModel )
{
	return( pModel->pBrushes );
}

jeProperty_List *	Model_BuildDescriptor( Model * pModel )
{
	jeProperty_List * pPropertyArray = NULL;
	jeProperty_List * pObjectArray;
	jeProperty_List * pArray = NULL;
	jeProperty		  Property;
	char		*	  Name;


	pObjectArray = jeProperty_ListCreateEmpty();

	Name = Util_LoadLocalRcString( IDS_NAME_FIELD );
	if( Name == NULL )
		goto UOBD_ERROR;
	jeProperty_FillString( &Property, Name, pModel->ObjectData.pszName, OBJECT_POSITION_FIELD );
	jeRam_Free( Name );
	if( !jeProperty_Append( pObjectArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( NULL );
	}
	
	if( !Model_FillPositionDescriptor( pModel, pObjectArray ) )
		goto UOBD_ERROR;

	if( !jeObject_GetPropertyList(pModel->pgeObject, &pPropertyArray) )
		goto UOBD_ERROR;


	 pArray = jeProperty_ListConCat( pObjectArray, pPropertyArray );
	 if( pArray == NULL )
		 goto UOBD_ERROR;

	jeProperty_ListDestroy( &pObjectArray );
	jeProperty_ListDestroy( &pPropertyArray );

	 return( pArray );
UOBD_ERROR:
	 if( pObjectArray )
		 jeProperty_ListDestroy( &pObjectArray );

	 if( pPropertyArray )
		 jeProperty_ListDestroy( &pPropertyArray );

	 if( pArray )
		 jeProperty_ListDestroy( &pArray );
	 return( NULL );
}


// FILE HANDLING
Model * Model_CreateFromFile( jeVFile * pF, const int32 nVersion, jePtrMgr * pPtrMgr )
{
	Model	*	pModel = NULL ;
	assert( jeVFile_IsValid( pF ) ) ;
	assert( nVersion <= MODEL_VERSION ) ;
	
	if( MODEL_VERSION != nVersion )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "Model_CreateFromFile Version.\n", NULL);
		return NULL ;
	}
	
	pModel = JE_RAM_ALLOCATE_STRUCT( Model ) ;
	if( pModel == NULL )
		goto MCFF_FAILURE ;

	memset( pModel, 0, sizeof *pModel ) ;
	assert( MODEL_SIGNATURE == (pModel->nSignature = MODEL_SIGNATURE) ) ;	// ASSIGN

	if( !Object_InitFromFile( pF , &pModel->ObjectData ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Object_InitFromFile.\n", NULL);
		return NULL;
	}

	if( !jeVFile_Read( pF, &pModel->bVisible, sizeof pModel->bVisible ) )
		goto MCFF_FAILURE ;

	if( !jeVFile_Read( pF, &pModel->bLocked, sizeof pModel->bLocked ) )
		goto MCFF_FAILURE ;

	if( !jeVFile_Read( pF, &pModel->nID, sizeof pModel->nID  ) )
		goto MCFF_FAILURE ;

	pModel->pgeObject = jeObject_CreateFromFile( pF, pPtrMgr );
	if( pModel->pgeObject == NULL )
		goto MCFF_FAILURE ;

	pModel->pInstance = jeObject_GetInstance(pModel->pgeObject) ;
	if( pModel->pInstance == NULL )
		goto MCFF_FAILURE ;
	jeObject_CreateInstanceRef(pModel->pgeObject); 
	jeObject_GetXForm(pModel->pgeObject, &pModel->XForm);

	pModel->pBrushes = BrushList_CreateFromFile( pF, pPtrMgr ) ;
	if( pModel->pBrushes == NULL )
		goto MCFF_FAILURE ;
	Object_SetInLevel( (Object*)pModel, JE_TRUE );


	return pModel ;

MCFF_FAILURE :
	if( pModel != NULL )
		Model_Destroy( &pModel ) ;

	jeErrorLog_AddString(JE_ERR_FILEIO_READ, "Model_CreateFromFile.\n", NULL);
	return NULL ;

}// Model_CreateFromFile



jeBoolean Model_WriteToFile( Model * pModel, Brush_WriteInfo * pWriteInfo )
{
	jeVFile * pF = pWriteInfo->pF;
	assert( pModel != NULL ) ;
	assert( MODEL_SIGNATURE == pModel->nSignature ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	if( !Object_WriteToFile( &pModel->ObjectData, pF ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.\n", NULL);
		return JE_FALSE;
	}


	if( !jeVFile_Write( pF, &pModel->bVisible, sizeof pModel->bVisible ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Model_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	if( !jeVFile_Write( pF, &pModel->bLocked, sizeof pModel->bLocked ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Model_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	if( !jeVFile_Write( pF, &pModel->nID, sizeof pModel->nID ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Model_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	if( !jeObject_WriteToFile( pModel->pgeObject, pF,  pWriteInfo->pPtrMgr ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "jeModel_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	if( !BrushList_WriteToFile( pModel->pBrushes, pWriteInfo ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Model_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	return JE_TRUE ;

}// Model_WriteToFile


//ENUM
int32 Model_EnumBrushes( Model * pModel, void * pVoid, BrushListCB Callback )
{
	assert( pModel );
	assert( Callback );

	return( BrushList_EnumBrushes( pModel->pBrushes, pVoid, Callback ) );
}

// DEBUGGING
#ifdef _DEBUG
jeBoolean Model_IsValid( const Model * pModel )
{
	assert( pModel != NULL ) ;
	return MODEL_SIGNATURE == pModel->nSignature ;
}
#endif //_DEBUG

/* EOF: Model.c */