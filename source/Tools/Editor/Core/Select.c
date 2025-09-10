/****************************************************************************************/
/*  SELECT.C                                                                            */
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
 On: 12/27/99 8:50:01 PM
 Comments: 1) Select_KindsSelected() - Return selection mask.
           2) Select_All() - Select all.
----------------------------------------------------------------------------------------*/


#include <Assert.h>
#include <Float.h>
#include <string.h>

#include "Defs.h"
#include "ObjectList.h"
#include "Rect.h"
#include "Units.h"
#include "Util.h"
#include "Light.h"
#include "../Resource.h"
#include "jeProperty.h"
#include "ram.h"
#include "userobj.h"

#include "Select.h"
#define BIG_SCALE	10000.0f
#define MAX(aa,bb)   ( ((aa)>(bb))?(aa):(bb) )
#define MIN(aa,bb)   ( ((aa)<(bb))?(aa):(bb) )


typedef	struct tagFindFaceInfo
{
	const Level *	pLevel;
	jeVec3d			Front; //front of test vector. Basicly Camera
	jeVec3d			Back ; //back of test vector. the mouse click pojected far into world space.	
	Brush		*	pBrush;
	jeBrush_Face *  pFace;
	jeFloat			fMinDistance ;
	uint32			c1;
	uint32			c2;
} FindFaceInfo;

typedef struct tagAddSelectInfo
{
	Level		*	pLevel ;
} AddSelectInfo	;

typedef struct tagDeleteSelectedInfo
{
	Level		*	pLevel ;
	jeExtBox	*	pWorldBounds ;
} DeleteSelectedInfo ;

typedef struct tagDeselectBrushInfo
{
	Level		*	pLevel ;
	jeExtBox	*	pWorldBounds ;
	int32			nCount ;
	OBJECT_KIND		eKind ;
} DeselectBrushInfo ;

typedef struct tagDupDeselInfo
{
	Level		*	pLevel ;
	ObjectList	*	pDups ;
} DupDeselInfo ;


typedef struct tagVertexPointInfo
{
	jeVec3d		WorldPoint ;
	ORTHO_AXIS	OAxis ;
	int32		nVertex ;
	jeFloat		fTolerance ;
} VertexPointInfo ;



static jeBoolean Select_AddAndSelect( Object * pObject, void * lParam )
{
	jeBoolean		bSuccess = JE_TRUE ;
	AddSelectInfo *	pasi = (AddSelectInfo*)lParam ;

	bSuccess = Level_AddObject( pasi->pLevel, pObject ) ;	// Bumps ref on success
	if( bSuccess )
	{
		Object_AddRef( pObject ) ;	// Bump the ref-count to match the list cleanup
		bSuccess = Level_SelectObject( pasi->pLevel, pObject, LEVEL_SELECT ) ;
	}
	return bSuccess ;
}// Select_AddAndSelect


// Added JH 25.03.2000
static jeBoolean Select_Add( Object * pObject, void * lParam )
{
	jeBoolean		bSuccess = JE_TRUE ;
	AddSelectInfo *	pasi = (AddSelectInfo*)lParam ;

	bSuccess = Level_AddObject( pasi->pLevel, pObject ) ;	// Bumps ref on success
	if( bSuccess )
	{
		Object_AddRef( pObject ) ;	// Bump the ref-count to match the list cleanup
	//	bSuccess = Level_SelectObject( pasi->pLevel, pObject, LEVEL_SELECT ) ;
	}
	return bSuccess ;
}// Select_Add

static jeBoolean Select_DeselectObjectsCB( Object * pObject, void * lParam )
{
	DeselectBrushInfo * pdbi ;
	jeExtBox			ObjectBounds;
	pdbi = (DeselectBrushInfo*)lParam ;
	if( Object_GetWorldDrawBounds( pObject, &ObjectBounds ) )
		Util_ExtBox_Union( pdbi->pWorldBounds,  &ObjectBounds, pdbi->pWorldBounds ) ;
	Level_SelectObject(  pdbi->pLevel, pObject, LEVEL_DESELECT ) ;
	pdbi->nCount++ ;

	return JE_TRUE ;
}// Select_DeselectObjectsCB

static jeBoolean Select_DeselectObjectsExcluding( Object * pObject, void * lParam )
{
	DeselectBrushInfo * pdbi = (DeselectBrushInfo*)lParam ;
	jeExtBox			ObjectBounds ;
	if( Object_GetKind( pObject ) != pdbi->eKind )
	{
		pdbi = (DeselectBrushInfo*)lParam ;
		if( Object_GetWorldDrawBounds( pObject, &ObjectBounds ) )
			Util_ExtBox_Union( pdbi->pWorldBounds, &ObjectBounds, pdbi->pWorldBounds ) ;

		Level_SelectObject(  pdbi->pLevel, pObject, LEVEL_DESELECT ) ;
		pdbi->nCount++ ;
	}
	return JE_TRUE ;

}// Select_DeselectObjectsExcluding

static jeBoolean Select_CreateSelectedUndoCB( Object * pObject, void *lParam  )
{
	Level		*	pLevel ;
	Undo		*	pUndo ;

	assert( pObject!= NULL );
	assert( lParam != NULL );

	pLevel = (Level*)lParam;
	pUndo = Level_GetUndo( pLevel ) ;

	return Undo_AddSubTransaction( pUndo, UNDO_CREATEOBJECT, pObject, pLevel );
	
}// Select_CreateSelectedUndoCB



static jeBoolean Select_ClosestObject( Object * pObject, void * lParam )
{
	FindInfo		*	pFindInfo ;

	assert( pObject != NULL );
	assert( lParam != NULL );

	pFindInfo = (FindInfo*)lParam ;
	return( Object_SelectClosest(  pObject, pFindInfo ) );

}// Select_ClosestObject

jeBoolean Select_PointOverVertexCB( Object * pObject, void * lParam )
{
	uint32	nVertex ;
	VertexPointInfo	* pvpi = (VertexPointInfo*)lParam ;
	if( Object_GetKind( pObject ) != KIND_BRUSH ) 
		return( JE_FALSE );
	if( Brush_IsPointOverVertex( (Brush*)pObject, &pvpi->WorldPoint, pvpi->OAxis, pvpi->fTolerance, &nVertex ) )
	{
		pvpi->nVertex = nVertex ;
		return JE_FALSE ;
	}
	return JE_TRUE ;
}// Select_PointOnVertexCB

jeBoolean	Select_ClosestFaceCB( Model * pModel, void * lParam )
{
	FindFaceInfo		*	pFindInfo ;
	jeBrushRayInfo		 	Info;
	jeFloat					Dist;


	assert( pModel != NULL );
	assert( lParam != NULL );

	pFindInfo = (FindFaceInfo*)lParam ;

	if( jeModel_RayIntersectsBrushes( 
		Model_GetguModel( pModel ), 
		&pFindInfo->Front, &pFindInfo->Back, 
		&Info)
	)
	{
		if( Info.Brush ==  NULL )
			return( JE_TRUE );
		Dist = jeVec3d_DistanceBetween( &pFindInfo->Back, &Info.Impact );
		if( Dist < pFindInfo->fMinDistance )
		{
			pFindInfo->fMinDistance =  pFindInfo->fMinDistance;
			pFindInfo->pBrush = Model_FindBrush( pModel, Info.Brush );

			if( Info.BrushFace != NULL )
				pFindInfo->pFace = Info.BrushFace;
			#pragma message( "Jet3D will give face * here" )
		}
	}

	return( JE_TRUE );
	Dist;Info;
}// Select_ClosestFaceCB

