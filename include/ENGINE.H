/*!
	@file Engine.h 
	
	@author Charles Bloom/John Pollard
	@brief Maintains the driver interface, as well as the bitmaps attached to the driver.

	@par Licence
	The contents of this file are subject to the Jet3D Public License       
	Version 1.02 (the "License"); you may not use this file except in         
	compliance with the License. You may obtain a copy of the License at       
	http://www.jet3d.com                                                        
                                                                             
	@par
	Software distributed under the License is distributed on an "AS IS"           
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See           
	the License for the specific language governing rights and limitations          
	under the License.                                                               
                                                                                  
	@par
	The Original Code is Jet3D, released December 12, 1999.                            
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
*/

#ifndef JE_ENGINE_H
#define JE_ENGINE_H

#include "BaseType.h"
#include "jeTypes.h"
#include "Bitmap.h"
#include "Camera.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------

/*! @typedef jeEngine
	@brief A reference to the Engine
*/
typedef struct jeEngine			jeEngine;

/*! @typedef jeDriver
	@brief A reference to a Renderer device
*/
typedef struct jeDriver			jeDriver;

/*! @typedef jeDriverMode
	@brief A reference to a Renderer mode
*/
typedef struct jeDriver_Mode	jeDriver_Mode;

/*! @typedef jeDriverSystem
	@brief A reference to a Renderer enumerator
*/
typedef struct jeDriver_System	jeDriver_System;


/*! @typedef jeTexture
	@brief A reference to a material texture
*/
typedef struct jeTexture		jeTexture;

/*!	@typedef jeShader
	@brief A reference to a shader
*/
typedef struct jeShader			jeShader;

/*! @typedef jeVertexBuffer
	@brief A reference to a vertex buffer
*/
typedef struct jeVertexBuffer	jeVertexBuffer;

/*! @typedef jeIndexBuffer
	@brief A reference to an index buffer
*/
typedef struct jeIndexBuffer	jeIndexBuffer;

/*!	@typedef jeFont
	@brief A reference to a true type font
*/
typedef struct jeFont			jeFont;

#define JE_FONT_NORMAL			0x00000001
#define JE_FONT_BOLD			0x00000002

typedef enum
{
	JE_ENGINE_BITMAP_TYPE_2D,
	JE_ENGINE_BITMAP_TYPE_3D,
} jeEngine_BitmapType;


typedef enum
{
	FrameState_None = 0,
	FrameState_Begin,
} jeEngine_FrameState;


//-------------------------------------------------
// Engine Functions
//-------------------------------------------------

// ------- Create/Destroy
#ifdef WINVER
		// include windows to get this API 
JETAPI jeEngine* JETCC jeEngine_Create(HWND hWnd, const char *AppName, const char *DriverDirectory);
#endif
	
JETAPI jeBoolean	JETCC jeEngine_CreateRef(jeEngine *Engine);

JETAPI void			JETCC jeEngine_Free(jeEngine *Engine);			// Don't use this :
JETAPI void			JETCC jeEngine_Destroy(jeEngine **pEngine);

JETAPI jeBoolean	JETCC jeEngine_IsValid(const jeEngine *E);

// ------- Misc functions
JETAPI void			JETCC jeEngine_EnableFrameRateCounter(jeEngine *Engine, jeBoolean Enabled);

JETAPI jeBoolean	JETCC jeEngine_Activate(jeEngine *Engine, jeBoolean bActive);
JETAPI jeBoolean	JETCC jeEngine_UpdateWindow(jeEngine *Engine);

JETAPI jeBoolean	JETCC jeEngine_GetFrameState(const jeEngine *Engine, jeEngine_FrameState *FrameState);

//-------- The main Frame functions:

JETAPI jeBoolean	JETCC jeEngine_BeginFrame(jeEngine *Engine, jeCamera *Camera, jeBoolean ClearScreen);
JETAPI jeBoolean	JETCC jeEngine_EndFrame(jeEngine *Engine);

JETAPI jeBoolean	JETCC jeEngine_FlushScene(jeEngine *Engine);

// call updategamma when drivers change
JETAPI jeBoolean	JETCC jeEngine_SetGamma(jeEngine *Engine, float Gamma);
JETAPI jeBoolean	JETCC jeEngine_GetGamma(jeEngine *Engine, float *pGamma);
JETAPI void			JETCC jeEngine_UpdateGamma(jeEngine *Engine);

JETAPI jeBoolean	JETCC jeEngine_AddBitmap(		jeEngine *Engine, jeBitmap *Bitmap, jeEngine_BitmapType Type);
JETAPI jeBoolean	JETCC jeEngine_RemoveBitmap(	jeEngine *Engine, jeBitmap *Bitmap);

