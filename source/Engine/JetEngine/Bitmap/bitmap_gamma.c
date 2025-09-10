/****************************************************************************************/
/*  Bitmap_Gamma.c                                                                      */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description:  The Bitmap_Gamma_Apply function                                       */
/*					Fast Gamma correction routines for various pixel formats			*/
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
#include "Bitmap._h"
#include "Bitmap.__h"
#include "bitmap_gamma.h"
#include "PixelFormat.h"
#include "Errorlog.h"
#include <assert.h>
#include <math.h>

/*}{*******************************************************/

// this *MUST* be done with a semaphore;
//  we want these statics to stick around, and two threads
//	coming in here might want different gamma values, which
//	would mean different tables!

extern jeThreadQueue_Semaphore * Bitmap_Gamma_Lock;

static uint32	Gamma_Lut[256];
static uint32	Gamma_Lut_Inverse[256];
static jeFloat	ComputedGamma_Lut = 0.0f;

static uint16 	Gamma_565_RGB[1<<16];			// 128k !
static jeFloat	ComputedGamma_565_RGB = 0.0f;
static uint16	Gamma_4444_ARGB[1<<16];			// 128k !
static jeFloat	ComputedGamma_4444_ARGB = 0.0f;

/*}{*******************************************************/

void jeBitmap_Gamma_Compute_Lut(double Gamma);
void jeBitmap_GammaCorrect_Data_4444_ARGB(void * Bits,jeBitmap_Info * pInfo);
void jeBitmap_GammaCorrect_Data_565_RGB(void * Bits,jeBitmap_Info * pInfo);
jeBoolean jeBitmap_GammaCorrect_Data(void * Bits,jeBitmap_Info * pInfo, jeBoolean Invert);

/*}{*******************************************************/

jeBoolean jeBitmap_Gamma_Apply(jeBitmap * Bitmap,jeBoolean Invert)
{
jeBoolean Ret = JE_TRUE;
jeFloat Gamma;

	assert(Bitmap);

	Gamma = Bitmap->DriverGamma;
	if ( Gamma <= 0.1f ) // assume they meant 1.0f
		return JE_TRUE;

	// Gamma only works on driver data

	// do-nothing gamma:
	if ( fabs(Gamma - 1.0) < 0.1 )
		return JE_TRUE;

	if ( Bitmap->LockOwner )
		Bitmap = Bitmap->LockOwner;
	if ( Bitmap->LockCount || Bitmap->DataOwner )
		return JE_FALSE;

	if ( ! Bitmap->DriverHandle )	// nothing to do
		return JE_TRUE;

	assert(Bitmap_Gamma_Lock);
	jeThreadQueue_Semaphore_Lock(Bitmap_Gamma_Lock);

	if ( ComputedGamma_Lut != Gamma )
	{
		jeBitmap_Gamma_Compute_Lut(Gamma);
	}

	if ( jePixelFormat_HasPalette(Bitmap->DriverInfo.Format) )
	{
	jeBitmap_Palette *	Pal;
	jeBitmap_Info		PalInfo;
	void *	Bits;
	int		Size;
	jePixelFormat Format;
	
		// gamma correct the palette

		assert(Bitmap->DriverInfo.Palette);
		Pal = Bitmap->DriverInfo.Palette;

		if ( ! jeBitmap_Palette_Lock(Pal,&Bits,&Format,&Size) )
			goto fail;

		jeBitmap_Palette_GetInfo(Pal,&PalInfo);

		if ( ! jeBitmap_GammaCorrect_Data(Bits,&PalInfo,Invert) )
			Ret = JE_FALSE;

		jeBitmap_Palette_UnLock(Pal);
	}
	else
	{
	jeBitmap_Info	Info;
	void * 			Bits;
	int				mip,mipCount;

		assert( Bitmap->DriverInfo.MinimumMip == 0 );
		mipCount = Bitmap->DriverInfo.MaximumMip + 1;

		// work directly on the driver bits so that
		//	we don't get any DriverDataChanged or Modified[mip] flags !

		assert(Bitmap->Driver);
		assert(Bitmap->DriverHandle);

#if 0	// <> raw!

		for(mip=0;mip<mipCount;mip++)
		{
			Info = Bitmap->DriverInfo;
			if ( ! jeBitmap_MakeDriverLockInfo(Bitmap,mip,&Info) )
			{
				jeErrorLog_AddString(-1,"jeBitmap_Gamma_Apply : UpdateInfo failed", NULL);
				goto fail;
			}

			if ( ! Bitmap->Driver->THandle_Lock(Bitmap->DriverHandle,mip,&Bits) )
			{
				jeErrorLog_AddString(-1,"jeBitmap_Gamma_Apply : THandle_Lock", NULL);
				goto fail;
			}
			assert(Bits);

			if ( ! jeBitmap_GammaCorrect_Data(Bits,&Info,Invert) )
			{
				jeErrorLog_AddString(-1,"jeBitmap_Gamma_Apply : GammaCorrect_Data", NULL);
				Ret = JE_FALSE;
			}
			
			if ( ! Bitmap->Driver->THandle_UnLock(Bitmap->DriverHandle,mip) )
			{
				jeErrorLog_AddString(-1,"jeBitmap_Gamma_Apply : THandle_UnLock", NULL);
				goto fail;
			}
		}

#else
		{
		jeBitmap *Locks[8],*Lock;

		//if ( mipCount > 1 ) ; true, but pointless
		//	assert(mipCount == 4);

		if ( ! jeBitmap_LockForWrite(Bitmap,Locks,0,mipCount-1) )
		{
			jeErrorLog_AddString(-1,"jeBitmap_Gamma_Apply : LockforWrite failed", "");
			goto fail;
		}

		for(mip=0;mip<mipCount;mip++)
		{
			Lock = Locks[mip];
			
			if ( ! jeBitmap_GetInfo(Lock,&Info,0) )
			{
				jeErrorLog_AddString(-1,"jeBitmap_Gamma_Apply : GetInfo failed", "");
				goto fail;
			}

			Bits = jeBitmap_GetBits(Lock);
			assert(Bits);

			if ( ! jeBitmap_GammaCorrect_Data(Bits,&Info,Invert) )
			{
				jeErrorLog_AddString(-1,"jeBitmap_Gamma_Apply : GammaCorrect_Data", "");
				Ret = JE_FALSE;
			}
		}

		if ( ! jeBitmap_UnLockArray_NoChange(Locks,mipCount) )
		{
			jeErrorLog_AddString(-1,"jeBitmap_Gamma_Apply : UnLock failed", "");
			goto fail;
		}

		}
#endif
	}

	jeThreadQueue_Semaphore_UnLock(Bitmap_Gamma_Lock);

return Ret;

fail:

	jeThreadQueue_Semaphore_UnLock(Bitmap_Gamma_Lock);

return JE_FALSE;
}