jeBoolean Select_CreateModel( Level * pLevel, const char * pszName )
{
	assert( pLevel != NULL ) ;
	assert( pszName != NULL ) ;

	return JE_TRUE ;	 // I'm not even sure why I want to return anything...
}// Select_CreateModel

jeBoolean	Select_Face(Level * pLevel, const jeCamera * pCamera,  const Point * pViewPt, uint32 *c1, uint32 *c2 ) 
{
	jeVec3d	Vector;
	jeVec3d	Front;
	jeVec3d	Back;
	jeXForm3d pXForm;
	Object * pObject;
	Group * pGroup;
	jeCollisionInfo Info;
	jeBrushRayInfo		 	RayInfo;
	
	assert( pLevel );
	assert( pCamera );
	assert( pViewPt );

	jeCamera_ScreenPointToWorld(	pCamera, pViewPt->X, pViewPt->Y, &Vector );

	jeVec3d_Scale( &Vector, BIG_SCALE, &Vector );
	jeCamera_GetXForm( pCamera,&pXForm);
	Front = pXForm.Translation;
	jeVec3d_Add( &Front, &Vector, &Back );
	if( jeWorld_Collision( Level_GetjeWorld(pLevel ), NULL,  &Front, &Back, &Info  ) )
	{
		pObject  = Level_FindgeObject( pLevel,  Info.Object );

		if( (Object_GetKind( pObject ) == KIND_MODEL) && !Model_IsLocked( (Model*)pObject ) )
		{
			if( jeModel_RayIntersectsBrushes( 
				Model_GetguModel( (Model*)pObject ), 
				&Front, &Back, 
				&RayInfo)
			)
			{
				pObject = (Object*)Model_FindBrush( (Model*)pObject, RayInfo.Brush );
				if( RayInfo.BrushFace != NULL )
				{
					Brush_SelectFace( (Brush*)pObject, RayInfo.BrushFace );
					*c1 = RayInfo.c1;
					*c2 = RayInfo.c2;
				}
			}
		}
		if( Object_GetKind( pObject) == KIND_USEROBJ )
		{
			UserObj_Select3d( (UserObj*)pObject, &Front, &Back, &(Info.Impact) );
		}

		pGroup = Object_GetGroup( pObject );
		if( Group_IsLocked( pGroup ) )
			Level_SelectGroup( pLevel, pGroup, LEVEL_NOFACESELECT );
		else
			Level_SelectObject( pLevel, pObject, LEVEL_NOFACESELECT );
		return( JE_TRUE );
	}
	return( JE_FALSE );
} //Select_Face

static jeBoolean Select_AllFaceCB( Object *pObject, void * pVoid ) 
{
	if( Object_GetKind( pObject ) != KIND_BRUSH ) 
		return( JE_TRUE );

	Brush_SelectAllFaces( (Brush*)pObject );
	return( JE_TRUE );
	pVoid;
}

jeBoolean Select_AllFaces( Level * pLevel )
{
	Level_EnumSelected( pLevel, NULL, Select_AllFaceCB );
	return( JE_TRUE );
}
typedef struct SelectFaceInfo {
	Brush* pBrush;
	jeBrush_Face *pFace;
} SelectFaceInfo;

static jeBoolean Select_NextFaceCB( Object *pObject, void * pVoid ) 
{
	SelectFaceInfo	*	pInfo = (SelectFaceInfo *)pVoid;
	Brush			*	pBrush = (Brush*)pObject;
	assert( pObject );
	assert( pVoid );
	assert( Object_GetKind( pObject ) == KIND_BRUSH ) ;

	if( !Brush_HasSelectedFace( pBrush ) )
		return(JE_TRUE );
	pInfo->pBrush = pBrush;
	pInfo->pFace = Brush_GetNextSelFace( pBrush );
	if( pInfo->pFace == NULL )
		return( JE_TRUE );
	return( JE_FALSE );
} //Select_NextFaceCB

void Select_NextFace( Level * pLevel )
{
	SelectFaceInfo Info;
	if( !Level_EnumSelected( pLevel, &Info, Select_NextFaceCB ) )
	{
		Select_DeselectAllFaces( pLevel );
		Brush_SelectFace( Info.pBrush, Info.pFace );
	}
	else
	{
		Select_DeselectAllFaces( pLevel );
		Level_SelectFirstFace( pLevel );
	}
}

static jeBoolean Select_PrevFaceCB( Object *pObject, void * pVoid ) 
{
	SelectFaceInfo	*	pInfo = (SelectFaceInfo *)pVoid;
	Brush			*	pBrush = (Brush*)pObject;
	assert( pObject );
	assert( pVoid );
	if( Object_GetKind( pObject ) != KIND_BRUSH ) 
		return( JE_TRUE );

	if( !Brush_HasSelectedFace( pBrush ) )
		return(JE_TRUE );
	pInfo->pBrush = pBrush;
	pInfo->pFace = Brush_GetPrevSelFace( pBrush );
	if( pInfo->pFace == NULL )
		return( JE_TRUE );
	return( JE_FALSE );
} //Select_PrevFaceCB

void Select_PrevFace( Level * pLevel )
{
	SelectFaceInfo Info;
	if( !Level_EnumSelected( pLevel, &Info, Select_PrevFaceCB ) )
	{
		Select_DeselectAllFaces( pLevel );
		Brush_SelectFace( Info.pBrush, Info.pFace );
	}
	else
	{
		Select_DeselectAllFaces( pLevel );
		Level_SelectLastFace( pLevel );
	}
}// Select_PrevFace

typedef struct ApplyMatrInfo_Struct {
	const jeFaceInfo *pFaceInfo;
	Undo *		pUndo;
#ifdef _USE_BITMAPS
	jeBitmap *  pCurBitmap;
#else
	jeMaterialSpec* pCurMaterialSpec;
#endif
} ApplyMatrInfo_Struct;

static jeBoolean Select_ApplyMatrCB( Object *pObject, void * pVoid ) 
{
	ApplyMatrInfo_Struct *pApplyMatrInfo = (ApplyMatrInfo_Struct *)pVoid;
	assert( pObject );
	assert( pVoid );

	switch( Object_GetKind( pObject ) )
	{
		case KIND_BRUSH:
		if( Brush_HasSelectedFace( (Brush*)pObject ) )
			Brush_ApplyMatrToFaces( (Brush*)pObject, pApplyMatrInfo->pFaceInfo, pApplyMatrInfo->pUndo );
		break;

		case KIND_USEROBJ:
#ifdef _USE_BITMAPS
			UserObj_ApplyMatr( (UserObj*)pObject, pApplyMatrInfo->pCurBitmap );
#else
			UserObj_ApplyMatr( (UserObj*)pObject, pApplyMatrInfo->pCurMaterialSpec );
#endif
		break;
		
		default:
		break;
	}
	return( JE_TRUE );
} //Select_PrevFaceCB

