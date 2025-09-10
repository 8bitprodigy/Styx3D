/****************************************************************************************/
/*  BRUSH.H                                                                             */
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

#ifndef BRUSH_H
#define BRUSH_H

#include "Defs.h"
#include "BaseType.h"
#include "ExtBox.h"
#include "jeBrush.h"
#include "VertList.h"
//#include "Model.h"
#include "Group.h"
#include "jePtrMgr.h"
#include "jeModel.h"
#include "Descriptor.h"
#include "BrushTemplate.h"
#include "Undo.h"
#include "jeWorld.h"

typedef struct tagModel Model ;

#define BRUSH_VERSION		(1)
#define BRUSH_REATTACH_GOOD	((uint32)(-1))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagBrush Brush ;

#define FACE_INIT_ALL 0xFFFFFFFF
#define BRUSH_INIT_ALL 0xFFFFFFFF

typedef struct Brush_WriteInfo {
	jeVFile * pF;
	jePtrMgr * pPtrMgr;
	jeWorld *pWorld;
} Brush_WriteInfo;

typedef struct tagBrushReattachInfo
{
	uint32		nIndexTag ;
	Model	*	pModel ;
	jeBrush	*	pgeBrush ;
	jeWorld *	pWorld;
} BrushReattachInfo ;

typedef struct tagFaceVertInfo
{
	jeBrush_Face *	pFace ;
	int32			nVert ;
} FaceVertInfo ;
typedef void (*BrushFaceVertCB)( FaceVertInfo * pVec, void * pVoid ) ;

typedef enum
{
	BRUSH_EFLAG_SELECTED	= (1<<0),
	BRUSH_EFLAG_LAST		= (1<<1)
} BRUSH_EFLAG ;
// Define your own edit flags beginning at EFLAG_LAST in your own module

typedef struct FaceInfoCB_Struct {
	jeFaceInfo *pFaceInfo;
	int32		FieldFlag;
	Brush	*	pBrush;
	jeModel *	pgeModel;
	Undo	*	pUndo;
} FaceInfoCB_Struct;

enum {
	BRUSH_FLOCKING_FIELD = DESCRIPTOR_LOCAL_DATATYPE_START,
	BRUSH_SOLID_FIELD,
	BRUSH_EMPTY_FIELD,
	BRUSH_AIR_FIELD,
	BRUSH_CONTENT_FIELD_END,
	BRUSH_FACEINFO_FIELD,
	BRUSH_GOURAND_FIELD,
	BRUSH_FLAT_FIELD,
	BRUSH_TRANSPARENT_FIELD,
	BRUSH_FULLBRIGHT_FIELD,
	BRUSH_VIS_PORTAL_FIELD,
	BRUSH_RENDER_PORTAL_ONLY,
	BRUSH_VIS_PORTAL_COMBO,
	BRUSH_ALPHA_FIELD,
	BRUSH_ROTATE_FIELD,
	BRUSH_SHIFT_GROUP,
	BRUSH_SHIFTU_FIELD,
	BRUSH_SHIFTV_FIELD,
	BRUSH_SHIFT_GROUP_END,
	BRUSH_DRAWSCALE_GROUP,
	BRUSH_DRAWSCALEU_FIELD,
	BRUSH_DRAWSCALEV_FIELD,
	BRUSH_DRAWSCALE_GROUP_END,
	BRUSH_LIGHTMAP_GROUP,
	BRUSH_LMAPSCALEU_FIELD,
	BRUSH_LMAPSCALEV_FIELD,
	BRUSH_LIGHTMAP_GROUP_END,
	BRUSH_FACEINFO_FIELD_END
};

typedef enum 
{
	FACE_FIELD_GOURAUD		= (1<<0),
	FACE_FIELD_VIS_PORTAL	= (1<<1),
	FACE_FIELD_INVISIBLE	= (1<<2),
	FACE_FIELD_ROTATE		= (1<<3),
	FACE_FIELD_SHIFTU		= (1<<4),
	FACE_FIELD_SHIFTV		= (1<<5),
	FACE_FIELD_DRAWSCALEU	= (1<<6),
	FACE_FIELD_DRAWSCALEV	= (1<<7),
	FACE_FIELD_LMAPSCALEU	= (1<<8),
	FACE_FIELD_LMAPSCALEV	= (1<<9),
	FACE_FIELD_ALPHA		= (1<<10),
	FACE_FIELD_FULLBRIGHT   = (1<<11),
	FACE_FIELD_FLAT		    = (1<<12),
	FACE_FIELD_PORTALCAMERA	= (1<<13),
	FACE_FIELD_ONLYPORTAL	= (1<<14),


} FACEINFO_FIELDS;

