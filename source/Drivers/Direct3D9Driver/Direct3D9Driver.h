#ifndef DIRECT3D9DRIVER_H
#define DIRECT3D9DRIVER_H

#define DRIVERAPI	_declspec(dllexport)

#define INITGUID

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "DCommon.h"
#include "jeVertexBuffer.h"

#define LOG_LEVEL								1

#ifdef _DEBUG
#define REPORT(x)							OutputDebugString(x)
#else
#define REPORT(x)
#endif

extern HWND									hWnd;
extern IDirect3D9								*pD3D;
extern IDirect3DDevice9						*pDevice;
extern float									localgamma;
extern D3DCAPS9								g_Caps;
extern bool									CanDoAntiAlias;

#define MAX_LAYERS							2
#define JE_HW_FVF							( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 )

#define SAFE_DELETE(x)						{ if (x) delete x; x = NULL; }
#define SAFE_DELETE_ARRAY(x)				{ if (x) delete [] x; x = NULL; }
#define SAFE_RELEASE(x)						{ if (x) (x)->Release(); x = NULL; }

jeVertexCache	*jeVertexCache_Create(const void* lpD3D,const DWORD& FVF,
						const unsigned int& VertexCount, unsigned int Size, JVB_BUFFER_TYPE bufType);

void BuildRGBGammaTables(float Gamma);

typedef DRV_Driver				D3D9Driver;

void									D3DMatrix_ToXForm3d(D3DMATRIX *mat, jeXForm3d *XForm);
void									jeXForm3d_ToD3DMatrix(jeXForm3d *XForm, D3DMATRIX *mat);

jeBoolean								DRIVERCC D3D9Drv_EnumSubDrivers(DRV_ENUM_DRV_CB *Cb, void *Context);
jeBoolean								DRIVERCC D3D9Drv_EnumModes(S32 Driver, char *DriverName, DRV_ENUM_MODES_CB *Cb, void *Context);

jeBoolean								DRIVERCC D3D9Drv_EnumPixelFormats(DRV_ENUM_PFORMAT_CB *Cb, void *Context);

jeBoolean								DRIVERCC D3D9Drv_Init(DRV_DriverHook *hook);
jeBoolean								DRIVERCC D3D9Drv_Shutdown();
jeBoolean								DRIVERCC D3D9Drv_Reset();
jeBoolean								DRIVERCC D3D9Drv_UpdateWindow();
jeBoolean								DRIVERCC D3D9Drv_SetActive(jeBoolean Active);

jeBoolean								DRIVERCC D3D9Drv_BeginScene(jeBoolean Clear, jeBoolean ClearZ, RECT *WorldRect, jeBoolean Wireframe);
jeBoolean								DRIVERCC D3D9Drv_EndScene(void);
jeBoolean								DRIVERCC D3D9Drv_BeginBatch(void);
jeBoolean								DRIVERCC D3D9Drv_EndBatch(void);

jeBoolean								DRIVERCC D3D9Drv_RenderGouraudPoly(jeTLVertex *Pnts, int32 NumPoints, uint32 Flags);
jeBoolean								DRIVERCC D3D9Drv_RenderWorldPoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags);
jeBoolean								DRIVERCC D3D9Drv_RenderMiscTexturePoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags);

jeBoolean								DRIVERCC D3D9Drv_DrawDecal(jeTexture *Handle, RECT *SrcRect, int32 x, int32 y);

jeBoolean								DRIVERCC D3D9Drv_Screenshot(const char *filename);

jeBoolean								DRIVERCC D3D9Drv_GetGamma(float *gamma);
jeBoolean								DRIVERCC D3D9Drv_SetGamma(float gamma);

jeBoolean								DRIVERCC D3D9Drv_SetMatrix(uint32 Type, jeXForm3d *Matrix);
jeBoolean								DRIVERCC D3D9Drv_GetMatrix(uint32 Type, jeXForm3d *Matrix);

uint32									DRIVERCC D3D9Drv_CreateStaticMesh(jeHWVertex *Points, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags);
jeBoolean								DRIVERCC D3D9Drv_RemoveStaticMesh(uint32 id);
jeBoolean								DRIVERCC D3D9Drv_RenderStaticMesh(uint32 id, int32 StartVertex, int32 NumPolys, jeXForm3d *XForm);

jeFont *								DRIVERCC D3D9Drv_CreateFont(int32 Height, int32 Width, uint32 Weight, jeBoolean Italic, const char *facename);
jeBoolean								DRIVERCC D3D9Drv_DrawFont(jeFont *Font, int32 x, int32 y, uint32 Color, const char *text);
jeBoolean								DRIVERCC D3D9Drv_DestroyFont(jeFont **Font);

jeBoolean								DRIVERCC D3D9Drv_SetRenderState(uint32 state, uint32 value);
jeBoolean								DRIVERCC D3D9Drv_DrawText(char *text, int x, int y, uint32 color);

extern "C" DRIVERAPI D3D9Driver			g_D3D9Drv;

int32 GetLog(int32 Width, int32 Height);

typedef struct RGB_LUT
{
	uint32				R[256];
	uint32				G[256];
	uint32				B[256];
	uint32				A[256];
} RGB_LUT;

extern RGB_LUT Lut1;

#endif //DIRECT3D9DRIVER_H
