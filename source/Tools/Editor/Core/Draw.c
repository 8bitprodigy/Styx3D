/****************************************************************************************/
/*  DRAW.C                                                                              */
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
#include <Float.h>

#include "AppData.h"
#include "ExtBox.h"
//#include "jet.h"
#include "GroupList.h"
#include "Rect.h"
#include "Settings.h"
#include "Units.h"
#include "Util.h"

#include "Draw.h"
#include "TerrnObj.h"
#include "Brush.h"
#include "Light.h"
#include "CamObj.h"
#include "UserObj.h"
#include "DrawTool.h"

typedef struct tagBrushDrawData
{
	HDC					hDC ;
	Ortho			*	pOrtho ;
	jeBoolean			bDrawVertex ;
	jeBoolean			bDrawFaces ;
	jeBoolean			bColorOverRide;
} BrushDrawData ;


static jeBoolean Draw_Object( Object *pObject, void *lParam )
{
	BrushDrawData	*	pData = (BrushDrawData*)lParam ;
	uint32 flags;
	HPEN				hOldPen = NULL;
	HPEN				hPen ;

	assert( pObject != NULL );
	assert( lParam != NULL );

	flags = Object_GetMiscFlags( pObject );
	if( flags & AllSubSelect )
	{
		hPen = CreatePen( PS_SOLID, 1, Settings_GetSubSelectedColor( ) ) ;		// Selected objects
		hOldPen = SelectPen( pData->hDC, hPen ) ;
		pData->bColorOverRide = JE_TRUE;
	}
	switch( Object_GetKind( pObject ) )
	{
		case KIND_BRUSH:
			Brush_RenderOrthoFaces( (Brush *)pObject, pData->pOrtho,  (int32)pData->hDC, pData->bDrawVertex,pData->bDrawFaces, pData->bColorOverRide );
			break;

		case KIND_LIGHT:
			Light_RenderOrtho( pData->pOrtho, (Light *)pObject, (int32)pData->hDC, pData->bColorOverRide);
			break;

		case KIND_CAMERA:
			Camera_RenderOrtho( pData->pOrtho, (Camera *)pObject, (int32)pData->hDC, pData->bColorOverRide );
			break;

		case KIND_USEROBJ:
			UserObj_RenderOrtho( pData->pOrtho, (UserObj *)pObject, (int32)pData->hDC, pData->bColorOverRide );
			break;


		case KIND_MODEL:
			break;

		case KIND_CLASS:
			break;

		default:
			assert(0);
			break;
	}
	if( flags & AllSubSelect )
	{
		hPen = SelectPen( pData->hDC, hOldPen ) ;
		DeletePen( hPen ) ;
	}
	return( JE_TRUE );

}// Draw_Object

static void Draw_Handle( HDC hDCDst, HDC hDCSrc, int32 Left, int32 Top )
{
	BitBlt( hDCDst, Left, Top, Left+6, Top+6, hDCSrc, 0, 0, SRCCOPY );
}// Draw_Handle

static HBITMAP Draw_SelectHandleBitmap( HDC hMemDC, SELECT_HANDLE Select_Handle, MODE eMode )
{
	HBITMAP	hBitmap = NULL ;
	assert( Select_Handle > Select_None && Select_Handle < Select_Last ) ;
	assert( eMode > MODE_NONE  && eMode < MODE_LAST ) ;
	
	switch( eMode )
	{
	case MODE_POINTER_BB :
		if( Select_Handle >= Select_Left )	// Edge
			hBitmap = (HBITMAP)AppData_GetHandleEdgeBitmap() ;
		else
			hBitmap = (HBITMAP)AppData_GetHandleCornerBitmap() ;
		break ;

	case MODE_POINTER_RS :
		switch( Select_Handle )
		{
		case Select_TopLeft :		hBitmap = (HBITMAP)AppData_GetHandleRotateTL() ; break ;
		case Select_TopRight :		hBitmap = (HBITMAP)AppData_GetHandleRotateTR() ; break ;
		case Select_BottomLeft :	hBitmap = (HBITMAP)AppData_GetHandleRotateBL() ; break ;
		case Select_BottomRight :	hBitmap = (HBITMAP)AppData_GetHandleRotateBR() ; break ;

		case Select_Left :
		case Select_Right :
			hBitmap = (HBITMAP)AppData_GetHandleShearLR() ; break ;
		case Select_Top :
		case Select_Bottom :
			hBitmap = (HBITMAP)AppData_GetHandleShearTB() ; break ;
		}
		break ;
	}

	return SelectBitmap( hMemDC, hBitmap ) ;

}// Draw_SelectHandleBitmap

