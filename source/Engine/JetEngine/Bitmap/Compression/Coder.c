/****************************************************************************************/
/*  CODER.C                                                                             */
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
//#define STUFF

#include <string.h>

#include "arithc.h"
#include "Coder.h"
#include "CodeUtil.h"
#include "IntMath.h"
#include "rungae.h"

int tune_param = 0;

uint32 asmFunc_RungModelDecBit = (uint32) &rungModelDecBit;
uint32 asmFunc_ArithDecBit = (uint32) &arithDecBitRaw;
uint32 asmFunc_ArithTellDecPos = (uint32) &arithTellDecPos;

extern coder coderBP,coderBPBF,coderBPB2,coderFast,coderFast2,coderFast3,coderBPBFR,coderBPBFRL;
const int num_coders = 8;
const coder * coder_list[] = 
{
	&coderBP, 
	&coderBPBF,
	&coderBPB2,
	&coderFast,
	&coderFast2,
	&coderFast3,
	&coderBPBFR,
	&coderBPBFRL,
	NULL
};

coder * coderCreateWrite(int coderN,uint8 * compArray)
{
coder *c;
const coder *codorTemplate;

	if ( (c = (coder *)new(coder)) == NULL ) return NULL;

	if ( coderN >= num_coders ) coderN = 0;

	codorTemplate = coder_list[coderN];
	if ( codorTemplate ) memcpy(c,codorTemplate,sizeof(coder));
 
	if ( (c->arith = arithInit()) == NULL )
	{
		coderDestroy(c); return NULL;
	}
	arithEncodeInit(c->arith,compArray);

	c->init(c);

return c;
}

coder * coderCreateReadNoInit(int coderN,int stopLen)
{
coder *ret;
const coder *codorTemplate;

	if ( (ret = (coder *)new(coder)) == NULL ) return NULL;

	codorTemplate = coder_list[coderN];
	if ( codorTemplate ) memcpy(ret,codorTemplate,sizeof(coder));

	ret->wStopLen = stopLen;
	
	if ( (ret->arith = arithInit()) == NULL )
	{
		coderDestroy(ret); return NULL;
	}

return ret;
}

coder * coderCreateRead(int coderN,uint8 * compArray,int stopLen)
{
coder * c;
	c = coderCreateReadNoInit(coderN,stopLen);
	if ( !c ) return NULL;
	arithDecodeInit(c->arith,compArray);
	c->init(c);
return c;
}

void coderFlushRead(coder *c)
{
	arithDecodeDone(c->arith);
}

int coderFlushWrite(coder *c)
{
return arithEncodeDone(c->arith);
}

void coderDestroy(coder *c)
{
	if ( c ) 
	{
		if ( c->arith )
		{
			if ( c->free )
				c->free(c);
			arithFree(c->arith);
		}
		destroy(c);
	}
}

/************* routines for DPCM coding the top plane 

as of v1.5 this coder is quite bad-ass , even beats things like CALIC on your teeny
images (because context coders take too long to rev up) :

we do something cheezy like all the standard image coders.  Since the LL band
is so small, we don't have time to adapt a context coder.  Also, the different LL
bands of images have widely varying statistics, so we can't just use the plain
order -1 coder.

instead we try to make a local estimate of the mean prediction error (MPE).  We then
code using a static coder :

	0  		+ (1 < MPE)
	01 		+ (MPE < 3*MPE )
	001 	+ (3*MPE < 7*MPE )
	0001	+ (7*MPE < 15 * MPE)
	...

it would make more sense to use a Guassian probability distribution
sent to the arithcoder, with MPE as the standard dev, but the cumprobs
for a gaussian are the error function.  you would think we could
handle that by tabulation, but there are intricacies with coding
(sym/mpe) as a real number vs. an integer. (eg. hard to define the
"next" and "prev" symbols to get the neighboring cumprobs)

------

our sign coder is even cheezier.  We build the N+W sign neighbors context.
if N == W we have confidence, and use a binary adaptive coder on (sign == neighbor)
else we just send sign raw

***************/

#define EST_ERROR_SHIFT			3
#define EST_ERROR_PAD			0
#define EST_ERROR(grad,prev)	(((grad + grad + prev)>>EST_ERROR_SHIFT) + 2 + EST_ERROR_PAD)
#define ERROR_CODER_MULT		2		// tunes to 1 !!! almost an 0.2 b gain (on the LL)

static int signs_p0,signs_pt;

