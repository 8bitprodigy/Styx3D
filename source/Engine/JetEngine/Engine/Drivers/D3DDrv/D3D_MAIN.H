/****************************************************************************************/
/*  D3D_Main.h                                                                          */
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
#ifndef D3D_MAIN_H
#define D3D_MAIN_H

#include <Windows.h>
#include <DDraw.h>
#include <D3D.h>
#include <d3dx.h>


#define INITGUID

#include "DCommon.h"

#define MAX_APP_MODES					50
#define DDMAIN_MAX_D3D_DRIVERS			10
#define DDMAIN_MAX_TEXTURE_FORMATS		128
#define DDMAIN_MAX_SURFACE_FORMATS		128

#define D3DMAIN_LOG_FILENAME			"D3DDrv.Log"

#define MAX_DRIVER_NAME					1024

//================================================================================
//	Structure defs
//================================================================================

typedef struct
{
    char				Name[MAX_DRIVER_NAME];	// Short name of the driver 
    char				About[MAX_DRIVER_NAME];	// Short string about the driver 
    D3DDEVICEDESC7		Desc;					// D3DDEVICEDESC7 for complete information 
    GUID				Guid;					// it's GUID 
    BOOL				IsHardware;				// does this driver represent a hardware device? 
    BOOL				DoesTextures;			// does this driver do texture mapping? 
    BOOL				DoesZBuffer;			// can this driver use a z-buffer? 
    BOOL				CanDoWindow;			// can it render to Window's display depth? 
	BOOL				DoesTransparency;
	BOOL				DoesAlpha;
	BOOL				DoesClamping;
	BOOL				DoesSrcBlending;
	BOOL				DoesDestBlending;

    WORD				MaxTextureBlendStages;
    WORD				MaxSimultaneousTextures;	
	
	BOOL				CanUse;					// We can use this driver
} DDMain_D3DDriver;

typedef struct 
{
    int32				Width;							// width
    int32				Height;							// height
    int32				Bpp;							// bits per pixel 
    BOOL				ThisDriverCanDo;				// == TRUE if d3d driver can render into
} App_Mode;

typedef struct
{
    DDSURFACEDESC2		ddsd;							// DDSURFACEDESC for complete information 
    BOOL				HasOneBitAlpha;	
    BOOL				HasFourBitAlpha;
} DDMain_SurfFormat;

typedef struct
{
	uint32				R[256];
	uint32				G[256];
	uint32				B[256];
	uint32				A[256];
} RGB_LUT;

typedef struct
{
	float				R;
	float				G;
	float				B;
	float				Start;
	float				End;
	BOOL				Enabled;
} FOGI;

