/****************************************************************************************/
/*  TBLOCK.C                                                                            */
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
/*{**** BOF ****/

#include "TBlock.h"
#include "codeimage.h"
#include "Tsc.h"
#include "Log.h"

#include "Timer.h"

TIMER_VARS(TBlock_All);
TIMER_VARS(TBlock_Ram);
TIMER_VARS(TBlock_Transpose);
TIMER_VARS(TBlock_H);
TIMER_VARS(TBlock_HB);
TIMER_VARS(TBlock_V);
TIMER_VARS(TBlock_VB);
TIMER_VARS(TBlock_H_SpinUpDown);
TIMER_VARS(TBlock_H_Waver);
TIMER_VARS(TBlock_H_Block);
TIMER_VARS(TBlock_V_UnBlock);
TIMER_VARS(TBlock_V_Waver);

//#define cachetouch_w(x,y)
//#define cachetouch_r(x,y)

/**
with cachetouch disabled:

TBlock_All           : 0.039409 : 48.0 %
TBlock_Ram           : 0.000285 : 0.3 %
TBlock_Transpose     : 0.000000 : 0.0 %
TBlock_H             : 0.012152 : 14.8 %	// these four numbers vary pretty wildly:
TBlock_HB            : 0.009154 : 11.2 %
TBlock_V             : 0.013977 : 17.0 %
TBlock_VB            : 0.003804 : 4.6 %

with cachetouch:

TBlock_All           : 0.043376 : 48.9 %
TBlock_Ram           : 0.000291 : 0.3 %
TBlock_Transpose     : 0.000000 : 0.0 %
TBlock_H             : 0.012161 : 13.7 %
TBlock_HB            : 0.010536 : 11.9 %
TBlock_V             : 0.016425 : 18.5 %
TBlock_VB            : 0.003923 : 4.4 %

**/

typedef struct {
	jeWaveletFunc waver;
	int * blocks;
	int * trows[9];
	int stride8;
	int ** rows;
} tblockInfo;

#define DO8(x)	do { x; x; x; x; x; x; x; x; } while(0)

/*}{**** row <-> block copiers ****/

void __inline rowtoblock(int * bptr,const int *row,int w8)
{
int x8;

	for(x8=w8;x8--;)
	{
		DO8(*bptr++ = *row++);
		bptr += 56;
	}
}

void __inline blocktorow(int * row,const int *bptr,int x)
{
int x8;

	x8 = x>>3;
	while(x8--)
	{
		DO8(*row++ = *bptr++);
		bptr += 56;
	}
	x = x&7;
	while(x--)
	{
		*row++ = *bptr++;
	}
}

void __inline blockvtorow(int * row,const int *bptr,int y,int stride8)
{
int y8;

	y8 = y>>3;
	while(y8--)
	{
		DO8(*row++ = *bptr; bptr += 8);
		bptr += stride8 - 64;
	}
	y = y&7;
	while(y--)
	{
		*row++ = *bptr; bptr += 8;
	}
}

void __inline rowtoblockv(int *bptr,const int *row,int y,int stride8)
{
int y8;

	y8 = y>>3;
	while(y8--)
	{
		DO8(*bptr = *row++; bptr += 8);
		bptr += stride8 - 64;
	}
	y = y&7;
	while(y--)
	{
		*bptr = *row++; bptr += 8;
	}
}

/***
void rowtoblock8(int * bptr,const int **inrows,int w8)
{
int x8;
int * rows[8];

	memcpy(rows,inrows,32);

	for(x8=w8;x8--;)
	{
		DO8(*bptr++ = *rows[0]++);
		DO8(*bptr++ = *rows[1]++);
		DO8(*bptr++ = *rows[2]++);
		DO8(*bptr++ = *rows[3]++);
		DO8(*bptr++ = *rows[4]++);
		DO8(*bptr++ = *rows[5]++);
		DO8(*bptr++ = *rows[6]++);
		DO8(*bptr++ = *rows[7]++);
	}
}
***/

