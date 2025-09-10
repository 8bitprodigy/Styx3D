/****************************************************************************************/
/*  LEVEL.C                                                                             */
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

/* Open Source Revision -----------------------------------------------------------------
 By: Dennis Tierney (DJT) dtierney@oneoverz.com
 On: 12/27/99 8:52:31 PM
 Comments: Added Level_TestForObject() - Test to see if an Object Kind is in the 
                                         current level.
----------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include <Assert.h>
#include <Memory.h>
#include <String.h>

#include "Brush.h"
#include "ErrorLog.h"
#include "Group.h"
#include "jeWorld.h"
#include "jeList.h"	// Linked List module
#include "MatrArray.h"
#include "ObjectList.h"
#include "Ram.h"
#include "../Resource.h"
#include "jeresource.h" //added CyRiuS
#include "Util.h"
#include "BrushTemplate.h"
#include "Level.h"
#include "UserObj.h"

#include "Settings.h"

#include "jwe.h"

#define SIGNATURE (0xACDCABBA)
#define LEVEL_VERSION	(1)

#define LEVEL_DEFAULTSNAPSIZE	(8)
#define LEVEL_DEFAULROTATESIZE	(15)
#define LEVEL_DEFAULTUNDODEPTH	(3)

// added by cjp
// When we create a new level or load one, vertex manipulation snapping is initially on.
#define LEVEL_DEFAULTSHOULDSNAPVERTS (1)
// end added by cjp

typedef struct tagFaceInfo 
{
	jeFaceInfo				FaceInfo ;		//FaceInfo data used to init current FaceIndex
} FaceInfo_Struct;

typedef struct tagSelectKindInfo
{
	int32	nEntities ;
	int32	nLights ;
	int32	nActors ;
	int32	nBrushes ;
	int32	nModels ;
	int32	nTerrain ;
	int32	nCameras ;
	int32	nUserObjects;
	int32   nClass;
} SelectKindInfo ;

typedef struct tagLevel
{
#ifdef _DEBUG
	int						nSignature ;
#endif

	jeBoolean				bChanged;

	ObjectList			*	pSelObjects ;
	ObjectList			*   pSubSelObjects; //This is used by controler objects
	jeExtBox				SelBounds ;
	GroupList			*	pGroups ;
	LEVEL_GROUPVIS			GroupVisibility ;
	Group				*	CurrentGroup ;

	jeFaceInfo_Array	*	pFaceInfoArray;
	jeMaterial_Array	*	pMatrArray;

    // Krouer: use the AfxGetApp() Material list pointer instead
	//MaterialList_Struct	*	pGlobalMaterials; // List of materials available to all levels
	MaterialList_Struct	*	pGlobalShaders; // List of shaders available to all levels (cyrius)

	FaceInfo_Struct			DefaultFace;		// This is the default face info applied to new brushes

	jeWorld				*	pWorld ;		// Don't delete this, Doc does
	jeBoolean				bDirty ;

	ObjectList			*	pClassList ;

	Model				*	ParentModel;   //Hack to get Hiarchy until real hiarchy is done.
	Model				*	pCurrentModel ;
	ModelList			*	pModels ;

	int32					nGridSnapSize ;
	int32					nRotateSnapSize;
	jeBSP_Options			Options; 
	jeBSP_Logic				Logic; 
	jeBSP_LogicBalance		LogicBalance;

	jeBoolean				bSnapToGrid ;
	jeVec3d					ConstructLines;	//The depth at witch new objects will be placed

		// Added by cjp
	jeBoolean				bSnapVertsToGrid;
	// end added by cjp

	Undo				*	pUndo ;
	
	LEVEL_SEL				SelType ;


	LightList			*	pLightList;	
	CameraList			*	pCameraList;
	Camera				*	pCurCamera;
	ObjectList			*	pUserObjList;

	LEVEL_UPDATE			BrushUpdate;
	jeBoolean				BrushLightIncremental;
	LEVEL_UPDATE			LightUpdate;

} Level ;


static void Level_DestroyGroupCB( void *p1 )
{
	Group * pGroup = (Group*)p1 ;
	assert( pGroup != nullptr ) ;

	Group_Destroy( &pGroup ) ;
}// DestroyGroupCB

static void Level_DestroyModelCB( void * p1 )
{
	Model * pModel = (Model*)p1 ;
	assert( pModel != nullptr ) ;

	Object_Free( (Object**)&pModel ) ;
}// Level_DestroyModelCB

static jeBoolean Level_RestoreTransformCB( Object *pObject, void *Context )
{
	return( Object_SetTransform( pObject, (jeXForm3d*)Context ) );
}

static void Level_DestroyTransformContextCB( void *Context )
{
	jeRam_Free( Context );
}
/*
static jeBoolean Level_DestroyBrushCB( Brush * pBrush, void *Context )
{
	Object_Free( (Object**)&pBrush );
	Context;
	return( JE_TRUE );
}
*/
static jeBoolean Level_DestroyUserObjCb( Object * pObject, void * Context )
{
	//Royce
	Level * pLevel = (Level*)Context ;
	
	
	UserObj_RemoveFromWorld((UserObj*)pObject, pLevel->pWorld );
	//jeObject_RemoveChild( Model_GetjeObject(pModel), UserObj_GetjeObject( (UserObj*)pObject ) );
	//------

	Object_Free( &pObject );
	return( JE_TRUE );
}

static jeBoolean Level_RestoreCreateCB( Object *pObject, void *Context )
{
	Level * pLevel = (Level*)Context ;

	Level_SelectObject(  pLevel, pObject, LEVEL_DESELECT ) ;
	Level_DeleteObject( pLevel, pObject );
	return( JE_TRUE );
}

static void Level_DestroyCreateContextCB( void *Context )
{
	Context;
}

static jeBoolean Level_RestoreDeleteCB( Object *pObject, void *Context )
{
	Level * pLevel = (Level*)Context ;

	if( !Level_AddObject( pLevel, pObject ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Level_RestoreDeleteCB:Level_AddObject");
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

static jeBoolean Level_SelectKindInfoCB( Object * pObject, void * Context )
{
	SelectKindInfo * pski = (SelectKindInfo*)Context ;

	switch( Object_GetKind( pObject ) )
	{
		case KIND_BRUSH :	pski->nBrushes++ ;	break ;
		case KIND_MODEL :	pski->nModels++ ;	break ;
		case KIND_LIGHT :	pski->nLights++ ;	break ;
		case KIND_CAMERA :	pski->nCameras++ ;	break ;
		case KIND_USEROBJ :	pski->nUserObjects++ ;	break ;
		case KIND_CLASS :   pski->nClass++ ; break ;
	}
	return JE_TRUE ;

}// Level_SelectKindInfoCB

static void Level_DestroyDeleteContextCB( void *Context )
{
	Context;
}

static jeBoolean Level_RestoreBrushShearCB( Object *pObject, void *Context )
{
	Brush *pBrush;
	jeBrush *OldBrush;
	Model * pModel;


	assert( pObject );
	assert( Context );
	assert( Object_GetKind( pObject )== KIND_BRUSH );

	pBrush = (Brush*)pObject;
	pModel = Brush_GetModel( pBrush );
	Model_RemoveBrushWorld( pModel, pBrush );
	OldBrush = Brush_GetjeBrush( pBrush );
	if( OldBrush )
	{
		jeBrush_Destroy( &OldBrush );
	}
	Brush_SetGeBrush( pBrush, Brush_GetKind( pBrush ),(jeBrush*)Context );
	Brush_SetModel( pBrush, nullptr );
	Model_AddBrushWorld( pModel, pBrush, JE_TRUE, JE_TRUE );
	jeBrush_CreateRef((jeBrush*)Context ); //The undo will release a ref on destroy undo
	return( JE_TRUE );
}

static void Level_DestroyBrushShearCB( void *Context )
{
	jeBrush *pBrush = (jeBrush *)Context;
	
	assert( Context );
	jeBrush_Destroy( &pBrush );
}
	
 
static void Level_InitUndoFunctions( Level * pLevel )
{
	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pLevel->pUndo );

	Undo_RegisterCallBack( pLevel->pUndo, UNDO_TRANSFORM, Level_RestoreTransformCB, Level_DestroyTransformContextCB );
	Undo_RegisterCallBack( pLevel->pUndo, UNDO_CREATEOBJECT, Level_RestoreCreateCB, Level_DestroyCreateContextCB );
	Undo_RegisterCallBack( pLevel->pUndo, UNDO_DELETEOBJECT, Level_RestoreDeleteCB, Level_DestroyDeleteContextCB );
	Undo_RegisterCallBack( pLevel->pUndo, UNDO_APPLYTEXTURE, Brush_RestoreMaterialCB, Brush_DestroyMaterialContextCB );
	Undo_RegisterCallBack( pLevel->pUndo, UNDO_BRUSHSHEAR, Level_RestoreBrushShearCB, Level_DestroyBrushShearCB );
} // Level_InitUndoFunctions

static Object*  Level_NewCamera( Level * pLevel, jeVec3d *pWorldPt )
{
	Camera *pCamera ;
	char  * Name;
	int32	nNumber;
	jeXForm3d	XForm;
	jeObject *pgeObject;

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pLevel->pCameraList );

	Name = Object_CreateDefaultName( KIND_CAMERA, 0 );
	nNumber = Level_GetNextObjectId( pLevel, KIND_CAMERA, Name );

	pCamera = Camera_Create( Name, pLevel->CurrentGroup, nNumber );

	// [MLB-ICE]
	jeRam_Free(Name);	// Icestorm: Don't forget to clean up your name ;=)
	// [MLB-ICE]

	if( pCamera == nullptr )
	{
		jeErrorLog_Add(JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( nullptr );
	}
	
	jeXForm3d_SetTranslation( &XForm, pWorldPt->X, pWorldPt->Y, pWorldPt->Z );
	Camera_SetXForm( pCamera, &XForm );

	if( CameraList_Append( pLevel->pCameraList, pCamera ) == nullptr ) 
	{
		jeErrorLog_Add(JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( nullptr );
	}
	pgeObject = Camera_GetjeObject( pCamera );
	assert( pgeObject );

	// This was commented out (but why), added it again JH 25.4.2000
	jeWorld_AddObject( pLevel->pWorld, pgeObject );

	Undo_Push( pLevel->pUndo, UNDO_CREATE );
	Undo_AddSubTransaction( pLevel->pUndo, UNDO_CREATEOBJECT, (Object*)pCamera, pLevel );
	Object_Free( (Object**)&pCamera );
	return( (Object*)pCamera );
} // Level_NewCamera



static Object*  Level_NewLight( Level * pLevel, jeVec3d *pWorldPt )
{
	Light *pLight ;
	char  * Name;
	int32	nNumber;
	Light *pLightTemplate;
	jeXForm3d	XForm;
	jeBoolean  bUpdate;

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pLevel->pLightList );


	Name = Object_CreateDefaultName( KIND_LIGHT, 0 );
	nNumber = Level_GetNextObjectId( pLevel, KIND_LIGHT, Name );

	pLightTemplate = Light_CreateTemplate(  pLevel->pWorld );
	
	jeXForm3d_SetTranslation( &XForm, pWorldPt->X, pWorldPt->Y, pWorldPt->Z );
	Light_SetXForm( pLightTemplate, &XForm );
	bUpdate = (pLevel->LightUpdate == LEVEL_UPDATE_CHANGE) || (pLevel->LightUpdate == LEVEL_UPDATE_REALTIME );
	pLight = Light_FromTemplate( Name, pLevel->CurrentGroup, pLightTemplate, nNumber, bUpdate );
	Light_Destroy( &pLightTemplate );

	if( !bUpdate )
		Object_Dirty( (Object*)pLight );

	if( pLight == nullptr )
	{
		jeErrorLog_Add(JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( nullptr );
	}
	if( LightList_Append( pLevel->pLightList, pLight ) == nullptr ) 
	{
		jeErrorLog_Add(JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( nullptr );
	}
	Undo_Push( pLevel->pUndo, UNDO_CREATE );
	Undo_AddSubTransaction( pLevel->pUndo, UNDO_CREATEOBJECT, (Object*)pLight, pLevel );
	Object_Free( (Object**)&pLight );
	return( (Object*)pLight );
} // Level_NewLight

static jeBoolean Level_SearchMatrIdxByName( Level * pLevel, const char * Name, jeMaterial_ArrayIndex* Index )
{
	const jeMaterial		*pMaterial = nullptr;

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pLevel->pMatrArray );
	assert( Name );

	pMaterial = jeMaterial_ArrayGetNextMaterial(pLevel->pMatrArray, pMaterial);

	while (pMaterial )
	{
		if( strcmp( jeMaterial_GetName(pMaterial), Name) == 0 )
		{
			*Index = jeMaterial_ArrayGetMaterialIndex(pLevel->pMatrArray, pMaterial);
			return( JE_TRUE );
		}
		pMaterial = jeMaterial_ArrayGetNextMaterial(pLevel->pMatrArray, pMaterial);
	}

	return( JE_FALSE );
} // Level_SearchMatrIdxByName

#ifdef _USE_BITMAPS
static jeBoolean Level_GetMaterialIdx( Level * pLevel, const char * Name, jeBitmap *pBitmap, jeMaterial_ArrayIndex *MaterialIndex )
{
	jeMaterial_ArrayIndex pMatrIdx;

	assert( pLevel );
	assert( Name );
	assert( pBitmap );
	assert( MaterialIndex );
	assert( pLevel->pMatrArray );

	if( !Level_SearchMatrIdxByName( pLevel, Name, &pMatrIdx ) )
	{
		pMatrIdx = jeMaterial_ArrayCreateMaterial( pLevel->pMatrArray, Name);

		if (pMatrIdx == JE_MATERIAL_ARRAY_NULL_INDEX)
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			return( JE_FALSE );
		}

		// Krouer : make BitmapName different from Name
		// BitmapName parameter of next function is now the following pattern
		// folder:file
		//strcpy(TotalName, "GlobalMaterials:");
		//strcat(TotalName, Name);
		// change back to something better, with suddir and pak files

		if( !jeMaterial_ArraySetMaterialBitmap(pLevel->pMatrArray, pMatrIdx, pBitmap, Name) )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			return( JE_FALSE );
		}
	}
	*MaterialIndex = pMatrIdx;
	return( JE_TRUE );
}  //Level_GetMaterialIdx
#else
static jeBoolean Level_GetMaterialIdx( Level * pLevel, const char * Name, jeMaterialSpec *pMatSpec, jeMaterial_ArrayIndex *MaterialIndex )
{
	jeMaterial_ArrayIndex pMatrIdx;

	assert( pLevel );
	assert( Name );
	assert( MaterialIndex );
	assert( pLevel->pMatrArray );

	if( !Level_SearchMatrIdxByName( pLevel, Name, &pMatrIdx ) )
	{
		pMatrIdx = jeMaterial_ArrayCreateMaterial( pLevel->pMatrArray, Name);

		if (pMatrIdx == JE_MATERIAL_ARRAY_NULL_INDEX)
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			return( JE_FALSE );
		}

		// Krouer : make BitmapName different from Name
		// BitmapName parameter of next function is now the following pattern
		// folder:file
		//strcpy(TotalName, "GlobalMaterials:");
		//strcat(TotalName, Name);
		// change back to something better, with suddir and pak files

		if( !jeMaterial_ArraySetMaterialSpec(pLevel->pMatrArray, pMatrIdx, pMatSpec, Name) )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			return( JE_FALSE );
		}
	}
	*MaterialIndex = pMatrIdx;
	return( JE_TRUE );
}  //Level_GetMaterialIdx
#endif


