/****************************************************************************************/
/*  MFCUTIL.CPP                                                                         */
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
#include "stdafx.h"

#include "..\Resource.h"

#include "MfcUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void PositionDialogUnderTabs( CDialog * pDlg )
{
	RECT			rect{};
	RECT			TabRect{};
	CTabCtrl	*	pTC = NULL;
	int				i{};
	int				nTabs{};

	pTC = (CTabCtrl*)pDlg->GetParent( ) ;
	if (pTC)
	{
		nTabs = pTC->GetItemCount() ;
		TabRect.top = TabRect.left = 1000 ;
		TabRect.bottom = TabRect.right = 0 ;

		for( i=0; i< nTabs; i++ )
		{
			pTC->GetItemRect( i, &rect );
			if( rect.left < TabRect.left )
				TabRect.left = rect.left ;
			if( rect.right > TabRect.right )
				TabRect.right = rect.right ;
			if( rect.top < TabRect.top )
				TabRect.top = rect.top ;
			if( rect.bottom > TabRect.bottom )
				TabRect.bottom  = rect.bottom ;
		}
	}
	pDlg->SetWindowPos
		( 
		NULL, 
		TabRect.left + GetSystemMetrics(SM_CXDLGFRAME),	
		TabRect.bottom + GetSystemMetrics(SM_CYDLGFRAME), 
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE
		) ;

}// PositionDialogUnderTabs

