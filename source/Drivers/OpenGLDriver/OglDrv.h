/*!
	@file OglDrv.h
	@author Anthony Rufrano (paradoxnj)
	@brief OpenGL Driver Main Header
*/
#ifndef OGLDRV_H
#define OGLDRV_H

#include <windows.h>
#include <gl/gl.h>

#include "glext.h"
#include "wglext.h"

#include "DCommon.h"

#define DRIVERAPI				extern "C" _declspec(dllexport)

extern HWND						hWnd;
extern HDC						hDC;
extern HGLRC					hglRC;

extern bool						Has_Stencil;
extern bool						Has_MultiTexture;

extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB;

extern DRV_Window				ClientWindow;
extern DRV_Driver				OGLDRV;

// Private Driver Functions
void							BuildRGBGammaTables(float Gamma);

// Public Driver Functions
jeBoolean						DRIVERCC OGLDrv_EnumSubDrivers(DRV_ENUM_DRV_CB *Cb, void *Context);
jeBoolean						DRIVERCC OGLDrv_EnumModes(S32 Driver, char *DriverName, DRV_ENUM_MODES_CB *Cb, void *Context);
jeBoolean						DRIVERCC OGLDrv_EnumPixelFormats(DRV_ENUM_PFORMAT_CB *Cb, void *Context);

jeBoolean						DRIVERCC OGLDrv_Init(DRV_DriverHook *Hook);
jeBoolean						DRIVERCC OGLDrv_Shutdown();
jeBoolean						DRIVERCC OGLDrv_UpdateWindow();
jeBoolean						DRIVERCC OGLDrv_SetActive(jeBoolean Active);

jeBoolean						DRIVERCC OGLDrv_BeginScene(jeBoolean Clear, jeBoolean ClearZ, RECT *WorldRect, jeBoolean WireFrame);
jeBoolean						DRIVERCC OGLDrv_EndScene();
jeBoolean						DRIVERCC OGLDrv_BeginBatch();
jeBoolean						DRIVERCC OGLDrv_EndBatch();

jeBoolean						DRIVERCC OGLDrv_RenderGouraudPoly(jeTLVertex *Pnts, int32 NumPoints, uint32 Flags);
jeBoolean						DRIVERCC OGLDrv_RenderWorldPoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags);
jeBoolean						DRIVERCC OGLDrv_RenderMiscTexturePoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags);

jeBoolean						DRIVERCC OGLDrv_DrawDecal(jeTexture *THandle, RECT *SrcRect, int32 x, int32 y);
jeBoolean						DRIVERCC OGLDrv_Screenshot(const char *filename);

jeBoolean						DRIVERCC OGLDrv_GetGamma(float *Gamma);
jeBoolean						DRIVERCC OGLDrv_SetGamma(float Gamma);

jeBoolean						DRIVERCC OGLDrv_SetMatrix(uint32 Type, jeXForm3d *Matrix);
jeBoolean						DRIVERCC OGLDrv_GetMatrix(uint32 Type, jeXForm3d *Matrix);

jeBoolean						DRIVERCC OGLDrv_SetCamera(jeCamera *Camera);
jeBoolean						DRIVERCC OGLDrv_SetRenderState(uint32 state, uint32 value);

typedef struct RGB_LUT
{
	uint32						r[256];
	uint32						g[256];
	uint32						b[256];
	uint32						a[256];
} RGB_LUT;

extern RGB_LUT					Lut1;

#endif