jeBoolean Level_SetFaceInfoToCurMaterial( Level * pLevel )
{
	jeMaterial_ArrayIndex MaterialIndex;
	Material_Struct * Material = nullptr;
	CJweApp* pApp = (CJweApp*)AfxGetApp();

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;

	Material = MaterialList_GetCurMaterial( pApp->GetMaterialList() );
#ifdef _USE_BITMAPS
	if( !Level_GetMaterialIdx( pLevel, Materials_GetName( Material ), (jeBitmap*) Materials_GetBitmap( Material ), &MaterialIndex  ) )
#else
	if( !Level_GetMaterialIdx( pLevel, Materials_GetName( Material ), (jeMaterialSpec*) Materials_GetMaterialSpec( Material ), &MaterialIndex  ) )
#endif
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}
	if( MaterialIndex != pLevel->DefaultFace.FaceInfo.MaterialIndex )
	{
		pLevel->DefaultFace.FaceInfo.MaterialIndex = MaterialIndex;
	}
	return( JE_TRUE );
} // Level_SetFaceInfoToCurMaterial
	

static jeBoolean Level_InitDefaultFace( Level * pLevel )
{
	jeFaceInfo*			pFaceInfo ;

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;

	pFaceInfo = &pLevel->DefaultFace.FaceInfo;
	memset(pFaceInfo, 0, sizeof(jeFaceInfo) ) ;
	jeFaceInfo_SetDefaults( pFaceInfo );
	Level_SetFaceInfoToCurMaterial( pLevel );
	return( JE_TRUE );

} // Level_InitDefaultFace


static jeBoolean Level_InitWorldData( Level* pLevel, jeWorld * pWorld,MaterialList_Struct * pGlobalMaterials )
{
	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pWorld );
	assert( pGlobalMaterials );

	pLevel->bChanged = JE_FALSE;
	pLevel->pWorld = pWorld ;
    // Krouer: not use when use directly the CJweApp material list
	//pLevel->pGlobalMaterials = pGlobalMaterials ;
	pLevel->pFaceInfoArray = jeWorld_GetFaceInfoArray( pWorld ) ;
	if( pLevel->pFaceInfoArray == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "pFaceInfoArray" );
		return( JE_FALSE );
	}

	pLevel->pMatrArray = jeWorld_GetMaterialArray(pWorld);
	if( pLevel->pMatrArray == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "jeWorld_GetMaterialArray" );
		return( JE_FALSE );
	}

	return( JE_TRUE );
} // Level_InitWorldData

