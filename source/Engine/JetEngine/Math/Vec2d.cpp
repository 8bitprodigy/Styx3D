/****************************************************************************************/
/*  VEC2D.C                                                                             */
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
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "Vec2d.h"

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif

void jeVec2d_Set( jeVec2d * V, jeFloat X, jeFloat Y )
{
	assert( V ) ;

	V->X = X ;
	V->Y = Y ;
}

void jeVec2d_Add( const jeVec2d * pV1, const jeVec2d * pV2, jeVec2d * pV1PlusV2 )
{
	assert ( pV1 );
	assert ( pV2 );
	assert ( pV1PlusV2 );
	
	pV1PlusV2->X = pV1->X + pV2->X;
	pV1PlusV2->Y = pV1->Y + pV2->Y;
}/* jeVec2d_Add */

void jeVec2d_Copy(const jeVec2d *VSrc, jeVec2d *VDst)
{
	assert ( VSrc );
	assert ( VDst );
	
	*VDst = *VSrc;
}//jeVec2d_Copy

void jeVec2d_Clear( jeVec2d *V)
{
	assert ( V );
	
	V->X = 0.0f;
	V->Y = 0.0f;
}//jeVec2d_Clear


jeFloat jeVec2d_DotProduct(const jeVec2d *V1, const jeVec2d *V2)
{
	assert ( V1 );
	assert ( V2 );
	
	return(V1->X*V2->X + V1->Y*V2->Y);
}//jeVec2d_DotProduct


jeFloat jeVec2d_DistanceBetween(const jeVec2d *V1, const jeVec2d *V2)	// returns length of V1-V2	
{
	jeVec2d B;
	
	assert( V1 );
	assert( V2 );

	jeVec2d_Subtract(V1,V2,&B);
	return jeVec2d_Length(&B);
}// jeVec2d_DistanceBetween

// returns length of V1-V2 Squared
jeFloat jeVec2d_DistBetweenSquared(const jeVec2d *V1, const jeVec2d *V2)
{
	jeFloat f, d;
	
	assert( V1 );
	assert( V2 );

	f = V2->Y - V1->Y;
	f *= f;
	d = V2->X - V1->X;
	d *= d;
	d += f;

	return(d);

} // jeVec2d_DistanceBetween


jeFloat jeVec2d_Length(const jeVec2d *V1)
{	
float Len;

#ifdef WIN32
	__asm
	{
		mov		eax,V1
		fld		[eax+0]		//	st(0) = vec->X
		fmul	[eax+0]		//	st(0) = vecX ^2

		fld		[eax+4]
		fmul	[eax+4]
							// now st(0),st(1) are Y^2,X^2
	    fxch    st(1)       // ST(0) mul is still in progress, so let's add
							//	ST(1) and ST(2)
        faddp   st(1),st(0) 
		fsqrt
        fstp    [Len]
	}
#endif
#ifdef BUILD_BE
	__asm__ __volatile__ ("
		movl %1, %%eax //; %%eax, %1  ;$1, %%eax
		fldl 0(%%eax) //[%%eax] ;// st(0) = vec->X
		fmull 0(%%eax) // [%%eax] ;// st(0) = vecX ^2
		
		fldl		4(%%eax) //[%%eax+4]
		fmull	4(%%eax) // [%%eax+4]
							//;// now st(0),st(1) are Y^2,X^2
	    fxch    %%st(1)     // ; // ST(0) mul is still in progress, so let's add
							//;//	ST(1) and ST(2)
        faddp   %%st(1),%%st(0) 
		fsqrt
        fstp    %0 //;[Len]
        "
        : "=m" (Len)// outputs
        : "g" (V1)
        : "%eax" , "%st(1)"// registers
        );
#endif

return Len;
}
//Vec2d_Length


jeFloat jeVec2d_Normalize( jeVec2d *V1 )
{
	jeFloat	OneOverDist;
	jeFloat	Dist;

	assert( V1 );

	Dist = jeVec2d_Length(V1);

	OneOverDist = 1.0f/( Dist + 0.000000001f);
	
	V1->X *= OneOverDist;
	V1->Y *= OneOverDist;

	return Dist;
}// jeVec2d_Normalize

void jeVec2d_Scale( const jeVec2d *VSrc, jeFloat fScale, jeVec2d *VDst)
{
	assert ( VSrc );
	assert ( VDst );

	VDst->X = VSrc->X * fScale;
	VDst->Y = VSrc->Y * fScale;
}// jeVec2d_Scale


void jeVec2d_Subtract(const jeVec2d *V1, const jeVec2d *V2, jeVec2d *V1MinusV2)
{
	assert ( V1 );
	assert ( V2 );
	assert ( V1MinusV2 );

	V1MinusV2->X = V1->X - V2->X;
	V1MinusV2->Y = V1->Y - V2->Y;
}// jeVec2d_Subtract

void	jeVec2d_Perp_Clockwise( const jeVec2d *pVec, jeVec2d *pDest)
{
jeVec2d Vec;
	// rotates by 90 clockwise:
	assert(pVec && pDest);
	Vec = *pVec;
	pDest->X =	 Vec.Y;
	pDest->Y = - Vec.X;
}

void	jeVec2d_Perp_CClockwise( const jeVec2d *pVec, jeVec2d *pDest)
{
jeVec2d Vec;
	assert(pVec && pDest);
	Vec = *pVec;
	// rotates by 90 counterclockwise:
	pDest->X = - Vec.Y;
	pDest->Y =   Vec.X;
}

void	jeVec2d_Rotate( const jeVec2d *pVec, jeFloat Radians, jeVec2d *pDest)
{
jeVec2d Vec;
jeFloat c,s;
	
	assert(pVec);
	Vec = *pVec;

	// rotates clockwise:
	//	(really? seems true)

	c = jeFloat_Cos(Radians);
	s = jeFloat_Sin(Radians);

	pDest->X = + c * Vec.X + s * Vec.Y;
	pDest->Y = - s * Vec.X + c * Vec.Y;
}

int		jeVec2d_SideX(const jeVec2d *pSeg1,const jeVec2d *pSeg2,const jeVec2d *pPoint)
{
jeFloat minY,maxY,minX,maxX;
	assert( pSeg1 && pSeg2 && pPoint );

	minY = min(pSeg1->Y,pSeg2->Y);
	maxY = max(pSeg1->Y,pSeg2->Y);
	minX = min(pSeg1->X,pSeg2->X);
	maxX = max(pSeg1->X,pSeg2->X);

	if ( pPoint->Y < minY || pPoint->Y >= maxY )
		return 0;

	if ( pPoint->X < minX )
		return -1;
	if ( pPoint->X >= maxX )
		return +1;

	{
	jeFloat testX;

	// y = mx + b	(with x and y swapped)

	testX = pSeg1->X + ( pPoint->Y - pSeg1->Y ) * ( pSeg2->X - pSeg1->X ) / ( pSeg2->Y - pSeg1->Y );

	if ( pPoint->X < testX )
		return -1;
	else
		return 1;
	}

}

/* EOF: jeVec2d.c */