void coderEncodesign(arithInfo *ari,jeBoolean sign,jeBoolean W,jeBoolean N)
{
	if ( N == W ) 
	{
		if ( ! N ) sign = !sign;
		bitEnc(sign,ari,signs_p0,signs_pt);
	}
	else 
	{
		arithEncBitRaw(ari,sign);
	}
}
jeBoolean coderDecodesign(arithInfo *ari,jeBoolean W,jeBoolean N)
{
	if ( N == W ) 
	{
		jeBoolean sign;
		bitDec(sign,ari,signs_p0,signs_pt);
		return sign ? N : !N;
	}
	else 
	{
		return arithDecBitRaw(ari);
	}
}

void coderEncodeDPCM(coder *c,int *plane,int width,int height,int rowpad)
{
int x,y;
int pred,grad,val,sign;
int prev_err,est,fullw;
arithInfo * ari = c->arith;
int *ptr,*pline;

#ifdef STUFF
	arithEncode(c->arith,77,78,373);
#endif

	fullw = width + rowpad;
	bitModelInit(signs_p0,signs_pt);
	ptr = plane; prev_err = 99;
	for(y=0;y<height;y++) 
	{
		for(x=0;x<width;x++) 
		{
		
		#ifdef STUFF
			arithEncode(c->arith,77,78,373);
		#endif

			pline = ptr - fullw;

			if ( y == 0 ) { 
				if ( x == 0 ) { pred = 0; grad = 99; }
				else if ( x == 1 ) { pred = ptr[-1]; grad = 99; }
				else { pred = (ptr[-1] + ptr[-2])>>1; grad = abs(ptr[-1] - ptr[-2]); }
			} else if ( x == 0 ) { pred = (pline[0] + pline[1])>>1; grad = abs(pline[0] - pline[1]); 
			} else if ( x == width-1 ) {
				pred = (ptr[-1] + pline[0])>>1;
				grad = abs(ptr[-1] - pline[0]);
			} else {
				pred = (ptr[-1]*3 + pline[0]*3 + ptr[-1] + pline[1])>>3;
				grad = max( abs(ptr[-1] - ptr[-1]) , abs( pline[0] - pline[1]) );
			}

			val = (*ptr) - pred;

			if ( val < 0 ) { sign = 1; val = -val; }
			else sign = 0;
	
			est = EST_ERROR(grad,prev_err);
			assert( est < 16384 && est > 0 );
			cu_putMulting_ari(val,ari,est,ERROR_CODER_MULT);

			if ( val > 0 ) 
			{
				if ( x == 0 || x == 1 ) 
				{
					arithEncBitRaw(ari,sign);
				}
				else if ( y == 0 ) 
				{
					coderEncodesign(ari,sign,isneg(ptr[-1]),isneg(ptr[-2]));
				}
				else 
				{
					coderEncodesign(ari,sign,isneg(ptr[-1]),isneg(pline[0]));
				}
			}

			ptr++;
			prev_err = val;
		}
		ptr += rowpad;
	}
	
#ifdef STUFF
	arithEncode(c->arith,77,78,373);
#endif
}

void coderDecodeDPCM(coder *c,int *plane,int width,int height,int rowpad)
{
int x,y,prev_err,est,fullw;
int pred,grad,val;
arithInfo * ari = c->arith;
int *ptr,*pline;

#ifdef STUFF
	{
	uint32 got;
		got = arithGet(c->arith,373);
		assert( got == 77 );
		if ( got != 77 )
			return;
		arithDecode(c->arith,77,78,373);
	}
#endif

	fullw = width + rowpad;
	bitModelInit(signs_p0,signs_pt);
	ptr = plane; prev_err = 99;
	for(y=0;y<height;y++) 
	{
		for(x=0;x<width;x++) 
		{
			#ifdef STUFF
				{
				uint32 got;
					got = arithGet(c->arith,373);
					assert( got == 77 );
					if ( got != 77 )
						return;
					arithDecode(c->arith,77,78,373);
				}
			#endif

			pline = ptr - fullw;

			if ( y == 0 ) { 
				if ( x == 0 ) { pred = 0; grad = 99; }
				else if ( x == 1 ) { pred = ptr[-1]; grad = 99; }
				else { pred = (ptr[-1] + ptr[-2])>>1; grad = abs(ptr[-1] - ptr[-2]); }
			} else if ( x == 0 ) { pred = (pline[0] + pline[1])>>1; grad = abs(pline[0] - pline[1]); 
			} else if ( x == width-1 ) {
				pred = (ptr[-1] + pline[0])>>1;
				grad = abs(ptr[-1] - pline[0]);
			} else {
				pred = (ptr[-1]*3 + pline[0]*3 + ptr[-1] + pline[1])>>3;
				grad = max( abs(ptr[-1] - ptr[-1]) , abs( pline[0] - pline[1]) );
			}

			est = EST_ERROR(grad,prev_err);
			assert( est < 16384 && est > 0 );
			val = cu_getMulting_ari(ari,est,ERROR_CODER_MULT);

			prev_err = val;

			if ( val > 0 ) 
			{
				if ( x == 0 || x == 1 ) 
				{
					if ( arithDecBitRaw(ari) ) val = -val;
				}
				else if ( y == 0 ) 
				{
					if ( coderDecodesign(ari,isneg(ptr[-1]),isneg(ptr[-2])) )
						val = -val;
				}
				else 
				{
					if ( coderDecodesign(ari,isneg(ptr[-1]),isneg(pline[0])) )
						val = -val;
				}
			}

			*ptr++ = val + pred;
		}
		ptr += rowpad;
	}
	
#ifdef STUFF
	{
	uint32 got;
		got = arithGet(c->arith,373);
		assert( got == 77 );
		if ( got != 77 )
			return;
		arithDecode(c->arith,77,78,373);
	}
#endif
}