void Select_ApplyCurMaterial( Level * pLevel )
{
	ApplyMatrInfo_Struct  ApplyMatrInfo;

	assert( pLevel );

	Level_SetFaceInfoToCurMaterial( pLevel );
	ApplyMatrInfo.pFaceInfo = Level_GetCurFaceInfo( pLevel );
	ApplyMatrInfo.pUndo = Level_GetUndo( pLevel );
#ifdef _USE_BITMAPS
	ApplyMatrInfo.pCurBitmap = Level_GetCurMaterialjeBitmap( pLevel );
#else
	ApplyMatrInfo.pCurMaterialSpec = Level_GetCurMaterialSpec( pLevel );
#endif

	Undo_Push( ApplyMatrInfo.pUndo, UNDO_APPLYTEXTURE );
	Level_EnumSelected( pLevel, (void*)&ApplyMatrInfo, Select_ApplyMatrCB );
}//Select_ApplyCurMaterial




static jeBoolean Select_LightInfoCB( Object *pObject, void * pVoid ) 
{
	LightInfoCB_Struct *pLightInfoCBData = (LightInfoCB_Struct *)pVoid;
	Light			*	pLight = (Light*)pObject;

	assert( pObject );
	assert( pVoid );
	if( Object_GetKind( pObject ) != KIND_LIGHT ) 
		return( JE_TRUE );

	Light_GetInfo( pLight, pLightInfoCBData->LightInfo, &pLightInfoCBData->FieldFlag );
	return( JE_TRUE );
} //Select_LightInfoCB


void Select_GetLightInfo( Level * pLevel, LightInfo *LightInfo, int32 *pBlankFieldFlag  )
{
	LightInfoCB_Struct LightInfoCBData; 

	assert( pLevel );
	assert( LightInfo );
	assert( pBlankFieldFlag );
	
	LightInfoCBData.FieldFlag = LIGHT_INIT_ALL;
	LightInfoCBData.LightInfo = LightInfo;

	Level_EnumSelected( pLevel, &LightInfoCBData, Select_LightInfoCB );
	*pBlankFieldFlag = LightInfoCBData.FieldFlag;
}//Select_GetLightInfo


static jeBoolean Select_SetFaceInfoCB( Object *pObject, void * pVoid ) 
{
	FaceInfoCB_Struct *pFaceInfoCBData = (FaceInfoCB_Struct *)pVoid;
	Brush			*	pBrush = (Brush*)pObject;

	assert( pObject );
	assert( pVoid );
	assert( Object_GetKind( pObject ) == KIND_BRUSH ) ;

	if( !Brush_HasSelectedFace( pBrush ) )
		return(JE_TRUE );
	Brush_SetFaceInfo( pBrush, pFaceInfoCBData );
	return( JE_TRUE );
} //Select_FaceInfoCB

void Select_SetFaceInfo( Level * pLevel, jeFaceInfo *pFaceInfo, int32 BlankFieldFlag )
{
	FaceInfoCB_Struct FaceInfoCBData; 

	assert( pLevel );
	
	FaceInfoCBData.FieldFlag = BlankFieldFlag;
	FaceInfoCBData.pFaceInfo = pFaceInfo;

	Level_EnumSelected( pLevel, &FaceInfoCBData, Select_SetFaceInfoCB );

}// Select_SetFaceInfo

static jeBoolean Select_SetLightInfoCB( Object *pObject, void * pVoid ) 
{
	LightInfoCB_Struct *pLightInfoCBData = (LightInfoCB_Struct *)pVoid;
	Light			*	pLight = (Light*)pObject;

	assert( pObject );
	assert( pVoid );
	if( Object_GetKind( pObject ) != KIND_LIGHT ) 
		return( JE_TRUE );

	Light_SetInfo( pLight, pLightInfoCBData->LightInfo, pLightInfoCBData->FieldFlag );
	return( JE_TRUE );
} //Select_FaceInfoCB

void Select_SetLightInfo( Level * pLevel, LightInfo *pLightInfo, int32 BlankFieldFlag  )
{

	LightInfoCB_Struct LightInfoCBData; 

	assert( pLevel );
	assert( pLightInfo );
	assert( BlankFieldFlag );
	
	LightInfoCBData.FieldFlag = BlankFieldFlag;
	LightInfoCBData.LightInfo = pLightInfo;

	

	Level_EnumSelected( pLevel, &LightInfoCBData, Select_SetLightInfoCB );

}// Select_SetLightInfo


static jeBoolean Select_DupSelection( Object * pObject, void * lParam )
{
	DupDeselInfo	* pddi ;
	Object			* pNewObject ;

	pddi = (DupDeselInfo*)lParam ;

	pNewObject = Object_Copy
	( 
		pObject, 
		Level_GetNextObjectId( pddi->pLevel, Object_GetKind( pObject ), Object_GetName( pObject ) ) 
	) ;
	if( pNewObject == NULL )
		return JE_FALSE ;

	if( ObjectList_Append( pddi->pDups, pNewObject ) == NULL )
	{
		Object_Free( &pNewObject ) ;
		return JE_FALSE ;
	}
	return JE_TRUE ;
}// Select_DupDesel



static jeBoolean Select_HandleIn( int32 Left, int32 Top, Point * pViewPt )
{
	Rect	r ;

	r.Left = Left ;
	r.Top = Top ;
	r.Right = Left + HANDLESIZE ;
	r.Bottom = Top + HANDLESIZE ;

	return Rect_IsPointIn( &r, pViewPt ) ;

}// Select_HandleIn

static jeBoolean Select_DragBeginCB( Object * pObject, void * lParam )
{
	Level * pLevel;

	assert( pObject != NULL );
	assert( lParam != NULL );
	
	pLevel = (Level*)lParam;

	return( Level_DragBegin( pLevel, pObject )) ;

}// Select_DragBeginCB

static jeBoolean Select_DragEndCB( Object * pObject, void * lParam )
{
	Level * pLevel;

	assert( pObject != NULL );
	assert( lParam != NULL );
	
	pLevel = (Level*)lParam;

	Object_Update( pObject, OBJECT_UPDATE_CHANGE, JE_FALSE );

	return( JE_TRUE ) ;

}// Select_DragBeginCB

static jeBoolean Select_DeleteCB( Object * pObject, void * lParam )
{
	jeExtBox				Bounds ;
	DeleteSelectedInfo	*	pdsi = (DeleteSelectedInfo*)lParam ;
	Level				*	pLevel;
	Undo				*	pUndo;

	assert( pObject != NULL );
	assert( lParam != NULL );
	
	pLevel = pdsi->pLevel ;


	pUndo = Level_GetUndo( pLevel );
	Undo_AddSubTransaction( pUndo, UNDO_DELETEOBJECT, pObject, pLevel );
	
	if( Object_GetWorldDrawBounds( pObject, &Bounds ) )
		Util_ExtBox_Union( pdsi->pWorldBounds, &Bounds, pdsi->pWorldBounds ) ;

	Level_SelectObject(  pLevel, pObject, LEVEL_DESELECT ) ;
	Level_DeleteObject( pLevel, pObject );
	return( JE_TRUE ) ;

}// Select_DragBeginCB

