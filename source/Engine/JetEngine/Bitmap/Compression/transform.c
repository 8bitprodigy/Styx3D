/****************************************************************************************/
/*  TRANSFORM.C                                                                         */
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
#define DO_UNTRANS_BLOCK

#include "Utility.h"
#include "Image.h"
#include "IntMath.h"
#include "transform.h"
#include "Tsc.h"

typedef void (*jeWaveletFunc) (int *to,int *fm,int len);
void unjeWaveletImageIntPyramid(image *im,int levels,jeWaveletFunc waver,int lowScale,int highScale,pyramidHook hook,void *passback,jeBoolean doTransposeLHs);
void jeWaveletImageInt(image *im,int levels,jeBoolean inverse,jeWaveletFunc waver,jeBoolean doTransposeLHs,jeBoolean tblock);

extern void untransformBlocked(image *im,int levels,jeWaveletFunc i_waver,jeBoolean doLHs);

/********* transforms ***********/

/*** the master transform list : ****/

extern void cdf22forward(int * to,int *from,int len);
extern void cdf22inverse(int * to,int *from,int len);
extern void cdf22qforward(int * to,int *from,int len);
extern void cdf22qinverse(int * to,int *from,int len);
extern void cdf24forward(int * to,int *from,int len);
extern void cdf24inverse(int * to,int *from,int len);
extern void l97forward(int * to,int *from,int len);
extern void l97inverse(int * to,int *from,int len);
extern void d4forward(int * to,int *from,int len);
extern void d4inverse(int * to,int *from,int len);
extern void bcw3forward(int * to,int *from,int len);
extern void bcw3inverse(int * to,int *from,int len);
extern void sptforward(int * to,int *from,int len);
extern void sptinverse(int * to,int *from,int len);
extern void haarforward(int *to,int *from,int len);
extern void haarinverse(int *to,int *from,int len);

int nTransforms = 8;
jeWaveletFunc forwardTransforms[] = { l97forward, cdf22forward, cdf24forward, bcw3forward, d4forward, cdf22qforward, sptforward, haarforward, NULL };
jeWaveletFunc inverseTransforms[] = { l97inverse, cdf22inverse, cdf24inverse, bcw3inverse, d4inverse, cdf22qinverse, sptinverse, haarinverse, NULL };
char *    transformNames[] = { "l97", "cdf22", "cdf24", "bcw3", "d4" , "cdf22q", "S+P", "Haar", NULL };
jeBoolean transformMips [] = { 0, 1, 0, 0, 0, 1, 1, 1, 0 };

/****/

void unjeWaveletImageIntPyramid(image *im,int levels,jeWaveletFunc waver,int lowScale,int highScale,
										pyramidHook hook,void *passback,jeBoolean doLHs)
{
int x, y, w, h, l, p, width, height;
int *buffer,*tempbuf,*temprow;
int **rows;

	SetupUtility();

	width = im->width;
	height = im->height;

	assert(lowScale >= 0 );
	assert( (((width)>>(levels+1))<<(levels+1)) == width );
  	assert( (((height)>>(levels+1))<<(levels+1)) == height );
  
    /* Allocate a work array (for transposing columns) */
    
  	buffer = (int *)jeRam_Allocate(sizeof(int)*(height+height+max(width,height)));
	assert(buffer);

	temprow = buffer+height;
	tempbuf = buffer+height+height;

	if ( highScale > levels )
	{
	image * newim;
	int level,w,h;

		w = width>>levels;
		h = height>>levels;

		newim = newImage(w,h,im->planes);
		if ( ! newim ) return;
		
		// if highScale > levels, must sub-sample !

		patchImage(im,newim,0,0,w,h,0,0);

		for(level=levels+1;level<=highScale;level++)
		{
		int p;

			w = width>>level;
			h = height>>level;

			for(p=0;p<(newim->planes);p++)
			{
			int **rows;

				rows = newim->data[p];

				for(y=0;y<h;y++)
				{
				int *outptr,*line0,*line1;

					outptr = rows[y];	line0 = rows[y+y];	line1 = rows[y+y+1];
					for(x=w;x--;)
					{
						*outptr++ = (( line0[0] + line0[1] + line1[0] + line1[1] + 2 )>>2);
						line0 += 2; line1 += 2;
					}
				}
			}

			if ( level >= lowScale )
				hook(passback,newim,level,w,h);
		}
		
		freeImage(newim);
	}

	if ( levels <= highScale ) 
	{
		hook(passback,im,levels,width>>levels,height>>levels);
	}

	for (l = levels-1; l >= lowScale; l--) 
	{											 /** backwards in scale **/	
		w = width >> l;
		h = height >> l;

		// must do planes inside levels for pyramid version

		for(p=0;p<(im->planes);p++) 
		{
			rows = im->data[p];
			if ( doLHs )			
				transposeLH(im,p,l);

			/* Columns */

			cachetouch_w(tempbuf,h>>3);
			cachetouch_w(buffer, h>>3);
			for (x = 0; x < w; x++) 
			{
			int * tbptr;

				// <> ALL the time is spent on these two lines (see below)
				//	reading from a vertical array in memory is so bad!
				for (y = 0; y < h; y++) buffer[y] = rows[y][x];

				waver(tempbuf,buffer,h);

				tbptr = tempbuf - 1;

				// <> !!
				for (y = 1; y < h; y++) rows[y][x] = tbptr[y];

				temprow[x] = tempbuf[h-1];
			}

			/* Rows */
			cachetouch_w(rows[0], w>>3);
			for (y = 0; y < h-1; y++)
			{
				// <> this is amazingly cache-frienly : 
				//	row 0 is read from, then row 0 is written while row 1 is read, etc..
				waver(rows[y],rows[y+1],w);
			}
			waver(rows[h-1],temprow,w);
		}
		if ( l <= highScale ) 
		{
			hook(passback,im,l,w,h);
		}
	}

	jeRam_Free(buffer);
}

