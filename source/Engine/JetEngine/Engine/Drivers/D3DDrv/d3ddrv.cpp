/****************************************************************************************/
/*  D3DDrv.cpp                                                                          */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: D3D driver                                                             */
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
#include <ddraw.h>
#include <stdio.h>

#include "D3DDrv.h"
#include "DCommon.h"

#include "d3d_Scene.h"
#include "d3d_Render.h"
#include "D3DCache.h"
#include "D3D_Main.h"
#include "d3d_PCache.h"
#include "d3d_THandle.h"
#include "d3d_GSpan.h"
#include "D3D_fx.h"

DRV_Window						D3DDrv_ClientWindow;

static char						LastErrorStr[200];

static BOOL						ExitHandlerActive = FALSE;
static	DRV_EngineSettings		EngineSettings;
static	jeRDriver_PixelFormat	PixelFormat[10]; 

#define NUM_PIXEL_FORMATS (sizeof(PixelFormats)/sizeof(jeRDriver_PixelFormat))

static	HINSTANCE				DirectDrawInstance;
static	LPDIRECTDRAWCREATE		DirectDrawCreateFN;
static	LPDIRECTDRAWENUMERATE	DirectDrawEnumerateFN;


jeBoolean	DRIVERCC DrvShutdown(void);
jeBoolean	DRIVERCC ScreenShot(const char *Name);
jeBoolean	DRIVERCC DrawDDText(char *text, int x,int y, uint32 color);
jeBoolean	DRIVERCC SetFog(float r, float g, float b, float start, float endi, jeBoolean Enable);

//============================================================================================
//============================================================================================

static	void	D3DDrv_InitStaticsAndGlobals(void)
{
	memset(&D3DDrv_ClientWindow, 0, sizeof(D3DDrv_ClientWindow));
	//LastError = 0;
	memset(LastErrorStr, 0, sizeof(LastErrorStr));
	if	(DirectDrawInstance != nullptr)
		FreeLibrary(DirectDrawInstance);
	DirectDrawInstance = nullptr;
	DirectDrawCreateFN = nullptr;
	DirectDrawEnumerateFN = nullptr;
	memset(&EngineSettings, 0, sizeof(EngineSettings));
	memset(PixelFormat, 0, sizeof(PixelFormat));
}

//============================================================================================
//============================================================================================

/*
	This next part is a little ugly thing to let us get the name of certain
	Windows functions right for calls to GetProcAddress.  This macro lets us
	get the right one of DirectDrawEnumerateA vs. DirectDrawEnumerateW in case
	someone wants to build the wide char version of this beast.
*/
#define MacroStr1(n) #n

#define MacroStr(n) MacroStr1(n)

static	HRESULT __stdcall DirectDrawCreateFake( GUID FAR *lpGUID, LPDIRECTDRAW7 FAR *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter )
{
	return !DD_OK;
}

static	HRESULT __stdcall DirectDrawEnumerateFake( LPDDENUMCALLBACK lpCallback, LPVOID lpContext )
{
	return !DD_OK;
}

static	BOOL	LoadDirectDraw(void)
{
	if	(DirectDrawInstance)
		return TRUE;

	DirectDrawInstance = LoadLibrary("ddraw.dll");

	DirectDrawCreateFN	=(LPDIRECTDRAWCREATE)GetProcAddress(DirectDrawInstance, "DirectDrawCreateEx");
	DirectDrawEnumerateFN	=(LPDIRECTDRAWENUMERATE)GetProcAddress(DirectDrawInstance, MacroStr(DirectDrawEnumerate));
	if	(!DirectDrawCreateFN || !DirectDrawEnumerateFN)
	{
		FreeLibrary(DirectDrawInstance);
		DirectDrawInstance = nullptr;
		DirectDrawCreateFN = DirectDrawCreateFake;
		DirectDrawEnumerateFN = DirectDrawEnumerateFake;
		return FALSE;
	}

	return TRUE;
}

LPDIRECTDRAWCREATE D3DDrv_DirectDrawCreate(void)
{
	LoadDirectDraw();
	
	return DirectDrawCreateFN;
}