//
// END STATIC
//

jeBoolean Select_IsCorner( SELECT_HANDLE SelectHandle )
{
	assert( SelectHandle > Select_None &&  SelectHandle < Select_Last ) ;

	return SelectHandle >= Select_TopLeft && SelectHandle <= Select_BottomRight ; 
}// Select_IsCorner

jeBoolean Select_IsEdge( SELECT_HANDLE SelectHandle )
{
	assert( SelectHandle > Select_None &&  SelectHandle < Select_Last ) ;

	return SelectHandle >= Select_Left && SelectHandle <= Select_Bottom ; 
}// Select_IsCorner

jeBoolean Select_IsPointOverVertex( const Ortho * pOrtho, const Point * pViewPt, Level * pLevel )
{
	VertexPointInfo	vpi ;
	jeBoolean		b ;

	Ortho_ViewToWorld( pOrtho, pViewPt->X, pViewPt->Y, &vpi.WorldPoint ) ;
	vpi.OAxis = Ortho_GetOrthogonalAxis( pOrtho ) ;
	vpi.nVertex = -1 ;
	vpi.fTolerance = Ortho_GetWorldHandleSelectThreshold( pOrtho ) / 2.0f ;
	b = Level_EnumSelected( pLevel, &vpi, Select_PointOverVertexCB ) ;
		
	return !b ;
}// Select_IsPointOnVertex

jeBoolean Select_ClosestThing( Level * pLevel, const Ortho * pOrtho, const Point * pViewPt, LEVEL_STATE eState, jeExtBox * pWorldBounds, MODE eMode, jeBoolean bControl_Held )
{
	FindInfo			findInfo ;
	SELECT_RESULT		SelResult = SELECT_RESULT_NONE ;
	jeVec3d				WorldPoint ;
	jeBoolean			bSelDone = JE_FALSE ;
	jeVertArray_Index	nVertex ;
	jeExtBox			DeselectBox;

	assert( pLevel != NULL ) ;
	assert( pViewPt != NULL ) ;
	assert( pWorldBounds != NULL ) ;

	Util_ExtBox_SetInvalid( &DeselectBox );
	findInfo.fMinDistance = FLT_MAX ;
	findInfo.nFace = -1 ;
	findInfo.pObject = NULL ;
	findInfo.pViewPt = pViewPt ;
	findInfo.pOrtho = pOrtho ;
	findInfo.eMode = eMode ;
	if( MODE_POINTER_VM == eMode )
		findInfo.eSelKind = KIND_BRUSH ;
	else
		findInfo.eSelKind = OBJECT_KINDALL ;

	*pWorldBounds = *Level_GetSelDrawBounds( pLevel );
	Level_EnumObjects( pLevel, &findInfo, Select_ClosestObject ) ;
	if( findInfo.pObject != NULL && findInfo.fMinDistance < Ortho_GetWorldSelectThreshold( pOrtho ) )
	{

		if( MODE_POINTER_VM == eMode )
		{
			if( Level_IsSelected( pLevel, findInfo.pObject ) )
			{
				uint32 uVertex ;
				if( !bControl_Held )
				{
					SelResult = SELECT_RESULT_CHANGED;

					// CJP : The problem with using Select_DeselctAll is that it deselects the brush as well as the vertices.
					//		 This means that we would have to reselect the brush to manipulate the vertices (a big pain).
					// Select_DeselectAll( pLevel, &DeselectBox );
				
					// So what we do now is deselect the other vertices in the same brush.
					Select_DeselectAllVerts(pLevel);

				}
				Ortho_ViewToWorld( pOrtho, pViewPt->X, pViewPt->Y, &WorldPoint ) ;
				if( Brush_IsPointOverNearVertex( (Brush*)findInfo.pObject, &WorldPoint, Ortho_GetOrthogonalAxis( pOrtho ), Ortho_GetWorldSelectThreshold( pOrtho ), &uVertex ) )
				{
					nVertex = (jeVertArray_Index)uVertex ;
					switch( eState )
					{
					case LEVEL_TOGGLE :	Brush_ToggleVert( (Brush*)findInfo.pObject, nVertex ) ;	break ;
					} 
					bSelDone = JE_TRUE ;
					SelResult = SELECT_RESULT_CHANGED;
				}
			}
			Util_ExtBox_Union( pWorldBounds, Level_GetSubSelDrawBounds( pLevel ), pWorldBounds );
		}
		
		if( bSelDone == JE_FALSE )
		{	// BB or RS mode
			Group * pGroup;
			uint32	flags;


			flags = Object_GetMiscFlags( findInfo.pObject );
			pGroup = Object_IsMemberOfLockedGroup( findInfo.pObject );

			if( flags & AllSubSelect )
			{
				if( !bControl_Held )
				{
					SelResult = SELECT_RESULT_SUBSELECT;
					Level_DeselectAllSub( pLevel, &DeselectBox );
				}
				Level_SubSelectObject( pLevel, findInfo.pObject, eState );
				SelResult = SELECT_RESULT_SUBSELECT;						
				Util_ExtBox_Union( pWorldBounds, Level_GetSubSelDrawBounds( pLevel ), pWorldBounds );
			}
			else
			if( pGroup != NULL )
			{
				if( !bControl_Held )
				{
					Select_DeselectAll( pLevel, &DeselectBox );
					SelResult = SELECT_RESULT_CHANGED;
				}
				Level_SelectGroup( pLevel, pGroup, eState );
				SelResult = SELECT_RESULT_CHANGED;
				Util_ExtBox_Union( pWorldBounds, Level_GetSelDrawBounds( pLevel ), pWorldBounds );
			}
			else
			{
				if( !bControl_Held )
				{
					Select_DeselectAll( pLevel, &DeselectBox );
					SelResult = SELECT_RESULT_CHANGED;
				}
				Level_SelectObject( pLevel, findInfo.pObject, eState );
				SelResult = SELECT_RESULT_CHANGED;
				Util_ExtBox_Union( pWorldBounds, Level_GetSelDrawBounds( pLevel ), pWorldBounds );
			}
		}

	}
	else
	if( !bControl_Held && Level_HasSelections( pLevel ) )
	{
		// CJP : This provides a click - away clear, type of thing.  With out with we would assert in the 3D view when they select a face.
		if(eMode == MODE_POINTER_VM)
			Select_DeselectAllVerts(pLevel);

		Select_DeselectAll( pLevel, &DeselectBox );
		SelResult = SELECT_RESULT_CHANGED;
	}
	return SelResult ;

}// Select_ClosestThing


jeBoolean Select_CreateSelectedUndo( Level * pLevel, UNDO_TYPES Type )
{
	Undo *			pUndo;
	assert( pLevel != NULL ) ;

	pUndo = Level_GetUndo( pLevel );
	assert( pUndo );
	Undo_Push( pUndo, Type );
	return Level_EnumSelected( pLevel, pLevel, Select_CreateSelectedUndoCB ) ;

}// Select_CreateSelectedUndo