// LoadBMPImage	- Loads a BMP file and creates a bitmap GDI object
//		  also creates logical palette for it.
// Returns	- TRUE for success
// sBMPFile	- Full path of the BMP file
// bitmap	- The bitmap object to initialize
// pPal		- Will hold the logical palette. Can be NULL
BOOL LoadBMPImage( LPCTSTR sBMPFile, CBitmap& bitmap, CPalette *pPal )
{
	CFile				file{};
	BITMAPFILEHEADER	bmfHeader{};	// Read file header
	WORD				nColors{};

	if( !file.Open( sBMPFile, CFile::modeRead) )
		return FALSE ;


	if( file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) != sizeof(bmfHeader) )
		return FALSE;	// File type should be 'BM'

	if( bmfHeader.bfType != ((WORD) ('M' << 8) | 'B'))
		return FALSE;

	// Get length of the remainder of the file and allocate memory
	DWORD nPackedDIBLen = (DWORD)(file.GetLength() - sizeof(BITMAPFILEHEADER));

	HGLOBAL hDIB = ::GlobalAlloc(GMEM_FIXED, nPackedDIBLen);
	if( hDIB == 0 )
		return FALSE;	// Read the remainder of the bitmap file.

	if( file.Read((LPSTR)hDIB, nPackedDIBLen) != nPackedDIBLen )
	{
		::GlobalFree(hDIB);
		return FALSE;
	}
	BITMAPINFOHEADER &bmiHeader = *(LPBITMAPINFOHEADER)hDIB ;
	BITMAPINFO &bmInfo = *(LPBITMAPINFO)hDIB ;
	// If bmiHeader.biClrUsed is zero we have to infer the number
	// of colors from the number of bits used to specify it.
	nColors = (bmiHeader.biClrUsed)
		? (WORD) bmiHeader.biClrUsed 
		: (WORD)(1 << bmiHeader.biBitCount);

	LPVOID lpDIBBits;
	if( bmInfo.bmiHeader.biBitCount > 8 )
		lpDIBBits = (LPVOID)((LPDWORD)(bmInfo.bmiColors + bmInfo.bmiHeader.biClrUsed) + 
			((bmInfo.bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
	else
		lpDIBBits = (LPVOID)(bmInfo.bmiColors + nColors);

	// Create the logical palette
	if( pPal != nullptr )
	{	// Create the palette
		if( nColors <= 256 )
		{
			UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
			LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];
			pLP->palVersion = 0x300;
			pLP->palNumEntries = nColors;
			for( int i=0; i < nColors; i++)
			{
				pLP->palPalEntry[i].peRed = bmInfo.bmiColors[i].rgbRed;
				pLP->palPalEntry[i].peGreen = bmInfo.bmiColors[i].rgbGreen;
				pLP->palPalEntry[i].peBlue = bmInfo.bmiColors[i].rgbBlue;
				pLP->palPalEntry[i].peFlags = 0;
			}
			pPal->CreatePalette( pLP );
			delete[] pLP;
		}
	}

	CClientDC dc(nullptr);
	CPalette* pOldPalette = nullptr;
	if( pPal )
	{
		pOldPalette = dc.SelectPalette( pPal, FALSE );
		dc.RealizePalette();	
	}
	HBITMAP hBmp = CreateDIBitmap
	(
		dc.m_hDC,			// handle to device context
		&bmiHeader,			// pointer to bitmap size and format data
		CBM_INIT,			// initialization flag
		lpDIBBits,			// pointer to initialization data
		&bmInfo,			// pointer to bitmap color-format data
		DIB_RGB_COLORS		// color-data usage
	);
	bitmap.Attach( hBmp );
	if( pOldPalette )
		dc.SelectPalette( pOldPalette, FALSE );

	::GlobalFree(hDIB) ;
	return TRUE ;
}// LoadBMPImage


void TrimString( CString &strString )
{
	strString.TrimRight() ;
	strString.TrimLeft() ;
}// TrimString


void TrimStringByPixelCount( HDC hDC, char * psz, const int nMaxPixels )
{
	int		nLen = strlen( psz ) ;
	SIZE	size{};
	

	do
	{
		::GetTextExtentPoint32( hDC, psz, nLen, &size ) ;
		if( size.cx > nMaxPixels )
			psz[--nLen] = 0 ;

	}while( (size.cx > nMaxPixels) && (nLen>0) ) ;
}// TrimStringByPixelCount

void SetupTemplateDialogIcons( CDialog * pDlg )
{
	HICON	hIcon{};
	CWinApp* pApp{};

	pApp = AfxGetApp() ;
	hIcon = pApp->LoadIcon( IDI_CUBE );
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACECUBE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon) ;
	hIcon = pApp->LoadIcon( IDI_SPHERE ) ;
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACESPHEROID, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon) ;
	hIcon = pApp->LoadIcon( IDI_CYLINDER ) ;
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACECYLINDER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon) ;
	hIcon = pApp->LoadIcon( IDI_STAIRS ) ;
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACESTAIRCASE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon) ;
	hIcon = pApp->LoadIcon( IDI_ARCH ) ;
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACEARCH, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon ) ;
	hIcon = pApp->LoadIcon( IDI_CONE ) ;
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACECONE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon ) ;

	hIcon = pApp->LoadIcon( IDI_LIGHT ) ;
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACELIGHT, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon ) ;
	hIcon = pApp->LoadIcon( IDI_ENTITY ) ;
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACEENTITY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon ) ;
	hIcon = pApp->LoadIcon( IDI_TERRAIN ) ;
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACETERRAIN, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon ) ;
	hIcon = pApp->LoadIcon( IDI_SHEET ) ;
	pDlg->SendDlgItemMessage( IDM_TOOLS_PLACESHEET, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon ) ;

}// SetupTemplateDialogIcons


HTREEITEM TreeViewIsInBranch( CTreeCtrl *pTV, HTREEITEM hItem, const char * psz)
{
	HTREEITEM	hChild{};
	CString		cstr ;

	ASSERT( pTV != NULL ) ;
	ASSERT( psz != NULL ) ;

	if( pTV->ItemHasChildren( hItem ) == 0 )
		return nullptr ;

	hChild = pTV->GetChildItem( hItem ) ;
	do
	{
		cstr = pTV->GetItemText( hChild ) ;
		if( cstr.Compare( psz ) == 0 )
			return hChild ;

		hChild = pTV->GetNextSiblingItem( hChild ) ;
	}while( hChild != NULL ) ;

	return nullptr ;

}// TreeViewIsInBranch



//----------------------------------------------------------
// BMP-Utils


#define DIB_HEADER_MARKER		((WORD) ('M' << 8) | 'B')
#define PALVERSION				0x300
#define IS_WIN30_DIB(lpbi)		((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)		((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)		((lpRect)->bottom - (lpRect)->top)
#define WIDTHBYTES(bits)		(((bits) + 31) / 32 * 4)

