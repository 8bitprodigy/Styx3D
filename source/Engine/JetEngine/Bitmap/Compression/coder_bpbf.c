/****************************************************************************************/
/*  CODER_BPBF.C                                                                        */
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

BP Binary Fast

major places to speed up :

fast abs()
rung binary arith

*****/

#include "Utility.h"
#include "arithc.h"
#include "Coder.h"

extern int tune_param;

#define	VAL_CONTEXTS		16
#define	VAL_CONTEXT_MAX		(VAL_CONTEXTS -1)
#define SHAPE_BASE			VAL_CONTEXTS
#define SHAPE(x)			(SHAPE_BASE<<(x))
#define NUM_SHAPES			2
#define NUM_CONTEXTS		(VAL_CONTEXTS<<NUM_SHAPES)

#define P0Init			8
#define P1Init			0

#define SIGN_CONTEXTS	9

jeBoolean coderBPBFInit(coder *c);
void coderBPBFFree(coder *c);
void coderBPBFEncodeBandBP(coderParams *P);
void coderBPBFDecodeBandBP(coderParams *P);

coder coderBPBF = 
{
	"BP Bin Fast",
	coderBPBFInit,
	coderBPBFFree,
	coderBPBFEncodeBandBP,
	coderBPBFDecodeBandBP
};

typedef struct 
{
	int signs_p0[SIGN_CONTEXTS];
	int signs_pt[SIGN_CONTEXTS];
	int stats_p0[NUM_CONTEXTS];
	int stats_pt[NUM_CONTEXTS];
} bpbfInfo;

jeBoolean coderBPBFInit(coder *c)
{
bpbfInfo *d;
int i;

	if ( (d = (bpbfInfo *)new(bpbfInfo)) == NULL )
		return JE_FALSE;

	c->data = d;

	for(i=0;i<NUM_CONTEXTS;i++) 
	{
		d->stats_p0[i] = P0Init+1;
		d->stats_pt[i] = 2+P0Init+P1Init;
	}

	for(i=0;i<SIGN_CONTEXTS;i++) 
	{
		d->signs_p0[i] = 100;
		d->signs_pt[i] = 200;
	}

return JE_TRUE;
}

void coderBPBFFree(coder *c)
{
	if ( c->data ) {
		destroy(c->data);
		c->data = NULL;
	}
}

static int __inline mcontext(int *dp,int pp,int x,int y,int fullw,
					int donemask,int nextmask,int *psign_context,int *pVD);

void coderBPBFEncodeBandBP(coderParams *p)
{
uint32 x,y,bit,width,height,fullw;
uint32 bitshift,bitmask,context;
bpbfInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*parent;
int *dp,*pp;
int *signs_p0,*signs_pt;
int *stats_p0,*stats_pt;
int VD,donemask,nextmask,sign_context;

	c = p->coderPtr;
	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	bpi = (bpbfInfo *)c->data;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;

	stats_p0 = bpi->stats_p0;
	stats_pt = bpi->stats_pt;
	signs_p0 = bpi->signs_p0;
	signs_pt = bpi->signs_pt;

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{	
		for(x=0;x<width;x++) 
		{

			context = mcontext(&dp[x],pp[x>>1],x,y,fullw,
							donemask,nextmask,&sign_context,&VD);

			bit = (abs(dp[x])&bitmask)?1:0;
			bitEnc(bit,ari,stats_p0[context],stats_pt[context]);

			if ( bit && !VD ) 
			{
				bitEnc( signbit(dp[x]) ,ari,signs_p0[sign_context],signs_pt[sign_context]);
			}
		}
		if ( y&1 ) pp += fullw;
		dp += fullw;
	}
}

void coderBPBFDecodeBandBP(coderParams *p)
{
uint32 x,y,bit,width,height,fullw;
uint32 bitshift,context;
int bitmask;
bpbfInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*parent;
int *dp,*pp;
int *signs_p0,*signs_pt;
int *stats_p0,*stats_pt;
int VD,donemask,nextmask,sign_context;

	c = p->coderPtr;
	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	bpi = (bpbfInfo *)c->data;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;

	stats_p0 = bpi->stats_p0;
	stats_pt = bpi->stats_pt;
	signs_p0 = bpi->signs_p0;
	signs_pt = bpi->signs_pt;

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{
		if ( coderStopD(c) )return;
		for(x=0;x<width;x++) 
		{

			context = mcontext(&dp[x],pp[x>>1],x,y,fullw,
							donemask,nextmask,&sign_context,&VD);

			bitDec(bit,ari,stats_p0[context],stats_pt[context]);

			if ( bit ) 
			{
				if ( ! VD ) 
				{
					bitDec(bit,ari,signs_p0[sign_context],signs_pt[sign_context]);
					if ( bit ) dp[x] = - bitmask;
					else dp[x] = bitmask;
				}
				else 
				{
					if ( isneg(dp[x]) ) dp[x] -= bitmask;
					else dp[x] += bitmask; 
				}
			}
		}
		if ( y&1 ) pp += fullw;
		dp += fullw;
	}
}

static int __inline mcontext(int *dp,int pp,int x,int y,int fullw,
					int donemask,int nextmask,int *psign_context,int *pVD)
{
int P,N,W;
int context,VD,sign_context;

	/** <> all these absolute values are painfully slow ***/

	VD	= abs(*dp)&donemask;	// current val already done
	P	= abs( pp)&nextmask;

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

	if ( N > VD ) context += SHAPE(0); 
	if ( W > VD ) context += SHAPE(1);

	*pVD = VD;
	*psign_context = sign_context;

return context;
}