jeBoolean Select_Delete( Level * pLevel, jeExtBox * pWorldBounds )
{
	DeleteSelectedInfo	dsi ;
	Undo  * pUndo;

	assert( pLevel != NULL ) ;
	assert( pWorldBounds != NULL ) ;


	pUndo = Level_GetUndo( pLevel );
	Undo_Push( pUndo, UNDO_DELETE );

	Util_ExtBox_SetInvalid( pWorldBounds ) ;
	dsi.pLevel = pLevel ;
	dsi.pWorldBounds = pWorldBounds ;
	Level_EnumSelected( pLevel, &dsi, Select_DeleteCB );
	return JE_TRUE ;
}// Select_Delete

jeBoolean Select_DeselectAll( Level * pLevel, jeExtBox * pWorldBounds )
{
	DeselectBrushInfo dbi ;

	assert( pLevel != NULL ) ;
	assert( pWorldBounds != NULL ) ;

	Util_ExtBox_SetInvalid( pWorldBounds ) ;
	dbi.pWorldBounds = pWorldBounds ;
	dbi.pLevel = pLevel ;
	dbi.nCount = 0 ;
	Level_EnumSelected( pLevel, &dbi, Select_DeselectObjectsCB ) ;
	Level_DeselectAllSub( pLevel, pWorldBounds );
	Level_UnMarkAllSub( pLevel, pWorldBounds );
	if( dbi.nCount != 0 )
	{
		Level_SetModifiedSelection( pLevel ) ;
		return JE_TRUE ;
	}
	return JE_FALSE ;
}// Select_DeselectAll

jeBoolean Select_DeselectAllExcluding( Level * pLevel, jeExtBox * pWorldBounds, OBJECT_KIND eKind )
{
	DeselectBrushInfo	dbi ;
	assert( pLevel != NULL ) ;
	assert( pWorldBounds != NULL ) ;

	Util_ExtBox_SetInvalid( pWorldBounds ) ;
	dbi.pWorldBounds = pWorldBounds ;
	dbi.pLevel = pLevel ;
	dbi.nCount = 0 ;
	dbi.eKind = eKind ;
	Level_EnumSelected( pLevel, &dbi, Select_DeselectObjectsExcluding ) ;
	if( dbi.nCount != 0 )
	{
		Level_SetModifiedSelection( pLevel ) ;
		return JE_TRUE ;
	}
	return JE_FALSE ;
}// Select_DeselectAllExcluding

jeBoolean Select_DupAndDeselectSelections( Level * pLevel )
{
	jeBoolean					bSuccess ;
	DupDeselInfo				ddi ;
	AddSelectInfo				asi ;
	jeExtBox					WorldBounds ;
	assert( pLevel != NULL ) ;

	ddi.pDups = ObjectList_Create( ) ;
	if( ddi.pDups == NULL )
		return JE_FALSE ;

	ddi.pLevel = pLevel ;
	bSuccess = Level_EnumSelected( pLevel, &ddi, Select_DupSelection ) ;
	if( JE_TRUE == bSuccess )
	{
		Select_DeselectAll( pLevel, &WorldBounds ) ;
		asi.pLevel = pLevel ;
		bSuccess = ObjectList_EnumObjects( ddi.pDups, &asi, Select_AddAndSelect ) ;
		if( JE_FALSE == bSuccess )	// Failed selecting some of the list
		{
			Select_DeselectAll( pLevel, &WorldBounds ) ;
		}
	}
	
	ObjectList_Destroy( &ddi.pDups, ObjectList_DestroyCB ) ; // Reduce refcount or delete, don't use Object_Free

	return bSuccess ;
}// Select_DupAndDeselectedSelections

 // Added JH 25.03.2000
jeBoolean		Select_Dup ( Level * pLevel ) 
{
	jeBoolean					bSuccess ;
	DupDeselInfo				ddi ;
	AddSelectInfo				asi ;
	jeExtBox					WorldBounds ;
	assert( pLevel != NULL ) ;

	ddi.pDups = ObjectList_Create( ) ;
	if( ddi.pDups == NULL )
		return JE_FALSE ;

	ddi.pLevel = pLevel ;
	bSuccess = Level_EnumSelected( pLevel, &ddi, Select_DupSelection ) ;
	if( JE_TRUE == bSuccess )
	{
	//	Select_DeselectAll( pLevel, &WorldBounds ) ;
		asi.pLevel = pLevel ;
		bSuccess = ObjectList_EnumObjects( ddi.pDups, &asi, Select_AddAndSelect ) ;
		if( JE_FALSE == bSuccess )	// Failed selecting some of the list
		{
			Select_DeselectAll( pLevel, &WorldBounds ) ;
		}
	}
	
	ObjectList_Destroy( &ddi.pDups, ObjectList_DestroyCB ) ; // Reduce refcount or delete, don't use Object_Free

	return bSuccess ;

}
// EOF JH


typedef struct SelectRectStruct {
	jeExtBox *pSelBox;
	jeBoolean bSelEncompeses;
	int32 Mask;
	Level * pLevel;
	jeBoolean bSelChanged;
} SelectRectStruct;

static jeBoolean Select_RectangleCB( Object *pObject, void * pVoid ) 
{
	SelectRectStruct *pSelectRectInfo = (SelectRectStruct*)pVoid;
	Group * pGroup;
	assert( pObject );
	assert( pVoid );

	if( !(Object_GetKind( pObject ) & pSelectRectInfo->Mask) ) 
		return(JE_TRUE );


	pGroup = Object_IsMemberOfLockedGroup( pObject );
	if( pGroup != NULL )
	{
		jeExtBox GroupBox;
		jeExtBox UnionBox;

		ObjectList_GetListBounds( Group_GetObjectList(pGroup), &GroupBox );
		jeExtBox_Union( &GroupBox, pSelectRectInfo->pSelBox, &UnionBox );
		//If the Union is equal to the selectRect the GroupBox is enclosed
		if( jeVec3d_Compare( &pSelectRectInfo->pSelBox->Min, &UnionBox.Min, 0.0f ) &&
			jeVec3d_Compare( &pSelectRectInfo->pSelBox->Max, &UnionBox.Max, 0.0f ) )
		{
			Level_SelectGroup( pSelectRectInfo->pLevel, pGroup, LEVEL_SELECT );
			pSelectRectInfo->bSelChanged = JE_TRUE;
		}
	}
	else
	{
		if( Object_IsInRect( pObject, pSelectRectInfo->pSelBox, pSelectRectInfo->bSelEncompeses ) )
		{
			Level_SelectObject( pSelectRectInfo->pLevel, pObject , LEVEL_SELECT ) ;
			pSelectRectInfo->bSelChanged = JE_TRUE;
		}
	}

	return( JE_TRUE );
}

