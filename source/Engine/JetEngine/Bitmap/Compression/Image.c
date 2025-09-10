/****************************************************************************************/
/*  IMAGE.C                                                                             */
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

//#define EXTEND_ANTISYM // else symmetric; symmetric seems better

#define ZERO_SAFE_ZONE (3)	// 3 seems perfect
							// it's of course transform-dependent,
							//	but daub97 is the largest filter we use..

//hareck ; aim for rmse = 5
// with daub97
// safe zone 2 : 1.206 bpp
// safe zone 3 : 1.045 bpp
// safe zone 4 : 1.046 bpp

#include <string.h>
#include <math.h>

#include "Utility.h"
#include "Image.h"
#include "Log.h"
#include "Cpu.h"
#include "IntMath.h"

#ifdef _DEBUG
#define DebugLog(x)	x
#else
#define DebugLog(x)
#endif

void transposeImage(image *im)
{
int p,x,y,l;
int **rows,z;

	l = max(im->width,im->height);

	for(p=0;p<im->planes;p++) 
	{
		rows = im->data[p];
		for(y=0;y<l;y++) 
		{
			for(x=y+1;x<l;x++) 
			{
				z			= rows[x][y];
				rows[x][y]	= rows[y][x];
				rows[y][x]	= z;
			}
		}
	}

	swapints(im->width,im->height);
}

jeBoolean extendImage(image *im,int fmw,int fmh)
{
int x,y,ex,ey;
int **rows;
int *ptr;
int p;

	ex = im->width - fmw;
	ey = im->height - fmh;

	if ( ex == 0 && ey == 0 )
		return JE_TRUE;

	assert( ex < fmw );
	assert( ey < fmh );

#ifdef EXTEND_ANTISYM //{
	for(p=0;p<im->planes;p++)
	{
	int pivot;
		rows = im->data[p];

		for(y=0;y < fmh; y++)
		{
			ptr = rows[y] + fmw;
			pivot = 2*ptr[-1];
			for( x = 0; x < ex ; x++ )
			{
				ptr[x] = pivot - ptr[-2-x];
			}
		}

		for(x=0;x < im->width; x++)
		{
			pivot = 2*rows[ fmh - 1][x];
			for(y=0;y<ey;y++)
			{
				rows[ fmh + y][x] = pivot - rows[ fmh - 2 - y][x];
			}
		}
	}
#else // }{ extend symmetric
	for(p=0;p<im->planes;p++)
	{
		rows = im->data[p];

		for(y=0;y < fmh; y++)
		{
			ptr = rows[y] + fmw;
			for( x = 0; x < ex ; x++ )
			{
				ptr[x] = ptr[-1-x];
			}
		}

		for(x=0;x < im->width; x++)
		{
			for(y=0;y<ey;y++)
			{
				rows[ fmh + y][x] = rows[ fmh - 1 - y][x];
			}
		}
	}
#endif //}

	if ( im->alpha )
	{
	uint8 * aptr;

		for(y=0;y < fmh; y++)
		{
			aptr = im->alpha + fmw + y * (im->stride);
			for( x = 0; x < ex ; x++ )
			{
				aptr[x] = aptr[-1-x];
			}
		}

		for(x=0;x < im->width; x++)
		{
			aptr = im->alpha + x + fmh * (im->stride);
			for(y=0;y<ey;y++)
			{
				aptr[ y * im->stride ] = aptr[ (- 1 - y) * im->stride ];
			}
		}
	}

return JE_TRUE;
}

jeBoolean zeroExtendedImage(image *im,int fmw,int fmh,int levels)
{
int y,ew,eh,w,h,gw,gh;
int **rows;
int *ptr1,*ptr2,*ptr3;
int p,level;
DebugLog(int zeroed = 0);

	if ( fmw == im->width && fmh == im->height )
		return JE_TRUE;

	for(p=0;p<im->planes;p++)
	{
		rows = im->data[p];
		for(level=1;level<=levels;level++)
		{
			w = (im->width)>>level;
			h = (im->height)>>level;
			ew = (im->width - fmw)>>level;
			eh = (im->height - fmh)>>level;

			// the wavelet reconstruction might walk over this far, so
			//  don't zero these...
			ew -= ZERO_SAFE_ZONE; eh -= ZERO_SAFE_ZONE;

			if ( ew <= 0 ) ew = 0;
			if ( eh <= 0 ) eh = 0;
			if ( ew == 0 && eh == 0 )
				continue;
			gw = w - ew;
			gh = h - eh;

			DebugLog( zeroed += gh * ew + eh * w );

			if ( ew > 0 )
			{
				for(y=0;y<gh;y++)
				{
					ptr1 = rows[y] + w + gw;
					ptr2 = rows[y+h] + gw;
					ptr3 = rows[y+h] + w + gw;
					memset(ptr1,0,sizeof(int)*ew);
					memset(ptr2,0,sizeof(int)*ew);
					memset(ptr3,0,sizeof(int)*ew);
				}
			}
			if ( eh > 0 )
			{
				for(y=gh;y<h;y++)
				{
					ptr1 = rows[y] + w;
					ptr2 = rows[y+h];
					ptr3 = rows[y+h] + w;
					memset(ptr1,0,sizeof(int)*w);
					memset(ptr2,0,sizeof(int)*w);
					memset(ptr3,0,sizeof(int)*w);
				}
			}
		}
	}

	DebugLog( Log_Printf("extend zeroed : %d\n",zeroed) );

return JE_TRUE;
}