jeResourceMgr	* Level_CreateResourceMgr( jeEngine* pEngine )
{
//	jeVFile			*	pFS = nullptr ;	[MLB-ICE]
	char		AppPath[255];
	char		SubPath[255];
	jeResourceMgr* ResourceMgr{};


	ResourceMgr =  jeResource_MgrCreate( pEngine);
	if( ResourceMgr == nullptr )
	{
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Level_CreateResourceMgr:jeResource_MgrCreate", nullptr);
		return nullptr;
	}
	// [MLB-ICE]
	Util_GetAppPath( AppPath, 255 );
	//I'm not sure where is the best place to store this
	strcpy( SubPath, AppPath );
	strcat( SubPath, "Sounds" );
	if (!jeResource_OpenDirectory(ResourceMgr, SubPath, "Sounds"))
		jeErrorLog_AddString(JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeResource_OpenDirectory", SubPath);

	// save bitmap vfile
	strcpy( SubPath, AppPath );
	strcat( SubPath, "GlobalMaterials" );
	if (!jeResource_OpenDirectory(ResourceMgr, SubPath, "GlobalMaterials"))
		jeErrorLog_AddString( JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeResource_OpenDirectory", SubPath );

	// save actors vfile
	strcpy( SubPath, AppPath );
	strcat( SubPath, "Actors" );
	if (!jeResource_OpenDirectory(ResourceMgr, SubPath, "Actors"))
		jeErrorLog_AddString( JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeResource_OpenDirectory", SubPath );

	//BEGIN CYRIUS
	// save shader vfile
	strcpy( SubPath, AppPath );
	strcat( SubPath, "Shaders" );
	if (!jeResource_OpenDirectory(ResourceMgr, SubPath, "Shaders"))
		jeErrorLog_AddString( JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeResource_OpenDirectory", SubPath );

	//END CYRIUS
/*	Util_GetAppPath( AppPath, 255 );
	//I'm not sure where is the best place to store this
	strcpy( SubPath, AppPath );
	strcat( SubPath, "Sounds" );
	pFS = jeVFile_OpenNewSystem
	(
		nullptr, 
		JE_VFILE_TYPE_DOS,
		SubPath,
		nullptr,
		JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY
	);
	if( pFS == nullptr )
	{
		jeErrorLog_AddString(JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeVFile_OpenNewSystem", SubPath);
	}
	else
	{
		jeResource_AddVFile( ResourceMgr, "Sounds", pFS );
	}

	// save bitmap vfile
	strcpy( SubPath, AppPath );
	strcat( SubPath, "GlobalMaterials" );
	pFS = jeVFile_OpenNewSystem(	nullptr,
									JE_VFILE_TYPE_DOS,
									SubPath,
									nullptr,
									JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY );
	if ( pFS == nullptr )
	{
		jeErrorLog_AddString( JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeVFile_OpenNewSystem", SubPath );
	}
	else
	{
		jeResource_AddVFile( ResourceMgr, "GlobalMaterials", pFS );
	}

	// save actors vfile
	strcpy( SubPath, AppPath );
	strcat( SubPath, "Actors" );
	pFS = jeVFile_OpenNewSystem(	nullptr,
									JE_VFILE_TYPE_DOS,
									SubPath,
									nullptr,
									JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY );
	if ( pFS == nullptr )
	{
		jeErrorLog_AddString( JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeVFile_OpenNewSystem", SubPath );
	}
	else
	{
		jeResource_AddVFile( ResourceMgr, "Actors", pFS );
	}

	//BEGIN CYRIUS
	// save shader vfile
	strcpy( SubPath, AppPath );
	strcat( SubPath, "Shaders" );
	pFS = jeVFile_OpenNewSystem(	nullptr,
									JE_VFILE_TYPE_DOS,
									SubPath,
									nullptr,
									JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY );
	if ( pFS == nullptr )
	{
		jeErrorLog_AddString( JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeVFile_OpenNewSystem", SubPath );
	}
	else
	{
		jeResource_AddVFile( ResourceMgr, "Shaders", pFS );
	}

	//END CYRIUS
	*/

	// [MLB-ICE] EOB

	// all done
	return ResourceMgr;

}

static void Level_InitDefaultPrefs( Level* pLevel )
{
	jeXForm3d			XForm ;

	assert( pLevel != nullptr ) ;
	assert( pLevel->nSignature == SIGNATURE ) ;
	
	// added by cjp
	pLevel->bSnapVertsToGrid = LEVEL_DEFAULTSHOULDSNAPVERTS;
	// end added by cjp

	pLevel->bSnapToGrid = JE_TRUE ;
	pLevel->nGridSnapSize = LEVEL_DEFAULTSNAPSIZE ;
	pLevel->nRotateSnapSize = LEVEL_DEFAULROTATESIZE ;
	pLevel->GroupVisibility = LEVEL_GROUPVIS_ALL ;
	pLevel->BrushUpdate = LEVEL_UPDATE_DESELECT;
	pLevel->LightUpdate = LEVEL_UPDATE_REALTIME;
	pLevel->BrushLightIncremental = JE_TRUE;

	//	tom morris feb 2005
	pLevel->Options = BSP_OPTIONS_CSG_BRUSHES | BSP_OPTIONS_MAKE_VIS_AREAS; 
//	pLevel->Options = BSP_OPTIONS_CSG_BRUSHES; 
//	end tom morris feb 2005

	pLevel->Logic = Logic_Normal; 
	pLevel->LogicBalance = 2;

	jeVec3d_Set( &pLevel->ConstructLines, 0.0f, 0.0f, 0.0f );
	jeXForm3d_SetIdentity( &XForm ) ;
}// Level_InitDefaultPrefs

static jeBoolean Level_LoadPrefs( Level* pLevel, jeVFile *pF, float Version )
{
	int32		SubKind;

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pF );

	if( !jeVFile_Read( pF, &pLevel->bSnapToGrid , sizeof pLevel->bSnapToGrid ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read bSnapToGrid" );
		return( JE_FALSE );
	}
#pragma message( "Need to make Rotate Snap size save also" )

	pLevel->nRotateSnapSize = LEVEL_DEFAULROTATESIZE ;

#pragma message( "Need to make should snap verts save also" )
	
	// added by cjp
	pLevel->bSnapVertsToGrid = LEVEL_DEFAULTSHOULDSNAPVERTS;
	// end added by cjp

	if( !jeVFile_Read( pF, &pLevel->nGridSnapSize , sizeof pLevel->nGridSnapSize ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read nGridSnapSize" );
		return( JE_FALSE );
	}

	if( !jeVFile_Read( pF, &pLevel->GroupVisibility , sizeof pLevel->GroupVisibility ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read GroupVisibility" );
		return( JE_FALSE );
	}

	if( !jeVFile_Read( pF, &SubKind , sizeof SubKind ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read SubKind" );
		return( JE_FALSE );
	}

	if( !jeVFile_Read( pF, &pLevel->BrushUpdate , sizeof &pLevel->BrushUpdate ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read BrushUpdate" );
		return( JE_FALSE );
	}

	if( !jeVFile_Read( pF, &pLevel->LightUpdate , sizeof &pLevel->LightUpdate ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read LightUpdate" );
		return( JE_FALSE );
	}

	if( !jeVFile_Read( pF, &pLevel->BrushLightIncremental , sizeof &pLevel->BrushLightIncremental ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read BrushLightIncremental" );
		return( JE_FALSE );
	}

	if( Version > 0.1f )
	{
		if( !jeVFile_Read( pF, &pLevel->Options , sizeof &pLevel->Options ) )
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read Options" );
			return( JE_FALSE );
		}

		if( !jeVFile_Read( pF, &pLevel->Logic , sizeof &pLevel->Logic ) )
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read Logic" );
			return( JE_FALSE );
		}
		if( !jeVFile_Read( pF, &pLevel->LogicBalance , sizeof &pLevel->LogicBalance ) )
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to read LogicBalance" );
			return( JE_FALSE );
		}
	}
	else
	{
		pLevel->Options = BSP_OPTIONS_CSG_BRUSHES; 
		pLevel->Logic = Logic_Normal; 
		pLevel->LogicBalance = 2;
	}
	return( JE_TRUE );
} // Level_LoadPrefs

static jeBoolean Level_SavePrefs( Level* pLevel, jeVFile *pF  )
{

	int32		SubKind;

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pF );


	if( !jeVFile_Write( pF, &pLevel->bSnapToGrid , sizeof pLevel->bSnapToGrid ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write bSnapToGrid" );
		return( JE_FALSE );
	}

	if( !jeVFile_Write( pF, &pLevel->nGridSnapSize , sizeof pLevel->nGridSnapSize ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write nGridSnapSize" );
		return( JE_FALSE );
	}

	if( !jeVFile_Write( pF, &pLevel->GroupVisibility , sizeof pLevel->GroupVisibility ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write GroupVisibility" );
		return( JE_FALSE );
	}

	if( !jeVFile_Write( pF, &SubKind , sizeof SubKind ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write SubKind" );
		return( JE_FALSE );
	}

	if( !jeVFile_Write( pF, &pLevel->BrushUpdate , sizeof &pLevel->BrushUpdate ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write BrushUpdate" );
		return( JE_FALSE );
	}

	if( !jeVFile_Write( pF, &pLevel->LightUpdate , sizeof &pLevel->LightUpdate ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write LightUpdate" );
		return( JE_FALSE );
	}

	if( !jeVFile_Write( pF, &pLevel->BrushLightIncremental , sizeof &pLevel->BrushLightIncremental ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write BrushLightIncremental" );
		return( JE_FALSE );
	}

	if( !jeVFile_Write( pF, &pLevel->Options , sizeof &pLevel->Options ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write Options" );
		return( JE_FALSE );
	}

	if( !jeVFile_Write( pF, &pLevel->Logic , sizeof &pLevel->Logic ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write Logic" );
		return( JE_FALSE );
	}
	if( !jeVFile_Write( pF, &pLevel->LogicBalance , sizeof &pLevel->LogicBalance ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "Unable to write LogicBalance" );
		return( JE_FALSE );
	}

	return( JE_TRUE );
} // Level_SavePrefs



static jeBoolean Level_InitLists( Level* pLevel )
{
	char		szDefault[GROUP_MAXNAMELENGTH+1] ;
	Group		* pGroup{};

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pLevel->pWorld != nullptr );

	pLevel->pModels		= ModelList_Create( ) ;
	if( pLevel->pModels == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}
	pLevel->pLightList = LightList_Create(  pLevel->pWorld  );
	if( pLevel->pLightList == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}

	pLevel->pCameraList = CameraList_Create();
	if( pLevel->pCameraList == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}

	pLevel->pClassList = ObjectList_Create( ) ;
	if( pLevel->pClassList == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}

	pLevel->pUserObjList = ObjectList_Create( ) ;
	if( pLevel->pUserObjList == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}

	pLevel->pSelObjects = ObjectList_Create( ) ;
	if( pLevel->pSelObjects == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}

	pLevel->pSubSelObjects = ObjectList_Create( ) ;
	if( pLevel->pSubSelObjects == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}

	pLevel->pGroups		= GroupList_Create( ) ;
	if( pLevel->pGroups == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}
	{   //Init default Group
		Util_GetRcString( szDefault, IDS_DEFAULTGROUPNAME ) ;
		pGroup = Group_Create( szDefault ) ;
		if( pGroup == nullptr )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			return( JE_FALSE );
		}
		if( GroupList_Append( pLevel->pGroups, pGroup ) == nullptr )
		{
			Group_Destroy( &pGroup ) ;
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			return( JE_FALSE );
		}

		pLevel->CurrentGroup = pGroup ;
	}

	{ //Init default Model
		pLevel->pCurrentModel = Model_Create( pGroup, "Default", 0 ) ;
		if( pLevel->pCurrentModel == nullptr )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			return( JE_FALSE );
		}
		pLevel->ParentModel = pLevel->pCurrentModel;
		if( jeWorld_AddObject( pLevel->pWorld, Model_GetjeObject( pLevel->pCurrentModel ) )== JE_FALSE )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_InitLists:jeWorld_AddObject" );
			return( JE_FALSE );
		}
		if( ModelList_Append( pLevel->pModels, pLevel->pCurrentModel ) == nullptr )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			return( JE_FALSE );
		}
		if( !Group_AddObject( pGroup, (Object *)pLevel->pCurrentModel ) )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_InitLists:Group_AddObject" );
			return( JE_FALSE );
		}
		Object_SetInLevel( (Object*)pLevel->pCurrentModel, JE_TRUE );
	}
	return( JE_TRUE );
}

static jeBoolean Level_ReattachGroupsCB( Object * pObject, void * lParam )
{
	GroupList * pGroupList = (GroupList*)lParam;

	assert( pObject );
	assert( lParam );

	return( GroupList_ReattachObject( pGroupList, pObject ) );
}


static jeBoolean Level_UserObjListCreateFromFile( Level *pLevel, jeVFile *pF, jePtrMgr * pPtrMgr )
{
	int nItems;
	int i;
	UserObj * pUserObj;

	assert( pLevel );
	assert( pF );
	assert( pPtrMgr );

	pLevel->pUserObjList = ObjectList_Create();
	if( pLevel->pUserObjList == nullptr )
		return( JE_FALSE );

	if( jeVFile_Read( pF, &nItems, sizeof nItems ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_UserObjListCreateFromFile.\n", nullptr);
		return JE_FALSE;
	}

	for( i = 0; i < nItems; i++ )
	{
		pUserObj = UserObj_CreateFromFile( pF, pPtrMgr );
		if( pUserObj == nullptr )
		{
			jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_UserObjListCreateFromFile.\n", nullptr);
			return JE_FALSE;
		}
		ObjectList_Append( pLevel->pUserObjList, (Object*)pUserObj );
	}
	return( JE_TRUE );
}



static jeBoolean Level_LoadLists( Level* pLevel, jeVFile *pF, jePtrMgr * pPtrMgr )

{
	ModelIterator		pMI ;
	GroupIterator		GI;

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pLevel->pWorld != nullptr );
	assert( pF );

	pLevel->pGroups		= GroupList_CreateFromFile( pF ) ;
	if( pLevel->pGroups == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "GroupList_CreateFromFile" );
		return( JE_FALSE );
	}

	pLevel->pClassList = ObjectList_Create( ) ;
	if( pLevel->pClassList == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}

	pLevel->pModels		= ModelList_CreateFromFile( pF, pPtrMgr ) ;
	if( pLevel->pModels == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_LoadLists" );
		return( JE_FALSE );
	}
	ModelList_Reattach( pLevel->pModels, pLevel->pWorld ) ;
	pLevel->pCurrentModel = ModelList_GetFirst( pLevel->pModels, &pMI ) ;
	pLevel->ParentModel = pLevel->pCurrentModel;
	Object_AddRef( (Object*)pLevel->pCurrentModel );


	pLevel->pLightList = LightList_CreateFromFile( pF, pLevel->pWorld, pPtrMgr  );
	if( pLevel->pLightList == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_LoadLists" );
		return( JE_FALSE );
	}
	
	pLevel->pCameraList = CameraList_CreateFromFile( pF, pPtrMgr );
	if( pLevel->pCameraList == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_LoadLists" );
		return( JE_FALSE );
	}

	if( !Level_UserObjListCreateFromFile( pLevel, pF, pPtrMgr ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_LoadLists" );
		return( JE_FALSE );
	}

	// @@ CB : Attach to the root model!
	//Royce-3
	//ObjectList_EnumObjects( pLevel->pUserObjList, Model_GetjeObject( pLevel->ParentModel), (ObjectListCB) UserObj_AddToObject );
	//---

#pragma message( "should the current selection be loaded" )
	pLevel->pSelObjects = ObjectList_Create( ) ;
	if( pLevel->pSelObjects == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}

	pLevel->pSubSelObjects = ObjectList_Create( ) ;
	if( pLevel->pSubSelObjects == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( JE_FALSE );
	}

	Level_EnumObjects( pLevel, pLevel->pGroups, Level_ReattachGroupsCB );
	pLevel->CurrentGroup = GroupList_GetFirst( pLevel->pGroups, &GI ) ;
	pLevel->pCurCamera = CameraList_GetFirst( pLevel->pCameraList, &GI );
	return( JE_TRUE );
}// Level_LoadLists


static jeBoolean Level_UserObjWriteToFile( Level *pLevel, jeVFile *pF, jePtrMgr * pPtrMgr )
{
	int32	nItems ;
	Object	* pObject;
	ListIterator pli;

	assert( pLevel != nullptr ) ;
	assert( pLevel->pUserObjList != nullptr ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	
	nItems = ObjectList_GetNumItems( pLevel->pUserObjList ) ;
	if( jeVFile_Write( pF, &nItems, sizeof nItems ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_UserObjWriteToFile.\n", nullptr);
		return JE_FALSE;
	}
	
	pObject = ObjectList_GetFirst (pLevel->pUserObjList, &pli);
	while( pObject )
	{
		if( !UserObj_WriteToFile( (UserObj*)pObject, pF,  pPtrMgr ) )
		{
			jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_UserObjWriteToFile.\n", nullptr);
			return JE_FALSE;
		}

		pObject = ObjectList_GetNext(pLevel->pUserObjList, &pli);
	}
	return  JE_TRUE;

}// Level_UserObj


static jeBoolean Level_SaveLists( Level* pLevel, jeVFile *pF, jePtrMgr * pPtrMgr )
{

	assert( pLevel );
	assert( pLevel->nSignature == SIGNATURE ) ;
	assert( pLevel->pWorld != nullptr );
	assert( pF );

	if( !GroupList_WriteToFile( pLevel->pGroups, pF ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_SaveLists:GroupList_WriteToFile" );
		return( JE_FALSE );
	}
	if( !ModelList_WriteToFile( pLevel->pModels, pF, pPtrMgr ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_SaveLists:ModelList_WriteToFile" );
		return( JE_FALSE );
	}
	if( !LightList_WriteToFile( pLevel->pLightList, pF, pPtrMgr) ) 
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_SaveLists:LightList_WriteToFile" );
		return( JE_FALSE );
	}
	if( !CameraList_WriteToFile( pLevel->pCameraList, pF, pPtrMgr) ) 
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_SaveLists:CameraList_WriteToFile" );
		return( JE_FALSE );
	}
	
	if( !Level_UserObjWriteToFile( pLevel, pF, pPtrMgr ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_SaveLists:Level_UserObjWriteToFile" );
		return( JE_FALSE );
	}

#pragma message( "We need to save the groups!" )

	return( JE_TRUE );
}// Level_SaveLists


//
// END STATIC FUNCTIONS
//

static void Level_SetDefaultBoxFace( Brush * pBrush, int FaceIndex, float ShiftU, float ShiftV, float ScaleU, float ScaleV )
{
	jeProperty_Data Data;
	jeBrush_Face	*	pFace;

	pFace = Brush_GetFaceByIndex( pBrush, FaceIndex ) ;
	if( pFace == nullptr )
		return;
	Brush_DeselectAllFaces( pBrush  );
	Brush_SelectFace( pBrush, pFace );

	Data.Float = ShiftU;
	Brush_SetProperty( pBrush, BRUSH_SHIFTU_FIELD, FLOAT_STRING_TYPE, &Data, JE_TRUE, JE_TRUE );
	Data.Float = ShiftV;
	Brush_SetProperty( pBrush, BRUSH_SHIFTV_FIELD, FLOAT_STRING_TYPE, &Data, JE_TRUE, JE_TRUE );
	Data.Float = ScaleU;
	Brush_SetProperty( pBrush, BRUSH_DRAWSCALEU_FIELD, FLOAT_STRING_TYPE, &Data, JE_TRUE, JE_TRUE );
	Data.Float = ScaleV;
	Brush_SetProperty( pBrush, BRUSH_DRAWSCALEV_FIELD, FLOAT_STRING_TYPE, &Data, JE_TRUE, JE_TRUE );
}

void Level_SetDefaultBoxTexture( Level * pLevel, Object	* pBoxObject )
{
	Material_Struct *	pMaterial;
	MaterialIterator	MI;
	Brush *				pBrush = (Brush*)pBoxObject;
	CJweApp*            pApp = (CJweApp*)AfxGetApp();

	pMaterial =	MaterialList_SearchByName( pApp->GetMaterialList(), &MI, "jet3d" );
	if( pMaterial == nullptr )
		return;
	MaterialList_SetCurMaterial(  pApp->GetMaterialList(), pMaterial );
	Level_SetFaceInfoToCurMaterial( pLevel );
	Brush_SelectAllFaces( pBrush );
	Brush_ApplyMatrToFaces( pBrush, &pLevel->DefaultFace.FaceInfo, pLevel->pUndo );
	Level_SetDefaultBoxFace( pBrush, 0, 128.0f, 128.0f, 0.5f, 0.5 );
	Level_SetDefaultBoxFace( pBrush, 1, 128.0f, 128.0f, -0.5f, 0.5 );
	Level_SetDefaultBoxFace( pBrush, 2, 128.0f, 128.0f, 0.5f, 0.5 );
	Level_SetDefaultBoxFace( pBrush, 3, 128.0f, 128.0f, -0.5f, 0.5 );
	Level_SetDefaultBoxFace( pBrush, 4, 128.0f, 128.0f, 0.5f, 0.5 );
	Level_SetDefaultBoxFace( pBrush, 5, 128.0f, 128.0f, -0.5f, 0.5 );
	Brush_DeselectAllFaces( pBrush  );
}

Level * Level_Create( jeWorld * pWorld, MaterialList_Struct * pGlobalMaterials )
{
	Level* pLevel{};
	jeExtBox			TempBox{};
	Object* pLightObject{};
	Object* pBoxObject{};
	jeProperty_Data Data{};
	LEVEL_UPDATE		TempBrushUpdate{};
	LEVEL_UPDATE		TempLightUpdate{};

	assert( pWorld != nullptr ) ;
	assert( pGlobalMaterials != nullptr );
	pLevel = JE_RAM_ALLOCATE_STRUCT( Level ) ;
	if( pLevel == nullptr )
		goto LC_FAILURE ;

	memset( pLevel, 0, sizeof *pLevel ) ;
	assert( (pLevel->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	
	pLevel->SelType = LEVEL_SELNONE ;

	if( !Level_InitWorldData( pLevel, pWorld, pGlobalMaterials ) )
		goto LC_FAILURE ;
	
	Level_InitDefaultPrefs( pLevel );
	Level_InitDefaultFace( pLevel );

	if( !Level_InitLists( pLevel ) )
		goto LC_FAILURE ;

//	pLevel->pUndo = Undo_Create( LEVEL_DEFAULTUNDODEPTH ) ;
	pLevel->pUndo = Undo_Create( Settings_GetGlobal_UndoBuffer() ) ;	// Changed JH 13.3.2000
	if( pLevel->pUndo == nullptr )
		goto LC_FAILURE ;
	
	Level_InitUndoFunctions( pLevel );

	//Create Intial Brush and light
	TempBrushUpdate = pLevel->BrushUpdate;
	TempLightUpdate = pLevel->LightUpdate;

	pLevel->BrushUpdate = LEVEL_UPDATE_MANUEL;
	pLevel->LightUpdate = LEVEL_UPDATE_MANUEL;

	jeExtBox_Set( &TempBox, 0.0f, 64.0f, 128.0f, 0.0f, 64.0f, 0.0f);
	pLevel->pCurCamera = (Camera*)Level_NewObject( pLevel, KIND_CAMERA, 0,  &TempBox );

	jeExtBox_Set( &TempBox, -64.0f, 0.0f, -64.0f, 64.0f, 128.0f, 64.0f);
	pBoxObject = Level_SubtractBrush( pLevel, BRUSH_BOX,  &TempBox );

	jeExtBox_Set( &TempBox, 0.0f, 64.0f, 0.0f, 0.0f, 64.0f, 0.0f);
	pLightObject = Level_NewObject( pLevel, KIND_LIGHT, 0,  &TempBox );
	Data.Float = 2.0f;
	Light_SetProperty( (Light*)pLightObject, LIGHT_BRIGHTNESS_FIELD, FLOAT_STRING_TYPE, &Data, JE_TRUE);

	Level_SetDefaultBoxTexture( pLevel, pBoxObject );

	Level_SelectObject( pLevel, pLightObject , LEVEL_SELECT ) ;

	pLevel->BrushUpdate = TempBrushUpdate;
	pLevel->LightUpdate = TempLightUpdate;

	Undo_Reset( pLevel->pUndo );
	return pLevel ;

LC_FAILURE :
	if( pLevel != nullptr )
		Level_Destroy( &pLevel ) ;

	return nullptr ;
}// Level_Create

static void SelectDestroyCb( void * Data )
{
	Object * pObject = (Object*)Data;

	assert( pObject );

	Object_Free( &pObject );
}
static void Level_DestroyClassCB( void * Data )
{
	Object * pObject = (Object*)Data;

	assert( pObject );

	Object_Free( &pObject );
}

void Level_Destroy( Level ** ppLevel ) 
{
	Level * pLevel ;

	assert( ppLevel != nullptr ) ;
	pLevel = *ppLevel ;
	assert( pLevel->nSignature == SIGNATURE ) ;

	if( pLevel->pCurrentModel )
		Object_Free( (Object**)&pLevel->pCurrentModel );
	
	if( pLevel->pLightList != nullptr )
		LightList_Destroy( &pLevel->pLightList );

	if( pLevel->pCameraList != nullptr )
		CameraList_Destroy( &pLevel->pCameraList );

	if( pLevel->pUndo != nullptr )
		Undo_Destroy( &pLevel->pUndo ) ;
	
	if( pLevel->pSelObjects != nullptr )
		ObjectList_Destroy( &pLevel->pSelObjects, SelectDestroyCb ) ;

	if( pLevel->pSubSelObjects != nullptr )
		ObjectList_Destroy( &pLevel->pSubSelObjects, SelectDestroyCb ) ;

	if( pLevel->pGroups != nullptr )
		GroupList_Destroy( &pLevel->pGroups, Level_DestroyGroupCB ) ;

	if( pLevel->pUserObjList )
	{
		//Royce
		ObjectList_EnumObjects( pLevel->pUserObjList, pLevel, Level_DestroyUserObjCb );
		//-----
		ObjectList_Destroy( &pLevel->pUserObjList, nullptr );
	}

	if( pLevel->pClassList != nullptr )
	{
		ObjectList_Destroy( &pLevel->pClassList, Level_DestroyClassCB );
	}

	if( pLevel->pModels != nullptr )
	{
		//Level_EnumBrushes( pLevel, nullptr, Level_DestroyBrushCB ) ;
		ModelList_Destroy( &pLevel->pModels, Level_DestroyModelCB ) ;
	}

	assert( ((*ppLevel)->nSignature = 0) == 0 ) ;	// CLEAR

	jeRam_Free( *ppLevel ) ;
}// Level_Destroy

// ACCESSORS

Group * Level_GetCurrentGroup( const Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->CurrentGroup ;
}// Level_GetCurrentGroup


int32 Level_GetGridSnapSize( const Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->nGridSnapSize ;
}// Level_GetGridSnapSize

int32 Level_GetRotateSnapSize( const Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->nRotateSnapSize ;
}// Level_GetGridSnapSize

GroupList *	Level_GetGroupList( Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->pGroups ;

}// Level_GetGroupList

LEVEL_GROUPVIS Level_GetGroupVisibility( const Level *pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->GroupVisibility ;
}// Level_GetGroupVisibility

ModelList * Level_GetModelList( Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->pModels ;
}// Level_GetModelList

Model *	Level_GetCurModel( Level * pLevel ) 
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->pCurrentModel ;
}// Level_GetCurModel

ObjectList * Level_GetSelList( Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->pSelObjects ;
}// Level_GetSelList

ObjectList * Level_GetSubSelList( Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->pSubSelObjects ;
}// Level_GetSubSelList

LightList *	Level_GetLightList( Level * pLevel ) 
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->pLightList ;
}// Level_GetLightList

CameraList *	Level_GetCameraList( Level * pLevel ) 
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->pCameraList ;
}// Level_GetCameraList




LEVEL_SEL Level_GetSelType( const Level * pLevel ) 
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->SelType ;
}// Level_GetSelType

int32 Level_SelXFormModFlags(  const Level * pLevel )
{
	Object * pObject;
	ObjectIterator  Iterator;
	int32 ModFlags = JE_OBJECT_XFORM_ALL;

	assert( pLevel );
	assert( SIGNATURE == pLevel->nSignature ) ;
	assert( pLevel->pSelObjects );

	pObject = ObjectList_GetFirst( pLevel->pSelObjects, &Iterator ) ;

	while( pObject != nullptr )
	{
		ModFlags &=  Object_GetXFormModFlags( pObject );
		pObject = ObjectList_GetNext( pLevel->pSelObjects, &Iterator ) ;
	}

	 
	if( !(pLevel->SelType & LEVEL_SELMANY) )
	{
		return( ModFlags );
	}

	//If we can translate a multiple select we can rotate the objects about each other
	if( ModFlags & JE_OBJECT_XFORM_TRANSLATE )
	{
		ModFlags |= JE_OBJECT_XFORM_ROTATE;
		ModFlags |= JE_OBJECT_XFORM_SCALE;
	}
	return( ModFlags );

}

int32 Level_SubSelXFormModFlags(  const Level * pLevel )
{
	Object * pObject;
	ObjectIterator  Iterator;
	int32 ModFlags = AllSubSelect ;
	int32 ObjFlags;
	int32 ObjModFlags;

	assert( pLevel );
	assert( SIGNATURE == pLevel->nSignature ) ;
	assert( pLevel->pSubSelObjects );

	pObject = ObjectList_GetFirst( pLevel->pSubSelObjects, &Iterator ) ;

	while( pObject != nullptr )
	{
		ObjModFlags = 0;
		ObjFlags = Object_GetMiscFlags( pObject );
		if( ObjFlags & SubSelect_Move )
			ObjModFlags |= SubSelect_Move;
		if( ObjFlags & SubSelect_Rotate )
			ObjModFlags |= SubSelect_Rotate;
		ModFlags &=  ObjModFlags;
		pObject = ObjectList_GetNext( pLevel->pSubSelObjects, &Iterator ) ;
	}
	return( ModFlags );
}

float Level_GetConstructorPlane( const Level * pLevel, int32 Index )
{
	assert( pLevel );
	assert( SIGNATURE == pLevel->nSignature ) ;

	return( jeVec3d_GetElement( &pLevel->ConstructLines, Index) );
}

LEVEL_UPDATE Level_GetBrushUpdate( const Level * pLevel )
{
	assert( pLevel );
	assert( SIGNATURE == pLevel->nSignature ) ;

	return( pLevel->BrushUpdate );
}

LEVEL_UPDATE Level_GetLightUpdate( const Level * pLevel )
{
	assert( pLevel );
	assert( SIGNATURE == pLevel->nSignature );

	return( pLevel->LightUpdate );
}

void Level_GetBSPBuildOptions( const Level * pLevel, jeBSP_Options * Options, jeBSP_Logic * Logic, jeBSP_LogicBalance * LogicBalance )
{
	assert( pLevel );
	assert( SIGNATURE == pLevel->nSignature ) ;

	
	*Options = pLevel->Options;
	*Logic = pLevel->Logic;
	*LogicBalance = pLevel->LogicBalance;
}

jeWorld	*	Level_GetjeWorld( const Level * pLevel )
{
	assert( pLevel );
	assert( SIGNATURE == pLevel->nSignature ) ;

	return( pLevel->pWorld );
}

#ifdef _USE_BITMAPS
jeBitmap *	Level_GetCurMaterialjeBitmap( const Level * pLevel )
{
	Material_Struct *	pCurMaterial;
	assert( pLevel );
	assert( SIGNATURE == pLevel->nSignature ) ;
	assert( pLevel->pGlobalMaterials );

	pCurMaterial = MaterialList_GetCurMaterial( pLevel->pGlobalMaterials );

	if( pCurMaterial == nullptr )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Level_GetCurMaterialgeBitmap:MaterialList_GetCurMaterial");
		return( nullptr );
	}
	return ( (jeBitmap *) Materials_GetBitmap( pCurMaterial ) );
}

jeBitmap * Level_GetMaterialBitmapByName( const Level * pLevel, char* szBitmapName )
{
	Material_Struct *	pMaterial;
	MaterialIterator	MI;

	pMaterial = MaterialList_SearchByName(pLevel->pGlobalMaterials, &MI, szBitmapName);

	return ( (jeBitmap *) Materials_GetBitmap( pMaterial ) );
}
#else
jeMaterialSpec *	Level_GetCurMaterialSpec( const Level * pLevel )
{
	Material_Struct *	pCurMaterial;
	assert( pLevel );
	assert( SIGNATURE == pLevel->nSignature ) ;

    CJweApp*            pApp = (CJweApp*)AfxGetApp();

	pCurMaterial = MaterialList_GetCurMaterial( pApp->GetMaterialList() );

	if( pCurMaterial == nullptr )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Level_GetCurMaterialgeBitmap:MaterialList_GetCurMaterial");
		return( nullptr );
	}
	return ( (jeMaterialSpec *) Materials_GetMaterialSpec( pCurMaterial ) );
}

jeMaterialSpec * Level_GetMaterialSpecByName( const Level * pLevel, char* szMatName )
{
	Material_Struct *	pMaterial;
	MaterialIterator	MI;
	CJweApp*            pApp = (CJweApp*)AfxGetApp();

	pMaterial = MaterialList_SearchByName(pApp->GetMaterialList(), &MI, szMatName);

	return ( (jeMaterialSpec*) Materials_GetMaterialSpec( pMaterial ) );
}
#endif

jeBoolean Level_GetBrushLighting( const Level * pLevel )
{
	return( pLevel->BrushLightIncremental );
}

jeBoolean	Level_GetCurCamXForm( const Level * pLevel, jeXForm3d * pXForm )
{
	if( pLevel->pCurCamera == nullptr )
	{
		return( JE_FALSE );
	}
	Camera_GetXForm( pLevel->pCurCamera, pXForm );

	return( JE_TRUE );
}

jeBoolean	Level_GetCurCamFOV( const Level * pLevel, float *pFOV )
{
	if( pLevel->pCurCamera == nullptr )
	{
		return( JE_FALSE );
	}
	*pFOV = Camera_GetFOV( pLevel->pCurCamera );

	return( JE_TRUE );
}

jeObject *	Level_GetCurCamObject( const Level * pLevel )
{
	if( pLevel->pCurCamera == nullptr )
		return( nullptr );

	return( Camera_GetjeObject( pLevel->pCurCamera ) );
}

void Level_GetCurCamXYRot( const Level * pLevel, float *XRot, float *YRot )
{
	if( pLevel->pCurCamera == nullptr )
		return;

	*XRot = Camera_GetCurCamX( pLevel->pCurCamera );
	*YRot = Camera_GetCurCamY( pLevel->pCurCamera );
}

const jeExtBox *	Level_GetCurCamBounds( const Level * pLevel )
{
	assert( pLevel );
	assert( pLevel->pCurCamera );

	return( Camera_GetWorldAxialBounds( pLevel->pCurCamera ) );
}

jeBoolean Level_HasSelections( const Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return (ObjectList_GetNumItems( pLevel->pSelObjects )) ? JE_TRUE : JE_FALSE ;
}// Level_HasSelections

jeBoolean Level_HasSubSelections( const Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return (ObjectList_GetNumItems( pLevel->pSubSelObjects )) ? JE_TRUE : JE_FALSE ;
}// Level_HasSubSelections

const jeExtBox * Level_GetSelBounds( const Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;
	
	//if( JE_TRUE == pLevel->bDirty )
	{
		Level * pNCLevel = (Level*)pLevel ;	// Lazy eval forces this
		Util_ExtBox_SetInvalid( &pNCLevel->SelBounds ) ;
		ObjectList_GetListBounds( pNCLevel->pSelObjects, &pNCLevel->SelBounds ) ;

		pNCLevel->bDirty = JE_FALSE ;
	}
	return &pLevel->SelBounds ;
}// Level_GetSelBounds

const jeExtBox * Level_GetSelDrawBounds( const Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;
	
	//if( JE_TRUE == pLevel->bDirty )
	{
		Level * pNCLevel = (Level*)pLevel ;	// Lazy eval forces this
		Util_ExtBox_SetInvalid( &pNCLevel->SelBounds ) ;
		ObjectList_GetListDrawBounds( pNCLevel->pSelObjects, &pNCLevel->SelBounds ) ;

		pNCLevel->bDirty = JE_FALSE ;
	}
	return &pLevel->SelBounds ;
}// Level_GetSelDrawBounds

const jeExtBox * Level_GetSubSelDrawBounds( const Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;
	
	//if( JE_TRUE == pLevel->bDirty )
	{
		Level * pNCLevel = (Level*)pLevel ;	// Lazy eval forces this
		Util_ExtBox_SetInvalid( &pNCLevel->SelBounds ) ;
		ObjectList_GetListDrawBounds( pNCLevel->pSubSelObjects, &pNCLevel->SelBounds ) ;

		pNCLevel->bDirty = JE_FALSE ;
	}
	return &pLevel->SelBounds ;
}// Level_GetSelDrawBounds

jeBoolean Level_GetSelBoundsCenter( const Level * pLevel, jeVec3d * const pCenter )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;
	assert( pCenter != nullptr ) ;

	Level_GetSelBounds( pLevel ) ;	// Force lazy eval

	if( !jeExtBox_IsValid( &pLevel->SelBounds ) )
		return( JE_FALSE );
	jeExtBox_GetTranslation( &pLevel->SelBounds, pCenter ) ; 
	return( JE_TRUE );
}// Level_GetSelBoundsCenter


typedef struct IdSearchContext {
	const char * Name;
	int32 MaxId;
} IdSearchContext;

jeBoolean Level_GetMaxIdCB( Object* pObject, void * pVoid )
{
	IdSearchContext *pSearchContext = (IdSearchContext*)pVoid;

	if( strcmp( Object_GetName( pObject ), pSearchContext->Name ) )
		return( JE_TRUE );
	if( Object_GetNameTag( pObject ) > pSearchContext->MaxId )
		pSearchContext->MaxId = Object_GetNameTag( pObject );
	return( JE_TRUE );
}

int32 Level_GetNextObjectId( Level * pLevel, OBJECT_KIND Kind, const char* Name )
{
	IdSearchContext SearchContext;

	SearchContext.MaxId = 0;
	SearchContext.Name = Name;
	switch( Kind )
	{
		case KIND_BRUSH:
			Level_EnumBrushes( pLevel, &SearchContext, (BrushListCB)Level_GetMaxIdCB ) ;
			break;

		case KIND_LIGHT:
			LightList_EnumLights( pLevel->pLightList, &SearchContext, (LightListCB)Level_GetMaxIdCB ) ;
			break;

		case KIND_CAMERA:
			CameraList_EnumCameras( pLevel->pCameraList, &SearchContext, (CameraListCB)Level_GetMaxIdCB ) ;
			break;


		case KIND_USEROBJ:
			ObjectList_EnumObjects( pLevel->pUserObjList, &SearchContext, Level_GetMaxIdCB ) ;
			break;


		case KIND_MODEL:
			ModelList_EnumModels( pLevel->pModels, &SearchContext, (ModelListCB)Level_GetMaxIdCB ) ;
			break;

		default:
			break;
	}
	return( SearchContext.MaxId+1 );
}

// IS

jeBoolean Level_IsObjectVisible( const Level * pLevel, const Object * pObject )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	// Test group visiblity
	// Test group with this brush
	pLevel;
	pObject;
	return JE_TRUE ;
}// Level_IsBrushVisible

jeBoolean Level_IsSelected( Level * pLevel, Object * pObject )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;
	assert( pObject != nullptr ) ;
	assert( pLevel->pSelObjects );
	
	return (ObjectList_Find( pLevel->pSelObjects, pObject ) == nullptr ) ? JE_FALSE : JE_TRUE ;
}// Level_IsSelected

jeBoolean Level_IsSubSelected( Level * pLevel, Object * pObject )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;
	assert( pObject != nullptr ) ;
	assert( pLevel->pSubSelObjects );
	
	return (ObjectList_Find( pLevel->pSubSelObjects, pObject ) == nullptr ) ? JE_FALSE : JE_TRUE ;
}// Level_IsSelected

jeBoolean Level_IsSnapGrid( const Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->bSnapToGrid ;
}//Level_IsSnapGrid

jeBoolean Level_HasChanged( const Level * pLevel )
{
	assert( pLevel );

	return( pLevel->bChanged );
}

Undo *Level_GetUndo( const Level * pLevel )
{
	return( pLevel->pUndo );
}

const jeFaceInfo * Level_GetCurFaceInfo( const Level * pLevel )
{
	return( &pLevel->DefaultFace.FaceInfo );
}


//
// STATE CHANGES
//
void Level_SetChanged( Level * pLevel, jeBoolean bChanged )
{
	assert( pLevel );

	pLevel->bChanged = bChanged;
}

void	Level_SetCurCamXYRot( const Level * pLevel, float XRot, float YRot )
{
	if( pLevel->pCurCamera == nullptr )
		return;
	Camera_SetCurCamY( pLevel->pCurCamera, YRot );
	Camera_SetCurCamX( pLevel->pCurCamera, XRot );
}
static jeBoolean Level_ClearMiscFlagsCB( Object *pObject, void* lParam )
{
	Object_ClearMiscFlags( pObject, (const uint32)lParam ) ;
	return JE_TRUE ;
}// Level_ClearMiscFlagsCB


void Level_ClearMiscFlags( Level * pLevel, const uint32 nFlags )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	Level_EnumSelected( pLevel, (void*)nFlags, Level_ClearMiscFlagsCB ) ;

}// Level_ClearMiscFlags

static jeBoolean Level_ResetSelFaceCB( Object * pObject, void * Context )
{
	Context;
	if( Object_GetKind( pObject ) != KIND_BRUSH )
		return( JE_TRUE );

	Brush_ResetSelFace( (Brush*)pObject );
	return( JE_TRUE );
}

static void Level_ResetSelFace( Level * pLevel )
{
	Level_EnumSelected( pLevel, nullptr, Level_ResetSelFaceCB );
}
void Level_RebuildAll( Level * pLevel, jeBSP_Options Options, jeBSP_Logic Logic, jeBSP_LogicBalance LogicBalance )
{
	ModelIterator MI;
	Model * pModel;

	pModel = ModelList_GetFirst( pLevel->pModels, &MI );
	while( pModel )
	{
		jeModel_RebuildBSP
		(
			Model_GetguModel(pModel ), 
			Options,  
			Logic, 
			LogicBalance
		) ;
		jeModel_RebuildLights( Model_GetguModel(pModel) );
		pModel = ModelList_GetNext( pLevel->pModels, &MI );
	}
	Level_ResetSelFace( pLevel );
}// Level_RebuildAll

void Level_RebuildLights( Level * pLevel )
{
	ModelIterator MI;
	Model * pModel;

	pModel = ModelList_GetFirst( pLevel->pModels, &MI );
	while( pModel )
	{
		jeModel_RebuildLights(Model_GetguModel(pModel) );
		pModel = ModelList_GetNext( pLevel->pModels, &MI );
	}
}

void Level_RebuildBSP( Level * pLevel, jeBSP_Options Options, jeBSP_Logic Logic, jeBSP_LogicBalance LogicBalance )
{
	ModelIterator MI;
	Model * pModel;

	pModel = ModelList_GetFirst( pLevel->pModels, &MI );
	while( pModel )
	{
		jeModel_RebuildBSP
		(
			Model_GetguModel(pModel ), 
			Options,  
			Logic, 
			LogicBalance
		) ;
		pModel = ModelList_GetNext( pLevel->pModels, &MI );
	}
	Level_ResetSelFace( pLevel );
}
static jeBoolean Levelt_SetMiscFlagsCB( Object *pObject, void* lParam )
{
	Object_SetMiscFlags( pObject, (const uint32)lParam ) ;
	return JE_TRUE ;
}// Levelt_SetMiscFlagsCB

void Level_SetMiscFlags( Level * pLevel, const uint32 nFlags )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	Level_EnumSelected( pLevel, (void*)nFlags, Levelt_SetMiscFlagsCB ) ;

}// Level_SetMiscFlags

void Level_SetModifiedSelection( Level * pLevel )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	pLevel->bDirty = JE_TRUE ;
}// Level_SetModifiedSelection

void Level_SetSnapGrid( Level * pLevel, jeBoolean bState )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	pLevel->bSnapToGrid = bState ;
}// Level_SetSnapGrid

