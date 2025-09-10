/****************************************************************************************/
/*  CODEALPHAS.C                                                                        */
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

#define USE_RUNGAE

/*********

codealpha.c :
lossless coders for the alpha/transparency channel of the 'image'

note : alpha now coded as a 4th plane of the wavelet; the alpha
	routines here are never used (transparency routines still are)

------

<> we do (did) three things wrong : <>

	ALPHAS :
		1. alphas could be lossy	(transparencies can't be)
		** this is mostly fixed by now coding the alpha as the 4th wavelet plane

		2. the image & the alpha have mutual dependence which should be used
		** this is part of a more general problem of Y-U-V-A correlation
		
	TRANSPARENCIES:
		3. when a point is transparent, there's no need to code the actual data; we really should
			code transparency (or alphas as ):
				code boolean transparency
				if not transparent
					code pixel
		we are currently sending transparency first, so this could be directly added to codeimage;
			(we would have to clear the transparent image data to a standard zero as we skip it,
			for use in contexts)
		** this is mostly fixed by the new "zeroAlphaed" function in image.c


**********/

#include "Utility.h"
#include "Image.h"
#include "Coder.h"
#include "CodeUtil.h"
#include "o1coder.h"
#include "arithc.h"
#include "rungae.h"

#include "codealphas.h"

#include "Log.h"
#include "Tsc.h"

void codeImageAlpha(coder * coder,image * im,jeBoolean encoding);

void encodeImageAlpha(coder * encoder,image * im)
{
	if ( ! im->alpha ) return;
	codeImageAlpha(encoder,im,JE_TRUE);
}

void decodeImageAlpha(coder * decoder,image * im)
{
	if ( ! im->alpha ) return;
	codeImageAlpha(decoder,im,JE_FALSE);
}


static unsigned char grad_context_table[256] = 
{
	0,
	1,
	2,
	3,3,
	4,4,4,4,4,
	5,5,5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7
};

#define GRAD_BITS		(3)
#define SHAPE_BITS		(6)
#define SHAPES			(1<<SHAPE_BITS)
#define ALPHA_CONTEXTS	(1<<(GRAD_BITS + SHAPE_BITS))
#define SIGN_SHAPE_BITS	(4)
#define SIGN_SHAPE_MASK	((1<<SIGN_SHAPE_BITS)-1)
#define SIGN_CONTEXTS	(1<<(SIGN_SHAPE_BITS+1))		

#define BIT_CONTEXTS	(1<<5)