WORD WINAPI DIBNumColors(LPSTR lpbi)
{
	WORD wBitCount{};  // DIB bit count

	/*  If this is a Windows-style DIB, the number of colors in the
	 *  color table can be less than the number of bits per pixel
	 *  allows for (i.e. lpbi->biClrUsed can be set to some value).
	 *  If this is the case, return the appropriate value.
	 */

	if (IS_WIN30_DIB(lpbi))
	{
		DWORD dwClrUsed;

		dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;
		if (dwClrUsed != 0)
			return (WORD)dwClrUsed;
	}

	/*  Calculate the number of colors in the color table based on
	 *  the number of bits per pixel for the DIB.
	 */
	if (IS_WIN30_DIB(lpbi))
		wBitCount = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;
	else
		wBitCount = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;

	/* return number of colors based on bits per pixel */
	switch (wBitCount)
	{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			return 0;
	}
}

WORD WINAPI PaletteSize(LPSTR lpbi)
{
   /* calculate the size required by the palette */
   if (IS_WIN30_DIB (lpbi))
	  return (WORD)(::DIBNumColors(lpbi) * sizeof(RGBQUAD));
   else
	  return (WORD)(::DIBNumColors(lpbi) * sizeof(RGBTRIPLE));
}


/*************************************************************************
 *
 * SaveDIB()
 *
 * Saves the specified DIB into the specified CFile.  The CFile
 * is opened and closed by the caller.
 *
 * Parameters:
 *
 * HDIB hDib - Handle to the dib to save
 *
 * CFile& file - open CFile used to save DIB
 *
 * Return value: TRUE if successful, else FALSE or CFileException
 *
 *************************************************************************/

#include "ErrorLog.h"

jeBoolean  SaveDIB(jeVFile	*	pF, jePtrMgr	*pPtrMgr ,HANDLE hDib)
{
	BITMAPFILEHEADER bmfHdr{}; // Header for Bitmap file
	LPBITMAPINFOHEADER lpBI{};   // Pointer to DIB info structure
	DWORD dwDIBSize{};

	if (hDib == nullptr)
		return JE_FALSE;

	/*
	 * Get a pointer to the DIB memory, the first of which contains
	 * a BITMAPINFO structure
	 */
	lpBI = (LPBITMAPINFOHEADER) ::GlobalLock((HGLOBAL) hDib);
	if (lpBI == nullptr)
		return JE_FALSE;

	if (!IS_WIN30_DIB(lpBI))
	{
		::GlobalUnlock((HGLOBAL) hDib);
		jeErrorLog_AddString( JE_ERR_WINDOWS_API_FAILURE, "SaveDIB: Unsupported DIB", "Jet3D");

		return JE_FALSE;       // It's an other-style DIB (save not supported)
	}

	/*
	 * Fill in the fields of the file header
	 */

	/* Fill in file type (first 2 bytes must be "BM" for a bitmap) */
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

	// Calculating the size of the DIB is a bit tricky (if we want to
	// do it right).  The easiest way to do this is to call GlobalSize()
	// on our global handle, but since the size of our global memory may have
	// been padded a few bytes, we may end up writing out a few too
	// many bytes to the file (which may cause problems with some apps).
	//
	// So, instead let's calculate the size manually (if we can)
	//
	// First, find size of header plus size of color table.  Since the
	// first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
	// the size of the structure, let's use this.

	dwDIBSize = *(LPDWORD)lpBI + ::PaletteSize((LPSTR)lpBI);  // Partial Calculation

	// Now calculate the size of the image

	if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4))
	{
		// It's an RLE bitmap, we can't calculate size, so trust the
		// biSizeImage field

		dwDIBSize += lpBI->biSizeImage;
	}
	else
	{
		DWORD dwBmBitsSize;  // Size of Bitmap Bits only

		// It's not RLE, so size is Width (DWORD aligned) * Height

		dwBmBitsSize = WIDTHBYTES((lpBI->biWidth)*((DWORD)lpBI->biBitCount)) * lpBI->biHeight;

		dwDIBSize += dwBmBitsSize;

		// Now, since we have calculated the correct size, why don't we
		// fill in the biSizeImage field (this will fix any .BMP files which
		// have this field incorrect).

		lpBI->biSizeImage = dwBmBitsSize;
	}


	// Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)

	bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;

	/*
	 * Now, calculate the offset the actual bitmap bits will be in
	 * the file -- It's the Bitmap file header plus the DIB header,
	 * plus the size of the color table.
	 */
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize;

			// Write the file header 
	if(jeVFile_Write( pF, &bmfHdr, sizeof(BITMAPFILEHEADER) ) == JE_FALSE )
		{   ::GlobalUnlock((HGLOBAL) hDib);
			jeErrorLog_AddString( JE_ERR_WINDOWS_API_FAILURE, "SaveDIB: Write of BMP-Header failed", "Jet3D");
			return JE_FALSE;
		}

	// Write the DIB header and the bits 
	if(jeVFile_Write( pF, lpBI, dwDIBSize ) == JE_FALSE )
		{   ::GlobalUnlock((HGLOBAL) hDib);
			jeErrorLog_AddString( JE_ERR_WINDOWS_API_FAILURE, "SaveDIB: Write of BMP-Body failed", "Jet3D");
			return JE_FALSE;
		}

	::GlobalUnlock((HGLOBAL) hDib);
	return TRUE;
}



