/****************************************************************************************/
/*  TRANSFORM.H                                                                         */
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

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "BaseType.h"
#include "Brush.h"
#include "Level.h"
#include "XForm3d.h"

#ifdef __cplusplus
extern "C" {
#endif

void		Transform_GetHandlePoint( SELECT_HANDLE eCorner, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, const jeExtBox * pBounds, jeVec3d * pPoint ) ;
void		Transform_PlaceSnap( Level * pLevel, jeVec3d *placePt, jeVec3d * pSnapDelta );
void		Transform_MoveSelected( Level * pLevel, const jeVec3d * pWorldDistance, jeExtBox * pWorldBounds ) ;
void		Transform_MoveSelectedSub( Level * pLevel, const jeVec3d * pWorldDistance, jeExtBox * pWorldBounds );
void		Transform_MoveSnapSelected( Level * pLevel, SELECT_HANDLE eCorner, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, jeExtBox * pWorldBounds, jeVec3d * pSnapDelta ) ;
void		Transform_PointToGrid( Level * pLevel, const jeVec3d * pPoint, jeVec3d * pGridPoint ) ;
void		Transform_RotateSelected( Level * pLevel, jeFloat fRadianAngle, ORTHO_AXIS RAxis, jeVec3d *pCenter3d, jeExtBox * pWorldBounds ) ;
void		Transform_RotateSubSelected( Level * pLevel, jeFloat fRadianAngle, ORTHO_AXIS RAxis, jeExtBox * pWorldBounds ) ;
void		Transform_SizeSelected( Level * pLevel, const jeVec3d * pWorldDistance, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, jeExtBox * pWorldBounds ) ;
void		Transform_SizeSnapSelected( Level *	pLevel, SELECT_HANDLE eCorner, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, jeExtBox * pWorldBounds, jeVec3d * pSnapDelta ) ;
void		Transform_SnapBounds( const jeExtBox * pBox, const jeFloat fSnapSize, jeVec3d * pDelta ) ;
void		Transform_SnapPoint( const jeVec3d * pPoint, const jeFloat fSnapSize, jeVec3d * pDelta ) ;
void		Transform_SnapPointLR( const jeVec3d * pPoint, const jeFloat fSnapSize, jeVec3d * pDelta ) ;
jeBoolean	Transform_AddSelectedUndo( Level * pLevel, UNDO_TYPES Type  );
jeBoolean	Transform_AddShearSelectedUndo( Level * pLevel );
void Transform_ShearSelected( Level * pLevel, const jeVec3d * pWorldDistance,  SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, jeExtBox * pWorldBounds );
#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Transform.h */