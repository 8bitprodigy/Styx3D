/****************************************************************************************/
/*  CODEIMAGE.C                                                                         */
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

// <> could special-case the common occurance of the H-band being pure zeros (?)

//#define CODEIMAGE_DOZERO

#include "Utility.h"
#include "CodeUtil.h"

#include "Image.h"
#include "Coder.h"
#include "Wavelet.h"
#include "Wavelet._h"
#include "codeimage.h"
#include "codealphas.h"
#include "IntMath.h"
#include "transform.h"
#include "Timer.h"

TIMER_VARS(CodeImage_Zero);
TIMER_VARS(CodeImage_DecodeBand);

/******

'codeimage' is the Object Master, calling the objects in 'coder_*.c' and 'trans_*.c'

coder.c has no idea what an "image" is.
image.c has no idea what a "coder is.

only this module knows about both.

********/

void CodeImage_Report(void)
{
	TIMER_REPORT(CodeImage_Zero);
	TIMER_REPORT(CodeImage_DecodeBand);
}

/***** internal-only protos : ******/

void encodeImage(jeWavelet *w,coder * encoder,image *im);
void decodeImage(jeWavelet *w,coder * decoder,image *im);
void encodeLL(jeWavelet * w,coder * encoder,image *im);
void decodeLL(jeWavelet * w,coder * decoder,image *im);

void encodeImageBand(coderParams *CP);

/*******/

jeBoolean encodeWaveletImage(jeWavelet *w,image *im)
{
coder *encoder;

	if ( (encoder = coderCreateWrite(w->coderN,w->comp)) == NULL ) 
	{
		return JE_FALSE;
	}

	encodeImageAlpha(encoder,im);

	encodeImage(w,encoder,im);

	w->compLen = coderFlushWrite(encoder);
	coderDestroy(encoder);

return JE_TRUE;
}

jeBoolean decodeWaveletImage(jeWavelet *w,image *im)
{
coder *decoder;

	if ( (decoder = coderCreateRead(w->coderN,w->comp,w->stopLen)) == NULL ) 
	{
		return JE_FALSE;
	}

	decodeImageAlpha(decoder,im);

	decodeImage(w,decoder,im);

	coderFlushRead(decoder);
	coderDestroy(decoder);

return JE_TRUE;
}

void encodeImage(jeWavelet *w,coder * encoder,image *im)
{
int p,sizeX,sizeY,x,y;
int l,levels;
int val,top_val,top_bitpn,bitmask;
int **rows;
int *dp;
coderParams CP;

	if ( ! encoder->encodeBandBP ) return;

	levels = w->levels;

	encodeLL(w,encoder,im);

	w->compLenLL = arithTellEncPos(encoder->arith) + 2;

	top_val = 0;

	for(p=0;p<(im->planes);p++) 
	{
		rows = im->data[p];

		sizeX = (im->width) >> levels;
		sizeY = (im->height) >> levels;
		for(y=0;y<sizeY;y++) 
		{
			dp = rows[y] + sizeX;
			for(x=sizeX;x<(im->width);x++) 
			{
				val = abs(*dp); dp++;
				if ( val > top_val ) 
					top_val = val;
			}
		}
		for(y=sizeY;y<im->height;y++)
		{
			dp = rows[y];
			for(x=im->width;x--;)
			{
				val = abs(*dp); dp++;
				if ( val > top_val ) 
					top_val = val;
			}
		}
	}

	assert( top_val >= 0 && top_val < (1<<24) );

	for(top_bitpn=0;(1<<(top_bitpn+1))<=top_val;top_bitpn++) ;

	w->bps = top_bitpn;
	top_val = 1<<top_bitpn;

	CP.coderPtr = encoder;
	CP.fullW = im->stride;

	for(bitmask = top_val;bitmask>=1;bitmask>>=1) 
	{
		CP.bitshift = intlog2(bitmask);
		assert( (1<<CP.bitshift) == bitmask);
		for(l= bitmask,CP.nextmask=0; l<CODE_MAX_VAL ;l+=l) CP.nextmask += l;

		for (l = levels; l > 0; l--) 
		{
			CP.w = sizeX = (im->width) >> l;
			CP.h = sizeY = (im->height) >> l;

			for(p=0;p<(im->planes);p++) 
			{
				rows = im->data[p];

				CP.bandN = 0;
				CP.band = rows[0] + sizeX;
				CP.parent = rows[0] + (sizeX>>1);		
				encodeImageBand(&CP);
				CP.bandN = 1;
				CP.band = rows[sizeY];
				CP.parent = rows[sizeY>>1];
				encodeImageBand(&CP);
				CP.bandN = 2;
				CP.band = rows[sizeY] + sizeX;
				CP.parent = rows[sizeY>>1] + (sizeX>>1);		
				encodeImageBand(&CP);
			}
		}
	}
}

void zeroBand(coderParams *CP)
{
	TIMER_P(CodeImage_Zero);
#ifdef CODEIMAGE_DOZERO
{
int *dp,y,width,height;
	dp = CP->band;
	width  = CP->w;
	height = CP->h;
	for(y=0;y<height;y++)
	{
		fastmemclear(dp,width*sizeof(int));
		dp += CP->fullW;
	}
}
#endif
	TIMER_Q(CodeImage_Zero);
}