void rowtoblock8(int * inbptr,const int **inrows,int w8)
{
uint32 rows[8],bptr;

	//__asm { int 3 };

	bptr = (uint32)inbptr;
	memcpy(rows,inrows,32);

	while(w8--)
	{
		copy32_8((char *)bptr,(char **)rows);
		rows[0] += 32; rows[1] += 32; rows[2] += 32; rows[3] += 32;
		rows[4] += 32; rows[5] += 32; rows[6] += 32; rows[7] += 32;
		bptr += 256;
	}
}

void blockvtorow8(int ** rows,const int *bptr,int h,int stride8)
{
int y8;
int y;
int *row0,*row1,*row2,*row3,*row4,*row5,*row6,*row7;

	row0 = rows[0]; cachetouch_w(row0,h>>3);
	row1 = rows[1]; cachetouch_w(row1,h>>3);
	row2 = rows[2]; cachetouch_w(row2,h>>3);
	row3 = rows[3]; cachetouch_w(row3,h>>3);
	row4 = rows[4]; cachetouch_w(row4,h>>3);
	row5 = rows[5]; cachetouch_w(row5,h>>3);
	row6 = rows[6]; cachetouch_w(row6,h>>3);
	row7 = rows[7]; cachetouch_w(row7,h>>3);
	y8 = h>>3;
	while(y8--)
	{
		DO8(*row0++ = bptr[0]; *row1++ = bptr[1]; *row2++ = bptr[2]; *row3++ = bptr[3];	\
			*row4++ = bptr[4]; *row5++ = bptr[5]; *row6++ = bptr[6]; *row7++ = bptr[7];	\
			bptr += 8; );
		bptr += stride8 - 64;
	}
	y = h&7;
	while(y--)
	{
		*row0++ = *bptr++;
		*row1++ = *bptr++;
		*row2++ = *bptr++;
		*row3++ = *bptr++;
		*row4++ = *bptr++;
		*row5++ = *bptr++;
		*row6++ = *bptr++;
		*row7++ = *bptr++;
	}
}

void rowtoblockv8(int *bptr,const int ** rows,int h,int stride8)
{
int y8;
int y;
const int *row0,*row1,*row2,*row3,*row4,*row5,*row6,*row7;

	row0 = rows[0];	row1 = rows[1];
	row2 = rows[2];	row3 = rows[3];
	row4 = rows[4];	row5 = rows[5];
	row6 = rows[6];	row7 = rows[7];
	y8 = h>>3;
	while(y8--)
	{
		cachetouch_w(bptr,1);
		DO8(bptr[0] = *row0++; bptr[1] = *row1++; bptr[2] = *row2++; bptr[3] = *row3++; \
			bptr[4] = *row4++; bptr[5] = *row5++; bptr[6] = *row6++; bptr[7] = *row7++; \
			bptr += 8; );
		bptr += stride8 - 64;
	}
	y = h&7;
	while(y--)
	{
		bptr[0] = *row0++;
		bptr[1] = *row1++;
		bptr[2] = *row2++;
		bptr[3] = *row3++;
		bptr[4] = *row4++;
		bptr[5] = *row5++;
		bptr[6] = *row6++;
		bptr[7] = *row7++;
		bptr += 8;
	}
}

/*}{**** transformers ; row <-> block **************/