void Draw_SelectGetElipseBox( const jeExtBox * pSelWorldBounds, Ortho * pOrtho, Rect *pBox )
{
	Rect SelBounds;
	Point Center;
	double isoceleaseSide;
	int halfHeight;
	int halfWidth;
	int Side;

	Ortho_WorldToView( pOrtho, &pSelWorldBounds->Min, &SelBounds.TopLeft );
	Ortho_WorldToView( pOrtho, &pSelWorldBounds->Max, &SelBounds.BotRight );

	halfHeight = Rect_Height( &SelBounds )/2;
	halfWidth = Rect_Width( &SelBounds )/2;

	Center.X = halfWidth + SelBounds.Left;
	Center.Y = halfHeight + SelBounds.Top;

	isoceleaseSide = sqrt((halfWidth*halfWidth + halfHeight*halfHeight) );
	Side = (int)isoceleaseSide;

	pBox->Left = Center.X - Side;
	pBox->Top = Center.Y - Side;
	pBox->Right = Center.X + Side;
	pBox->Bottom = Center.Y + Side;
}

void Draw_ObjectAxis( Object * pObject, Ortho *pOrtho, HDC hDC )
{
	Point			points[2];
	jeXForm3d XF;
	HPEN				hOldPen ;
	jeVec3d			Vert1 ;
	jeVec3d			Vert2 ;
	HPEN				hPen ;

	if( !Object_GetTransform( pObject, &XF ) )
		return;
	hPen = CreatePen( PS_SOLID, 1, RGB( 255, 255, 255 ) ) ;		// Selected objects
	hOldPen = SelectPen( hDC, hPen ) ;
	Vert1 = XF.Translation ;
	jeXForm3d_GetIn( &XF, &Vert2 );
	jeVec3d_Normalize( &Vert2 );
	jeVec3d_Scale( &Vert2, 16.0f, &Vert2 );
	jeVec3d_Add( &Vert2, &Vert1, &Vert2 );
	Ortho_WorldToView( pOrtho, &Vert1, &points[0] ) ;
	Ortho_WorldToView( pOrtho, &Vert2, &points[1] ) ;
	Pen_Polyline( (long)hDC, points, 2 ) ;
	hPen = SelectPen( (HDC)hDC, hOldPen ) ;
	DeletePen( hPen ) ;
	//TextOut( (HDC)hDC, points[1].X, points[1].Y, "Z", 1 ) ;

	hPen = CreatePen( PS_SOLID, 1, RGB( 255, 0, 0 ) ) ;		// Selected objects
	hOldPen = SelectPen( (HDC)hDC, hPen ) ;
	Vert1 = XF.Translation ;
	jeXForm3d_GetUp( &XF, &Vert2 );
	jeVec3d_Normalize( &Vert2 );
	jeVec3d_Scale( &Vert2, 16.0f, &Vert2 );
	jeVec3d_Add( &Vert2, &Vert1, &Vert2 );
	Ortho_WorldToView( pOrtho, &Vert1, &points[0] ) ;
	Ortho_WorldToView( pOrtho, &Vert2, &points[1] ) ;
	Pen_Polyline( (long)hDC, points, 2 ) ;
	hPen = SelectPen( (HDC)hDC, hOldPen ) ;
	DeletePen( hPen ) ;
	//TextOut( (HDC)hDC, points[1].X, points[1].Y, "Y", 1 ) ;

	hPen = CreatePen( PS_SOLID, 1, RGB( 0, 255, 0 ) ) ;		// Selected objects
	hOldPen = SelectPen( (HDC)hDC, hPen ) ;
	Vert1 = XF.Translation ;
	jeXForm3d_GetLeft( &XF, &Vert2 );
	jeVec3d_Normalize( &Vert2 );
	jeVec3d_Scale( &Vert2, 16.0f, &Vert2 );
	jeVec3d_Add( &Vert2, &Vert1, &Vert2 );
	Ortho_WorldToView( pOrtho, &Vert1, &points[0] ) ;
	Ortho_WorldToView( pOrtho, &Vert2, &points[1] ) ;
	Pen_Polyline( (long)hDC, points, 2 ) ;
	hPen = SelectPen( (HDC)hDC, hOldPen ) ;
	DeletePen( hPen ) ;
	//TextOut( (HDC)hDC, points[1].X, points[1].Y, "X", 1 ) ;
}

