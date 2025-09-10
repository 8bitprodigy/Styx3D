/****************************************************************************************/
/*  TRANSFORM.C                                                                         */
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

#include "Util.h"
#include "Ram.h"
#include "Errorlog.h"

#include "Transform.h"

#define GEXFORM3D_SCALE_TOLERANCE (0.00001f)

typedef struct tagMoveBrushInfo
{
	Level			*	pLevel ;
	jeExtBox		*	pWorldBounds ;
	const jeVec3d	*	pWorldDistance ;
} MoveBrushInfo ;

typedef struct tagRotateBrushInfo
{
	Level			*	pLevel ;
	jeExtBox		*	pWorldBounds ;
	ORTHO_AXIS			RAxis ;
	jeVec3d				RotationCenter ;
	jeFloat				fRadianAngle ;
} RotateBrushInfo ;

typedef struct tagShearBrushInfo
{
	Level			*	pLevel ;
	jeExtBox		*	pWorldBounds ;
	SELECT_HANDLE		eSizeType;
	ORTHO_AXIS			HAxis;
	ORTHO_AXIS			VAxis;
	const jeVec3d	*	pWorldDistance ;
	const jeExtBox	*	pSelectedBounds ;
} ShearBrushInfo ;

typedef struct tagSizeBrushInfo
{
	Level			*	pLevel ;
	jeExtBox		*	pWorldBounds ;
	const jeVec3d	*	pWorldDistance ;
	ORTHO_AXIS			HAxis ;
	ORTHO_AXIS			VAxis ;
	SELECT_HANDLE		eSizeType ;
	jeFloat				fHScale ;
	jeFloat				fVScale ;
	const jeExtBox	*	pSelectedBounds ;
} SizeBrushInfo ;


typedef struct tagSnapBrushInfo
{
	Level			*	pLevel ;
	jeExtBox		*	pWorldBounds ;
	jeFloat				fSnapSize ;
} SnapBrushInfo ;


static jeBoolean Transform_MoveObject( Object * pObject, void * lParam )
{
	MoveBrushInfo * pmbi ;
	jeExtBox		WorldBounds ;
	
	pmbi = (MoveBrushInfo*)lParam ;
	if( !Object_GetWorldDrawBounds( pObject, &WorldBounds ) )
		return( JE_FALSE );
	Util_ExtBox_Union( pmbi->pWorldBounds, &WorldBounds, pmbi->pWorldBounds ) ;
	Object_Move( pObject, pmbi->pWorldDistance ) ;
	if( !Object_GetWorldDrawBounds( pObject, &WorldBounds ) )
		return( JE_FALSE );
	Util_ExtBox_Union( pmbi->pWorldBounds, &WorldBounds, pmbi->pWorldBounds ) ;

	return JE_TRUE ;
}// Select_DeselectBrush

static jeBoolean Transform_ShearObject( Object * pObject, void * lParam )
{
	ShearBrushInfo * psbi ;
	jeExtBox		WorldBounds ;
	
	psbi = (ShearBrushInfo*)lParam ;
	if( !Object_GetWorldDrawBounds( pObject, &WorldBounds ) )
		return( JE_FALSE );
	Util_ExtBox_Union( psbi->pWorldBounds, &WorldBounds, psbi->pWorldBounds ) ;
	Object_Shear( pObject, psbi->pWorldDistance, psbi->eSizeType, psbi->HAxis, 
		psbi->VAxis, psbi->pSelectedBounds ) ;
	if( !Object_GetWorldDrawBounds( pObject, &WorldBounds ) )
		return( JE_FALSE );
	Util_ExtBox_Union( psbi->pWorldBounds, &WorldBounds, psbi->pWorldBounds ) ;

	return JE_TRUE ;
}// Select_DeselectBrush

static jeBoolean Transform_RotateObject( Object * pObject, void * lParam )
{
	RotateBrushInfo * prbi ;
	jeExtBox		WorldBounds ;
	
	prbi = (RotateBrushInfo*)lParam ;
	if( !Object_GetWorldDrawBounds( pObject, &WorldBounds ) )
		return( JE_FALSE );
	Util_ExtBox_Union( prbi->pWorldBounds, &WorldBounds, prbi->pWorldBounds ) ;
	Object_Rotate( pObject, prbi->RAxis, prbi->fRadianAngle, &prbi->RotationCenter ) ;
	if( !Object_GetWorldDrawBounds( pObject, &WorldBounds ) )
		return( JE_FALSE );
	Util_ExtBox_Union( prbi->pWorldBounds, &WorldBounds, prbi->pWorldBounds ) ;

	return JE_TRUE ;
}// Transform_RotateBrush