typedef struct BrushInfoCB_Struct {
	uint32	*	Contents;
	int32		FieldFlag;
} BrushInfoCB_Struct;

typedef enum
{
	BRUSH_FIELD_FLOCK,
	BRUSH_FIELD_DRAW,
	BRUSH_FIELD_SHEET,
	BRUSH_FIELD_TEXT_LOCK		= (1<<15)
} BRUSH_FIELDS;


typedef enum
{
	BRUSH_ADD,
	BRUSH_SUBTRACT,
	BRUSH_SUBTRACTNOMEMBERS
} BRUSH_TYPE ;

//GLOBAL PROPERTIES
jeProperty_List *	Brush_GlobalPropertyList();
void				Brush_SetGlobalProperty( int DataId, int DataType, jeProperty_Data * pData );

Brush *				Brush_Create( const char * const pszName, Group * pGroup, int32 nNumber ) ;
Brush *				Brush_CreateFromBox( const char * const pszName, int32 nNumber, jeXForm3d * pXForm, const jeExtBox* pBox, jeFaceInfo_ArrayIndex nFaceIndex ) ;
void				Brush_Destroy( Brush ** ppBrush ) ;
Brush * Brush_FromTemplate( BrushTemplate *pTemplate,  Group * pGroup, char * Name, int32 nNumber, jeFaceInfo * pFaceInfo, BRUSH_TYPE eAddType );
Brush *				Brush_Copy( const Brush * pBrush, const int32 nNumber ) ;
char  *				Brush_CreateDefaultName( BRUSH_TYPE Type );
char *				Brush_CreateKindName( );
jeBrush *			Brush_CopygeBrush( const Brush * pBrush );


// ACCESSORS
int32				Brush_GetFaceCount( const Brush * pBrush ) ;
jeBrush *			Brush_GetjeBrush( const Brush * pBrush ) ;
BRUSH_KIND			Brush_GetKind( const Brush * pBrush );
int32				Brush_GetLargestFaceVertexCount( const Brush * pBrush ) ;
jeBrush_Face *		Brush_GetNextFace( const Brush * pBrush, jeBrush_Face * pStart ) ;
jeBrush_Face *		Brush_GetFaceByIndex( const Brush * pBrush, int32 Index ) ;
jeBrush_Face *		Brush_GetNextSelFace( Brush * pBrush );
jeBrush_Face *		Brush_GetPrevSelFace( Brush * pBrush );
//					When no face is selected ruturns first face. 
//					When more then one face is selected on a brush the first in selection.
//					When one face is slected it returns next face.
//					Returns curently selected face is last in list.
Model *				Brush_GetModel( Brush * pBrush ) ;
VertList		*	Brush_GetSelVert( const Brush * pBrush );
const jeExtBox *	Brush_GetWorldAxialBounds( const Brush * pBrush ) ;
void				Brush_GetWorldCenter( const Brush * pBrush, jeVec3d * pCenter ) ;
void				Brush_GetXForm( const Brush * pBrush, jeXForm3d *XForm );
void				Brush_GetVertexPoint( Brush * pBrush, jeVertArray_Index Index, jeVec3d * pVert ) ;
void				Brush_GetFaceInfo( Brush * pBrush, FaceInfoCB_Struct* FaceInfoData);
void				Brush_GetBrushInfo( Brush * pBrush, BrushInfoCB_Struct* BrushInfoData);
void				Brush_Update( Brush * pBrush, int Update_Type );
jeBoolean			Brush_SelectClosest( Brush * pBrush, FindInfo *	pFindInfo );


// IS
jeBoolean			Brush_IsInModel( const Brush * pBrush ) ;
jeBoolean			Brush_IsPointOverVertex( const Brush * pBrush, const jeVec3d * pWorldPt, ORTHO_AXIS OAxis, const jeFloat fThreshold, uint32 * pnVertex ) ;
jeBoolean			Brush_IsPointOverNearVertex( const Brush * pBrush, const jeVec3d * pWorldPt, ORTHO_AXIS OAxis, const jeFloat fThreshold, uint32 * pnVertex ) ;
jeBoolean			Brush_HasSelectedVert( const Brush * pBrush );
jeBoolean			Brush_HasSelectedFace( const Brush * pBrush );
jeBoolean			Brush_IsFaceSelected( const Brush * pBrush, jeBrush_Face * pFace );
jeBoolean			Brush_IsInRect( const Brush * pBrush, jeExtBox *pSelRect, jeBoolean bSelEncompeses ) ;
jeBoolean			Brush_IsCutBrush( const Brush * pBrush );

