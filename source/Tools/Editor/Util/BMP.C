/****************************************************************************************/
/*  BMP.C                                                                               */
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
#include "bmp.h"
#include "ram.h"

// This will create a 24BIT bgr...
HBITMAP CreateHBitmapFromgeBitmap (jeBitmap *Bitmap, HDC hdc)
{
	jeBitmap * Lock;
	jePixelFormat Format;
	jeBitmap_Info info;
	HBITMAP hbm = NULL;

	// <> choose format to be 8,16,or 24, whichever is closest to Bitmap
	Format = JE_PIXELFORMAT_24BIT_BGR;

	if ( ! jeBitmap_LockForRead(Bitmap, &Lock, 0, 0, Format, JE_FALSE,0) )
	{
		return NULL;
	}

	jeBitmap_GetInfo(Lock,&info,NULL);

	if ( info.Format != Format )
		return NULL;
	{
		void * bits;
		BITMAPINFOHEADER bmih;
		int pelbytes;

		pelbytes = jePixelFormat_BytesPerPel(Format);
		bits = jeBitmap_GetBits(Lock);

		bmih.biSize = sizeof(bmih);
		bmih.biHeight = - info.Height;
		bmih.biPlanes = 1;
		bmih.biBitCount = 24;
		bmih.biCompression = BI_RGB;
		bmih.biSizeImage = 0;
		bmih.biXPelsPerMeter = bmih.biYPelsPerMeter = 10000;
		bmih.biClrUsed = bmih.biClrImportant = 0;

		if ( (info.Stride*pelbytes) == (((info.Stride*pelbytes)+3)&(~3)) )
		{
			bmih.biWidth = info.Stride;
			hbm = CreateDIBitmap( hdc, &bmih , CBM_INIT , bits, (BITMAPINFO *)&bmih , DIB_RGB_COLORS );
		}
		else
		{
			void * newbits;
			int Stride;

			bmih.biWidth = info.Width;
			Stride = (((info.Width*pelbytes)+3)&(~3));
			newbits = jeRam_Allocate(Stride * info.Height);
			if ( newbits )
			{
				char *newptr,*oldptr;
				int y;

				newptr = (char *)newbits;
				oldptr = (char *)bits;
				for(y=0; y<info.Height; y++)
				{
					memcpy(newptr,oldptr,(info.Width)*pelbytes);
					oldptr += info.Stride*pelbytes;
					newptr += Stride;
				}
				hbm = CreateDIBitmap( hdc, &bmih , CBM_INIT , newbits, (BITMAPINFO *)&bmih , DIB_RGB_COLORS );
				jeRam_Free(newbits);
			}
		}
	}

	jeBitmap_UnLock (Lock);

	return hbm;
}

HBITMAP Bmp_CreateEmpty24BitDIB( HDC hDC, int32 nWidth, int32 nHeight, void * pBits )
{
	BITMAPINFOHEADER bmih;

	bmih.biSize = sizeof( bmih ) ;
	bmih.biHeight = - nHeight ;
	bmih.biWidth = nWidth ;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = bmih.biYPelsPerMeter = 10000;
	bmih.biClrUsed = bmih.biClrImportant = 0;

	return CreateDIBitmap( hDC, &bmih, CBM_INIT, pBits, (BITMAPINFO *)&bmih, DIB_RGB_COLORS ) ;
}// Bmp_CreateEmpty24BitDIB

BITMAPINFO *PrepareRGBBitmapInfo(WORD wWidth, WORD wHeight)
{
	//	gets deleted inside calling function
	BITMAPINFO *pRes = JE_RAM_ALLOCATE_STRUCT(BITMAPINFO);
	memset(pRes, 0, sizeof(BITMAPINFO));
	pRes->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pRes->bmiHeader.biWidth = wWidth;
	pRes->bmiHeader.biHeight = wHeight;
	pRes->bmiHeader.biPlanes = 1;
	pRes->bmiHeader.biBitCount = 24;

	pRes->bmiHeader.biSizeImage = 
	((3 * wWidth + 3) & ~3) * wHeight;
		
	return pRes;
}

int *CreateCoeffInt(int nLen, int nNewLen, BOOL bShrink)
{
	int i;
	int nSum = 0, nSum2;
	int *pRes = (int*)jeRam_Allocate(2 * nLen * sizeof(int));
	int *pCoeff = pRes;
	int nNorm =  (bShrink) ? (nNewLen << 12) / nLen : 0x1000;
	int	nDenom = (bShrink) ? nLen : nNewLen;
	
	ZeroMemory(pRes, 2 * nLen * sizeof(int));
	for(i = 0; i < nLen; i++, pCoeff += 2)
	{
		nSum2 = nSum + nNewLen;
		if(nSum2 > nLen)
		{
			*pCoeff = ((nLen - nSum) << 12) / nDenom;
			pCoeff[1] = ((nSum2 - nLen) << 12) / nDenom;
			nSum2 -= nLen;
		}
		else
		{
			*pCoeff = nNorm;
			if(nSum2 == nLen)
			{
				pCoeff[1] = -1;
				nSum2 = 0;
			}
		}
		nSum = nSum2;
	}
	
	return pRes;
}