static jeBoolean Transform_SizeObject( Object * pObject, void * lParam )
{
	SizeBrushInfo * psbi ;
	jeExtBox		WorldBounds ;
	
	psbi = (SizeBrushInfo*)lParam ;
	if( !Object_GetWorldDrawBounds( pObject, &WorldBounds ) )
		return( JE_FALSE );
	Util_ExtBox_Union( psbi->pWorldBounds, &WorldBounds, psbi->pWorldBounds ) ;
	Object_Size( pObject, psbi->pSelectedBounds, psbi->fHScale, psbi->fVScale, psbi->eSizeType, psbi->HAxis, psbi->VAxis ) ;
	if( !Object_GetWorldDrawBounds( pObject, &WorldBounds ) )
		return( JE_FALSE );
	Util_ExtBox_Union( psbi->pWorldBounds, &WorldBounds, psbi->pWorldBounds ) ;

	return JE_TRUE ;
}// Select_DeselectBrush



// Snap to a multiple of the snap size
static jeFloat Transform_SnapCoord( jeFloat fCoord, jeFloat fSnapSize )
{
	jeFloat fRemainder ;

	fRemainder = (jeFloat) fmod( fCoord, fSnapSize ) ;
	if( fabs( fRemainder ) < (fSnapSize *0.5f) )
	{
		return fRemainder ;		
	}
	else
	{
		if( fCoord < 0.0f )
		{
			return fSnapSize + fRemainder ;
		}
		else
		{
			return -(fSnapSize - fRemainder) ;
		}
	}
}// Transform_SnapCoord

// Snap to the nearest grid line
static jeFloat Transform_SnapCoordLR( jeFloat fCoord, jeFloat fSnapSize )
{
	jeFloat fRemainder ;

	fRemainder = (jeFloat) fmod( fCoord, fSnapSize ) ;
	if( fabs( fRemainder ) < (fSnapSize/2.0f) )
	{
		return fRemainder ;		
	}
	else
	{
		if( fCoord < 0.0f )
		{
//			return -(fSnapSize + fRemainder) ;
			return fSnapSize + fRemainder ;
		}
		else
		{
//			return fSnapSize - fRemainder ;
			return fRemainder - fSnapSize ;
		}
	}
}// Transform_Snap

static void Transform_Snap( jeFloat fMin, jeFloat fMax, jeFloat fSnapSize, jeFloat * pResult )
{
	jeFloat fSide1 ;
	jeFloat fSide2 ;

	fSide1 = Transform_SnapCoordLR( fMin, fSnapSize ) ;
	fSide2 = Transform_SnapCoordLR( fMax, fSnapSize ) ;
	
	if( fabs( fSide1 ) < fabs( fSide2 ) )
		*pResult = fSide1 ;
	else
		*pResult = fSide2 ;
}// Transform_Snap


//
// END STATIC
//

void Transform_MoveSelected( Level * pLevel, const jeVec3d * pWorldDistance, jeExtBox * pWorldBounds )
{
	MoveBrushInfo	mbi ;

	assert( pLevel != NULL ) ;
	assert( pWorldDistance != NULL ) ;
	assert( pWorldBounds != NULL ) ;

	Util_ExtBox_SetInvalid( pWorldBounds ) ;
	mbi.pWorldBounds = pWorldBounds ;
	mbi.pLevel = pLevel ;
	mbi.pWorldDistance = pWorldDistance ;

	Level_EnumSelected( pLevel, &mbi, Transform_MoveObject ) ;
	Level_SetModifiedSelection( pLevel ) ;

}// Transform_MoveSelected

void Transform_MoveSelectedSub( Level * pLevel, const jeVec3d * pWorldDistance, jeExtBox * pWorldBounds )
{
	MoveBrushInfo	mbi ;

	assert( pLevel != NULL ) ;
	assert( pWorldDistance != NULL ) ;
	assert( pWorldBounds != NULL ) ;

	Util_ExtBox_SetInvalid( pWorldBounds ) ;
	mbi.pWorldBounds = pWorldBounds ;
	mbi.pLevel = pLevel ;
	mbi.pWorldDistance = pWorldDistance ;

	Level_EnumSubSelected( pLevel, &mbi, Transform_MoveObject ) ;
	Level_SetModifiedSelection( pLevel ) ;

}// Transform_MoveSelectedSub

