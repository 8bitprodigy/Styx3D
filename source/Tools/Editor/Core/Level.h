/****************************************************************************************/
/*  LEVEL.H                                                                             */
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

#pragma once

#ifndef LEVEL_H
#define LEVEL_H

#include "ObjectList.h"
#include "Jet.h"
#include "GroupList.h"
#include "jeWorld.h"
#include "ModelList.h"
#include "MaterialList.h"
#include "LightList.h"
#include "EntityList.h"
#include "CameraList.h"
#include "TernList.h"
#include "Symbol.h"
#include "Undo.h"
#include "jePtrMgr.h"
#include "Class.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagLevel	Level ;

typedef enum
{
	LEVEL_GROUPVIS_ALL,
	LEVEL_GROUPVIS_VISIBLE,
	LEVEL_GROUPVIS_CURRENT,
	LEVEL_GROUPVIS_LAST
} LEVEL_GROUPVIS ;

typedef enum
{
	LEVEL_SELECT,
	LEVEL_DESELECT,
	LEVEL_TOGGLE,
	LEVEL_NOFACESELECT
} LEVEL_STATE ;

typedef enum
{
	LEVEL_UPDATE_MANUEL,
	LEVEL_UPDATE_DESELECT,
	LEVEL_UPDATE_CHANGE,
	LEVEL_UPDATE_REALTIME
} LEVEL_UPDATE;

typedef struct tagSelectObjectInfo
{
	Level	* pLevel;
	LEVEL_STATE eState;
}SelectObjectInfo;



Level *				Level_Create( jeWorld * pWorld, MaterialList_Struct * pGlobalMaterials) ;
void				Level_Destroy( Level ** ppLevel ) ;
jeResourceMgr	*	Level_CreateResourceMgr( jeEngine* pEngine );

// ACCESSORS
Group *					Level_GetCurrentGroup( const Level * pLevel ) ;
jeSymbol_Table *		Level_GetEntities( Level * pLevel ) ;
int32					Level_GetGridSnapSize( const Level * pLevel ) ;
int32					Level_GetRotateSnapSize( const Level * pLevel );

GroupList *				Level_GetGroupList( Level * pLevel ) ;
LEVEL_GROUPVIS			Level_GetGroupVisibility( const Level *pLevel ) ;
ModelList *				Level_GetModelList( Level * pLevel ) ;
LightList *				Level_GetLightList( Level * pLevel ) ;
CameraList *			Level_GetCameraList( Level * pLevel ) ;
GroupList *				Level_GetGroupList( Level * pLevel ) ;
TerrainList	*			Level_GetTerrainList( Level * pLevel ) ;
Model *					Level_GetCurModel( Level * pLevel ) ;
ObjectList *			Level_GetSelList( Level * pLevel );
ObjectList *			Level_GetSubSelList( Level * pLevel );
int32					Level_GetNextObjectId( Level * pLevel, OBJECT_KIND Kind, const char* Name );
const jeExtBox *		Level_GetSelBounds( const Level * pLevel ) ;
const jeExtBox *		Level_GetSubSelDrawBounds( const Level * pLevel );
const jeExtBox *		Level_GetSelDrawBounds( const Level * pLevel ) ;
jeBoolean				Level_GetSelBoundsCenter( const Level * pLevel, jeVec3d * const pCenter ) ;
LEVEL_SEL				Level_GetSelType( const Level * pLevel ) ;
Undo	*				Level_GetUndo( const Level * pLevel );
const jeFaceInfo *		Level_GetCurFaceInfo( const Level * pLevel );
int32					Level_SelXFormModFlags(  const Level * pLevel );
int32					Level_SubSelXFormModFlags(  const Level * pLevel );
float					Level_GetConstructorPlane( const Level * pLevel, int32 Index );
LEVEL_UPDATE			Level_GetBrushUpdate( const Level * pLevel );
LEVEL_UPDATE			Level_GetLightUpdate( const Level * pLevel );
jeBoolean				Level_GetBrushLighting( const Level * pLevel );
jeBoolean				Level_GetCurCamXForm( const Level * pLevel, jeXForm3d * pXForm );
jeBoolean				Level_GetCurCamFOV( const Level * pLevel, float *pFOV );
jeObject *				Level_GetCurCamObject( const Level * pLevel );
const jeExtBox *		Level_GetCurCamBounds( const Level * pLevel );
void					Level_GetCurCamXYRot( const Level * pLevel, float *XRot, float *YRot );
void					Level_GetBSPBuildOptions( const Level * pLevel, jeBSP_Options * Options, jeBSP_Logic * Logic, jeBSP_LogicBalance * LogicBalance );
jeWorld	*				Level_GetjeWorld( const Level * pLevel );
// Krouer: Move from jeBitmap to jeMaterialSpec
#ifdef _USE_BITMAPS
jeBitmap *				Level_GetCurMaterialjeBitmap( const Level * pLevel );
jeBitmap *				Level_GetMaterialBitmapByName( const Level * pLevel, char* szBitmapName );
#else
jeMaterialSpec *		Level_GetMaterialSpecByName( const Level * pLevel, char* szMatName );
jeMaterialSpec *		Level_GetCurMaterialSpec( const Level * pLevel );
#endif