/*}{*******************************************************/

jeBoolean jeBitmap_GammaCorrect_Data(void * Bits,jeBitmap_Info * pInfo, jeBoolean Invert)
{
const jePixelFormat_Operations * ops;
uint32 bpp,w,h,xtra,x,y;
uint32 * Lut;
jePixelFormat Format;
jePixelFormat_Decomposer	Decompose;
jePixelFormat_Composer		Compose;
jePixelFormat_ColorGetter	GetColor;
jePixelFormat_ColorPutter	PutColor;

	Format = pInfo->Format;
	ops = jePixelFormat_GetOperations(Format);
	if ( ! ops )
		return JE_FALSE;
	
	Decompose	= ops->DecomposePixel;
	Compose		= ops->ComposePixel;
	GetColor 	= ops->GetColor;
	PutColor	= ops->PutColor;

	assert( Compose && Decompose && GetColor && PutColor );

	if ( ! Invert )
	{
		if ( Format == JE_PIXELFORMAT_16BIT_565_RGB )
		{
			jeBitmap_GammaCorrect_Data_565_RGB(Bits,pInfo);
			return JE_TRUE;
		}
		else if ( Format == JE_PIXELFORMAT_16BIT_4444_ARGB )
		{
			jeBitmap_GammaCorrect_Data_4444_ARGB(Bits,pInfo);
			return JE_TRUE;
		}
	}

	if ( Invert )
		Lut = Gamma_Lut_Inverse;
	else
		Lut = Gamma_Lut;

	bpp = ops->BytesPerPel;
	w = pInfo->Width;
	h = pInfo->Height;
	xtra = pInfo->Stride - w;

	if ( pInfo->HasColorKey )
	{
		switch(bpp)
		{
			default:
			case 0:
				return JE_FALSE;
			case 1:
			{
			uint8 *ptr,ck;
				ck = (uint8)pInfo->ColorKey;
				ptr = (uint8 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						if ( *ptr != ck )
						{
							*ptr = (uint8)Lut[*ptr];
							if ( *ptr == ck )
								*ptr ^= 1;
						}
						ptr++;
					}
					ptr += xtra;
				}
				assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride ) );
				break;
			}
			case 2:
			{
			uint16 *ptr,ck;
			uint32 R,G,B,A,Pixel;
				ck = (uint16)pInfo->ColorKey;
				ptr = (uint16 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						if ( *ptr == ck )
						{
							ptr++;
						}
						else
						{
							Decompose(*ptr,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
							R = Lut[R];
							G = Lut[G];
							B = Lut[B];
							Pixel = Compose(R,G,B,A);
							if ( Pixel == ck )
								Pixel ^= 1;
							*ptr++ = (uint16)Pixel;
						}
					}
					ptr += xtra;
				}
				assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride * 2 ) );
				break;
			}

			case 3:
			{
			uint8 *ptr;
			uint32 R,G,B,A,Pixel,ck;
				ptr = (uint8 *)Bits;
				xtra *= 3;
				ck = pInfo->ColorKey;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						Pixel = (ptr[0]<<16) + (ptr[1]<<8) + ptr[2];
						if ( Pixel == ck )
						{
							ptr+=3;
						}
						else
						{
							Decompose(Pixel,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
							R = Lut[R];
							G = Lut[G];
							B = Lut[B];
							Pixel = Compose(R,G,B,A);	
							if ( Pixel == ck )
								Pixel ^= 1;
							*ptr++ = (uint8)((Pixel>>16)&0xFF);
							*ptr++ = (uint8)((Pixel>>8)&0xFF);
							*ptr++ = (uint8)((Pixel)&0xFF);
						}
					}
					ptr += xtra;
				}
				assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride * 3 ) );
				break;
			}

			case 4:
			{
			uint32 *ptr,ck;
			uint32 R,G,B,A,Pixel;
				ck = pInfo->ColorKey;
				ptr = (uint32 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						if ( *ptr == ck )
						{
							ptr++;
						}
						else
						{
							Decompose(*ptr,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
							R = Lut[R];
							G = Lut[G];
							B = Lut[B];
							Pixel = Compose(R,G,B,A);
							if ( Pixel == ck )
								Pixel ^= 1;
							*ptr++ = Pixel;
						}
					}
					ptr += xtra;
				}
				assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride * 4 ) );
				break;
			}
		}
	}
	else
	{
		switch(bpp)
		{
			default:
			case 0:
				return JE_FALSE;
			case 1:
			{
			uint8 *ptr;
				ptr = (uint8 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						*ptr++ = (uint8)Lut[*ptr];
					}
					ptr += xtra;
				}
				assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride ) );
				break;
			}
			case 2:
			{
			uint16 *ptr;
			uint32 R,G,B,A;
				ptr = (uint16 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						Decompose(*ptr,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
						R = Lut[R];
						G = Lut[G];
						B = Lut[B];
						*ptr++ = (uint16)Compose(R,G,B,A);
					}
					ptr += xtra;
				}
				assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride * 2 ) );
				break;
			}

			case 3:
			{
			uint8 *ptr,*ptrz;
			uint32 R,G,B,A;
				ptr = (uint8 *)Bits;
				xtra *= 3;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						ptrz = ptr;
						GetColor(&ptrz,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
						R = Lut[R];
						G = Lut[G];
						B = Lut[B];
						PutColor(&ptr,R,G,B,A);
					}
					ptr += xtra;
				}
				assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride * 3 ) );
				break;
			}

			case 4:
			{
			uint32 *ptr;
			uint32 R,G,B,A;
				ptr = (uint32 *)Bits;
				for(y=h;y--;)
				{
					for(x=w;x--;)
					{
						Decompose(*ptr,(int *)&R,(int *)&G,(int *)&B,(int *)&A);
						R = Lut[R];
						G = Lut[G];
						B = Lut[B];
						*ptr++ = Compose(R,G,B,A);
					}
					ptr += xtra;
				}
				assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride * 4 ) );
				break;
			}
		}
	}

	return JE_TRUE;
}

