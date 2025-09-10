/****************************************************************************************/
/*  CODEPAL.C                                                                           */
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
#include "CodePal.h"
#include "Bitmap.h"
#include "VFile.h"
#include "arithc.h"
#include "CodeUtil.h"
#include "YUV.h"
#include "Log.h"
#include "o0coder.h"
#include "huffa.h"

//#define OZERO
#define LAP
//#define HUFF

// LAP : 531
// Ozero : 548
// Huff : 552
// Rkive : ~ 526
// Mult : 535 @ (5,1)


/**/
void cu_Laplace_PutScaleFromAverage(arithInfo * ari,int avg,int *pscale);
void cu_Laplace_GetScale(arithInfo * ari,int *pscale);
void cu_putLaplaceSigned_ari(int val, arithInfo * ari, int r, int max);
int cu_getLaplaceSigned_ari(arithInfo * ari, int r, int max);
int cu_Laplace_AverageToScale(int avg);
int cu_LaplaceComputeMax_ari(int r);
/**/

#if 0 // {
static int mscale = 5,mmult = 1;
//#define MULT_SCALE	(mscale)
//#define MULT_MULT	(mmult)

void codePal_Optimize(const jeBitmap_Palette * P)
{
jeVFile * F;
int bmult,bscale,blen,zOutLen;

#ifndef MULT_SCALE
	return;
#endif

	blen = 999;
	bmult=bscale=0;

	for(mmult=1;mmult<16;mmult++)
	{
		for(mscale=1;mscale<16;mscale++)
		{
			F = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,"r:\\z",NULL,JE_VFILE_OPEN_CREATE);
			assert(F);

			codePal_Write(P,F,&zOutLen);

			jeVFile_Close(F);

			if ( zOutLen < blen )
			{
				blen = zOutLen;
				bmult = mmult;
				bscale = mscale;
			}
		}
	}

	mmult = bmult;
	mscale = bscale;
	Log_Printf("best : len = %d, scale = %d , mult = %d\n",blen,bscale,bmult);
}
#endif // }