jeBoolean Select_Rectangle( Level * pLevel, jeExtBox *pSelBox, jeBoolean bSelEncompeses, int32 Mask, jeExtBox *Bounds )
{
	SelectRectStruct SelectRectInfo;

	assert( pLevel );
	assert( pSelBox );
	assert( Bounds );

	SelectRectInfo.bSelEncompeses = bSelEncompeses;
	SelectRectInfo.Mask = Mask;
	SelectRectInfo.pSelBox = pSelBox;
	SelectRectInfo.pLevel = pLevel;
	SelectRectInfo.bSelChanged = JE_FALSE;
	Level_EnumObjects( pLevel, &SelectRectInfo, Select_RectangleCB ) ;
	*Bounds = *Level_GetSelDrawBounds(  pLevel );
	return( SelectRectInfo.bSelChanged );
}//Select_Rectangle

static jeBoolean Select_VertsInRectangleCB( Object *pObject, void * pVoid ) 
{
	SelectRectStruct *pSelectRectInfo = (SelectRectStruct*)pVoid;
	assert( pObject );
	assert( pVoid );

	if( Object_GetKind( pObject ) != KIND_BRUSH  )
		return( JE_TRUE );

	if( Brush_SelectVertInRect( (Brush*)pObject, pSelectRectInfo->pSelBox) )
	{
		pSelectRectInfo->bSelChanged = JE_TRUE;
	}
	return( JE_TRUE );
}

jeBoolean Select_VertsInRectangle( Level * pLevel, jeExtBox *pSelBox, jeBoolean bSelEncompeses, jeExtBox *Bounds ) 
{
	SelectRectStruct SelectRectInfo;
	assert( pLevel );
	assert( pSelBox );
	assert( Bounds );

	SelectRectInfo.bSelEncompeses = bSelEncompeses;
	SelectRectInfo.pSelBox = pSelBox;
	SelectRectInfo.pLevel = pLevel;
	SelectRectInfo.bSelChanged = JE_FALSE;
	Level_EnumSelected( pLevel, &SelectRectInfo, Select_VertsInRectangleCB ) ;
	*Bounds = *Level_GetSelDrawBounds(  pLevel );
	return( SelectRectInfo.bSelChanged );
}//Select_VertsInRectangle

jeBoolean Select_DragBegin( Level * pLevel )
{
	assert( pLevel != NULL ) ;

	Level_SetChanged( pLevel, JE_TRUE );
	return Level_EnumSelected( pLevel, pLevel, Select_DragBeginCB ) ;

}// Select_DragBegin

jeBoolean Select_DragEnd( Level * pLevel )
{
	assert( pLevel != NULL ) ;


	return Level_EnumSelected( pLevel, pLevel, Select_DragEndCB ) ;

}// Select_DragEnd

jeBoolean Select_DragBeginSub( Level * pLevel )
{
	assert( pLevel != NULL ) ;

	Level_SetChanged( pLevel, JE_TRUE );
	return Level_EnumSubSelected( pLevel, pLevel, Select_DragBeginCB ) ;

}// Select_DragBegin

jeBoolean Select_DragEndSub( Level * pLevel )
{
	assert( pLevel != NULL ) ;


	return Level_EnumSubSelected( pLevel, pLevel, Select_DragEndCB ) ;

}// Select_DragEnd

SELECT_HANDLE Select_ViewPointHandle( Ortho * pOrtho, Point * pViewPt, jeExtBox * pWorldBox )
{
	Rect	viewRect ;
	Rect	handleRect ;
	int32	nMiddle ;

	assert( pOrtho != NULL ) ;
	assert( pViewPt != NULL ) ;
	assert( pWorldBox != NULL ) ;

	Ortho_WorldToViewRect( pOrtho, pWorldBox, &viewRect ) ;

	handleRect = viewRect ;
	Rect_Inflate( &handleRect, HALFHANDLESIZE, HALFHANDLESIZE ) ;
	if( Rect_IsPointIn( &handleRect, pViewPt ) == JE_FALSE )
		return Select_None ;

	// Corners
	if( Select_HandleIn( handleRect.Left, handleRect.Top, pViewPt ) )
		return Select_TopLeft ;
	if( Select_HandleIn( handleRect.Right - HANDLESIZE, handleRect.Top, pViewPt ) )
		return Select_TopRight ;
	if( Select_HandleIn( handleRect.Left, handleRect.Bottom - HANDLESIZE, pViewPt ) )
		return Select_BottomLeft ;
	if( Select_HandleIn( handleRect.Right - HANDLESIZE, handleRect.Bottom - HANDLESIZE, pViewPt ) )
		return Select_BottomRight ;

	// Edges
	nMiddle = (handleRect.Top + handleRect.Bottom)/2 ;
	if( Select_HandleIn( handleRect.Left, nMiddle-HALFHANDLESIZE, pViewPt ) )
		return Select_Left ;
	if( Select_HandleIn( handleRect.Right-HANDLESIZE, nMiddle-HALFHANDLESIZE, pViewPt ) )
		return Select_Right ;
	nMiddle = (handleRect.Left + handleRect.Right)/2 ;
	if( Select_HandleIn( nMiddle-HALFHANDLESIZE, handleRect.Top, pViewPt ) )
		return Select_Top ;
	if( Select_HandleIn( nMiddle-HALFHANDLESIZE, handleRect.Bottom-HANDLESIZE, pViewPt ) )
		return Select_Bottom ;

	return Select_None ;

}// Select_ViewPointHandle

SELECT_HANDLE Select_NearestCornerHandle( Ortho * pOrtho, Point * pViewPt, jeExtBox * pWorldBox )
{
	Rect	viewRect ;
	Point	Center ;
	double	rad ;
	assert( pOrtho != NULL ) ;
	assert( pViewPt != NULL ) ;
	assert( pWorldBox != NULL ) ;

	Ortho_WorldToViewRect( pOrtho, pWorldBox, &viewRect ) ;
	Rect_GetTranslation( &viewRect, &Center ) ;
	
	rad = atan2( Center.Y - pViewPt->Y, pViewPt->X - Center.X ) ;
	if( rad < -JE_HALFPI )
		return Select_BottomLeft ;
	else if( rad < 0.0f )
		return Select_BottomRight ;
	else if( rad < JE_HALFPI )
		return Select_TopRight ;
	
	return Select_TopLeft ;
}// Select_NearestHandle

jeBoolean Select_MoveVertCB( Object *pObject, void * pVoid )
{
	assert( pObject );
	assert( pVoid );
	assert( Object_GetKind( pObject ) == KIND_BRUSH );

	Brush_MoveSelectedVert( (Brush*)pObject, pVoid);
/*
	if( !jeBrush_IsConvex( Brush_GetgeBrush( (Brush*)pObject ) ) )
	{
		Brush_RestoreSelVert( (Brush*)pObject );
		return( JE_FALSE );
	}
*/
	return( JE_TRUE );
}

jeBoolean Select_MoveSelectedVert( Level * pLevel, jeVec3d * dWorldDist, jeExtBox * WorldBounds )
{
	*WorldBounds  =	*Level_GetSelBounds( pLevel );

	Level_EnumSelected( pLevel, dWorldDist, Select_MoveVertCB );
	Level_SetModifiedSelection( pLevel );
	jeExtBox_Union ( WorldBounds, Level_GetSelDrawBounds( pLevel ), WorldBounds );
	return( JE_TRUE );
}