void Transform_ShearSelected( Level * pLevel, const jeVec3d * pWorldDistance,  SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, jeExtBox * pWorldBounds )
{
	ShearBrushInfo	sbi ;

	assert( pLevel != NULL ) ;
	assert( pWorldDistance != NULL ) ;
	assert( pWorldBounds != NULL ) ;

	Util_ExtBox_SetInvalid( pWorldBounds ) ;
	sbi.pWorldBounds = pWorldBounds ;
	sbi.pLevel = pLevel ;
	sbi.pWorldDistance = pWorldDistance ;
	sbi.eSizeType = eSizeType;
	sbi.HAxis = HAxis;
	sbi.VAxis = VAxis;
	sbi.pSelectedBounds = Level_GetSelDrawBounds( pLevel ) ;
	Level_EnumSelected( pLevel, &sbi, Transform_ShearObject ) ;
	Level_SetModifiedSelection( pLevel ) ;

}// Transform_MoveSelected

void Transform_RotateSelected( Level * pLevel, jeFloat fRadianAngle, ORTHO_AXIS RAxis, jeVec3d *pCenter3d, jeExtBox * pWorldBounds )
{
	RotateBrushInfo	rbi ;
	assert( pLevel != NULL ) ;
	assert( pWorldBounds != NULL ) ;

	Util_ExtBox_SetInvalid( pWorldBounds ) ;
	rbi.pLevel = pLevel ;
	rbi.pWorldBounds = pWorldBounds ;
	rbi.RAxis = RAxis ;
	rbi.fRadianAngle = fRadianAngle ;
	rbi.RotationCenter = *pCenter3d ;

	Level_EnumSelected( pLevel, &rbi, Transform_RotateObject) ;
	Level_SetModifiedSelection( pLevel ) ;
}// Transform_RotateSelected

void Transform_RotateSubSelected( Level * pLevel, jeFloat fRadianAngle, ORTHO_AXIS RAxis, jeExtBox * pWorldBounds )
{
	RotateBrushInfo	rbi ;
	const jeExtBox * pSubDrawBounds;

	assert( pLevel != NULL ) ;
	assert( pWorldBounds != NULL ) ;
	Util_ExtBox_SetInvalid( pWorldBounds ) ;
	rbi.pLevel = pLevel ;
	rbi.pWorldBounds = pWorldBounds ;
	rbi.RAxis = RAxis ;
	rbi.fRadianAngle = fRadianAngle ;
	pSubDrawBounds = Level_GetSubSelDrawBounds( pLevel );
	jeExtBox_GetTranslation( pSubDrawBounds, &rbi.RotationCenter ) ; 

	Level_EnumSubSelected( pLevel, &rbi, Transform_RotateObject) ;
	Level_SetModifiedSelection( pLevel ) ;
}// Transform_RotateSubSelected

#define TRANSFORM_MIN_EXTENT	(0.001f)

