#include "stdafx.h"

#include "ram.h"
#include "bitmapresize.h"
#include "bmp.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CBitmapResize::CBitmapResize(void)
{
	
}

CBitmapResize::~CBitmapResize(void)
{
	
}



// Functions for smooth bitmap resize
//
// Improvement: float calculations changed to int.
//
// Ivaylo Byalkov, January 24, 2000
// e-mail: ivob@i-n.net
//




///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Main resize function

HBITMAP CBitmapResize::ScaleBitmapInt(HBITMAP hBmp, 
                       WORD wNewWidth, 
                       WORD wNewHeight)
{
	BITMAP bmp;
	::GetObject(hBmp, sizeof(BITMAP), &bmp);
	
	// check for valid size
	if((bmp.bmWidth > wNewWidth 
		&& bmp.bmHeight < wNewHeight) 
		|| bmp.bmWidth < wNewWidth 
		&& bmp.bmHeight > wNewHeight)
		return NULL;
	
	HDC hDC = ::GetDC(NULL);
	BITMAPINFO *pbi = PrepareRGBBitmapInfo((WORD)bmp.bmWidth, (WORD)bmp.bmHeight);
	BYTE *pData = new BYTE[pbi->bmiHeader.biSizeImage];
	
	::GetDIBits(hDC, hBmp, 0, bmp.bmHeight, pData, pbi, DIB_RGB_COLORS);
	
	jeRam_Free(pbi);
	pbi = NULL;
	
	pbi = PrepareRGBBitmapInfo(wNewWidth, wNewHeight);
	BYTE *pData2 = new BYTE[pbi->bmiHeader.biSizeImage];

	if(bmp.bmWidth >= wNewWidth && bmp.bmHeight >= wNewHeight) {
		ShrinkDataInt(pData, 
					(WORD)bmp.bmWidth, 
					(WORD)bmp.bmHeight,
					pData2, 
					wNewWidth, 
					wNewHeight);
	} else {
		EnlargeDataInt(pData, 
					(WORD)bmp.bmWidth, 
					(WORD)bmp.bmHeight,
					pData2, 
					wNewWidth, 
					wNewHeight);
	}
	delete pData;
	pData = NULL;
	
	HBITMAP hResBmp = ::CreateCompatibleBitmap(hDC, 
		wNewWidth, 
		wNewHeight);
	
	::SetDIBits(hDC, 
		hResBmp, 
		0, 
		wNewHeight, 
		pData2, 
		pbi, 
		DIB_RGB_COLORS);
	
	::ReleaseDC(NULL, hDC);
	
	jeRam_Free(pbi);
	pbi = NULL;
	delete pData2;
	pData2 = NULL;
	//DeleteObject(hBmp);
	//hBmp = NULL;
	
	return hResBmp;
}

///////////////////////////////////////////////////////////