void codeImageAlpha(coder * coder,image * im,jeBoolean encoding)
{

	if ( im->alphaIsBoolean )
	{
	int x,y,bit,cntx,width,stride;
	uint8 *ptr;
	arithInfo * ari;
	int startPos;

		ari = coder->arith;
		startPos = 	arithTellEncPos(ari);
	
		if ( im->width < 4 )
		{
			// boot & send it raw !
			ptr = im->alpha;
			for(x=im->plane_size;x--;)
			{
				if ( encoding )
				{
					bit = *ptr++;
					arithEncBitRaw(ari,bit);
				}
				else
				{
					bit = arithDecBitRaw(ari);
					*ptr++ = bit;
				}
			}
			return;
		}

		// transparency mask

		// kind of like a JBIG coder; binary coder, context is the local neighborhood

		width = im->width;
		stride = im->stride;
		ptr = im->alpha;

		pushTSC();

	#ifdef USE_RUNGAE // {
	{
	rung_t rungs[BIT_CONTEXTS];

		// the rungs could stand to be more skewed for this purpose;
		//  we live pegged at P = min or P = max most of the time

		for(cntx=0;cntx<BIT_CONTEXTS;cntx++)
			rungModelInit(rungs+cntx);
		
		// y = 0 special case
		y = 0;
		for(x=0;x< width;x++)
		{
			cntx = 0;

			if ( x > 0 && ptr[-1] ) cntx += (1<<0);
			if ( x > 1 && ptr[-2] ) cntx += (1<<1);

			if ( encoding )
			{
				bit = *ptr++;
				rungModelEncBit(ari,bit,rungs+cntx);
			}
			else
			{
				bit = rungModelDecBit(ari,rungs+cntx);
				assert( (bit&1) == bit );
				*ptr++ = bit;
			}
		}
		
		ptr += stride - width;
								
		for(y=1;y< (im->height);y++)
		{
			x = 0;
			cntx = 0;
			if ( ptr[-stride] ) cntx += (1<<2);
			if ( encoding )
			{
				bit = *ptr++;
				rungModelEncBit(ari,bit,rungs+cntx);
			}
			else
			{
				bit = rungModelDecBit(ari,rungs+cntx);
				assert( (bit&1) == bit );
				*ptr++ = bit;
			}

			if ( encoding )
			{
				for(x=1;x< width;x++)
				{				
				uint8 *prev = ptr - stride;

					cntx = ptr[-1] ? (1<<0) : 0;
					if ( x > 1 ) cntx += ptr[-2] ? (1<<1) : 0;
					cntx += prev[0] ? (1<<2) : 0;
					cntx += prev[-1]? (1<<3) : 0;
				
					bit = *ptr++;
					rungModelEncBit(ari,bit,rungs+cntx);
				}
			}
			else
			{
				for(x=1;x< width;x++)
				{				
				uint8 *prev = ptr - stride;

					cntx = ptr[-1] ? (1<<0) : 0;
					if ( x > 1 ) cntx += ptr[-2] ? (1<<1) : 0;
					cntx += prev[0] ? (1<<2) : 0;
					cntx += prev[-1]? (1<<3) : 0;
				
					bit = rungModelDecBit(ari,rungs+cntx);
					assert( (bit&1) == bit );
					*ptr++ = bit;
				}
			}
			ptr += stride - width;
		}
	}
	#else // }{
	{
	int bitP0[BIT_CONTEXTS],bitPT[BIT_CONTEXTS];

		for(cntx=0;cntx<BIT_CONTEXTS;cntx++)
			bitModelInit(bitP0[cntx],bitPT[cntx]);
						
		for(y=0;y< (im->height);y++)
		{
			for(x=0;x< width;x++)
			{
				cntx = 0;

				if ( x > 1 && ptr[-2] ) cntx += (1<<0);
				if ( x > 0 && ptr[-1] ) cntx += (1<<1);

				if ( y > 0 )
				{
				uint8 *prev,*prev2;

					prev = ptr - stride;
					if ( y == 1 ) prev2 = prev;
					else prev2 = prev - stride;

					if ( x > 0 && prev[-1] ) cntx += (1<<2);

					if (  prev[0] ) cntx += (1<<3);
					if ( prev2[0] ) cntx += (1<<4);
				}
				
				if ( encoding )
				{
					bit = *ptr++;
					bitEnc(bit,ari,bitP0[cntx],bitPT[cntx]);
				}
				else
				{
					bitDec(bit,ari,bitP0[cntx],bitPT[cntx]);
					assert( (bit&1) == bit );
					*ptr++ = bit;
				}
			}
			ptr += stride - width;
		}
	}
	#endif }
		
		showPopTSC("Brando : Transparency");
		Log_Printf("Brando : Transparency : %d -> %d bits\n",width*(im->height),(arithTellEncPos(ari) - startPos)*8);
	}
	else	// alpha channel
	{
	oOne * o1;
	int x,width,pred,val,err,sign;
	int y;
	int gradCntx,shapeCntx,signCntx,cntx;
	uint8 *ptr,*prevline,*prevline2;
	int signP0[SIGN_CONTEXTS],signPT[SIGN_CONTEXTS];
	arithInfo * ari;

		ari = coder->arith;
	
		assert("should not get here" == NULL);

		// we use a CALIC-like DPCM
		//	this is not fast !!

		if ( im->width < 4 )
		{
			// boot & send it raw !
			ptr = im->alpha;
			for(x=im->plane_size;x--;)
			{
				if ( encoding )
				{
					arithEncByteRaw(ari,*ptr++);
				}
				else
				{
					*ptr++ = (uint8)arithDecByteRaw(ari);
				}
			}
			return;
		}

		// @@ there's a bug in this, it seems

		// <> use stride

		for(signCntx=0;signCntx<SIGN_CONTEXTS;signCntx++)
			bitModelInit(signP0[signCntx],signPT[signCntx]);

		o1 = oOneCreate(ari,256, ALPHA_CONTEXTS);
		assert(o1);

		sign = 0;
		width = im->width;
		ptr = im->alpha;

		for(y=0;y< (im->height);y++)
		{
			for(x=0;x< width;x++)
			{
			int w,ww,nw,n,ne,nn,nne;
			int dx,dy,predx,predy,d45,d135;

				prevline = ptr - width;
				prevline2 = prevline - width;

				if ( y == 0 )
				{
					prevline = prevline2 = ptr;

					if ( x == 0 ) w = ww = 128;
					else if ( x == 1 ) w = ww = ptr[-1];
					else { w = ptr[-1]; ww = ptr[-2]; }

					n = nn = nw = ne = nne = w;
				}
				else
				{
					if ( y == 1 )
						prevline2 = prevline;

					n = prevline[0];
					nn = prevline2[0];

					if ( x == 0 ) {
						w = ww = nw = prevline[0];
					} else if ( x == 1 ) {
						w = ww = ptr[0];
						nw = prevline[0];
					} else {
						w = ptr[-1]; ww = ptr[-2];
						nw = prevline[-1];
					}

					if ( x != (width-1) ) {
						nne = prevline2[1];
						ne = prevline[1];
					} else {
						nne = nn; ne = n;
					}
				}

				predx = w + ((w - ww)>>2) + ((ne - nw)>>1);	// vector from x
				predy = n + ((n - nn) + (w - nw) + (ne - nne))/3;	// vector from y

				dx = abs(w - ww) + abs(ne - n) + abs(n - nw);
				dy = abs(n - nn) + abs(w - nw) + abs(ne - nne);
				
		// gradient cuttoffs
#define GRAD1_DIFF	50	// <- doesn't matter much or help much
#define GRAD1_LO	20
#define GRAD2_DIFF	10	// <- hard to tune
#define GRAD2_LO	50

					   if ( (dy - dx) > GRAD1_DIFF && dx < GRAD1_LO ) {	// horizontal line
					/** almost zero pixels use the high-gradient predictor **/
					pred = predx;
				} else if ( (dx - dy) > GRAD1_DIFF && dy < GRAD1_LO ) { // vertical line
					pred = predy;
				} else if ( (dy - dx) > GRAD2_DIFF && dx < GRAD2_LO ) { // weak horizontal
					pred = (3*predx + predy)>>2;
				} else if ( (dx - dy) > GRAD2_DIFF && dy < GRAD2_LO ) { // weak vertical
					pred = (3*predy + predx)>>2;
				} else {						// no strong lines (or both strong lines)
					pred = (predx + predy)>>1;
				}

				if ( y > 1 && x > 0 ) {
					int nnw = prevline2[-1];

					/*** now do corrections for 45/135 gradients: ***/
					// uses nww and nnw	(not loaded)
						
					d45  = abs(w - ww) + abs(ne - nn) + abs(n - nnw);
					d135 = abs(w - n) + abs(nw - nn) + abs(n - nne);

					if ( d45 - d135 > 32 )
						pred += ((ne - nw)>>3);
					else if ( d45 - d135 > 16 )
						pred += ((ne - nw)>>4);
					else if ( d45 - d135 < 32 )
						pred -= ((ne - nw)>>3);
					else if ( d45 - d135 < 16 )
						pred -= ((ne - nw)>>4);
				}

				putminmax(pred,0,255);

				shapeCntx = 0;
				if ( w  > pred ) shapeCntx += 1<<0;
				if ( n  > pred ) shapeCntx += 1<<1;
				if ( ww > pred ) shapeCntx += 1<<2;
				if ( nn > pred ) shapeCntx += 1<<3;
				if ( nw > pred ) shapeCntx += 1<<4;
				if ( ne > pred ) shapeCntx += 1<<5;

				gradCntx = abs(dx + dy); putminmax(gradCntx,0,255);
				gradCntx = grad_context_table[gradCntx];
				cntx = (gradCntx<<SHAPE_BITS) + shapeCntx;
				signCntx = (sign<<SIGN_SHAPE_BITS) + (shapeCntx & SIGN_SHAPE_MASK);


				if ( encoding )
				{
					val = *ptr++;
					err = val - pred;

					if ( err < 0 )
					{
						err = -err;
						sign = 1;
					}
					else if ( err > 0 )
					{
						sign = 0;
					}

					oOneEncode(o1, err ,cntx);

					if ( err ) bitEnc(sign,ari,signP0[signCntx],signPT[signCntx]);
				}
				else
				{
					err = oOneDecode(o1,cntx);

					if ( err )
					{
						bitDec(sign,ari,signP0[signCntx],signPT[signCntx]);
						if ( sign )
							err = -err;
					}

					val = err + pred;
					*ptr++ = val;
				}
			}
			ptr += im->stride - width;
		}

		oOneFree(o1);
	}
	
#ifdef STUFF // <>
	{
	int stuff,got;

		stuff = 77;

		if ( encoding )
		{
			arithEncode(ari,stuff,stuff+1,373);
		}
		else
		{
			got = arithGet(ari,373);
			assert( got == stuff );
			arithDecode(ari,stuff,stuff+1,373);
		}

	}
#endif

}

