/****************************************************************************************/
/*  CODER_BPB2.C                                                                        */
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
//@@ STATICS !

// #define BIG_SIGN_CONTEXT , hurts !?
//#define BIG_SHAPE_CNTX

/*****

512, l6, ts
Lena 4.163
Barb 4.496

*****/

#include "Utility.h"
#include "arithc.h"
#include "Coder.h"

#include "ThreadQueue.h"

extern int tune_param;

#define	VAL_CONTEXTS		20
#define	VAL_CONTEXT_MAX		(VAL_CONTEXTS -1)
#define SHAPE_BASE			VAL_CONTEXTS
#define SHAPE(x)			(SHAPE_BASE<<(x))

#ifdef BIG_SHAPE_CNTX
#define NUM_SHAPES			4
#else
#define NUM_SHAPES			2
#endif

#define NUM_CONTEXTS		(VAL_CONTEXTS<<NUM_SHAPES)

#ifdef BIG_SIGN_CONTEXT
#define SIGN_CONTEXTS	729	// 3^6
#else
#define SIGN_CONTEXTS	81	// 3^4
#endif // BIG_SIGN_CONTEXT

#define P0Init			8
#define P1Init			0

#define AddSignContext(context,val,mask) do { context *= 3; if( abs(val)&(mask) ) { if ( (val) > 0 ) context ++; else context += 2; } } while(0)

typedef struct 
{
	int p0,pt;
} binContext;

jeBoolean coderBPB2Init(coder *c);
void coderBPB2Free(coder *c);
void coderBPB2EncodeBandBP(coderParams *P);
void coderBPB2DecodeBandBP(coderParams *P);

coder coderBPB2 = 
{
	"BitPlane Binary 2",
	coderBPB2Init,
	coderBPB2Free,
	coderBPB2EncodeBandBP,
	coderBPB2DecodeBandBP
};

typedef struct 
{
	binContext signs[SIGN_CONTEXTS];
	binContext stats_array[NUM_CONTEXTS];
	jeThreadQueue_Semaphore * lock;
} bpb2Info;

jeBoolean coderBPB2Init(coder *c)
{
bpb2Info *d;
int i;

	if ( ! (d = (bpb2Info *)new(bpb2Info)) )
		return JE_FALSE;

	c->data = d;

	for(i=0;i<NUM_CONTEXTS;i++) 
	{
		d->stats_array[i].p0 = P0Init+1;
		d->stats_array[i].pt = 2+P0Init+P1Init;
	}

	for(i=0;i<SIGN_CONTEXTS;i++) 
	{
		d->signs[i].p0 = 100;
		d->signs[i].pt = 200;
	}

	d->lock = jeThreadQueue_Semaphore_Create();
	assert(d->lock);

return JE_TRUE;
}

void coderBPB2Free(coder *c)
{
	if ( c->data ) 
	{
		bpb2Info *d;
		d = (bpb2Info *)c->data;
		jeThreadQueue_Semaphore_Destroy(&(d->lock));
		destroy(d);
		c->data = NULL;
	}
}

/**********

lazy way to pass the state from getStats to fixStats
	and also interacts with the codeBand()

these are re-initialized at each codeBand() call, so this is
	quite re-entrant as long as we aren't multi-threaded
	(that is, no more than one call to codeBand() at a time)

*********/

static int VD;
static binContext *stats_array;
static binContext *statptr;
static int donemask,nextmask;
static int band_n,*sister_x,*sister_y;

static void getStats(int *dp,int *pp,int x,int y,int width,int height,int fullw)
{
int shapes;
int P,N,W,NE,NW,X1,X2,X3,X4;

	/*** elaborate context-making ***/

	VD	= abs(*dp)&donemask;	// current val already done

	P	= abs(*pp)&nextmask;

	if ( y == 0 ) 
	{
		N = NW = NE = VD;
		if ( x == 0 ) W = VD; else W = abs(dp[-1]) & nextmask;
	}
	else if ( x == 0 ) 
	{
		W = NW = 0;
		N  = abs(dp[-fullw])	& nextmask;
		NE = abs(dp[1-fullw])	& nextmask;
	}
	else 
	{
		N = abs(dp[-fullw])		& nextmask;
		W = abs(dp[-1])			& nextmask;
		NW = abs(dp[-1-fullw])	& nextmask;
		if ( x == (width-1) ) NE = VD;
		else	NE = abs(dp[1-fullw]) & nextmask;
	}

	shapes = 0;
	if ( N > VD ) shapes += SHAPE(0);
	if ( W > VD ) shapes += SHAPE(1);

#ifdef BIG_SHAPE_CNTX
	if ( (abs(dp[fullw])& donemask) > VD ) shapes += SHAPE(2);	// S
	if ( (abs(dp[1])	& donemask) > VD ) shapes += SHAPE(3);	// E
#endif

	/** band 0 has more vertical correlation **/

	switch(band_n) 
	{

		case 0:
		case 1:	// transposed!
#if 1
			if ( y > 1 ) 			X1 = abs(dp[-fullw-fullw]) & nextmask; else X1 = VD;	//NN
			if ( y < (height-1) )	X2 = abs(dp[fullw])		& donemask;	else X2 = VD;			//S
			if ( y > 2 ) 			X3 = abs(pp[-fullw])	& nextmask; else X3 = VD;			//PN
			if ( y < (height-2) )	X4 = abs(pp[fullw]) 	& nextmask; else X4 = VD;			//PS
#else
			if ( x > 1		)		X1 = abs(dp[-2]) & nextmask; else X1 = VD;	//WW
			if ( x < (width-1) )	X2 = abs(dp[fullw]) & donemask; else X2 = VD;	//E
			if ( x > 2 ) 			X3 = abs(pp[-1]) & nextmask; else X3 = VD;	//PW
			if ( x < (width-2) )	X4 = abs(pp[1])  & nextmask; else X4 = VD;	//PE
#endif
			break;

		case 2:
			if ( y < (height-1) )	X1 = abs(dp[fullw])	& donemask;	else X1 = VD;	//S
			if ( x < (width-1) )	X2 = abs(dp[1])		& donemask;	else X2 = VD;	//E
			X3 = abs( sister_x[ x + fullw*y ] ) & nextmask;
			X4 = abs( sister_y[ x + fullw*y ] ) & nextmask;
			break;
		default:
			assert("band_n not in 0-2 !" == NULL);
			break;
	}

	statptr = &stats_array[ min(VAL_CONTEXT_MAX, ((VD + P + N + W + NW + NE + X1 + X2 + X3 + X4)>>2)) + shapes ];
}

