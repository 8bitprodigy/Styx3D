/****************************************************************************************/
/*  TRANS_D4.C                                                                          */
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

#define SHIFT(x)	((x)>>(12))

#define ROOT3	7094
#define ROOT2O2	2896
#define ROOT3O4	1774
#define ROOT3M2O4	(-275)

void d4forward(int * to,int *from,int len)
{
int x,*ptr,*low,*high,half;

	half = len>>1;

	assert( (len & 1) == 0 );
	assert( half >= 2 );

	ptr = from;	high = to + half;
	for(x=0;x<half;x++) 
	{
		*high++ = ptr[1] - SHIFT( ROOT3 * ptr[0] );
		ptr += 2;
	}

	ptr = from; low = to; high = to + half;
	for(x=0;x<half;x++) 
	{
		if ( x == half-1 )
			*low = ptr[0] + SHIFT( ROOT3O4 * high[0] + ROOT3M2O4 * high[0] );
		else
			*low = ptr[0] + SHIFT( ROOT3O4 * high[0] + ROOT3M2O4 * high[1] );

		if ( x == 0 )	*high += low[0] ;
		else			*high += low[-1] ;
		ptr += 2; high++; low++;
	}
}

void d4inverse(int *to,int *from,int len)
{
int x,*ptr,*low,*high,half;

	half = len>>1;

	ptr = to;	low  = from;	high = from + half;

	low  = from;	high = from + half;
	for(x=0;x<half;x++) 
	{
		if ( x == 0 )	*high -= low[0] ;
		else			*high -= low[-1] ;
		high++; low++;
	}

	ptr = to;	low  = from;	high = from + half;
	for(x=0;x<half;x++) 
	{
		if ( x == half-1 )
			ptr[0] = *low - SHIFT( ROOT3O4 * high[0] + ROOT3M2O4 * high[0] );
		else
			ptr[0] = *low - SHIFT( ROOT3O4 * high[0] + ROOT3M2O4 * high[1] );

		ptr += 2; high++; low++;
	}

	ptr = to;	high = from + half;
	for(x=0;x<half;x++) 
	{
		ptr[1] = (*high++) + SHIFT( ROOT3 * ptr[0] );
		ptr += 2;
	}
}

