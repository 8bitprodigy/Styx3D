/****************************************************************************************/
/*  D3D_Main.cpp                                                                        */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: DD/D3D wrapper                                                         */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
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
#include <Windows.h>
#include <Assert.h>
#include <stdio.h>
#include <DDraw.h>
#include <D3D.h>
#include <Math.h>

#include "D3D_Main.h"
#include "D3D_Err.h"
#include "D3D_fx.h"
#include "d3dcache.h"

#include "d3d_Render.h"
#include "D3DCache.h"
#include "d3d_THandle.h"
#include "d3d_PCache.h"
#include "d3d_scene.h"
#include "D3DDrv.h"

#undef ATTEMPT
#define ATTEMPT(x) if (!(x)) goto exit_with_error

#undef RELEASE
#define RELEASE(x) if (x) { x->Release(); x = NULL; }

//================================================================================
//	Globals
//================================================================================
D3DMain_GlobalInfo	D3DInfo;				// Our global structure that knows all... (once initialized)

#define MAX_DRIVERS		64

typedef struct
{
	jeBoolean	IsPrimary;
	GUID		Guid;
	char		Name[MAX_DRIVER_NAME];
} D3DMain_DDEnum;

typedef struct
{
	int32			NumDrivers;
	D3DMain_DDEnum	*Drivers;
} D3DMain_DDEnumInfo;

static D3DMain_DDEnum	Drivers[MAX_DRIVERS];

//================================================================================
//	Local static functions
//================================================================================
static BOOL D3DMain_CreateD3D(void);
static BOOL D3DMain_EnumDevices(void);
static BOOL D3DMain_ClearBuffers(void);
static BOOL OutputDriverInfo(const char *Filename, DDMain_D3DDriver *Driver);
static BOOL D3DMain_RememberOldMode(HWND hWnd);
static BOOL D3DMain_SetDisplayMode(HWND hWnd, int w, int h, int bpp, BOOL FullScreen);
static BOOL D3DMain_PickDevice(void);
static BOOL D3DMain_CreateDevice(void);
static BOOL D3DMain_CreateBuffers(void);
static void D3DMain_DestroyBuffers(void);
static BOOL D3DMain_CreateZBuffer(void);
static void D3DMain_DestroyZBuffer(void);
static jeBoolean D3DMain_SetDefaultRenderStates(void);
static BOOL D3DMain_RestoreDisplayMode(void);
static BOOL D3DMain_CreateDDFromName(const char *DriverName);
static jeBoolean CreateDDFromDriver(D3DMain_DDEnum *pDriver);
static jeBoolean CreateDDFromName(const char *DriverName, const D3DMain_DDEnumInfo *Info);

BOOL D3DMain_RestoreAllSurfaces(void)
{
	HRESULT	ddrval;

#ifdef _DEBUG
	OutputDebugString("--- D3DMain_RestoreAllSurfaces ---\n");
#endif
	
	if (D3DInfo.lpDD)
	{
		if (!D3DMain_SetDisplayMode(D3DInfo.hWnd, D3DInfo.CurrentWidth, D3DInfo.CurrentHeight, D3DInfo.CurrentBpp, D3DInfo.FullScreen))
			return FALSE;

		// Restore all the surfaces
		ddrval = D3DInfo.lpDD->RestoreAllSurfaces();

		if(ddrval!=DD_OK)
		{
			D3DMain_Log("D3DMain_RestoreAllSurfaces: D3DInfo.lpDD->RestoreAllSurfaces() failed:\n  %s\n", D3DErrorToString(ddrval));
			return	FALSE;
		}
	}

	if ( ! THandle_EvictAll() )
		return FALSE;

	return TRUE;
}

//================================================================================
//	BPPToDDBD
//	Convert an integer bit per pixel number to a DirectDraw bit depth flag
//================================================================================
static DWORD BPPToDDBD(int bpp)
{
	switch(bpp) 
	{
		case 1:
			return DDBD_1;
		case 2:
			return DDBD_2;
		case 4:
			return DDBD_4;
		case 8:
			return DDBD_8;
		case 16:
			return DDBD_16;
		case 24:
			return DDBD_24;
		case 32:
			return DDBD_32;
		default:
			assert(!"BOGUS bpp");
	}	

	return DDBD_1;		// Shutup compiler warning
}

//================================================================================
//	D3DMain_InitD3D
//	Does all what is needed to get an app ready to go at a specified with height
//	NOTE - It only makes 16 bit modes availible
//================================================================================
BOOL D3DMain_InitD3D(HWND hWnd, const char *DriverName, int32 Width, int32 Height)
{
	SYSTEMTIME		Time;

	memset(&D3DInfo, 0, sizeof(D3DMain_GlobalInfo));
	memset(Drivers, 0, sizeof(Drivers));
	
	GetSystemTime(&Time);
		
	//unlink(D3DMAIN_LOG_FILENAME);

	D3DMain_Log("=================================================================\n");
	D3DMain_Log(" D3DInfo v%i.%i\n", DRV_VERSION_MAJOR, DRV_VERSION_MINOR);
	//	D3DMain_Log(" Build Date: "__DATE__", Time: "__TIME__"\n"); // by trilobite
	D3DMain_Log((" Build Date:%s, Time: %s", __DATE__, __TIME__)); // by trilobite
	D3DMain_Log("=================================================================\n\n");

	D3DMain_Log("Current Time: %2i:%2i:%2i\n", Time.wHour, Time.wMinute, Time.wSecond);
	D3DMain_Log("Current Date: %2i-%2i-%4i\n", Time.wMonth, Time.wDay, Time.wYear);
	D3DMain_Log("\n ** D3D Driver Initializing **\n\n");

	D3DInfo.hWnd = hWnd;

	// Create DD
	ATTEMPT(D3DMain_CreateDDFromName(DriverName));
	
	ATTEMPT(D3DMain_GetTextureMemory());

	// We must do this after the DD object is created!!!
	ATTEMPT(D3DMain_RememberOldMode(hWnd));		// Store old mode

	// Get available fullscreen display modes
	ATTEMPT(D3DMain_EnumDisplayModes());

	// Create D3D, and enum it's devices
	ATTEMPT(D3DMain_CreateD3D());
	ATTEMPT(D3DMain_EnumDevices());

	if (Width == -1 && Height == -1)		// Window Mode
	{
		// Force Width/Height to client window area size
		Width = D3DInfo.OldWindowWidth;
		Height = D3DInfo.OldWindowHeight;
		
		ATTEMPT(D3DMain_SetDisplayMode(hWnd, Width, Height, D3DInfo.OldBpp, FALSE));
	}
	else
	{
		//ATTEMPT(D3DMain_SetDisplayMode(hWnd, Width, Height, 16, TRUE)); // 16 bpp is a hack that excludes modern video cards. Won't run in fullscreen. trilobite
		//ATTEMPT(D3DMain_SetDisplayMode(hWnd, Width, Height, 32, TRUE));  // 32 bpp works for newer graphics cards that don't support 16 bpp rendering... but still a hack
		ATTEMPT(D3DMain_SetDisplayMode(hWnd, Width, Height, D3DInfo.Modes->Bpp, TRUE));	// getting bpp info from unum-ed modes from current d3d driver.
	}

	// Pick a device we will be happy with
	ATTEMPT(D3DMain_PickDevice());

	// Create front/back buffer
	ATTEMPT(D3DMain_CreateBuffers());		
	
	// For some reason, we have to create the zbuffer BEFORE the device??? Why???
	ATTEMPT(D3DMain_CreateZBuffer());

	// Create the device
	ATTEMPT(D3DMain_CreateDevice());

	// Get the surface formats for textures, and 2d surfaces
	ATTEMPT(D3DMain_GetSurfaceFormats());

#if 1
	D3DInfo.CanDoMultiTexture = (D3DInfo.Drivers[D3DInfo.CurrentDriver].MaxSimultaneousTextures > 1) ? JE_TRUE : JE_FALSE;
#else
	D3DInfo.CanDoMultiTexture = JE_FALSE;
#endif

	D3DMain_Log("--- D3DMain_SetRenderState --- \n");
    
	ATTEMPT(D3DMain_SetDefaultRenderStates());

	D3DInfo.RenderingIsOK = TRUE;

	ATTEMPT(D3DMain_ClearBuffers());
	ATTEMPT(D3DMain_GetTextureMemory());

	if (!THandle_Startup())
		return JE_FALSE;

	D3DMain_Log("\n ** Initialization was successful **\n\n");

	return TRUE;

	exit_with_error:;
		D3DMain_Log(" ** Initialization was NOT successful **\n");
		D3DMain_ShutdownD3D();
		return FALSE;
}

//================================================================================
//	D3DMain_ShutdownD3D
//================================================================================
BOOL D3DMain_ShutdownD3D(void)
{
	D3DMain_Log("\n--- D3DMain_ShutdownD3D ---\n");

	THandle_Shutdown();

	// Destroys all objects including Direct Draw.
	D3DInfo.RenderingIsOK = FALSE;

	RELEASE(D3DInfo.lpD3DDevice);

	if (D3DInfo.lpZBuffer)
	{
		assert(D3DInfo.lpBackBuffer);
		D3DInfo.lpBackBuffer->DeleteAttachedSurface(0, D3DInfo.lpZBuffer);
		RELEASE(D3DInfo.lpZBuffer);
	}

	if (D3DInfo.lpFrontBuffer)
		D3DInfo.lpFrontBuffer->SetClipper(NULL);

	RELEASE(D3DInfo.lpClipper);
	RELEASE(D3DInfo.lpBackBuffer);
	RELEASE(D3DInfo.lpFrontBuffer);

	D3DMain_RestoreDisplayMode();

	RELEASE(D3DInfo.lpD3D);
	RELEASE(D3DInfo.lpDD);

	memset(&D3DInfo, 0, sizeof(D3DMain_GlobalInfo));

	D3DMain_Log("  Shutdown was successful...\n\n");

	return TRUE;
}

//================================================================================
//================================================================================
jeBoolean D3DMain_Reset(void)
{
	D3DMain_Log("\n--- D3DMain_Reset ---\n");

	THandle_Shutdown();
	PCache_Reset();

	if (!THandle_Startup())
		return JE_FALSE;

	Scene_CurrentFrame = 0;

	return JE_TRUE;
}

//================================================================================
//	D3DMain_Log
//================================================================================
void D3DMain_Log(LPSTR Str, ... )
{
	char	Buffer[2048];
	FILE	*f;

	wvsprintf(Buffer, Str, (char*)(&Str+1));

	f = fopen(D3DMAIN_LOG_FILENAME, "a+t");

	if (!f)
		return;

	fprintf(f, "%s", Buffer);

	fclose(f);
}

//================================================================================
//	CompareModes
//================================================================================
static int CompareModes(const void* element1, const void* element2) 
{
	App_Mode *lpMode1, *lpMode2;
  
	lpMode1 = (App_Mode*)element1;
	lpMode2 = (App_Mode*)element2;
  
	if (lpMode1->Bpp > lpMode2->Bpp)
		return -1;
	else if (lpMode2->Bpp > lpMode1->Bpp)
		return 1;
	else if (lpMode1->Width > lpMode2->Width)
		return -1;
	else if (lpMode2->Width > lpMode1->Width)
		return 1;
	else if (lpMode1->Height > lpMode2->Height)
		return -1;
	else if (lpMode2->Height > lpMode1->Height)
		return 1;
	else
		return 0;
}

//================================================================================
//	EnumDisplayModesCallback
//================================================================================
static HRESULT CALLBACK EnumDisplayModesCallback(LPDDSURFACEDESC2 pddsd, LPVOID lpContext)
{
	App_Mode		*pMode;

	if (!pddsd)
		return DDENUMRET_OK;
		
	/** <> CB; nyet!
	if (pddsd->dwWidth > 1024 || pddsd->dwHeight > 768)
		return DDENUMRET_OK;
	**/
	if (pddsd->dwWidth > 2048 || pddsd->dwHeight > 1024)
		return DDENUMRET_OK;

	if (D3DInfo.NumModes >= MAX_APP_MODES)
		return DDENUMRET_CANCEL;

	pMode = &D3DInfo.Modes[D3DInfo.NumModes++];

	// Save this mode at the end of the mode array and increment mode count
	pMode->Width = pddsd->dwWidth;
	pMode->Height = pddsd->dwHeight;
	pMode->Bpp = pddsd->ddpfPixelFormat.dwRGBBitCount;
	pMode->ThisDriverCanDo = FALSE;

	return DDENUMRET_OK;
}