void Level_SetGridSnapSize( Level * pLevel, int32 nSnapSize )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;
	
	pLevel->nGridSnapSize = nSnapSize ;
}// Level_SetGridSnapSize

void Level_SetRotateSnapSize( Level * pLevel, int32 nSnapSize )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;
	
	pLevel->nRotateSnapSize = nSnapSize ;
}// Level_SetRotateSnapSize

Group * Level_AddGroup( Level * pLevel, const char * pszName )
{
	Group * pGroup;

	pGroup = Group_Create( pszName );
	if( pGroup == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_AddGroup:Group_Create");
		return( nullptr );
	}
	if( GroupList_Append( pLevel->pGroups, pGroup ) == LIST_INVALID_NODE )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_AddGroup:GroupList_Append");
		Group_Destroy( &pGroup );
		return( nullptr );
	}
	return( pGroup );
}

Model *	Level_AddModel( Level * pLevel, const char * pszName )
{
	Model * pModel;
	int32 nNumber;

	assert( pLevel );
	assert( pszName );

	nNumber = Level_GetNextObjectId( pLevel, KIND_MODEL, pszName );
	pModel = Model_Create( pLevel->CurrentGroup, pszName, nNumber );
	if( pModel == nullptr)
		return( nullptr );

	jeModel_SetDefaultContents( Model_GetguModel(pModel ), JE_BSP_CONTENTS_AIR );

	// This was commented out (but why), added it again JH 25.4.2000
	if( jeWorld_AddObject( pLevel->pWorld, Model_GetjeObject( pModel ) )== JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_AddModel:jeWorld_AddObject" );
		return( nullptr );
	}
	// EOF JH

	if( ModelList_Append( pLevel->pModels, pModel ) == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_AddModel:ModelList_Append" );
		return( nullptr );
	}
	if( !Group_AddObject( pLevel->CurrentGroup, (Object *)pModel ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_AddModel:Group_AddObject" );
		return( nullptr );
	}
	Object_SetInLevel( (Object*)pModel, JE_TRUE );
	jeObject_AddChild( Model_GetjeObject( pLevel->ParentModel), Model_GetjeObject( pModel ) );
	return( pModel );
}

Class * Level_AddClass( Level * pLevel, const char * pszName, int Kind )
{
	Class * pClass;

	assert( pLevel );
	assert( pLevel->pClassList );
	assert( pszName );

	pClass = Class_Create( pszName, Kind );
	if( pClass == nullptr )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Level_AddClass:Class_Create" );
		return( nullptr );
	}
	
	ObjectList_Append( pLevel->pClassList, (Object*)pClass );
	return( pClass );
}