void Draw_SelectAxis( Level * pLevel, Ortho * pOrtho, HDC hDC )
{
	ObjectList * pSelList;
	Object * pObject;
	ObjectIterator Iterator;

	pSelList = Level_GetSelList( pLevel );
	assert( pSelList );

	pObject = ObjectList_GetFirst( pSelList, &Iterator );
	while( pObject )
	{
		Draw_ObjectAxis( pObject, pOrtho, hDC );
		pObject = ObjectList_GetNext( pSelList, &Iterator );
	}

}

void Draw_SelectBoundElipse( const jeExtBox * pSelWorldBounds, Ortho * pOrtho, HDC hDC )
{
	HBRUSH				hOldBrush ;
	COLORREF			coBackGround ;
	COLORREF			coOld ;
	Rect				Box;
	
	Draw_SelectGetElipseBox( pSelWorldBounds, pOrtho, &Box );

	coBackGround = Settings_GetSelectedBk() ;
	hOldBrush = SelectBrush( hDC, GetStockObject( NULL_BRUSH ) ) ;
	coOld = SetBkColor( hDC, coBackGround ) ;
	Ellipse(
		hDC,
		Box.Left,
		Box.Top,
		Box.Right,
		Box.Bottom
		);
	SetBkColor( hDC, coOld ) ;
	SelectBrush( hDC, hOldBrush ) ;
}
void Draw_SelectBounds( const jeExtBox * pSelWorldBounds, Ortho * pOrtho, HDC hDC, 	Rect * pSelBounds, COLORREF	co )
{
	HPEN				hOldPen ;
	HPEN				hPen ;
	HBRUSH				hOldBrush ;
	COLORREF			coBackGround ;
	COLORREF			coOld ;

	Ortho_WorldToView( pOrtho, &pSelWorldBounds->Min, &pSelBounds->TopLeft );
	Ortho_WorldToView( pOrtho, &pSelWorldBounds->Max, &pSelBounds->BotRight );
	if(pSelBounds->Top > pSelBounds->Bottom )
	{
		int Temp = pSelBounds->Top;

		pSelBounds->Top = pSelBounds->Bottom;
		pSelBounds->Bottom = Temp;
	}
	if( pSelBounds->Left > pSelBounds->Right )
	{
		int Temp = pSelBounds->Left;

		pSelBounds->Left = pSelBounds->Right;
		pSelBounds->Right = Temp;
	}
	if( Rect_IsEmpty( pSelBounds ) == JE_FALSE )
	{
		coBackGround = Settings_GetSelectedBk() ;

		hPen = CreatePen( PS_DOT, 1, co ) ;
		hOldPen = SelectPen( hDC, hPen ) ;
		hOldBrush = SelectBrush( hDC, GetStockObject( NULL_BRUSH ) ) ;
		coOld = SetBkColor( hDC, coBackGround ) ;
		Rectangle
		( 
			hDC, 
			pSelBounds->Left,
			pSelBounds->Top,
			pSelBounds->Right+1,
			pSelBounds->Bottom+1
		) ;
		SetBkColor( hDC, coOld ) ;
		hPen = SelectPen( hDC, hOldPen ) ;
		DeleteObject( hPen ) ;
		SelectBrush( hDC, hOldBrush ) ;
	}
}