// End added / modified by cjp

jeBoolean Select_HasNoSelVertCB( Object *pObject, void * pVoid )
{
	assert( pObject );

	if( Object_GetKind( pObject )!= KIND_BRUSH )
		return( JE_TRUE );
	return( !Brush_HasSelectedVert( (Brush *) pObject ) );
	pVoid;
}

jeBoolean Select_HasSelectedVerts( Level * pLevel )
{
	return( !Level_EnumSelected( pLevel, NULL, Select_HasNoSelVertCB ) );
}// Select_HasSelectedVerts

jeBoolean Select_DeselectAllVertCB( Object *pObject, void * pVoid )
{
	assert( pObject );

	if( Object_GetKind( pObject )!= KIND_BRUSH )
		return( JE_TRUE );
	Brush_DeselectAllVert( (Brush *) pObject  );
	return( JE_TRUE );
	pVoid;
}

jeBoolean		Select_DeselectAllVerts( Level *pLevel )
{
	return( Level_EnumSelected( pLevel, NULL, Select_DeselectAllVertCB ) );
}

jeBoolean Select_DeselectAllFacesCB( Object *pObject, void * pVoid )
{
	assert( pObject );

	if( Object_GetKind( pObject )!= KIND_BRUSH )
		return( JE_TRUE );
	Brush_DeselectAllFaces( (Brush *) pObject  );
	return( JE_TRUE );
	pVoid;
}

jeBoolean		Select_DeselectAllFaces( Level *pLevel )
{
	return( Level_EnumSelected( pLevel, NULL, Select_DeselectAllFacesCB ) );
}

typedef struct GetName_Struct {
	const char	* Name;
	int32		* nNumber;
} GetName_Struct;

jeBoolean Select_GetNameCB( Object *pObject, void * pVoid )
{
	GetName_Struct * pGetName_Data = (GetName_Struct *)pVoid;
	const char * Name;

	assert( pObject );
	assert( pVoid );

	Name = Object_GetName( pObject );

	if( pGetName_Data->Name == NULL )
	{
		pGetName_Data->Name = Name;
		*pGetName_Data->nNumber = Object_GetNameTag( pObject  );
	}
	else
	{
		*pGetName_Data->nNumber = SELECT_INVALID_NNUMBER;
		if( strcmp( pGetName_Data->Name, Name ) )
			return( JE_FALSE );
	}
	return( JE_TRUE );
}

//	Goes through the selection 
//  If the selection has differet types returns NULL
//	If the selection has same types but different names return NULL
//  If the selection has same type with same name return the name but nNumber set to SELECT_INVALID_NNUMBER
//  If the selctiion has only one thing it returns the name and the nNumber
const char  *	Select_GetName( Level * pLevel, int32 *nNumber )
{
	int32 SelType;
	GetName_Struct GetName_Data;

	SelType = Level_GetSelType( pLevel );

	if( SelType & LEVEL_SELNONE || SelType & LEVEL_SELMANY )
		return( NULL );
	*nNumber = SELECT_INVALID_NNUMBER;
	GetName_Data.Name = NULL;
	GetName_Data.nNumber = nNumber;
	if( !Level_EnumSelected( pLevel, &GetName_Data, Select_GetNameCB ) )
		return( NULL );
	return( GetName_Data.Name );
}

typedef struct SetName_Struct {
	const char	* Name;
	Level		* pLevel;
} SetName_Struct;

jeBoolean Select_SetNameCB( Object *pObject, void * pVoid )
{
	SetName_Struct * pSetName_Data = (SetName_Struct *)pVoid;
	int32 nNumber;

	assert( pObject );
	assert( pVoid );

	nNumber = Level_GetNextObjectId( pSetName_Data->pLevel, Object_GetKind( pObject ), pSetName_Data->Name );
	Object_SetName( pObject, pSetName_Data->Name, nNumber );
	return( JE_TRUE );
}

void Select_SetName( Level * pLevel, const char * Name )
{
	SetName_Struct SetName_Data;

	SetName_Data.Name = Name;
	SetName_Data.pLevel = pLevel;
	Level_EnumSelected( pLevel, &SetName_Data, Select_SetNameCB );
}


static jeBoolean Select_FillPositionDescriptor( Level * pLevel, jeProperty *pDescriptor )
{
	jeVec3d  Center;
	char * Name;

	if( !Level_GetSelBoundsCenter( pLevel, &Center )  )
		return( JE_FALSE );
	Name = Util_LoadLocalRcString( IDS_POSITION_FIELD );
	if( Name == NULL )
		return(JE_FALSE );
	jeProperty_FillVec3dGroup( &pDescriptor[0], Name, &Center,	OBJECT_POSITION_FIELD  );
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONX_FIELD );
	if( Name == NULL )
		return(JE_FALSE );
	jeProperty_FillFloat( &pDescriptor[1], Name, Center.X,	OBJECT_POSITION_FIELDX, -FLT_MAX, FLT_MAX, 1.0f );
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONY_FIELD );
	if( Name == NULL )
		return(JE_FALSE );
	jeProperty_FillFloat( &pDescriptor[2], Name, Center.Y,	OBJECT_POSITION_FIELDY, -FLT_MAX, FLT_MAX, 1.0f );
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONZ_FIELD );
	if( Name == NULL )
		return(JE_FALSE );
	jeProperty_FillFloat( &pDescriptor[3], Name, Center.Z,	OBJECT_POSITION_FIELDZ, -FLT_MAX, FLT_MAX, 1.0f );
	jeRam_Free( Name );

	jeProperty_FillGroupEnd( &pDescriptor[4], OBJECT_POSITION_FIELD_END );
	return( JE_TRUE );
}

static jeProperty_List * Select_AppendDescriptorArray(  Level * pLevel, jeProperty_List * pArray )
{
	jeProperty_List * pNewArray;
	jeProperty * foundDescriptor;
	jeProperty * pDescriptor;
	jeProperty * pNewDescriptor;
	int	CurDescriptor = 0;
	int i;


	pNewArray = jeProperty_ListCreate( pArray->jePropertyN +6  );
	if( pNewArray == NULL )
		return( NULL );

	foundDescriptor = jeProperty_ListFindByDataId( pArray, OBJECT_NAME_FIELD );
	pDescriptor = pArray->pjeProperty;
	pNewDescriptor = pNewArray->pjeProperty;
	if( foundDescriptor != NULL )
	{
		pNewDescriptor[CurDescriptor] = *foundDescriptor;
		if( foundDescriptor->FieldName  != NULL )
			pNewDescriptor[CurDescriptor].FieldName = Util_StrDup( foundDescriptor->FieldName );
		CurDescriptor++;
	}
	if( Select_FillPositionDescriptor( pLevel, &pNewDescriptor[CurDescriptor] ) )
		CurDescriptor += 5;
#pragma message ("Later add selection size and maybe rotation here" )
	for( i = 0 ; i < pArray->jePropertyN ; i++ )
	{
		if( ( pDescriptor[i].DataId == OBJECT_NAME_FIELD ) ||
			( pDescriptor[i].DataId == OBJECT_POSITION_FIELD ) ||
			( pDescriptor[i].DataId == OBJECT_POSITION_FIELDX ) ||
			( pDescriptor[i].DataId == OBJECT_POSITION_FIELDY ) ||
			( pDescriptor[i].DataId == OBJECT_POSITION_FIELDZ ) ||
			( pDescriptor[i].DataId == OBJECT_POSITION_FIELD_END )
			)
			continue;
		pNewDescriptor[CurDescriptor] = pDescriptor[i];
		if( pDescriptor[i].FieldName  != NULL )
			pNewDescriptor[CurDescriptor].FieldName = Util_StrDup( pDescriptor[i].FieldName );
		CurDescriptor++;
		assert( CurDescriptor <= pNewArray->jePropertyN );
	}
	pNewArray->jePropertyN = CurDescriptor;
	return( pNewArray );
}

