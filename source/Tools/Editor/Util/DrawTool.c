/****************************************************************************************/
/*  DRAWTOOL.C                                                                          */
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
#pragma warning(disable : 4201 4214 4115)
#include <Windows.h>
#include <Windowsx.h>
#pragma warning(default : 4201 4214 4115; disable : 4514)

#include "Ram.h"

#include	"DrawTool.h"

typedef struct jwePen
{
	HPEN			hPen;
	HPEN			hOldPen;
} jwePen;

/* Global Pen Handle */
static HPEN PenSelected = NULL; /* CreatePen( PS_SOLID, 1, RGB( 255, 0, 0 ) ); */
static HPEN PenSelFace = NULL; /* CreatePen( PS_SOLID, 1, RGB( 255, 0, 255 ) ); */
static HPEN PenSubBrush = NULL; /* CreatePen( PS_SOLID, 1, RGB( 255, 255, 255 ) ); */
static HPEN PenAddBrush = NULL; /* CreatePen( PS_SOLID, 1, RGB( 255, 128, 0 ) ) */
static HPEN PenSelColor = NULL;

jwePen * Pen_SelectColor( int32 hDC, int32 R, int32 G, int32 B )
{
	jwePen *pjwePen;

	pjwePen = JE_RAM_ALLOCATE_STRUCT_CLEAR( jwePen );
	if( pjwePen == NULL )
		return( NULL );

    if (PenSelColor != NULL) {
        DeletePen(PenSelColor);
        PenSelColor = NULL;
    }
	pjwePen->hPen = PenSelColor = CreatePen( PS_SOLID, 1, RGB( R, G, B ) ) ;
	pjwePen->hOldPen = SelectPen( (HDC)hDC, pjwePen->hPen ) ;
	return( pjwePen );
}

jwePen	* Pen_SelectSelectedColor( int32 hDC ) 
{
	jwePen *pjwePen;

	pjwePen = JE_RAM_ALLOCATE_STRUCT_CLEAR( jwePen );
	if( pjwePen == NULL )
		return( NULL );

    if (PenSelected == NULL) {
        PenSelected = CreatePen( PS_SOLID, 1, RGB( 255, 0, 0 ) );
    }

    pjwePen->hPen = PenSelected;
	pjwePen->hOldPen = SelectPen( (HDC)hDC, pjwePen->hPen ) ;
	return( pjwePen );
}

jwePen	*	Pen_SelectSelectedFaceColor( int32 hDC )
{
	jwePen *pjwePen;

	pjwePen = JE_RAM_ALLOCATE_STRUCT_CLEAR( jwePen );
	if( pjwePen == NULL )
		return( NULL );

    if (PenSelected == NULL) {
        PenSelFace = CreatePen( PS_SOLID, 1, RGB( 255, 0, 255 ) );
    }

    pjwePen->hPen = PenSelFace;
	pjwePen->hOldPen = SelectPen( (HDC)hDC, pjwePen->hPen ) ;
	return( pjwePen );
}

jwePen	* Pen_SelectSubtractBrushColor( int32 hDC )
{
	jwePen *pjwePen;

	pjwePen = JE_RAM_ALLOCATE_STRUCT_CLEAR( jwePen );
	if( pjwePen == NULL )
		return( NULL );

    if (PenSubBrush == NULL) {
        PenSubBrush = CreatePen( PS_SOLID, 1, RGB( 255, 255, 255 ) );
    }

	pjwePen->hPen = PenSubBrush;
	pjwePen->hOldPen = SelectPen( (HDC)hDC, pjwePen->hPen ) ;
	return( pjwePen );
}

jwePen	* Pen_SelectAddBrushColor( int32 hDC ) 
{
	jwePen *pjwePen;

	pjwePen = JE_RAM_ALLOCATE_STRUCT_CLEAR( jwePen );
	if( pjwePen == NULL )
		return( NULL );

    if (PenAddBrush == NULL) {
        PenAddBrush = CreatePen( PS_SOLID, 1, RGB( 255, 128, 0 ) );
    }

	pjwePen->hPen = PenAddBrush;
	pjwePen->hOldPen = SelectPen( (HDC)hDC, pjwePen->hPen ) ;
	return( pjwePen );
}

void Pen_Release( jwePen * pPen, int32 hDC )
{
		SelectPen( (HDC)hDC, pPen->hOldPen ) ;
		//DeletePen( pPen->hPen ) ;
		jeRam_Free( pPen );
}

jeBoolean	Pen_Polyline( int32 hDC, Point * pPoints, int32 nPoints )
{
	return Polyline( (HDC)hDC, (POINT*)pPoints, nPoints ) ;
}