int sample8(uint8 * plane,int s,int x,int y,int z)
{
int A;
uint8 *row,*ptr;
	A = 0;
	row = plane + x + y * s;
	for(y=0;y<z;y++)
	{
		ptr = row; row += s;
		for(x=0;x<z;x++)
		{
			A += (*ptr++);
		}
	}
return A;
}

jeBoolean zeroAlphaImage(image *im,int levels)
{
DebugLog(int zeroed = 0);

//return JE_TRUE; // @@
	// the image has already been transformed

	if ( im->planes == 4 )
	{
	int ** rows,**arows,*row1,*row2,*row3,*arow;
	int x,y,p,l,w,h,ay,ax;

		// the alpha's been transformed too;
		// we just go off the LL band of the alpha :^(

		arows = im->data[3];
		for(p=0;p<3;p++)
		{
			rows = im->data[p];
			for(l=0;l<(levels-2);l++)
			{
				w = (im->width )>>(levels - l);
				h = (im->height)>>(levels - l);

				for(y=0;y<h;y++)
				{
					ay = y >> l;
					arow = arows[ay];
					row1 = rows[y] + w;
					row2 = rows[y + h];
					row3 = rows[y + h] + w;
					for(x=0;x<w;x++)
					{
						ax = x >> l;
						if ( arow[ax] < 10 )
						{
							DebugLog( zeroed += (row1[x] ? 1 : 0) );
							DebugLog( zeroed += (row2[x] ? 1 : 0) );
							DebugLog( zeroed += (row3[x] ? 1 : 0) );
							row1[x] = row2[x] = row3[x] = 0;
						}
					}
				}
			}
		}

		DebugLog( Log_Printf("alpha zeroed : %d\n",zeroed) );
	}
	else if ( im->alpha )
	{
	int ** rows,*row1,*row2,*row3;
	int x,y,p,l,w,h,s,ay,ax,A;

		// the alpha plane has *NOT* been transformed;
		// for each pixel we have to look in the alpha mask
		//	in a region of the parent
		// (this is not totally accurate; we lose about 1 PSNR, but so what?)

		for(p=0;p<(im->planes);p++)
		{
			rows = im->data[p];
			for(l=0;l<(levels-2);l++)
			{
				s = levels - l;
				w = (im->width )>>s;
				h = (im->height)>>s;

				for(y=0;y<h;y++)
				{
					ay = max(0,(y-ZERO_SAFE_ZONE)) << s;
					row1 = rows[y] + w;
					row2 = rows[y + h];
					row3 = rows[y + h] + w;
					for(x=0;x<w;x++)
					{
					int z;
						ax = max(0,(x-ZERO_SAFE_ZONE)) << s;
						z = min(min((ZERO_SAFE_ZONE+ZERO_SAFE_ZONE)<<s,(im->width - ax)),(im->height - ay));
						A = sample8(im->alpha,im->stride,ax,ay,z);
						if ( A == 0 )
						{
							DebugLog( zeroed += (row1[x] ? 1 : 0) );
							DebugLog( zeroed += (row2[x] ? 1 : 0) );
							DebugLog( zeroed += (row3[x] ? 1 : 0) );
							row1[x] = row2[x] = row3[x] = 0;
						}
					}
				}
			}
		}
		
		DebugLog( Log_Printf("alpha zeroed : %d\n",zeroed) );
	}

return JE_TRUE;
}

image * newImage(int width, int height,int planes)
{
int p,y;
image * im;
int **rows;
int len;

	if ( (im = (image *)new(image)) == NULL ) return NULL;

	len = max(width,height);

#if 1 // @@
	// changes UNT time from 0.85 to 0.78 !!
	y = intlog2(len);
	if ( len == (1<<y) )
	{
		switch(y)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			break;
		case 4:
			len = 17;
			break;
		case 5:
			len = 37;
			break;
		case 6:		   
			len = 67;
			break;
		case 7:
			len = 131;
			break;
		case 8:
			len = 257;
			break;
		case 9:
			len = 521;
			break;
		case 10:
			len = 1031;
			break;
		default:
			len ++;
			break;
		}
	}
#endif

	im->width  = width;
	im->height = height;
	im->stride = len;
	im->planes = planes;
	im->plane_size = len*len;
	im->plane_bytes = (im->plane_size)*sizeof(int);
	im->tot_bytes = im->plane_bytes * planes;
	im->tot_size = im->plane_size * planes;

	if ( (im->data = (int ***)newarray(int **,planes)) == NULL ) 
	{
		freeImage(im); return NULL;
	}

	for(p=0;p<planes;p++) 
	{

		if ( (im->data[p] = (int **)newarray(int *,len+2)) == NULL ) 
		{
			freeImage(im); return NULL;
		}

		im->data[p] = &(im->data[p][1]);
		rows = im->data[p];

		if ( (rows[0] = (int *)jeRam_Allocate(im->plane_bytes + 2*len*sizeof(int))) == NULL )
		{
			freeImage(im); return NULL;
		}

		for (y = 1; y <= len; y++)
		    rows[y] = rows[y-1] + len;
		
		rows[-1] = rows[len];
  	}

