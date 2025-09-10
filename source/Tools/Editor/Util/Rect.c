/****************************************************************************************/
/*  RECT.C                                                                              */
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
#pragma warning(disable : 4201 4214 4115)
#include <Windows.h>
#include <Windowsx.h>
#pragma warning(default : 4201 4214 4115; disable : 4514)

#include "Rect.h"

jeBoolean Rect_Compare(const Rect* pRect1, const Rect* pRect2)
{
	assert(pRect1 != NULL);
	assert(pRect2 != NULL);

	return(EqualRect((const RECT*)pRect1, (const RECT*)pRect2));
}

jeBoolean Rect_IsContained(const Rect* pRect, const Rect* pQueryRect)
{
	assert(pRect != NULL);
	assert(pQueryRect != NULL);

	if( (pQueryRect->Left >= pRect->Left) &&
		(pQueryRect->Top >= pRect->Top) &&
		(pQueryRect->Right <= pRect->Right) &&
		(pQueryRect->Bottom <= pRect->Bottom) )
	{
		return(JE_TRUE);
	}

	return(JE_FALSE);
}

jeBoolean Rect_IsIntersecting(const Rect* pRect1, const Rect* pRect2)
{
	RECT r;

	assert(pRect1 != NULL);
	assert(pRect2 != NULL);

	return(IntersectRect(&r, (const RECT*)pRect1, (const RECT*)pRect2));
}

// NZ == intersected, Z = no intersection
jeBoolean Rect_Intersect( Rect * pIntersectedRect, const Rect * pR1, const Rect * pR2 )
{
	return IntersectRect( (RECT*)pIntersectedRect, (const RECT*)pR1, (const RECT*)pR2 ) ;
}/* Rect_Intersect */


void Rect_Normalize( Rect * pRect )
{
    int nTemp;

    if ( pRect->Left > pRect->Right)
    {
        nTemp = pRect->Left ;
        pRect->Left = pRect->Right;
        pRect->Right = nTemp;
    }
    if (pRect->Top > pRect->Bottom)
    {
        nTemp = pRect->Top;
        pRect->Top = pRect->Bottom;
        pRect->Bottom = nTemp;
    }
}/* Rect_Normalize */

jeBoolean Rect_Union( Rect * pDest, const Rect * pR1, const Rect * pR2 )
{
	assert( pDest != NULL ) ;
	assert( pR1 != NULL ) ;
	assert( pR2 != NULL ) ;

	return UnionRect( (RECT*)pDest, (const RECT*)pR1, (const RECT*)pR2 ) ;
}/* Rect_Union */


void Rect_Offset( Rect * pRect, int32 dx, int32 dy )
{
	OffsetRect( (RECT*)pRect, dx, dy ) ;
}/* Rect_Offset */

void Rect_Inflate( Rect * pRect, int32 dx, int32 dy )
{
	InflateRect( (RECT*)pRect, dx, dy ) ;
}/* Rect_Inflate */

void Rect_ExtendToEnclose( Rect * pRect, const Point * pPoint )
{
	assert( pRect != NULL );
	assert( pPoint != NULL );

	if( pPoint->X > pRect->Right ) pRect->Right = pPoint->X;
	if( pPoint->Y > pRect->Bottom ) pRect->Bottom = pPoint->Y;

	if( pPoint->X < pRect->Left ) pRect->Left = pPoint->X;
	if( pPoint->Y < pRect->Top ) pRect->Top = pPoint->Y;
}/* Rect_ExtendToEnclose */

void Rect_SetEmpty( Rect * pRect )
{
	assert( pRect != NULL);

	SetRectEmpty( (RECT*)pRect ) ;
}/* Rect_SetEmpty */

//
// IS Functions
//

jeBoolean Rect_IsEmpty( const Rect * pRect )
{
	assert( pRect != NULL);
	
	return IsRectEmpty( (RECT*)pRect ) ;
}/* Rect_IsEmpty */

jeBoolean Rect_IsPointIn( const Rect * pRect, const Point * pPoint )
{
	assert(pRect != NULL);
	assert(pPoint != NULL);

	if( pPoint->X >= pRect->Left && pPoint->X <= pRect->Right )
		if( pPoint->Y >= pRect->Top && pPoint->Y <= pRect->Bottom )
			return JE_TRUE ;

	return JE_FALSE ;
}// Rect_IsPointIn


void Rect_GetTranslation( const Rect * pRect, Point * pCenter )
{
	assert( pRect != NULL ) ;
	assert( pCenter != NULL ) ;

	pCenter->X = (pRect->Left + pRect->Right)/2 ;
	pCenter->Y = (pRect->Top + pRect->Bottom)/2 ;

}// Rect_GetTranslation


/* EOF: Rect.c */