void ShrinkDataInt(BYTE *pInBuff, 
                   WORD wWidth, 
                   WORD wHeight,
                   BYTE *pOutBuff, 
                   WORD wNewWidth, 
                   WORD wNewHeight)
{
	BYTE  *pLine = pInBuff, *pPix;
	BYTE  *pOutLine = pOutBuff;
	DWORD dwInLn = (3 * wWidth + 3) & ~3;
	DWORD dwOutLn = (3 * wNewWidth + 3) & ~3;
	int   x, y, i, ii;
	BOOL  bCrossRow, bCrossCol;
	int   *pRowCoeff = CreateCoeffInt(wWidth, wNewWidth, TRUE);
	int   *pColCoeff = CreateCoeffInt(wHeight, wNewHeight, TRUE);
	int   *pXCoeff, *pYCoeff = pColCoeff;
	DWORD dwBuffLn = 3 * wNewWidth * sizeof(DWORD);
	DWORD *pdwBuff = (DWORD*) jeRam_Allocate(6 * wNewWidth * sizeof(DWORD));
	DWORD *pdwCurrLn = pdwBuff, 
		*pdwCurrPix, 
		*pdwNextLn = pdwBuff + 3 * wNewWidth;
	DWORD dwTmp, *pdwNextPix;
	
	ZeroMemory(pdwBuff, 2 * dwBuffLn);
	
	y = 0;
	while(y < wNewHeight)
	{
		pPix = pLine;
		pLine += dwInLn;
		
		pdwCurrPix = pdwCurrLn;
		pdwNextPix = pdwNextLn;
		
		x = 0;
		pXCoeff = pRowCoeff;
		bCrossRow = pYCoeff[1] > 0;
		while(x < wNewWidth)
		{
			dwTmp = *pXCoeff * *pYCoeff;
			for(i = 0; i < 3; i++)
				pdwCurrPix[i] += dwTmp * pPix[i];
			bCrossCol = pXCoeff[1] > 0;
			if(bCrossCol)
			{
				dwTmp = pXCoeff[1] * *pYCoeff;
				for(i = 0, ii = 3; i < 3; i++, ii++)
					pdwCurrPix[ii] += dwTmp * pPix[i];
			}
			if(bCrossRow)
			{
				dwTmp = *pXCoeff * pYCoeff[1];
				for(i = 0; i < 3; i++)
					pdwNextPix[i] += dwTmp * pPix[i];
				if(bCrossCol)
				{
					dwTmp = pXCoeff[1] * pYCoeff[1];
					for(i = 0, ii = 3; i < 3; i++, ii++)
						pdwNextPix[ii] += dwTmp * pPix[i];
				}
			}
			if(pXCoeff[1])
			{
				x++;
				pdwCurrPix += 3;
				pdwNextPix += 3;
			}
			pXCoeff += 2;
			pPix += 3;
		}
		if(pYCoeff[1])
		{
			// set result line
			pdwCurrPix = pdwCurrLn;
			pPix = pOutLine;
			for(i = 3 * wNewWidth; i > 0; i--, pdwCurrPix++, pPix++)
				*pPix = ((LPBYTE)pdwCurrPix)[3];
			
			// prepare line buffers
			pdwCurrPix = pdwNextLn;
			pdwNextLn = pdwCurrLn;
			pdwCurrLn = pdwCurrPix;
			ZeroMemory(pdwNextLn, dwBuffLn);
			
			y++;
			pOutLine += dwOutLn;
		}
		pYCoeff += 2;
	}
	
	jeRam_Free(pRowCoeff);
	jeRam_Free(pColCoeff);
	jeRam_Free(pdwBuff);
	
} 

///////////////////////////////////////////////////////////