/*}{*******************************************************/

void jeBitmap_GammaCorrect_Data_565_RGB(void * Bits,jeBitmap_Info * pInfo)
{
uint32 w,h,xtra,x,y;
uint16 * ptr;

	if ( ComputedGamma_Lut != ComputedGamma_565_RGB )
	{
	uint32 r,g,b,R,G,B;
	uint32 ipel,opel;

		// compute 565 lookup table
		for(r=0;r<32;r++)
		{
			R = Gamma_Lut[ (r<<3) ] >> 3;
			for(g=0;g<64;g++)
			{
				G = Gamma_Lut[ (g<<2) ] >> 2;
				for(b=0;b<32;b++)
				{
					B = Gamma_Lut[ (b<<3) ] >> 3;
					ipel = (r<<11) + (g<<5) + b;
					opel = (R<<11) + (G<<5) + B;
					assert( opel < 65536 );
					Gamma_565_RGB[ipel] = (uint16)opel;
				}
			}
		}
		ComputedGamma_565_RGB = ComputedGamma_Lut;
	}

	w		= pInfo->Width;
	h		= pInfo->Height;
	xtra	= pInfo->Stride - w;
	ptr 	= (uint16 *)Bits;

	if ( pInfo->HasColorKey )
	{
	uint16 ck;
		ck = (uint16) pInfo->ColorKey;
		
		for(y=h;y--;)
		{
			for(x=w;x--;)
			{
				if ( *ptr != ck )
				{
					*ptr = Gamma_565_RGB[ *ptr ];
					if ( *ptr == ck )
						*ptr ^= 1;
				}
				ptr ++;
			}
			ptr += xtra;
		}
	}
	else
	{
		for(y=h;y--;)
		{
			for(x=w;x--;)
			{
				*ptr++ = Gamma_565_RGB[ *ptr ];
			}
			ptr += xtra;
		}
	}

	assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride * 2 ) );

}
					