void Draw_CornerHandles( Rect * pSelBounds, HDC hDC, MODE eMode )
{
	HDC					hMemDC ;
	HBITMAP				hOldBitmap ;

	hMemDC = CreateCompatibleDC( hDC ) ;
	hOldBitmap = Draw_SelectHandleBitmap( hMemDC, Select_TopLeft, eMode ) ;
	Draw_Handle( hDC, hMemDC,pSelBounds->Left-3,pSelBounds->Top-3 ) ;
	Draw_SelectHandleBitmap( hMemDC, Select_TopRight, eMode ) ;
	Draw_Handle( hDC, hMemDC,pSelBounds->Right-3,pSelBounds->Top-3 ) ;
	Draw_SelectHandleBitmap( hMemDC, Select_BottomRight, eMode ) ;
	Draw_Handle( hDC, hMemDC,pSelBounds->Right-3,pSelBounds->Bottom-3 ) ;
	Draw_SelectHandleBitmap( hMemDC, Select_BottomLeft, eMode ) ;
	Draw_Handle( hDC, hMemDC,pSelBounds->Left-3,pSelBounds->Bottom-3 ) ;
	SelectBitmap( hMemDC, hOldBitmap ) ;
	DeleteDC( hMemDC ) ;
}

void Draw_EdgeHandles( Rect * pSelBounds, HDC hDC, MODE eMode )
{
	HDC					hMemDC ;
	HBITMAP				hOldBitmap ;
	int32				nMiddle ;

	hMemDC = CreateCompatibleDC( hDC ) ;
	hOldBitmap = Draw_SelectHandleBitmap( hMemDC, Select_TopLeft, eMode ) ;

	Draw_SelectHandleBitmap( hMemDC, Select_Left, eMode ) ;
	nMiddle = (pSelBounds->Top +pSelBounds->Bottom)/2 ;
	Draw_Handle( hDC, hMemDC,pSelBounds->Left-3, nMiddle-3 ) ;
	Draw_Handle( hDC, hMemDC,pSelBounds->Right-3, nMiddle-3 ) ;

	Draw_SelectHandleBitmap( hMemDC, Select_Top, eMode ) ;
	nMiddle =(pSelBounds->Left +pSelBounds->Right)/2 ;
	Draw_Handle( hDC, hMemDC, nMiddle-3,pSelBounds->Top-3 ) ;
	Draw_Handle( hDC, hMemDC, nMiddle-3,pSelBounds->Bottom-3 ) ;

	SelectBitmap( hMemDC, hOldBitmap ) ;
	DeleteDC( hMemDC ) ;
}

void Draw_Objects(  Level * pLevel, Ortho * pOrtho, HDC hDC )
{
	LEVEL_GROUPVIS		GroupVisibility ;
	GroupList		*	pGroups ;
	GroupIterator		pGI ;
	const Group		*	pGroup ;
	BrushDrawData		bdd ;
	HPEN				hOldPen ;
	HPEN				hPen ;

	// Draw the brushes
	GroupVisibility = Level_GetGroupVisibility( pLevel ) ;
	pGroups = Level_GetGroupList( pLevel ) ;
	pGroup = GroupList_GetFirst( pGroups, &pGI ) ;

	bdd.hDC = hDC ;
	bdd.pOrtho = pOrtho ;
	bdd.bDrawVertex = JE_FALSE ;
	bdd.bDrawFaces = JE_FALSE ;
	bdd.bColorOverRide = JE_FALSE;

    // Create and assign the group pen
	hPen = CreatePen( PS_SOLID, 1, Group_GetColor( pGroup ) ) ;

    while( pGroup != NULL )
	{
    	hOldPen = SelectPen( hDC, hPen );
		if( (GroupVisibility == LEVEL_GROUPVIS_ALL) ||
			(GroupVisibility == LEVEL_GROUPVIS_VISIBLE && Group_IsVisible( pGroup )) ||
			(GroupVisibility == LEVEL_GROUPVIS_CURRENT && pGroup == Level_GetCurrentGroup( pLevel )) )
		{
			Level_EnumObjects( pLevel, &bdd, Draw_Object ) ;
		}
        hPen = SelectPen( hDC, hOldPen ) ;
		pGroup = GroupList_GetNext( pGroups, &pGI ) ;
	} while( pGroup != NULL ) ;

    // Restore the default pen
	DeletePen( hPen ) ;
}