void EnlargeDataInt(BYTE *pInBuff, 
                    WORD wWidth, 
                    WORD wHeight,
                    BYTE *pOutBuff, 
                    WORD wNewWidth, 
                    WORD wNewHeight)
{
	BYTE  *pLine = pInBuff, 
		*pPix = pLine, 
		*pPixOld, 
		*pUpPix, 
		*pUpPixOld;
	BYTE  *pOutLine = pOutBuff, *pOutPix;
	DWORD dwInLn = (3 * wWidth + 3) & ~3;
	DWORD dwOutLn = (3 * wNewWidth + 3) & ~3;
	int   x, y, i;
	BOOL  bCrossRow, bCrossCol;
	int   *pRowCoeff = CreateCoeffInt(wNewWidth, 
		wWidth, 
		FALSE);
	int   *pColCoeff = CreateCoeffInt(wNewHeight, 
		wHeight, 
		FALSE);
	int   *pXCoeff, *pYCoeff = pColCoeff;
	DWORD dwTmp, dwPtTmp[3];
	
	y = 0;
	while(y < wHeight)
	{
		bCrossRow = pYCoeff[1] > 0;
		x = 0;
		pXCoeff = pRowCoeff;
		pOutPix = pOutLine;
		pOutLine += dwOutLn;
		pUpPix = pLine;
		if(pYCoeff[1])
		{
			y++;
			pLine += dwInLn;
			pPix = pLine;
		}
		
		while(x < wWidth)
		{
			bCrossCol = pXCoeff[1] > 0;
			pUpPixOld = pUpPix;
			pPixOld = pPix;
			if(pXCoeff[1])
			{
				x++;
				pUpPix += 3;
				pPix += 3;
			}
			
			dwTmp = *pXCoeff * *pYCoeff;
			
			for(i = 0; i < 3; i++)
				dwPtTmp[i] = dwTmp * pUpPixOld[i];
			
			if(bCrossCol)
			{
				dwTmp = pXCoeff[1] * *pYCoeff;
				for(i = 0; i < 3; i++)
					dwPtTmp[i] += dwTmp * pUpPix[i];
			}
			
			if(bCrossRow)
			{
				dwTmp = *pXCoeff * pYCoeff[1];
				for(i = 0; i < 3; i++)
					dwPtTmp[i] += dwTmp * pPixOld[i];
				if(bCrossCol)
				{
					dwTmp = pXCoeff[1] * pYCoeff[1];
					for(i = 0; i < 3; i++)
						dwPtTmp[i] += dwTmp * pPix[i];
				}
			}
			
			for(i = 0; i < 3; i++, pOutPix++)
				*pOutPix = ((LPBYTE)(dwPtTmp + i))[3];
			
			pXCoeff += 2;
		}
		pYCoeff += 2;
	}
	
	jeRam_Free(pRowCoeff);
	jeRam_Free(pColCoeff);
}

#define THUMBNAIL_SIZE	64
jeBoolean CreateThumbnails(jeBitmap* pBmps, jeMaterialSpec_Thumbnail* pThumb)
{
	HDC hdc;
	HBITMAP hBitmap;
	BITMAPINFO *pbi;
	BYTE *pDataIn, *pDataOut;
	BITMAP bmp;
	int DataInLen;

	hdc = GetDC(NULL);
	hBitmap = CreateHBitmapFromgeBitmap(pBmps, hdc);

    if (hBitmap == NULL) {
        return JE_FALSE;
    }

	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	pbi = PrepareRGBBitmapInfo((WORD)bmp.bmWidth, (WORD)bmp.bmHeight);
	pDataIn = (BYTE*) jeRam_Allocate(pbi->bmiHeader.biSizeImage);
	DataInLen = pbi->bmiHeader.biSizeImage;
	
	GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pDataIn, pbi, DIB_RGB_COLORS);
	
	jeRam_Free(pbi);
	pbi = NULL;
	
	pbi = PrepareRGBBitmapInfo(THUMBNAIL_SIZE, THUMBNAIL_SIZE);
	pDataOut = (BYTE*) jeRam_Allocate(pbi->bmiHeader.biSizeImage);

	if (bmp.bmWidth == THUMBNAIL_SIZE && bmp.bmHeight == THUMBNAIL_SIZE) {
		memcpy(pDataOut, pDataIn, pbi->bmiHeader.biSizeImage);
		pThumb->width = THUMBNAIL_SIZE;
		pThumb->height = THUMBNAIL_SIZE;
	} else
	if (bmp.bmWidth >= THUMBNAIL_SIZE) {
		pThumb->width = THUMBNAIL_SIZE;
		pThumb->height = (uint8)  min(bmp.bmHeight,THUMBNAIL_SIZE);
		ShrinkDataInt(pDataIn, 
					(WORD)bmp.bmWidth, 
					(WORD)bmp.bmHeight,
					pDataOut, 
					THUMBNAIL_SIZE, 
					pThumb->height);
	} else {
		memcpy(pDataOut, pDataIn, DataInLen);
		pThumb->width = (uint8) bmp.bmWidth;
		pThumb->height = (uint8) bmp.bmHeight;
	}

    DeleteObject(hBitmap);
	jeRam_Free(pbi);
	jeRam_Free(pDataIn);

	pThumb->contents = pDataOut;

	return JE_TRUE;
}


/* EOF: Bmp.c */