JETAPI jeBoolean	JETCC jeEngine_ScreenShot(jeEngine *Engine, const char *FileName);

jeBoolean				jeEngine_BitmapListInit(jeEngine *Engine);
jeBoolean				jeEngine_BitmapListShutdown(jeEngine *Engine);
jeBoolean				jeEngine_AttachAll(jeEngine *Engine);
jeBoolean				jeEngine_DetachAll(jeEngine *Engine);

/*	// CB disabled in 2.0
//-------- the splash screen
jeBoolean				jeEngine_DoSplashScreen(jeEngine *Engine, jeDriver_Mode *DriverMode);
*/

//-------- engine fonts
jeBoolean				jeEngine_InitFonts(jeEngine *Engine);
jeBoolean				jeEngine_ShutdownFonts(jeEngine *Engine);
// BEGIN - Hardware True Type Fonts - paradoxnj 8/3/2005
JETAPI jeFont *		JETCC jeEngine_CreateFont(jeEngine *Engine, int32 Height, int32 Width, uint32 Weight, jeBoolean Italic, const char *facename);
JETAPI jeBoolean	JETCC jeEngine_DestroyFont(jeEngine *Engine, jeFont **Font);

JETAPI jeBoolean	JETCC jeEngine_Printf(jeEngine *Engine, jeFont *Font, int32 x, int32 y, uint32 Color, const char *String, ...);
jeBoolean			jeEngine_DebugPrintf(jeEngine *Engine, uint32 Color, const char *String, ...);
// END - Hardware True Type Fonts - paradoxnj 8/3/2005

// ------- Built-in drivers (parameters to jeEngine_RegisterDriver)
// BEGIN - Software Driver no longer exists - paradoxnj 8/3/2005
//JETAPI	void*    JETCC jeEngine_SoftwareDriver(void);
// END - Software Driver no longer exists - paradoxnj 8/3/2005

JETAPI	void*    JETCC jeEngine_D3DDriver(void);

//-------- Engine Driver system stuff (for getting a driver to set the mode)
JETAPI jeDriver_System * JETCC jeEngine_GetDriverSystem(jeEngine *Engine);

JETAPI jeDriver		* JETCC jeDriver_SystemGetNextDriver(jeDriver_System *DriverSystem, jeDriver *Start);
JETAPI jeDriver_Mode * JETCC jeDriver_GetNextMode(jeDriver *Driver, jeDriver_Mode *Start);

JETAPI jeBoolean	JETCC jeDriver_GetName(const jeDriver *Driver, const char **Name);
JETAPI jeBoolean	JETCC jeDriver_ModeGetName(const jeDriver_Mode *Mode, const char **Name);
JETAPI jeBoolean	JETCC jeDriver_ModeGetWidthHeight(const jeDriver_Mode *Mode, int32 *pWidth, int32 *pHeight);
JETAPI jeBoolean	JETCC jeDriver_ModeGetAttributes(const jeDriver_Mode *Mode, int32 *pWidth, int32 *pHeight, int32 *pBpp);
							// CB : GetWidthHeight is deprecated ; use GetAttributes instead!

JETAPI jeBoolean 	JETCC jeEngine_RegisterDriver(jeEngine *Engine, void* HookProc);

JETAPI jeBoolean	JETCC jeEngine_GetDeviceCaps(jeEngine *pEngine, jeDeviceCaps *DeviceCaps);
JETAPI jeBoolean	JETCC jeEngine_GetDefaultRenderFlags(jeEngine *pEngine, uint32 *RenderFlags);
JETAPI jeBoolean	JETCC jeEngine_SetDefaultRenderFlags(jeEngine *pEngine, uint32 RenderFlags);

#ifdef WINVER
JETAPI jeBoolean	JETCC jeEngine_SetDriverAndMode(	jeEngine		*Engine, 
													HWND			hWnd,
													jeDriver		*Driver, 
													jeDriver_Mode	*DriverMode);
#endif
													
JETAPI jeBoolean	JETCC jeEngine_GetDriverAndMode(	const jeEngine *Engine, 
													jeDriver **Driver, 
													jeDriver_Mode **DriverMode);

JETAPI jeBoolean	JETCC jeEngine_ShutdownDriver(jeEngine *Engine);

jeBoolean				jeEngine_ResetDriver(jeEngine *Engine);	// Resets the current driver


