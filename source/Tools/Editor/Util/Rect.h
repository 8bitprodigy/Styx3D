/****************************************************************************************/
/*  RECT.H                                                                              */
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

// This module is thin wrapper on the Windows Rect

#ifndef RECT_H
#define RECT_H

// Disable inline functions removed warning
#pragma warning( disable: 4514 4201 )

#include "Point.h"
#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	union
	{
		struct {
		int32 Left;
		int32 Top;
		int32 Right;
		int32 Bottom;
		};
		struct {
		Point TopLeft;
		Point BotRight;
		};
	};
} Rect;

__inline int32 Rect_Width(const Rect* pRect) { return(pRect->Right - pRect->Left); }
__inline int32 Rect_Height(const Rect* pRect) { return(pRect->Bottom - pRect->Top); }

jeBoolean	Rect_Compare(const Rect* pRect1, const Rect* pRect2);

jeBoolean	Rect_IsContained(const Rect* pRect, const Rect* pQueryRect);
jeBoolean	Rect_IsEmpty( const Rect * pRect ) ;
jeBoolean	Rect_IsIntersecting(const Rect* pRect1, const Rect* pRect2);
jeBoolean	Rect_IsPointIn( const Rect * pRect, const Point * pPoint ) ;


jeBoolean	Rect_Intersect( Rect * pIntersectedRect, const Rect * pR1, const Rect * pR2 ) ;
void		Rect_Normalize( Rect * pRect ) ;
jeBoolean	Rect_Union( Rect * pDest, const Rect * pR1, const Rect * pR2 ) ;
void		Rect_Offset( Rect * pRect, int32 dx, int32 dy ) ;
void		Rect_Inflate( Rect * pRect, int32 dx, int32 dy ) ;
void		Rect_ExtendToEnclose( Rect * pRect, const Point * pPoint ) ;


void		Rect_SetEmpty( Rect * pRect ) ;

void		Rect_GetTranslation( const Rect * pRect, Point * pCenter ) ;

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
#endif // Prevent multiple inclusion
/* EOF: Rect.h */