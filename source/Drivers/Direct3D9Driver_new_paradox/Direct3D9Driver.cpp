#include "Direct3D9Driver.h"
#include "pixelformat.h"
#include "D3D9TextureMgr.h"
#include "PolyCache.h"
#include "D3D9Log.h"

/*
#ifdef _DEBUG
#pragma comment(lib, "Jet3Dd.lib")
#else
#pragma comment(lib, "Jet3D.lib")
#endif
*/
#ifdef _DEBUG
#pragma comment(lib, "Jet3DClassic10d.lib")
#else
#pragma comment(lib, "Jet3DClassic10.lib")
#endif


jeRDriver_PixelFormat							PixelFormat[10];
DRV_EngineSettings								EngineSettings;
HWND											hWnd{};
IDirect3D9* pD3D = nullptr;
IDirect3DDevice9* pDevice = nullptr;
float											localgamma{};

D3DCAPS9										g_Caps{};
bool											CanDoAntiAlias = false;
bool											CanDoShaders = false;
bool											CanDoAnisotropic = false;

D3DDISPLAYMODE									old_mode{};

ID3DXSprite* pSprite = nullptr;
ID3DXFont* pFont = nullptr;

PolyCache* g_pPolyCache = nullptr;

#define JE_FONT_NORMAL			0x00000001
#define JE_FONT_BOLD			0x00000002

RGB_LUT Lut1{};

typedef struct jeFont
{
	ID3DXFont* pFont;
} jeFont;

static jeBoolean PrepPolyVerts(jeTLVertex* Pnts, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, void* LMapCBContext);
static void FillLMapSurface(jeTLVertex* Pnts, int32 NumPoints, jeTexture* THandle, jeRDriver_LMapCBInfo* Info, int32 LNum);

inline DWORD FtoDW(FLOAT f) { return *((DWORD*)& f); }

void jeXForm3d_ToD3DMatrix(jeXForm3d* XForm, D3DMATRIX* mat)
{
	mat->_11 = XForm->AX;
	mat->_12 = XForm->AY;
	mat->_13 = XForm->AZ;
	mat->_14 = 0.0f;

	mat->_21 = XForm->BX;
	mat->_22 = XForm->BY;
	mat->_23 = XForm->BZ;
	mat->_24 = 0.0f;

	mat->_31 = XForm->CX;
	mat->_32 = XForm->CY;
	mat->_33 = XForm->CZ;
	mat->_34 = 0.0f;

	mat->_41 = XForm->Translation.X;
	mat->_42 = XForm->Translation.Y;
	mat->_43 = XForm->Translation.Z;
	mat->_44 = 1.0f;
}

void D3DMatrix_ToXForm3d(D3DMATRIX* mat, jeXForm3d* XForm)
{
	XForm->AX = mat->_11;
	XForm->AY = mat->_12;
	XForm->AZ = mat->_13;

	XForm->BX = mat->_21;
	XForm->BY = mat->_22;
	XForm->BZ = mat->_23;

	XForm->CX = mat->_31;
	XForm->CY = mat->_32;
	XForm->CZ = mat->_33;

	XForm->Translation.X = mat->_41;
	XForm->Translation.Y = mat->_42;
	XForm->Translation.Z = mat->_43;
}

jeBoolean DRIVERCC D3D9Drv_EnumSubDrivers(DRV_ENUM_DRV_CB* Cb, void* Context)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  EnumSubDrivers()");
	else
		REPORT("Function Call:  EnumSubDrivers()");

	Cb(1, "Direct3D 9 Driver", Context);
	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_EnumModes(S32 Driver, char* DriverName, DRV_ENUM_MODES_CB* Cb, void* Context)
{
	HRESULT							hres{};
	D3DFORMAT						fmt[] = { D3DFMT_X8R8G8B8, D3DFMT_R5G6B5 };
	int32							y = 0, numModes = 0;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  EnumModes()");
	else
		REPORT("Function Call:  EnumModes()");

	for (int32 x = 0; x < 2; x++)
	{
		int32 modecount = pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, fmt[x]);
		for (y = 0; y < modecount; y++)
		{
			D3DDISPLAYMODE			mode{};

			hres = pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, fmt[x], y, &mode);
			if (FAILED(hres))
			{
				LOG("ERROR:  Could not enumerate display modes!!");
				return FALSE;
			}

			if (mode.Width <= 2048 && mode.Height <= 1024 && mode.RefreshRate == 60)
			{
				char				modename[32];
				int32				bpp{};

				if (mode.Format == D3DFMT_X8R8G8B8)
					bpp = 32;
				else if (mode.Format == D3DFMT_R5G6B5)
					bpp = 16;
				else
					bpp = 0;

				sprintf_s(modename, "%dx%dx%d", mode.Width, mode.Height, bpp);

				Cb(y, modename, mode.Width, mode.Height, bpp, Context);
				numModes++;
			}
		}
	}

	Cb(numModes, "WindowMode", -1, -1, -1, Context);

	return TRUE;
}

