/****************************************************************************************/
/*  CODER_F3.C                                                                          */
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

coderFast3

*****/

#include "Utility.h"
#include "arithc.h"
#include "Coder.h"
#include "rungae.h"
#include "Log.h"
//#include "coder_asm.h"

#ifdef _LOG
#define LOG(x) x
int linesSkipped=0,linesCoded=0;
#else
#define LOG(x)
#endif

jeBoolean coderFast3Init(coder *c);
void coderFast3Free(coder *c);
void coderFast3_flush(coder *c);
void coderFast3EncodeBandBP(coderParams *p);
void coderFast3DecodeBandBP(coderParams *p);

#define CONTEXTS	(3)

coder coderFast3 = 
{
	"Fast3",
	coderFast3Init,
	coderFast3Free,
	coderFast3EncodeBandBP,
	coderFast3DecodeBandBP,
	coderFast3_flush
	// the rest gets NULLs
};

typedef struct 
{
	rung_t rungs[CONTEXTS];
} bpInfo;

jeBoolean coderFast3Init(coder *c)
{
bpInfo *d;

	if ( !(d = (bpInfo *)new(bpInfo)) )
		return JE_FALSE;

	c->data = d;

	coderFast3_flush(c);

return JE_TRUE;
}

void coderFast3Free(coder *c)
{
	
#ifdef _LOG
	if ( linesCoded )
	{
		Log_Printf("skipped = %d, coded = %d\n",linesSkipped,linesCoded);
		linesCoded = linesSkipped = 0;
	}
#endif

	if ( c->data ) 
	{
		bpInfo *d;
		d = (bpInfo *)c->data;
		destroy(d);
		c->data = NULL;
	}
}

void coderFast3_flush(coder *c)
{
bpInfo *d;
int i;
	d = (bpInfo *)c->data;
	for(i=0;i<CONTEXTS;i++)
	{
		rungModelInit(&(d->rungs[i]));
	}
}

void coderFast3EncodeBandBP(coderParams *p)
{
int y,width,height,fullw;
int bitshift;
int sign;
bpInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*dp;
rung_t * rungs;
int val,x;

	c		= p->coderPtr;
	ari		= c->arith;
	bpi		= (bpInfo *)c->data;
	rungs	= bpi->rungs;

	bitshift= p->bitshift;
	band	= p->band;
	width	= p->w;
	height	= p->h;
	fullw	= p->fullW;

	dp = band;
	for(y=0;y<height;y++) 
	{
		// pre-scan whole line:

		dp = band + y*fullw;
		for(x=width;x--;) 
		{
			val = *dp++;
			val = abs(val);
			val = (val>>bitshift)&1;
			if ( val )
				goto CodeLine;
		}

		rungModelEncBit(ari,0,rungs+2);
		LOG(linesSkipped++);
		continue;

		CodeLine:
		rungModelEncBit(ari,1,rungs+2);
		LOG(linesCoded++);

		dp = band + y*fullw;

		for(x=width;x--;) 
		{
			val = *dp++;

			/* could do this blazing fast with cdq :
			mov		eax, val	// eax = val
			cdq					// edx = sign	
			mov		ecx, edx
			xor		eax, edx	// no stall on edx
			and		ecx, 01h
			add		eax, ecx	// now eax = abs(val) and ecx = (val < 0 ) ? 1 : 0;
			*/

			sign = (val < 0) ? -1 : 0;
			val = abs(val);
			val >>= bitshift;
			rungModelEncBit(ari,val&1,rungs);
			if ( val == 1 )
			{
				arithEncBitRaw(ari,sign);
			}
		}
	}
}

void coderFast3DecodeBandBP(coderParams *p)
{
int x,y,width,height,fullw;
int donemask,bitshift,bitmask;
bpInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*dp;
rung_t * rungs;

	c		= p->coderPtr;
	ari		= c->arith;
	bpi		= (bpInfo *)c->data;
	rungs	= bpi->rungs;

	bitshift= p->bitshift;
	bitmask = 1<<bitshift;
	donemask= p->nextmask ^ bitmask;
	band	= p->band;
	width	= p->w;
	height	= p->h;
	fullw	= p->fullW;

#if 0 // @@
	// almost pure ASM
	// on hare 512:
	// Decode Image : 0.055393 secs = 105.7 cycles / pixel

	// the ASM uses the cdq tricks and cmov !
	// god damn!

	// in coder_asm.asm :
	asmCoderFast3DecodeLines(ari,rungs,bitmask,donemask,band,width,height,fullw,rungs+2,c->wStopLen);
#else

	// pure C :  
	// on hare 512:
	// Decode Image : 0.052965 secs = 101.0 cycles / pixel = 4949350 pixels /sec

	for(y=0;y<height;y++) 
	{
		if ( ! rungModelDecBit(ari,rungs+2) )
			continue;

		if ( coderStopD(c) ) return;
		dp = band + y*fullw;
		
		for(x=0;x<width;x++)
		{
		int val;
			if ( ! rungModelDecBit(ari,rungs) )
				continue;

			val = dp[x];

			if ( val < 0 )
			{
				val = - val;
				
				if ( val&donemask )
				{
					dp[x] -= bitmask;
					continue;
				}

				// bit & not done
				dp[x] = arithDecBitRaw(ari) ? (- bitmask) : bitmask;

				continue;
			}

			if ( val&donemask )
			{
				dp[x] += bitmask;
				continue;
			}

			// bit & not done
			dp[x] = arithDecBitRaw(ari) ? (- bitmask) : bitmask;
		}
	}
#endif

}