void Draw_Selected( Level * pLevel, Ortho * pOrtho, HDC hDC, MODE eMode )
{
	BrushDrawData		bdd ;
	HPEN				hOldPen ;
	HPEN				hPen ;


	bdd.hDC = hDC ;
	bdd.pOrtho = pOrtho ;
	bdd.bColorOverRide = JE_TRUE;
	Level_SetMiscFlags( pLevel, BRUSH_EFLAG_SELECTED ) ;	// Set sel flags

	// Draw the selected things
	bdd.bDrawVertex = ( eMode == MODE_POINTER_VM ) ? JE_TRUE : JE_FALSE ;
	bdd.bDrawFaces = ( eMode == MODE_POINTER_FM ) ? JE_TRUE : JE_FALSE ;
	hPen = CreatePen( PS_SOLID, 1, Settings_GetSelectedColor( ) ) ;		// Selected objects
	hOldPen = SelectPen( hDC, hPen ) ;
	Level_EnumSelected( pLevel, &bdd, Draw_Object ) ;
	Level_EnumSubSelected( pLevel, &bdd, Draw_Object ) ;
	hPen = SelectPen( hDC, hOldPen ) ;
	DeletePen( hPen ) ;
	Level_ClearMiscFlags( pLevel, BRUSH_EFLAG_SELECTED ) ;	// Temporarily set sel flags
}

void Draw_SelectHandles( Level * pLevel, HDC hDC, MODE eMode, Rect*pSelBounds )
{
	int32				ModFlags;

	ModFlags = Level_SelXFormModFlags( pLevel );
	if( (eMode == MODE_POINTER_BB && ModFlags & JE_OBJECT_XFORM_SCALE) ||
		(eMode == MODE_POINTER_RS  && ModFlags & JE_OBJECT_XFORM_ROTATE))
	{
	
		Draw_CornerHandles( pSelBounds, hDC, eMode );

	}

	if( (eMode == MODE_POINTER_BB && ModFlags & JE_OBJECT_XFORM_SCALE) ||
		(eMode == MODE_POINTER_RS  && ModFlags & JE_OBJECT_XFORM_SHEAR))
	{
		Draw_EdgeHandles( pSelBounds, hDC, eMode );

	}
}

void Draw_OrthoName( Ortho * pOrtho, HDC hDC )
{
	const char		*	pszViewName ;
	int					nOldMode ;

	pszViewName = Ortho_GetName( pOrtho ) ;
	nOldMode = SetBkMode( hDC, TRANSPARENT ) ;
	TextOut( hDC, 4, 4, pszViewName, strlen( pszViewName ) ) ;
	SetBkMode( hDC, nOldMode ) ;
}


void Draw_Grid( const Level * pLevel, const Ortho * pOrtho, HDC hDC ) 
{
	jeFloat	fGridSize ;
	jeFloat	fSnapSize ;
	HPEN	hPen ;
	HPEN	hOldPen ;

	assert( pLevel != NULL ) ;
	assert( pOrtho != NULL ) ;

	fGridSize = Ortho_GetGridDistance( pOrtho ) ;
	fSnapSize = (jeFloat)Level_GetGridSnapSize( pLevel ) ;

	//  If the grid size and the snap size are the same, then just render
	//  the snap grid.
	//  Otherwise we always want to render the regular grid.  If the
	//  snap grid is larger than the regular grid, then render it, too.
#if 1
	if( fGridSize == fSnapSize )
	{
		hPen = CreatePen( PS_SOLID, 1, Settings_GetGridSnapColor() ) ;
		hOldPen = SelectPen( hDC, hPen ) ;
		Draw_GridAtSize( pOrtho, fSnapSize, hDC ) ;
		SelectPen( hDC, hOldPen ) ;
		DeletePen( hPen ) ;
	}
	else
	{
		hPen = CreatePen( PS_SOLID, 1, Settings_GetGridColor() ) ;
		hOldPen = SelectPen( hDC, hPen ) ;
		Draw_GridAtSize( pOrtho, fGridSize, hDC ) ;

		if( fSnapSize > fGridSize )			// render snap grid?
		{
			SelectPen( hDC, hOldPen ) ;
			DeletePen( hPen ) ;
			hPen = CreatePen( PS_SOLID, 1, Settings_GetGridSnapColor() ) ;
			hOldPen = SelectPen( hDC, hPen ) ;
			Draw_GridAtSize( pOrtho, fSnapSize, hDC ) ;
		}

		SelectPen( hDC, hOldPen ) ;
		DeletePen( hPen ) ;
	}
#else
//	hPen = CreatePen( PS_SOLID, 1, Settings_GetGridSnapColor() ) ;
//	hOldPen = SelectPen( hDC, hPen ) ;
//	Draw_GridAtSize( pOrtho, fGridSize, hDC ) ;
//	SelectPen( hDC, hOldPen ) ;
//	DeletePen( hPen ) ;

	hPen = CreatePen( PS_SOLID, 1, Settings_GetGridColor() ) ;
	hOldPen = SelectPen( hDC, hPen ) ;
	Draw_GridAtSize( pOrtho, fGridSize, hDC ) ;
	SelectPen( hDC, hOldPen ) ;
	DeletePen( hPen ) ;
#endif
}// Draw_Grid

