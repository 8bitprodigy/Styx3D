/****************************************************************************************/
/*  TRANS_CDF22.C                                                                       */
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

/*****

note : all attempts to apply a high-pass "predict" filter to cdf22 have failed !

*****/

void cdf22forward(int * to,int *from,int len)
{
int x,*ptr,*low,*high,half;

	assert( len >= 2);
	half = len>>1;

	assert( (len & 1) == 0 );
	assert( half >= 2 );

	low  = to; high = to + half; ptr = from;

	*high = ptr[1] - ((ptr[2] + ptr[0])>>1);
	*low  = ptr[0] + ((high[0] )>>1);
	ptr += 2; high++; low++;

	for(x=(half-2);x--;) 
	{
		*high = ptr[1] - ((ptr[2] + ptr[0])>>1);
		*low =  ptr[0] + ((high[0] + high[-1])>>2);
		ptr += 2; high++; low++;
	}

	*high = ptr[1] - ptr[0];
	*low = *ptr + ((high[0] + high[-1])>>2);

}

void cdf22inverse(int *to,int *from,int len)
{
int x,*ptr,*low,*high;

	assert( len >= 2);
	assert( (len&1) == 0 );
	x = len>>1;
	low  = from;
	high = from + x;
	ptr = to;
	
	ptr[0] = low[0] - ((high[0])>>1);
	ptr += 2; high++; low++;
	x--; // count this startup one
	if ( x&1 )
	{
		ptr[ 0] = low[0]  - ((high[0] + high[-1])>>2);
		ptr[-1] = high[-1] + ((ptr[0] + ptr[-2])>>1);
		ptr += 2; high++; low++;
	}
	x >>= 1;
	while ( x-- )
	{
		ptr[ 0] = low[0]  - ((high[0] + high[-1])>>2);
		ptr[-1] = high[-1] + ((ptr[0] + ptr[-2])>>1);
		ptr += 2; high++; low++;
		ptr[ 0] = low[0]  - ((high[0] + high[-1])>>2);
		ptr[-1] = high[-1] + ((ptr[0] + ptr[-2])>>1);
		ptr += 2; high++; low++;
	}
	ptr[-1] = high[-1] + ptr[-2];

}