//================================================================================
//	D3DMain_EnumDisplayModes
//================================================================================
BOOL D3DMain_EnumDisplayModes(void)
{
	HRESULT		LastError;

	D3DMain_Log("--- D3DMain_EnumDisplayModes ---\n");

	// Get a list of available display modes from DirectDraw
	D3DInfo.NumModes = 0;
  
	LastError = D3DInfo.lpDD->EnumDisplayModes(0, NULL, 0, EnumDisplayModesCallback);

	if(LastError != DD_OK ) 
	{
		D3DMain_Log("EnumDisplayModes failed.\n  %s\n", D3DErrorToString(LastError));
		D3DInfo.NumModes = 0;
		return FALSE;
	}

	// Sort the list of display modes
	qsort((void *)&D3DInfo.Modes[0], (size_t)D3DInfo.NumModes, sizeof(App_Mode), CompareModes);
	
	return TRUE;
}

//================================================================================
//	D3DMain_CreateD3D
//================================================================================
static BOOL D3DMain_CreateD3D(void)
{
	HRESULT		LastError;
	
	assert(D3DInfo.lpDD);

	D3DMain_Log("--- D3DMain_CreateD3D ---\n");

	LastError = D3DInfo.lpDD->QueryInterface(IID_IDirect3D7, (LPVOID*)&D3DInfo.lpD3D);

	if (LastError != DD_OK) 
	{
		D3DMain_Log("Creation of IDirect3D7 failed.\n  %s\n", D3DErrorToString(LastError));
		goto exit_with_error;
	}

	return TRUE;

	exit_with_error:
		return FALSE;
}
/*
#define MUST_BLEND (D3DPBLENDCAPS_BOTHINVSRCALPHA | \
					D3DPBLENDCAPS_BOTHSRCALPHA | \
					D3DPBLENDCAPS_DESTALPHA | \
					D3DPBLENDCAPS_DESTCOLOR | \
					D3DPBLENDCAPS_INVDESTALPHA | \
					D3DPBLENDCAPS_INVDESTCOLOR | \
					D3DPBLENDCAPS_INVSRCALPHA | \
					D3DPBLENDCAPS_INVSRCCOLOR | \
					D3DPBLENDCAPS_ONE | \
					D3DPBLENDCAPS_SRCALPHA | \
					D3DPBLENDCAPS_SRCALPHASAT | \
					D3DPBLENDCAPS_SRCCOLOR | \
					D3DPBLENDCAPS_ZERO)
*/
#if 0
#define MUST_BLEND_SRC	(D3DPBLENDCAPS_SRCALPHA | \
						D3DPBLENDCAPS_INVSRCALPHA | \
						D3DPBLENDCAPS_DESTCOLOR | \
						D3DPBLENDCAPS_ONE | \
						D3DPBLENDCAPS_ZERO)

#define MUST_BLEND_DEST	(D3DPBLENDCAPS_SRCALPHA | \
						D3DPBLENDCAPS_INVSRCALPHA | \
						D3DPBLENDCAPS_SRCCOLOR | \
						D3DPBLENDCAPS_ONE | \
						D3DPBLENDCAPS_ZERO)

#else
#define MUST_BLEND_SRC	(D3DPBLENDCAPS_SRCALPHA | \
						D3DPBLENDCAPS_DESTCOLOR | \
						D3DPBLENDCAPS_ONE | \
						D3DPBLENDCAPS_ZERO)

#define MUST_BLEND_DEST	(D3DPBLENDCAPS_INVSRCALPHA | \
						D3DPBLENDCAPS_ONE | \
						D3DPBLENDCAPS_ZERO)
#endif

//================================================================================
//	D3DEnumDevicesCallback7
//================================================================================
static HRESULT CALLBACK D3DEnumDevicesCallback7(
						LPSTR				lpDeviceDescription, 
						LPSTR				lpDeviceName, 
						LPD3DDEVICEDESC7	lpD3DDeviceDesc,
						LPVOID				lpContext)		
{
	DDMain_D3DDriver	*Driver;	
	BOOL				Good;

	if (!lpDeviceDescription || !lpDeviceName || !lpD3DDeviceDesc)
		return (D3DENUMRET_OK);

	if (strlen(lpDeviceDescription) >= MAX_DRIVER_NAME)
		return (D3DENUMRET_OK);

	if (strlen(lpDeviceName) >= MAX_DRIVER_NAME)
		return (D3DENUMRET_OK);

	if (D3DInfo.NumDrivers >= DDMAIN_MAX_D3D_DRIVERS)
		return (D3DENUMRET_CANCEL);

	if (!memcmp(&lpD3DDeviceDesc->deviceGUID, &IID_IDirect3DRGBDevice, sizeof (GUID)))
		return D3DENUMRET_OK;		// We only want HW devices

	lpContext = lpContext;

	Good = TRUE;

	D3DInfo.CurrentDriver = D3DInfo.NumDrivers;

	Driver = &D3DInfo.Drivers[D3DInfo.NumDrivers];
	
	// Record the D3D driver's inforamation
	memcpy(&Driver->Guid, &lpD3DDeviceDesc->deviceGUID, sizeof(GUID));
	lstrcpy(Driver->About, lpDeviceDescription);
	lstrcpy(Driver->Name, lpDeviceName);

	memcpy(&Driver->Desc, lpD3DDeviceDesc, sizeof(D3DDEVICEDESC7));

	Driver->IsHardware = TRUE;

    Driver->MaxTextureBlendStages = lpD3DDeviceDesc->wMaxTextureBlendStages;
    Driver->MaxSimultaneousTextures = lpD3DDeviceDesc->wMaxSimultaneousTextures;	

	if (!(lpD3DDeviceDesc->dwDeviceZBufferBitDepth))
		Good = FALSE;
	else
		Driver->DoesZBuffer = TRUE;

	if (!(lpD3DDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE))
		Good = FALSE;
	else
		Driver->DoesTextures = TRUE;

	// Skip if it does not support alpha blending
	if (!(lpD3DDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHA))
		Good = FALSE;
	else
		Driver->DoesAlpha = TRUE;

	if (!(lpD3DDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY))
		Good = FALSE;
	else
		Driver->DoesTransparency = TRUE;
		
	Driver->DoesClamping = TRUE;

	if ((lpD3DDeviceDesc->dpcTriCaps.dwSrcBlendCaps & MUST_BLEND_SRC) != MUST_BLEND_SRC)
		Good = FALSE;
	else
		Driver->DoesSrcBlending = TRUE;

	if ((lpD3DDeviceDesc->dpcTriCaps.dwDestBlendCaps & MUST_BLEND_DEST) != MUST_BLEND_DEST)
		Good = FALSE;
	else
		Driver->DoesDestBlending = TRUE;

	// Stop as soon as we find a driver that can render into a window
	if ((lpD3DDeviceDesc->dwDeviceRenderBitDepth & BPPToDDBD(D3DInfo.OldBpp)) && D3DInfo.IsPrimary && Good)
	{
		Driver->CanDoWindow = TRUE;
		D3DInfo.CanDoWindow = TRUE;
	}
	else
		Driver->CanDoWindow = FALSE;

	// Store if we can use this driver
	Driver->CanUse = Good;

	if (!Good)
		return (D3DENUMRET_OK);

	// Tell global structure that we found a good device
	D3DInfo.FoundGoodDevice = TRUE;
	
	// If all was good, increment the number of drivers
	D3DInfo.NumDrivers++;

	return (D3DENUMRET_OK);
}

//================================================================================
//	EnumDevices
//================================================================================
static BOOL D3DMain_EnumDevices(void)
{
	HRESULT		LastError;

	D3DMain_Log("--- D3DMain_EnumDevices ---\n");

	D3DInfo.NumDrivers = 0;

	LastError = D3DInfo.lpD3D->EnumDevices(D3DEnumDevicesCallback7, NULL);

	if (LastError != DD_OK) 
	{
		D3DMain_Log("Enumeration of drivers failed.\n  %s\n", D3DErrorToString(LastError));
		return FALSE;
	}

	D3DInfo.CurrentDriver = 0;

	return TRUE;
}

//================================================================================
//	CreateSurface
//================================================================================
static HRESULT CreateSurface(LPDDSURFACEDESC2 lpDDSurfDesc, LPDIRECTDRAWSURFACE7 FAR *lpDDSurface) 
{
	HRESULT Result;
	
	//if (D3DInfo.OnlySystemMemory)
	//	lpDDSurfDesc->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	Result = D3DInfo.lpDD->CreateSurface(lpDDSurfDesc, lpDDSurface, NULL);
	
	return Result;
}

//================================================================================
//	GetSurfDesc
//================================================================================
static HRESULT GetSurfDesc(LPDDSURFACEDESC2 lpDDSurfDesc, LPDIRECTDRAWSURFACE7 lpDDSurf)
{
	HRESULT Result;
	
	memset(lpDDSurfDesc, 0, sizeof(DDSURFACEDESC2));
	
	lpDDSurfDesc->dwSize = sizeof(DDSURFACEDESC2);
	
	Result = lpDDSurf->GetSurfaceDesc(lpDDSurfDesc);
	
	return Result;
}

//================================================================================
//	EnumTextureFormatsCallback
//	Record information about each texture format the current D3D driver can
//	support. Choose one as the default format and return it through lpContext.
//================================================================================
static HRESULT CALLBACK EnumTextureFormatsCallback(LPDDPIXELFORMAT lpddpfPixelFormat, LPVOID lpContext)
{
	
	DDMain_SurfFormat	*pTexFormat;
	
	if(!lpddpfPixelFormat)
		return DDENUMRET_OK;

	if (D3DInfo.NumTextureFormats+1 >= DDMAIN_MAX_TEXTURE_FORMATS )
	{
		return DDENUMRET_CANCEL;
	}

	pTexFormat = &D3DInfo.TextureFormats[D3DInfo.NumTextureFormats];

	// Clear out this texture format slot
	memset(pTexFormat, 0, sizeof(DDMain_SurfFormat));

	if(lpddpfPixelFormat->dwFlags & DDPF_ALPHAPIXELS) 
	{
		if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0x8000) 
		{
			if(lpddpfPixelFormat->dwRBitMask != 0x7c00 || 
				lpddpfPixelFormat->dwGBitMask != 0x3e0 ||
				lpddpfPixelFormat->dwBBitMask != 0x1f)
					return DDENUMRET_OK;
				
			pTexFormat->HasOneBitAlpha = TRUE;
			pTexFormat->HasFourBitAlpha = FALSE;
		}
		else if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0xf000) 
		{
			if(lpddpfPixelFormat->dwRBitMask != 0xf00 || 
				lpddpfPixelFormat->dwGBitMask != 0xf0 ||
				lpddpfPixelFormat->dwBBitMask != 0xf)
					return DDENUMRET_OK;

			pTexFormat->HasOneBitAlpha = FALSE;
			pTexFormat->HasFourBitAlpha = TRUE;
		}
		else
		{
			pTexFormat->HasOneBitAlpha = FALSE;
			pTexFormat->HasFourBitAlpha = FALSE;
		}
	}
	else 
	{
		if(!(lpddpfPixelFormat->dwFlags & DDPF_RGB))
			return DDENUMRET_OK;
		
		#if 0
		if(lpddpfPixelFormat->dwRGBBitCount != 16) 
			return DDENUMRET_OK;

		if(	(lpddpfPixelFormat->dwRBitMask != 0xf800 && lpddpfPixelFormat->dwRBitMask != 0x7c00) ||
			(lpddpfPixelFormat->dwGBitMask != 0x7e0 && lpddpfPixelFormat->dwGBitMask != 0x3e0) ||
			(lpddpfPixelFormat->dwBBitMask != 0x1f))
				return DDENUMRET_OK;
		#endif

		pTexFormat->HasOneBitAlpha = FALSE;
		pTexFormat->HasFourBitAlpha = FALSE;
	}

	// Record the PixelFormat of this texture
	memcpy(&pTexFormat->ddsd.ddpfPixelFormat, lpddpfPixelFormat, sizeof(DDPIXELFORMAT));

	D3DInfo.NumTextureFormats++;

	return DDENUMRET_OK;
}