// MODIFIERS
void				Brush_Move( Brush * pBrush, const jeVec3d * pWorldDistance ) ;
void				Brush_Rotate( Brush * pBrush, ORTHO_AXIS RAxis, jeFloat RadianAngle, const jeVec3d * pRotationCenter ) ;
void				Brush_SetModel( Brush * pBrush, Model * pModel ) ;
void				Brush_SetModified( Brush * pBrush ) ;
void				Brush_Size( Brush * pBrush, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis ) ;
void				Brush_SetXForm( Brush * pBrush, const jeXForm3d * XForm );
void				Brush_UpdateBounds( const Brush * pBrush ) ;
void				Brush_SetGeBrush( Brush* pBrush, BRUSH_KIND Kind, jeBrush * pgeBrush );
void				Brush_Shear( Brush* pBrush, const jeVec3d * pWorldDistance, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, const jeExtBox * pSelectedBounds);
jeProperty_List *	Brush_BuildDescriptor( Brush * pBrush );
void				Brush_SetProperty( Brush * pBrush, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bBrushUpdate, jeBoolean bLightUpdate );
void				Brush_ResetSelFace( Brush * pBrush );
jeBoolean			Brush_AttachWorld( Brush * pBrush, jeWorld * jeWorld );
jeBoolean			Brush_DettachWorld( Brush * pBrush, jeWorld * jeWorld );

//FACE MODIFIERS
void				Brush_SelectFace( Brush * pBrush, jeBrush_Face * pFace );
void				Brush_DeselectFace( Brush * pBrush, jeBrush_Face * pFace, jeBoolean *FaceListEmpty );
void				Brush_DeselectAllFaces( Brush * pBrush  );
void				Brush_SelectAllFaces( Brush * pBrush );
void				Brush_SelectFirstFace( Brush * pBrush );
void				Brush_SelectLastFace( Brush * pBrush );
void				Brush_ApplyMatrToFaces( Brush * pBrush, const jeFaceInfo *pFaceInfo, Undo *pUndo );
void				Brush_SetFaceInfo( Brush * pBrush, FaceInfoCB_Struct* FaceInfoData );

//VERT MODIFIERS
jeBoolean			Brush_SelectVert( Brush * pBrush, jeVertArray_Index  Index );
void				Brush_DeselectVert( Brush * pBrush, jeVertArray_Index  Vert);
void				Brush_ToggleVert( Brush * pBrush, jeVertArray_Index );
void				Brush_DeselectAllVert( Brush * pBrush );
void				Brush_MoveSelectedVert( Brush * pBrush, jeVec3d *dWorldDist );
void				Brush_RestoreSelVert( Brush * pBrush );
jeBoolean			Brush_SelectVertInRect( Brush * pBrush, jeExtBox *pSelBox );
// ENUMERATION
void				Brush_EnumFaceVerts( Brush * pBrush, void * pParam, BrushFaceVertCB Callback ) ;

// CALLBACK (public)
jeBoolean			Brush_ReattachCB( Brush *pBrush, void* lParam ) ;

// FILE HANDLING
Brush *				Brush_CreateFromFile( jeVFile * pF, const int32 nVersion, jePtrMgr * pPtrMgr ) ;
jeBoolean			Brush_WriteToFile( Brush * pBrush, Brush_WriteInfo * pWriteInfo ) ;


// FACE MODULE?
int32				Brush_FaceGetVertCount( jeBrush_Face * pFace ) ;
const jeVec3d *		Brush_FaceGetVertByIndex( jeBrush_Face * pFace, int32 Index ) ;
jeVec3d				Brush_FaceGetWorldSpaceVertByIndex(const Brush * pBrush, const jeBrush_Face * pFace, int32 Index ) ;

// DEBUGGING
#ifdef _DEBUG
jeBoolean	Brush_IsValid( const Brush * pBrush ) ;
jeBoolean	Brush_IsBoundsValid( Brush * pBrush ) ;
#endif

// PRESENTATION
void Brush_RenderOrthoFaces( Brush *pBrush, const Ortho * pOrtho,  int32 hDC, jeBoolean bDrawVertex, jeBoolean bDrawSelFaces, jeBoolean bColorOveride );


//UNDO
jeBoolean			Brush_RestoreMaterialCB( Object *pObject, void *Context );
void				Brush_DestroyMaterialContextCB( void *Context );

// Krouer: add a visibility manager 
jeBoolean		    Brush_IsVisible( const Brush* pBrush );
void                Brush_Show( Brush* pBrush, jeBoolean Visible );


#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Brush.h */