void Level_ModelLock( Level * pLevel, Model * pModel, jeBoolean bLock )
{
	BrushList	*pBrushList;
	BrushIterator  BI;
	Brush *	pBrush;
	jeBoolean bSelect = JE_FALSE;

	pBrushList = Model_GetBrushList( pModel );
	assert( pBrushList );
	if( bLock )
	{
		pBrush = BrushList_GetFirst( pBrushList, &BI );
		while( pBrush )
		{
			if( Level_IsSelected( pLevel, (Object*)pBrush ) )
			{
				Level_SelectObject( pLevel, (Object*)pBrush, LEVEL_DESELECT );
				bSelect = JE_TRUE;
			}
			pBrush = BrushList_GetNext( pBrushList, &BI );
		}
		Model_SetLocked( pModel, JE_TRUE );
	}
	else
		Model_SetLocked( pModel, JE_FALSE );
}
		

void	Level_SetCurrentGroup( Level * pLevel, Group * pGroup )
{
	assert( pLevel );
	assert( pGroup );

	pLevel->CurrentGroup = pGroup ;
}

void Level_SetCurrentModel( Level * pLevel, Model * pModel )
{
	assert( pLevel );
	assert( pModel );

	if( pLevel->pCurrentModel )
		Object_Free( (Object**)&pLevel->pCurrentModel );
	pLevel->pCurrentModel = pModel;
	Object_AddRef( (Object*)pLevel->pCurrentModel );
}

