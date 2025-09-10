/****************************************************************************************/
/*  DRAWTOOL.H                                                                          */
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
#ifndef DRAWTOOL_DEFINE
#define DRAWTOOL_DEFINE

#include <Windows.h>
#include "Point.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jwePen jwePen;

jwePen  *	Pen_SelectColor( int32 hDC, int32 R, int32 G, int32 B );
jwePen	*	Pen_SelectSelectedColor( int32 hDC ) ;
jwePen	*	Pen_SelectSubtractBrushColor( int32 hDC ) ;
jwePen	*	Pen_SelectAddBrushColor( int32 hDC ) ;
jwePen	*	Pen_SelectSelectedFaceColor( int32 hDC ) ;
void		Pen_Release( jwePen * pPen, int32 hDC );
jeBoolean	Pen_Polyline( int32 hDC, Point * pPoints, int32 nPoints ) ;


#ifdef __cplusplus
}
#endif

#endif //DRAWTOOL_DEFINE