// DDBToDIB		- Creates a DIB from a DDB
// bitmap		- Device dependent bitmap
// dwCompression	- Type of compression - see BITMAPINFOHEADER
// pPal			- Logical palette
HANDLE DDBToDIB( CBitmap& bitmap, DWORD dwCompression, CPalette* pPal ) 
{
	BITMAP			bm{};
	BITMAPINFOHEADER	bi{};
	LPBITMAPINFOHEADER 	lpbi{};
	DWORD			dwLen{};
	HANDLE			hDIB{};
	HANDLE			handle{};
	HDC 			hDC{};
	HPALETTE		hPal{};


	ASSERT( bitmap.GetSafeHandle() );

	// The function has no arg for bitfields
	if( dwCompression == BI_BITFIELDS )
		return nullptr;

	// If a palette has not been supplied use defaul palette
	hPal = (HPALETTE) pPal->GetSafeHandle();
	if (hPal==NULL)
		hPal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

	// Get bitmap information
	bitmap.GetObject(sizeof(bm),(LPSTR)&bm);

	// Initialize the bitmapinfoheader
	bi.biSize		= sizeof(BITMAPINFOHEADER);
	bi.biWidth		= (bm.bmWidth/4+1)*4;
	bi.biHeight 		= bm.bmHeight;
	bi.biPlanes 		= 1;
	bi.biBitCount		= 0;	//Icestorm: GetDIBits want this to be 0 th first time
	bi.biCompression	= dwCompression;
	bi.biSizeImage		= 0;
	bi.biXPelsPerMeter	= 0;
	bi.biYPelsPerMeter	= 0;
	bi.biClrUsed		= 0;
	bi.biClrImportant	= 0;

	
	// Compute the size of the  infoheader and the color table
	int nColors = (1 << (bm.bmPlanes * bm.bmBitsPixel));
	if( nColors > 256 ) 
		nColors = 0;
	dwLen  = bi.biSize + nColors * sizeof(RGBQUAD);
	
	// We need a device context to get the DIB from
	hDC = GetDC(nullptr);
	hPal = SelectPalette(hDC,hPal,FALSE);
	RealizePalette(hDC);

	// Allocate enough memory to hold bitmapinfoheader and color table
	hDIB = GlobalAlloc(GMEM_FIXED,dwLen);

	if (!hDIB){
		SelectPalette(hDC,hPal,FALSE);
		ReleaseDC(NULL,hDC);
	    jeErrorLog_AddString( JE_ERR_WINDOWS_API_FAILURE, "DDBToDIB: No mem left", "Jet3D");
		return nullptr;
	}

	lpbi = (LPBITMAPINFOHEADER)hDIB;

	*lpbi = bi;

	// Call GetDIBits with a NULL lpBits param, so the device driver 
	// will calculate the biSizeImage field 
	GetDIBits(hDC, (HBITMAP)bitmap.GetSafeHandle(), 0L, (DWORD)bi.biHeight,
			(LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

	bi = *lpbi;

	// If the driver did not fill in the biSizeImage field, then compute it
	// Each scan line of the image is aligned on a DWORD (32bit) boundary
	if (bi.biSizeImage == 0){
		bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) 
						* bi.biHeight;

		// If a compression scheme is used the result may infact be larger
		// Increase the size to account for this.
		if (dwCompression != BI_RGB)
			bi.biSizeImage = (bi.biSizeImage * 3) / 2;
	}

	// Realloc the buffer so that it can hold all the bits
	dwLen += bi.biSizeImage;
	if (handle = GlobalReAlloc(hDIB, dwLen, GMEM_MOVEABLE))
		hDIB = handle;
	else{
		GlobalFree(hDIB);

		// Reselect the original palette
		SelectPalette(hDC,hPal,FALSE);
		ReleaseDC(NULL,hDC);
	    jeErrorLog_AddString( JE_ERR_WINDOWS_API_FAILURE, "DDBToDIB: GlobalReAlloc failed", "Jet3D");
		return NULL;
	}

	// Get the bitmap bits
	lpbi = (LPBITMAPINFOHEADER)hDIB;

	// FINALLY get the DIB
	BOOL bGotBits = GetDIBits( hDC, (HBITMAP)bitmap.GetSafeHandle(),
				0L,				// Start scan line
				(DWORD)bi.biHeight,		// # of scan lines
				(LPBYTE)lpbi 			// address for bitmap bits
				+ (bi.biSize /*+ nColors * sizeof(RGBQUAD)*/),	//Icestorm: GetDIBBits copies the table,too
				(LPBITMAPINFO)lpbi,		// address of bitmapinfo
				(DWORD)DIB_RGB_COLORS);		// Use RGB for color table

	if( !bGotBits )
	{
		GlobalFree(hDIB);
		
		SelectPalette(hDC,hPal,FALSE);
		ReleaseDC(NULL,hDC);
	    jeErrorLog_AddString( JE_ERR_WINDOWS_API_FAILURE, "DDBToDIB: GetDIBits failed", "Jet3D");
		return NULL;
	}

	SelectPalette(hDC,hPal,FALSE);
	ReleaseDC(NULL,hDC);
	return hDIB;
}


jeBoolean WriteWindowToDIB( jeVFile	*	pF, jePtrMgr	*pPtrMgr , CWnd *pWnd )
{
	CBitmap 	bitmap{};
	CWindowDC	dc(pWnd);
	CDC 		memDC{};
	CRect		rect{};
	BOOL		ret{};

	memDC.CreateCompatibleDC(&dc); 

	pWnd->GetWindowRect(rect);

	bitmap.CreateCompatibleBitmap(&dc, rect.Width(),rect.Height() );

	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);
	memDC.BitBlt(0, 0, rect.Width(),rect.Height(), &dc, 0, 0, SRCCOPY); 

	// Create logical palette if device support a palette
	CPalette pal;
	if( dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE )
	{
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256);
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];
		pLP->palVersion = 0x300;

		pLP->palNumEntries = 
			GetSystemPaletteEntries( dc, 0, 255, pLP->palPalEntry );

		// Create the palette
		pal.CreatePalette( pLP );

		delete[] pLP;
	}

	memDC.SelectObject(pOldBitmap);

	// Convert the bitmap to a DIB
	HANDLE hDIB = DDBToDIB( bitmap, BI_RGB, &pal );

	if( hDIB == nullptr )
		{
		  GlobalFree( hDIB );
		  jeErrorLog_AddString( JE_ERR_WINDOWS_API_FAILURE, "WriteWindowToDIB: bitmap to a DIB failed", "Jet3D");
		  return JE_FALSE;
		}

	// Write it to file
	ret = SaveDIB( pF, pPtrMgr, hDIB );

	// Free the memory allocated by DDBToDIB for the DIB
	GlobalFree( hDIB );
	return JE_TRUE;
}