void jeBitmap_GammaCorrect_Data_4444_ARGB(void * Bits,jeBitmap_Info * pInfo)
{
uint32 w,h,xtra,x,y;
uint16 * ptr;

	if ( ComputedGamma_Lut != ComputedGamma_4444_ARGB )
	{
	uint32 r,g,b,a,R,G,B;
	uint32 ipel,opel;

		// compute 565 lookup table
		for(r=0;r<16;r++)
		{
			R = Gamma_Lut[r<<4] >> 4;
			for(g=0;g<16;g++)
			{
				G = Gamma_Lut[g<<4] >> 4;
				for(b=0;b<16;b++)
				{
					B = Gamma_Lut[b<<4] >> 4;
					for(a=0;a<16;a++)
					{
						ipel = (a<<12) + (r<<8) + (g<<4) + b;
						opel = (a<<12) + (R<<8) + (G<<4) + B;
						assert( opel < 65536 );
						Gamma_4444_ARGB[ipel] = (uint16)opel;
					}
				}
			}
		}
		ComputedGamma_4444_ARGB = ComputedGamma_Lut;
	}

	w		= pInfo->Width;
	h		= pInfo->Height;
	xtra	= pInfo->Stride - w;
	ptr 	= (uint16 *)Bits;

	if ( pInfo->HasColorKey )
	{
	uint16 ck;
		ck = (uint16) pInfo->ColorKey;
		
		for(y=h;y--;)
		{
			for(x=w;x--;)
			{
				if ( *ptr != ck )
				{
					*ptr = Gamma_4444_ARGB[ *ptr ];
					if ( *ptr == ck )
						*ptr ^= 1;
				}
				ptr ++;
			}
			ptr += xtra;
		}
	}
	else
	{
		for(y=h;y--;)
		{
			for(x=w;x--;)
			{
				*ptr++ = Gamma_4444_ARGB[ *ptr ];
			}
			ptr += xtra;
		}
	}

	assert( (int)(ptr) == ( ((int)Bits) + pInfo->Height * pInfo->Stride * 2 ) );
}
																	
void jeBitmap_Gamma_Compute_Lut(double Gamma)
{
uint32 c,gc,lgc;

	lgc = 0;
	for(c=0;c<256;c++)
	{
		gc = (uint32)( 255.0 * pow( c * (1.0/255.0) , 1.0 / Gamma ) + 0.4 );
		if ( gc > 255 ) gc = 255;
		Gamma_Lut[c] = gc;
		assert( lgc <= gc );
		for(lgc;lgc<=gc;lgc++)
			Gamma_Lut_Inverse[lgc] = c;
		lgc = gc;
	}
	for(gc;gc<256;gc++)
		Gamma_Lut_Inverse[gc] = 255;

	Gamma_Lut[0] = 0;
	Gamma_Lut_Inverse[0] = 0;
	Gamma_Lut[255] = 255;
	Gamma_Lut_Inverse[255] = 255;

	ComputedGamma_Lut = (float)Gamma;
}

/*}{*******************************************************/
