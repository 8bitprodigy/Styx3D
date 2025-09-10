/****************************************************************************************/
/*  CODER_F2.C                                                                          */
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

coderFast2

same as coderFast1 with rung on the signs

a *MINISCULE* improvement, though speed doesn't
seem to be affected much

*****/

#include "Utility.h"
#include "arithc.h"
#include "Coder.h"
#include "rungae.h"

jeBoolean coderFast2Init(coder *c);
void coderFast2Free(coder *c);
void coderFast2Flush(coder *c);
void coderFast2EncodeBandBP(coderParams *p);
void coderFast2DecodeBandBP(coderParams *p);

#define LINE_RUNG	(4)
#define SIGN_RUNG	(5)
#define CONTEXTS	(7)

coder coderFast2 = 
{
	"Fast",
	coderFast2Init,
	coderFast2Free,
	coderFast2EncodeBandBP,
	coderFast2DecodeBandBP,
	coderFast2Flush
	// the rest gets NULLs
};

typedef struct 
{
	rung_t rungs[CONTEXTS];
} bpInfo;

jeBoolean coderFast2Init(coder *c)
{
bpInfo *d;

	if ( !(d = (bpInfo *)new(bpInfo)) )
		return JE_FALSE;

	c->data = d;

	coderFast2Flush(c);

return JE_TRUE;
}

void coderFast2Free(coder *c)
{
	if ( c->data ) 
	{
		bpInfo *d;
		d = (bpInfo *)c->data;
		destroy(d);
		c->data = NULL;
	}
}

void coderFast2Flush(coder *c)
{
bpInfo *d;
int i;
	d = (bpInfo *)c->data;
	for(i=0;i<CONTEXTS;i++)
	{
		rungModelInit(&(d->rungs[i]));
	}
}

void coderFast2EncodeBandBP(coderParams *p)
{
int x,y,width,height,fullw;
int bitshift;
int par,sign,signcntx;
int val;
bpInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*parent,*dp,*pp;
rung_t * rungs;

	c		= p->coderPtr;
	ari		= c->arith;
	bpi		= (bpInfo *)c->data;
	rungs	= bpi->rungs;

	bitshift= p->bitshift;
	band	= p->band;
	parent	= p->parent;
	width	= p->w;
	height	= p->h;
	fullw	= p->fullW;

	signcntx = 0;

	assert(width >= 2);
	assert((width&1) == 0 );

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{		
		// pre-scan whole line:

		dp = band + y*fullw;
		for(x=width;x--;) 
		{
		int val;
			val = *dp++;
			val = abs(val);
			val = (val>>bitshift)&1;
			if ( val )
				goto CodeLine;
		}

		rungModelEncBit(ari,0,rungs+LINE_RUNG);
		continue;

		CodeLine:
		rungModelEncBit(ari,1,rungs+LINE_RUNG);

		dp = band + y*fullw;	
		pp = parent + (y>>1)*fullw;
		for(x=0;x<width;x+=2) 
		{
			par = abs(*pp); pp++;
			par = par >> bitshift;
			par = par ? 2 : 0;

			val = *dp++;
			sign = 0;
			if ( val < 0 )
			{
				val = - val;
				sign = 1;
			}

			if ( (val>>bitshift)&1 )
			{
				if ( val>>(bitshift+1) )
				{
					rungModelEncBit(ari,1,rungs+par+1);
				}
				else	// bit & not done
				{
					rungModelEncBit(ari,1,rungs+par);
					rungModelEncBit(ari,sign,rungs+SIGN_RUNG+signcntx);
					signcntx = sign;
				}
			}
			else
			{
			int context;
				context = par + ((val>>(bitshift+1)) ? 1 : 0);
				rungModelEncBit(ari,0,rungs+context);
			}

			val = *dp++;
			sign = 0;
			if ( val < 0 )
			{
				val = - val;
				sign = 1;
			}

			if ( (val>>bitshift)&1 )
			{
				if ( val>>(bitshift+1) )
				{
					rungModelEncBit(ari,1,rungs+par+1);
				}
				else	// bit & not done
				{
					rungModelEncBit(ari,1,rungs+par);
					rungModelEncBit(ari,sign,rungs+SIGN_RUNG+signcntx);
					signcntx = sign;
				}
			}
			else
			{
			int context;
				context = par + ((val>>(bitshift+1)) ? 1 : 0);
				rungModelEncBit(ari,0,rungs+context);
			}
		}
	}
}

void coderFast2DecodeBandBP(coderParams *p)
{
int x,y,width,height,fullw;
int donemask,nextmask,bitshift,bitmask;
int par,bit;
int val,mask,sign,signcntx;
bpInfo * bpi;
arithInfo *ari;
coder *c;
int *band,*parent,*dp,*pp;
rung_t * rungs;

	c		= p->coderPtr;
	ari		= c->arith;
	bpi		= (bpInfo *)c->data;
	rungs	= bpi->rungs;

	bitshift= p->bitshift;
	bitmask = 1<<bitshift;
	nextmask= p->nextmask;
	donemask= nextmask ^ bitmask;
	band	= p->band;
	parent	= p->parent;
	width	= p->w;
	height	= p->h;
	fullw	= p->fullW;

	signcntx = 0;

	dp = band;	pp = parent;
	for(y=0;y<height;y++) 
	{
		if ( ! rungModelDecBit(ari,rungs+LINE_RUNG) )
			continue;

		if ( coderStopD(c) ) return;
		
		pp = parent + (y>>1)*fullw;
		dp = band + y*fullw;
		for(x=0;x<width;x+=2) 
		{
			par = *pp++;
			par = abs(par) & nextmask;
			par = par ? 2 : 0;

			val = *dp;
			sign = (val < 0) ? (-1) : (0);
			val  = (val ^ sign)		+ (sign&1);
			mask = (bitmask ^ sign) + (sign&1);
			assert( val >= 0 );
			assert( mask == bitmask || mask == (-bitmask) );

			if ( val&donemask )
			{
				bit = rungModelDecBit(ari,rungs+par+1);
				*dp += (bit ? mask : 0 );
			}
			else	// bit & not done
			{
				if ( rungModelDecBit(ari,rungs+par) )
				{
					signcntx = rungModelDecBit(ari,rungs+SIGN_RUNG+signcntx);
					*dp = signcntx ? (-bitmask) : bitmask;
				}
			}
			dp++;

			val = *dp;
			sign = (val < 0) ? (-1) : (0);
			val  = (val ^ sign)		+ (sign&1);
			mask = (bitmask ^ sign) + (sign&1);
			assert( val >= 0 );
			assert( mask == bitmask || mask == (-bitmask) );

			if ( val&donemask )
			{
				bit = rungModelDecBit(ari,rungs+par+1);
				*dp += (bit ? mask : 0 );
			}
			else	// bit & not done
			{
				if ( rungModelDecBit(ari,rungs+par) )
				{
					signcntx = rungModelDecBit(ari,rungs+SIGN_RUNG+signcntx);
					*dp = signcntx ? (-bitmask) : bitmask;
				}
			}
			dp++;
		}
	}

}