jeBoolean codePal_Write(const jeBitmap_Palette * P,jeVFile * F,int *pWroteLen)
{
int Size;
jePixelFormat Format;
void *Data;
int R,G,B,A;
int Ya[256],Ua[256],Va[256],Aa[256];
int *Yp,*Up,*Vp,*Ap;
uint8 * DataPtr;
int s,i,scale,max;
uint8 OutBuf[2048];
arithInfo * ari;
jeBoolean HasAlpha;
int WroteLen;
uint16 OutLen;
#ifdef OZERO
ozero * oz;
#endif

	assert(P && F);

	if ( ! jeBitmap_Palette_Lock((jeBitmap_Palette *)P,&Data,&Format,&Size) )
		return JE_FALSE;

	assert( Size <= 256 );

	HasAlpha = jePixelFormat_HasAlpha(Format);

	DataPtr = (uint8*)Data;
	Yp = Ya; Up = Ua; Vp = Va; Ap = Aa;
	for(s=Size;s--;)
	{
		jePixelFormat_GetColor(Format,&DataPtr,&R,&G,&B,&A);
		RGBi_to_YUVi(R,G,B,Yp++,Up++,Vp++);
		*Ap++ = A;
	}

	if ( ! jeBitmap_Palette_UnLock((jeBitmap_Palette *)P) )
		return JE_FALSE;

	for(s=Size-1;s>0;s--)
	{
		// <> could use a gradient-adjusted DPCM predictor
		Ya[s] -= Ya[s-1];
		Ua[s] -= Ua[s-1];
		Va[s] -= Va[s-1];
		Aa[s] -= Aa[s-1];
	}

	// send first byte of each raw
	WroteLen = 0;

	assert( isinrange(Ya[0],0,255) );
	assert( isinrange(Ua[0],0,255) );
	assert( isinrange(Va[0],0,255) );
	assert( isinrange(Aa[0],0,255) );
	OutBuf[0] = (uint8) Ya[0];
	OutBuf[1] = (uint8) Ua[0];
	OutBuf[2] = (uint8) Va[0];
	OutBuf[3] = (uint8) Aa[0];
	if ( HasAlpha )
	{
		if ( ! jeVFile_Write(F,OutBuf,4) )
			return JE_FALSE;
		WroteLen += 4;
	}
	else
	{
		if ( ! jeVFile_Write(F,OutBuf,3) )
			return JE_FALSE;
		WroteLen += 3;
	}

#ifdef LAP
	{
	int tot,count;
		tot = 0; count  =0;
		for(s=1;s<Size;s++)
		{
			tot += abs(Ya[s]);
			tot += abs(Ua[s]);
			tot += abs(Va[s]);
			if ( HasAlpha )
			{
				tot += abs(Aa[s]);
				count += 4;
			}
			else
			{
				count += 3;
			}
		}

		scale = (tot + count - 1) / count;
	}
#endif

#ifdef HUFF //{
	{
	uint8 WorkBuf[1024],*WorkPtr;
	int WorkLen,z;

	WorkLen = 0;
	WorkPtr = WorkBuf;

	for(i=0; (i<3) || (i==3 && HasAlpha);i++)
	{
	int * ptr,v;
		switch(i)
		{
			case 0: ptr = Ya+1; break;
			case 1: ptr = Ua+1; break;
			case 2: ptr = Va+1; break;
			case 3: ptr = Aa+1; break;
		}

		for(s=(Size-1);s--;)
		{
			v = (*ptr++) + 128;
			if ( v < 0 ) v += 256;
			if ( v > 255 ) v-= 256;
			*WorkPtr++ = v;
		}
	}

	WorkLen = WorkPtr - WorkBuf;
	O0HuffArray(WorkBuf,WorkLen,OutBuf,&z,JE_TRUE);
	OutLen = z;

	}
#else //}{ not huff, ari

	if ( ! (ari = arithInit()) )
		return JE_FALSE;

	arithEncodeInit(ari,OutBuf);

#ifdef LAP
	cu_Laplace_PutScaleFromAverage(ari,scale,&scale);
	max = cu_LaplaceComputeMax_ari(scale);
#endif
#ifdef OZERO
	oz = ozeroCreate(ari,256);
#endif

	for(i=0; (i<3) || (i==3 && HasAlpha);i++)
	{
	int * ptr;
		switch(i)
		{
			case 0: ptr = Ya+1; break;
			case 1: ptr = Ua+1; break;
			case 2: ptr = Va+1; break;
			case 3: ptr = Aa+1; break;
		}

		for(s=(Size-1);s--;)
		{
#ifdef LAP
			cu_putLaplaceSigned_ari(*ptr++, ari, scale, max );
#else
#ifdef OZERO
			{
			int v;
				v = *ptr++;
				ozeroEncode(oz,abs(v));
				if ( v )
				{
					arithEncBitRaw(ari,(v<0)?1:0);
				}
			}
#else
			cu_putMultingSigned_ari(*ptr++, ari, MULT_SCALE, MULT_MULT);
#endif
#endif
		}
	}

#ifdef OZERO
	ozeroFree(oz);
#endif

	OutLen = (uint16)arithEncodeDone(ari);

	arithFree(ari);

#endif //ari }

	Log_Printf("palette : 768 -> %d\n",OutLen);

	if ( ! jeVFile_Write(F,&OutLen,sizeof(OutLen)) )
		return JE_FALSE;
	WroteLen += sizeof(OutLen);
	if ( ! jeVFile_Write(F,OutBuf,OutLen) )
		return JE_FALSE;
	WroteLen += OutLen;

	*pWroteLen = WroteLen;

return JE_TRUE;
}