void untH(int starty,int endy,int w,tblockInfo * tbi)
{
int *workrow;
int y8,yi,y,w8;
int * bptr;
int stride8,**rows,*blocks;
jeWaveletFunc waver;

	TIMER_P(TBlock_H);

	waver	= tbi->waver;
	stride8 = tbi->stride8;
	blocks	= tbi->blocks;
	rows	= tbi->rows;

	// (row+row) -> (brow)

	// <> we should make a roll-8 version, but then we need
	//	a spin-up and a spin-down loop

	w8 = (w+7)>>3;
	y8 = (starty>>3);
	yi = (starty&7);
	bptr = blocks + stride8*y8 + 8*yi;
	for(y=starty;y<endy;y++)
	{
		workrow = rows[y-1];
		waver(workrow,rows[y],w);		// workrow <- rows[y]
		rowtoblock(bptr,workrow,w8);	// block   <- workrow
	 
		bptr += 8;	// point to next line in blocks !
		yi++;
		if ( yi == 8 )
		{
			yi = 0; y8 ++;
			bptr = blocks + stride8*y8;
		}
		assert(y8 == ((y+1)>>3));
	}
	
	TIMER_Q(TBlock_H);
}

void untH2(int starty,int endy,int w,tblockInfo * tbi)
{
int y8,nexty,y,w8,i;
int * bptr;
int stride8,**rows,*blocks;
jeWaveletFunc waver;

	TIMER_P(TBlock_H);

	waver	= tbi->waver;
	stride8 = tbi->stride8;
	blocks	= tbi->blocks;
	rows	= tbi->rows;

	// (row+row) -> (brow)

	// a roll-8 version
	//	with a spin-up and a spin-down loop

	y  = starty;
	w8 = (w+7)>>3;
	bptr = blocks + stride8*(starty>>3) + 8*(starty&7);

	nexty = ((starty+7)&(~7));
	if ( nexty > endy ) nexty = endy;

	TIMER_P(TBlock_H_SpinUpDown);

	for(;y<nexty;y++)
	{
		waver(rows[y-1],rows[y],w);		// workrow <- rows[y]
		rowtoblock(bptr,rows[y-1],w8);	// block   <- workrow
		bptr += 8;	// point to next line in blocks !
	}
	
	TIMER_Q(TBlock_H_SpinUpDown);

	bptr = blocks + stride8*(y>>3);
	y8 = (endy - y)>>3;

	nexty = y + (y8<<3);

/**

	----
	separated : 
	TBlock_H_Waver       : 0.005721 : 6.7 %
	TBlock_H_Block       : 0.008363 : 9.9 %
	----
		
	TIMER_P(TBlock_H_Waver);
	for(i=y;i<nexty;i++)
	{
		// this is cache optimal ; read a row, then write it
		waver(rows[i-1],rows[i],w);		// workrow <- rows[y]
	}
	TIMER_Q(TBlock_H_Waver);

	TIMER_P(TBlock_H_Block);
	while(y8--)
	{
		rowtoblock8(bptr,rows + y-1,w8);	// blocks <- rows
		y += 8;
		bptr += stride8;
	}
	TIMER_Q(TBlock_H_Block);

	----
	merged:
	TBlock_H_Waver       : 0.005636 : 6.8 %
	TBlock_H_Block       : 0.006693 : 8.1 %
	----

	copy32_8 assembly:

	TBlock_H_Waver       : 0.005728 : 6.4 %
	TBlock_H_Block       : 0.006458 : 7.2 %

**/

	while(y8--)
	{
		TIMER_P(TBlock_H_Waver);
		cachetouch_w(rows[y-1],w8); // this row may not be in cache yet
		//cachetouch_r(rows[y],w8);
		for(i=0;i<8;i++)
		{
			// this is cache optimal ; read a row, then write it
			waver(rows[y+i-1],rows[y+i],w);		// workrow <- rows[y]
		}
		TIMER_Q(TBlock_H_Waver);
		TIMER_P(TBlock_H_Block);
		// all rows should be in cache now
		rowtoblock8(bptr,(const int **)(rows + y-1),w8);	// blocks <- rows
		TIMER_Q(TBlock_H_Block);
		y += 8;
		bptr += stride8;
	}

	TIMER_P(TBlock_H_SpinUpDown);

	for(;y<endy;y++)
	{
		waver(rows[y-1],rows[y],w);		// workrow <- rows[y]
		rowtoblock(bptr,rows[y-1],w8);	// block   <- workrow
		bptr += 8;	// point to next line in blocks !
	}

	TIMER_Q(TBlock_H_SpinUpDown);

	TIMER_Q(TBlock_H);
}