void jeWaveletImageInt(image *im,int levels,jeBoolean inverse,jeWaveletFunc waver,jeBoolean doLHs,jeBoolean doBlock)
{
int y, w, p, width, height;
int l;
int *buffer,*tempbuf,*temprow;
int **rows;

	SetupUtility();

	if ( inverse && doBlock )
	{
		untransformBlocked(im,levels,waver,doLHs);
		return;
	}

	width = im->width;
	height = im->height;

	// need (levels+1) because some of our coders need the LL to be even-sized !
	assert( (((width)>>(levels+1))<<(levels+1)) == width );
	assert( (((height)>>(levels+1))<<(levels+1)) == height );
  
    /* Allocate a work array (for transposing columns) */
    
  	buffer = (int *)jeRam_Allocate(sizeof(int)*(3*max(width,height)));
	assert(buffer);

	temprow = buffer +   max(width,height);
	tempbuf = buffer + 2*max(width,height);

	if ( ! inverse ) 
	{
		int h,x,l;

		for(p=0;p<(im->planes);p++) 
		{
		int y;
			rows = im->data[p];
  
			for (l = 0; l < levels; l++) 
			{
				w = width >> l;
				h = height >> l;

				/* Rows */
	
				waver(temprow,rows[h-1],w);
				for (y = h-2; y >=0; y--) 
				{
					waver(rows[y+1],rows[y],w);
				}
	
				/* Columns */
	
				for (x = 0; x < w; x++) 
				{
				int y;
					for (y = 1; y < h; y++) buffer[y-1] = rows[y][x];
					buffer[h-1] = temprow[x];

					waver(tempbuf,buffer,h);

					for (y = 0; y < h; y++) rows[y][x] = tempbuf[y];
				}

				if ( doLHs )			
					transposeLH(im,p,l);
			}
		}

		// @@ roll the transpose in with the last transform pass on columns, so its a pass on rows
		if ( doBlock )
			transposeImage(im);
	}
	else  // inverse
	{
		int h,x;

		assert(!doBlock);

		pushTSC();

		for(p=0;p<(im->planes);p++) 
		{
			rows = im->data[p];

			for (l = levels-1; l >= 0; l--) 
			{ /** backwards in scale **/
				w = width  >> l;
				h = height >> l;

				/* normal wavelet transform inverse */

				if ( doLHs )
					transposeLH(im,p,l);

				/* Columns */

				cachetouch_w(tempbuf,h>>3);
				cachetouch_w(buffer, h>>3);
				for (x = 0; x < w; x++) 
				{
				int * tbptr;

					// ALL the time is spent on these two lines (see below)
					//	reading from a vertical array in memory is so bad!
					for (y = 0; y < h; y++) buffer[y] = rows[y][x];

					waver(tempbuf,buffer,h);

					tbptr = tempbuf - 1;

					for (y = 1; y < h; y++) rows[y][x] = tbptr[y];

					temprow[x] = tempbuf[h-1];
				}

				/* Rows */
				cachetouch_w(rows[0], w>>3);
				for (y = 0; y < h-1; y++)
				{
					// this is amazingly cache-frienly : 
					//	row 0 is read from, then row 0 is written while row 1 is read, etc..
					waver(rows[y],rows[y+1],w);
				}
				waver(rows[h-1],temprow,w);
			}
		}
		
		showPopTSC("untrans : NOT blocked");
	}

	jeRam_Free(buffer);
}