void Level_SetConstructor( Level * pLevel, int Index, float Value  )
{
	jeVec3d_SetElement( &pLevel->ConstructLines, Index, Value );
}

void Level_SetBrushUpdate( Level * pLevel, int Update )
{
	pLevel->BrushUpdate = (LEVEL_UPDATE) Update;
}

void Level_SetLightUpdate( Level * pLevel, int Update )
{
	pLevel->LightUpdate = (LEVEL_UPDATE) Update;
}

void Level_SetBrushLighting( Level * pLevel, int BrushLighting )
{
	pLevel->BrushLightIncremental = BrushLighting;
}

typedef struct UpdateObject_Struct
{
	jeBoolean bBrushLighting;
	jeBoolean bDirtyOveride;
} UpdateObject_Struct;

jeBoolean Level_UpdateObjectCB( Object * pObject , void * lParam )
{
	UpdateObject_Struct *UpdateObjectInfo = (UpdateObject_Struct*)lParam;
	assert( pObject );

	Object_Update( pObject, OBJECT_UPDATE_MANUEL, UpdateObjectInfo->bDirtyOveride);
	return( JE_TRUE);

}// Level_SelectObjectCB


void Level_UpdateAll( Level * pLevel )
{
	UpdateObject_Struct UpdateObjectInfo;

	UpdateObjectInfo.bBrushLighting = pLevel->BrushLightIncremental;
	UpdateObjectInfo.bDirtyOveride = JE_FALSE;
	Level_EnumObjects( pLevel, (void*)&UpdateObjectInfo, Level_UpdateObjectCB);
}

void Level_UpdateSelected( Level * pLevel )
{
	UpdateObject_Struct UpdateObjectInfo;

	UpdateObjectInfo.bBrushLighting = pLevel->BrushLightIncremental;
	UpdateObjectInfo.bDirtyOveride = JE_TRUE;
	Level_EnumSelected( pLevel, (void*)&UpdateObjectInfo, Level_UpdateObjectCB);
}

jeBoolean Level_RotCurCamX( const Level * pLevel, float Radians )
{
	assert( pLevel != nullptr );
	assert( pLevel->pCurCamera != nullptr );

	Camera_RotCurCamX( pLevel->pCurCamera, Radians );
	return( JE_TRUE );
}

jeBoolean	Level_RotCurCamY( const Level * pLevel, float Radians )
{
	assert( pLevel != nullptr );
	assert( pLevel->pCurCamera != nullptr );

	Camera_RotCurCamY( pLevel->pCurCamera, Radians );
	return( JE_TRUE );
}

jeBoolean Level_TranslateCurCam( const Level * pLevel, jeVec3d * Offset )
{
	assert( pLevel != nullptr );
	assert( pLevel->pCurCamera != nullptr );

	Camera_TranslateCurCam( pLevel->pCurCamera, Offset );
	return( JE_TRUE );
}

jeBoolean Level_SetRenderMode( Level * pLevel, int Mode )
{
	assert( pLevel );
	assert( pLevel->pWorld );

	return( jeModel_SetRenderOptions( Model_GetguModel(pLevel->pCurrentModel), (jeBSP_RenderMode) Mode ) );
}

void Level_SetBSPBuildOptions( Level * pLevel, jeBSP_Options  Options, jeBSP_Logic  Logic, jeBSP_LogicBalance  LogicBalance )
{
	pLevel->Options = Options;
	pLevel->Logic = Logic;
	pLevel->LogicBalance = LogicBalance;
}

void Level_RenameSelected( Level * pLevel, char * Name )
{
	Object * pObject;
	ObjectIterator Interator;
	int ObjectId;
	jeProperty_Data Data;

	Data.String = Name;

	pObject = ObjectList_GetFirst( pLevel->pSelObjects, &Interator );
	while( pObject )
	{
		if( strcmp( Name, Object_GetName( pObject ) ) )
		{
			ObjectId = Level_GetNextObjectId( pLevel, Object_GetKind( pObject ), Name );
			Object_SetName( pObject, Name, ObjectId );
			Object_SetProperty( pObject,  OBJECT_NAME_FIELD, PROPERTY_STRING_TYPE, &Data, LEVEL_UPDATE_MANUEL, LEVEL_UPDATE_MANUEL, JE_FALSE);
		}
		pObject = ObjectList_GetNext( pLevel->pSelObjects, &Interator );
	}
}


//
// BRUSH MANIPULATION
//

Object* Level_NewBrush( Level * pLevel, BRUSH_KIND BrushKind, BRUSH_TYPE eAddType,  const jeExtBox * pBrushBounds ) 
{
	Brush * pBrush ;
	char  * Name;
	int32	nNumber;
	BrushTemplate *  pTemplate; 
	jeXForm3d		 XForm;
	jeVec3d			 Scale;
	jeVec3d			 Pos;
	jeBoolean		 bUpdate;	
	jeBoolean		 bLightUpDate;


	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	Name = Object_CreateDefaultName( KIND_BRUSH, BrushKind );
	nNumber = Level_GetNextObjectId( pLevel, KIND_BRUSH, Name );
	Level_SetFaceInfoToCurMaterial( pLevel );
	pTemplate = BrushTemplate_Create( BrushKind );
	if( pTemplate == nullptr )
	{
		// [MLB-ICE]
		jeRam_Free(Name);	// Icestorm: Someone doesn't like names?
		// [MLB-ICE] EOB

		return( nullptr );
	}
	pBrush = Brush_FromTemplate( pTemplate, pLevel->CurrentGroup, Name, nNumber, &pLevel->DefaultFace.FaceInfo, eAddType ) ;

	// [MLB-ICE]
	jeRam_Free(Name);	// Icestorm: Someone doesn't like names?
	// [MLB-ICE] EOB

	if( pBrush == nullptr )
		return nullptr ;

	jeExtBox_GetTranslation ( pBrushBounds, &Pos );
	jeExtBox_GetScaling( pBrushBounds, &Scale );
	jeXForm3d_SetScaling( &XForm, Scale.X, Scale.Y, Scale.Z);
	jeXForm3d_Translate( &XForm, Pos.X, Pos.Y, Pos.Z );
	Brush_SetXForm( pBrush, &XForm );
	if( Model_AddBrush( pLevel->pCurrentModel, pBrush ) == JE_FALSE )
	{
		Object_Free( (Object**)&pBrush ) ;
		return nullptr ;
	}
	if( pLevel->BrushUpdate == LEVEL_UPDATE_CHANGE )
	{
		bUpdate = JE_TRUE;
		bLightUpDate = pLevel->BrushLightIncremental;
	}
	else
	{
		bUpdate = JE_FALSE;
		bLightUpDate = JE_FALSE;
		Object_Dirty( (Object*)pBrush );
	}
	if( Model_AddBrushWorld( pLevel->pCurrentModel, pBrush, bUpdate, bLightUpDate ) == JE_FALSE )
	{
		Object_Free( (Object**)&pBrush ) ;
		return nullptr ;
	}
	Brush_AttachWorld( pBrush, pLevel->pWorld );
	Undo_Push( pLevel->pUndo, UNDO_CREATE );
	Undo_AddSubTransaction( pLevel->pUndo, UNDO_CREATEOBJECT, (Object*)pBrush, pLevel );
	Object_Free( (Object**)&pBrush );

	return (Object*)pBrush ;

}// Level_NewBrush


Object * Level_NewUserObject( Level * pLevel, const char * TypeName, const jeExtBox * pBrushBounds )
{
	jeObject * pgeObject;
	Object * pObject;
	char * Name;
	int nNumber;
	jeXForm3d XF;
	jeVec3d Center;

	pgeObject = jeObject_Create( TypeName );
	if( pgeObject == nullptr )
		return( nullptr );

	Name = Util_StrDup( TypeName );
	nNumber = Level_GetNextObjectId( pLevel, KIND_USEROBJ, Name );
	pObject = (Object*)UserObj_Create( Name, pLevel->CurrentGroup, nNumber, pgeObject );
	if( pObject == nullptr )
	{
		jeObject_Destroy( &pgeObject );
		jeRam_Free( Name );
	}
	//Royce
	jeWorld_AddObject(pLevel->pWorld, pgeObject);

	jeExtBox_GetTranslation( pBrushBounds, &Center );
	jeXForm3d_SetTranslation( &XF, Center.X, Center.Y, Center.Z );
	UserObj_SetXForm( (UserObj*)pObject, &XF );
	ObjectList_Append( pLevel->pUserObjList, pObject );
	if( pObject )
	{
		Object_SetInLevel( pObject, JE_TRUE );
		Group_AddObject( pLevel->CurrentGroup ,pObject );
	}
	Undo_Push( pLevel->pUndo, UNDO_CREATE );
	Undo_AddSubTransaction( pLevel->pUndo, UNDO_CREATEOBJECT, (Object*)pObject, pLevel );

	//jeObject_AddChild( Model_GetjeObject( pLevel->ParentModel), pgeObject);
	//----
	return( pObject );
}

Object * Level_NewObject( Level * pLevel, int Kind, int SubKind,  const jeExtBox * pBrushBounds )
{
	Object * pObject = nullptr;
	jeVec3d	WorldPt;


	assert( pLevel != nullptr );
	assert( SIGNATURE == pLevel->nSignature ) ;

	switch( Kind )
	{
	case KIND_BRUSH:
		pObject = Level_NewBrush( pLevel, (BRUSH_KIND) SubKind, BRUSH_ADD, pBrushBounds );
		break;

	case KIND_LIGHT:
		jeExtBox_GetTranslation ( pBrushBounds, &WorldPt );
		pObject = Level_NewLight( pLevel, &WorldPt ) ;
		break;

	case KIND_CAMERA:
		jeExtBox_GetTranslation ( pBrushBounds, &WorldPt );
		pObject = Level_NewCamera( pLevel, &WorldPt );
		jeObject_AddChild( Model_GetjeObject( pLevel->ParentModel), Camera_GetjeObject((Camera*)pObject));
		break;

	default:
		assert( 0 );
	}
	if( pObject )
	{
		Object_SetInLevel( pObject, JE_TRUE );
		Group_AddObject( pLevel->CurrentGroup ,pObject );
	}
	return( pObject );
}

Object * Level_SubtractBrush( Level * pLevel, int SubKind,  const jeExtBox * pBrushBounds ) 
{
	Object * pObject;


	pObject = Level_NewBrush( pLevel, (BRUSH_KIND) SubKind, BRUSH_SUBTRACT, pBrushBounds );
	if( pObject != nullptr )
	{
		Object_SetInLevel( pObject, JE_TRUE );
		Group_AddObject( pLevel->CurrentGroup,pObject );
	}
	return( pObject );
}

static jeBoolean Level_AddBrush( Level * pLevel, Brush * pBrush )
{
	assert( pLevel != nullptr ) ;
	assert( pBrush != nullptr );

	if( Model_AddBrush( pLevel->pCurrentModel, pBrush ) == JE_FALSE )
	{
		return JE_FALSE ;
	}
	return( JE_TRUE );
}// Level_AddBrush

