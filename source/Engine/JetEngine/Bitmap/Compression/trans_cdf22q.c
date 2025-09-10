/****************************************************************************************/
/*  TRANS_CDF22Q.C                                                                      */
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

/***

note that quantizing in the transform has the funny effect that
the HH band gets quantized TWICE !  So, with my quantizer=2,
the HH gets quantized by four !

--------

there's an anomaly here, in that negatives round *up* when shifted :

	(-1)>>2 == -1
	(-5)>>2 == -2
etc..

therefore must use divide to quantize; can use shifts to dequantize

***/

#if 0 //{ // test code

#define QUANT(x)	((x)/4)
#define DEQ(x)		((x)*4)

/**
iro_a -> 4096 , coder_f3
psnr by shift:
	26.2
	28.0
	25.7
iro_a -> 4096 , coder_bp
psnr by shift:
	28.75
	28.2
	25.7	(using divide by 4 : 26.7 )
			(using divide by 3 : 29.0 )
			(using divide by 2 : 28.8 )
			(using divide by 1 : 28.75 )
**/

void cdf22qforward(int * to,int *from,int len)
{
int x,*ptr,*low,*high,half;

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

	high = to + half;
	for(x=half;x--;)
	{
		*high++ = QUANT(*high);
	}
}

void cdf22qinverse(int *to,int *from,int len)
{
int x,*ptr,*low,*high,half;

	half = len>>1;
	low  = from; high = from + half; ptr = to;
	
	ptr[0] = low[0] - (DEQ(high[0])>>1);
	ptr += 2; high++; low++;
	for(x=(half-1);x--;) 
	{
		ptr[ 0] = low[0]  - ((DEQ(high[0] + high[-1]))>>2);
		ptr[-1] = DEQ(high[-1]) + ((ptr[0] + ptr[-2])>>1);
		ptr += 2; high++; low++;
	}
	ptr[-1] = DEQ(high[-1]) + ptr[-2];
}

#else // }{ // with Quantizer = 2

void cdf22qforward(int * to,int *from,int len)
{
int x,*ptr,*low,*high,half;

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

	high = to + half;
	for(x=half;x--;)
	{
		*high++ = *high/2;	// can't shift cuz of sign handling!
	}
}

void cdf22qinverse(int *to,int *from,int len)
{
int x,*ptr,*low,*high,half;

	half = len>>1;
	low  = from; high = from + half; ptr = to;
	
	ptr[0] = low[0] - high[0];
	ptr += 2; high++; low++;
	for(x=(half-1);x--;) 
	{
		// <> this inner loop actually has one *more* shift than the plain cdf22 !
		ptr[ 0] = low[0]  - ((high[0] + high[-1])>>1);
		ptr[-1] = (high[-1]<<1) + ((ptr[0] + ptr[-2])>>1);
		ptr += 2; high++; low++;
	}
	ptr[-1] = (high[-1]<<1) + ptr[-2];
}

#endif //}