void untHb(int starty,int endy,int w,tblockInfo * tbi)
{
int *workrow,*row;
int y8,yi,y,w8;
int * bptr;
int stride8,**rows,*blocks;
jeWaveletFunc waver;

	TIMER_P(TBlock_HB);

	waver	= tbi->waver;
	stride8 = tbi->stride8;
	blocks	= tbi->blocks;
	rows	= tbi->rows;

	// (brow+row) -> (block row)

	// <> we should make a roll-8 version, 

	w8 = (w+7)>>3;
	y8 = (starty>>3);
	yi = (starty&7);
	bptr = blocks + stride8*y8 + 8*yi;
	for(y=starty;y<endy;y++)
	{
		row = rows[y];
		workrow = rows[y-1];
		cachetouch_w(row,w8);
		blocktorow(row,bptr,w>>1);	// get the LL out of blocks
									// the LH part is already in row[]

		waver(workrow,row,w);		// workrow <- row		; write to the row we just read from
		rowtoblock(bptr,workrow,w8);// block   <- workrow	; back in the blocks

		yi++;
		bptr += 8;	// point to next line in blocks !
		if ( yi == 8 )
		{
			yi = 0; y8 ++;
			bptr = blocks + stride8*y8;
		}
		assert(y8 == ((y+1)>>3));
	}
	
	TIMER_Q(TBlock_HB);
}

void untV2(int w,int h,tblockInfo * tbi)
{
int x8,xi,y;
int * bptr;
int stride8,**rows,*blocks;
jeWaveletFunc waver;

	//  this is just bad:
	// TBlock_V_UnBlock     : 0.009621 : 11.4 %
	// TBlock_V_Waver       : 0.007168 : 8.5 %

	TIMER_P(TBlock_V);

	waver	= tbi->waver;
	stride8 = tbi->stride8;
	blocks	= tbi->blocks;
	rows	= tbi->rows;

	// this is only done once, at the very end
	//	 at this point all our data is in the blocks,
	//	 so we can trash anything in the rows

	// (bcolumn) -> (row)

	x8 = w>>3;
	xi = w&7;
	bptr = blocks;
	y = -1;
	
	TIMER_P(TBlock_V_UnBlock);
	while(x8--)
	{
		blockvtorow8(rows+y,bptr,h,stride8);
		bptr += 64;		// step past 8 columns in blocks !
		y += 8;
	}
	TIMER_Q(TBlock_V_UnBlock);
	
	while(xi--)
	{
		cachetouch_w(rows[y],h>>3);
		blockvtorow(rows[y],bptr,h,stride8);
		bptr ++;		// point to next column in blocks !
		y++;
	}

	TIMER_P(TBlock_V_Waver);
	cachetouch_w(rows[w-1],h>>3);
	for(y = w - 1;(y>=0);y--)
	{
		// this is cache-optimal : we read from row (y) then write to row (y)
		waver(rows[y],rows[y-1],h);
	}
	TIMER_Q(TBlock_V_Waver);

	TIMER_Q(TBlock_V);
}