// Adds a version of the object that is not in the world to the world
jeBoolean Level_AddObject( Level * pLevel, Object* pObject ) 
{
	assert( pLevel != nullptr ) ;
	assert( pObject != nullptr );
	assert( SIGNATURE == pLevel->nSignature ) ;


	switch( Object_GetKind( pObject ) )
	{
		case KIND_BRUSH:
			if( !Level_AddBrush( pLevel, (Brush*)pObject ) )
				return( JE_FALSE );
			break;

		case KIND_LIGHT:
			LightList_Append( pLevel->pLightList,(Light*)pObject );
			break;

		case KIND_CAMERA:
			CameraList_Append( pLevel->pCameraList,(Camera*)pObject );
			break;

		case KIND_USEROBJ:
			ObjectList_Append( pLevel->pUserObjList,pObject );
			break;

		case KIND_MODEL:
			ModelList_Append( pLevel->pUserObjList, (Model*)pObject);
			Model_RestoreBrush( (Model*)pObject);
			break;

		default:
			assert( 0 );
			break;
	}
	Group_AddObject( Object_GetGroup( pObject ), pObject );
	Object_SetInLevel( pObject, JE_TRUE );
	if( !Level_AddToWorld( pLevel, pObject, LEVEL_UPDATE_CHANGE) )
	{
		return( JE_FALSE );
	}
	return JE_TRUE ;

}// Level_AddObject

void Level_DeleteObject( Level * pLevel, Object* pObject )
{
	jeBoolean bDeleted = JE_FALSE;
	Model * pModel;
	Group * pGroup;
	assert( pLevel != nullptr ) ;
	assert( pObject != nullptr );
	assert( SIGNATURE == pLevel->nSignature ) ;


	switch( Object_GetKind( pObject ) )
	{
		case KIND_BRUSH:
			pModel = Brush_GetModel((Brush*)pObject );
			if( pModel )
				Model_RemoveBrush( pModel,(Brush*)pObject );
			Brush_DeselectAllVert( (Brush*)pObject );
			Brush_DeselectAllFaces( (Brush*)pObject );
			//Object_Free( &pObject );
			bDeleted = JE_TRUE;
			break;

		case KIND_LIGHT:
			Light_RemoveFromWorld( (Light*)pObject );
			LightList_DeleteLight( pLevel->pLightList, (Light*)pObject );
			bDeleted = JE_TRUE;
			break;

		case KIND_CAMERA:
			if( CameraList_GetNumItems( pLevel->pCameraList ) > 1 )
			{
				//jeWorld_RemoveObject( pLevel->pWorld, Camera_GetjeObject( (Camera*)pObject ) );
				jeObject_RemoveChild( Model_GetjeObject(pLevel->ParentModel), Camera_GetjeObject( (Camera*)pObject ) );
				CameraList_DeleteCamera( pLevel->pCameraList, (Camera*)pObject );
				bDeleted = JE_TRUE;
			}
			else
				jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "Level_DeleteObject:KIND_CAMERA", "Must have one camera" );
			break;

		case KIND_USEROBJ:
			ObjectList_Remove( pLevel->pUserObjList, pObject ) ;
			//Royce
			//jeObject_RemoveChild( Model_GetjeObject(pLevel->ParentModel), UserObj_GetjeObject( (UserObj*)pObject ) );
			UserObj_RemoveFromWorld( (UserObj*)pObject, pLevel->pWorld );
			Object_Free( &pObject ) ;
			//-----
			bDeleted = JE_TRUE;
			break;

		case KIND_MODEL:
			if( ModelList_GetNumItems( pLevel->pModels ) > 1 )
			{
				Model_FreeBrushList( (Model*)pObject );
				ModelList_Remove( pLevel->pModels, (Model*)pObject );
				if( pLevel->pCurrentModel == (Model*)pObject )
				{
					ModelIterator MI;
					Object_Free( &pObject );
					pLevel->pCurrentModel = ModelList_GetFirst( pLevel->pModels, &MI ) ;
					if( pLevel->pCurrentModel )
						Object_AddRef( (Object*)pLevel->pCurrentModel );
				}
				
				//jeWorld_RemoveObject( pLevel->pWorld, Model_GetjeObject( (Model*)pObject ) );
				jeObject_RemoveChild( Model_GetjeObject(pLevel->ParentModel), Model_GetjeObject( (Model*)pObject ) );
				Object_SetInLevel( pObject, JE_FALSE );
				bDeleted = JE_TRUE;
			}
			else
				jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "Level_DeleteObject:KIND_MODEL", "Must have one model" );
			break;

		default:
			assert( 0 );
			break;
	}
	if( bDeleted )
	{
		pGroup = Object_GetGroup( pObject );
		if( pGroup )
			Group_RemoveObject( pGroup, pObject );
		Object_SetInLevel( pObject, JE_FALSE );
	}

	
}// Level_DeleteObject

jeBoolean Level_SelectObjectCB( Object * pObject , void * lParam )
{
	SelectObjectInfo *psoi = (SelectObjectInfo *)lParam;

	assert( pObject );
	assert( lParam );



	return( Level_SelectObject( psoi->pLevel, pObject, psoi->eState ) );

}// Level_SelectObjectCB

jeBoolean Level_SelectGroup( Level * pLevel, Group * pGroup, LEVEL_STATE eState )
{
	ObjectList * pObjectList;
	SelectObjectInfo soi;

	assert( pLevel );
	assert( pGroup );

	pObjectList = Group_GetObjectList( pGroup );
	assert( pObjectList );

	soi.eState = eState;
	soi.pLevel = pLevel;

	return( ObjectList_EnumObjects( pObjectList, &soi, Level_SelectObjectCB ) );
}


jeBoolean Level_SelectObject( Level * pLevel, Object * pObject , LEVEL_STATE eState )
{
	jeBoolean			bSuccess ;	// Means that no alloc err occurred
	Model * pModel;
	
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	bSuccess = JE_TRUE ;
	
	if( Object_GetKind( pObject )== KIND_BRUSH )
	{
		pModel = Brush_GetModel( (Brush *)pObject );
		if( pModel && Model_IsLocked( pModel ) )
			pObject = (Object*)pModel;
	}

	if( LEVEL_TOGGLE == eState )
	{
		if( ObjectList_Find( pLevel->pSelObjects, pObject ) != nullptr )
			eState = LEVEL_DESELECT ;
		else
			eState = LEVEL_SELECT ;
	}

	switch( eState )
	{
	case LEVEL_SELECT :
	case LEVEL_NOFACESELECT:
		if( !Level_IsSelected( pLevel, pObject ) )
		{
			bSuccess = ObjectList_AppendNoDup( pLevel->pSelObjects, pObject ) ;
			Object_AddRef( pObject );
			if( Object_GetKind(pObject ) == KIND_BRUSH && eState != LEVEL_NOFACESELECT )
			{
				Brush_SelectAllFaces( (Brush *)pObject  );
			}
			if( Object_GetKind(pObject ) == KIND_CAMERA  )
			{
				pLevel->pCurCamera = (Camera*)pObject;
			}
		}
		break ;
	case LEVEL_DESELECT :
		if( Level_IsSelected( pLevel, pObject ) )
		{
			if( Object_GetKind(pObject ) == KIND_BRUSH && eState != LEVEL_NOFACESELECT )
			{
				Brush_DeselectAllFaces( (Brush *)pObject  );
			}
			ObjectList_Remove( pLevel->pSelObjects, pObject ) ;

			Object_Free( &pObject );
		}
		break ;
	}
	

	
	Level_SetSelType( pLevel ) ;
	Level_SetModifiedSelection( pLevel ) ;

	return bSuccess ;
}// Level_SelectObject


jeBoolean Level_SubSelectObject( Level * pLevel, Object * pObject , LEVEL_STATE eState )
{
	jeBoolean			bSuccess ;	// Means that no alloc err occurred

	
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	bSuccess = JE_TRUE ;
	
	if( LEVEL_TOGGLE == eState )
	{
		if( ObjectList_Find( pLevel->pSubSelObjects, pObject ) != nullptr )
			eState = LEVEL_DESELECT ;
		else
			eState = LEVEL_SELECT ;
	}

	switch( eState )
	{
	case LEVEL_SELECT :
	case LEVEL_NOFACESELECT:
		if( !Level_IsSubSelected( pLevel, pObject ) )
		{
			bSuccess = ObjectList_AppendNoDup( pLevel->pSubSelObjects, pObject ) ;
			Object_AddRef( pObject );
		}
		break ;
	case LEVEL_DESELECT :
		if( Level_IsSubSelected( pLevel, pObject ) )
		{
			ObjectList_Remove( pLevel->pSubSelObjects, pObject ) ;
			Object_Free( &pObject );
		}
		break ;
	}
	

	
	Level_SetSelType( pLevel ) ;
	Level_SetModifiedSelection( pLevel ) ;

	return bSuccess ;
}// Level_SelectObject

jeBoolean  Level_DeselectAllSub( Level * pLevel, jeExtBox * pWorldBounds )
{
	ObjectIterator  Iterator;
	ObjectIterator  NextIterator;
	Object * pObject = nullptr;
	Object * pNextObject = nullptr;
	jeExtBox	ObjExtBox;

	pObject = ObjectList_GetFirst( pLevel->pSubSelObjects, &Iterator ) ;
	NextIterator = Iterator;
	while( pObject != nullptr )
	{
		pNextObject = ObjectList_GetNext( pLevel->pSubSelObjects, &NextIterator ) ;
		if( Object_GetWorldDrawBounds( pObject, &ObjExtBox ) )
			Util_ExtBox_Union( pWorldBounds, &ObjExtBox, pWorldBounds );
		List_Remove (pLevel->pSubSelObjects, Iterator, nullptr );
		Object_Free( &pObject );
		pObject = pNextObject;
		Iterator = NextIterator;
	}
	return( JE_TRUE );
}

static jeBoolean Level_UnMarkAllSubCB( Object * pObject, void * Context )
{
	jeExtBox * pWorldBounds = (jeExtBox*)Context;
	jeExtBox	ObjExtBox;

	Object_ClearMiscFlags( pObject, AllSubSelect );
	if( Object_GetWorldDrawBounds( pObject, &ObjExtBox ) )
		Util_ExtBox_Union( pWorldBounds, &ObjExtBox, pWorldBounds );
	return( JE_TRUE );
}

jeBoolean Level_UnMarkAllSub( Level * pLevel, jeExtBox * pWorldBounds )
{
	Level_EnumObjects( pLevel, pWorldBounds, Level_UnMarkAllSubCB );
	return( JE_TRUE );
}


Object *Level_FindgeObject( Level * pLevel,  jeObject * pgeObject )
{
	ObjectIterator  Iterator;
	ModelIterator	ModelIterator;
	CameraIterator CameraIterator;
	Object * pObject = nullptr;
	Model * pModel;
	Camera * pCamera;

	pObject = ObjectList_GetFirst( pLevel->pUserObjList, &Iterator ) ;
	while( pObject != nullptr )
	{
		if( UserObj_GetjeObject( (UserObj *)pObject ) == pgeObject )
			return( pObject );
		pObject = ObjectList_GetNext( pLevel->pUserObjList, &Iterator ) ;
	}
	pModel =  ModelList_GetFirst( pLevel->pModels, &ModelIterator );
	while( pModel )
	{
		if( pgeObject == Model_GetjeObject( pModel ) )
			return((Object*)pModel );
		pModel =  ModelList_GetNext( pLevel->pModels, &ModelIterator );
	}
	pCamera = CameraList_GetFirst( pLevel->pCameraList, &CameraIterator );
	while( pCamera )
	{
		if( pgeObject == Camera_GetjeObject( pCamera ) )
			return((Object*)pCamera );
		pCamera = CameraList_GetNext( pLevel->pCameraList, &CameraIterator );
	}
	return( nullptr );
}

jeBoolean Level_SubSelectgeObject( Level * pLevel,  jeObject * pgeObject , LEVEL_STATE eState ) 
{
	Object * pObject;
	
	pObject = Level_FindgeObject( pLevel, pgeObject );
	if( pObject == nullptr )
		return( JE_FALSE );

	Level_SubSelectObject( pLevel, pObject, eState);
	return( JE_TRUE );
}

