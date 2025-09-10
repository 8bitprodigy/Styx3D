/****************************************************************************************/
/*  VEC2D.H                                                                             */
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
#ifndef VEC2D_H
#define VEC2D_H

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeVec2d
{
	jeFloat	X;
	jeFloat	Y;
} jeVec2d;

void	jeVec2d_Set( jeVec2d * V, jeFloat X, jeFloat Y ) ;

void	jeVec2d_Add( const jeVec2d * pV1, const jeVec2d * pV2, jeVec2d * pV1PlusV2 ) ;
void	jeVec2d_Copy( const jeVec2d *VSrc, jeVec2d *VDst ) ;
void	jeVec2d_Clear( jeVec2d *V ) ;
jeFloat	jeVec2d_DistanceBetween( const jeVec2d *V1, const jeVec2d *V2 ) ;
jeFloat	jeVec2d_DistBetweenSquared( const jeVec2d *V1, const jeVec2d *V2 );
jeFloat	jeVec2d_DotProduct( const jeVec2d *V1, const jeVec2d *V2 ) ;
jeFloat	jeVec2d_Length( const jeVec2d *V1 ) ;
jeFloat	jeVec2d_Normalize( jeVec2d *V1 ) ;
void	jeVec2d_Scale( const jeVec2d *VSrc, jeFloat fScale, jeVec2d *VDst) ;
void	jeVec2d_Subtract( const jeVec2d *V1, const jeVec2d *V2, jeVec2d *V1MinusV2 ) ;

			//(assuming positive X is along 3 o'clock and Y is along '12')
void	jeVec2d_Perp_Clockwise( const jeVec2d *Src, jeVec2d *Dst);
void	jeVec2d_Perp_CClockwise( const jeVec2d *Src, jeVec2d *Dst);
			// makes a perpendicular vector (as close to cross product as you get in 2d)
void	jeVec2d_Rotate( const jeVec2d *pVec, jeFloat Radians, jeVec2d *pDest);
			// rotates clockwise (!?)

int		jeVec2d_SideX(const jeVec2d *pSeg1,const jeVec2d *pSeg2,const jeVec2d *pPoint);
			// -1 if point is to the left, +1 to the right, 0 is not in Y range
			// this is not oriented, its absolte X-lower is left

#define jeVec2d_LengthSquared(V)	jeVec2d_DotProduct(V,V)

#ifdef __cplusplus
}
#endif

#endif // Prevent Multiple Inclusion
/* EOF: jeVec2d.h */