LPDIRECTDRAWENUMERATE D3DDrv_DirectDrawEnumerate(void)
{
	LoadDirectDraw();

	return DirectDrawEnumerateFN;
}

BOOL DRIVERCC DrvInit(DRV_DriverHook *Hook)
{
	RECT	WRect{};

	D3DDrv_InitStaticsAndGlobals();
	D3DFx_InitStaticsAndGlobals();
	PCache_InitStaticsAndGlobals();

	#ifdef USE_SPANS
	GSpan_InitStaticsAndGlobals();
	#endif

	// Start up
	if (!D3DMain_InitD3D(Hook->hWnd, Hook->DriverName, Hook->Width, Hook->Height))
	{
		//SetLastDrvError(DRV_ERROR_INIT_ERROR, "D3D_DrvInit: Could not init driver.\n");
		return FALSE;
	}
	
	// If they are asking for a window mode, use there hWnd for the size
	if (Hook->Width ==-1 && Hook->Height == -1)
	{
		GetClientRect(Hook->hWnd, &WRect);
		
		Hook->Width = (WRect.right - WRect.left);
		Hook->Height = (WRect.bottom - WRect.top);
	}

	D3DDrv_ClientWindow.Width = Hook->Width;
	D3DDrv_ClientWindow.Height = Hook->Height;
	D3DDrv_ClientWindow.hWnd = Hook->hWnd;


	return TRUE;
}

//============================================================================================
//============================================================================================
BOOL DRIVERCC DrvShutdown(void)
{
	D3DMain_ShutdownD3D();

	/*
		Make sure that we shut down the DLL, and reset things to the faked out
		functions.  If it needs to be restarted, it will happend automatically.
	*/
	if	(DirectDrawInstance)
	{
		FreeLibrary(DirectDrawInstance);
		DirectDrawInstance = nullptr;
		DirectDrawCreateFN = DirectDrawCreateFake;
		DirectDrawEnumerateFN = DirectDrawEnumerateFake;
	}

	return TRUE;
}

//============================================================================================
//	DrvResetAll
//============================================================================================
jeBoolean DRIVERCC DrvResetAll(void)
{
	return D3DMain_Reset();
}

//============================================================================================
//	EnumPixelFormats
//	NOTE: that this function only works after the video mode has been set...
//============================================================================================
jeBoolean DRIVERCC EnumPixelFormats(DRV_ENUM_PFORMAT_CB *Cb, void *Context)
{
	int32			i{};
	jePixelFormat	Format3d{}, Format2d{};
	uint32			CurrentBpp{};

	CurrentBpp = D3DInfo.ddsd.ddpfPixelFormat.dwRGBBitCount;

	// Setup the 2d surface format
	if (CurrentBpp == 32 && D3DInfo.ddSurfFormat.ddpfPixelFormat.dwRGBAlphaBitMask == 0xff000000)
		Format2d = JE_PIXELFORMAT_32BIT_ARGB;
	else if (CurrentBpp == 32 && D3DInfo.ddSurfFormat.ddpfPixelFormat.dwBBitMask == 0xff)
		Format2d = JE_PIXELFORMAT_32BIT_XRGB;
	else if (CurrentBpp == 24 && D3DInfo.ddSurfFormat.ddpfPixelFormat.dwBBitMask == 0xff)
		Format2d = JE_PIXELFORMAT_24BIT_RGB;
	else if (D3DInfo.ddSurfFormat.ddpfPixelFormat.dwGBitMask == (31<<5))
		Format2d = JE_PIXELFORMAT_16BIT_555_RGB;
	else
		Format2d = JE_PIXELFORMAT_16BIT_565_RGB;

	// Setup the 3d (Texture) format
	if (D3DInfo.ddTexFormat.ddpfPixelFormat.dwGBitMask == (31<<5))
		Format3d = JE_PIXELFORMAT_16BIT_555_RGB;
	else
		Format3d = JE_PIXELFORMAT_16BIT_565_RGB;


	// Create the surface formats now
	PixelFormat[0].PixelFormat = Format3d;							// 3d 565/555 surface
	PixelFormat[0].Flags = RDRIVER_PF_3D | RDRIVER_PF_COMBINE_LIGHTMAP;
		
	PixelFormat[1].PixelFormat = JE_PIXELFORMAT_16BIT_4444_ARGB;	// 3d 4444 surface
	PixelFormat[1].Flags = RDRIVER_PF_3D | RDRIVER_PF_COMBINE_LIGHTMAP | RDRIVER_PF_ALPHA;

	PixelFormat[2].PixelFormat = Format2d;							// 2d 565/555 surface
	PixelFormat[2].Flags = RDRIVER_PF_2D | RDRIVER_PF_CAN_DO_COLORKEY;

	PixelFormat[3].PixelFormat = Format3d;							// Lightmap 565/555 surface
	PixelFormat[3].Flags = RDRIVER_PF_LIGHTMAP;

	PixelFormat[4].PixelFormat = JE_PIXELFORMAT_16BIT_1555_ARGB;	
	PixelFormat[4].Flags = RDRIVER_PF_3D | RDRIVER_PF_COMBINE_LIGHTMAP | RDRIVER_PF_ALPHA;

	// Then hand them off to the caller
	for (i=0; i<5; i++)
	{
		if (!Cb(&PixelFormat[i], Context))
			return JE_TRUE;
	}

	return TRUE;
}