jeBoolean DRIVERCC D3D9Drv_Init(DRV_DriverHook* hook)
{
	HRESULT						hres{};
	D3DPRESENT_PARAMETERS		d3dpp{};
	int32						bpp{};
	int32						dummy{};
	const char* modename = nullptr;
	int							w{}, h{};

	if (LOG_LEVEL > 1)
		LOG("Function Call:  Init()");
	else
		REPORT("Function Call:  Init()");

	if (pDevice != nullptr)
	{
		SAFE_RELEASE(pDevice);
	}

	pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &old_mode);

	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	sscanf_s(hook->ModeName, "%dx%dx%d", &w, &h, &bpp);
	if (hook->Width == -1 && hook->Height == -1)
	{
		RECT					r{};

		GetClientRect(hook->hWnd, &r);

		d3dpp.BackBufferWidth = r.right - r.left;
		d3dpp.BackBufferHeight = r.bottom - r.top;
		d3dpp.BackBufferFormat = old_mode.Format;
		d3dpp.Windowed = JE_TRUE;
	}
	else
	{
		d3dpp.BackBufferWidth = hook->Width;
		d3dpp.BackBufferHeight = hook->Height;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

		if (bpp == 32)
			d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		else
			d3dpp.BackBufferFormat = D3DFMT_R5G6B5;

		d3dpp.FullScreen_RefreshRateInHz = 60;
		d3dpp.Windowed = FALSE;	
		
	}

	sscanf_s(hook->ModeName, "%dx%dx%d", &dummy, &dummy, &bpp);

	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hook->hWnd;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;  // original
//	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;	//	trilobite

	hres = pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, d3dpp.Windowed, D3DMULTISAMPLE_2_SAMPLES, NULL);
	if (FAILED(hres))
	{
		LOG("Feature Request:  Full Scene Anti-Aliasing OFF");
		CanDoAntiAlias = false;
	}
	else
	{
		LOG("Feature Request:  Full Scene Anti-Aliasing ON");
		CanDoAntiAlias = true;
	}

	if (!CanDoAntiAlias)
		d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	else
		d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;

	d3dpp.MultiSampleQuality = 0;
	d3dpp.Flags = 0;

	hres = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hook->hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
	if (FAILED(hres))
	{
		hres = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hook->hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
		if (FAILED(hres))
		{
			hres = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_SW, hook->hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
			if (FAILED(hres))
			{
				hres = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hook->hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
				if (FAILED(hres))
				{
					if (hres == D3DERR_DEVICELOST)
						LOG("ERROR:  Device Lost!!");
					else if (hres == D3DERR_INVALIDCALL)
						LOG("ERROR:  Invalid Call!!");
					else if (hres == D3DERR_NOTAVAILABLE)
						LOG("ERROR:  Not Available!!");
					else if (hres == D3DERR_OUTOFVIDEOMEMORY)
						LOG("ERROR:  Out of video memory!!");
					else
						LOG("ERROR:  Unknown error!!");

					return JE_FALSE;
				}
				else
					LOG("DEVICE:  Created using REF/SW");
			}
			else
				LOG("DEVICE:  Created using SW/SW");
		}
		else
			LOG("DEVICE:  Created using HW/SW");
	}
	else
		LOG("DEVICE:  Created using HW/HW");

	pDevice->GetDeviceCaps(&g_Caps);

	if (g_Caps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY)
	{
		LOG("Feature Request:  Anisotropic Filtering ON");
		CanDoAnisotropic = true;
	}
	else
		LOG("Feature Request:  Anisotropic Filtering OFF");

	if ((g_Caps.VertexShaderVersion >= D3DVS_VERSION(1, 1)) && (g_Caps.PixelShaderVersion >= D3DPS_VERSION(1, 4)))
	{
		LOG("Feature Request:  Vertex and Pixel Shaders ON");
		CanDoShaders = true;
	}
	else
		LOG("Feature Request:  Vertex and Pixel Shaders OFF");

	if (CanDoAntiAlias)
		pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

	pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	g_pPolyCache = new PolyCache();
	if (!g_pPolyCache)
	{
		LOG("ERROR:  Could not create poly cache!!");
		return FALSE;
	}

	if (!g_pPolyCache->Initialize(pDevice))
	{
		LOG("ERROR:  Could not initialize poly cache!!");
		return FALSE;
	}

	D3DXMATRIX matProj{};
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 4000.0f);
	pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	localgamma = 1.0f;

	BuildRGBGammaTables(1);

	if (FAILED(D3DXCreateSprite(pDevice, &pSprite)))
	{
		LOG("ERROR:  Could not create sprite interface!!");
		return FALSE;
	}

	D3DXCreateFont(pDevice, 18, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &pFont);

	if (LOG_LEVEL > 1)
		LOG("DEBUG:  Initialization successful");
	else
		REPORT("DEBUG:  Initialization complete...");

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_Shutdown(void)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  Shutdown()");
	else
		REPORT("Function Call:  Shutdown()");