return im;
}

image * copyImage(image *im)
{
image *newImage;
int p;

	if ( (newImage = newImageFrom(im)) == NULL )
		return NULL;

	for(p=0;p<im->planes;p++) 
	{
		fastmemcpy((char *)newImage->data[p][0],(const char *)im->data[p][0],im->plane_bytes);
	}

	if ( im->alpha )
		fastmemcpy((char *)newImage->alpha,(const char *)im->alpha,im->plane_size);

return newImage;
}

image * newImageAlpha(int width, int height,int planes,jeBoolean alphaIsBoolean)
{
image * im;
	if ( (im = newImage(width,height,planes)) == NULL ) 
		return NULL;
	if ( (im->alpha = (uint8*)jeRam_Allocate(im->plane_size)) == NULL )
	{
		freeImage(im);
		return NULL;
	}
	im->alphaIsBoolean = alphaIsBoolean;
return im;
}

image * newImageFrom(image *im)
{
image *newImagePtr;

	if ( im->alpha )
	{
		if ( (newImagePtr = newImageAlpha(im->width,im->height,im->planes,im->alphaIsBoolean)) == NULL )
			return NULL;
	}
	else
	{
		if ( (newImagePtr = newImage(im->width,im->height,im->planes)) == NULL )
			return NULL;
	}

return newImagePtr;
}

void freeImage(image *im) 
{
	if (im ) 
	{
		if ( im->data ) 
		{
			int p;
			for(p=0;p<im->planes;p++) 
			{
				if ( im->data[p] ) 
				{
					if (im->data[p][0] )
						jeRam_Free(im->data[p][0]);
					im->data[p] = &(im->data[p][-1]);
					jeRam_Free(im->data[p]);
				}
			}
			jeRam_Free(im->data);
		}
		destroy( im->alpha );
		jeRam_Free(im);
	}
}

void zeroImage(image *im)
{
int p;

	for(p=0;p<im->planes;p++)
		fastmemclear((char *)im->data[p][0],im->plane_bytes);

	if ( im->alpha )
		fastmemclear((char *)im->alpha,im->plane_size);
}

void insertImageAlpha(image *im,uint8 *alpha)
{
	if ( im->alpha )
	{
		fastmemcpy((char *)im->alpha,(const char *)alpha,im->plane_size);
	}
}

void patchImage(image *fm,image *to,int fmx,int fmy,int w,int h,int tox,int toy)
{
int x,y,p;
int **fmrows,**torows;
int *fmrow,*torow;

	for(p=0;p<fm->planes;p++) 
	{
		fmrows = fm->data[p];
		torows = to->data[p];
		for(y=0;y<h;y++) 
		{
			torow = (torows[y+toy])+tox;
			fmrow = (fmrows[y+fmy])+fmx;
			for(x=w;x--;) *torow++ = *fmrow++;
		}
	}
	if ( fm->alpha && to->alpha )
	{
	uint8 *fmptr,*toptr;
	int fmpad,topad;

		fmptr = fm->alpha + (fmy * fm->width) + fmx;
		toptr = to->alpha + (toy * to->width) + tox;
		fmpad = fm->width - w;
		topad = to->width - w;
		for(y=0;y<h;y++)
		{
			for(x=w;x--;) *toptr++ = *fmptr++;
			toptr += topad;
			fmptr += fmpad;
		}	
	}
}

#if 0 // @@ not used;

#define MAX_DIFF 256
#define PSNR_MAX 	(48.165)	//(10*log10(256^2))

double PSNR(double rmse)
{
return ( PSNR_MAX - 20*log10(rmse) );
}

double imageRMSE(image *src,image *comp)
{
int diffs[MAX_DIFF+1];
int diff,i,totsq,pnum;
double mse;

	jeCPU_PauseMMX();	// !! be careful of MMX used elsewhere!

	memclear(diffs,(MAX_DIFF+1)*sizeof(long));

	for(pnum=0;pnum<(src->planes);pnum++) 
	{
		int *rptr,*vptr;
		rptr = src->data[pnum][0]; 
		vptr = comp->data[pnum][0];
		for(i=(src->plane_size);i--;) 
		{
		int d;
			d = *rptr++ - *vptr++;
			diff = abs(d);
			if ( diff > MAX_DIFF ) diff = MAX_DIFF;
			diffs[diff] ++;
		}
	}

	totsq = 0;
	for(i=1;i<=MAX_DIFF;i++) 
	{
		if ( diffs[i] > 0 ) 
		{
			totsq += i*i * diffs[i];
		}
	}

	mse = (float)totsq/(src->tot_size);

	jeCPU_ResumeMMX();

return sqrt(mse);
}

#endif

