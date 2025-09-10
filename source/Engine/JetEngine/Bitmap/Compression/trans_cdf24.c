/****************************************************************************************/
/*  TRANS_CDF24.C                                                                       */
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

void cdf24forward(int * to,int *from,int len)
{
int x,*ptr,*low,*high,half,A;

	half = len>>1;

	assert( (len & 1) == 0 );
	assert( half >= 2 );

	high = to + half;
	ptr = from;
	for(x=0;x<(half-1);x++) 
	{
		*high++ = ptr[1] - ((ptr[2] + ptr[0])>>1);
		ptr += 2;
	}
	*high = ptr[1] - ptr[0];

	low  = to;
	high = to + half;
	ptr = from;
	for(x=0;x<half;x++) 
	{
		if ( x == 0 ) 
			A = ((high[0] )>>1) ;
		else if ( x == 1 ||  x == (half-1)) 
			A = ((high[0] + high[-1])>>2) ;
		else
			A = ((19*(high[0] + high[-1]) - 3*(high[1] + high[-2]))>>6) ;
		*low = *ptr + A;
		ptr += 2; high++; low++;
	}
}

void cdf24inverse(int *to,int *from,int len)
{
int x,*ptr,*low,*high,half,A;

	half = len>>1;

	low  = from;
	ptr = to;
	high = from + half;
	for(x=0;x<half;x++) 
	{
		if ( x == 0 )
			A = ((high[0] )>>1) ;
		else if ( x == 1 ||  x == (half-1))
			A = ((high[0] + high[-1])>>2) ;
		else
			A = ((19*(high[0] + high[-1]) - 3*(high[1] + high[-2]))>>6) ;

		*ptr = *low - A;
		ptr += 2; high++; low ++;
	}

	high = from + half;
	ptr = to;
	for(x=0;x<(half-1);x++)
	{
		ptr[1] = (*high++) + ((ptr[2] + ptr[0])>>1);
		ptr += 2;
	}
	ptr[1] = *high + ptr[0];
}