/*
The problem is that the Fullscreen image contines to display in front of
the editor, even after the d3d9 driver has shutdown. Hides the editor.

Below are attempts to clear the screen.

	HRESULT			hRes{};
	D3DDISPLAYMODE* pMode{};
//	hRes = pDevice->GetDisplayMode(1, pMode);
//	hRes = pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, pMode);
//	if (hRes == D3D_OK)
	{
		UINT height, width;
		
//		height = pMode->Height;
//		width = pMode->Width;
//		D3DRECT* pRect = new D3DRECT;
//		pRect->x1 = 0;
//		pRect->x2 = 1024;
//		pRect->y1 = 0;
//		pRect->y2 = 768;

		HRESULT hClearRes{};
//		hClearRes = pDevice->Clear(1, pRect, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, NULL, 1, NULL);

		hClearRes = pDevice->Clear(0, NULL, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		if (hClearRes)
		{
			int i = 1;
		}

	D3DVIEWPORT9* pViewport{};
	HRESULT hVResult{};

	hVResult = pDevice->GetViewport(pViewport);
	if (hVResult == D3D_OK)
	{
		if (pViewport != nullptr)
			SAFE_DELETE(pViewport);
			
	}

//	}

*/	

	
//

	SAFE_RELEASE(pFont);
	if (pFont != nullptr)
	{
		;
	}
	

	SAFE_DELETE(g_pPolyCache);
	if (g_pPolyCache != nullptr)
	{
		;
	}


	if (!D3D9_THandle_Shutdown())
	{
		;
	}


	SAFE_RELEASE(pDevice);
	if (pDevice != nullptr)
	{
		;
	}

	
	SAFE_RELEASE(pD3D);
	if (pD3D != nullptr)
	{
		;
	}

	LOG("Shutdown complete...");
	//D3D9Log::GetPtr()->Shutdown();
	D3D9Log::getSingletonPtr()->Shutdown();

	//	SAFE_DELETE(D3D9Log::GetPtr());
	return JE_TRUE;
}

/*jeRDriver_PixelFormat PFormats[] =
{
	{	JE_PIXELFORMAT_32BIT_ARGB, RDRIVER_PF_3D | RDRIVER_PF_COMBINE_LIGHTMAP },
	{	JE_PIXELFORMAT_24BIT_RGB, RDRIVER_PF_2D | RDRIVER_PF_CAN_DO_COLORKEY },
	{	JE_PIXELFORMAT_24BIT_RGB, RDRIVER_PF_LIGHTMAP }
};

#define NUM_PIXEL_FORMATS				(sizeof(PFormats) / sizeof(jeRDriver_PixelFormat))

jeBoolean DRIVERCC D3D9Drv_EnumPixelFormats(DRV_ENUM_PFORMAT_CB *Cb, void *Context)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  EnumPixelFormats()");
	else
		REPORT("Function Call:  EnumPixelFormats()");

	for (int i = 0; i < NUM_PIXEL_FORMATS; i++)
	{
		if (!Cb(&PFormats[i], Context))
		{
			LOG("ERROR:  Cannot enumerate pixel formats!!");
			return JE_FALSE;
		}
	}

	return JE_TRUE;
}*/

jeBoolean DRIVERCC D3D9Drv_EnumPixelFormats(DRV_ENUM_PFORMAT_CB* Cb, void* Context)
{
	int32			i{};
	jePixelFormat	Format3d{}, Format2d{};
	uint32			CurrentBpp{};
	D3DDISPLAYMODE	mode{};

	pDevice->GetDisplayMode(0, &mode);

	if (mode.Format == D3DFMT_X8R8G8B8 || mode.Format == D3DFMT_A8R8G8B8 || mode.Format == D3DFMT_A2R10G10B10)
		CurrentBpp = 32;
	else if (mode.Format == D3DFMT_R5G6B5 || mode.Format == D3DFMT_A1R5G5B5 || mode.Format == D3DFMT_X1R5G5B5)
		CurrentBpp = 16;

	// Setup the 2d surface format
	if (CurrentBpp == 32 && mode.Format == D3DFMT_A8R8G8B8)
		Format2d = JE_PIXELFORMAT_32BIT_ARGB;
	else if (CurrentBpp == 32 && mode.Format == D3DFMT_X8R8G8B8)
		Format2d = JE_PIXELFORMAT_32BIT_XRGB;
	else if (CurrentBpp == 16 && mode.Format == D3DFMT_A1R5G5B5)
		Format2d = JE_PIXELFORMAT_16BIT_1555_ARGB;
	else if (CurrentBpp == 16 && mode.Format == D3DFMT_R5G6B5)
		Format2d = JE_PIXELFORMAT_16BIT_565_RGB;
	else
		Format2d = JE_PIXELFORMAT_16BIT_555_RGB;

	// Setup the 3d (Texture) format
	//if (mode.Format == D3DFMT_R5G6B5)
	//	Format3d = JE_PIXELFORMAT_16BIT_555_RGB;
	//else
	Format3d = JE_PIXELFORMAT_16BIT_565_RGB;
	//Format3d = JE_PIXELFORMAT_32BIT_ARGB;

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
	for (i = 0; i < 5; i++)
	{
		if (!Cb(&PixelFormat[i], Context))
			return JE_TRUE;
	}

	return TRUE;
}

jeBoolean DRIVERCC D3D9Drv_GetDeviceCaps(jeDeviceCaps* DeviceCaps)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  GetDeviceCaps()");
	else
		REPORT("Function Call:  GetDeviceCaps()");

	DeviceCaps->SuggestedDefaultRenderFlags = JE_RENDER_FLAG_BILINEAR_FILTER;
	DeviceCaps->CanChangeRenderFlags = 0xFFFFFFFF;
	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_SetGamma(float gamma)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  SetGamma()");
	else
		REPORT("Function Call:  SetGamma()");

	localgamma = gamma;
	BuildRGBGammaTables(gamma);
	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_GetGamma(float* gamma)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  GetGamma()");
	else
		REPORT("Function Call:  GetGamma()");

	*gamma = localgamma;
	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_Reset()
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  Reset()");
	else
		REPORT("Function Call:  Reset()");

	D3D9_THandle_Shutdown();
	g_pPolyCache->Shutdown();

	D3D9_THandle_Startup();
	g_pPolyCache->Initialize(pDevice);

	return TRUE;
}

