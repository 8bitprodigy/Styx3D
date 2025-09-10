/****************************************************************************************/
/*  CODER_BP.C                                                                          */
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

coder_BP : the archetype, the prototype, the progenitor

--------

it's hard to avoid the abs() here.
why ?  if we try to send the signs on the side (so that this data is unsigned)
then when do we send the signs?  That is, we want to truncate our stream, and
we don't want to send signs for zero-valued coefficients, so.. ?

*****/

#include "Utility.h"
#include "arithc.h"
#include "soz.h"
#include "Coder.h"

static int bp_remap[] = {0,1,2,5,3,9,6,11,4,10,7,12,8,13,14,15};
static int bp_unmap[] = {0,1,2,4,8,3,6,10,12,5,9,7,11,13,14,15};

#define arithBit(ari,bit)	arithEncBitRaw(ari,bit)
#define arithGetBit(ari)	arithDecBitRaw(ari)

#define ORDER1_CONTEXTS		10
#define ORDER1_ALPHABET		16	//4 bits

#define ORDER1_TOTMAX		15000
#define ORDER1_INC			30

jeBoolean coderBPInit(coder *c);
void coderBPFree(coder *c);
void coderBP_flush(coder *c);
void coderBPEncodeBandBP(coderParams *p);
void coderBPDecodeBandBP(coderParams *p);

coder coderBP = 
{
	"BitPlane",
	coderBPInit,
	coderBPFree,
	coderBPEncodeBandBP,
	coderBPDecodeBandBP,
	coderBP_flush
	// the rest gets NULLs
};

typedef struct 
{
	soz ** o1;
} bpInfo;

jeBoolean coderBPInit(coder *c)
{
bpInfo *d;
int i;

	if ( !(d = (bpInfo *)new(bpInfo)) )
		return JE_FALSE;

	c->data = d;

	if ( (d->o1 = (soz **)newarray(void *,ORDER1_CONTEXTS)) == NULL )
	{
		coderBPFree(c);
		return JE_FALSE;
	}

	for(i=0;i<ORDER1_CONTEXTS;i++)
	{
		if ( (d->o1[i] = sozCreate(c->arith,ORDER1_ALPHABET,ORDER1_TOTMAX,ORDER1_INC)) == NULL )
		{
			coderBPFree(c);
			return JE_FALSE;
		}
	}

return JE_TRUE;
}

void coderBPFree(coder *c)
{
	if ( c->data ) 
	{
		bpInfo *d;
		d = (bpInfo *)c->data;
		if ( d->o1 ) 
		{
			int i;
			for(i=0;i<ORDER1_CONTEXTS;i++) 
			{
				if ( d->o1[i] ) sozFree(d->o1[i]);
			}
			destroy(d->o1);
		}
		destroy(d);
		c->data = NULL;
	}
}

void coderBP_flush(coder *c)
{
bpInfo *d;
int i;
	d = (bpInfo *)c->data;
	for(i=0;i<ORDER1_CONTEXTS;i++) 
	{
		sozReset(d->o1[i]);
	}
}

