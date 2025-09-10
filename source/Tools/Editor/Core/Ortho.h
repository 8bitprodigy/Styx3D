/****************************************************************************************/
/*  ORTHO.H                                                                             */
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

#ifndef ORTHO_H
#define ORTHO_H

#include "ExtBox.h"
#include "Point.h"
#include "Rect.h"
#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagOrtho Ortho ;
typedef enum 
{
	Ortho_ViewFront,
	Ortho_ViewSide,
	Ortho_ViewTop,
} Ortho_ViewType ;


Ortho *			Ortho_Create( void ) ;
void			Ortho_Destroy( Ortho ** ppOrtho ) ;

// ACCESSORS
jeFloat			Ortho_GetGridDistance( const Ortho * pOrtho ) ;
long			Ortho_GetHeight( const Ortho * pOrtho ) ;
ORTHO_AXIS		Ortho_GetHorizontalAxis( const Ortho * pOrtho ) ;
const char *	Ortho_GetName( const Ortho * pOrtho ) ;
ORTHO_AXIS		Ortho_GetOrthogonalAxis( const Ortho * pOrtho ) ;
jeFloat			Ortho_GetRotationFromView(const Ortho * pOrtho, Point *pMousePt, Point *pAnchor, Point * pSelCenter ) ;
ORTHO_AXIS		Ortho_GetVerticalAxis( const Ortho * pOrtho ) ;
Ortho_ViewType	Ortho_GetViewType( const Ortho * pOrtho ) ;
int32			Ortho_GetViewSelectThreshold( Ortho * pOrtho ) ;
long			Ortho_GetWidth( const Ortho * pOrtho ) ;
jeFloat			Ortho_GetWorldHandleSelectThreshold( const Ortho * pOrtho ) ;
jeFloat			Ortho_GetWorldSelectThreshold( const Ortho * pOrtho ) ;

// IS
jeBoolean		Ortho_IsViewPointInWorldBox( const Ortho * pOrtho, const int x, const int y, const jeExtBox * pWorldBox ) ;

// MODIFIERS
void			Ortho_MoveCamera( Ortho * pOrtho, const jeVec3d * pDelta ) ;
void			Ortho_ResetSettings( Ortho * pOrtho, long vx, long vy ) ;
void			Ortho_ResizeView( Ortho * pOrtho, long vx, long vy ) ;
void			Ortho_SetAngles( Ortho * pOrtho, const jeVec3d * pAngles ) ;
void			Ortho_SetAnglesRPY( Ortho * pOrtho, jeFloat roll, jeFloat pitch, jeFloat yaw ) ;
void			Ortho_SetBoxOrthogonalToMax( const Ortho * pOrtho, jeExtBox * pBox ) ;
void			Ortho_SetCameraPos( Ortho * pOrtho, const jeVec3d * pPos ) ;
void			Ortho_SetSelectThreshold( Ortho * pOrtho, const int nPixels ) ;
void			Ortho_SetViewType( Ortho * pOrtho, const Ortho_ViewType vt ) ;
void			Ortho_SetZoom( Ortho * pOrtho, const jeFloat zf ) ;
void			Ortho_UpdateWorldBounds( Ortho * pOrtho ) ;
void			Ortho_ZoomChange( Ortho * pOrtho, const jeFloat fFactor ) ;


// COORDINATES AND TRANSLATION
void			Ortho_GetViewCenter( const Ortho * pOrtho, jeVec3d * pCenter ) ;
void			Ortho_ViewToWorld( const Ortho * pOrtho, const int x, const int y, jeVec3d *pW ) ;
void			Ortho_ViewToWorldDistance( const Ortho * pOrtho, const int x, const int y, jeVec3d *pW ) ;
void			Ortho_ViewToWorldRect( const Ortho * pOrtho, const Point * pV1, const Point * pV2, jeExtBox * pWorldBox ) ;
void			Ortho_WorldToView( const Ortho * pOrtho, const jeVec3d * pW, Point * pPt ) ;
void            Ortho_WorldToViewRect( const Ortho * pOrtho, const jeExtBox * pWorldBox, Rect * pViewRect ) ;
jeBoolean		Ortho_TestWorldToViewRect( const Ortho * pOrtho, const jeExtBox * pWorldBox, Rect * pViewRect ) ;


#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Ortho.h */