//============================================================================================
//	SetGamma
//============================================================================================
jeBoolean DRIVERCC SetGamma(float Gamma)
{
	return JE_TRUE;
}

//============================================================================================
//	GetGamma
//============================================================================================
jeBoolean DRIVERCC GetGamma(float *Gamma)
{
	*Gamma = 1.0f;
		
	return JE_TRUE;
}

static jeBoolean DRIVERCC DrvGetDeviceCaps(jeDeviceCaps *DeviceCaps)
{
	DeviceCaps->SuggestedDefaultRenderFlags = JE_RENDER_FLAG_BILINEAR_FILTER;
	DeviceCaps->CanChangeRenderFlags = 0xFFFFFFFF;

	return JE_TRUE;
}

BOOL DRIVERCC EnumSubDrivers2(DRV_ENUM_DRV_CB *Cb, void *Context);
BOOL DRIVERCC EnumModes2(int32 Driver, char *DriverName, DRV_ENUM_MODES_CB *Cb, void *Context);



DRV_Driver D3DDRV =
{

	//	"D3D driver. v"DRV_VMAJS"."DRV_VMINS". Copyright 1999, Eclipse Entertainment; All Rights Reserved." , // by trilobite
	//	"D3D driver: v "DRV_VMAJS" . "DRV_VMINS". Copyright 1999, Eclipse Entertainment; All Rights Reserved.", // by trilobite
	//	strcat("D3d driver : v %s.%s.Copyright",DRV_VMAJS,DRV_VMINS),
	//	strcat("D3D Driver: v%s, Copyright 1999, Eclipse Entertainment; All Rights Reserved.", DRV_VMAJS), // by trilobite
		"D3D driver.v" DRV_VMAJS_PLUS_DRV_VMINS". Copyright 1999, Eclipse Entertainment; All Rights Reserved." ,	// by trilobite
		
	
	DRV_VERSION_MAJOR,
	DRV_VERSION_MINOR,

	DRV_ERROR_NONE,
	NULL,
	
	EnumSubDrivers2,
	EnumModes2,

	EnumPixelFormats,

	DrvGetDeviceCaps,

	DrvInit,
	DrvShutdown,
	DrvResetAll,
	D3DMain_UpdateWindow,
	D3DMain_SetActive,

	THandle_Create,
	NULL,
	THandle_Destroy,

	THandle_Lock,
	THandle_UnLock,

	NULL,			// SetPal
	NULL,			// GetPal

	NULL,			// SetAlpha
	NULL,			// GetAlpha

	THandle_GetInfo,

	D3DBeginScene,
	D3DEndScene,

	PCache_BeginBatch,
	PCache_EndBatch,

	RenderGouraudPoly,
	RenderWorldPoly,
	RenderMiscTexturePoly,

	DrawDecal,

	0,0,0,
	
	&PCache_CacheInfo,

	ScreenShot,

	SetGamma,
	GetGamma,

	// BEGIN - Hardware T&L - paradoxnj 4/5/2005
	NULL,
	NULL,
	NULL,
	// END - Hardware T&L - paradoxnj 4/5/2005

	NULL,
	NULL,								// Init to NULL, engine SHOULD set this (SetupLightmap)

	// KROUER
	DrawDDText,	// optimized draw text functions

	SetFog,

	NULL,
	NULL,
	NULL,
	
	NULL,
	NULL,
	NULL,

	D3DMain_SetRenderState
};