jeBoolean codePal_Read(jeBitmap_Palette * P,jeVFile * F)
{
int Size;
jePixelFormat Format;
void *Data;
jeBitmap_Info Info;
int R,G,B;
int Ya[256],Ua[256],Va[256],Aa[256];
int *Yp,*Up,*Vp,*Ap;
uint8 * DataPtr;
int s,i,scale,max;
uint8 OutBuf[2048];
uint16 OutLen;
arithInfo * ari;
jeBoolean HasAlpha;

	assert(P && F);

	if (! jeBitmap_Palette_GetInfo(P,&Info) )
		return JE_FALSE;

	Size = Info.Width;
	assert( Size <= 256 );

	HasAlpha = jePixelFormat_HasAlpha(Info.Format);

	// get first byte of each raw
	if ( HasAlpha )
	{
		if ( ! jeVFile_Read(F,OutBuf,4) )
			return JE_FALSE;
	}
	else
	{
		if ( ! jeVFile_Read(F,OutBuf,3) )
			return JE_FALSE;
		OutBuf[3] = 255;
	}
	Ya[0] = OutBuf[0];
	Ua[0] = OutBuf[1];
	Va[0] = OutBuf[2];
	Aa[0] = OutBuf[3];

	if ( ! jeVFile_Read(F,&OutLen,sizeof(OutLen)) )
		return JE_FALSE;
	if ( ! jeVFile_Read(F,OutBuf,OutLen) )
		return JE_FALSE;

	if ( ! (ari = arithInit()) )
		return JE_FALSE;

	arithDecodeInit(ari,OutBuf);

#ifndef MULT_SCALE
	cu_Laplace_GetScale(ari,&scale);
	max = cu_LaplaceComputeMax_ari(scale);
#endif

	for(i=0; (i<3) || (i==3 && HasAlpha);i++)
	{
	int * ptr;
		switch(i)
		{
			case 0: ptr = Ya+1; break;
			case 1: ptr = Ua+1; break;
			case 2: ptr = Va+1; break;
			case 3: ptr = Aa+1; break;
		}

		for(s=(Size-1);s--;)
		{
#ifndef MULT_SCALE
			*ptr++ = cu_getLaplaceSigned_ari(ari, scale, max );
#else
			*ptr++ = cu_getMultingSigned_ari(ari, MULT_SCALE, MULT_MULT);
#endif
		}
	}

	arithDecodeDone(ari);

	arithFree(ari);

	// un-DPCM

	for(s=1;s<Size;s++)
	{
		Ya[s] += Ya[s-1];
		Ua[s] += Ua[s-1];
		Va[s] += Va[s-1];
		Aa[s] += Aa[s-1];
	}

	// fill out the palette

	if ( ! jeBitmap_Palette_Lock((jeBitmap_Palette *)P,&Data,&Format,&Size) )
		return JE_FALSE;

	DataPtr = (uint8*)Data;
	Yp = Ya; Up = Ua; Vp = Va; Ap = Aa;
	for(s=Size;s--;)
	{
		YUVi_to_RGBi(*Yp++,*Up++,*Vp++,&R,&G,&B);
		jePixelFormat_PutColor(Format,&DataPtr,R,G,B,*Ap++);
	}

	if ( ! jeBitmap_Palette_UnLock((jeBitmap_Palette *)P) )
		return JE_FALSE;

   return JE_TRUE;
}

/****************************/

#include <math.h>

#define ONE_SHIFT	(13)
#define ONE			(1<<ONE_SHIFT)

int cu_LaplaceComputeMax_ari(int r)
{
int low,prob;
int val;

	val = 256;
	
	low = 0;
	prob = ONE - r;
	assert( r > 1 && r < ONE );
	assert( prob < ONE && prob > 0 );
	while(val)
	{
		low += prob;
		prob = (prob * r) >> ONE_SHIFT;
		if ( prob < 1 ) prob = 1;
		val--;
	}

return low + prob;
}

void cu_putLaplaceSigned_ari(int sval, arithInfo * ari, int r, int max)
{
int low,prob;
int val;

	val = abs(sval);
	
	low = 0;
	prob = ONE - r;
	assert( prob < ONE && prob > 0 );
	while(val)
	{
		low += prob;
		prob = (prob * r) >> ONE_SHIFT;
		if ( prob < 1 ) prob = 1;
		val--;
	}

	assert( (low+prob) <= max );
	arithEncode(ari,low,low+prob,max);
	
	if ( sval )
	{
		arithEncBitRaw(ari,sval < 0 ? 1 : 0);
	}
}

int cu_getLaplaceSigned_ari(arithInfo * ari, int r, int max)
{
int got,low,high,prob,val;
	
	got = arithGet(ari,max);

	val = 0;
	low = 0;
	prob = ONE - r;
	high = low+prob;
	assert( prob < ONE );
	while( got >= high )
	{
		prob = (prob * r) >> ONE_SHIFT;
		if ( prob < 1 ) prob = 1;
		low = high;
		high += prob;
		val ++;
	}

	assert( got >= low && got < high );
	assert( high <= max );
	arithDecode(ari,low,high,max);
	
	if ( val )
	{
		if ( arithDecBitRaw(ari) )
			val *= -1;
	}
return val;
}

void cu_Laplace_PutScaleFromAverage(arithInfo * ari,int avg,int *pscale)
{

	avg = abs(avg);	

	arithEncByteRaw(ari,avg);

	avg++;
	if ( avg < 2 ) avg = 2;
	*pscale = ONE - 1 - (ONE/avg);
}

void cu_Laplace_GetScale(arithInfo * ari,int *pscale)
{
int avg;

	avg = arithDecByteRaw(ari);

	avg++;
	if ( avg < 2 ) avg = 2;
	*pscale = ONE - 1 - (ONE/avg);
}