void Draw_ConstructorLine( const Level * pLevel, const Ortho * pOrtho, HDC hDC ) 
{
	float Plane;
	float TempFloat;
	jeVec3d TempVec;
	Point MinPt;
	Point MaxPt;
	HPEN				hPen ;
	HPEN				hOldPen ;
	jeExtBox			ViewBox ;
	ORTHO_AXIS			OrthoAxis ;
	jeVec3d				XTemp ;

	// Setup the world bounding box for this view (xz(top), xy(front), yz(side))
	Ortho_ViewToWorld( pOrtho, 0, 0, &XTemp ) ;
	jeExtBox_SetToPoint( &ViewBox, &XTemp ) ;
	Ortho_ViewToWorld( pOrtho, Ortho_GetWidth( pOrtho ), Ortho_GetHeight( pOrtho ), &XTemp ) ;
	jeExtBox_ExtendToEnclose( &ViewBox, &XTemp ) ;

	// The remaining axis is set to extremes
	OrthoAxis = Ortho_GetOrthogonalAxis( pOrtho ) ;
	jeVec3d_SetElement( &ViewBox.Min, OrthoAxis, -FLT_MAX ) ;
	jeVec3d_SetElement( &ViewBox.Max, OrthoAxis, FLT_MAX ) ;

	hPen = CreatePen( PS_SOLID, 1, Settings_GetConstructorColor() ) ;
	hOldPen = SelectPen( hDC, hPen ) ;
	
	//Draw Vertical Constructor
	Plane = Level_GetConstructorPlane( pLevel, Ortho_GetVerticalAxis( pOrtho ) );
	
	//Does the Plane intersect the view box
	if( Plane > jeVec3d_GetElement( &ViewBox.Min, Ortho_GetVerticalAxis( pOrtho ) ) &&
		Plane < jeVec3d_GetElement( &ViewBox.Max, Ortho_GetVerticalAxis( pOrtho ) ) )
	{
		// Calc the Min point of the line to be drawn
		jeVec3d_SetElement( &TempVec, Ortho_GetVerticalAxis( pOrtho ), Plane );
		
		// set the horzontal axis element to view Min
		TempFloat = jeVec3d_GetElement( &ViewBox.Min, Ortho_GetHorizontalAxis( pOrtho ) );
		jeVec3d_SetElement( &TempVec, Ortho_GetHorizontalAxis( pOrtho ), TempFloat );

		// set the Orthognal axis element to 0.0f since its not important here
		jeVec3d_SetElement( &TempVec, Ortho_GetOrthogonalAxis( pOrtho ), 0.0f );

		//Get the Min View point
		Ortho_WorldToView( pOrtho, &TempVec, &MinPt ) ;

		// set the horzontal axis element to view Max
		TempFloat = jeVec3d_GetElement( &ViewBox.Max, Ortho_GetHorizontalAxis( pOrtho ) );
		jeVec3d_SetElement( &TempVec, Ortho_GetHorizontalAxis( pOrtho ), TempFloat );

		//Get the Max View point
		Ortho_WorldToView( pOrtho, &TempVec, &MaxPt ) ;
		
		MoveToEx( hDC, MinPt.X, MinPt.Y, NULL ) ;
		LineTo( hDC,  MaxPt.X, MaxPt.Y) ;
	}

	//Draw Horzontal Constructor
	Plane = Level_GetConstructorPlane( pLevel, Ortho_GetHorizontalAxis( pOrtho ) );
	
	//Does the Plane intersect the view box
	if( Plane > jeVec3d_GetElement( &ViewBox.Min, Ortho_GetHorizontalAxis( pOrtho ) ) &&
		Plane < jeVec3d_GetElement( &ViewBox.Max, Ortho_GetHorizontalAxis( pOrtho ) ) )
	{
		// Calc the Min point of the line to be drawn
		jeVec3d_SetElement( &TempVec, Ortho_GetHorizontalAxis( pOrtho ), Plane );
		
		// set the horzontal axis element to view Min
		TempFloat = jeVec3d_GetElement( &ViewBox.Min, Ortho_GetVerticalAxis( pOrtho ) );
		jeVec3d_SetElement( &TempVec, Ortho_GetVerticalAxis( pOrtho ), TempFloat );

		// set the Orthognal axis element to 0.0f since its not important here
		jeVec3d_SetElement( &TempVec, Ortho_GetOrthogonalAxis( pOrtho ), 0.0f );

		//Get the Min View point
		Ortho_WorldToView( pOrtho, &TempVec, &MinPt ) ;

		// set the horzontal axis element to view Max
		TempFloat = jeVec3d_GetElement( &ViewBox.Max, Ortho_GetVerticalAxis( pOrtho ) );
		jeVec3d_SetElement( &TempVec, Ortho_GetVerticalAxis( pOrtho ), TempFloat );

		//Get the Max View point
		Ortho_WorldToView( pOrtho, &TempVec, &MaxPt ) ;
		
		MoveToEx( hDC, MinPt.X, MinPt.Y, NULL ) ;
		LineTo( hDC,  MaxPt.X, MaxPt.Y ) ;
	}

	SelectPen( hDC, hOldPen ) ;
	DeletePen( hPen ) ;
}