static void codeBandInit(bpb2Info *d,int *band,uint width,uint height,uint fullw,uint band_n)
{
	switch(band_n)
	{
		case 0:
		case 1:
			sister_x = sister_y = NULL;
		break;
		case 2:
			sister_x = band - width;
			sister_y = band - height*fullw;
		break;
		default:
			assert(0);
	}

	stats_array = d->stats_array;
}

void coderBPB2EncodeBandBP(coderParams *p)
{
uint32 x,y,bit,width,height,fullw;
uint32 bitshift,bitmask;
bpb2Info * bpi;
binContext *signs;
arithInfo *ari;
coder *c;
int *band,*parent;
int *dp,*pp,*dpn;

	c = p->coderPtr;
	bpi = (bpb2Info *)c->data;

	jeThreadQueue_Semaphore_Lock(bpi->lock);

	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	signs = bpi->signs;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;
	
	codeBandInit(bpi,band,width,height,fullw,p->bandN);

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{
		dpn = dp + fullw;
		for(x=0;x<width;x++) 
		{

			getStats(&dp[x],&pp[x>>1],x,y,width,height,fullw);

			bit = (abs(dp[x])&bitmask)?1:0;
			arithEncBit(ari,statptr->p0,statptr->pt,bit);
			bitModel(bit,statptr->p0,statptr->pt);

			if ( bit & !VD ) 
			{
				int context;
				/** code the sign **/
				context = 0;
				if ( x < (width-1) )	AddSignContext(context,dp[x+1],donemask); else context *= 3;
				if ( x > 0 )			AddSignContext(context,dp[x-1],nextmask); else context *= 3;
				if ( y < (height-1) )	AddSignContext(context,dp[x+fullw],donemask); else context *= 3;
				if ( y > 0 )			AddSignContext(context,dp[x-fullw],nextmask); else context *= 3;
#ifdef BIG_SIGN_CONTEXT
				if ( y > 0 && x > 0 )	AddSignContext(context,dp[x-fullw-1],nextmask); else context *= 3;
				if ( y > 0 && x < (width-1) )	AddSignContext(context,dp[x-fullw+1],nextmask); else context *= 3;
#endif
				bitEnc( (isneg(dp[x])?1:0) ,ari,signs[context].p0,signs[context].pt);
			}
		}
		if ( y&1 ) pp += fullw;
		dp += fullw;
	}
	
	jeThreadQueue_Semaphore_UnLock(bpi->lock);
}

void coderBPB2DecodeBandBP(coderParams *p)
{
uint32 x,y,bit,width,height,fullw;
uint32 bitshift,bitmask;
bpb2Info * bpi;
binContext *signs;
arithInfo *ari;
coder *c;
int *band,*parent;
int *dp,*pp,*dpn;

	c = p->coderPtr;
	bpi = (bpb2Info *)c->data;
	
	jeThreadQueue_Semaphore_Lock(bpi->lock);

	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	signs = bpi->signs;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;
	
	codeBandInit(bpi,band,width,height,fullw,p->bandN);

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{
		if ( coderStopD(c) ) goto done;
		dpn = dp + fullw;
		for(x=0;x<width;x++) 
		{

			getStats(&dp[x],&pp[x>>1],x,y,width,height,fullw);

			bit = arithDecBit(ari,statptr->p0,statptr->pt);
			bitModel(bit,statptr->p0,statptr->pt);

			if ( bit ) 
			{
				if ( isneg(dp[x]) ) dp[x] -= bitmask;
				else dp[x] += bitmask; 
				if ( ! VD ) 
				{
					int context;
					/** code the sign **/
					context = 0;
					if ( x < (width-1) )	AddSignContext(context,dp[x+1],donemask); else context *= 3;
					if ( x > 0 )			AddSignContext(context,dp[x-1],nextmask); else context *= 3;
					if ( y < (height-1) )	AddSignContext(context,dp[x+fullw],donemask); else context *= 3;
					if ( y > 0 )			AddSignContext(context,dp[x-fullw],nextmask); else context *= 3;
#ifdef BIG_SIGN_CONTEXT
					if ( y > 0 && x > 0 )	AddSignContext(context,dp[x-fullw-1],nextmask); else context *= 3;
					if ( y > 0 && x < (width-1) )	AddSignContext(context,dp[x-fullw+1],nextmask); else context *= 3;
#endif
					bitDec(bit,ari,signs[context].p0,signs[context].pt);
					if ( bit ) dp[x] = - dp[x];
				}
			}
		}
		if ( y&1 ) pp += fullw;
		dp += fullw;
	}
	
done:
	jeThreadQueue_Semaphore_UnLock(bpi->lock);
}

