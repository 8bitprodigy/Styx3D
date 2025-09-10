/****************************************************************************************/
/*  DRAW.H                                                                              */
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

#ifndef DRAW_H
#define DRAW_H

#pragma warning(disable : 4201 4214 4115)
#include <Windows.h>
#include <Windowsx.h>
#pragma warning(default : 4201 4214 4115; disable : 4514)

#include "BaseType.h"
#include "Defs.h"
#include "Level.h"
#include "Ortho.h"

#ifdef __cplusplus
extern "C" {
#endif

void		Draw_CornerHandles( Rect * pSelBounds, HDC hDC, MODE eMode );
void		Draw_EdgeHandles( Rect * pSelBounds, HDC hDC, MODE eMode );

void		Draw_Grid( const Level * pLevel, const Ortho * pOrtho, HDC hDC ) ;
void		Draw_Objects(  Level * pLevel, Ortho * pOrtho, HDC hDC );
void		Draw_Selected( Level * pLevel, Ortho * pOrtho, HDC hDC, MODE eMode );
void		Draw_SelectHandles( Level * pLevel, HDC hDC, MODE eMode, Rect*pSelBounds );
void		Draw_OrthoName( Ortho * pOrtho, HDC hDC );
void		Draw_GridAtSize( const Ortho * pOrtho, jeFloat fInterval, HDC hDC ) ;
void		Draw_ConstructorLine( const Level * pLevel, const Ortho * pOrtho, HDC hDC);
void		Draw_SelectBounds( const jeExtBox * pSelWorldBounds, Ortho * pOrtho, HDC hDC, 	Rect * pSelBounds, COLORREF co);
void		Draw_SelectBoundElipse( const jeExtBox * pSelWorldBounds, Ortho * pOrtho, HDC hDC );
void		Draw_SelectGetElipseBox( const jeExtBox * pSelWorldBounds, Ortho * pOrtho, Rect *pBox );
void		Draw_SelectAxis( Level * pLevel, Ortho * pOrtho, HDC hDC );
#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Draw.h */