static jeFloat Transform_CalcScale( const jeExtBox * pWorldBounds, jeBoolean bHorzAxis, ORTHO_AXIS Axis,  const jeVec3d * pWorldDistance, SELECT_HANDLE eSizeType )
{
	jeFloat fExtent;
	jeFloat fScale = 1.0;
	fExtent = Util_geExtBox_GetExtent( pWorldBounds, Axis ) ;
	if( fExtent < TRANSFORM_MIN_EXTENT )
		return 1.0f;

	switch( eSizeType )
	{
	case Select_Top :
		if( bHorzAxis )
			fScale = 1.0f;
		else
			if( Axis == Ortho_Axis_Z )
				fScale = ( fExtent - jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
			else
				fScale = ( fExtent + jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		break ;

	case Select_Bottom :
		if( bHorzAxis )
			fScale = 1.0f;
		else
			if( Axis == Ortho_Axis_Z )
				fScale = ( fExtent + jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
			else
				fScale = ( fExtent - jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		break ;

	case Select_Left :
		if( bHorzAxis )
			fScale = ( fExtent - jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		else
			fScale = 1.0f;
		break ;

	case Select_Right :
		if( bHorzAxis )
			fScale = ( fExtent + jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		else
			fScale = 1.0f;
		break ;

	case Select_TopLeft :
		if( bHorzAxis )
			fScale = ( fExtent - jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		else
			if( Axis == Ortho_Axis_Z )
				fScale = ( fExtent - jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
			else
				fScale = ( fExtent + jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		break ;

	case Select_TopRight :
		if( bHorzAxis )
			fScale = ( fExtent + jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		else
			if( Axis == Ortho_Axis_Z )
				fScale = ( fExtent - jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
			else
				fScale = ( fExtent + jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		break ;

	case Select_BottomLeft :
		if( bHorzAxis )
			fScale = ( fExtent - jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		else
			if( Axis == Ortho_Axis_Z )
				fScale = ( fExtent + jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
			else
				fScale = ( fExtent - jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		break ;

	
	case Select_BottomRight :
		if( bHorzAxis )
			fScale = ( fExtent + jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		else
			if( Axis == Ortho_Axis_Z )
				fScale = ( fExtent + jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
			else
				fScale = ( fExtent - jeVec3d_GetElement(pWorldDistance,Axis) )/fExtent ;
		break ;

	}
	if( fScale < GEXFORM3D_MINIMUM_SCALE )
		fScale = 1.0f;
	return( fScale );
}

void Transform_SizeSelected( Level * pLevel, const jeVec3d * pWorldDistance, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, jeExtBox * pWorldBounds )
{
	jeFloat				fExtent ;
	SizeBrushInfo		sbi ;
	const jeExtBox	*	pSelectedBounds ;
	assert( pLevel != NULL ) ;
	assert( pWorldDistance != NULL ) ;
	assert( pWorldBounds != NULL ) ;


	pSelectedBounds = Level_GetSelDrawBounds( pLevel ) ;

	sbi.fHScale = Transform_CalcScale( pSelectedBounds, JE_TRUE, HAxis,  pWorldDistance, eSizeType );
	assert( sbi.fHScale >= 0.0f ) ;
	if( sbi.fHScale < GEXFORM3D_SCALE_TOLERANCE )
		sbi.fHScale = GEXFORM3D_SCALE_TOLERANCE ;

	fExtent = Util_geExtBox_GetExtent( pSelectedBounds, VAxis ) ;
	if( fExtent < TRANSFORM_MIN_EXTENT )
		return ;

	sbi.fVScale = Transform_CalcScale( pSelectedBounds, JE_FALSE, VAxis,  pWorldDistance, eSizeType );
	assert( sbi.fVScale >= 0.0f ) ;
	if( sbi.fVScale < GEXFORM3D_SCALE_TOLERANCE )
		sbi.fVScale = GEXFORM3D_SCALE_TOLERANCE ;

	Util_ExtBox_SetInvalid( pWorldBounds ) ;
	sbi.pLevel = pLevel ;
	sbi.pWorldBounds = pWorldBounds ;
	sbi.pWorldDistance = pWorldDistance ;
	sbi.HAxis = HAxis ;
	sbi.VAxis = VAxis ;
	sbi.eSizeType = eSizeType ;
	sbi.pSelectedBounds = pSelectedBounds ;

	Level_EnumSelected( pLevel, &sbi, Transform_SizeObject ) ;
	Level_SetModifiedSelection( pLevel ) ;
	

}// Transform_SizeSelected

void Transform_PlaceSnap( Level * pLevel, jeVec3d *placePt, jeVec3d * pSnapDelta )
{
	jeFloat				fSnapSize ;

	fSnapSize = (Level_IsSnapGrid( pLevel )) ? (jeFloat)Level_GetGridSnapSize( pLevel ) : 1.0f ;
	Transform_SnapPointLR( placePt, fSnapSize, pSnapDelta ) ;
	jeVec3d_Inverse( pSnapDelta ) ;
}

void Transform_MoveSnapSelected( Level * pLevel, SELECT_HANDLE eCorner, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, jeExtBox * pWorldBounds, jeVec3d * pSnapDelta )
{
	const jeExtBox *	pSelBounds ;
	jeVec3d				Corner ;
	jeFloat				fSnapSize ;
	assert( pLevel != NULL ) ;
	assert( pWorldBounds != NULL ) ;

	pSelBounds = Level_GetSelBounds( pLevel ) ;
	Transform_GetHandlePoint( eCorner, HAxis, VAxis, pSelBounds, &Corner ) ;

	fSnapSize = (Level_IsSnapGrid( pLevel )) ? (jeFloat)Level_GetGridSnapSize( pLevel ) : 1.0f ;

	Transform_SnapPointLR( &Corner, fSnapSize, pSnapDelta ) ;
	jeVec3d_Inverse( pSnapDelta ) ;
	Transform_MoveSelected( pLevel, pSnapDelta, pWorldBounds ) ;

}// Transform_MoveSnapSelected

void Transform_SizeSnapSelected
( 
	Level			*	pLevel, 
	SELECT_HANDLE		eCorner, 
	ORTHO_AXIS			HAxis, 
	ORTHO_AXIS			VAxis,
	jeExtBox		*	pWorldBounds, 
	jeVec3d			*	pSnapDelta
)
{
	const jeExtBox *	pSelBounds ;
	jeVec3d				Corner ;
	jeFloat				fSnapSize ;
	assert( pLevel != NULL ) ;
	assert( pWorldBounds != NULL ) ;

	// This begins a move by snapping the moving handle of selected brushes to the
	// grid, based on the current view.  It doesn't try to go the direction the mouse
	// is, just gets on the grid
	pSelBounds = Level_GetSelBounds( pLevel ) ;

	Transform_GetHandlePoint( eCorner, HAxis, VAxis, pSelBounds, &Corner ) ;
	fSnapSize = (Level_IsSnapGrid( pLevel )) ? (jeFloat)Level_GetGridSnapSize( pLevel ) : 1.0f ;

	Transform_SnapPointLR( &Corner, fSnapSize, pSnapDelta ) ;
	jeVec3d_Inverse( pSnapDelta ) ;
	Transform_SizeSelected( pLevel, pSnapDelta, eCorner, HAxis, VAxis, pWorldBounds ) ;

}// Transform_SizeSnapSelected

void Transform_SnapBounds( const jeExtBox * pBox, const jeFloat fSnapSize, jeVec3d * pDelta )
{
	Transform_Snap( pBox->Min.X, pBox->Max.X, fSnapSize, &pDelta->X ) ;
	Transform_Snap( pBox->Min.Y, pBox->Max.Y, fSnapSize, &pDelta->Y ) ;
	Transform_Snap( pBox->Min.Z, pBox->Max.Z, fSnapSize, &pDelta->Z ) ;
}// Transform_SnapBounds

void Transform_SnapPoint( const jeVec3d * pPoint, const jeFloat fSnapSize, jeVec3d * pDelta )
{
	assert( pPoint != NULL ) ;
	assert( pDelta != NULL ) ;
	
	pDelta->X = Transform_SnapCoord( pPoint->X, fSnapSize )	;
	pDelta->Y = Transform_SnapCoord( pPoint->Y, fSnapSize )	;
	pDelta->Z = Transform_SnapCoord( pPoint->Z, fSnapSize )	;
}// Transform_SnapPoint

void Transform_SnapPointLR( const jeVec3d * pPoint, const jeFloat fSnapSize, jeVec3d * pDelta )
{
	assert( pPoint != NULL ) ;
	assert( pDelta != NULL ) ;
	
	pDelta->X = Transform_SnapCoordLR( pPoint->X, fSnapSize )	;
	pDelta->Y = Transform_SnapCoordLR( pPoint->Y, fSnapSize )	;
	pDelta->Z = Transform_SnapCoordLR( pPoint->Z, fSnapSize )	;
}// Transform_SnapPoint

void Transform_PointToGrid( Level * pLevel, const jeVec3d * pPoint,  jeVec3d * pGridPoint )
{
	jeFloat		fSnapSize ;
	jeVec3d		SnapDelta ;
	assert( pLevel != NULL ) ;

	fSnapSize = (Level_IsSnapGrid( pLevel )) ? (jeFloat)Level_GetGridSnapSize( pLevel ) : 1.0f ;
	Transform_SnapPoint( pPoint, fSnapSize, &SnapDelta ) ;
	jeVec3d_Subtract( pPoint, &SnapDelta, pGridPoint ) ;
}// Transform_PointToGrid


void Transform_GetHandlePoint( SELECT_HANDLE eCorner, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, const jeExtBox * pBounds, jeVec3d * pPoint )
{
	assert( pBounds != NULL ) ;
	assert( pPoint != NULL ) ;
	
	// For this View, fill in the two coordinates of the handle associated with this box
	jeVec3d_Clear( pPoint ) ;

	// The Z axis is flipped (down is positive)
	switch( eCorner )
	{
	case Select_Top :
		if( Ortho_Axis_Z == VAxis )
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Min, VAxis ) ) ;
		else
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Max, VAxis ) ) ;
		break ;

	case Select_Bottom :
		if( Ortho_Axis_Z == VAxis )
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Max, VAxis ) ) ;
		else
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Min, VAxis ) ) ;
		break ;

	case Select_Left :
		jeVec3d_SetElement( pPoint, HAxis, jeVec3d_GetElement( &pBounds->Min, HAxis ) ) ;
		break ;

	case Select_Right :
		jeVec3d_SetElement( pPoint, HAxis, jeVec3d_GetElement( &pBounds->Max, HAxis ) ) ;
		break ;

	case Select_TopLeft :
		if( Ortho_Axis_Z == VAxis )
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Min, VAxis ) ) ;
		else
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Max, VAxis ) ) ;
		jeVec3d_SetElement( pPoint, HAxis, jeVec3d_GetElement( &pBounds->Min, HAxis ) ) ;
		break ;		

	case Select_TopRight :
		if( Ortho_Axis_Z == VAxis )
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Min, VAxis ) ) ;
		else
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Max, VAxis ) ) ;
		jeVec3d_SetElement( pPoint, HAxis, jeVec3d_GetElement( &pBounds->Max, HAxis ) ) ;
		break ;

	case Select_BottomLeft :
		if( Ortho_Axis_Z == VAxis )
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Max, VAxis ) ) ;
		else
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Min, VAxis ) ) ;
		jeVec3d_SetElement( pPoint, HAxis, jeVec3d_GetElement( &pBounds->Min, HAxis ) ) ;
		break ;
	
	case Select_BottomRight :
		if( Ortho_Axis_Z == VAxis )
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Max, VAxis ) ) ;
		else
			jeVec3d_SetElement( pPoint, VAxis, jeVec3d_GetElement( &pBounds->Min, VAxis ) ) ;
		jeVec3d_SetElement( pPoint, HAxis, jeVec3d_GetElement( &pBounds->Max, HAxis ) ) ;
		break ;
	}
}// Transform_GetHandlePoint