jeProperty_List * Select_BuildDescriptor( Level * pLevel )
{
	LEVEL_SEL SelType;
	jeProperty_List * pArray = NULL;
	jeProperty_List * pArray2 = NULL;
	jeProperty_List * pTempArray = NULL;
	ObjectList		* pSelList;
	Object			* pObject;
	ObjectIterator    Iterator;
	int				  bSameType = JE_FALSE;
	jeProperty * foundDescriptor;

	SelType = Level_GetSelType( pLevel ) ;
	pSelList = Level_GetSelList( pLevel );
	if( (SelType & LEVEL_SELONEBRUSH ) ||
		(SelType & LEVEL_SELONELIGHT ) ||
		(SelType & LEVEL_SELONECAMERA ) ||
		(SelType & LEVEL_SELONEMODEL ) ||
		(SelType & LEVEL_SELONEOBJECT )||
		(SelType & LEVEL_SELONECLASS ) )
	{
		pObject = ObjectList_GetFirst( pSelList, &Iterator );
		pArray = Object_BuildDescriptor( pObject );
		if( pArray == NULL )
			return( NULL );
	}
	else
	if( (SelType & LEVEL_SELBRUSHES ) ||
		(SelType & LEVEL_SELLIGHTS ) ||
		(SelType & LEVEL_SELCAMERAS ) ||
		(SelType & LEVEL_SELOBJECTS ) ||
		(SelType & LEVEL_SELMODELS ) ||
		(SelType & LEVEL_SELCLASS ) ||
		(SelType & LEVEL_SELMANY )	
		)
	{
		if( !(SelType & LEVEL_SELMANY )   )
			bSameType = JE_TRUE;
		pObject = ObjectList_GetFirst( pSelList, &Iterator );
		pArray = Object_BuildDescriptor( pObject );
		if( pArray == NULL )
			return( NULL );

		pObject = ObjectList_GetNext( pSelList, &Iterator );
		while( pObject )
		{
			pArray2 = Object_BuildDescriptor( pObject );
			if( pArray2 == NULL )
				goto SBD_ERROR;
			pTempArray  = jeProperty_ListMerge( pArray, pArray2, bSameType );
			if( pTempArray == NULL )
				goto SBD_ERROR;
			jeProperty_ListDestroy( &pArray );
			jeProperty_ListDestroy( &pArray2 );
			pArray = pTempArray;
			pObject = ObjectList_GetNext( pSelList, &Iterator );
		}
		pArray = Select_AppendDescriptorArray( pLevel, pArray );
		if( pArray == NULL )
			goto SBD_ERROR;
	}
	else
		return( NULL );
	if( Level_IsSnapGrid( pLevel ) )
	{
		foundDescriptor = jeProperty_ListFindByDataId( pArray, OBJECT_POSITION_FIELDX );
		if( foundDescriptor != NULL )
		{
			foundDescriptor->TypeInfo.NumInfo.Increment = (float)Level_GetGridSnapSize( pLevel );
		}
		foundDescriptor = jeProperty_ListFindByDataId( pArray, OBJECT_POSITION_FIELDY );
		if( foundDescriptor != NULL )
		{
			foundDescriptor->TypeInfo.NumInfo.Increment = (float)Level_GetGridSnapSize( pLevel );
		}
		foundDescriptor = jeProperty_ListFindByDataId( pArray, OBJECT_POSITION_FIELDZ );
		if( foundDescriptor != NULL )
		{
			foundDescriptor->TypeInfo.NumInfo.Increment = (float)Level_GetGridSnapSize( pLevel );
		}
	}

	return( pArray );
SBD_ERROR:
	if( pArray )
		jeProperty_ListDestroy( &pArray );
	if( pArray2 )
		jeProperty_ListDestroy( &pArray2 );
	return NULL;

}

//---------------------------------------------------
// Added DJT - 12/20/99 9:48:54 PM
//---------------------------------------------------

typedef struct tagSelectAllStruct
{
	int32       Mask;
	Level *     pLevel;
	jeBoolean   bSelChanged;
} SelectAllStruct;


static jeBoolean Select_AllCB( Object *pObject, void * pVoid ) 
{
	SelectAllStruct *pSelectAllInfo = (SelectAllStruct*)pVoid;
	Group * pGroup;

	assert( pObject );
	assert( pVoid );

	if( !(Object_GetKind( pObject ) & pSelectAllInfo->Mask) ) 
		return(JE_TRUE );

	pGroup = Object_IsMemberOfLockedGroup(pObject);
	if( pGroup != NULL )
	{
		Level_SelectGroup( pSelectAllInfo->pLevel, pGroup, LEVEL_SELECT );
		pSelectAllInfo->bSelChanged = JE_TRUE;
	}
	else
	{
		Level_SelectObject( pSelectAllInfo->pLevel, pObject , LEVEL_SELECT ) ;
		pSelectAllInfo->bSelChanged = JE_TRUE;
	}

	return( JE_TRUE );
}

jeBoolean Select_All( Level * pLevel, int32 Mask, jeExtBox *Bounds )
{
	SelectAllStruct SelectAllInfo;

	assert( pLevel );
	assert( Bounds );

	SelectAllInfo.Mask           = Mask;
	SelectAllInfo.pLevel         = pLevel;
	SelectAllInfo.bSelChanged    = JE_FALSE;

	Level_EnumObjects( pLevel, &SelectAllInfo, Select_AllCB ) ;
	*Bounds = *Level_GetSelDrawBounds(  pLevel );
	return( SelectAllInfo.bSelChanged );
}


jeBoolean Select_KindsSelectedCB(Object *pObject, void * pVoid)
{
	assert(pObject);

	{
		int iMask;
		int iKind;
	
		iMask = (int)pVoid;
		iKind = (int)Object_GetKind(pObject);

		iMask = (iMask | iKind);
	}
	return (JE_TRUE);
}


// Return mask containing kinds of selected items
int32 Select_KindsSelected(Level * pLevel)
{
	int32 iMask = 0;

	Level_EnumSelected(pLevel, (void *)iMask, Select_KindsSelectedCB);
	return iMask;
}

//---------------------------------------------------
// End DJT
//---------------------------------------------------


/* EOF: Select.c */