jeBoolean DRIVERCC D3D9Drv_UpdateWindow()
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  UpdateWindow()");
	else
		REPORT("Function Call:  UpdateWindow()");

	return TRUE;
}

jeBoolean DRIVERCC D3D9Drv_SetActive(jeBoolean Active)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  SetActive()");
	else
		REPORT("Function Call:  SetActive()");

	Active;
	return TRUE;
}

jeBoolean DRIVERCC D3D9Drv_BeginScene(jeBoolean Clear, jeBoolean ClearZ, RECT* WorldRect, jeBoolean Wireframe)
{
	HRESULT							hres{};

	if (LOG_LEVEL > 1)
		LOG("Function Call:  BeginScene()");
	else
		REPORT("Function Call:  BeginScene()");

	if (Clear)
	{
		g_D3D9Drv.NumRenderedPolys = 0;
		pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0L);
	}

	if (ClearZ)
	{
		pDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0L);
	}

	if (Wireframe)
		pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	hres = pDevice->BeginScene();
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not begin scene!!");
		return JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_EndScene()
{
	HRESULT							hres{};

	if (LOG_LEVEL > 1)
		LOG("Function Call:  EndScene()");
	else
		REPORT("Function Call:  EndScene()");

	if (!g_pPolyCache->Flush())
	{
		LOG("ERROR:  Failed to flush poly cache!!");
		return JE_FALSE;
	}

	hres = pDevice->EndScene();
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not end scene!!");
		return JE_FALSE;
	}

	hres = pDevice->Present(NULL, NULL, NULL, NULL);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not present scene!!");
		return JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_BeginBatch()
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  BeginBatch()");
	else
		REPORT("Function Call:  BeginBatch()");

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_EndBatch()
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  EndBatch()");
	else
		REPORT("Function Call:  EndBatch()");

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_RenderGouraudPoly(jeTLVertex* Pnts, int32 NumPoints, uint32 Flags)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  RenderGouraudPoly()");
	else
		REPORT("Function Call:  RenderGouraudPoly()");

	if (!g_pPolyCache)
	{
		LOG("ERROR:  Poly cache not initialized!!");
		return JE_FALSE;
	}

	return g_pPolyCache->AddGouraudPoly(Pnts, NumPoints, Flags);
}
jeBoolean DRIVERCC D3D9Drv_RenderWorldPoly(jeTLVertex* Pnts, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, void* LMapCBContext, uint32 Flags)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  RenderWorldPoly()");
	else
		REPORT("Function Call:  RenderWorldPoly()");

	if (!g_pPolyCache)
	{
		LOG("ERROR:  Poly cache not initialized!!");
		return JE_FALSE;
	}

	return g_pPolyCache->AddWorldPoly(Pnts, NumPoints, Layers, NumLayers, LMapCBContext, Flags);
}
/*
typedef struct
{
	float x, y, z, pad;								// screen points
	float r, g, b, a;								// color
	float u, v, pad1,pad2;							// Uv's
	float sr, sg, sb, pad3;							// specular color
} jeTLVertex;	// 64 bytes

#define JE_RENDER_FLAG_ALPHA				(1<<0)	// Alpha in the vertices are valid
#define JE_RENDER_FLAG_SPECULAR				(1<<1)	// Specular in the vertices are valid
#define JE_RENDER_FLAG_COLORKEY				(1<<2)	// Texture format has color key on the poly being rendered
#define JE_RENDER_FLAG_CLAMP_UV				(1<<3)	// Clamp U and V in BOTH directions
#define JE_RENDER_FLAG_COUNTER_CLOCKWISE	(1<<4)	// Winding of poly will be counter-clockwise
#define JE_RENDER_FLAG_NO_ZTEST				(1<<5)	// No ZTest should be performed
#define JE_RENDER_FLAG_NO_ZWRITE			(1<<6)	// No ZWrites should be performed
#define JE_RENDER_FLAG_STEST				(1<<7)	// Span test should be performed (if set, polys should be front to back)
#define JE_RENDER_FLAG_SWRITE				(1<<8)	// Spans should be written to the sbuffer
#define JE_RENDER_FLAG_FLUSHBATCH			(1<<9)	// Flushes the current batch of polys (if any), and the current poly
#define JE_RENDER_FLAG_BILINEAR_FILTER		(1<<10) // Enable bilinear filtering
*/

jeBoolean DRIVERCC D3D9Drv_RenderMiscTexturePoly(jeTLVertex* Pnts, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, uint32 Flags)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  RenderMiscTexturePoly()");
	else
		REPORT("Function Call:  RenderMiscTexturePoly()");

	if (!g_pPolyCache)
	{
		LOG("ERROR:  Poly cache not initialized!!");
		return JE_FALSE;
	}

	return g_pPolyCache->AddMiscTexturePoly(Pnts, NumPoints, Layers, NumLayers, Flags);
}