static BOOL DriverHook(DRV_Driver **Driver)
{
	// If there is no DirectDraw, bail out right away.
	//	if	(LoadDirectDraw() == FALSE)
	//		return FALSE;

	EngineSettings.CanSupportFlags = (DRV_SUPPORT_ALPHA | DRV_SUPPORT_COLORKEY);
	EngineSettings.PreferenceFlags = 0;

	D3DDRV.EngineSettings = &EngineSettings;
    
	*Driver = &D3DDRV;

	// Make sure the error string ptr is not null, or invalid!!!
    D3DDRV.LastErrorStr = LastErrorStr;

	SetLastDrvError(DRV_ERROR_NONE, "D3DDrv:  No error.");

	return TRUE;
}

extern "C" JETAPI	void * JETCC jeEngine_D3DDriver(void)
{
	return (void *)DriverHook;
}

void SetLastDrvError(int32 Error, char *ErrorStr)
{
	//LastError = Error;
	
	if (ErrorStr)
		strcpy_s(LastErrorStr, ErrorStr);
	else
		LastErrorStr[0] = NULL;

    D3DDRV.LastErrorStr = LastErrorStr;
    D3DDRV.LastError = Error;
}


//============================================================================================
//	D3D Screenshot BMp code
//============================================================================================
#include "Ram.h"

#ifdef BITS16
	#define	RED(x)		((unsigned short)((x>>11) & 0x1f))
	#define	GREEN(x)	((unsigned short)((x>>6 ) & 0x1f))
	#define	BLUE(x)		((unsigned short)((x>>0 ) & 0x1f))
#else
	#define	RED(x)		((unsigned short)((x>>11) & 0x1f))
	#define	GREEN(x)	((unsigned short)((x>>5 ) & 63))
	#define	BLUE(x)		((unsigned short)((x>>0 ) & 0x1f))
#endif

static jeBoolean WriteBMP(unsigned short *ScreenBuffer, int32 Width, int32 Height, int32 Stride, const char *Name)
{
	BITMAPFILEHEADER bfh = 
	{
		((unsigned short)'B' | ((unsigned short)'M' << 8)),
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + Width * Height * 2,
		0,
		0,
		sizeof(BITMAPINFOHEADER)
	};

	BITMAPINFO		bi =
	{
		{
		sizeof(BITMAPINFOHEADER),
		Width,
		Height,
		1,
		24,
		BI_RGB,
		0,
		0,
		0,
		0,
		0
		}
	};
	FILE* out{};
	int				y{};
	unsigned char* Buff{};

	Buff = JE_RAM_ALLOCATE_ARRAY(uint8, Width*3);

	if (!Buff)
		return JE_FALSE;

	out = fopen(Name, "wb");

	if (!out)
		return JE_FALSE;

	if	(fwrite(&bfh, sizeof(bfh), 1, out) != 1)
		return JE_FALSE;

	if	(fwrite(&bi, sizeof(bi), 1, out) != 1)
		return JE_FALSE;

	for	(y = Height-1; y >= 0; y--)
	{
		int				i{};
		unsigned short* p{};
		unsigned char* BuffPtr{};

		BuffPtr = &Buff[0];

		p = &ScreenBuffer[y*Stride];

		for	(i = 0; i < Width; i++)
		{
			#ifdef BIT16
				unsigned short	c;
			
				c = p[i];
				c = (RED(c) << 10) + (GREEN(c) << 5) + BLUE(c);
				p[i] = c;
			#else
				
				#ifdef VER1
				char c[3];
				c = (char)(GREEN(p[i]) << 2);
				fwrite(&c, 1, 1, out);
				c = (char)(RED(p[i]) << 3);
				fwrite(&c, 1, 1, out);
				c = (char)(BLUE(p[i]) << 3);
				fwrite(&c, 1, 1, out);
				#else
				#if 0
				char c[3];
				c = (char)(BLUE(p[i]) << 3);
				fwrite(&c, 1, 1, out);
				c = (char)(GREEN(p[i]) << 2);
				fwrite(&c, 1, 1, out);
				c = (char)(RED(p[i]) << 3);
				fwrite(&c, 1, 1, out);
				#else
				*BuffPtr++ = (char)(BLUE(p[i]) << 3);
				*BuffPtr++ = (char)(GREEN(p[i]) << 2);
				*BuffPtr++ = (char)(RED(p[i]) << 3);
//				fwrite(&c[0], 3, 1, out);
 				#endif
				#endif
			#endif
		}

		fwrite(&Buff[0], Width * 3, 1, out);
		#ifdef BIT16
			p = &ScreenBuffer[y*Stride];
			for	(i = 0; i < Width; i++)
				fwrite(&p[(i+2)%Width], 2, 1, out);
		#endif
	}

	jeRam_Free(Buff);

	if (fclose(out))
		return JE_FALSE;

	return JE_TRUE;
}