//================================================================================
//	Main_EnumTextureFormats
//	Get a list of available texture map formats from the Direct3D driver by
//	enumeration.  Choose a default format.
//================================================================================
BOOL Main_EnumTextureFormats(void)
{
	HRESULT	LastError;

	assert(D3DInfo.lpD3DDevice);

	D3DInfo.NumTextureFormats = 0;

	LastError = D3DInfo.lpD3DDevice->EnumTextureFormats(EnumTextureFormatsCallback, NULL);
	
	if (LastError != DD_OK) 
	{
		D3DMain_Log("Main_EnumTextureFormats:  Enumeration of texture formats failed.\n  %s\n",
			D3DErrorToString(LastError));
		return FALSE;
	}

	return TRUE;
}


//================================================================================
//	EnumSurfaceFormatsCallback
//================================================================================
HRESULT WINAPI EnumSurfaceFormatsCallback(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext)
{
	LPDDPIXELFORMAT		lpddpfPixelFormat;
	DDMain_SurfFormat	*pSurfFormat;

	// Don't need this.
	RELEASE(lpDDSurface);
	
	lpddpfPixelFormat = &lpDDSurfaceDesc->ddpfPixelFormat;

	if(!lpddpfPixelFormat)
		return DDENUMRET_OK;

	if (D3DInfo.NumSurfFormats+1 >= DDMAIN_MAX_SURFACE_FORMATS )
		return DDENUMRET_CANCEL;

	pSurfFormat = &D3DInfo.SurfFormats[D3DInfo.NumSurfFormats];

	// Clear out this texture format slot
	memset(pSurfFormat, 0, sizeof(DDMain_SurfFormat));

	if(lpddpfPixelFormat->dwFlags & DDPF_ALPHAPIXELS) 
	{
		if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0x8000) 
		{
			// 1555
			if(lpddpfPixelFormat->dwRBitMask != 0x7c00 || 
				lpddpfPixelFormat->dwGBitMask != 0x3e0 ||
				lpddpfPixelFormat->dwBBitMask != 0x1f)
					return DDENUMRET_OK;
				
			pSurfFormat->HasOneBitAlpha = TRUE;
			pSurfFormat->HasFourBitAlpha = FALSE;
		}
		else if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0xf000) 
		{
			// 4444
			if(lpddpfPixelFormat->dwRBitMask != 0xf00 || 
				lpddpfPixelFormat->dwGBitMask != 0xf0 ||
				lpddpfPixelFormat->dwBBitMask != 0xf)
					return DDENUMRET_OK;

			pSurfFormat->HasOneBitAlpha = FALSE;
			pSurfFormat->HasFourBitAlpha = TRUE;
		}
		else
		{
			pSurfFormat->HasOneBitAlpha = FALSE;
			pSurfFormat->HasFourBitAlpha = FALSE;
		}
	}
	else 
	{
		if(!(lpddpfPixelFormat->dwFlags & DDPF_RGB))
			return DDENUMRET_OK;

		pSurfFormat->HasOneBitAlpha = FALSE;
		pSurfFormat->HasFourBitAlpha = FALSE;
	}

	// Record the PixelFormat of this texture
	memcpy(&pSurfFormat->ddsd.ddpfPixelFormat, lpddpfPixelFormat,sizeof(DDPIXELFORMAT));

	D3DInfo.NumSurfFormats++;

	return DDENUMRET_OK;
}
  