//--------- Render driver stuff (this is the main hard-core driver.  the engine just wraps this up with jeDriver)
//	NOTE - You MUST include dcommon.h BEFORE engine.h to use the hard-core driver functions!
#ifdef DCOMMON_H
	typedef struct jeEngine_ChangeDriverCB		jeEngine_ChangeDriverCB;

	typedef jeBoolean JETCC jeEngine_ShutdownDriverCB(DRV_Driver *Driver, void *Conext);
	typedef jeBoolean JETCC jeEngine_StartupDriverCB(DRV_Driver *Driver, void *Conext);

	// Driver will make more sense in the future (instead of RDriver)
	DRV_Driver* JETCF jeEngine_GetDriver(const jeEngine * Engine);
	
	JETAPI jeEngine_ChangeDriverCB * JETCC jeEngine_CreateChangeDriverCB(	jeEngine					*Engine, 
																		jeEngine_ShutdownDriverCB	*ShutdownDriverCB, 
																		jeEngine_StartupDriverCB	*StartupDriverCB,
																		void						*Context);
	JETAPI void JETCC jeEngine_DestroyChangeDriverCB(jeEngine *Engine, jeEngine_ChangeDriverCB **ChangeDriverCB);
#endif

#ifdef WINVER
HINSTANCE				jeEngine_LoadLibrary( const char * lpLibFileName, const char *DriverDirectory);
#endif

//-------- drawing with the engine (Decals & Misc Polys)

JETAPI jeBoolean JETCC jeEngine_DrawBitmap(const jeEngine *Engine,
	const jeBitmap *Bitmap,	// should have been a TYPE_2D
	const jeRect * Source, uint32 x, uint32 y);

JETAPI jeBoolean JETCC jeEngine_DrawBitmap3D(const jeEngine *Engine,
	const jeBitmap *Bitmap, // should have been a TYPE_3D
	const jeRect * pRect, uint32 x, uint32 y);

// BEGIN - jeTexture implementation - paradoxnj 5/12/2005
JETAPI jeTexture *JETCC jeEngine_CreateTextureFromFile(const jeEngine *Engine, jeVFile *File);
JETAPI void JETCC jeEngine_DestroyTexture(const jeEngine *Engine, jeTexture *Texture);

JETAPI jeBoolean JETCC jeEngine_DrawTexture(const jeEngine *Engine, const jeTexture *Texture, int32 x, int32 y);
// END - jeTexture implementation - paradoxnj 5/12/2005

// foward definition of jeMaterialSpec
typedef struct jeMaterialSpec jeMaterialSpec;

JETAPI void JETCC jeEngine_RenderPoly(const jeEngine *Engine, const JE_TLVertex *Points, 
						int NumPoints, const jeMaterialSpec *Texture, uint32 Flags);
						// Texture should have been a TYPE_3D

JETAPI void JETCC jeEngine_RenderPolyArray(const jeEngine *Engine, const JE_TLVertex ** pPoints, int * pNumPoints, int NumPolys, 
								const jeMaterialSpec *Texture, uint32 Flags);

JETAPI void	JETCC jeEngine_FillRect(jeEngine *Engine, const jeRect *Rect, const jeRGBA *Color);

//--------------------------------------------------------------------------

#ifdef WINVER
// Registers an Object given a handle to a DLL - Incarnadine
JETAPI jeBoolean JETCC jeEngine_RegisterObject(HINSTANCE DllHandle);

// Registers all Objects in a particular path. - Incarnadine
JETAPI jeBoolean JETCC jeEngine_RegisterObjects(char * DllPath);
#endif

// Krouer
// debug BSP info
JETAPI jeBoolean JETCC jeEngine_GetBSPDebugInfo(jeEngine *Engine, int32 *NumMakeFaces, int32 *NumMergedFaces, int32 *NumSubdividedFaces, int32 *NumDrawFaces);
// toggle wireframe rendering for gameshell/BSP object
JETAPI jeBoolean JETCC jeEngine_SetRenderMode(jeEngine *Engine, int32 RenderMode);

// Cool function from Darkrift
JETAPI jeFloat JETCC jeEngine_GetFPS(jeEngine *Engine);
JETAPI jeBoolean JETCC jeEngine_DrawText(jeEngine *Engine, char *text, int x,int y,uint32 color);
JETAPI jeBoolean JETCC jeEngine_SetFog(jeEngine *Engine, float r, float g, float b, float start, float endi, jeBoolean enable);

// BEGIN - Hardware T&L - paradoxnj 6/8/2005
JETAPI jeBoolean JETCC jeEngine_SetMatrix(jeEngine *Engine, uint32 Type, jeXForm3d *Matrix);
JETAPI jeBoolean JETCC jeEngine_GetMatrix(jeEngine *Engine, uint32 Type, jeXForm3d *Matrix);
JETAPI jeBoolean JETCC jeEngine_SetCamera(jeEngine *Engine, jeCamera *Camera);
// END - Hardware T&L - paradoxnj 6/8/2005

#ifdef __cplusplus
}
#endif

#endif // JE_ENGINE_H