static jeBoolean Transform_AddUndoCB( Object * pObject, void *lParam  )
{
	jeXForm3d			*	XFormContext;
	jeXForm3d				ObjectXForm;
	Undo				*	pUndo;

	assert( pObject!= NULL );
	assert( lParam != NULL );

	pUndo = (Undo*)lParam;
	if( !Object_GetTransform( pObject, &ObjectXForm ) )
		return( JE_TRUE );
	XFormContext = JE_RAM_ALLOCATE_STRUCT( jeXForm3d );
	if( XFormContext  == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Undable to allocate XForm Context" );
		return( JE_FALSE );
	}
	*XFormContext = ObjectXForm;
	assert( pUndo!= NULL );
	return( Undo_AddSubTransaction( pUndo, UNDO_TRANSFORM, pObject, XFormContext ) );
}// Transform_AddUndoCB

jeBoolean	Transform_AddSelectedUndo( Level * pLevel, UNDO_TYPES Type )
{
	Undo *			pUndo;

	assert( pLevel != NULL ) ;

	pUndo = Level_GetUndo( pLevel );
	assert( pUndo );
	Undo_Push( pUndo, Type );
	Level_EnumSelected( pLevel, pUndo, Transform_AddUndoCB ) ;
	return( JE_TRUE );
}// Transform_AddSelectedUndo