// D3DMain_GlobalInfo, used for everything global. 
typedef struct
{
	// Window info
	HWND				hWnd;								// Handle to parent Window

	DDSURFACEDESC2		ddsd;

	// Mode that we were in before initializing
	int32				OldWidth;							// Old screen width
	int32				OldHeight;
	int32				OldBpp;

	int32				CurrentWidth;
	int32				CurrentHeight;
	int32				CurrentBpp;

	int32				OldWindowWidth;						// Old client width
	int32				OldWindowHeight;
	int32				WindowXOffset;
	int32				WindowYOffset;

	RECT				OldWindowRect;
	ULONG				OldGWL_STYLE;

	jeBoolean			ModeSet;

	char				DDName[MAX_DRIVER_NAME];			// Have no idea how big to make this.  Anyone?

	LPDIRECTDRAW7		lpDD;								// The current initialized DD object
    LPDIRECT3D7		    lpD3D;								// The current initialized D3D object

    LPDIRECTDRAWSURFACE7	lpFrontBuffer;					// front buffer surface 
    LPDIRECTDRAWSURFACE7	lpBackBuffer;					// back buffer surface 
    LPDIRECTDRAWSURFACE7	lpZBuffer;						// z-buffer surface 
	LPDIRECTDRAWCLIPPER		lpClipper;						// Clipper in windowed case
    BOOL					BackBufferInVideo;				// back buf in video mem? 
    BOOL					ZBufferInVideo;					// is Z-buf in video mem? 
    LPDIRECT3DDEVICE7		lpD3DDevice;					// D3D device 
//	LPD3DXFONT							g_Font;

	// 2d surface format (for blt'ing to the display)
	DDSURFACEDESC2		ddSurfFormat;						// 555 or 565 surface desc

	// Texture formats (for the D3D device)
	DDSURFACEDESC2		ddTexFormat;						// 555 or 565 surface desc
	DDSURFACEDESC2		ddFourBitAlphaSurfFormat;			// 4444 surface desc
	DDSURFACEDESC2		ddOneBitAlphaSurfFormat;			// 1555 surface desc

	RGB_LUT				Lut1;
	RGB_LUT				Lut2;
	RGB_LUT				Lut3;

	BOOL				IsPrimary;							// 
	BOOL				FullScreen;
   BOOL           WireFrame;
	
	int32				NumModes;
	App_Mode			Modes[MAX_APP_MODES];

	int32				NumDrivers;
	DDMain_D3DDriver	Drivers[DDMAIN_MAX_D3D_DRIVERS];
	int32				CurrentDriver;
	DWORD				dwTextureOpCaps;

	// Surface formats
	int32				NumSurfFormats;						// Num 2D texture formats avail (from DD4 object)
    DDMain_SurfFormat	SurfFormats[DDMAIN_MAX_SURFACE_FORMATS];

    // Texture formats
	int32				NumTextureFormats;					// Num 3D texture formats avail (from device)
    DDMain_SurfFormat	TextureFormats[DDMAIN_MAX_TEXTURE_FORMATS];

	BOOL				LogToFile;
	BOOL				FoundGoodDevice;
	BOOL				CanDoWindow;
	
	BOOL				RenderingIsOK;

	DWORD				VidMemFree;

	float				Gamma;
	BOOL				GammaChanged;

	jeBoolean			CanDoMultiTexture;

	// DD / D3D Flags
	uint32				Flags;

	
	int32				MaxTextureWidth;
	int32				MaxTextureHeight;
	int32				MaxSurfaceCount;
	FOGI				Fog;

} D3DMain_GlobalInfo;

// DD enum strcuture.  Used when enuming dd
typedef struct
{
	LPDIRECTDRAW7	lpDD;
	char			DriverName[MAX_DRIVER_NAME];
	BOOL			FoundDD;
} DD_Enum;

//================================================================================
//	Globals
//================================================================================
extern D3DMain_GlobalInfo	D3DInfo;			// Our global structure that knows all... (once initialized)

//================================================================================
//	Global functions
//================================================================================
BOOL				D3DMain_InitD3D(HWND hWnd, const char *DriverName, int32 Width, int32 Height);
BOOL				D3DMain_ShutdownD3D(void);
jeBoolean			D3DMain_Reset(void);
void				D3DMain_Log(LPSTR Str, ... );
BOOL				D3DMain_RestoreAllSurfaces(void);

BOOL				Main_EnumTextureFormats(void);
BOOL				D3DMain_EnumDisplayModes(void);
BOOL				Main_ClearBackBuffer(BOOL Clear, BOOL ClearZ);
BOOL				Main_ShowBackBuffer(void);

BOOL				D3DMain_GetSurfaceFormats(void);

BOOL				Main_CheckDD(void);
BOOL				D3DMain_GetTextureMemory(void);
void				Main_BuildRGBGammaTables(float Gamma);

BOOL				D3DMain_GetClientWindowOffset(HWND hWnd);
jeBoolean DRIVERCC	D3DMain_UpdateWindow(void);
jeBoolean DRIVERCC	D3DMain_SetActive(jeBoolean wParam);

BOOL DRIVERCC EnumSubDrivers(DRV_ENUM_DRV_CB *Cb, void *Context);
BOOL DRIVERCC EnumModes(int32 Driver, char *DriverName, DRV_ENUM_MODES_CB *Cb, void *Context);

// utility crap
uint32 Log2(uint32 P2);
int32 SnapToPower2(int32 Width);
int32 GetLog(int32 Width, int32 Height);

void Draw_Text(char *text, int x,int y,
               COLORREF color,
               LPDIRECTDRAWSURFACE7 lpdds,
               HFONT fnt);

// BEGIN - Render state access - paradoxnj 12/25/2005
jeBoolean DRIVERCC D3DMain_SetRenderState(uint32 state, uint32 value);
// END - Render state access - paradoxnj 12/25/2005

#endif