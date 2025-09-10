/****************************************************************************************/
/*  CODER_BPBFRL.C                                                                      */
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

BPBFRL

BP Binary Fast - Rung AE - RunLength

all the significant runs are in the high BP's in the high levels

we are almost as good as coder_bpbf when we do LEVELS_THEN_BITS and
	when Ratio >= 10
if either of those is not true, we just eat ass, because we end up
	coding lots of short runlength without using the context info.

todo :

	1. try only coding runlength once you see like 4 zeros in a row!

*****/

#define DO_REPORT

#include "Utility.h"
#include "arithc.h"
#include "Coder.h"
#include "CodeUtil.h"
#include "rungae.h"
#include "IntMath.h"
#include "Report.h"

extern int tune_param;

REPORT_VARS(RunLen);

#define RLRUNG_UNARYBITS	(0)
#define RLRUNG_BITS			(1)
#define RLRUNG_ZERO			(0)

#define	VAL_CONTEXTS		(16)
#define	VAL_CONTEXT_MAX		(VAL_CONTEXTS -1)
#define SHAPE_BASE			VAL_CONTEXTS
#define SHAPE(x)			(SHAPE_BASE<<(x))
#define NUM_SHAPES			(2)
#define NUM_CONTEXTS		(VAL_CONTEXTS<<NUM_SHAPES)

#define SIGN_CONTEXTS	9

jeBoolean coderBPBFRLInit(coder *c);
void coderBPBFRLFree(coder *c);
void coderBPBFRLEncodeBandBP(coderParams *P);
void coderBPBFRLDecodeBandBP(coderParams *P);

coder coderBPBFRL = 
{
	"BP Bin Fast Rung RunLength",
	coderBPBFRLInit,
	coderBPBFRLFree,
	coderBPBFRLEncodeBandBP,
	coderBPBFRLDecodeBandBP
};

typedef struct 
{
	rung_t signs[SIGN_CONTEXTS];
	rung_t stats[NUM_CONTEXTS];
	rung_t runstats[4];
} BPBFRLInfo;

jeBoolean coderBPBFRLInit(coder *c)
{
BPBFRLInfo *d;
int i;

	if ( (d = (BPBFRLInfo *)new(BPBFRLInfo)) == NULL )
		return JE_FALSE;

	c->data = d;

	for(i=0;i<NUM_CONTEXTS;i++) 
	{
		rungModelInit(&(d->stats[i]));
	}

	for(i=0;i<4;i++) 
	{
		rungModelInit(&(d->runstats[i]));
	}

	for(i=0;i<SIGN_CONTEXTS;i++) 
	{
		rungModelInit(&(d->signs[i]));
	}

	REPORT_RESET(RunLen);

return JE_TRUE;
}

void coderBPBFRLFree(coder *c)
{

	REPORT_REPORT(RunLen);

	if ( c->data ) {
		destroy(c->data);
		c->data = NULL;
	}
}

static int __inline mcontext(	int *dp,int pp,int x,int y,
								int fullw,int donemask,int nextmask,
								int *psign_context,int *pVD,int *pAbs,int *pNeg);

void putRL(arithInfo *ari,rung_t * rungs,int len)
{
int bits,msb;
	assert( len >= 0 );

	if ( len == 0 )
	{
		rungModelEncBit(ari,1,rungs+RLRUNG_ZERO);
		return;
	}
	rungModelEncBit(ari,0,rungs+RLRUNG_ZERO);

#ifdef DO_REPORT
{
int RunLen = len;
	REPORT_ADD(RunLen);
}
#endif

	bits = intlog2(len);
	msb = (1<<bits);
	len -= msb;

	//unary-code the # of bits	
	while(bits)
	{
		rungModelEncBit(ari,0,rungs + RLRUNG_UNARYBITS);
		bits--;
	}
	rungModelEncBit(ari,1,rungs + RLRUNG_UNARYBITS);

	/** sym < msb now **/
	for(msb = msb>>1;msb>=1;msb>>=1) 
	{
		rungModelEncBit(ari,len&msb,rungs + RLRUNG_BITS);
	}
}

int getRL(arithInfo *ari,rung_t * rungs)
{
int msb,len,bits;

	if ( rungModelDecBit(ari,rungs+ RLRUNG_ZERO) )
		return 0;

	bits = 0;
	while( ! rungModelDecBit(ari,rungs + RLRUNG_UNARYBITS) )
		bits++;

	msb = 1<<bits;
	len = msb;
	for(msb = msb>>1;msb>=1;msb>>=1)
	{
		if ( rungModelDecBit(ari,rungs + RLRUNG_BITS) )
			len += msb;
	}
	
#ifdef DO_REPORT
{
int RunLen = len;
	REPORT_ADD(RunLen);
}
#endif

return len;
}

void coderBPBFRLEncodeBandBP(coderParams *p)
{
int x,y,bit,width,height,fullw;
int bitshift,bitmask,context,absval,negval;
BPBFRLInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*parent;
int *dp,*pp;
rung_t *signs_r,*stats_r,*rlstats;
int VD,donemask,nextmask,sign_context;
int runlen;

	c = p->coderPtr;
	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	bpi = (BPBFRLInfo *)c->data;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;

	stats_r = bpi->stats;
	rlstats = bpi->runstats;
	signs_r = bpi->signs;

	runlen = 0;

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{	
		for(x=0;x<width;x++) 
		{

			context = mcontext(dp+x,pp[x>>1],x,y,fullw,
							donemask,nextmask,&sign_context,&VD,&absval,&negval);

			bit = (absval>>bitshift)&1;

			if ( runlen )
			{
				if ( bit == 0 )
				{
					runlen++;
					continue;
				}
				else
				{
					putRL(ari,rlstats,runlen-1);
					runlen = 0;
				}
			}

			rungModelEncBit(ari,bit,stats_r + context);

			if ( ! bit )
				runlen = 1;

			if ( bit && !VD ) 
			{
				rungModelEncBit(ari,negval,signs_r + sign_context);
			}
		}
		if ( y&1 ) pp += fullw;
		dp += fullw;
	}
	if ( runlen )
		putRL(ari,rlstats,runlen-1);
}

void coderBPBFRLDecodeBandBP(coderParams *p)
{
int x,y,width,height,fullw;
int bitshift,context;
int bitmask,absval,negval;
BPBFRLInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*parent;
int *dp,*pp;
rung_t *signs_r,*stats_r,*rlstats;
int VD,donemask,nextmask,sign_context;
int runlen;

	c = p->coderPtr;
	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	bpi = (BPBFRLInfo *)c->data;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;

	stats_r = bpi->stats;
	signs_r = bpi->signs;
	rlstats = bpi->runstats;
	runlen = 0;

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{
		if ( coderStopD(c) )return;
		for(x=0;x<width;x++,dp++) 
		{
			if ( runlen )
			{
				runlen--;
				continue;
			}

			context = mcontext(dp,pp[x>>1],x,y,fullw,
							donemask,nextmask,&sign_context,&VD,&absval,&negval);

			if ( ! rungModelDecBit(ari,stats_r + context) ) 
			{
				runlen = getRL(ari,rlstats);
				continue;
			}

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
