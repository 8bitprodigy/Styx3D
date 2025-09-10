/****************************************************************************************/
/*  TRANS_SPT.C                                                                         */
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

/**

These are *NOT* the original S+P parameters; we've tweaked them to
apparently better values !

at 20:1 ratio
s,e,w,n : Hare512	iro_a	iro_b	reader	iro_d
4,3,1,4 : 29.17		33.57	33.71	28.02	33.86
4,2,2,4 : 29.19		33.50	33.57	28.02	33.81
4,2,1,4 : 29.24		33.39	33.44	28.08	33.66
4,2,1,2 : 29.27		33.52	33.53	28.24	33.61
4,2,0,2 : 29.34		33.52	33.47	28.26	33.73
4,3,0,4 :					33.63			33.73
cdf22   : 29.32		33.48	32.67	29.13	32.59

note : applying any lifting filter to the low-pass (S) band seems to *HURT* !!
	Attempts to make a smoother low-pass filter have all failed!

**/

#define SHIFT	(4)
#define PE		(3)
#define PW		(1)
#define	PN		(4)

#define PRED(low,high)		(( low[1]*(PE + PW) - low[-1]*PE - low[0]*PW + high[1]*PN )>>SHIFT)

/**
	int E,W,ND,pred;
		E = low[1] - low[-1];
		W = low[1] - low[0];
		ND = high[1];
		pred = ( E + E + W + ND + ND ) >> 3;
**/

void sptforward(int *to,int *from,int len)
{
int x,half;
int *low,*high;

	assert( len >= 2);
	assert( (len&1) == 0 );

	half = len>>1;

	// 'S' (Haar) step :

	low  = to; high = to + half;

	for(x=half;x--;)
	{
	int a,b;
		a = *from++;
		b = *from++;

		*low++  = (a+b)>>1;
		*high++ = a-b;
	}

	// the 'P' (Prediction) step:

	low  = to; high = to + half;

	high[0] += ( low[1] - low[0] + high[1] ) >> (SHIFT-1);

	low++;
	high++;

	for(x=half-2;x--;)
	{
	int pred;
		pred = PRED(low,high);
		high[0] += pred;
		low++;
		high++;
	}

	high[0] += (low[0] - low[-1])>>(SHIFT-2);
}

void sptinverse(int *to,int *from,int len)
{
int x,half;
int *low,*high;

	half = len>>1;

#if 0 // two pass

	// the 'P' (Prediction) step:
	// must go backwards cuz of ND

	low  = from; high = from + half;

	low  += half-1;
	high += half-1;

	high[0] -= (low[0] - low[-1])>>(SHIFT-2);

	low--;
	high--;

	for(x=half-2;x--;)
	{
	int pred;
		pred = PRED(low,high);
		high[0] -= pred;
		low--;
		high--;
	}

	assert(low == from); // at the beginning now
	high[0] -= ( low[1] - low[0] + high[1] ) >> (SHIFT-1);

	// 'S' (Haar) step :

	low  = from; high = from + half;

	for(x=half;x--;)
	{
	int A,D;
		A = *low++; // average
		D = *high++; // delta

		A += ((D+1)>>1);
		*to++ = A;
		*to++ = A - D;
	}

#else // one pass
{
int *ptr,A,D;

	// S+P one pass
	// must go backwards cuz of ND

	low  = from + half - 1;
	high = from + len - 1;
	ptr  = to + len - 1;

	high[0] -= (low[0] - low[-1])>>(SHIFT-2);

	A = *low; D = *high;
	A += ((D+1)>>1);
	ptr[0 ] = A - D;
	ptr[-1] = A;

	low--;
	high--;
	ptr -= 2;

	for(x=half-2;x--;)
	{
	int pred;
		pred = PRED(low,high);
		high[0] -= pred;

		A = *low; D = *high;
		A += ((D+1)>>1);
		ptr[0 ] = A - D;
		ptr[-1] = A;

		low--;
		high--;
		ptr -= 2;
	}

	assert(low == from); // at the beginning now

	A = *low;
	D = high[0] - (( low[1] - A + high[1] ) >> (SHIFT-1));
	A += ((D+1)>>1);
	ptr[0 ] = A - D;
	ptr[-1] = A;

}
#endif

}

void haarforward(int *to,int *from,int len)
{
int x,a,b;
int *low,*high;

	assert( len >= 2);
	assert( (len&1) == 0 );

	x = len>>1;
	low  = to;
	high = to + x;

	if( x&1 )
	{
		a = *from++;
		b = *from++;

		*low++  = (a+b)>>1;
		*high++ = a-b;
	}

	x >>= 1;

	while(x--)
	{
		a = from[0];
		b = from[1];

		low [0] = (a+b)>>1;
		high[0] = a-b;
		
		a = from[2];
		b = from[3];

		low [1] = (a+b)>>1;
		high[1] = a-b;

		from += 4;
		low  += 2;
		high += 2;
	}
}


void haarinverse(int *to,int *from,int len)
{
int x,A,D;
int *low,*high;

	x	 = len>>1;
	low  = from;
	high = from + x;

	if ( x&1 )
	{
		A = *low++; // average
		D = *high++; // delta

		A += ((D+1)>>1);
		*to++ = A;
		*to++ = A - D;
	}

	x >>= 1;

	while(x--)
	{
		A = low [0]; // average
		D = high[0]; // delta

		A += ((D+1)>>1);
		to[0] = A;
		to[1] = A - D;
		
		A = low [1]; // average
		D = high[1]; // delta

		A += ((D+1)>>1);
		to[2] = A;
		to[3] = A - D;

		to   += 4;
		low  += 2;
		high += 2;
	}
}