void coderBPEncodeBandBP(coderParams *p)
{
uint32 x,y;
uint32 context,block,par,A,B,C,D;
uint32 donemask,nextmask,bitshift,bitmask;
uint32 width,height,fullw;
bpInfo * bpi;
soz **o1;
arithInfo *ari;
coder *c;
int *band,*parent;
int *dp,*pp,*dpn;

	c = p->coderPtr;
	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	bpi = (bpInfo *)c->data;
	o1 = bpi->o1;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;
	
	assert(height >= 2);
	assert(width >= 2);
	assert((height&1) == 0 );
	assert((width&1) == 0 );

	dp = band;	pp = parent;
	for(y=0;y<height;y+=2) 
	{
		dpn = dp + fullw;
		for(x=0;x<width;x+=2) 
		{	/** x & y are the parent's location *2 **/
			par = abs(pp[x>>1]);
			A = abs(dp[x]);		B = abs(dp[x+1]);
			C = abs(dpn[x]);	D = abs(dpn[x+1]);

			context = ((A & donemask)?1:0) + ((B & donemask)?1:0) + ((C & donemask)?1:0) + ((D & donemask)?1:0);
			if (par&nextmask) context += 5;

			// block is in 4 bits
			block  = ((A>>bitshift)&1);
			block |= ((B>>bitshift)&1)<<1;
			block |= ((C>>bitshift)&1)<<2;
			block |= ((D>>bitshift)&1)<<3;

			if ( block )
			{
				sozEncode(o1[context],bp_remap[block]);

				/** send signs when we see the first 'on' bit **/

				if ( (A & nextmask) == bitmask ) arithBit(ari, signbit(dp[x]   ) );
				if ( (B & nextmask) == bitmask ) arithBit(ari, signbit(dp[x+1] ) );
				if ( (C & nextmask) == bitmask ) arithBit(ari, signbit(dpn[x]  ) );
				if ( (D & nextmask) == bitmask ) arithBit(ari, signbit(dpn[x+1]) );
			}
			else
			{
				sozEncode(o1[context],0);
			}

		}
		pp += fullw;
		dp += fullw + fullw;
	}

}

void coderBPDecodeBandBP(coderParams *p)
{
uint32 x,y;
uint32 context,block,par,A,B,C,D;
uint32 donemask,nextmask,bitshift;
uint32 width,height,fullw;
bpInfo * bpi;
soz **o1;
arithInfo *ari;
coder *c;
int bitmask;
int *band,*parent;
int *dp,*pp,*dpn;

	c = p->coderPtr;
	bitshift = p->bitshift;
	bitmask = 1<<bitshift;
	nextmask = p->nextmask;
	donemask = nextmask ^ bitmask;
	bpi = (bpInfo *)c->data;
	o1 = bpi->o1;
	ari = c->arith;
	band = p->band;
	parent = p->parent;
	width = p->w;
	height = p->h;
	fullw = p->fullW;

	dp = band;	pp = parent;
	for(y=0;y<height;y+=2) 
	{
		if ( coderStopD(c) ) return;
		dpn = dp + fullw;
		for(x=0;x<width;x+=2) 
		{
			par = abs(pp[x>>1]);
			A = abs(dp[x]);		B = abs(dp[x+1]);
			C = abs(dpn[x]);	D = abs(dpn[x+1]);
			context = ((A & donemask)?1:0) + ((B & donemask)?1:0) + ((C & donemask)?1:0) + ((D & donemask)?1:0);
			if (par&nextmask) context += 5;

			block = sozDecode(o1[context]);
			if ( block )
			{
				block = bp_unmap[block];

				if ( block & 1 ) 
				{ 
					if ( !A ) dp[x] = arithGetBit(ari)? -bitmask: bitmask;
					else if ( dp[x] <0 ) dp[x] -= bitmask; else dp[x] += bitmask;
				}
				if ( block & 2 ) 
				{ 
					if ( !B ) dp[x+1] = arithGetBit(ari)? -bitmask: bitmask;
					else if ( dp[x+1] <0 ) dp[x+1] -= bitmask; else dp[x+1] += bitmask;
				}
				if ( block & 4 ) 
				{ 
					if ( !C ) dpn[x] = arithGetBit(ari)? -bitmask: bitmask;
					else if ( dpn[x] <0 ) dpn[x] -= bitmask; else dpn[x] += bitmask;
				}
				if ( block & 8 ) 
				{ 
					if ( !D ) dpn[x+1] = arithGetBit(ari)? -bitmask: bitmask;
					else if ( dpn[x+1] <0 ) dpn[x+1] -= bitmask; else dpn[x+1] += bitmask;
				}
			}
		}
		pp += fullw;
		dp += fullw + fullw;
	}

}