// Added by cjp
jeBoolean				Level_GetShouldSnapVerts( const Level * pLevel );
void					Level_SetShouldSnapVerts( Level * pLevel, jeBoolean bShouldSnapVerts );
// end added by cjp

// HAS-IS
jeBoolean			Level_HasSelections( const Level * pLevel ) ;
jeBoolean			Level_HasSubSelections( const Level * pLevel ) ;
jeBoolean			Level_IsObjectVisible( const Level * pLevel, const Object * pObject ) ;
jeBoolean			Level_IsSelected( Level * pLevel, Object * pObject ) ;
jeBoolean			Level_IsSnapGrid( const Level * pLevel ) ;
jeBoolean			Level_HasChanged( const Level * pLevel );

//---------------------------------------------------
// Added DJT - 12/20/99
//---------------------------------------------------
jeBoolean           Level_TestForObject(Level * pLevel, OBJECT_KIND Kind);
//---------------------------------------------------
// End DJT
//---------------------------------------------------


// ENUMERATION
int32				Level_EnumBrushes( Level * pLevel, void *lParam, BrushListCB Callback ) ;
int32				Level_EnumModels( Level * pLevel, void *lParam, ModelListCB Callback ) ;
int32				Level_EnumObjects( Level * pLevel, void * lParam, ObjectListCB Callback );
int32				Level_EnumSelected( Level * pLevel, void * lParam, ObjectListCB Callback ) ;
int32				Level_EnumSubSelected( Level * pLevel, void * lParam, ObjectListCB Callback ) ;

