/****************************************************************************************/
/*  TRANS_BCW3.C                                                                        */
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

#include "Utility.h"

#define SHIFT(x)		((x)>>15)

void bcw3forward(int * to,int *from,int len)
{
int x,*ptr,*low,*high,half,A;

	half = len>>1;

	assert( (len & 1) == 0 );
	assert( half >= 2 );

	ptr = from;
	low = to;	high = to + half;
	for(x=0;x<half;x++) 
	{
		if ( x == 0 || x== (half-1) || x== (half-2) )
			A = SHIFT( 19195 * ptr[0] );
		else
			A = SHIFT( ( 18432 * ptr[2] + 4859 * ptr[0] ) - ((ptr[-2] + ptr[4])<<11) );

		*high = ptr[1] - A;
		*low  = ptr[0] + *high;
		*high -= SHIFT( 9598 * (*low) );
		ptr += 2; low++; high++;
	}

	low = to;	high = to + half;
	for(x=0;x<half;x++) 
	{
		if ( x == 0 || x == 1 || x== (half-1) )  
			A = SHIFT( 13573 * high[0] );
		else
			A = SHIFT( ( 27909 * high[0] - 18432 * high[-1] ) + ((high[1] + high[-2])<<11) );

		*low++  -= A;
		high++;
	}
}

void bcw3inverse(int *to,int *from,int len)
{
int x,*ptr,*low,*high,half,A;
	half = len>>1;

	low = from;	high = from + half;
	for(x=0;x<half;x++) 
	{
		if ( x == 0 || x == 1 || x== (half-1) )
			A = SHIFT( 13573 * high[0] );
		else
			A = SHIFT( ( 27909 * high[0] - 18432 * high[-1] ) + ((high[1] + high[-2])<<11) );

		*low++  += A;
		high++;
	}

	ptr = to;
	low = from;	high = from + half;
	for(x=0;x<half;x++) 
	{
		*high += SHIFT( 9598 * (*low) );
		ptr[0] = *low  - *high;
		ptr += 2; low++; high++;
	}

	ptr = to;
	low = from;	high = from + half;
	for(x=0;x<half;x++) 
	{
		if ( x == 0 || x== (half-1) || x== (half-2) )
			A = SHIFT( 19195 * ptr[0] );
		else
			A = SHIFT( ( 18432 * ptr[2] + 4859 * ptr[0] ) - ((ptr[-2] + ptr[4])<<11) );
		ptr[1] = *high + A;
		ptr += 2; low++; high++;
	}
}