//================================================================================
//	Main_EnumSurfaceFormats
//================================================================================
BOOL Main_EnumSurfaceFormats(void)
{
	HRESULT		LastError;

	assert(D3DInfo.lpDD);

	D3DInfo.NumSurfFormats = 0;

	LastError = D3DInfo.lpDD->EnumSurfaces(DDENUMSURFACES_DOESEXIST|DDENUMSURFACES_ALL,
		NULL, NULL, EnumSurfaceFormatsCallback);
	
	if (LastError != DD_OK) 
	{
		D3DMain_Log("Main_EnumSurfaceFormats:  Enumeration of texture formats failed.\n  %s\n",
			D3DErrorToString(LastError));
		return FALSE;
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
// Name: EnumZBufferFormatsCallback()
// Desc: Enumeration function to report valid pixel formats for z-buffers.
//-----------------------------------------------------------------------------
static HRESULT WINAPI EnumZBufferFormatsCallback( DDPIXELFORMAT* pddpf,
                                                  VOID* pddpfDesired )
{
    if( NULL==pddpf || NULL==pddpfDesired )
        return D3DENUMRET_CANCEL;

    // If the current pixel format's match the desired ones (DDPF_ZBUFFER and
    // possibly DDPF_STENCILBUFFER), lets copy it and return. This function is
    // not choosy...it accepts the first valid format that comes along.
    if( pddpf->dwFlags == ((DDPIXELFORMAT*)pddpfDesired)->dwFlags )
    {
        memcpy( pddpfDesired, pddpf, sizeof(DDPIXELFORMAT) );

		// We're happy with a 16-bit z-buffer. Otherwise, keep looking.
		if( pddpf->dwZBufferBitDepth == 16 )
			return D3DENUMRET_CANCEL;
    }

    return D3DENUMRET_OK;
}

//================================================================================
//	D3DMain_ClearBuffers
//================================================================================
static BOOL D3DMain_ClearBuffers(void)
{
	DDSURFACEDESC2	ddsd;
	RECT			dst;
	DDBLTFX			ddbltfx;
	HRESULT			LastError;

	// Find the width and height of the front buffer by getting its
	// DDSURFACEDESC2
	if (D3DInfo.lpFrontBuffer) 
	{
		LastError = GetSurfDesc(&ddsd, D3DInfo.lpFrontBuffer);
		if (LastError != DD_OK) 
		{
			D3DMain_Log("D3DMain_ClearBuffers:  Failure getting the surface description of the front buffer before clearing.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
    
		// Clear the front buffer to black
		memset(&ddbltfx, 0, sizeof(ddbltfx));
		ddbltfx.dwSize = sizeof(DDBLTFX);
		SetRect(&dst, 0, 0, ddsd.dwWidth, ddsd.dwHeight);
    
		LastError = D3DInfo.lpFrontBuffer->Blt(&dst, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, 
							&ddbltfx);
    
		if (LastError != DD_OK) 
		{
			if(LastError==DDERR_SURFACELOST)
			{
				if (!D3DMain_RestoreAllSurfaces())
				{
					D3DMain_Log("D3DMain_ClearBuffers:  D3DMain_RestoreAllSurfaces failed...\n");
					return FALSE;
				}
			}
			else
			{
				D3DMain_Log("D3DMain_ClearBuffers:  Clearing the front buffer failed.\n  %s\n",
					D3DErrorToString(LastError));
				return FALSE;
			}
		}
	}
	
	if (D3DInfo.lpBackBuffer) 
	{
		// Find the width and height of the back buffer by getting its
		// DDSURFACEDESC2
		
		LastError = GetSurfDesc(&ddsd, D3DInfo.lpBackBuffer);
		
		if (LastError != DD_OK) 
		{
			D3DMain_Log("D3DMain_ClearBuffers:  Failure while getting the surface description of the back buffer before clearing.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
		
		// Clear the back buffer to black
		memset(&ddbltfx, 0, sizeof(ddbltfx));
		ddbltfx.dwSize = sizeof(DDBLTFX);
		SetRect(&dst, 0, 0, ddsd.dwWidth, ddsd.dwHeight);
		LastError = D3DInfo.lpBackBuffer->Blt(&dst, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT,
					&ddbltfx);

		if (LastError != DD_OK) 
		{
			if(LastError==DDERR_SURFACELOST)
			{
				if (!D3DMain_RestoreAllSurfaces())
				{
					D3DMain_Log("D3DMain_ClearBuffers:  D3DMain_RestoreAllSurfaces failed.\n");
					return FALSE;
				}
			}
			else
			{
				D3DMain_Log("D3DMain_ClearBuffers:  Clearing the back buffer failed.\n  %s\n",
					D3DErrorToString(LastError));
				return FALSE;
			}
		}
	}
	
	return TRUE;
}

//================================================================================
//	Main_ShowBackBuffer
//================================================================================
BOOL Main_ShowBackBuffer(void)
{
	HRESULT		LastError;

	if (!D3DInfo.RenderingIsOK) 
		return TRUE;
  
	if (D3DInfo.FullScreen) 
	{  
		// Flip the back and front buffers
		#if 1
			LastError = D3DInfo.lpFrontBuffer->Flip(D3DInfo.lpBackBuffer, DDFLIP_WAIT);
		#else
			LastError = D3DInfo.lpFrontBuffer->Flip(D3DInfo.lpBackBuffer, DDFLIP_NOVSYNC);
		#endif
		
		if (LastError == DDERR_SURFACELOST) 
		{
			D3DMain_RestoreAllSurfaces();
			//D3DInfo.lpFrontBuffer->Restore();
			//D3DInfo.lpBackBuffer->Restore();
			
			D3DMain_ClearBuffers();

		} 
		else if (LastError == DDERR_WASSTILLDRAWING)
		{
		}
		else if (LastError != DD_OK) 
		{
			D3DMain_Log("Flipping complex display surface failed.\n  %s\n", D3DErrorToString(LastError));
			return FALSE;
		}
	} 
	else 
	{
		RECT	FRect, BRect;

		FRect.left = D3DInfo.WindowXOffset;
		FRect.right = FRect.left + D3DInfo.CurrentWidth;
		FRect.top = D3DInfo.WindowYOffset;
		FRect.bottom = FRect.top + D3DInfo.CurrentHeight;

		BRect.left = 0;
		BRect.right = D3DInfo.CurrentWidth;
		BRect.top = 0;
		BRect.bottom = D3DInfo.CurrentHeight;

		LastError = D3DInfo.lpFrontBuffer->Blt(&FRect, D3DInfo.lpBackBuffer,
				&BRect, DDBLT_WAIT, NULL);

		if (LastError != DD_OK) 
		{
			if(LastError==DDERR_SURFACELOST)
			{
				if (!D3DMain_RestoreAllSurfaces())
				{
					D3DMain_Log("Main_ShowBackBuffer:  D3DMain_RestoreAllSurfaces.\n");
					return FALSE;
				}
			}
			else
			{
				D3DMain_Log("Main_ShowBackBuffer:  Blt of back buffer to front buffer failed.\n  %s\n", D3DErrorToString(LastError));
				return FALSE;
			}
		}
	}

	return TRUE;
}

//================================================================================
//	Main_ClearBackBuffer
//================================================================================
BOOL Main_ClearBackBuffer(BOOL Clear, BOOL ClearZ)
{
    int			ClearFlags;
    D3DRECT		Dummy;
	HRESULT		LastError;

	if (!D3DInfo.RenderingIsOK) 
		return TRUE;
	
    // Default to clear nothing
	ClearFlags = 0;

	// Then set in what callers wants to clear
	if (Clear)
		ClearFlags |= D3DCLEAR_TARGET;
	
	if (ClearZ)
		ClearFlags |= D3DCLEAR_ZBUFFER;

    Dummy.x1 = Dummy.y1 = 0;
    Dummy.x2 = D3DInfo.CurrentWidth;
    Dummy.y2 = D3DInfo.CurrentHeight;
    
	LastError = D3DInfo.lpD3DDevice->Clear(1, &Dummy, ClearFlags, 0, 1.0f, 0);
    
	if (LastError != D3D_OK) 
	{
		D3DMain_Log("Main_ClearBackBuffer: Viewport clear failed.\n  %s\n",
			D3DErrorToString(LastError));
		return FALSE;
	}

	return TRUE;
}

//================================================================================
//	Surface manipulation
//================================================================================

typedef struct
{
	unsigned char r, g, b;
} MY_D3D_RGB;

typedef struct
{
	DWORD	R_Shift;
	DWORD	G_Shift;
	DWORD	B_Shift;
	DWORD	A_Shift;

	DWORD	R_Mask;
	DWORD	G_Mask;
	DWORD	B_Mask;
	DWORD	A_Mask;

	DWORD	R_Width;
	DWORD	G_Width;
	DWORD	B_Width;
	DWORD	A_Width;
} D3D_PixelMask;

//================================================================================
//	GetSurfacePixelMask
//================================================================================
static void GetSurfacePixelMask(DDSURFACEDESC2 *ddsd, D3D_PixelMask *PixelMask)
{
	DWORD	red_mask, grn_mask, blu_mask, a_mask;
	DWORD	red_shift, grn_shift, blu_shift, a_shift;
	DWORD	red_width, grn_width, blu_width, a_width;

	red_mask = grn_mask = blu_mask = a_mask = 
	red_shift = grn_shift = blu_shift = a_shift = 
	red_width = grn_width = blu_width = a_width = 0;


	int		i;

	red_mask = ddsd->ddpfPixelFormat.dwRBitMask;
	grn_mask = ddsd->ddpfPixelFormat.dwGBitMask;
	blu_mask = ddsd->ddpfPixelFormat.dwBBitMask;
	a_mask = ddsd->ddpfPixelFormat.dwRGBAlphaBitMask;

	//
	// Derive shift, width values from masks
	//

	for (i=31; i >= 0; i--)
	{
		if (red_mask & (1 << i))
			red_shift = i;

		if (grn_mask & (1 << i))
			grn_shift = i;

		if (blu_mask & (1 << i))
			blu_shift = i;

		if (a_mask & (1 << i))
			a_shift = i;
	}

	for (i=0; i <= 31; i++)
	{
		if (red_mask & (1 << i))
			red_width = i - red_shift + 1;

		if (grn_mask & (1 << i))
			grn_width = i - grn_shift + 1;

		if (blu_mask & (1 << i))
			blu_width = i - blu_shift + 1;

		if (a_mask & (1 << i))
			a_width = i - a_shift + 1;
	}
	//
	// Pass all requested values back to the caller
	//

	PixelMask->R_Shift = red_shift;
	PixelMask->G_Shift = grn_shift;
	PixelMask->B_Shift = blu_shift;
	PixelMask->A_Shift = a_shift;

	PixelMask->R_Mask  = red_mask;
	PixelMask->G_Mask  = grn_mask;
	PixelMask->B_Mask  = blu_mask;
	PixelMask->A_Mask  = a_mask;

	PixelMask->R_Width = red_width;
	PixelMask->G_Width = grn_width;
	PixelMask->B_Width = blu_width;
	PixelMask->A_Width = a_width;
}

//================================================================================
//	MyRGB
//================================================================================
static unsigned int MyRGB(DWORD R, DWORD G, DWORD B, D3D_PixelMask *PixelMask)
{
   DWORD       R_Left, G_Left, B_Left;
   DWORD       R_Right, G_Right, B_Right;
   

   // Get shift constants for current video mode
   R_Left = PixelMask->R_Shift;
   G_Left = PixelMask->G_Shift;
   B_Left = PixelMask->B_Shift;

   R_Right = 8 - PixelMask->R_Width;
   G_Right = 8 - PixelMask->G_Width;
   B_Right = 8 - PixelMask->B_Width;
   // Shift R,G, and B into one value
   return( 
       (((((unsigned int) R) >> R_Right) << R_Left) & PixelMask->R_Mask) |
       (((((unsigned int) G) >> G_Right) << G_Left) & PixelMask->G_Mask) |
       (((((unsigned int) B) >> B_Right) << B_Left) & PixelMask->B_Mask)
   );
}

//==========================================================================================
//	D3DMain_GetSurfaceFormats
//==========================================================================================
BOOL D3DMain_GetSurfaceFormats(void)
{
	int32		i;

    D3DMain_Log("--- D3DMain_GetSurfaceFormats ---\n");
	
	if (!Main_EnumTextureFormats())
	{
        D3DMain_Log("D3DMain_GetSurfaceFormats:  Main_EnumTextureFormats failed.\n");
		return FALSE;
    }
	
	if (!Main_EnumSurfaceFormats())
	{
        D3DMain_Log("D3DMain_GetSurfaceFormats:  Main_EnumSurfaceFormats failed.\n");
		return FALSE;
	}

#if 1
	for(i = 0; i < D3DInfo.NumSurfFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

		lpddpfPixelFormat = &D3DInfo.SurfFormats[i].ddsd.ddpfPixelFormat;

		if(lpddpfPixelFormat->dwRGBBitCount != D3DInfo.ddsd.ddpfPixelFormat.dwRGBBitCount)
			continue;

        if (lpddpfPixelFormat->dwRGBAlphaBitMask != D3DInfo.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask)
			continue;
		if (lpddpfPixelFormat->dwRBitMask != D3DInfo.ddsd.ddpfPixelFormat.dwRBitMask)
			continue;
        if (lpddpfPixelFormat->dwGBitMask != D3DInfo.ddsd.ddpfPixelFormat.dwGBitMask)
			continue;
        if (lpddpfPixelFormat->dwBBitMask != D3DInfo.ddsd.ddpfPixelFormat.dwBBitMask)
			continue;

	#if 0	// For debugging (This is the surface it is going to use for 2d decals)
		D3DMain_Log("Bits: %i, A:%x, R:%x, G:%x, B:%x\n",	D3DInfo.ddsd.ddpfPixelFormat.dwRGBBitCount, 
															D3DInfo.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask, 
															D3DInfo.ddsd.ddpfPixelFormat.dwRBitMask, 
															D3DInfo.ddsd.ddpfPixelFormat.dwGBitMask, 
															D3DInfo.ddsd.ddpfPixelFormat.dwBBitMask);
		return FALSE;
	#endif

		
		D3DInfo.ddSurfFormat = D3DInfo.SurfFormats[i].ddsd;

		break;
	}

    if(i == D3DInfo.NumSurfFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find a 2d surface format that matches current bit depth.\n");
        return FALSE;
    }

#else
	for(i = 0; i < D3DInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

		lpddpfPixelFormat = &D3DInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

		if(lpddpfPixelFormat->dwRGBBitCount != D3DInfo.ddsd.ddpfPixelFormat.dwRGBBitCount)
			continue;

        if (lpddpfPixelFormat->dwRGBAlphaBitMask != D3DInfo.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask)
			continue;
		if (lpddpfPixelFormat->dwRBitMask != D3DInfo.ddsd.ddpfPixelFormat.dwRBitMask)
			continue;
        if (lpddpfPixelFormat->dwGBitMask != D3DInfo.ddsd.ddpfPixelFormat.dwGBitMask)
			continue;
        if (lpddpfPixelFormat->dwBBitMask != D3DInfo.ddsd.ddpfPixelFormat.dwBBitMask)
			continue;

	#if 0	// For debugging (This is the surface it is going to use for 2d decals)
		D3DMain_Log("Bits: %i, A:%x, R:%x, G:%x, B:%x\n",	D3DInfo.ddsd.ddpfPixelFormat.dwRGBBitCount, 
															D3DInfo.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask, 
															D3DInfo.ddsd.ddpfPixelFormat.dwRBitMask, 
															D3DInfo.ddsd.ddpfPixelFormat.dwGBitMask, 
															D3DInfo.ddsd.ddpfPixelFormat.dwBBitMask);
		return FALSE;
	#endif

		
		D3DInfo.ddSurfFormat = D3DInfo.TextureFormats[i].ddsd;

		break;
	}

    if(i == D3DInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find a 2d surface format that matches current bit depth.\n");
        return FALSE;
    }
#endif

	// Now get the 3d surface formats

	// Get 1555
	for(i = 0; i < D3DInfo.NumTextureFormats; i++)
	{
        if(D3DInfo.TextureFormats[i].HasOneBitAlpha == TRUE) 
		{
            D3DInfo.ddOneBitAlphaSurfFormat = D3DInfo.TextureFormats[i].ddsd;
            break;
        }
	}

    if(i == D3DInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 1555 texture support.\n");
        return FALSE;
    }
    
    // Get 4444
	for(i = 0; i < D3DInfo.NumTextureFormats; i++)
	{
        if(D3DInfo.TextureFormats[i].HasFourBitAlpha == TRUE) 
		{
            D3DInfo.ddFourBitAlphaSurfFormat = D3DInfo.TextureFormats[i].ddsd;
            break;
        }
	}

    if(i == D3DInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 4444 texture support.\n");
        return FALSE;
    }

	// Get either 555, or 565.
	for(i = 0; i < D3DInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

        if(D3DInfo.TextureFormats[i].HasOneBitAlpha == TRUE)
			continue;

		if (D3DInfo.TextureFormats[i].HasFourBitAlpha == TRUE) 
			continue;

		lpddpfPixelFormat = &D3DInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

		// For now, force 3d textures with RGB only info to be either 565 or 555
		// We could enum all formats and let the caller pick between several different RGB formats...
		if (lpddpfPixelFormat->dwFlags & DDPF_ALPHAPIXELS)
			continue;		// We don't want any surface that has alpha, just pure RGB...

		if(lpddpfPixelFormat->dwRGBBitCount != 16) 
			continue;

		if(	(lpddpfPixelFormat->dwRBitMask != 0xf800 && lpddpfPixelFormat->dwRBitMask != 0x7c00) ||
			(lpddpfPixelFormat->dwGBitMask != 0x7e0 && lpddpfPixelFormat->dwGBitMask != 0x3e0) ||
			(lpddpfPixelFormat->dwBBitMask != 0x1f))
				continue;


		// This is it
		D3DInfo.ddTexFormat = D3DInfo.TextureFormats[i].ddsd;
		break;
	}

    if(i == D3DInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 555 or 565 texture support.\n");
		return FALSE;
    }

	Main_BuildRGBGammaTables(1.0f);
	
	return TRUE;
}

//==========================================================================================
//	Main_CheckDD
//	Checks to see if current DD driver has any usable D3D Devices...
//==========================================================================================
BOOL Main_CheckDD(void)
{
	D3DInfo.NumDrivers = 0;
	D3DInfo.CurrentDriver = 0;
	D3DInfo.FoundGoodDevice = FALSE;
	D3DInfo.CanDoWindow = FALSE;

	assert(D3DInfo.lpDD);
	
	if (!D3DMain_RememberOldMode(GetDesktopWindow()))
		return FALSE;

	memset(D3DInfo.Drivers, 0, sizeof(DDMain_D3DDriver)*DDMAIN_MAX_D3D_DRIVERS);

	if (!D3DMain_CreateD3D())
		return FALSE;

	if (!D3DMain_EnumDevices())			// See if we can enumerate at least one good device for this DD Driver
		return FALSE;

	if (!D3DInfo.FoundGoodDevice)		// Return FALSE if not...
		return FALSE;

	return TRUE;	// Found at least one!!!
}

//==========================================================================================
//	OutputDriverInfo
//==========================================================================================
static BOOL OutputDriverInfo(const char *FileName, DDMain_D3DDriver *Driver)
{
	FILE			*f;
	SYSTEMTIME		Time;
	char			YesNo[2][10];

	f = fopen(FileName, "a+t");
      
	if (!f)
		return FALSE;

	GetSystemTime(&Time);

	strcpy(YesNo[0], "No\n");
	strcpy(YesNo[1], "Yes\n");
	
	fprintf(f,"=================================================================\n");
	fprintf(f,"Time: %2i:%2i:%2i\n", Time.wHour, Time.wMinute, Time.wSecond);
	fprintf(f,"Date: %2i-%2i-%4i\n", Time.wMonth, Time.wDay, Time.wYear);

	fprintf(f, "DirectDraw Name: \n");
	fprintf(f, "    %s\n", D3DInfo.DDName);

	fprintf(f, "D3D Driver Name: \n");
	fprintf(f, "    %s\n", Driver->Name);

	fprintf(f, "D3D Driver Description: \n");
	fprintf(f, "    %s\n", Driver->About);

	fprintf(f, "3D Acceleration        : %s", YesNo[Driver->IsHardware]);
	fprintf(f, "Texture Support        : %s", YesNo[Driver->DoesTextures]);
	fprintf(f, "Transparency Support   : %s", YesNo[Driver->DoesTransparency]);
	fprintf(f, "Alpha Support          : %s", YesNo[Driver->DoesAlpha]);
	fprintf(f, "UV Clamping Support    : %s", YesNo[Driver->DoesClamping]);
	fprintf(f, "Src Blending Support   : %s", YesNo[Driver->DoesSrcBlending]);
	fprintf(f, "Dest Blending Support  : %s", YesNo[Driver->DoesDestBlending]);
	fprintf(f, "Window Support         : %s", YesNo[Driver->CanDoWindow]);
	fprintf(f, "Can Use                : %s", YesNo[Driver->CanUse]);

	fclose(f);

	return TRUE;
}

//==========================================================================================
//	D3DMain_GetTextureMemory
//==========================================================================================
BOOL D3DMain_GetTextureMemory(void)
{

	DDSCAPS2		ddsCaps;
	DWORD			dwTotal;
	DWORD			dwFree;
	HRESULT			Error;

	D3DMain_Log("--- D3DMain_GetTextureMemory ---\n");

	memset(&ddsCaps, 0, sizeof(ddsCaps));

	//ddsCaps.dwSize = sizeof(DDSCAPS2);
	ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;

	Error = D3DInfo.lpDD->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree);

	if(Error !=DD_OK)
	{
		D3DMain_Log("Getting DD capabilities failed while checking total video memory.\n  %s\n", D3DErrorToString(Error));
		return FALSE;
	}

	D3DInfo.VidMemFree = dwFree;

	D3DMain_Log("  Ram free: %i\n", D3DInfo.VidMemFree);

	return TRUE;
}


//==========================================================================================
//==========================================================================================
void Main_BuildRGBGammaTables(float Gamma)
{
	int32				i, Val;
	int32				GammaTable[256];
	D3D_PixelMask	PixelMask;
	DWORD			R_Left, G_Left, B_Left, A_Left;
	DWORD			R_Right, G_Right, B_Right, A_Right;
	

	D3DInfo.Gamma = Gamma;

	if (Gamma == 1.0)
	{
	#ifdef USE_2X_MODULATION
		if (!(D3DInfo.dwTextureOpCaps & D3DTOP_MODULATE2X))
		{
			for (i=0 ; i<256 ; i++)
			{
				float Ratio = ((float)i+0.5f)/255.5f;

				float RGB = (float)(255.0 * pow((double)Ratio, 1.0/(double)3.0) + 0.5);
		
				if (RGB < 0.0f)
					RGB = 0.0f;
				if (RGB > 255.0f)
					RGB = 255.0f;

				GammaTable[i] = (int32)RGB;
				//GammaTable[i] = min(255, i<<1);
			}
		}
		else 
	#endif
		{
			for (i=0 ; i<256 ; i++)
				GammaTable[i] = i;
		}
	}
	else for (i=0 ; i<256 ; i++)
	{
		float Ratio = (i+0.5f)/255.5f;

		float RGB = (float)(255.0 * pow((double)Ratio, 1.0/(double)Gamma) + 0.5);
		
		if (RGB < 0.0f)
			RGB = 0.0f;
		if (RGB > 255.0f)
			RGB = 255.0f;

		GammaTable[i] = (int32)RGB;
	}

	GetSurfacePixelMask(&D3DInfo.ddTexFormat, &PixelMask);
	for (i=0; i< 256; i++)
	{
		// Get shift constants for current video mode/pixel format
		R_Left = PixelMask.R_Shift;
		G_Left = PixelMask.G_Shift;
		B_Left = PixelMask.B_Shift;
		A_Left = PixelMask.A_Shift;

		R_Right = 8 - PixelMask.R_Width;
		G_Right = 8 - PixelMask.G_Width;
		B_Right = 8 - PixelMask.B_Width;
		A_Right = 8 - PixelMask.A_Width;

		Val = GammaTable[i];

		D3DInfo.Lut1.R[i] = (((uint32)Val >> R_Right) << R_Left) & PixelMask.R_Mask;
		D3DInfo.Lut1.G[i] = (((uint32)Val >> G_Right) << G_Left) & PixelMask.G_Mask;
		D3DInfo.Lut1.B[i] = (((uint32)Val >> B_Right) << B_Left) & PixelMask.B_Mask;
		D3DInfo.Lut1.A[i] = (((uint32)  i >> A_Right) << A_Left) & PixelMask.A_Mask;
	}
	GetSurfacePixelMask(&D3DInfo.ddFourBitAlphaSurfFormat, &PixelMask);
	for (i=0; i< 256; i++)
	{
		// Get shift constants for current video mode/pixel format
		R_Left = PixelMask.R_Shift;
		G_Left = PixelMask.G_Shift;
		B_Left = PixelMask.B_Shift;
		A_Left = PixelMask.A_Shift;

		R_Right = 8 - PixelMask.R_Width;
		G_Right = 8 - PixelMask.G_Width;
		B_Right = 8 - PixelMask.B_Width;
		A_Right = 8 - PixelMask.A_Width;

		Val = GammaTable[i];

		D3DInfo.Lut2.R[i] = (((uint32)Val >> R_Right) << R_Left) & PixelMask.R_Mask;
		D3DInfo.Lut2.G[i] = (((uint32)Val >> G_Right) << G_Left) & PixelMask.G_Mask;
		D3DInfo.Lut2.B[i] = (((uint32)Val >> B_Right) << B_Left) & PixelMask.B_Mask;
		D3DInfo.Lut2.A[i] = (((uint32)  i >> A_Right) << A_Left) & PixelMask.A_Mask;
	}
	GetSurfacePixelMask(&D3DInfo.ddOneBitAlphaSurfFormat, &PixelMask);
	for (i=0; i< 256; i++)
	{
		// Get shift constants for current video mode/pixel format
		R_Left = PixelMask.R_Shift;
		G_Left = PixelMask.G_Shift;
		B_Left = PixelMask.B_Shift;
		A_Left = PixelMask.A_Shift;

		R_Right = 8 - PixelMask.R_Width;
		G_Right = 8 - PixelMask.G_Width;
		B_Right = 8 - PixelMask.B_Width;
		A_Right = 8 - PixelMask.A_Width;

		Val = GammaTable[i];

		D3DInfo.Lut3.R[i] = (((uint32)Val >> R_Right) << R_Left) & PixelMask.R_Mask;
		D3DInfo.Lut3.G[i] = (((uint32)Val >> G_Right) << G_Left) & PixelMask.G_Mask;
		D3DInfo.Lut3.B[i] = (((uint32)Val >> B_Right) << B_Left) & PixelMask.B_Mask;
		D3DInfo.Lut3.A[i] = (((uint32)  i >> A_Right) << A_Left) & PixelMask.A_Mask;
	}
}

//====================================================================================================
//	D3DMain_UpdateWindow
//====================================================================================================
jeBoolean DRIVERCC D3DMain_UpdateWindow(void)
{
	//D3DMain_GetClientWindowOffset(D3DInfo.hWnd);

	if (!D3DInfo.ModeSet)
		return JE_TRUE;

	if (D3DInfo.FullScreen)
		return JE_TRUE;

	//D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,false);
	// <> Pollard changes from 9/2 :

	RELEASE(D3DInfo.lpClipper);
	RELEASE(D3DInfo.lpZBuffer);
	RELEASE(D3DInfo.lpFrontBuffer);
	RELEASE(D3DInfo.lpBackBuffer);
	RELEASE(D3DInfo.lpD3DDevice);

	D3DMain_RememberOldMode(D3DInfo.hWnd);

	//ATTEMPT(D3DMain_SetDisplayMode(D3DInfo.hWnd, D3DInfo.OldWindowWidth, D3DInfo.OldWindowHeight, D3DInfo.OldBpp, JE_FALSE));
	
	D3DInfo.CurrentWidth  = D3DInfo.OldWindowWidth;
	D3DInfo.CurrentHeight = D3DInfo.OldWindowHeight;

	// Pick a device we will be happy with
	// ATTEMPT(D3DMain_PickDevice());

	// Create front/back buffer
	ATTEMPT(D3DMain_CreateBuffers());		
	
	// For some reason, we have to create the zbuffer BEFORE the device??? Why???
	ATTEMPT(D3DMain_CreateZBuffer());

	// Create the device
	ATTEMPT(D3DMain_CreateDevice());

	ATTEMPT(D3DMain_SetDefaultRenderStates());

	return JE_TRUE;

	// Error
	exit_with_error:
	{
		return JE_FALSE;
	}
}

//====================================================================================================
//	D3DMain_SetActive
//====================================================================================================
jeBoolean DRIVERCC D3DMain_SetActive(jeBoolean wParam)
{
	if (D3DInfo.lpFrontBuffer)
		D3DInfo.RenderingIsOK = wParam;

	if(D3DInfo.RenderingIsOK)							// Regaining focus
	{
		HRESULT	Result;

		if (D3DInfo.lpFrontBuffer)
		{
			OutputDebugString("Regaining Focus\n");

			Result = D3DInfo.lpFrontBuffer->IsLost();

			if(Result != DD_OK)
			{
				if(Result == DDERR_SURFACELOST)
				{
					if(!D3DMain_RestoreAllSurfaces())
					{
						OutputDebugString("Couldn't restore surfaces!\n");
						return JE_FALSE;
					}
		
					ShowWindow(D3DInfo.hWnd, SW_SHOWNORMAL);		// Dx doesn't restore it
				}
			}
			else
				OutputDebugString("Regained focus, no surfaces lost\n");
		}
	}

	return	JE_TRUE;
}


//================================================================================
//	D3DMain_GetClientWindowOffset
//================================================================================
BOOL D3DMain_GetClientWindowOffset(HWND hWnd)
{
	POINT			CPoint;

	CPoint.x = CPoint.y = 0;

	ClientToScreen(hWnd, &CPoint);

	D3DInfo.WindowXOffset = CPoint.x;
	D3DInfo.WindowYOffset = CPoint.y;

	return TRUE;
}

//================================================================================
//	D3DMain_RememberOldMode
//================================================================================
static BOOL D3DMain_RememberOldMode(HWND hWnd)
{
	DDSURFACEDESC2	ddsd;
	HRESULT			LastError;
	RECT			CRect;
  
	D3DMain_Log("--- D3DMain_RememberOldMode ---\n");

	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	
	LastError = D3DInfo.lpDD->GetDisplayMode(&ddsd);

	if (LastError != DD_OK) 
	{
		D3DMain_Log("Getting the current display mode failed.\n  %s\n", D3DErrorToString(LastError));
		return FALSE;
	}
	
	GetClientRect(hWnd, &CRect);

	// Get old fulscreen width/height/bpp
	D3DInfo.OldWidth = ddsd.dwWidth;
	D3DInfo.OldHeight = ddsd.dwHeight;
	D3DInfo.OldBpp = ddsd.ddpfPixelFormat.dwRGBBitCount;

	// Get old window width/pos
	D3DInfo.OldWindowWidth = CRect.right;
	D3DInfo.OldWindowHeight = CRect.bottom;

	GetWindowRect(hWnd, &CRect);
	D3DInfo.OldWindowRect = CRect;
	
//	D3DInfo.OldGWL_STYLE = GetWindowLong(hWnd, GWL_STYLE);

	D3DMain_GetClientWindowOffset(hWnd);

	return TRUE;
}

//==========================================================================================
//	D3DMain_SetDisplayMode
//==========================================================================================
static BOOL D3DMain_SetDisplayMode(HWND hWnd, int w, int h, int bpp, BOOL FullScreen)
{
	HRESULT	LastError;
	int		DWidth, DHeight;
	char	YN[2][32];

	strcpy(YN[0], "NO");
	strcpy(YN[1], "YES");
  
	D3DMain_Log("--- D3DMain_SetDisplayMode ---\n");
	D3DMain_Log("  W: %i, H: %i, Bpp: %i, FullScreen: %s\n", w, h, bpp, YN[FullScreen]);

	if (FullScreen)
	{
		LastError = D3DInfo.lpDD->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);// | DDSCL_ALLOWREBOOT);
		
		if(LastError != DD_OK ) 
		{
			D3DMain_Log("SetCooperativeLevel to fullscreen failed.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}

		D3DMain_Log("		SetCooperativeLevel: DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN succesfull...\n");

		LastError = D3DInfo.lpDD->SetDisplayMode(w, h, bpp,0,0);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("SetFullScreenDisplayMode:  Mode %dx%dx%d failed\n  %s\n", w, h, bpp, D3DErrorToString(LastError));
			return FALSE;
		}

		D3DMain_Log("		SetDisplayMode: succesfull...\n");

		DWidth = GetSystemMetrics(SM_CXSCREEN);
		DHeight = GetSystemMetrics(SM_CYSCREEN);

#if 0 //{
		//
		// Set window boundaries to cover entire desktop, and show it
		//
		SetWindowLong(hWnd, GWL_STYLE, D3DInfo.OldGWL_STYLE | WS_POPUP);
	
		SetWindowLong(hWnd, GWL_STYLE, D3DInfo.OldGWL_STYLE & 
											  ~(WS_OVERLAPPED  | 
												WS_CAPTION     | 
                                                WS_SYSMENU     | 
                                                WS_MINIMIZEBOX | 
                                                WS_MAXIMIZEBOX | 
                                                WS_THICKFRAME));


		SetWindowPos(D3DInfo.hWnd, 
		            HWND_TOP, 
		            0,
		            0,
		            DWidth,
		            DHeight,
		            SWP_NOCOPYBITS | SWP_NOZORDER);

		ShowWindow(D3DInfo.hWnd, SW_SHOWNORMAL);
#endif //}

	}
	else
	{
		LastError = D3DInfo.lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("SetCooperativeLevel to normal failed.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
	}

	D3DInfo.hWnd = hWnd;
	D3DInfo.CurrentWidth = w;
	D3DInfo.CurrentHeight = h;
	D3DInfo.CurrentBpp = bpp;
	D3DInfo.FullScreen = FullScreen;
   D3DInfo.WireFrame = FALSE;

	D3DInfo.ModeSet = JE_TRUE;

	return TRUE;
}

//================================================================================
//	D3DMain_PickDevice
//================================================================================
static BOOL D3DMain_PickDevice(void)
{
	int32	i;
	DWORD	Depths;

	D3DMain_Log("--- D3DMain_PickDevice ---\n");

	// Find a device with the same bpp as the mode set
	Depths = BPPToDDBD(D3DInfo.CurrentBpp);

	for (i = 0; i < D3DInfo.NumDrivers; i++)
	{
		if (!(D3DInfo.Drivers[i].IsHardware))		// ONLY hardware
			continue;

		// Only choose drivers that can support our draw buffer bpp
		if (!(D3DInfo.Drivers[i].Desc.dwDeviceRenderBitDepth & Depths))
			continue;

		// Only choose drivers that can create the zbuffer we need
		if (!(D3DInfo.Drivers[i].Desc.dwDeviceZBufferBitDepth & DDBD_16))
			continue;


		if (!D3DInfo.Drivers[i].CanDoWindow && !D3DInfo.FullScreen)
			continue;

		// Remember the current driver
		D3DInfo.CurrentDriver = i;

		return TRUE;
	}

	return FALSE;
}

char * stristr(const char *StrBase,const char *SubBase)
{

while ( *StrBase )
  {
  if ( toupper(*StrBase) == toupper(*SubBase) )
	 {
	 const char * Str,* Sub;
	 Str = StrBase + 1;
	 Sub = SubBase + 1;
	 while ( *Sub && toupper(*Sub) == toupper(*Str) )
		{
		Sub++; Str++;
		}
	 if ( ! *Sub) return((char *)StrBase);
	 }
  StrBase++;
  }

return(NULL);
}

//================================================================================
//	D3DMain_CreateDevice
//================================================================================
static BOOL D3DMain_CreateDevice(void)
{
	HRESULT			Error;
	D3DVIEWPORT7	ViewPort;

	D3DMain_Log("--- D3DMain_CreateDevice ---\n");

	// Release old device
	RELEASE(D3DInfo.lpD3DDevice);
  
	Error = D3DInfo.lpD3D->CreateDevice(D3DInfo.Drivers[D3DInfo.CurrentDriver].Guid, 
										D3DInfo.lpBackBuffer, 
										&D3DInfo.lpD3DDevice);

	if (Error != DD_OK) 
	{
		D3DMain_Log("D3DMain_CreateDevice:  lpD3D->CreateDevice failed:\n  %s\n", D3DErrorToString(Error));
		return FALSE;
	}

	{
		// Get some info from the device
		D3DDEVICEDESC7		DeviceDesc;

		D3DInfo.lpD3DDevice->GetCaps(&DeviceDesc);

		D3DInfo.MaxTextureWidth		= DeviceDesc.dwMaxTextureWidth;
		D3DInfo.MaxTextureHeight	= DeviceDesc.dwMaxTextureHeight;
		D3DInfo.MaxTextureWidth		= max(D3DInfo.MaxTextureWidth ,256);
		D3DInfo.MaxTextureHeight	= max(D3DInfo.MaxTextureHeight,256);
		
		// Fog reset
		D3DInfo.Fog.R = D3DInfo.Fog.G = D3DInfo.Fog.B = D3DInfo.Fog.Start = D3DInfo.Fog.End = 0.0f;
		D3DInfo.Fog.Enabled = FALSE;

		#if 1
			char str[1024];
			sprintf(str,"D3DDrv : Max Texture Dimensions = %d x %d\n",
				D3DInfo.MaxTextureWidth,D3DInfo.MaxTextureHeight);
			OutputDebugString(str);
			D3DMain_Log(str);
		#endif

		// <> test this by forcing MaxWidth back to 256 ; seems ok cept for uv's
		#if 0
			D3DInfo.MaxTextureWidth  = 
			D3DInfo.MaxTextureHeight = 256;
		#endif

		//D3DInfo.dwTextureOpCaps = hw.dwTextureOpCaps;
		//D3DInfo.dwTextureOpCaps = D3DTOP_MODULATE2X;
		D3DInfo.dwTextureOpCaps = 0;
	}

	// <> look in name for a 3dfx and set up a MaxSurfaces variable
	if ( stristr(D3DInfo.DDName,"3dfx") )
	{
		OutputDebugString("D3DDrv : found 3dfx card, using 1024 surfaces\n");
		D3DMain_Log("D3DDrv : found 3dfx card, using 1024 surfaces\n");
		D3DInfo.MaxSurfaceCount = 1024;
	}
	else
	{
		D3DInfo.MaxSurfaceCount = 65536 - 1024;
	}

	#if 0 // <> test it! seems ok
	D3DInfo.MaxSurfaceCount = 128;
	#endif

	// Set ViewPort data for this device
	ViewPort.dwX = 0;
	ViewPort.dwY = 0;
	ViewPort.dwWidth = D3DInfo.CurrentWidth;
	ViewPort.dwHeight = D3DInfo.CurrentHeight;
	ViewPort.dvMinZ = 0.0f;
	ViewPort.dvMaxZ = 1.0f;

	Error = D3DInfo.lpD3DDevice->SetViewport(&ViewPort);
	
	if (Error != D3D_OK)
	{
		D3DMain_Log("D3DMain_CreateDevice:  SetViewport failed.\n  %s\n", D3DErrorToString(Error));
		{
			RELEASE(D3DInfo.lpD3DDevice);
			return FALSE;
		}
	}

	return TRUE;
}

//================================================================================
//	D3DMain_CreateBuffers
//================================================================================
static BOOL D3DMain_CreateBuffers(void)
{
	DDSURFACEDESC2	ddsd;
	HRESULT			LastError;
  
	D3DMain_Log("--- D3DMain_CreateBuffers ---\n");

	// Release any old objects that might be lying around.  This should have
	// already been taken care of, but just in case...
	RELEASE(D3DInfo.lpClipper);
	RELEASE(D3DInfo.lpBackBuffer);
	RELEASE(D3DInfo.lpFrontBuffer);
  
	if (D3DInfo.FullScreen) 
	{
		DDSCAPS2		ddscaps = {DDSCAPS_BACKBUFFER, 0, 0, 0};
		
		// Create a complex flipping surface for fullscreen mode with one
		// back buffer.
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP |
		DDSCAPS_3DDEVICE | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;

		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

		LastError = CreateSurface(&ddsd, &D3DInfo.lpFrontBuffer);

		if(LastError != DD_OK) 
		{
			if (LastError == DDERR_OUTOFMEMORY || LastError == DDERR_OUTOFVIDEOMEMORY) 
			{
				D3DMain_Log("CreateBuffers:  There was not enough video memory to create the rendering surface.\n  Please restart the program and try another fullscreen mode with less resolution or lower bit depth.\n");
			} 
			else 
			{
				D3DMain_Log("CreateBuffers:  CreateSurface for fullscreen flipping surface failed.\n  %s\n",
						D3DErrorToString(LastError));
			}
			
			goto exit_with_error;
		}

		// Obtain a pointer to the back buffer surface created above so we
		// can use it later.  For now, just check to see if it ended up in
		// video memory (FYI).
		LastError = D3DInfo.lpFrontBuffer->GetAttachedSurface(&ddscaps, &D3DInfo.lpBackBuffer);
		
		if(LastError != DD_OK) 
		{
			D3DMain_Log("CreateBuffers:  GetAttachedSurface failed to get back buffer.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
		
		LastError = GetSurfDesc(&ddsd, D3DInfo.lpBackBuffer);
		
		if (LastError != DD_OK) 
		{
			D3DMain_Log("CreateBuffers:  Failed to get surface description of back buffer.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
		
		D3DInfo.BackBufferInVideo =
			(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ? TRUE : FALSE;

		D3DInfo.ddsd = ddsd;	// Save the format of the back buffer
	}
	else 
	{	
		// In the window case, create a front buffer which is the primary
		// surface and a back buffer which is an offscreen plane surface.

		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
		
		LastError = D3DInfo.lpDD->CreateSurface(&ddsd, &D3DInfo.lpFrontBuffer, NULL);

		if(LastError != DD_OK ) 
		{
			if (LastError == DDERR_OUTOFMEMORY || LastError == DDERR_OUTOFVIDEOMEMORY) 
			{
				D3DMain_Log("CreateBuffers:  There was not enough video memory to create the rendering surface.\n  To run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n");
			} 
			else 
			{
				D3DMain_Log("CreateBuffers:  CreateSurface for window front buffer failed.\n  %s\n",
					D3DErrorToString(LastError));
			}
			goto exit_with_error;
		}
    
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth = D3DInfo.CurrentWidth;
		ddsd.dwHeight = D3DInfo.CurrentHeight;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
    
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    
		LastError = CreateSurface(&ddsd, &D3DInfo.lpBackBuffer);
	
		if (LastError != DD_OK) 
		{
			if (LastError == DDERR_OUTOFMEMORY || LastError == DDERR_OUTOFVIDEOMEMORY) 
			{
				D3DMain_Log("CreateBuffers:  There was not enough video memory to create the rendering surface.\n  To run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n");
			} 
			else 
			{
				D3DMain_Log("CreateBuffers:  CreateSurface for window back buffer failed.\n  %s\n",
					D3DErrorToString(LastError));
			}
			goto exit_with_error;
		}
    
		// Check to see if the back buffer is in video memory (FYI).
		LastError = GetSurfDesc(&ddsd, D3DInfo.lpBackBuffer);

		if (LastError != DD_OK) 
		{
			D3DMain_Log("CreateBuffers:  Failed to get surface description for back buffer.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
    
		D3DInfo.BackBufferInVideo =	(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ? TRUE : FALSE;
    
		// Create the DirectDraw Clipper object and attach it to the window
		// and front buffer.
		LastError = D3DInfo.lpDD->CreateClipper(0, &D3DInfo.lpClipper, NULL);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("CreateBuffers:  CreateClipper failed.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
	
		LastError = D3DInfo.lpClipper->SetHWnd(0, D3DInfo.hWnd);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("CreateBuffers:  Attaching clipper to window failed.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
	
		
		LastError = D3DInfo.lpFrontBuffer->SetClipper(D3DInfo.lpClipper);
    
		if(LastError != DD_OK ) 
		{
			D3DMain_Log("CreateBuffers:  Attaching clipper to front buffer failed.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}

		D3DInfo.ddsd = ddsd;		// Save the format of the back buffer
	}	
  
	D3DMain_ClearBuffers();
  
	return TRUE;
  
	exit_with_error:
		
		RELEASE(D3DInfo.lpFrontBuffer);
		RELEASE(D3DInfo.lpBackBuffer);
		RELEASE(D3DInfo.lpClipper);
		return FALSE;
}

//================================================================================
//	D3DMain_DestroyBuffers
//================================================================================
static void D3DMain_DestroyBuffers(void)
{
	RELEASE(D3DInfo.lpClipper);
	RELEASE(D3DInfo.lpBackBuffer);
	RELEASE(D3DInfo.lpFrontBuffer);
}

//================================================================================
//	D3DMain_CreateZBuffer
//	Create a Z-Buffer of the appropriate depth and attach it to the back buffer.
//================================================================================
static BOOL D3DMain_CreateZBuffer(void)
{
	DDSURFACEDESC2	ddsd;
	HRESULT			LastError;
	
	assert(D3DInfo.lpBackBuffer);

	D3DMain_Log("--- D3DMain_CreateZBuffer ---\n");

	// Release any Z-Buffer that might be around just in case.
	RELEASE(D3DInfo.lpZBuffer);
  
	memset(&ddsd, 0 ,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof( ddsd );
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;//DDSD_ZBUFFERBITDEPTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
	ddsd.dwWidth = D3DInfo.CurrentWidth;
	ddsd.dwHeight = D3DInfo.CurrentHeight;
	
	ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;

    // Find a valid zbuffer, from the current device
    D3DInfo.lpD3D->EnumZBufferFormats(D3DInfo.Drivers[D3DInfo.CurrentDriver].Guid, EnumZBufferFormatsCallback,
										(VOID*)&ddsd.ddpfPixelFormat);

    
	if( sizeof(DDPIXELFORMAT) != ddsd.ddpfPixelFormat.dwSize )
    {
		D3DMain_Log("CreateZBuffer:  No zbuffer found for 3d device.\n");
		return FALSE;
    }

	LastError = D3DInfo.lpDD->CreateSurface(&ddsd, &D3DInfo.lpZBuffer, NULL);
	
	if(LastError != DD_OK) 
	{
		if (LastError == DDERR_OUTOFMEMORY || LastError == DDERR_OUTOFVIDEOMEMORY) 
		{
			if (D3DInfo.FullScreen) 
			{
				D3DMain_Log("CreateZBuffer:  There was not enough video memory to create the Z-buffer surface.\n  Please try another mode with less resolution.\n");
			} 
			else 
			{
				D3DMain_Log("CreateZBuffer:  There was not enough video memory to create the Z-buffer surface.\n  Please try another mode with less resolution.\n");
			}
		} 
		else 
		{
			D3DMain_Log("CreateZBuffer:  CreateSurface for Z-buffer failed.\n  %s\n",
				D3DErrorToString(LastError));
		}
	
		goto exit_with_error;
	}
	
	// Attach the Z-buffer to the back buffer so D3D will find it
	LastError = D3DInfo.lpBackBuffer->AddAttachedSurface(D3DInfo.lpZBuffer);
	
	if(LastError != DD_OK) 
	{
		D3DMain_Log("CreateZBuffer:  AddAttachedBuffer failed for Z-Buffer.\n  %s\n",
			D3DErrorToString(LastError));
		goto exit_with_error;
	}
	
	// Find out if it ended up in video memory.
	LastError = GetSurfDesc(&ddsd, D3DInfo.lpZBuffer);

	if (LastError != DD_OK) 
	{
		D3DMain_Log("CreateZBuffer:  Failed to get surface description of Z buffer.\n  %s\n",
			D3DErrorToString(LastError));
		goto exit_with_error;
	}
	
	D3DInfo.ZBufferInVideo = (ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ? TRUE : FALSE;
  
	return TRUE;
  
	exit_with_error:
		RELEASE(D3DInfo.lpZBuffer);
		return FALSE;
}

//================================================================================
//	D3DMain_DestroyZBuffer
//================================================================================
static void D3DMain_DestroyZBuffer(void)
{
	RELEASE(D3DInfo.lpZBuffer)
}

//================================================================================
//	D3DMain_SetDefaultRenderStates
//================================================================================
static jeBoolean D3DMain_SetDefaultRenderStates(void)
{
	HRESULT			LastError;

	// Set some defaults render states
	LastError = D3DInfo.lpD3DDevice->BeginScene();

	if (LastError != D3D_OK)
	{
		D3DMain_Log("D3DMain_SetDefaultRenderStates:  BeginScene failed.\n  %s\n",
			D3DErrorToString(LastError));
		goto exit_with_error;
	}

	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
	//D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_GREATEREQUAL);

	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
    D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);


#if 0
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, TRUE);
#endif
	
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
    D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);

	// Turn off HW clipping
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPING, FALSE);
	// Turn off HW lighting
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
	// Disable D3D updating extents during DrawPrimitve
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_EXTENTS , FALSE);

	LastError = D3DInfo.lpD3DDevice->EndScene();

	if (LastError != D3D_OK)
	{
		D3DMain_Log("D3DMain_SetDefaultRenderStates:  EndScene failed.\n  %s\n",
			D3DErrorToString(LastError));
		goto exit_with_error;
	}
	
	return JE_TRUE;

	exit_with_error:
	{
		return JE_FALSE;
	}
}

//================================================================================
//	D3DMain_RestoreDisplayMode
//	 Does nothing if mode has not been set yet
//================================================================================
static BOOL D3DMain_RestoreDisplayMode(void)
{
	HRESULT		LastError;

	if (!D3DInfo.ModeSet)
		return TRUE;

	D3DInfo.ModeSet = JE_FALSE;

	assert(D3DInfo.lpDD);

	if (D3DInfo.FullScreen) 
	{
		LastError = D3DInfo.lpDD->RestoreDisplayMode();
	
		if (LastError != DD_OK) 
		{
			D3DMain_Log("D3DMain_RestoreDisplayMode:  RestoreDisplayMode failed.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
	
		LastError = D3DInfo.lpDD->SetCooperativeLevel(D3DInfo.hWnd, DDSCL_NORMAL);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("SetCooperativeLevel to normal failed.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
	}
	
#if 0 //{
	// Restore window width/height
	SetWindowLong(D3DInfo.hWnd, GWL_STYLE, D3DInfo.OldGWL_STYLE);

	SetWindowPos(D3DInfo.hWnd, 
	            HWND_TOP, 
				D3DInfo.OldWindowRect.left,
				D3DInfo.OldWindowRect.top,
				(D3DInfo.OldWindowRect.right - D3DInfo.OldWindowRect.left),
				(D3DInfo.OldWindowRect.bottom - D3DInfo.OldWindowRect.top),
	            SWP_NOCOPYBITS | SWP_NOZORDER);

	ShowWindow(D3DInfo.hWnd, SW_SHOWNORMAL);
#endif //}

	return TRUE;
}

//========================================================================================================
//	EnumDriversCB2
//========================================================================================================
BOOL FAR PASCAL EnumDriversCB2(GUID FAR *lpGUID, LPSTR lpDriverDesc, LPSTR lpDriverName, LPVOID lpContext)
{
	D3DMain_DDEnumInfo	*Info;
	D3DMain_DDEnum		*pDriver;

	if (!lpDriverDesc || !lpDriverName)
		return (D3DENUMRET_OK);

	if (strlen(lpDriverDesc) + 5 >= MAX_DRIVER_NAME)
		return DDENUMRET_OK;

	Info = (D3DMain_DDEnumInfo*)lpContext;

	if (Info->NumDrivers >= MAX_DRIVERS)
		return DDENUMRET_CANCEL;

	pDriver = &Info->Drivers[Info->NumDrivers++];

	if (lpGUID)
	{
		pDriver->IsPrimary = JE_FALSE;
		memcpy(&pDriver->Guid, lpGUID, sizeof(GUID));
	}
	else
	{
		pDriver->IsPrimary = JE_TRUE;
	}

	// Save name
	sprintf(pDriver->Name, "(D3D)%s", lpDriverDesc);

	return DDENUMRET_OK;
}

//========================================================================================================
//	EnumSubDrivers2
//========================================================================================================
BOOL DRIVERCC EnumSubDrivers2(DRV_ENUM_DRV_CB *Cb, void *Context)
{
	HRESULT				hr;
	int32				i;
	D3DMain_DDEnumInfo	Info;

	unlink(D3DMAIN_LOG_FILENAME);
	
	Info.Drivers = Drivers;
	Info.NumDrivers = 0;

	//hr = DirectDrawEnumerate(EnumDriversCB2, &Info);
	hr = (D3DDrv_DirectDrawEnumerate())(EnumDriversCB2, &Info);

    if (hr != DD_OK) 
	{
		D3DMain_Log("D3DMain_EnumSubDrivers: DirectDrawEnumerate failed.\n");
		return FALSE;
    }

	for (i=0; i< Info.NumDrivers; i++)
	{
 		// Create the DD object for this driver
		if (!CreateDDFromDriver(&Info.Drivers[i]))
			return JE_FALSE;

		if (Main_CheckDD())
		{
			D3DInfo.IsPrimary = Info.Drivers[i].IsPrimary;

			if (!Cb(i, Info.Drivers[i].Name, Context))
			{
				RELEASE(D3DInfo.lpD3D);
				D3DInfo.lpD3D = NULL;

				//D3DMain_ShutdownD3D();
				RELEASE(D3DInfo.lpDD);
				D3DInfo.lpDD = NULL;
				memset(&D3DInfo, 0, sizeof(D3DInfo));
				break;
			}
		}
		
	#if 0
		D3DMain_ShutdownD3D();
	#else
		RELEASE(D3DInfo.lpD3D);
		D3DInfo.lpD3D = NULL;

		RELEASE(D3DInfo.lpDD);
		D3DInfo.lpDD = NULL;
	#endif
		memset(&D3DInfo, 0, sizeof(D3DInfo));
	}

#if 0		// Test
	Cb(i, "(D3D)HackDriver", Context);
#endif

	return TRUE;
}	

//========================================================================================================
//	EnumModes2
//========================================================================================================
BOOL DRIVERCC EnumModes2(int32 Driver, char *DriverName, DRV_ENUM_MODES_CB *Cb, void *Context)
{
	HRESULT				hr;
	int32				i, Width, Height, Bpp;
	char				ModeName[MAX_DRIVER_NAME];
	D3DMain_DDEnumInfo	Info;
	
#if 0		// Testing
	Cb(0, "HackMode 2", 640, 480, Context);	
	return JE_TRUE;
#endif

	Info.Drivers = Drivers;
	Info.NumDrivers = 0;

	//hr = DirectDrawEnumerate(EnumDriversCB2, &Info);
	hr = (D3DDrv_DirectDrawEnumerate())(EnumDriversCB2, &Info);

    if (hr != DD_OK) 
	{
		D3DMain_Log("D3DMain_EnumModes:  DirectDrawEnumerate failed.\n");
		return FALSE;
    }


	if (!CreateDDFromName(DriverName, &Info))
		return JE_FALSE;

	if (!D3DMain_EnumDisplayModes())
	{
		D3DMain_ShutdownD3D();
		
		D3DMain_Log("D3DMain_EnumModes: D3DMain_EnumDisplayModes failed.\n");
		return FALSE;
	}
		
	for (i=0; i< D3DInfo.NumModes; i++)
	{
		// Get the width/height of mode
		Width = D3DInfo.Modes[i].Width;
		Height = D3DInfo.Modes[i].Height;
		Bpp = D3DInfo.Modes[i].Bpp;

		// Make a unique name
		sprintf(ModeName, "%ix%ix%i", Width, Height, Bpp);

		#if 0
		if (Bpp != 16)
			continue;
		#endif

		// Call their callback with this driver mode
		if (!Cb(i, ModeName, Width, Height, Bpp, Context))
		{
			D3DMain_ShutdownD3D();
			return JE_TRUE;
		}
	}

	if (D3DInfo.CanDoWindow)
	{
		if (!Cb(i, "WindowMode", -1, -1, -1, Context))
		{
			D3DMain_ShutdownD3D();
			return JE_TRUE;
		}
	}
  
	D3DMain_ShutdownD3D();

	return TRUE;
}

//================================================================================
//	D3DMain_CreateDDFromName
//	Creates DD, searching for the specified DD name using DriverName
//================================================================================
static BOOL D3DMain_CreateDDFromName(const char *DriverName)
{
	HRESULT				hr;
	DDCAPS				DriverCaps, HELCaps;
	D3DMain_DDEnumInfo	Info;
	
	D3DMain_Log("--- D3DMain_CreateDDFromName ---\n");
	
	if (strlen(DriverName) >= MAX_DRIVER_NAME)
		return JE_FALSE;

	D3DMain_Log("  Name: %s\n", DriverName);

	Info.Drivers = Drivers;
	Info.NumDrivers = 0;

	//hr = DirectDrawEnumerate(EnumDriversCB2, &Info);
	hr = (D3DDrv_DirectDrawEnumerate())(EnumDriversCB2, &Info);

    if (hr != DD_OK) 
	{
		D3DMain_Log("D3DMain_CreateDDFromName:  DirectDrawEnumerate failed.\n");
		return FALSE;
    }
		
	if (!CreateDDFromName(DriverName, &Info))
		return JE_FALSE;

	assert(D3DInfo.lpDD);

	memset(&DriverCaps, 0, sizeof(DDCAPS));
	DriverCaps.dwSize = sizeof(DDCAPS);
	memset(&HELCaps, 0, sizeof(DDCAPS));
	HELCaps.dwSize = sizeof(DDCAPS);

	if (FAILED(D3DInfo.lpDD->GetCaps(&DriverCaps, &HELCaps))) 
	{
		D3DMain_Log("D3DMain_CreateDDFromName:  GetCaps failed.\n");
		D3DMain_ShutdownD3D();
		return FALSE;
	}

	if (DriverCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
		D3DMain_Log("   DDCAPS2_CANRENDERWINDOWED    : YES\n");
	else
		D3DMain_Log("   DDCAPS2_CANRENDERWINDOWED    : NO\n");

	if (DriverCaps.dwCaps2 & DDCAPS2_NO2DDURING3DSCENE)
		D3DMain_Log("   DDCAPS2_NO2DDURING3DSCENE    : YES\n");
	else
		D3DMain_Log("   DDCAPS2_NO2DDURING3DSCENE    : NO\n");

	// Save the DD object
	strcpy(D3DInfo.DDName, DriverName);
	
	return TRUE;
}

//========================================================================================================
//	CreateDDFromDriver
//========================================================================================================
static jeBoolean CreateDDFromDriver(D3DMain_DDEnum *pDriver)
{
	HRESULT			hr;

	if (pDriver->IsPrimary)
	{
		//hr = DirectDrawCreateEx(NULL, &D3DInfo.lpDD, IID_IDirectDraw7, NULL );
		hr = (D3DDrv_DirectDrawCreate())( NULL, &D3DInfo.lpDD, IID_IDirectDraw7, NULL );
	}
	else
	{
		//hr = DirectDrawCreateEx(pDriver->Guid, &D3DInfo.lpDD, IID_IDirectDraw7, NULL );
		hr = (D3DDrv_DirectDrawCreate())( &pDriver->Guid, &D3DInfo.lpDD, IID_IDirectDraw7, NULL );
	}

	if( FAILED( hr ) )
		return JE_FALSE;

	D3DInfo.IsPrimary = pDriver->IsPrimary;

	return JE_TRUE;
}

//========================================================================================================
//	CreateDDFromName
//========================================================================================================
static jeBoolean CreateDDFromName(const char *DriverName, const D3DMain_DDEnumInfo *Info)
{
	int32			i;

	for (i=0; i < Info->NumDrivers; i++)
	{
		if (!strcmp(Info->Drivers[i].Name, DriverName))
			break;
	}

	if (i == Info->NumDrivers)
		return JE_FALSE;

 	// Create the DD object for this driver
	if (!CreateDDFromDriver(&Info->Drivers[i]))
		return JE_FALSE;

	return JE_TRUE;
}

//=====================================================================================
//	Log2
//	Return the log of a size
//=====================================================================================
uint32 Log2(uint32 P2)
{
	uint32		p = 0;
	int32		i = 0;
	
	for (i = P2; i > 0; i>>=1)
		p++;

	return (p-1);
}

//=====================================================================================
//	SnapToPower2
//	Snaps a number to a power of 2
//=====================================================================================
int32 SnapToPower2(int32 Width)
{
		 if (Width <= 1) return 1;
	else if (Width <= 2) return 2;
	else if (Width <= 4) return 4;
	else if (Width <= 8) return 8;
	else if (Width <= 16) return 16;
	else if (Width <= 32) return 32;
	else if (Width <= 64) return 64;
	else if (Width <= 128) return 128;
	else if (Width <= 256) return 256;
	else if (Width <= 512) return 512;
	else if (Width <= 1024) return 1024;
	else if (Width <= 2048) return 2048;
	else 
		return -1;
}

//=====================================================================================
//	Return the max log of a (power of 2) width and height
//=====================================================================================
int32 GetLog(int32 Width, int32 Height)
{
	int32	LWidth = SnapToPower2(max(Width, Height));
	
	return Log2(LWidth);
}

//////////////////////
// Lamex
//////////////////////
void Draw_Text(char *text, int x,int y,
               COLORREF color,
               LPDIRECTDRAWSURFACE7 lpdds,
               HFONT fnt)
{
	//this function draws text in 16-bit DX mode
	// with the selected font

	HDC dc; // the dc
	// get the dc from surface
	lpdds->GetDC(&dc);

	// set the colors 
	SetTextColor(dc,color);

	// set background mode to transparent
	// so black isn't copied
	SetBkMode(dc, TRANSPARENT);

	// draw the text using the font
	SelectObject(dc, fnt);
	TextOut(dc,x,y,text,strlen(text));

	// release the dc
	lpdds->ReleaseDC(dc);

} // end Draw_Text

// BEGIN - Render state access - paradoxnj 12/25/2005
jeBoolean DRIVERCC D3DMain_SetRenderState(uint32 state, uint32 value)
{
	HRESULT								hres;
	DWORD								val;

	switch (state)
	{
	case JE_RENDERSTATE_ENABLE_ZBUFFER:
		{
			if ((jeBoolean)value == JE_TRUE)
				val = D3DZB_TRUE;
			else
				val = D3DZB_FALSE;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, val);
			break;
		}
	case JE_RENDERSTATE_ENABLE_ZWRITES:
		{
			if ((jeBoolean)value == JE_TRUE)
				val = TRUE;
			else
				val = FALSE;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, val);
			break;
		}
	case JE_RENDERSTATE_ENABLE_ALPHABLENDING:
		{
			if ((jeBoolean)value == JE_TRUE)
				val = TRUE;
			else
				val = FALSE;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, val);
			break;
		}
	case JE_RENDERSTATE_ENABLE_ALPHATESTING:
		{
			if ((jeBoolean)value == JE_TRUE)
				val = TRUE;
			else
				val = FALSE;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, val);
			break;
		}
	case JE_RENDERSTATE_ALPHAREF:
		{
			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, value);
			break;
		}
	case JE_RENDERSTATE_ALPHAFUNC:
	case JE_RENDERSTATE_DEPTHFUNC:
	case JE_RENDERSTATE_STENCILFUNC:
		{
			D3DCMPFUNC				func;
			D3DRENDERSTATETYPE		st;

			if (value == JE_CMP_NEVER)
				func = D3DCMP_NEVER;
			else if (value == JE_CMP_LESS)
				func = D3DCMP_LESS;
			else if (value == JE_CMP_EQUAL)
				func = D3DCMP_EQUAL;
			else if (value == JE_CMP_LEQUAL)
				func = D3DCMP_LESSEQUAL;
			else if (value == JE_CMP_GREATER)
				func = D3DCMP_GREATER;
			else if (value == JE_CMP_GEQUAL)
				func = D3DCMP_GREATEREQUAL;
			else if (value == JE_CMP_NEQUAL)
				func = D3DCMP_NOTEQUAL;
			else if (value == JE_CMP_ALWAYS)
				func = D3DCMP_ALWAYS;

			if (state == JE_RENDERSTATE_ALPHAFUNC)
				st = D3DRENDERSTATE_ALPHAFUNC;
			else if (state == JE_RENDERSTATE_DEPTHFUNC)
				st = D3DRENDERSTATE_ZFUNC;
			else if (state == JE_RENDERSTATE_STENCILFUNC)
				st = D3DRENDERSTATE_STENCILFUNC;

			hres = D3DInfo.lpD3DDevice->SetRenderState(st, func);
			break;
		}
	case JE_RENDERSTATE_FILLMODE:
		{
			D3DFILLMODE					fm;

			if (value == JE_FILL_POINT)
				fm = D3DFILL_POINT;
			else if (value == JE_FILL_WIREFRAME)
				fm = D3DFILL_WIREFRAME;
			else if (value == JE_FILL_SOLID)
				fm = D3DFILL_SOLID;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, fm);
			break;
		}
	case JE_RENDERSTATE_SHADEMODE:
		{
			D3DSHADEMODE				sm;

			if (value == JE_SHADE_FLAT)
				sm = D3DSHADE_FLAT;
			else if (value == JE_SHADE_GOURAUD)
				sm = D3DSHADE_GOURAUD;
			else if (value == JE_SHADE_PHONG)
				sm = D3DSHADE_PHONG;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, sm);
			break;
		}
	case JE_RENDERSTATE_CULLMODE:
		{
			D3DCULL						cm;

			if (value == JE_CULL_NONE)
				cm = D3DCULL_NONE;
			else if (value == JE_CULL_CW)
				cm = D3DCULL_CW;
			else if (value == JE_CULL_CCW)
				cm = D3DCULL_CCW;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, cm);
			break;
		}
	case JE_RENDERSTATE_ENABLE_FOG:
		{
			if ((jeBoolean)value == JE_TRUE)
				val = TRUE;
			else
				val = FALSE;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, val);
			break;
		}
	case JE_RENDERSTATE_FOGCOLOR:
		{
			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, value);
			break;
		}
	case JE_RENDERSTATE_FOGSTART:
		{
			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGSTART, value);
			break;
		}
	case JE_RENDERSTATE_FOGEND:
		{
			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGEND, value);
			break;
		}
	case JE_RENDERSTATE_HWLIGHTINGENABLE:
		{
			if ((jeBoolean)value == JE_TRUE)
				val = TRUE;
			else
				val = FALSE;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, val);
			break;
		}
	case JE_RENDERSTATE_AMBIENTLIGHT:
		{
			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, value);
			break;
		}
	case JE_RENDERSTATE_ENABLE_STENCIL:
		{
			if ((jeBoolean)value == JE_TRUE)
				val = TRUE;
			else
				val = FALSE;

			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE, val);
			break;
		}
	case JE_RENDERSTATE_STENCILREF:
		{
			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_STENCILREF, value);
			break;
		}
	case JE_RENDERSTATE_STENCILMASK:
		{
			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_STENCILMASK, value);
			break;
		}
	case JE_RENDERSTATE_STENCILWRITEMASK:
		{
			hres = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_STENCILWRITEMASK, value);
			break;
		}
	case JE_RENDERSTATE_STENCILFAIL:
	case JE_RENDERSTATE_STENCILZFAIL:
		{
			D3DSTENCILOP			so;
			D3DRENDERSTATETYPE		rs;

			if (state == JE_RENDERSTATE_STENCILFAIL)
				rs = D3DRENDERSTATE_STENCILFAIL;
			else if (state == JE_RENDERSTATE_STENCILZFAIL)
				rs = D3DRENDERSTATE_STENCILZFAIL;
			else if (state == JE_RENDERSTATE_STENCILPASS)
				rs = D3DRENDERSTATE_STENCILPASS;

			if (value == JE_STENCILOP_KEEP)
				so = D3DSTENCILOP_KEEP;
			else if (value == JE_STENCILOP_ZERO)
				so = D3DSTENCILOP_ZERO;
			else if (value == JE_STENCILOP_REPLACE)
				so = D3DSTENCILOP_REPLACE;
			else if (value == JE_STENCILOP_INCRWRAP)
				so = D3DSTENCILOP_INCRSAT;
			else if (value == JE_STENCILOP_DECRWRAP)
				so = D3DSTENCILOP_DECRSAT;
			else if (value == JE_STENCILOP_INVERT)
				so = D3DSTENCILOP_INVERT;
			else if (value == JE_STENCILOP_INCR)
				so = D3DSTENCILOP_INCR;
			else if (value == JE_STENCILOP_DECR)
				so = D3DSTENCILOP_DECR;

			hres = D3DInfo.lpD3DDevice->SetRenderState(rs, so);
			break;
		}
	}

	if (FAILED(hres))
		return JE_FALSE;

	return JE_TRUE;
}