BOOL DRIVERCC ScreenShot(const char *Name)
{
#if 1
	int32				Width{}, Height{}, Stride{};
	DDSURFACEDESC2		SurfDesc{};
	uint16* Surface{};
	HRESULT				Result{};

    memset(&SurfDesc, 0, sizeof(DDSURFACEDESC2));
    SurfDesc.dwSize = sizeof(DDSURFACEDESC2);

	Result = D3DInfo.lpBackBuffer->Lock(NULL, &SurfDesc, DDLOCK_WAIT, NULL);

	if (Result != DD_OK)
		return JE_FALSE;

	Surface = (uint16*)SurfDesc.lpSurface;
	Width = SurfDesc.dwWidth;
	Height = SurfDesc.dwHeight;
	Stride = SurfDesc.lPitch>>1;

	if (!WriteBMP(Surface, Width, Height, Stride, Name))
		return JE_FALSE;

    Result = D3DInfo.lpFrontBuffer->Unlock(NULL);

	if (Result != DD_OK)
		return JE_FALSE;
#endif

	return JE_TRUE;
}

BOOL DRIVERCC SetFog(float r, float g, float b, float start, float endi, jeBoolean enabled)
{
	
	D3DInfo.Fog.R = r;
	D3DInfo.Fog.G = g;
	D3DInfo.Fog.B = b;
	D3DInfo.Fog.Start = start;
	D3DInfo.Fog.End = endi;
	D3DInfo.Fog.Enabled = enabled;

	if(enabled)
	{

		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,true);
		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,((DWORD)r<<16)|((DWORD)g<<8)|(DWORD)b);//0x00FFFFFF);//((DWORD)AppInfo.FogR<<16)|((DWORD)AppInfo.FogG<<8)|(DWORD)AppInfo.FogB
		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR);
		
  		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGTABLESTART,*(DWORD*)(&start));
  		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEEND, *(DWORD*)(&endi));
		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEDENSITY, (DWORD)0.004f);
	}
	else
	{
		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,false);
	}

	return JE_TRUE;
}


jeBoolean DRIVERCC DrawDDText(char *text, int x,int y, uint32 color)
{
	HFONT fnt{};
	fnt = CreateFont(14,0,0,0,0,0,0,0,0,0,0,0,0, "Courier New");
	
	//Draw_Text(text, x,y, RGB(r,g,b),D3DInfo.lpBackBuffer ,fnt);
	//Draw_Text(text, x,y, RGB(r,g,b),D3DInfo.lpZBuffer ,fnt);
	Draw_Text(text, x,y, (COLORREF)color,D3DInfo.lpBackBuffer ,fnt);
	DeleteObject(fnt);

	return JE_TRUE;
}