void untV3(int w,int h,tblockInfo * tbi)
{
int x8,xi,y,i;
int * bptr;
int stride8,**rows,**trows,*blocks;
jeWaveletFunc waver;

	// The Waver is slow cuz we're writing to memory not in cache at all
	//  on a K7 or P3, we the cachetouch_w fixes everything
	// TBlock_V_UnBlock     : 0.006368 : 7.4 %
	// TBlock_V_Waver       : 0.007712 : 9.0 %

	TIMER_P(TBlock_V);

	waver	= tbi->waver;
	stride8 = tbi->stride8;
	blocks	= tbi->blocks;
	rows	= tbi->rows;
	trows	= tbi->trows;

	// this is only done once, at the very end
	//	 at this point all our data is in the blocks,
	//	 so we can trash anything in the rows

	// (bcolumn) -> (row)

	x8 = w>>3;
	xi = w&7;
	bptr = blocks;
	y = 0;
	
	while(x8--)
	{
	TIMER_P(TBlock_V_UnBlock);
		blockvtorow8(trows,bptr,h,stride8);
	TIMER_Q(TBlock_V_UnBlock);
	TIMER_P(TBlock_V_Waver);
		for(i=0;i<8;i++)
		{
			cachetouch_w(rows[y+i],h>>3);
			waver(rows[y+i],trows[i],h);
		}
	TIMER_Q(TBlock_V_Waver);
		bptr += 64;		// step past 8 columns in blocks !
		y += 8;
	}
	
	cachetouch_w(trows[0],h>>3);
	while(xi--)
	{
		blockvtorow(trows[0],bptr,h,stride8);
		waver(rows[y],trows[0],h);
		bptr ++;		// point to next column in blocks !
		y++;
	}

	TIMER_Q(TBlock_V);
}

void untV4(int w,int h,tblockInfo * tbi)
{
int x8,xi,y,i;
int * bptr;
int stride8,**rows,*workrow,*blocks;
int *zrows[8];
jeWaveletFunc waver;

	// well, we sped up the Waver, but the UnBlock still hurts
	// TBlock_V_UnBlock     : 0.009015 : 11.1 %
	// TBlock_V_Waver       : 0.004652 : 5.7 %

	TIMER_P(TBlock_V);

	waver	= tbi->waver;
	stride8 = tbi->stride8;
	blocks	= tbi->blocks;
	rows	= tbi->rows;
	workrow = tbi->rows[-1];

	// this is only done once, at the very end
	//	 at this point all our data is in the blocks,
	//	 so we can trash anything in the rows

	// (bcolumn) -> (row)

	x8 = w>>3;
	xi = w&7;
	bptr = blocks;
	y = 0;
	
	zrows[0] = workrow;
	cachetouch_w(workrow,h>>3);
	while(x8--)
	{
		for(i=1;i<8;i++)
			zrows[i] = rows[y+i-1];
	TIMER_P(TBlock_V_UnBlock);
		blockvtorow8(zrows,bptr,h,stride8);
	TIMER_Q(TBlock_V_UnBlock);
	TIMER_P(TBlock_V_Waver);
		for(i=7;i>=0;i--)
		{
			cachetouch_w(rows[y+i],h>>3);
			// write to i, read from (i-1), step backwards; this is cache-optimal
			waver(rows[y+i],zrows[i],h);
		}
	TIMER_Q(TBlock_V_Waver);
		bptr += 64;		// step past 8 columns in blocks !
		y += 8;
	}
	
	cachetouch_w(workrow,h>>3);
	while(xi--)
	{
		blockvtorow(workrow,bptr,h,stride8);
		waver(rows[y],workrow,h);
		bptr ++;		// point to next column in blocks !
		y++;
	}

	TIMER_Q(TBlock_V);
}