void Draw_GridAtSize( const Ortho * pOrtho, jeFloat fInterval, HDC hDC )
{
	jeVec3d		xstep ;
	jeVec3d		ystep ;
	jeVec3d		Delta, End ;
	int			i ;
	int			cnt ;
	jeFloat		gsinv ;
	jeExtBox	ViewBox ;
	Point		sp ;
	ORTHO_AXIS	HAxis ;
	ORTHO_AXIS	VAxis ;
	ORTHO_AXIS	OAxis ;

	HAxis = Ortho_GetHorizontalAxis( pOrtho ) ;
	VAxis = Ortho_GetVerticalAxis( pOrtho ) ;
	OAxis = Ortho_GetOrthogonalAxis( pOrtho ) ;
//if( OAxis == 1 )
//{
//	char szDebug[128] ;
//	sprintf( szDebug, "%4.2f\n", fInterval ) ;
//	OutputDebugString( szDebug ) ;
//}

	Ortho_ViewToWorld
	( 
		pOrtho, 
		(int32)Util_Round(-fInterval), 
		(int32)Util_Round(-fInterval), 
		&Delta
	) ;
	Ortho_ViewToWorld
	( 
		pOrtho, 
		(int32)Util_Round(Ortho_GetWidth(pOrtho) + fInterval), 
		(int32)Util_Round(Ortho_GetHeight(pOrtho)+ fInterval), 
		&End
	) ;

	jeExtBox_Set 
	(
		&ViewBox,
		Delta.X, Delta.Y, Delta.Z,
		End.X, End.Y, End.Z
	) ;

	jeVec3d_SetElement( &ViewBox.Min, OAxis, -FLT_MAX ) ;
	jeVec3d_SetElement( &ViewBox.Max, OAxis, FLT_MAX ) ;

	//snap ViewBox to the grid
	gsinv = 1.0f/(jeFloat)fInterval;
	jeVec3d_SetElement
	( 
		&ViewBox.Min, 
		HAxis,
		((int)(jeVec3d_GetElement( &ViewBox.Min, HAxis ) * gsinv))*fInterval
	);
	jeVec3d_SetElement
	( 
		&ViewBox.Max, 
		HAxis,
		((int)(jeVec3d_GetElement( &ViewBox.Max, HAxis ) * gsinv))*fInterval
	);
	jeVec3d_SetElement
	( 
		&ViewBox.Min, 
		VAxis,
		((int)(jeVec3d_GetElement( &ViewBox.Min, VAxis ) * gsinv))*fInterval
	);
	jeVec3d_SetElement
	( 
		&ViewBox.Max, 
		VAxis,
		((int)(jeVec3d_GetElement( &ViewBox.Max, VAxis ) * gsinv))*fInterval
	);
// ???? WHY does this change OAxis to Zero?
//	for(i=0;i<3;i++)
//	{
//		VectorToSUB(ViewBox.Min, i)	=(jeFloat) ((int)(VectorToSUB(ViewBox.Min, i)*gsinv))*Interval;
//		VectorToSUB(ViewBox.Max, i)	=(jeFloat) ((int)(VectorToSUB(ViewBox.Max, i)*gsinv))*Interval;
//	}

	jeVec3d_Clear( &xstep ) ;
	jeVec3d_Clear( &ystep ) ;
	jeVec3d_SetElement( &xstep, HAxis, fInterval ) ;
	jeVec3d_SetElement( &ystep, VAxis, fInterval ) ;
//	VectorToSUB(ystep, yaxis)	=(jeFloat)Interval;
//	VectorToSUB(xstep, xaxis)	=(jeFloat)Interval;

	// horizontal lines
	jeVec3d_Copy( &ViewBox.Min, &Delta );
	jeVec3d_Copy( &ViewBox.Min, &End );
//	VectorToSUB(Delt2, xaxis)	=VectorToSUB(ViewBox.Max, xaxis);
	jeVec3d_SetElement( &End, HAxis, jeVec3d_GetElement( &ViewBox.Max, HAxis ) ) ;
//	cnt	=Units_Round((VectorToSUB(ViewBox.Max, yaxis) - VectorToSUB(ViewBox.Min, yaxis))*gsinv);
	cnt = Units_Round
	(
		(jeVec3d_GetElement( &ViewBox.Max, VAxis ) - jeVec3d_GetElement( &ViewBox.Min, VAxis ))*gsinv
	) ;
	for( i=0; i <= cnt; i++ )
	{
		Ortho_WorldToView( pOrtho, &Delta, &sp );
		MoveToEx( hDC, 0, sp.Y, NULL ) ;
//Ortho_WorldToView( pOrtho, &End, &sp );
		LineTo( hDC, Ortho_GetWidth( pOrtho ), sp.Y ) ;
		jeVec3d_Add( &Delta, &ystep, &Delta ) ;
//jeVec3d_Add( &End, &ystep, &End ) ;
	}

	// vertical lines
	jeVec3d_Copy( &ViewBox.Min, &Delta ) ;
	jeVec3d_Copy( &ViewBox.Min, &End ) ;
//	VectorToSUB(Delt2, yaxis)	=VectorToSUB(ViewBox.Max, yaxis);
	jeVec3d_SetElement( &End, VAxis, jeVec3d_GetElement( &ViewBox.Max, VAxis ) ) ;
//	cnt	=Units_Round((VectorToSUB(ViewBox.Max, xaxis) - VectorToSUB(ViewBox.Min, xaxis))*gsinv);
	cnt = Units_Round
	(
		(jeVec3d_GetElement( &ViewBox.Max, HAxis ) - jeVec3d_GetElement( &ViewBox.Min, HAxis)) * gsinv
	) ;
	for( i=0; i <= cnt ; i++ )
	{
		Ortho_WorldToView( pOrtho, &Delta, &sp ) ;
		MoveToEx( hDC, sp.X, 0, NULL ) ;
//Ortho_WorldToView( pOrtho, &End, &sp ) ;
		LineTo( hDC, sp.X, Ortho_GetHeight( pOrtho ) ) ;
		jeVec3d_Add( &Delta, &xstep, &Delta ) ;
//jeVec3d_Add( &End, &xstep, &End ) ;
	}


}//Draw_Grid


/* EOF: Draw.h */