jeBoolean DRIVERCC D3D9Drv_DrawDecal(jeTexture* Handle, RECT* SrcRect, int32 x, int32 y)
{
	HRESULT						hres;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  DrawDecal()");
	else
		REPORT("Function Call:  DrawDecal()");

	hres = pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not begin sprite frame!!");
		return JE_FALSE;
	}

	hres = pSprite->Draw(Handle->pTexture, NULL, NULL, &D3DXVECTOR3((float)x, (float)y, 1.0f), D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not draw sprite!!");
		return JE_FALSE;
	}

	hres = pSprite->End();
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not end sprite frame!!");
		return JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_Screenshot(const char* filename)
{
	HRESULT						hres{};
	IDirect3DSurface9* pSurface = nullptr;
	D3DDISPLAYMODE				dmode{};

	if (LOG_LEVEL > 1)
		LOG("Function Call:  Screenshot()");
	else
		REPORT("Function Call:  Screenshot()");

	hres = pDevice->GetDisplayMode(0, &dmode);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not get display mode!!");
		return JE_FALSE;
	}

	hres = pDevice->CreateOffscreenPlainSurface(dmode.Width, dmode.Height, dmode.Format, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not create offscreen plain surface!!");
		return JE_FALSE;
	}

	hres = pDevice->GetFrontBufferData(0, pSurface);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not get front buffer data!!");
		pSurface->Release();
		pSurface = nullptr;
		return JE_FALSE;
	}

	hres = D3DXSaveSurfaceToFile(filename, D3DXIFF_BMP, pSurface, NULL, NULL);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not save screenshot!!");
		pSurface->Release();
		pSurface = nullptr;
		return JE_FALSE;
	}

	pSurface->Release();
	pSurface = nullptr;

	return JE_TRUE;
}

uint32 DRIVERCC D3D9Drv_CreateStaticMesh(jeHWVertex* Points, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, uint32 Flags)
{
	uint32						id;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  CreateStaticMesh()");
	else
		REPORT("Function Call:  CreateStaticMesh()");

	if (!g_pPolyCache)
	{
		LOG("ERROR:  No poly cache!!");
		return 0;
	}

	id = g_pPolyCache->AddStaticBuffer(Points, NumPoints, Layers, NumLayers, Flags);
	return id;
}

jeBoolean DRIVERCC D3D9Drv_RemoveStaticMesh(uint32 id)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  RemoveStaticMesh()");
	else
		REPORT("Function Call:  RemoveStaticMesh()");

	if (!g_pPolyCache)
	{
		LOG("ERROR:  No poly cache!!");
		return JE_FALSE;
	}

	return g_pPolyCache->RemoveStaticBuffer(id);
}

jeBoolean DRIVERCC D3D9Drv_RenderStaticMesh(uint32 id, int32 StartVertex, int32 NumPolys, jeXForm3d* XForm)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  RenderStaticMesh()");
	else
		REPORT("Function Call:  RenderStaticMesh()");

	if (!g_pPolyCache)
	{
		LOG("ERROR:  No poly cache!!");
		return JE_FALSE;
	}

	return g_pPolyCache->RenderStaticBuffer(id, StartVertex, NumPolys, XForm);
}