void untVb3(int w,int h,tblockInfo * tbi)
{
int x8,xi,y;
int * bptr;
int stride8,**trows,*blocks;
jeWaveletFunc waver;

	TIMER_P(TBlock_VB);

	waver	= tbi->waver;
	stride8 = tbi->stride8;
	blocks	= tbi->blocks;
	trows	= tbi->trows;
	// (bcolumn) -> (bcolumn)

	// read the whole set of bcolumns out to rows,
	// then wavelet all the rows
	// then read 'em back to bcolumns

	x8 = w>>3;
	bptr = blocks;
	while(x8--)
	{
		// read 8 columns out to rows
		blockvtorow8(trows+1,bptr,h,stride8);
		// wave 'em, shifting down one
		cachetouch_w(trows[0],h>>3);
		for(y=1;y<9;y++)
			waver(trows[y-1],trows[y],h);

		// now put 'em back in blocks :
		rowtoblockv8(bptr,(const int **)trows,h,stride8);
		bptr += 64;		// step past 8 columns in blocks !
	}

	// spin down:

	xi = w&7;
	cachetouch_w(trows[1],h>>3);
	for(y=0;y<xi;y++)
	{
		blockvtorow(trows[0],bptr,h,stride8);
		waver(trows[1],trows[0],h);
		rowtoblockv(bptr,trows[1],h,stride8);
		bptr ++;		// point to next column in blocks !
	}
	
	TIMER_Q(TBlock_VB);
}

/*}{*** IT ********/

void untransformBlocked(image *im,int levels,jeWaveletFunc waver,jeBoolean doLHs)
{
int p,l;
tblockInfo tbi;
int * blocks;
int width8,height8,stride8,w,h;
int ** rows;
int ** trows;
int imw,imh,ims;

	Log_Printf("Doing untransformBlocked\n");

	TIMER_P(TBlock_All);

	imw = im->width;
	imh = im->height;
	ims = im->stride;
	width8  = (imw + 7)>>3;
	height8 = (imh + 7)>>3;

	stride8 = (((ims + 7)>>3)<<6) + 3;

	w = (ims + 7)>>3;
	h = w<<3;

	TIMER_P(TBlock_Ram);

	blocks = (int *)jeRam_Allocate(sizeof(int)*(stride8*w + 9*h));
	assert(blocks);

	TIMER_Q(TBlock_Ram);

	trows = tbi.trows;
	trows[0] = blocks + stride8*w;
	for(l=1;l<9;l++)
		trows[l] = trows[l-1] + h;

	tbi.blocks  = blocks;
	tbi.stride8 = stride8;
	tbi.waver   = waver;

pushTSC();

	for(p=0;p<(im->planes);p++) 
	{
		rows = im->data[p];
		tbi.rows = rows;

		for (l = levels-1; l >= 0; l--) 
		{
			w = imw >> l;
			h = imh >> l;

			/* untransform into blocks */

			//<> seems a shame not to use the blocks to transpose
			if ( doLHs )
			{
				TIMER_P(TBlock_Transpose);
				transposeHL(im,p,l);
				TIMER_Q(TBlock_Transpose);
			}

			if ( l == (levels - 1) )
			{
				untH2(0,h,w,&tbi);
			}
			else
			{
				untHb(0,h>>1,w,&tbi);
				untH2(h>>1,h,w,&tbi);
			}	

			/* Columns */

			if ( l == 0 )
			{
				untV4(w,h,&tbi);
			}
			else
			{
				untVb3(w,h,&tbi);
			}

			assert(jeRam_IsValidPtr(blocks));
		}
	}

showPopTSC("untrans blocked");

	TIMER_P(TBlock_Ram);

	jeRam_Free(blocks);

	TIMER_Q(TBlock_Ram);

	// we did a transpose !
	swapints(im->width,im->height);

	TIMER_Q(TBlock_All);
}

void TBlock_DoReport(void)
{
	TIMER_REPORT(TBlock_All);
	TIMER_REPORT(TBlock_Ram);
	TIMER_REPORT(TBlock_Transpose);
	TIMER_REPORT(TBlock_H);
	TIMER_REPORT(TBlock_HB);
	TIMER_REPORT(TBlock_V);
	TIMER_REPORT(TBlock_VB);
	TIMER_REPORT(TBlock_V_UnBlock);
	TIMER_REPORT(TBlock_V_Waver);
	TIMER_REPORT(TBlock_H_SpinUpDown);
	TIMER_REPORT(TBlock_H_Waver);
	TIMER_REPORT(TBlock_H_Block);
}

/*}*** EOF ********/