void coderCodeDPCM1d(coder *c,int *plane,int width,jeBoolean Decode)
{
int x;
int pred,grad,val,sign;
int prev_err,est,estMax;
arithInfo * ari;
int *ptr;
 
	ari = c->arith;
	bitModelInit(signs_p0,signs_pt);
	ptr = plane; prev_err = 99;

	estMax = ari->probMaxSafe;

	for(x=0;x<width;x++) 
	{

	/*{*****/
	/* lots of fancy predictors */
	/* don't do a damned bit of good */
	#if 0 // linear best fit!
		if ( x >= 3 )
		{
		int m,b,a,y0,y1,y2;
			y0 = ptr[-3];
			y1 = ptr[-2];
			y2 = ptr[-1];
			a = (y0 + y1 + y2 + 2)/3;
			m = (a - y0 + 1)/3;
			b = a - m;
			pred = 3*m + b;
			y0 = y0 - b;
			y1 = y1 - (m+b);
			y2 = y2 - (m+m+b);
			grad = y0*y0 + y1*y1 + y2*y2;
			grad = isqrt(grad);
			assert(grad >= 0);
		}
		else
	#endif
	#if 0
		if ( x >= 3 )
		{
		int y0,y1,y2;
			y2 = ptr[-3];
			y1 = ptr[-2];
			y0 = ptr[-1];
			pred = y0 + ((y0 - y1)>>1) + ((y1 - y2)>>2);
			grad = y1 - ((y0 + y2)>>1);
			grad = abs(grad);
		}
		else
	#endif
	#if 0
		if ( x >= 3 )
		{
		int y0,y1,y2;
			y2 = ptr[-3];
			y1 = ptr[-2];
			y0 = ptr[-1];
			pred = y0 + ((y0 - y1)>>1);
				// = 3/2 * y0 - y1 / 2
				// = (y0 + y1)/2 + (y0 - y1)	
				// same as taking the average and adding current slope
			grad = y1 - ((y0 + y2)>>1);
			grad = abs(grad);
			grad <<= 1;
		}
		else
	#endif
	#if 0
		if ( x >= 2 )
		{
		int y0,y1;
			y1 = ptr[-2];
			y0 = ptr[-1];
			grad = y0 - y1;
			pred = y0 + (grad>>1);
				// = 3/2 * y0 - y1 / 2
				// = (y0 + y1)/2 + (y0 - y1)	
				// same as taking the average and adding current slope
			grad = abs(grad);
		}
		else
	#endif
	/*}*****/

		if ( x == 0 ) { pred = 0; grad = 99; }
		else if ( x == 1 ) { pred = ptr[-1]; grad = 99; }
		else
		{
			pred = (ptr[-1] + ptr[-2])>>1;
			grad = abs(ptr[-1] - ptr[-2]);
		}

		est = EST_ERROR(grad,prev_err);
		if ( est >= estMax )
			est = estMax;
			
		if ( Decode )
		{
			val = cu_getMulting_ari(ari,est,ERROR_CODER_MULT);

			prev_err = val;

			if ( val > 0 ) 
			{
				if ( x == 0 || x == 1 ) 
				{
					if ( arithDecBitRaw(ari) ) val = -val;
				}
				else
				{
					if ( coderDecodesign(ari,isneg(ptr[-1]),isneg(ptr[-2])) )
						val = -val;
				}
			}

			*ptr = val + pred;
		}
		else
		{
			val = (*ptr) - pred;

			if ( val < 0 ) { sign = 1; val = -val; }
			else sign = 0;

			cu_putMulting_ari(val,ari,est,ERROR_CODER_MULT);

			if ( val > 0 ) 
			{
				if ( x == 0 || x == 1 ) 
				{
					arithEncBitRaw(ari,sign);
				}
				else
				{
					coderEncodesign(ari,sign,isneg(ptr[-1]),isneg(ptr[-2]));
				}
			}

			prev_err = val;
		}
		ptr++;
	}
}