static jeBoolean Transform_AddShearUndoCB( Object * pObject, void *lParam  )
{
	jeBrush			*	pgeBrush;
	Undo				*	pUndo;
	uint32				Contents;

	assert( pObject!= NULL );
	assert( lParam != NULL );

	if( Object_GetKind( pObject ) != KIND_BRUSH )
		Transform_AddUndoCB( pObject, lParam );

	pUndo = (Undo*)lParam;
	pgeBrush = Brush_CopygeBrush( (Brush *)pObject );
	Contents = jeBrush_GetContents( Brush_GetjeBrush( (Brush *)pObject) ) ;
	jeBrush_SetContents( pgeBrush, Contents ) ;

	return( Undo_AddSubTransaction( pUndo, UNDO_BRUSHSHEAR, pObject, pgeBrush ) );
}// Transform_AddUndoCB

jeBoolean	Transform_AddShearSelectedUndo( Level * pLevel )
{
	Undo *			pUndo;

	assert( pLevel != NULL ) ;

	pUndo = Level_GetUndo( pLevel );
	assert( pUndo );
	Undo_Push( pUndo, UNDO_SHEAR );
	Level_EnumSelected( pLevel, pUndo, Transform_AddShearUndoCB ) ;
	return( JE_TRUE );
}// Transform_AddSelectedUndo

#pragma warning (disable:4505)	// unreferenced function

/* EOF: Transform.c */