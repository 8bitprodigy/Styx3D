/****************************************************************************************/
/*  CODER_BPBFR.C                                                                       */
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

/*****

BP Binary Fast - Rung AE

the Rung coder is actually BETTER COMPRESSION than the plain binary arithcoder !

major places to speed up :

fast abs()

*****/

#include "Utility.h"
#include "arithc.h"
#include "Coder.h"
#include "rungae.h"

extern int tune_param;

#define	VAL_CONTEXTS		(16)
#define	VAL_CONTEXT_MAX		(VAL_CONTEXTS -1)
#define SHAPE_BASE			VAL_CONTEXTS
#define SHAPE(x)			(SHAPE_BASE<<(x))
#define NUM_SHAPES			(2)
#define NUM_CONTEXTS		(VAL_CONTEXTS<<NUM_SHAPES)

#define SIGN_CONTEXTS	9

jeBoolean coderBPBFRInit(coder *c);
void coderBPBFRFree(coder *c);
void coderBPBFREncodeBandBP(coderParams *P);
void coderBPBFRDecodeBandBP(coderParams *P);

coder coderBPBFR = 
{
	"BP Bin Fast Rung",
	coderBPBFRInit,
	coderBPBFRFree,
	coderBPBFREncodeBandBP,
	coderBPBFRDecodeBandBP
};

typedef struct 
{
	rung_t signs[SIGN_CONTEXTS];
	rung_t stats[NUM_CONTEXTS];
} BPBFRInfo;

jeBoolean coderBPBFRInit(coder *c)
{
BPBFRInfo *d;
int i;

	if ( (d = (BPBFRInfo *)new(BPBFRInfo)) == NULL )
		return JE_FALSE;

	c->data = d;

	for(i=0;i<NUM_CONTEXTS;i++) 
	{
		rungModelInit(&(d->stats[i]));
	}

	for(i=0;i<SIGN_CONTEXTS;i++) 
	{
		rungModelInit(&(d->signs[i]));
	}

return JE_TRUE;
}

void coderBPBFRFree(coder *c)
{
	if ( c->data ) {
		destroy(c->data);
		c->data = NULL;
	}
}

static int __inline mcontext(	int *dp,int pp,int x,int y,
								int fullw,int donemask,int nextmask,
								int *psign_context,int *pVD,int *pAbs,int *pNeg);

void coderBPBFREncodeBandBP(coderParams *p)
{
int x,y,bit,width,height,fullw;
int bitshift,bitmask,context,absval,negval;
BPBFRInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*parent;
int *dp,*pp;
rung_t *signs_r,*stats_r;
int VD,donemask,nextmask,sign_context;

	c = p->coderPtr;
	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	bpi = (BPBFRInfo *)c->data;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;

	stats_r = bpi->stats;
	signs_r = bpi->signs;

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{	
		for(x=0;x<width;x++) 
		{

			context = mcontext(dp+x,pp[x>>1],x,y,fullw,
							donemask,nextmask,&sign_context,&VD,&absval,&negval);

			bit = (absval>>bitshift)&1;
			rungModelEncBit(ari,bit,stats_r + context);

			if ( bit && !VD ) 
			{
				rungModelEncBit(ari,negval,signs_r + sign_context);
			}
		}
		if ( y&1 ) pp += fullw;
		dp += fullw;
	}
}

void coderBPBFRDecodeBandBP(coderParams *p)
{
int x,y,width,height,fullw;
int bitshift,context;
int bitmask,absval,negval;
BPBFRInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*parent;
int *dp,*pp;
rung_t *signs_r,*stats_r;
int VD,donemask,nextmask,sign_context;

	c = p->coderPtr;
	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	bpi = (BPBFRInfo *)c->data;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;

	stats_r = bpi->stats;
	signs_r = bpi->signs;

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{
		if ( coderStopD(c) )return;
		for(x=0;x<width;x++,dp++) 
		{

			context = mcontext(dp,pp[x>>1],x,y,fullw,
							donemask,nextmask,&sign_context,&VD,&absval,&negval);

			if ( ! rungModelDecBit(ari,stats_r + context) ) 
				continue;

			if ( ! VD ) 
				negval = rungModelDecBit(ari,signs_r + sign_context);

			assert( (negval&1) == negval );

			{
			int mask,val;
				val = *dp;
				mask = bitmask;

				#if 0

				// cmov !
				if ( negval ) mask = - bitmask;

				#else 

				#if 0

				// neg
				// xor
				// and
				// add
				negval = - negval;
				mask = (mask ^ negval) + (negval&1);

				#else

				#if 0

				// add
				// neg
				// add
				// mul

				negval = 1 - (negval + negval);
				mask *= negval;

				#else

				// shl
				// mul
				// sub
				mask -= (negval<<1) * mask;

				#endif

				#endif

				#endif

				val += mask;
				
				*dp = val;
			}
		}
		if ( y&1 ) pp += fullw;
		dp += fullw - width;
	}
}

static int __inline mcontext(int *dp,int pp,int x,int y,int fullw,int donemask,int nextmask,
					int *psign_context,int *pVD,int *pAbs,int *pNeg)
{
int P,N,W;
int context,VD,sign_context;

	*pAbs = abs(*dp);
	*pNeg = isneg(*dp);	

	VD	= (*pAbs)&donemask;	// current val already done
	*pVD = VD;

	P	= abs(pp)&nextmask;

	sign_context = 0;

	if ( y == 0 ) 
	{
		N = VD;
		if ( x == 0 ) W = VD; else W = abs(dp[-1]) & nextmask;
		if ( W ) 
		{
			if ( isneg(dp[-1]) ) sign_context += 3;
			else sign_context += 6;
		}
	}
	else if ( x == 0 ) 
	{
		W = VD;
		N  = abs(dp[-fullw])	& nextmask;
		if ( N ) 
		{
			if ( isneg(dp[-fullw]) ) sign_context += 1;
			else sign_context += 2;
		}
	} 
	else 
	{
		N = abs(dp[-fullw])		& nextmask;
		W = abs(dp[-1])			& nextmask;
		if ( N ) 
		{
			if ( isneg(dp[-fullw]) ) sign_context += 1;
			else sign_context += 2;
		}
		if ( W ) 
		{
			if ( isneg(dp[-1]) ) sign_context += 3;
			else sign_context += 6;
		}
	}

	context = min(VAL_CONTEXT_MAX, ((VD + P + N + W)>>2));

	context += ( N > VD ) ? SHAPE(0) : 0; 
	context += ( W > VD ) ? SHAPE(1) : 0;

	*psign_context = sign_context;

return context;
}