jeBoolean Level_MarkSubSelect( Level * pLevel,  jeObject * pgeObject , int32 flag ) 
{
	Object * pObject;
	
	pObject = Level_FindgeObject( pLevel, pgeObject );
	if( pObject == nullptr )
		return( JE_FALSE );
	
	Object_SetMiscFlags( pObject, flag );
	return( JE_TRUE );
}
void Level_SetSelType( Level * pLevel )
{
	int32		nSelType = 0 ;
	int32		nItems ;
	Object	*	pObject ;
	ObjectIterator    Iterator;

	nItems = ObjectList_GetNumItems( pLevel->pSelObjects ) ;
	if( nItems == 0 )
		nSelType = LEVEL_SELNONE ;
	else if( nItems == 1 )
	{
		pObject = ObjectList_GetFirst( pLevel->pSelObjects, &Iterator) ;

		switch( Object_GetKind( pObject ) )
		{
		case KIND_BRUSH :	 nSelType |= LEVEL_SELONEBRUSH;	break ;
		case KIND_MODEL :	nSelType |= LEVEL_SELONEMODEL ;	break ;
		case KIND_LIGHT :	nSelType |= LEVEL_SELONELIGHT ;	break ;
		case KIND_CAMERA :	nSelType |= LEVEL_SELONECAMERA ;	break ;
		case KIND_USEROBJ:	nSelType |= LEVEL_SELONEOBJECT;	break;
		case KIND_CLASS:	nSelType |= LEVEL_SELONECLASS;	break;
		}
	}
	else
	{
		SelectKindInfo	ski ;
		int32			nKinds ;
		memset( &ski, 0, sizeof ski ) ;
		Level_EnumSelected( pLevel, &ski, Level_SelectKindInfoCB ) ;
		nSelType = 0 ;
		nKinds = 0 ;
		if( ski.nBrushes > 0 )
		{
			nSelType |= LEVEL_SELBRUSHES ;
			nKinds++ ;
		}
		
		if( ski.nEntities > 0 )
		{
			nSelType |= LEVEL_SELENTITIES ;
			nKinds++ ;
		}

		if( ski.nLights > 0 )
		{
			nSelType |= LEVEL_SELLIGHTS ;
			nKinds++ ;
		}

		if( ski.nModels > 0 )
		{
			nSelType |= LEVEL_SELMODELS ;
			nKinds++ ;
		}

		if( ski.nCameras > 0 )
		{
			nSelType |= LEVEL_SELCAMERAS ;
			nKinds++ ;
		}

		if ( ski.nUserObjects > 0 )
		{
			nSelType |= LEVEL_SELOBJECTS;
			nKinds++;
		}

		if ( ski.nClass > 0 )
		{
			nSelType |= LEVEL_SELCLASS;
			nKinds++;
		}
		assert( nKinds != 0 ) ;
		if( nKinds > 1 )
			nSelType |= LEVEL_SELMANY ;
	}

	pLevel->SelType = (LEVEL_SEL) nSelType ;

}// Level_SetSelType


jeBoolean Level_DragBegin( Level * pLevel, Object* pObject )
{
	assert( pLevel != nullptr ) ;
	assert( pObject != nullptr );
	assert( SIGNATURE == pLevel->nSignature ) ;


	switch( Object_GetKind( pObject ) )
	{
		case KIND_BRUSH:
			//Model_RemoveBrushWorld( Brush_GetModel((Brush*)pObject ),(Brush*)pObject );
			break;

		case KIND_LIGHT:
			if( pLevel->LightUpdate == LEVEL_UPDATE_REALTIME )
			{
				Light_ChangeToDLight( (Light*)pObject );
			}
			break;

		case KIND_CAMERA:
		case KIND_USEROBJ:
		case KIND_MODEL:
			break;

		default:
			assert( 0 );
			break;
	}
	return( JE_TRUE );

}// Level_DragBegin

jeBoolean Level_AddToWorld( Level * pLevel, Object* pObject, int Update )
{
	jeBoolean bUpdate;

	assert( pLevel != nullptr ) ;
	assert( pObject != nullptr );
	assert( SIGNATURE == pLevel->nSignature ) ;


	switch( Object_GetKind( pObject ) )
	{
		case KIND_BRUSH:
			bUpdate = ( pLevel->BrushUpdate >= Update );
			Model_AddBrushWorld( pLevel->pCurrentModel,(Brush*)pObject, bUpdate, pLevel->BrushLightIncremental );
			Brush_AttachWorld( (Brush*)pObject, pLevel->pWorld );
			break;


		case KIND_LIGHT:
			Light_AddToWorld( (Light*)pObject );
			bUpdate = ( pLevel->LightUpdate >= Update );
			if( bUpdate )
				Light_UpdateData( (Light*)pObject );
			break;

		case KIND_CAMERA:
			break;

		case KIND_USEROBJ:
			UserObj_AddToWorld( (UserObj*)pObject, pLevel->pWorld );
			break;

		case KIND_MODEL:
			//jeWorld_AddObject( pLevel->pWorld, Model_GetjeObject((Model*)pObject)  );
			break;

		default:
			assert( 0 );
			break;
	}

		return( JE_TRUE );

}// Level_DragEnd

void Level_SelectFirstFace( Level * pLevel )
{
	Object * pObject;
	ObjectIterator    Iterator;

	pObject = ObjectList_GetFirst( pLevel->pSelObjects, &Iterator );

	assert( Object_GetKind( pObject ) == KIND_BRUSH );

	Brush_SelectFirstFace( (Brush*)pObject );
}


void Level_SelectLastFace( Level * pLevel )
{
	Object * pObject;

	pObject = ObjectList_GetLast( pLevel->pSelObjects );

	assert( Object_GetKind( pObject ) == KIND_BRUSH );

	Brush_SelectLastFace( (Brush*)pObject );
}

// ENUMERATION
//
int32 Level_EnumBrushes( Level * pLevel, void *lParam, BrushListCB Callback )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	//Level_RebuildAll( pLevel, BSP_OPTIONS_CSG_BRUSHES, Logic_Smart, 3 );
	
	return ModelList_EnumBrushes( pLevel->pModels, lParam, Callback ) ;
}// Level_EnumBrushes

int32 Level_EnumModels( Level * pLevel, void *lParam, ModelListCB Callback )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return ModelList_EnumModels( pLevel->pModels, lParam, Callback ) ;
}// Level_EnumModels

int32 Level_EnumSelected( Level * pLevel, void * lParam, ObjectListCB Callback )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return ObjectList_EnumObjects( pLevel->pSelObjects, lParam, Callback ) ;
}// Level_EnumSelected

int32 Level_EnumSubSelected( Level * pLevel, void * lParam, ObjectListCB Callback )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	return ObjectList_EnumObjects( pLevel->pSubSelObjects, lParam, Callback ) ;
}// Level_EnumSelected

//This must go down the various lists of objects and enum them.
//It is important that our callbacks check type first.
// It seems very easy that one of these call backs would expect an object type
// and get the wrong one by a call like this.
int32 Level_EnumObjects( Level * pLevel, void * lParam, ObjectListCB Callback )
{
	assert( pLevel != nullptr ) ;
	assert( SIGNATURE == pLevel->nSignature ) ;

	//Callback( pLevel->pTemplate, lParam ) ;
	ModelList_EnumBrushes( pLevel->pModels, lParam, (BrushListCB)Callback ) ;
	LightList_EnumLights( pLevel->pLightList, lParam, (LightListCB)Callback );
	CameraList_EnumCameras( pLevel->pCameraList, lParam, (CameraListCB)Callback );
	ObjectList_EnumObjects( pLevel->pUserObjList, lParam, Callback ) ;
	ModelList_EnumModels( pLevel->pModels, lParam, (ModelListCB)Callback ) ;
	return  0;
}// Level_EnumBrushes

static jeBoolean Level_NumberLightCB( Light * pLight, void * lParam )
{
	int32 *Counter = (int32*)lParam;

	Light_SetIndexTag( pLight, *Counter );
	*Counter += 1;
	return( JE_TRUE );
}// Level_NumberLightCB

static jeBoolean Level_NumberGroupsCB( Group * pGroup, void * lParam )
{
	int32 *Counter = (int32*)lParam;

	Group_SetIndexTag( pGroup, *Counter );
	*Counter += 1;
	return( JE_TRUE );
}// Level_NumberGroupsCB


jeBoolean Level_PrepareForSave( Level* pLevel )
{
	int32 Counter ;
	assert( pLevel != nullptr );
	assert( SIGNATURE == pLevel->nSignature ) ;

	Counter = 0 ;
	Level_EnumModels( pLevel, &Counter, ModelList_NumberModelsCB ) ;
	Counter = 0 ;
	LightList_EnumLights( pLevel->pLightList, &Counter, Level_NumberLightCB );
	Counter = 0 ;
	GroupList_EnumGroups( pLevel->pGroups, &Counter, Level_NumberGroupsCB );
	return( JE_TRUE );
}// Level_PrepareForSave


//
// FILE HANDLING
//
Level * Level_CreateFromFile( jeVFile * pF, jeWorld * pWorld, MaterialList_Struct * pGlobalMaterials, jePtrMgr * pPtrMgr, float Version )
{
	int32				nVersion ;
	Level			*	pLevel ;

	assert( jeVFile_IsValid( pF ) ) ;
	assert( pWorld != nullptr ) ;
	assert( pGlobalMaterials != nullptr ) ;

	pLevel = JE_RAM_ALLOCATE_STRUCT( Level ) ;
	if( pLevel == nullptr )
		goto LCFF_FAILURE ;

	memset( pLevel, 0, sizeof *pLevel ) ;
	assert( (pLevel->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN

	pLevel->SelType = LEVEL_SELNONE ;

	if( !jeVFile_Read( pF, &nVersion, sizeof nVersion ) )
		return nullptr;
	if( nVersion != LEVEL_VERSION )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "Level_CreateFromFile Version.\n", nullptr);
		return nullptr;
	}
	
	if( !Level_InitWorldData( pLevel, pWorld, pGlobalMaterials ) )
		goto LCFF_FAILURE ;

	Level_InitDefaultFace( pLevel );
	if( !Level_LoadLists( pLevel, pF, pPtrMgr ) )
		goto LCFF_FAILURE ;


	if( !Level_LoadPrefs( pLevel, pF, Version ) )
		goto LCFF_FAILURE ;


	pLevel->pUndo = Undo_Create( LEVEL_DEFAULTUNDODEPTH ) ;
	if( pLevel->pUndo == nullptr )
		goto LCFF_FAILURE ;
	
	Level_InitUndoFunctions( pLevel );

	return( pLevel );
LCFF_FAILURE :
	if( pLevel != nullptr )
		Level_Destroy( &pLevel ) ;

	jeErrorLog_AddString(JE_ERR_FILEIO_READ, "Level_CreateFromFile.\n", nullptr);
	return nullptr ;
}// Level_CreateFromFile



jeBoolean Level_WriteToFile( Level * pLevel, jeVFile * pF, jePtrMgr * pPtrMgr )
{
	int32	nVersion ;
	assert( pLevel != nullptr );
	assert( SIGNATURE == pLevel->nSignature ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	nVersion = LEVEL_VERSION ;
	if( jeVFile_Write( pF, &nVersion, sizeof nVersion ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_WriteToFile.\n", nullptr);
		return JE_FALSE;
	}

	if( !Level_SaveLists( pLevel, pF, pPtrMgr ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_WriteToFile.\n", nullptr);
		return JE_FALSE;
	}

	if( !Level_SavePrefs( pLevel, pF ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_WriteToFile.\n", nullptr);
		return JE_FALSE;
	}

//	if( !BrushList_WriteToFile( pLevel->pBrushes, pF ) )
//	{
//		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_WriteToFile.\n", nullptr);
//		return JE_FALSE;
//	}
	
	return JE_TRUE ;

}// Level_WriteToFile

// Added by cjp
jeBoolean Level_GetShouldSnapVerts( const Level * pLevel )
{
	assert( pLevel != nullptr );
	assert( SIGNATURE == pLevel->nSignature ) ;

	return pLevel->bSnapVertsToGrid;
}

void Level_SetShouldSnapVerts( Level * pLevel, jeBoolean bShouldSnapVerts)
{
	assert( pLevel != nullptr );
	assert( SIGNATURE == pLevel->nSignature ) ;

	pLevel->bSnapVertsToGrid = bShouldSnapVerts;
}
// end added by cjp



//---------------------------------------------------
// Added DJT - 12/20/99
//---------------------------------------------------
typedef struct tagTestForObjectStruct
{
	OBJECT_KIND Kind;
	jeBoolean   bFound;
} TestForObjectStruct;



static jeBoolean Level_TestForObjectCB(Object * pObject, void * lParam)
{
	TestForObjectStruct * pTest;

	assert( pObject != nullptr );
	assert( lParam != nullptr );

	pTest = (TestForObjectStruct*)lParam;
	if (pTest->bFound)
		return JE_FALSE;

	if (Object_GetKind(pObject) == pTest->Kind)
	{
		pTest->bFound = JE_TRUE;
		return JE_FALSE;
	}
	return JE_TRUE;
}


jeBoolean Level_TestForObject(Level * pLevel, OBJECT_KIND Kind)
{
	TestForObjectStruct TestInfo;

	TestInfo.bFound = JE_FALSE;
	TestInfo.Kind = Kind;

	Level_EnumObjects(pLevel, (void *)&TestInfo, Level_TestForObjectCB);

	return TestInfo.bFound;
}
//---------------------------------------------------
// End DJT
//---------------------------------------------------



/* EOF: Level.c */