void unTransformImageIntToPyramid(image *im,int levels,int transformN,
				int lowScale,int highScale,pyramidHook hook,void *passback,jeBoolean transposeLHs)
{
	if ( transformN > nTransforms || transformN < 0 )
		assert("tried to do invalid transformInt number" == NULL);

	unjeWaveletImageIntPyramid(im,levels,inverseTransforms[transformN],lowScale,highScale,hook,passback,transposeLHs);
}

void transformImageInt(image *im,int levels,jeBoolean inverse,int transformN,jeBoolean transposeLHs,jeBoolean doBlock)
{
	if ( transformN > nTransforms || transformN < 0 )
		assert("tried to do invalid transformInt number" == NULL);

	if ( inverse )	jeWaveletImageInt(im,levels,inverse,inverseTransforms[transformN],transposeLHs,doBlock);
	else			jeWaveletImageInt(im,levels,inverse,forwardTransforms[transformN],transposeLHs,doBlock);

}

void transposeLH(image *im,int plane,int level)
{
int x,y,w,h;
int **rows,*row,z;

	if ( im->width != im->height )
		return;

	// the LH is the lower-left band; it has vertical correlations;
	//	we want to make them horizontal !

	w = (im->width)  >> (level+1);
	h = (im->height) >> (level+1);
	rows = &(im->data[plane][h]);
	for(y=0;y<h;y++) 
	{
		row = rows[y];
		for(x=y+1;x<w;x++) 
		{
			z			= rows[x][y];
			rows[x][y]	= row[x];
			row[x]		= z;
		}
	}
}

void transposeHL(image *im,int plane,int level)
{
int x,y,w,yw,z;
int **rows,*row;

	if ( im->width != im->height )
		return;

	// the HL is the upper-right band

	w = (im->width)  >> (level+1);
	rows = im->data[plane];
	for(y=0;y<w;y++)
	{
		row = rows[y];
		yw = y+w;
		for(x=y+1;x<w;x++)
		{
			z			= rows[x][yw];
			rows[x][yw]	= row[x+w];
			row[x+w]	= z;
		}
	}
}

/**************

Encoder :

the raw data comes in at pyramid[0]

we do a transform like :

+------------------------+
| raw data (L0)          |
+------------+-----------+
|      L1    |      H1   |
+-----+------+-----------+
|  L2 |   H2 |
+--+--+------+
|LL|H3|
+--+--+

Then we code : LL,H3,H2,H1
with parents : 0 ,LL,L3,L2

(L3 == LL in this example)

****************/

void transformPyramid(int ** pyramid,int levels,int fulllen,int transformN,int inverse)
{
jeWaveletFunc waver;
int L,len;

	if ( transformN > nTransforms || transformN < 0 )
		assert("tried to do invalid transformInt number" == NULL);

	SetupUtility();

	if ( inverse )	waver = inverseTransforms[transformN];
	else			waver = forwardTransforms[transformN];

	if ( ! inverse )
	{
		//forward
		cachetouch_r(pyramid[0],fulllen>>3);
		for(L=0;L<levels;L++)
		{
			len = fulllen>>L;
			cachetouch_w(pyramid[L+1],len>>3);
			waver(pyramid[L+1],pyramid[L],len);
		}
	}
	else
	{
		//inverse
		cachetouch_r(pyramid[levels],fulllen>>(levels+3));
		for(L=levels-1;L>=0;L--)
		{
			len = fulllen>>L;
			cachetouch_w(pyramid[L],len>>3);
			waver(pyramid[L],pyramid[L+1],len);
		}
	}
}