void decodeImage(jeWavelet *w,coder * decoder,image *im)
{
int p,sizeX,sizeY;
int l,levels;
int top_bitmask,top_bitpn,bitmask;
int **rows;
coderParams CP;
int bandszeroed[32][3][4];
int i,j,k;

	assert( decoder->decodeBandBP );

	levels = w->levels;

	decodeLL(w,decoder,im);

	top_bitpn = w->bps;
	top_bitmask = 1<<top_bitpn;

	CP.coderPtr = decoder;
	CP.fullW = im->stride;

	for(i=0;i<32;i++)
		for(j=0;j<3;j++)
			for(k=0;k<4;k++)
				bandszeroed[i][j][k] = 0;

	for(bitmask = top_bitmask;bitmask>=1;bitmask>>=1) 
	{
		CP.bitshift = intlog2(bitmask);
		assert( (1<<CP.bitshift) == bitmask);
		for(l= bitmask,CP.nextmask=0; l<CODE_MAX_VAL ;l+=l) CP.nextmask += l;

		if ( coderStopD(decoder) )
			break;

		for (l = levels; l > 0; l--) 
		{
			CP.w = sizeX = (im->width) >> l;
			CP.h = sizeY = (im->height) >> l;

			if ( coderStopD(decoder) )
				break;

			for(p=0;p<(im->planes);p++) 
			{
				rows = im->data[p];

				if ( coderStopD(decoder) )
					break;

				if ( arithDecBitRaw(decoder->arith) )
				{
					CP.bandN = 0;
					CP.band = rows[0] + sizeX;
					CP.parent = rows[0] + (sizeX>>1);		
					if ( ! bandszeroed[l][CP.bandN][p] )
					{
						zeroBand(&CP);
						bandszeroed[l][CP.bandN][p] = 1;
					}
					TIMER_P(CodeImage_DecodeBand);
					decoder->decodeBandBP(&CP);
					TIMER_Q(CodeImage_DecodeBand);
				}

				if ( coderStopD(decoder) )
					break;

				if ( arithDecBitRaw(decoder->arith) )
				{
					CP.bandN = 1;
					CP.band = rows[sizeY];
					CP.parent = rows[(sizeY>>1)];		
					if ( ! bandszeroed[l][CP.bandN][p] )
					{
						zeroBand(&CP);
						bandszeroed[l][CP.bandN][p] = 1;
					}
					TIMER_P(CodeImage_DecodeBand);
					decoder->decodeBandBP(&CP);
					TIMER_Q(CodeImage_DecodeBand);
				}
						
				if ( coderStopD(decoder) )
					break;

				if ( arithDecBitRaw(decoder->arith) )
				{
					CP.bandN = 2;
					CP.band = rows[sizeY] + sizeX;
					CP.parent = rows[(sizeY>>1)] + (sizeX>>1);		
					if ( ! bandszeroed[l][CP.bandN][p] )
					{
						zeroBand(&CP);
						bandszeroed[l][CP.bandN][p] = 1;
					}
					TIMER_P(CodeImage_DecodeBand);
					decoder->decodeBandBP(&CP);
					TIMER_Q(CodeImage_DecodeBand);
				}
			}
		}
	}
	
	for(l=levels;l>0;l--)
	{
		for(p=0;p<(im->planes);p++)
		{
		int n;
			for(n=0;n<3;n++)
			{
				if ( ! bandszeroed[l][n][p] )
				{
					CP.w = (im->width)  >> l;
					CP.h = (im->height) >> l;
					CP.bandN = n;
					switch(n)
					{
						case 0:
							CP.band = rows[0] + sizeX;
							break;
						case 1:
							CP.band = rows[sizeY];
							break;
						case 2:
							CP.band = rows[sizeY] + sizeX;	
							break;
					}
					zeroBand(&CP);
				}
			}
		}
	}

}

void encodeLL(jeWavelet * w,coder * encoder,image *im)
{
int p,llw,llh;

	llw = (im->width )>>(w->levels);
	llh = (im->height)>>(w->levels);

	for(p=0;p<(im->planes);p++)
		coderEncodeDPCM(encoder,im->data[p][0],llw,llh,im->stride - llw);
}

void decodeLL(jeWavelet * w,coder * decoder,image *im)
{
int p,llw,llh;

	llw = (im->width )>>(w->levels);
	llh = (im->height)>>(w->levels);

	for(p=0;p<(im->planes);p++)
		coderDecodeDPCM(decoder,im->data[p][0],llw,llh,im->stride - llw);
}

void encodeImageBand(coderParams *CP)
{
int * dp;
int x,y,width,height,bitmask;
	dp = CP->band;
	width = CP->w;
	height = CP->h;
	bitmask = 1UL<<CP->bitshift;
	for(y=0;y<height;y++) 
	{
		for(x=(width>>1);x--;) 
		{
		int A,B;
			A = abs(dp[0]);
			B = abs(dp[1]);
			dp += 2;
			
			A = (A&bitmask) + (B&bitmask);

			if ( A )
			{
				arithEncBitRaw(CP->coderPtr->arith,1);

				CP->coderPtr->encodeBandBP(CP);

				return;
			}
		}
		dp += CP->fullW - width;
	}

	arithEncBitRaw(CP->coderPtr->arith,0);
return;
}