jeBoolean DRIVERCC D3D9Drv_SetMatrix(uint32 Type, jeXForm3d* Matrix)
{
	D3DTRANSFORMSTATETYPE				t;
	D3DMATRIX							out;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  SetMatrix()");
	else
		REPORT("Function Call:  SetMatrix()");

	switch (Type)
	{
	case JE_XFORM_TYPE_WORLD:
	{
		t = D3DTS_WORLD;
		break;
	}
	case JE_XFORM_TYPE_VIEW:
	{
		t = D3DTS_VIEW;
		break;
	}
	case JE_XFORM_TYPE_PROJECTION:
	{
		t = D3DTS_PROJECTION;
		break;
	}
	default:
		return JE_FALSE;
	}

	jeXForm3d_ToD3DMatrix(Matrix, &out);
	pDevice->SetTransform(t, &out);

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_GetMatrix(uint32 Type, jeXForm3d* Matrix)
{
	D3DTRANSFORMSTATETYPE				t;
	D3DMATRIX							out;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  GetMatrix()");
	else
		REPORT("Function Call:  GetMatrix()");

	switch (Type)
	{
	case JE_XFORM_TYPE_WORLD:
	{
		t = D3DTS_WORLD;
		break;
	}
	case JE_XFORM_TYPE_VIEW:
	{
		t = D3DTS_VIEW;
		break;
	}
	case JE_XFORM_TYPE_PROJECTION:
	{
		t = D3DTS_PROJECTION;
		break;
	}
	default:
		return JE_FALSE;
	}

	pDevice->GetTransform(t, &out);
	D3DMatrix_ToXForm3d(&out, Matrix);

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_SetCamera(jeCamera* Camera)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  SetCamera()");
	else
		REPORT("Function Call:  SetCamera()");

	return JE_TRUE;
}

jeFont* DRIVERCC D3D9Drv_CreateFont(int32 Height, int32 Width, uint32 Weight, jeBoolean Italic, const char* facename)
{
	jeFont* font = nullptr;
	HRESULT					hres{};
	uint32					w{};

	if (LOG_LEVEL > 1)
		LOG("Function Call:  CreateFont()");
	else
		REPORT("Function Call:  EnumSubDrivers()");

	if (Weight == JE_FONT_BOLD)
		w = FW_BOLD;
	else
		w = 0;

	font = new jeFont;
	if (!font)
	{
		LOG("ERROR:  Out of memory!!");
		return nullptr;
	}

	hres = D3DXCreateFont(pDevice, Height, Width, Weight, 0, Italic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT(facename), &font->pFont);
	if (FAILED(hres))
	{
		LOG("ERROR:  Could not create font!!");
		delete font;
		font = nullptr;

		return nullptr;
	}

	return font;
}

jeBoolean DRIVERCC D3D9Drv_DrawFont(jeFont* Font, int32 x, int32 y, uint32 Color, const char* text)
{
	RECT						r;

	if (LOG_LEVEL > 1)
		LOG("Function Call:  DrawFont()");

	pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	SetRect(&r, x, y, 0, 0);

	if (FAILED(Font->pFont->DrawText(NULL, text, (int)strlen(text), &r, DT_NOCLIP, Color)))
	{
		LOG("ERROR:  Could not draw font!!");
		return JE_FALSE;
	}

	pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_DestroyFont(jeFont** Font)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  DestroyFont()");
	else
		REPORT("Function Call:  DestroyFont()");

	(*Font)->pFont->Release();
	(*Font)->pFont = nullptr;

	delete (*Font);
	(*Font) = nullptr;

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_SetRenderState(uint32 state, uint32 value)
{
	HRESULT								hres{};
	DWORD								val{};

	if (LOG_LEVEL > 1)
		LOG("Function Call:  SetRenderState()");
	else
		REPORT("Function Call:  SetRenderState()");

	switch (state)
	{
	case JE_RENDERSTATE_ENABLE_ZBUFFER:
	{
		if ((jeBoolean)value == JE_TRUE)
			val = D3DZB_TRUE;
		else
			val = D3DZB_FALSE;

		hres = pDevice->SetRenderState(D3DRS_ZENABLE, val);
		break;
	}
	case JE_RENDERSTATE_ENABLE_ZWRITES:
	{
		if ((jeBoolean)value == JE_TRUE)
			val = TRUE;
		else
			val = FALSE;

		hres = pDevice->SetRenderState(D3DRS_ZWRITEENABLE, val);
		break;
	}
	case JE_RENDERSTATE_ENABLE_ALPHABLENDING:
	{
		if ((jeBoolean)value == JE_TRUE)
			val = TRUE;
		else
			val = FALSE;

		hres = pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, val);
		break;
	}
	case JE_RENDERSTATE_ENABLE_ALPHATESTING:
	{
		if ((jeBoolean)value == JE_TRUE)
			val = TRUE;
		else
			val = FALSE;

		hres = pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, val);
		break;
	}
	case JE_RENDERSTATE_ALPHAREF:
	{
		hres = pDevice->SetRenderState(D3DRS_ALPHAREF, value);
		break;
	}
	case JE_RENDERSTATE_ALPHAFUNC:
	case JE_RENDERSTATE_DEPTHFUNC:
	case JE_RENDERSTATE_STENCILFUNC:
	{
		D3DCMPFUNC				func{};
		D3DRENDERSTATETYPE		st{};

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
			st = D3DRS_ALPHAFUNC;
		else if (state == JE_RENDERSTATE_DEPTHFUNC)
			st = D3DRS_ZFUNC;
		else if (state == JE_RENDERSTATE_STENCILFUNC)
			st = D3DRS_STENCILFUNC;

		hres = pDevice->SetRenderState(st, func);
		break;
	}
	case JE_RENDERSTATE_FILLMODE:
	{
		D3DFILLMODE					fm{};

		if (value == JE_FILL_POINT)
			fm = D3DFILL_POINT;
		else if (value == JE_FILL_WIREFRAME)
			fm = D3DFILL_WIREFRAME;
		else if (value == JE_FILL_SOLID)
			fm = D3DFILL_SOLID;

		hres = pDevice->SetRenderState(D3DRS_FILLMODE, fm);
		break;
	}
	case JE_RENDERSTATE_SHADEMODE:
	{
		D3DSHADEMODE				sm{};

		if (value == JE_SHADE_FLAT)
			sm = D3DSHADE_FLAT;
		else if (value == JE_SHADE_GOURAUD)
			sm = D3DSHADE_GOURAUD;
		else if (value == JE_SHADE_PHONG)
			sm = D3DSHADE_PHONG;

		hres = pDevice->SetRenderState(D3DRS_SHADEMODE, sm);
		break;
	}
	case JE_RENDERSTATE_CULLMODE:
	{
		D3DCULL						cm{};

		if (value == JE_CULL_NONE)
			cm = D3DCULL_NONE;
		else if (value == JE_CULL_CW)
			cm = D3DCULL_CW;
		else if (value == JE_CULL_CCW)
			cm = D3DCULL_CCW;

		hres = pDevice->SetRenderState(D3DRS_CULLMODE, cm);
		break;
	}
	case JE_RENDERSTATE_ENABLE_FOG:
	{
		if ((jeBoolean)value == JE_TRUE)
			val = TRUE;
		else
			val = FALSE;

		hres = pDevice->SetRenderState(D3DRS_FOGENABLE, val);
		break;
	}
	case JE_RENDERSTATE_FOGCOLOR:
	{
		hres = pDevice->SetRenderState(D3DRS_FOGCOLOR, value);
		break;
	}
	case JE_RENDERSTATE_FOGSTART:
	{
		hres = pDevice->SetRenderState(D3DRS_FOGSTART, value);
		break;
	}
	case JE_RENDERSTATE_FOGEND:
	{
		hres = pDevice->SetRenderState(D3DRS_FOGEND, value);
		break;
	}
	case JE_RENDERSTATE_HWLIGHTINGENABLE:
	{
		if ((jeBoolean)value == JE_TRUE)
			val = TRUE;
		else
			val = FALSE;

		hres = pDevice->SetRenderState(D3DRS_LIGHTING, val);
		break;
	}
	case JE_RENDERSTATE_AMBIENTLIGHT:
	{
		hres = pDevice->SetRenderState(D3DRS_AMBIENT, value);
		break;
	}
	case JE_RENDERSTATE_ENABLE_STENCIL:
	{
		if ((jeBoolean)value == JE_TRUE)
			val = TRUE;
		else
			val = FALSE;

		hres = pDevice->SetRenderState(D3DRS_STENCILENABLE, val);
		break;
	}
	case JE_RENDERSTATE_STENCILREF:
	{
		hres = pDevice->SetRenderState(D3DRS_STENCILREF, value);
		break;
	}
	case JE_RENDERSTATE_STENCILMASK:
	{
		hres = pDevice->SetRenderState(D3DRS_STENCILMASK, value);
		break;
	}
	case JE_RENDERSTATE_STENCILWRITEMASK:
	{
		hres = pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, value);
		break;
	}
	case JE_RENDERSTATE_STENCILFAIL:
	case JE_RENDERSTATE_STENCILZFAIL:
	{
		D3DSTENCILOP			so{};
		D3DRENDERSTATETYPE		rs{};

		if (state == JE_RENDERSTATE_STENCILFAIL)
			rs = D3DRS_STENCILFAIL;
		else if (state == JE_RENDERSTATE_STENCILZFAIL)
			rs = D3DRS_STENCILZFAIL;
		else if (state == JE_RENDERSTATE_STENCILPASS)
			rs = D3DRS_STENCILPASS;

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

		hres = pDevice->SetRenderState(rs, so);
		break;
	}
	}

	if (FAILED(hres))
	{
		LOG("ERROR:  Could not set render state!!");
		return JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9Drv_DrawText(char* text, int x, int y, uint32 color)
{
	RECT							r{};

	if (LOG_LEVEL > 1)
		LOG("Function Call:  DrawText()");
	else
		REPORT("Function Call:  DrawText()");

	SetRect(&r, x, y, 0, 0);

	if (FAILED(pFont->DrawText(NULL, text, (int)strlen(text), &r, DT_NOCLIP, color)))
		return JE_FALSE;

	return JE_TRUE;
}

D3D9Driver g_D3D9Drv = {

	"Direct3D 9 Driver.  Copyright 2004-2006, Paradox Software",
	DRV_VERSION_MAJOR,
	DRV_VERSION_MINOR,

	// Error handling hooks set by driver
	DRV_ERROR_NONE,
	NULL,

	// Enum Modes/Drivers
	D3D9Drv_EnumSubDrivers,
	D3D9Drv_EnumModes,

	D3D9Drv_EnumPixelFormats,

	// Device Caps
	D3D9Drv_GetDeviceCaps,

	// Init/DeInit functions
	D3D9Drv_Init,
	D3D9Drv_Shutdown,
	D3D9Drv_Reset,
	NULL,
	D3D9Drv_SetActive,

	// Create/Destroy texture functions
	D3D9_THandle_Create,
	D3D9_THandle_CreateFromFile,
	D3D9_THandle_Destroy,

	// Texture manipulation functions
	D3D9_THandle_Lock,
	D3D9_THandle_Unlock,

	// Palette access functions
	NULL,
	NULL,

	// Palette access functions
	NULL,
	NULL,

	D3D9_THandle_GetInfo,

	// Scene management functions
	D3D9Drv_BeginScene,
	D3D9Drv_EndScene,

	D3D9Drv_BeginBatch,
	D3D9Drv_EndBatch,

	// Render functions
	D3D9Drv_RenderGouraudPoly,
	D3D9Drv_RenderWorldPoly,
	D3D9Drv_RenderMiscTexturePoly,

	//Decal functions
	D3D9Drv_DrawDecal,

	0,
	0,
	0,

	NULL,

	D3D9Drv_Screenshot,

	D3D9Drv_SetGamma,
	D3D9Drv_GetGamma,

	// BEGIN - Hardware T&L - paradoxnj 4/5/2005
	D3D9Drv_SetMatrix,
	D3D9Drv_GetMatrix,
	NULL,
	// END - Hardware T&L - paradoxnj 4/5/2005

	NULL,
	NULL,

	D3D9Drv_DrawText,

	NULL,

	D3D9Drv_CreateStaticMesh,
	D3D9Drv_RemoveStaticMesh,
	D3D9Drv_RenderStaticMesh,

	D3D9Drv_CreateFont,
	D3D9Drv_DrawFont,
	D3D9Drv_DestroyFont,

	D3D9Drv_SetRenderState,

	// Vertex Buffer
//	NULL,
//	NULL,
//	NULL,
//	NULL
};

extern "C" DRIVERAPI BOOL DriverHook(DRV_Driver** Driver)
{
	if (LOG_LEVEL > 1)
		LOG("Function Call:  DriverHook()");

	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
	{
		LOG("ERROR:  Could not create Direct3D object!!");
		return FALSE;
	}

	D3D9_THandle_Startup();

	EngineSettings.CanSupportFlags = (DRV_SUPPORT_ALPHA | DRV_SUPPORT_COLORKEY);
	EngineSettings.PreferenceFlags = 0;

	g_D3D9Drv.EngineSettings = &EngineSettings;

	*Driver = &g_D3D9Drv;
	return TRUE;
}

//====================================================================================
//	PrepPolyVerts
//====================================================================================
static jeBoolean PrepPolyVerts(jeTLVertex* Pnts, int32 NumPoints, jeRDriver_Layer* Layers, int32 NumLayers, void* LMapCBContext)
{
	float			InvScale{}, u{}, v{};
	float			ShiftU{}, ShiftV{}, ScaleU{}, ScaleV{};
	jeTLVertex* TempVerts{}, * pTVerts{}, * pVerts{};
	//float			du, dv, dx, dy, MipScale;
	int32			j{};
	//	D3DSURFACE_DESC				desc;

	float				InvScale2{}, ShiftU2{}, ShiftV2{};
	jeRDriver_Layer* LMapLayer{};
	jeRDriver_Layer* TexLayer{};

	TempVerts = (jeTLVertex*)malloc(sizeof(jeTLVertex) * NumPoints);
	memcpy(TempVerts, Pnts, sizeof(jeTLVertex) * NumPoints);

	pTVerts = &TempVerts[0];

	// Set up shifts and scaled for texture uv's
	TexLayer = &Layers[0];
	LMapLayer = &Layers[1];

	// Normalize UV's using Texture Size
	InvScale = 1.0f / (float)((1 << TexLayer->THandle->Log));

	ShiftU = TexLayer->ShiftU;
	ShiftV = TexLayer->ShiftV;
	ScaleU = (1.0f / (TexLayer->ScaleU));
	ScaleV = (1.0f / (TexLayer->ScaleV));

	if (NumLayers > 1)
	{
		// Set up shifts and scaled for lightmap uv's
		ShiftU2 = (float)-LMapLayer->ShiftU + 8.0f;
		ShiftV2 = (float)-LMapLayer->ShiftV + 8.0f;

		InvScale2 = 1.0f / (float)((1 << LMapLayer->THandle->Log) << 4);
	}

	pVerts = &Pnts[0];

	for (j = 0; j < NumPoints; j++)
	{

		u = pTVerts->u * ScaleU + ShiftU;
		v = pTVerts->v * ScaleV + ShiftV;

		pVerts->u = u * InvScale;
		pVerts->v = v * InvScale;

		if (NumLayers > 1)
		{
			u = pTVerts->u + ShiftU2;
			v = pTVerts->v + ShiftV2;

			pVerts->pad1 = u * InvScale2;
			pVerts->pad2 = v * InvScale2;
		}


		//	pVerts->color = pTVerts->Color;

		pTVerts++;
		pVerts++;
	}

	free(TempVerts);
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

	for (i = P2; i > 0; i >>= 1)
		p++;

	return (p - 1);
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

//=====================================================================================
//	FillLMapSurface
//=====================================================================================
static void FillLMapSurface(jeTLVertex* Pnts, int32 NumPoints, jeTexture* THandle, jeRDriver_LMapCBInfo* Info, int32 LNum)
{
	U16* pTempBits{}, * pTempBits2{};
	int32				w{}, h{}, Width{}, Height{}, Size{};
	U8* pBitPtr{};
	RGB_LUT* Lut{};
	int32				Extra{};
	//U8					PrevR,PrevG,PrevB;

	pBitPtr = (U8*)Info->RGBLight[LNum];

	Width = THandle->Width;
	Height = THandle->Height;
	Size = THandle->Log;

	Lut = &Lut1;

	D3D9_THandle_Lock(THandle, 0, (void**)& pTempBits);
	pTempBits2 = pTempBits;

	Extra = (THandle->stride - THandle->Width);

	for (h = 0; h < Height; h++)
	{
		for (w = 0; w < Width; w++)
		{
			U8	R{}, G{}, B{};
			U16	Color{};

			R = *pBitPtr++;
			G = *pBitPtr++;
			B = *pBitPtr++;

			Color = (U16)((Lut->R[R] | Lut->G[G] | Lut->B[B]) & 0xFFFF);

			*((U16*)pTempBits) = Color;
			((U16*)pTempBits)++;
		}
		pTempBits += Extra;
	}
	D3D9_THandle_Unlock(THandle, 0);


}

void BuildRGBGammaTables(float Gamma)
{
	int32				i{}, Val{};
	int32				GammaTable[256];
	DWORD			R_Left{}, G_Left{}, B_Left{}; //, A_Left;
	DWORD			R_Right{}, G_Right{}, B_Right{}; //, A_Right;

	if (Gamma == 1.0)
	{
		{
			for (i = 0; i < 256; i++)
				GammaTable[i] = i;
		}
	}
	else for (i = 0; i < 256; i++)
	{
		float Ratio = (i + 0.5f) / 255.5f;

		float RGB = (float)(255.0 * pow((double)Ratio, 1.0 / (double)Gamma) + 0.5);

		if (RGB < 0.0f)
			RGB = 0.0f;
		if (RGB > 255.0f)
			RGB = 255.0f;

		GammaTable[i] = (int32)RGB;
	}


	for (i = 0; i < 256; i++)
	{
		Val = GammaTable[i];

		R_Left = 11;
		G_Left = 5;
		B_Left = 0;
		//A_Left = PixelMask.A_Shift;

		R_Right = 3;
		G_Right = 2;
		B_Right = 3;
		//A_Right = 8 - PixelMask.A_Width;

		Val = GammaTable[i];

		Lut1.R[i] = (((uint32)Val >> R_Right) << R_Left) & 0xF800;
		Lut1.G[i] = (((uint32)Val >> G_Right) << G_Left) & 0x7E0;
		Lut1.B[i] = (((uint32)Val >> B_Right) << B_Left) & 0x1F;
		//D3DInfo.Lut1.A[i] = (((uint32)  i >> A_Right) << A_Left) & 0x00;

		//Lut1.R[i] = (i<<11) & 0xF800;
		//Lut1.G[i] = (i<<5) &	0x7E0;
		//Lut1.B[i] = (i) &		0x1F;
	}
}