// STATE CHANGES
void				Level_SetChanged( Level * pLevel, jeBoolean bChanged );
void				Level_SetCurCamXYRot( const Level * pLevel, float XRot, float YRot );
void				Level_ClearMiscFlags( Level * pLevel, const uint32 nFlags ) ;
void				Level_RebuildAll( Level * pLevel, jeBSP_Options Options, jeBSP_Logic Logic, jeBSP_LogicBalance LogicBalance );
void				Level_RebuildLights( Level * pLevel );
void				Level_RebuildBSP( Level * pLevel , jeBSP_Options Options, jeBSP_Logic Logic, jeBSP_LogicBalance LogicBalance );
void				Level_SetMiscFlags( Level * pLevel, const uint32 nFlags ) ;
void				Level_SetModifiedSelection( Level * pLevel ) ;
void				Level_SetSelType( Level * pLevel ) ;
void				Level_SetSnapGrid( Level * pLevel, jeBoolean bState ) ;
void				Level_SetGridSnapSize( Level * pLevel, int32 nSnapSize ) ;
void				Level_SetRotateSnapSize( Level * pLevel, int32 nSnapSize );
void				Level_SelectFirstFace( Level * pLevel );
void				Level_SelectLastFace( Level * pLevel  );
jeBoolean			Level_SetFaceInfoToCurMaterial( Level * pLevel );
Group		*		Level_AddGroup( Level * pLevel, const char * pszName );
Model		*		Level_AddModel( Level * pLevel, const char * pszName );
Class		*		Level_AddClass( Level * pLevel, const char * pszName, int Kind );
void				Level_ModelLock( Level * pLevel, Model * pModel, jeBoolean bLock );
void				Level_SetCurrentGroup( Level * pLevel, Group * pGroup );
void				Level_SetCurrentModel( Level * pLevel, Model * pModel );
void				Level_SetConstructor( Level * pLevel, int Index, float Value );
void				Level_SetBrushUpdate( Level * pLevel, int Update );
void				Level_SetLightUpdate( Level * pLevel, int Update );
void				Level_SetBrushLighting( Level * pLevel, int BrushLighting );
void				Level_UpdateAll( Level * pLevel );
void				Level_UpdateSelected( Level * pLevel );
jeBoolean			Level_RotCurCamX( const Level * pLevel, float Radians );
jeBoolean			Level_RotCurCamY( const Level * pLevel, float Radians );
jeBoolean			Level_TranslateCurCam( const Level * pLevel, jeVec3d * Offset );
jeBoolean			Level_SetRenderMode( Level * pLevel, int Mode );
void				Level_SetBSPBuildOptions( Level * pLevel, jeBSP_Options  Options, jeBSP_Logic  Logic, jeBSP_LogicBalance  LogicBalance );
void				Level_RenameSelected( Level * pLevel, char * Name );

// OBJECT MANIPULATION

Object		*		Level_NewObject( Level * pLevel, int Kind, int SubKind,  const jeExtBox * pBrushBounds ) ;
Object		*		Level_SubtractBrush( Level * pLevel, int SubKind, const jeExtBox * pBrushBounds  ) ;
Object		*		Level_NewUserObject( Level * pLevel, const char * TypeName, const jeExtBox * pBrushBounds );
jeBoolean			Level_SelectObject( Level * pLevel, Object * pObject , LEVEL_STATE eState ) ;
jeBoolean			Level_SubSelectObject( Level * pLevel, Object * pObject , LEVEL_STATE eState ) ;
jeBoolean			Level_DeselectAllSub( Level * pLevel, jeExtBox * pWorldBounds );
jeBoolean			Level_UnMarkAllSub( Level * pLevel, jeExtBox * pWorldBounds );
jeBoolean			Level_MarkSubSelect( Level * pLevel,  jeObject * pgeObject , int32 flag );
jeBoolean			Level_SubSelectgeObject( Level * pLevel, jeObject * pgeObject , LEVEL_STATE eState ) ;
jeBoolean			Level_SelectGroup( Level * pLevel, Group * pGroup, LEVEL_STATE eState );
jeBoolean			Level_AddObject( Level * pLevel, Object* pObject );
jeBoolean			Level_PrepareForSave( Level* pLevel );
void				Level_DeleteObject( Level * pLevel, Object* pObject );
jeBoolean			Level_DragBegin( Level * pLevel, Object* pObject );
jeBoolean			Level_AddToWorld( Level * pLevel, Object* pObject, int Update );
Object		*		Level_FindgeObject( Level * pLevel,  jeObject * pgeObject );

// FILE HANDLING
jeBoolean			Level_WriteToFile( Level * pLevel, jeVFile * pF, jePtrMgr * pPtrMgr ) ;
Level *				Level_CreateFromFile( jeVFile * pF, jeWorld * pWorld, MaterialList_Struct * pGlobalMaterials, jePtrMgr * pPtrMgr, float Version ) ;


#ifdef __cplusplus
}
#endif

#endif //Prevent multiple inclusion
/* EOF: Level.h */