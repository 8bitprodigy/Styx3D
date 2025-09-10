/****************************************************************************************/
/*  Engine.c                                                                            */
/*                                                                                      */
/*  Author: Charles Bloom/John Pollard                                                  */
/*  Description: Maintains the driver interface, as well as the bitmaps attached		*/
/*					to the driver.														*/
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

#ifdef WIN32
#include <windows.h>
#pragma warning (disable:4201)
#include <mmsystem.h> //timeGetTime
#pragma warning (default:4201)
#endif // WIN32

#ifdef BUILD_BE
#include <OS.h>
#include <image.h>
#include <Debug.h>
#include <stdio.h>
#define OutputDebugString(a) printf(a) //DEBUGGER
#define FreeLibrary unload_add_on
#define HINSTANCE image_id

#define Sleep(a) snooze(a * 1000)
#endif

#include <string.h>
#include <stdlib.h> // _MAX_PATH

#include "Dcommon.h"
#include "Engine.h"
#include "Engine._h"

#include "Errorlog.h"
#include "Dcommon.h"
#include "BitmapList.h"
#include "Bitmap.h"
#include "Bitmap._h"
#include "Log.h"
#include "List.h"
#include "jeAssert.h"
#include "Cpu.h"
#include "VFile.h"
#include "jeChain.h"
#include "Ram.h"
#include "jeVersion.h" // Incarnadine

#include "jeBSP.h"

#ifdef _DEBUG
	#define DEBUG_OUTPUT_LEVEL		0
	//#define DEBUG_OUTPUT_LEVEL		2
#else
	#define DEBUG_OUTPUT_LEVEL		0
#endif

//=====================================================================================
//=====================================================================================
typedef jeBoolean JETCC jeEngine_ShutdownDriverCB(DRV_Driver *Driver, void *Conext);
typedef jeBoolean JETCC jeEngine_StartupDriverCB(DRV_Driver *Driver, void *Conext);

typedef struct jeEngine_ChangeDriverCB
{
	jeEngine_ShutdownDriverCB		*ShutdownDriverCB;
	jeEngine_StartupDriverCB		*StartupDriverCB;
	void							*Context;
} jeEngine_ChangeDriverCB;

//=====================================================================================
//=====================================================================================
static jeBoolean Engine_EnumSubDrivers(Engine_DriverInfo *DriverInfo, const char *DriverDirectory);
static jeBoolean Engine_EnumSubDriversCB(int32 DriverId, char *Name, void *Context);
static jeBoolean Engine_EnumModesCB(int32 ModeId, char *Name, int32 Width, int32 Height, int32 Bpp, void *Context);

static jeBoolean Engine_InitDriver(	jeEngine		*Engine, 
									HWND			hWnd,
									jeDriver		*Driver,
									jeDriver_Mode	*DriverMode);

static void Engine_DrawFontBuffer(jeEngine *Engine);
static void Engine_Tick(jeEngine *Engine);

static void SubLarge(LARGE_INTEGER *start, LARGE_INTEGER *end, LARGE_INTEGER *delta);

#define ABS(xx)	( (xx) < 0 ? (-(xx)) : (xx) )


// extern data for stats purpose - Krouer
extern int32 NumMakeFaces;
extern int32 NumMergedFaces;
extern int32 NumSubdividedFaces;

//=====================================================================================
// ------- Create/Destroy
//=====================================================================================

//=====================================================================================
//	jeEngine_Create
//=====================================================================================
JETAPI jeEngine * JETCC jeEngine_Create(HWND hWnd, const char *AppName, const char *DriverDirectory)
{
	jeEngine* Engine{};
	int32		i{}, Length{};

	assert(AppName);
	assert(hWnd);

	// Attempt to create a new engine object
	Engine = (jeEngine *)jeRam_AllocateClear(sizeof(jeEngine));

	if (!Engine)
	{
		jeErrorLog_Add(JE_ERR_OUT_OF_MEMORY, NULL);
		goto ExitWithError;
	}

	Engine->RefCount = 1;

	Engine->MySelf1 = Engine;
	Engine->MySelf2 = Engine;
	
	if (!List_Start())
	{
		jeErrorLog_Add(JE_ERR_OUT_OF_MEMORY, NULL);
		goto ExitWithError;
	}	

	if	(DriverDirectory)
	{
		Length = strlen(DriverDirectory) + 1;
		Engine->DriverDirectory = (char *)jeRam_Allocate(Length);

		if (!Engine->DriverDirectory)
			goto ExitWithError;

		memcpy(Engine->DriverDirectory, DriverDirectory, Length);
	}
	
	Engine->hWnd = hWnd;
	strcpy(Engine->AppName, AppName);
	
	// Build the wavetable
	for (i = 0; i < 20; i++)
		Engine->WaveTable[i] = (int16)(((i * 65)%200) + 50);

	// Be flexible if they didn't want us to load any driver DLLs
	if	(DriverDirectory)
	{
		if (! Engine_EnumSubDrivers(&Engine->DriverInfo, DriverDirectory))
			goto ExitWithError;
	}

	if (!jeEngine_BitmapListInit(Engine))
		goto ExitWithError;

	if (!jeEngine_InitFonts(Engine))				// Must be after BitmapList
		goto ExitWithError;

	Engine->DisplayFrameRateCounter = JE_TRUE;	// Default to showing the FPS counter

#if 0
	// @@ impolite !!
	jeAssert_SetCriticalShutdownCallback( (jeAssert_CriticalShutdownCallback)jeEngine_ShutdownDriver , (uint32)Engine,
											NULL, NULL);
#endif	

	Engine->CurrentGamma = 3.0f;

	if (!jeCPU_GetInfo() )
		goto ExitWithError;

	Engine->ChangeDriverCBChain = jeChain_Create();

	if (!Engine->ChangeDriverCBChain)
		goto ExitWithError;

	return Engine;
	
	// Error cleanup:
	ExitWithError:
	{
		// BEGIN - FIX - Engine not cleaning up everything on error - paradoxnj
		//#pragma message ("Engine is not destroying everything on **error** here!")
		// END - FIX - Engine not cleaning up everything on error - paradoxnj
		if (Engine)
		{
			if (Engine->DriverDirectory)
				jeRam_Free(Engine->DriverDirectory);

			// BEGIN - FIX - Engine not cleaning up everything on error - paradoxnj
			if (Engine->ChangeDriverCBChain != nullptr)
				jeChain_Destroy(&Engine->ChangeDriverCBChain);

//			jeEngine_ShutdownFonts(Engine);
			jeEngine_BitmapListShutdown(Engine);
			List_Stop();
			// END - FIX - Engine not cleaning up everything on error - paradoxnj

			jeRam_Free(Engine);
		}

		return nullptr;
	}
}

//=====================================================================================
//	jeEngine_CreateRef
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_CreateRef(jeEngine *Engine)
{
	assert(jeEngine_IsValid(Engine));

	Engine->RefCount++;

	return JE_TRUE;
}

//=====================================================================================
//	jeEngine_Free
//=====================================================================================

JETAPI void	JETCC jeEngine_Destroy(jeEngine **pEngine)
{
	assert( pEngine );
	jeEngine_Free(*pEngine);
	*pEngine = nullptr;
}

JETAPI void JETCC jeEngine_Free(jeEngine *Engine)
{
	jeBoolean		Ret{};

	assert(jeEngine_IsValid(Engine));
	assert( Engine->RefCount > 0);

	Engine->RefCount--;

	if (Engine->RefCount > 0)
		return;

	Ret = jeEngine_ShutdownFonts(Engine);
	assert(Ret == JE_TRUE);

	Ret = jeEngine_ShutdownDriver(Engine);
	assert(Ret == JE_TRUE);

	Ret = jeEngine_BitmapListShutdown(Engine);
	assert(Ret == JE_TRUE);

	if (Engine->DriverDirectory)
		jeRam_Free(Engine->DriverDirectory);

	if (Engine->ChangeDriverCBChain)
		jeChain_Destroy(&Engine->ChangeDriverCBChain);

	List_Stop();

	jeRam_Free(Engine);
}

//=====================================================================================
//	jeEngine_IsValid
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_IsValid(const jeEngine *E)
{
	if (!E) 
		return JE_FALSE;
	if (E->MySelf1 != E) 
		return JE_FALSE;
	if (E->MySelf2 != E) 
		return JE_FALSE;
	if (E->RefCount < 0)
		return JE_FALSE;
	//if (!IsWindowHandleValid(E->hWnd)) 
	if (!E->hWnd) 
		return JE_FALSE;

	return JE_TRUE;
}

//=====================================================================================
// ------- Misc functions
//=====================================================================================

//=====================================================================================
//	jeEngine_EnabledFrameRateCounter
//=====================================================================================
JETAPI void	JETCC jeEngine_EnableFrameRateCounter(jeEngine *Engine, jeBoolean Enabled)
{
	assert( jeEngine_IsValid(Engine) );
	Engine->DisplayFrameRateCounter = Enabled;
}

//=====================================================================================
//	jeEngine_Activate
//		this hits the drivers activation code to manage
//		surfaces and exclusive modes for devices (WM_ACTIVATEAPP)
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_Activate(jeEngine *Engine, jeBoolean bActive)
{
	DRV_Driver	*RDriver{};
	
	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_None);

	RDriver	=Engine->DriverInfo.RDriver;

	if( RDriver)
	{
		if ( RDriver->SetActive )
			return	RDriver->SetActive(bActive);
	}

	return	JE_TRUE;
}

static int32 UpdateWindowRecursion = 0;
//====================================================================================
//	jeEngine_UpdateWindow
//		this call updates the drivers with a new rect to blit to
//		(usually the result of a window move or resize)
//====================================================================================
JETAPI jeBoolean JETCC jeEngine_UpdateWindow(jeEngine *Engine)
{
	DRV_Driver	*RDriver{};
		
	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_None);

	assert(UpdateWindowRecursion == 0);

	UpdateWindowRecursion++;
	
	// Driver->UpdateWindow ONLY supports re-positioning, and NOT resizing... (As of 2/24/99)
	RDriver	= Engine->DriverInfo.RDriver;

	if( RDriver)
	{
	#if 0
		if (RDriver->UpdateWindow )
			return RDriver->UpdateWindow();
	#else
		jeDriver* Driver{};
		jeDriver_Mode* DriverMode{};

		if (!jeEngine_GetDriverAndMode(Engine, &Driver, &DriverMode))
			return JE_FALSE;

		if (!jeEngine_SetDriverAndMode(Engine, Engine->hWnd, Driver, DriverMode))
			return JE_FALSE;		
	#endif
	}

	UpdateWindowRecursion--;

	assert(UpdateWindowRecursion == 0);

	return JE_TRUE;
}

//===================================================================================
//	jeEngine_GetFrameState
//===================================================================================
JETAPI jeBoolean JETCC jeEngine_GetFrameState(const jeEngine *Engine, jeEngine_FrameState *FrameState)
{
	*FrameState = Engine->FrameState;

	return JE_TRUE;
}

//===================================================================================
//-------- The main Frame functions:
//===================================================================================

//=====================================================================================
//	jeEngine_Prep
//=====================================================================================
static	jeBoolean jeEngine_Prep(jeEngine *Engine)
{
	assert( jeEngine_IsValid(Engine) );

	return jeEngine_AttachAll(Engine);
}

//===================================================================================
//	jeEngine_BeginFrame
//===================================================================================
JETAPI jeBoolean JETCC jeEngine_BeginFrame(jeEngine *Engine, jeCamera *Camera, jeBoolean ClearScreen)
{
	RECT	DrvRect{}, *pDrvRect{};

#if (DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("BEGIN jeEngine_BeginFrame\n");
#endif

	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_None);
	
	// Make sure the driver is avtive
	if (!Engine->DriverInfo.RDriver)
	{
		jeErrorLog_Add(JE_ERR_DRIVER_NOT_INITIALIZED, NULL);
		return JE_FALSE;
	}
	
	assert(Engine->DriverInfo.RDriver != NULL);

	if (!jeEngine_Prep(Engine))
		return JE_FALSE;

	// Do some timing stuff
#ifdef WIN32
	QueryPerformanceCounter(&Engine->CurrentTic);
#endif

	// Clear some debug info
	memset(&Engine->DebugInfo, 0, sizeof(Engine->DebugInfo));

//	Engine->FontInfo.NumDebugStrings = 0;

	if(Camera)
	{
		jeRect			gDrvRect{};
		jeDriver_Mode* CurMode{};

		jeCamera_GetClippingRect(Camera, &gDrvRect);
	
		CurMode = Engine->DriverInfo.CurMode;

		if (CurMode->Width != -1 && CurMode->Height != -1)
		{
			if (gDrvRect.Left < 0)
			{
				jeErrorLog_AddString(-1, "Invalid Camera for FULLSCREEN", NULL);
				return JE_FALSE;
			}

			if (gDrvRect.Right >= CurMode->Width)
			{
				jeErrorLog_AddString(-1, "Invalid Camera for FULLSCREEN", NULL);
				return JE_FALSE;
			}

			if (gDrvRect.Top < 0)
			{
				jeErrorLog_AddString(-1, "Invalid Camera for FULLSCREEN", NULL);
				return JE_FALSE;
			}

			if (gDrvRect.Bottom >= CurMode->Height)
			{
				jeErrorLog_AddString(-1, "Invalid Camera for FULLSCREEN", NULL);
				return JE_FALSE;
			}
		}

		DrvRect.left	=gDrvRect.Left;
		DrvRect.top		=gDrvRect.Top;
		DrvRect.right	=gDrvRect.Right;
		DrvRect.bottom	=gDrvRect.Bottom;

		pDrvRect = &DrvRect;
	}
	else
		pDrvRect = nullptr;

	if (!Engine->DriverInfo.RDriver->BeginScene(ClearScreen, JE_TRUE, pDrvRect, (Engine->RenderMode==RenderMode_Lines)))
	{
		jeErrorLog_Add(JE_ERR_DRIVER_BEGIN_SCENE_FAILED, NULL);
		return JE_FALSE;
	}

	Engine->FrameState = FrameState_Begin;

#if (DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("END jeEngine_BeginFrame\n");
#endif

	return JE_TRUE;
}

#ifdef WIN32
//=====================================================================================
//	IsKeyDown
//=====================================================================================
static jeBoolean IsKeyDown(int KeyCode, HWND hWnd)
{
	//if (GetFocus() == hWnd)
		if (GetAsyncKeyState(KeyCode) & 0x8000)
			return JE_TRUE;

	return JE_FALSE;
}
#endif

//===================================================================================
//	jeEngine_GetFPS
//===================================================================================
JETAPI jeFloat JETCC jeEngine_GetFPS(jeEngine *Engine)
{
	return Engine->Fps;
}

//===================================================================================
//	jeEngine_EndFrame
//===================================================================================
JETAPI jeBoolean JETCC jeEngine_EndFrame(jeEngine *Engine)
{
	LARGE_INTEGER		NowTic{}, DeltaTic{};
	float				Fps{};
	//DRV_Debug			*Debug;

#if (DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("BEGIN jeEngine_EndFrame\n");
#endif

	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_Begin);

	if (!Engine->DriverInfo.RDriver)
	{
		jeErrorLog_Add(JE_ERR_DRIVER_NOT_INITIALIZED, NULL);
		return JE_FALSE;
	}
	
	assert(Engine->DriverInfo.RDriver != NULL);

	// Flush the scene before the text is drawn...
	jeEngine_FlushScene(Engine);

	// Draw the text
	Engine_DrawFontBuffer(Engine);

	Engine->FrameState = FrameState_None;

	if (!Engine->DriverInfo.RDriver->EndScene())
	{
		jeErrorLog_Add(JE_ERR_DRIVER_END_SCENE_FAILED, NULL);
		return JE_FALSE;
	}

	// Do some timing stuff
	QueryPerformanceCounter(&NowTic);	
	SubLarge(&Engine->CurrentTic, &NowTic, &DeltaTic);	
	
	if (DeltaTic.LowPart > 0)
		Fps =  (float)jeCPU_PerformanceFreq / (float)DeltaTic.LowPart;
	else 
		Fps = 100.0f;

	Engine->Fps = Fps;

	#define AVERAGE_FPS_HISTORY (30)	// about one second

	if (Engine->DisplayFrameRateCounter == JE_TRUE)			// Dieplay debug info
	{
	float AverageFps{};
	DRV_CacheInfo	*pCacheInfo{};
	static float		FpsArray[AVERAGE_FPS_HISTORY];
	static int32		NumFps = 0, i;

		// Changed Average Fps to go accross last n frames, JP...
		FpsArray[(NumFps++) % AVERAGE_FPS_HISTORY] = Fps;
		
		for (AverageFps = 0.0f, i=0; i<AVERAGE_FPS_HISTORY; i++)
			AverageFps += FpsArray[i];

		AverageFps *= (1.0f/(float)AVERAGE_FPS_HISTORY);

		jeEngine_DebugPrintf(Engine, JE_COLOR_XRGB(255, 255, 255), "Fps    : %2.2f / %2.2f", Fps, AverageFps);
		
		
		Engine->DebugInfo.RenderedPolys = Engine->DriverInfo.RDriver->NumRenderedPolys;

		jeEngine_DebugPrintf(Engine, JE_COLOR_XRGB(255, 255, 255), "Polys  : %4i/%4i/%4i", Engine->DebugInfo.TraversedPolys, Engine->DebugInfo.SentPolys, Engine->DebugInfo.RenderedPolys);

		jeEngine_DebugPrintf(Engine, JE_COLOR_XRGB(255, 255, 255), "Mirrors: %3i, DLights: %3i, Fog    : %3i", 
								Engine->DebugInfo.NumMirrors,Engine->DebugInfo.NumDLights,Engine->DebugInfo.NumFog);

		jeEngine_DebugPrintf(Engine, JE_COLOR_XRGB(255, 255, 255), "Actors : %3i, Models: %3i", Engine->DebugInfo.NumActors, Engine->DebugInfo.NumModels);
		jeEngine_DebugPrintf(Engine, JE_COLOR_XRGB(255, 255, 255), "LMap1  : %3i, LMap2  : %3i", Engine->DebugInfo.LMap1, Engine->DebugInfo.LMap2);
		
		jeEngine_DebugPrintf(Engine, JE_COLOR_XRGB(255, 255, 255), "BSP    : TF %d", NumMakeFaces);

		pCacheInfo = Engine->DriverInfo.RDriver->CacheInfo;

		if (pCacheInfo)
		{
			jeEngine_DebugPrintf(Engine, JE_COLOR_XRGB(255, 255, 255), "Cache : tex: %3i (%3ik), lmap: %3i (%3ik)",
										pCacheInfo->TexMisses , pCacheInfo->TexMissBytes >>10,
										pCacheInfo->LMapMisses,pCacheInfo->LMapMissBytes >>10);
										
			jeEngine_DebugPrintf(Engine, JE_COLOR_XRGB(255, 255, 255), "Cache : mem: %3ik/%3ik/%3ik bal=%d bias=%f",
										pCacheInfo->CardMem >>10,pCacheInfo->SlotMem >>10,pCacheInfo->UsedMem >>10,
										pCacheInfo->Balances,pCacheInfo->MipBias);

			{
			char str[1024];
			int i;
				strcpy(str,"Cache : [");

				for(i=0;i<7;i++)
				{
				char work[1024];
					sprintf(work,"%d,%d,%d|",
						pCacheInfo->CacheSlots[i],
						pCacheInfo->CacheUses[i],
						pCacheInfo->CacheMisses[i]);
					strcat(str,work);
				}

				jeEngine_DebugPrintf(Engine, JE_COLOR_XRGB(255, 255, 255), str);
			}
		}
	}

	// Do an engine frame
	Engine_Tick(Engine);

#if 0
	if (IsKeyDown(VK_F12, Engine->hWnd))
	{
		int32		i{};
		FILE* f{};
		char		Name[256];

		for (i=0 ;i<999; i++)		// Only 999 bmps, oh well...
		{
			sprintf(Name, "J3D%i.Bmp", i);

			f = fopen(Name, "rb");

			if (f)
			{
				fclose(f);
				continue;
			}
							
			jeEngine_ScreenShot(Engine, Name);
		}
	}
#endif

#if (DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("END jeEngine_EndFrame\n");
#endif

	return JE_TRUE;
}

//=====================================================================================
//	jeEngine_ScreenShot
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_ScreenShot(jeEngine *Engine, const char *FileName)
{
	assert( jeEngine_IsValid(Engine) );

	return Engine->DriverInfo.RDriver->ScreenShot(FileName);
}

//=====================================================================================
//	jeEngine_DrawDDText
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_DrawText(jeEngine *Engine, char *text, int x,int y,uint32 color)
{
	assert( jeEngine_IsValid(Engine) );
   
   if (Engine->DriverInfo.RDriver->DrawText) {
	   return Engine->DriverInfo.RDriver->DrawText(text,x,y,color);
   } else {
      return jeEngine_Printf(Engine, Engine->FontInfo.Font, x, y, color, text);
   }
}

//=====================================================================================
//	jeEngine_SetFog
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_SetFog(jeEngine *Engine, float r, float g, float b, float start, float endi, jeBoolean enable)
{
	//assert( jeEngine_IsValid(Engine) ); 
	//for some reason this prevents the code from working...
#pragma message ("Krouer: do not tested at 17th january 2005")

	//MessageBox(Engine->hWnd,"poks","poks",MB_OK);
	return Engine->DriverInfo.RDriver->SetFog(r,g,b,start,endi,enable);
}

//=====================================================================================
//	jeEngine_GetDriver
//=====================================================================================
DRV_Driver * JETCF jeEngine_GetDriver(const jeEngine * Engine)
{
	assert( jeEngine_IsValid(Engine) );
	
	return Engine->DriverInfo.RDriver;
}

//===================================================================================
//	jeEngine_CreateChangeDriverCB
//===================================================================================
JETAPI jeEngine_ChangeDriverCB * JETCC jeEngine_CreateChangeDriverCB(	jeEngine					*Engine, 
																	jeEngine_ShutdownDriverCB	*ShutdownDriverCB, 
																	jeEngine_StartupDriverCB	*StartupDriverCB,
																	void						*Context)
{
	jeEngine_ChangeDriverCB		*ChangeDriverCB{};

	assert(jeEngine_IsValid(Engine));
	assert(Engine->ChangeDriverCBChain);
	assert(ShutdownDriverCB);
	assert(StartupDriverCB);

	ChangeDriverCB = JE_RAM_ALLOCATE_STRUCT(jeEngine_ChangeDriverCB);

	if (!ChangeDriverCB)
		return nullptr;

	ChangeDriverCB->ShutdownDriverCB = ShutdownDriverCB;
	ChangeDriverCB->StartupDriverCB = StartupDriverCB;
	ChangeDriverCB->Context = Context;

	// If there already is a driver, start it up now
	if (Engine->DriverInfo.RDriver)
	{
		if (!StartupDriverCB(Engine->DriverInfo.RDriver, Context))
		{
			jeRam_Free(ChangeDriverCB);
			return nullptr;
		}
	}

	

	if (!jeChain_AddLinkData(Engine->ChangeDriverCBChain, ChangeDriverCB))
	{
		if (!ShutdownDriverCB(Engine->DriverInfo.RDriver, Context))
		{
			assert(0);
		}
		jeRam_Free(ChangeDriverCB);
		return nullptr;
	}

	return ChangeDriverCB;
}

//===================================================================================
//	jeEngine_DestroyChangeDriverCB
//===================================================================================

JETAPI void JETCC jeEngine_DestroyChangeDriverCB(jeEngine *Engine, jeEngine_ChangeDriverCB **ChangeDriverCB)
{
	jeBoolean		Ret{};

	assert(jeEngine_IsValid(Engine));
	assert(ChangeDriverCB);
	assert(*ChangeDriverCB);
	assert((*ChangeDriverCB)->ShutdownDriverCB);
	assert((*ChangeDriverCB)->StartupDriverCB);
	assert(jeChain_FindLink(Engine->ChangeDriverCBChain, *ChangeDriverCB));

	// Actual calls to shutdown driver start here.
	// If there is a driver, shut it down now
	if (Engine->DriverInfo.RDriver)
	{
		if (!(*ChangeDriverCB)->ShutdownDriverCB(Engine->DriverInfo.RDriver, (*ChangeDriverCB)->Context))
		{
			assert(0);
		}
	}

	Ret = jeChain_RemoveLinkData(Engine->ChangeDriverCBChain, *ChangeDriverCB);
	assert(Ret == JE_TRUE);

	jeRam_Free(*ChangeDriverCB);
	*ChangeDriverCB = nullptr;
}

/*}{**** SECTION : Bitmap Lists  *********************/

//=====================================================================================
//	jeEngine_SetGamma
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_SetGamma(jeEngine *Engine, float Gamma)
{
	assert( jeEngine_IsValid(Engine) );

	if ( Gamma < 0.01f )
		Gamma  = 0.01f;

	if ( ABS( Engine->CurrentGamma - Gamma) < 0.01f )
		return JE_TRUE;

	Engine->CurrentGamma = Gamma;

	jeEngine_UpdateGamma(Engine);

	return JE_TRUE;
}

//=====================================================================================
//	jeEngine_GetGamma
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_GetGamma(jeEngine *Engine, float *Gamma)
{
	assert( jeEngine_IsValid(Engine) );
	assert(Gamma);

	*Gamma = Engine->CurrentGamma;

	return JE_TRUE;//Engine->DriverInfo.RDriver->GetGamma(Gamma);
}

JETAPI void JETCC jeEngine_UpdateGamma(jeEngine *Engine)
{
	DRV_Driver * RDriver{};
	jeFloat LastBitmapGamma{};

	assert( jeEngine_IsValid(Engine) );

	RDriver = Engine->DriverInfo.RDriver;

	LastBitmapGamma = Engine->BitmapGamma;

	if ( RDriver && (RDriver->EngineSettings->CanSupportFlags & DRV_SUPPORT_GAMMA) )
	{
		if ( RDriver->SetGamma(Engine->CurrentGamma) )
			Engine->BitmapGamma = 1.0f;
		else
			Engine->BitmapGamma = Engine->CurrentGamma;
	}
	else
	{
		Engine->BitmapGamma = Engine->CurrentGamma;
	}

	if ( ABS(Engine->BitmapGamma - LastBitmapGamma) < 0.1f )
	{
		Engine->BitmapGamma = LastBitmapGamma;
	}
	else
	{
		// Attach all the bitmaps for the engine
		if (!BitmapList_SetGamma(Engine->AttachedBitmaps, Engine->BitmapGamma))
		{
			jeErrorLog_AddString(-1, "jeEngine_UpdateGamma:  BitmapList_SetGamma for Engine failed", NULL);
		}
	}

}

//================================================================================
//	jeEngine_BitmapListInit
//	Initializes the engine bitmaplist
//================================================================================
jeBoolean jeEngine_BitmapListInit(jeEngine *Engine)
{
	assert( jeEngine_IsValid(Engine) );
	assert(Engine->AttachedBitmaps == NULL);

	if ( Engine->AttachedBitmaps == NULL )
	{
		Engine->AttachedBitmaps = BitmapList_Create();
		if ( ! Engine->AttachedBitmaps )
		{
			jeErrorLog_AddString(-1, "jeEngine_BitmapListInit:  BitmapList_Create failed...", NULL);
			return JE_FALSE;
		}
	}
	return JE_TRUE;
}

//================================================================================
//	jeEngine_BitmapListShutdown
//================================================================================
jeBoolean jeEngine_BitmapListShutdown(jeEngine *Engine)
{
	assert( jeEngine_IsValid(Engine) );

	if ( Engine->AttachedBitmaps )
	{
		assert(	Engine->DriverInfo.RDriver || BitmapList_CountMembersAttached(Engine->AttachedBitmaps) == 0 );

		//BitmapList_DetachAll(Engine->AttachedBitmaps);
		// Destroy detaches for you!
		BitmapList_Destroy(Engine->AttachedBitmaps);
		Engine->AttachedBitmaps = nullptr;
	}

	return JE_TRUE;
}

//================================================================================
//	jeEngine_AddBitmap
//================================================================================
JETAPI jeBoolean JETCC jeEngine_AddBitmap(jeEngine *Engine, jeBitmap *Bitmap, jeEngine_BitmapType Type)
{
	assert( jeEngine_IsValid(Engine) );
	assert(Bitmap);
	assert(Engine->AttachedBitmaps);
	//assert(Engine->FrameState == FrameState_None);

#if (DEBUG_OUTPUT_LEVEL >= 1)
	OutputDebugString("jeEngine_AddBitmap...\n");
#endif

	if ( Type == JE_ENGINE_BITMAP_TYPE_2D )
	{
		jeBitmap_SetDriverFlags(Bitmap,RDRIVER_PF_2D);
	}
	else if ( Type == JE_ENGINE_BITMAP_TYPE_3D )
	{
		jeBitmap_SetDriverFlags(Bitmap,RDRIVER_PF_3D); // <> combine lightmap is irrelevant ?
	}
	else
	{
		jeErrorLog_AddString(-1, "jeEngine_AddBitmap:  Invalid Type!", NULL);
		return JE_FALSE;
	}

	// Add bitmap to the list of bitmaps attached to the engine
	if ( BitmapList_Add(Engine->AttachedBitmaps, (jeBitmap *)Bitmap) )
	{
		if ( Engine->DriverInfo.RDriver )
		{
			if ( ! jeBitmap_AttachToDriver(Bitmap,Engine->DriverInfo.RDriver,0) )
			{
				jeErrorLog_AddString(-1, "jeEngine_AddBitmap:  AttachToDriver failed!", NULL);
				return JE_FALSE;
			}
		}
	}

	return JE_TRUE;
}

//================================================================================
//	jeEngine_RemoveBitmap
//================================================================================
JETAPI jeBoolean JETCC jeEngine_RemoveBitmap(jeEngine *Engine, jeBitmap *Bitmap)
{
	assert( jeEngine_IsValid(Engine) );
	assert(Bitmap);
	assert(Engine->AttachedBitmaps);
//	assert(Engine->FrameState == FrameState_None);

#if (DEBUG_OUTPUT_LEVEL >= 1)
	OutputDebugString("jeEngine_RemoveBitmap...\n");
#endif

	if ( BitmapList_Remove(Engine->AttachedBitmaps, Bitmap) )
	{
		if (!jeBitmap_DetachDriver(Bitmap, JE_TRUE))
		{
			jeErrorLog_AddString(-1, "jeEngine_RemoveBitmap:  jeBitmap_DetachDriver failed...", NULL);
			return JE_FALSE;
		}
	}
	
	return JE_TRUE;
}


/*}{**** SECTION : Render/Draw  *********************/

//================================================================================
//	jeEngine_RenderPoly
//================================================================================
JETAPI void JETCC jeEngine_RenderPoly(const jeEngine *Engine,
	const jeTLVertex *Points, int NumPoints, const jeMaterialSpec *Texture, uint32 Flags)
{
	jeBoolean	Ret{};

	assert(jeEngine_IsValid(Engine));
	assert(Engine->FrameState == FrameState_Begin);
	assert(Points );

	if ( Texture )
	{
		jeTexture* TH{};
		jeRDriver_Layer		Layer{};
		//assert(jeEngine_HasBitmap(Engine, Texture) == JE_TRUE);		// This check is slow, but safe

		TH = jeMaterialSpec_GetLayerTexture(Texture, 0);
		if (TH==nullptr) {
			jeBitmap* bmp = jeMaterialSpec_GetLayerBitmap(Texture, 0);
			if (bmp == nullptr) return;
			TH = jeBitmap_GetTHandle(bmp);
		}
		assert(TH);

		Layer.THandle = TH;

		Flags |= Engine->DefaultRenderFlags;

		Ret = Engine->DriverInfo.RDriver->RenderMiscTexturePoly((jeTLVertex *)Points, NumPoints, &Layer, 1, Flags);
	}
	else
	{
		Ret = Engine->DriverInfo.RDriver->RenderGouraudPoly((jeTLVertex *)Points, NumPoints, Flags);
	}

	assert(Ret == JE_TRUE);
}

JETAPI void JETCC jeEngine_RenderPolyArray(const jeEngine *Engine, const jeTLVertex ** pPoints, int * pNumPoints, int NumPolys, 
								const jeMaterialSpec *Texture, uint32 Flags)
{
	jeBoolean		Ret{};
	int				pn{};
	DRV_Driver* Driver{};
	jeRDriver_Layer Layer{};


	assert(jeEngine_IsValid(Engine));
	assert(Engine->FrameState == FrameState_Begin);
	assert(pPoints && pNumPoints );

	Driver = Engine->DriverInfo.RDriver;
	assert(Driver);

	if ( Texture )
	{
		jeTexture* TH{};
	
		TH = jeMaterialSpec_GetLayerTexture(Texture, 0);
		assert(TH);

		Layer.THandle = TH;

		Flags |= Engine->DefaultRenderFlags;

		for(pn=0;pn<NumPolys;pn++)
		{
			assert(pPoints[pn]);
			Ret = Driver->RenderMiscTexturePoly((jeTLVertex *)pPoints[pn],pNumPoints[pn],&Layer, 1, Flags);
			assert(Ret);
		}
	}
	else
	{
		for(pn=0;pn<NumPolys;pn++)
		{
			assert(pPoints[pn]);
			Ret = Driver->RenderGouraudPoly((jeTLVertex *)pPoints[pn],pNumPoints[pn],Flags);
			assert(Ret);
		}
	}

}

//================================================================================
//	jeEngine_DrawBitmap
//================================================================================
JETAPI jeBoolean JETCC jeEngine_DrawBitmap(const jeEngine *Engine,
	const jeBitmap *Bitmap,
	const jeRect * Source, uint32 x, uint32 y)
{
	jeTexture* TH{};
	jeBoolean			Ret{};
	
	//#pragma message("make jeRect the same as RECT, or don't use RECT!?")
	// The drivers once did not include Jet3D .h's
	// (D3D uses RECT so thats why the drivers adopted RECT's...)
	#pragma message("Engine : Make the drivers use jeRect, JP")

	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_Begin);
	assert(Bitmap);
	
	assert(Engine->AttachedBitmaps);
	assert(BitmapList_Has(Engine->AttachedBitmaps, (jeBitmap *)Bitmap) == JE_TRUE);

	TH = jeBitmap_GetTHandle(Bitmap);
	assert(TH);

	//Ret = Engine->DriverInfo.RDriver->Drawdecal(TH,(RECT *)Source,x,y);

	if (Source)		// Source CAN be NULL!!!
	{
		RECT rect{};

		rect.left = Source->Left;
		rect.top = Source->Top;
		rect.right = Source->Right;
		rect.bottom = Source->Bottom;

		Ret = Engine->DriverInfo.RDriver->DrawDecal(TH, &rect, x,y);
	}
	else
		Ret = Engine->DriverInfo.RDriver->DrawDecal(TH, NULL, x,y);

	if ( ! Ret )
	{
		jeErrorLog_AddString(-1,"jeEngine_DrawBitmap : DrawDecal failed", NULL);	
	}

	return Ret;
}

JETAPI jeTexture *JETCC jeEngine_CreateTextureFromFile(const jeEngine *Engine, jeVFile *File)
{
	jeBitmap* pBmp{};

	if (Engine->DriverInfo.RDriver->THandle_CreateFromFile) {
		return Engine->DriverInfo.RDriver->THandle_CreateFromFile(File);
	}
	
	// code to replace the splash screen - This is a memory leak - paradoxnj
	//pBmp = jeBitmap_CreateFromFile(File);
	//jeEngine_AddBitmap((jeEngine*)Engine, pBmp, JE_ENGINE_BITMAP_TYPE_3D);
	//return jeBitmap_GetTHandle(pBmp);
	return NULL;
}

JETAPI void JETCC jeEngine_DestroyTexture(const jeEngine *Engine, jeTexture *Texture)
{
	Engine->DriverInfo.RDriver->THandle_Destroy(Texture);
}

JETAPI jeBoolean JETCC jeEngine_DrawTexture(const jeEngine *Engine, const jeTexture *Texture, int32 x, int32 y)
{
	return Engine->DriverInfo.RDriver->DrawDecal((jeTexture*)Texture, NULL, x, y);
}

//================================================================================
//	jeEngine_DrawBitmap3D
//================================================================================
JETAPI jeBoolean JETCC jeEngine_DrawBitmap3D(const jeEngine *Engine,
	const jeBitmap *Bitmap, const jeRect * pRect, uint32 x, uint32 y)
{
	jeTexture* TH{};
	jeBoolean			Ret{};
	float				w{}, h{};
	float				u1{}, v1{}, u2{}, v2{};
	jeTLVertex			Points[4];
	jeRect				Rect{};
	jeRDriver_Layer		Layer{};

	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_Begin);
	assert(Bitmap);
	
	assert(Engine->AttachedBitmaps);
	assert(BitmapList_Has(Engine->AttachedBitmaps, (jeBitmap *)Bitmap) == JE_TRUE);

	w = (float)jeBitmap_Width( Bitmap);
	h = (float)jeBitmap_Height(Bitmap);

	assert( w <= 256.0f );
	assert( h <= 256.0f );

	w = 1.0f/w; h = 1.0f/h;

	TH = jeBitmap_GetTHandle(Bitmap);
	assert(TH);

	if ( pRect )
	{
		Rect = *pRect;
		Rect.Right ++;
		Rect.Bottom ++;
	}
	else
	{
		Rect.Left = Rect.Top = 0;
		Rect.Right = jeBitmap_Width( Bitmap);
		Rect.Bottom = jeBitmap_Height(Bitmap);
	}

	u1 = Rect.Left * w;
	u2 = Rect.Right* w;
	v1 = Rect.Top  * h;
	v2 = Rect.Bottom*h;

	assert( u1 >= 0.0f && u1 <= 1.0f );
	assert( u2 >= 0.0f && u2 <= 1.0f );
	assert( v1 >= 0.0f && v1 <= 1.0f );
	assert( v2 >= 0.0f && v2 <= 1.0f );
	assert( u2 >= u1 && v2 >= v1 );

	w = (float)(Rect.Right - Rect.Left);
	h = (float)(Rect.Bottom - Rect.Top);

	Points[0].x = (float)x;
	Points[0].y = (float)y;
	Points[0].z = 1.0f;
	Points[0].u = u1;
	Points[0].v = v1;
	Points[0].r = Points[0].g = Points[0].b = Points[0].a = 255.0f;

	Points[3] = Points[2] = Points[1] = Points[0];

	Points[2].u = Points[1].u = u2;
	Points[1].x += w;
	Points[2].x += w;

	Points[3].v = Points[2].v = v2;
	Points[2].y += h;
	Points[3].y += h;

	Layer.THandle = TH;

	Ret = Engine->DriverInfo.RDriver->RenderMiscTexturePoly((jeTLVertex *)Points, 4, &Layer, 1, JE_RENDER_FLAG_CLAMP_UV);

	if ( ! Ret )
	{
		jeErrorLog_AddString(-1,"jeEngine_DrawBitmap3D : Render failed", NULL);	
	}

	return Ret;
}

//=====================================================================================
//	jeEngine_AttachAll
//=====================================================================================
jeBoolean jeEngine_AttachAll(jeEngine *Engine)
{
	DRV_Driver* RDriver{};

	assert( Engine );
	// called in beginframe	

	RDriver = Engine->DriverInfo.RDriver;
	assert( RDriver );

    // If current driver is not active, then split
	if (! RDriver)
        return JE_TRUE;

#if (DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("BEGIN BitmapList_AttachAll\n");
#endif

	// Attach all the bitmaps for the engine
	if (!BitmapList_AttachAll(Engine->AttachedBitmaps, RDriver, Engine->BitmapGamma))
	{
		jeErrorLog_AddString(-1, "jeEngine_AttachAll:  BitmapList_AttachAll for Engine failed...", NULL);
		return JE_FALSE;
	}

#if (DEBUG_OUTPUT_LEVEL >= 2)
	OutputDebugString("END BitmapList_AttachAll\n");
#endif

	return JE_TRUE;
}

//=====================================================================================
//	jeEngine_DetachAll
//=====================================================================================
jeBoolean jeEngine_DetachAll(jeEngine *Engine)
{
	assert( jeEngine_IsValid(Engine) );

	// Shutdown all the jeBitmaps
	if (!BitmapList_DetachAll(Engine->AttachedBitmaps))
	{
		jeErrorLog_AddString(-1, "jeEngine_DetachAll:  BitmapList_DetachAll failed for engine.", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

/*}{**** SECTION : Init/Reset/Shutdown  *********************/

extern unsigned char splash_bmp[];
extern int splash_bmp_Length;
static	jeBoolean	jeEngine_DoSplashScreen(jeEngine *Engine, jeDriver_Mode *DriverMode)
{
	int32					Width{}, Height{};
	jeRect 					Rect{};
	jeBitmap* Bitmap{};
	jeTexture* Texture{};
	jeTexture_Info			Info{};
	jeVFile* MemFile{};
	jeVFile_MemoryContext	Context{};
	int32					ImageWidth{}, ImageHeight{};
	int32					X{}, Y{};
	jeBoolean				UseJeBitmap = JE_FALSE;

	jeDriver_ModeGetWidthHeight(DriverMode, &Width, &Height);
	if (Width == -1)
	{
	
		RECT	R{};
		GetClientRect(Engine->hWnd, &R);

		Rect.Left = R.left;
		Rect.Right = R.right;
		Rect.Top = R.top;
		Rect.Bottom = R.bottom;
	}
	else
	{
		Rect.Left = 0;
		Rect.Right = Width-1;
		Rect.Top = 0;
		Rect.Bottom = Height-1;
	}

	Context.Data = splash_bmp;
	Context.DataLength = splash_bmp_Length;
	MemFile = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_MEMORY, NULL, &Context, JE_VFILE_OPEN_READONLY);
	if	(!MemFile)
		return JE_FALSE;
	
	Texture = jeEngine_CreateTextureFromFile(Engine, MemFile);
	if (!Texture)
	{
		UseJeBitmap = JE_TRUE;

		Bitmap = jeBitmap_CreateFromFile(MemFile);
		if (!Bitmap)
		{
			OutputDebugString("jeEngine_DoSplashScreen:  Could not create bitmap!!");
			return JE_FALSE;
		}

		if (jeEngine_AddBitmap(Engine, Bitmap, JE_ENGINE_BITMAP_TYPE_2D) == JE_FALSE)
		{
			OutputDebugString("jeEngine_DoSplashScreen:  Could not add bitmap to engine!!");
			return JE_FALSE;
		}

		ImageWidth = jeBitmap_Width(Bitmap);
		ImageHeight = jeBitmap_Height(Bitmap);
	}
	else
	{
		Engine->DriverInfo.RDriver->THandle_GetInfo(Texture, 0, &Info);

		ImageWidth = Info.Width;
		ImageHeight = Info.Height;
	}

	jeVFile_Close(MemFile);

	X = (Rect.Right - ImageWidth) / 2;
	Y = (Rect.Bottom - ImageHeight) / 2;
	
	
	jeEngine_BeginFrame(Engine, NULL, JE_TRUE);

	if (!UseJeBitmap)
		jeEngine_DrawTexture(Engine, Texture, X, Y);
	else
		jeEngine_DrawBitmap(Engine, Bitmap, NULL, X, Y);

	jeEngine_EndFrame(Engine);
	
	if (UseJeBitmap)
	{
		jeEngine_RemoveBitmap(Engine, Bitmap);
		jeBitmap_Destroy(&Bitmap);
	}
	else
	{
		jeEngine_DestroyTexture(Engine, Texture);
		Texture = nullptr;
	}

	Sleep(2000);
	return JE_TRUE;
}

//=====================================================================================
//	jeEngine_SetDriverAndMode
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_SetDriverAndMode(	jeEngine		*Engine, 
												HWND			hWnd,
												jeDriver		*Driver, 
												jeDriver_Mode	*DriverMode)
{
	jeDeviceCaps		DeviceCaps{};

	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_None);		// They can't change modes in between begin/end frame calls
	assert(Driver);
	assert(DriverMode);

#if (DEBUG_OUTPUT_LEVEL >= 1)
	OutputDebugString("BEGIN jeEngine_SetDriverAndMode\n");
#endif

	//	Set up the Render Driver
	if (!Engine_InitDriver(Engine, hWnd, Driver, DriverMode))
		return JE_FALSE;

	// Get the default suggested render flags
	jeEngine_GetDeviceCaps(Engine, &DeviceCaps);
	// Set them
	jeEngine_SetDefaultRenderFlags(Engine, DeviceCaps.SuggestedDefaultRenderFlags);

	jeEngine_UpdateGamma(Engine);

	//if (!Engine->FontInfo.Font) {
	//	Engine->FontInfo.Font = jeEngine_CreateFont(Engine, 18, 0, JE_FONT_BOLD, JE_FALSE, "Arial");
	//}
/*
	if (!Engine->FontInfo.Font)
		return JE_FALSE;
*/
#if 1
	// Do the splash screen
	if	(Engine->SplashDisplayed == JE_FALSE)
	{
		if	(jeEngine_DoSplashScreen(Engine, DriverMode) == JE_FALSE)
			return JE_FALSE;
		Engine->SplashDisplayed = JE_TRUE;
	}
#endif

#if (DEBUG_OUTPUT_LEVEL >= 1)
	OutputDebugString("END jeEngine_SetDriverAndMode\n");
#endif

	return JE_TRUE;
}

//=====================================================================================
//	jeEngine_GetDriverAndMode
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_GetDriverAndMode(	const jeEngine *Engine, 
												jeDriver **Driver, 
												jeDriver_Mode **DriverMode)
{
	assert( jeEngine_IsValid(Engine) );
	assert(Driver);
	assert(DriverMode);

	*Driver = Engine->DriverInfo.CurDriver;
	*DriverMode = Engine->DriverInfo.CurMode;

	return JE_TRUE;
}

//=====================================================================================
//	jeEngine_GetDriverSystem
//=====================================================================================
JETAPI jeDriver_System * JETCC jeEngine_GetDriverSystem(jeEngine *Engine)
{
	assert( jeEngine_IsValid(Engine) );

	return (jeDriver_System*)&Engine->DriverInfo;
}

//=====================================================================================
//	jeEngine_ShutdownDriver
//=====================================================================================
JETAPI jeBoolean JETCC jeEngine_ShutdownDriver(jeEngine *Engine)
{
	//	by trilobite jan. 2011
	//Engine_DriverInfo *DrvInfo;
	Engine_DriverInfo* DrvInfo{};
	//

	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_None);

	DrvInfo = &(Engine->DriverInfo);

	assert(DrvInfo);

	if (!DrvInfo->RDriver)
		return JE_TRUE;			// Just return true, and don't do nothing

	// Destroy the font
	//if (Engine->FontInfo.Font)
	//	jeEngine_DestroyFont(Engine, &Engine->FontInfo.Font);

	// First, reset the driver
	if (!jeEngine_ResetDriver(Engine))
	{
		jeErrorLog_AddString(-1, "jeEngine_ShutdownDriver:  jeEngine_ResetDriver failed.", NULL);
		return JE_FALSE;
	}

	// Shutdown the driver
	DrvInfo->RDriver->Shutdown();

	if	(DrvInfo->DriverHandle)
	{
		if (!FreeLibrary((HINSTANCE)(DrvInfo->DriverHandle)) )
			return JE_FALSE;
	}

	DrvInfo->RDriver = nullptr;
	DrvInfo->DriverHandle = (int32)NULL;

	return JE_TRUE;
}

//=====================================================================================
//	jeEngine_ResetDriver
//=====================================================================================
jeBoolean jeEngine_ResetDriver(jeEngine *Engine)
{
	jeChain_Link		*Link{};

	assert(Engine != NULL);
	assert(Engine->DriverInfo.RDriver);

	// To be safe, detach all things from the current driver
	if (!jeEngine_DetachAll(Engine))
	{
		jeErrorLog_AddString(-1, "jeEngine_ResetDriver:  jeEngine_DetachAll failed.", NULL);
		return JE_FALSE;
	}

	for (Link = jeChain_GetFirstLink(Engine->ChangeDriverCBChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeEngine_ChangeDriverCB* ChangeDriverCB{};

		ChangeDriverCB = (jeEngine_ChangeDriverCB*)jeChain_LinkGetLinkData(Link);
		assert(ChangeDriverCB);

		if (!ChangeDriverCB->ShutdownDriverCB(Engine->DriverInfo.RDriver, ChangeDriverCB->Context))
			return JE_FALSE;
	}

	// Reset the driver
	if (!Engine->DriverInfo.RDriver->Reset())
	{
		jeErrorLog_AddString(-1, "jeEngine_ResetDriver:  Engine->DriverInfo.RDriver->Reset() failed.", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

//===================================================================================
//	jeEngine_InitFonts
//===================================================================================
jeBoolean jeEngine_InitFonts(jeEngine *Engine)
{
	Engine_FontInfo* Fi{};

	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_None);

	Fi = &Engine->FontInfo;

	assert(Fi->FontBitmap == NULL);

	// Load the bitmap
	{
		jeVFile* MemFile{};
		jeVFile_MemoryContext	Context{};

		{
			extern unsigned char font_bmp[];
			extern int font_bmp_length;

			Context.Data = font_bmp;
			Context.DataLength = font_bmp_length;

			MemFile = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_MEMORY, NULL, &Context, JE_VFILE_OPEN_READONLY);
		}

		if	(!MemFile)
		{
			jeErrorLog_AddString(-1,"InitFonts : jeVFile_OpenNewSystem Memory fontbmp failed.", NULL);
			return JE_FALSE;
		}

		if ( (Fi->FontBitmap = jeBitmap_CreateFromFile(MemFile)) == NULL)
		{
			jeErrorLog_AddString(-1,"InitFonts : jeBitmap_CreateFromFile failed.", NULL);
			goto fail;
		}

		#if 0
		#pragma message("Engine : fonts will have alpha once Decals do : CB");
		// <> CB : give fonts alpha so they look purty
		//			pointless right now cuz we don't get enum'ed a _2D_ type with alpha
		{
		jeBitmap * FontAlpha;
			FontAlpha = jeBitmap_Create( jeBitmap_Width(Fi->FontBitmap), jeBitmap_Height(Fi->FontBitmap), 1, JE_PIXELFORMAT_8BIT_GRAY );
			if ( FontAlpha )
			{
				if ( jeBitmap_BlitBitmap(Fi->FontBitmap,FontAlpha) )
				{
					if ( ! jeBitmap_SetAlpha( Fi->FontBitmap, FontAlpha ) )
					{
						jeErrorLog_AddString(-1,"InitFonts : SetAlpha failed : non-fatal", NULL);
					}
				}
				else
				{
					jeErrorLog_AddString(-1,"InitFonts : BlitBitmap failed : non-fatal", NULL);
				}
				jeBitmap_Destroy(&FontAlpha);
			}
		}
		#endif

		if (!jeBitmap_SetColorKey(Fi->FontBitmap, JE_TRUE, 0, JE_FALSE))
		{
			jeErrorLog_AddString(-1,"InitFonts : jeBitmap_SetColorKey failed.", NULL);
			goto fail;
		}

		if ( ! jeEngine_AddBitmap(Engine,Fi->FontBitmap,JE_ENGINE_BITMAP_TYPE_2D) )
		{
			jeErrorLog_AddString(-1,"InitFonts : jeEngine_AddBitmap failed.", NULL);
			goto fail;
		}

		goto success;

		fail:

		jeVFile_Close(MemFile);
		return JE_FALSE;

		success:
		
		jeVFile_Close(MemFile);
	}

	//
	//	Setup font lookups
	//
	{
		int PosX{}, PosY{}, Width{}, i{};

		PosX = 0;
		PosY = 0;
		Width = 128*8;

		for (i=0; i< 128; i++)
		{
			Fi->FontLUT1[i] = (PosX<<16) | PosY;
			PosX+=8;

			if (PosX >= Width)
			{
				PosY += 14;
				PosX = 0;
			}
		}
	}

	return JE_TRUE;
}

//===================================================================================
//	jeEngine_ShutdownFonts
//===================================================================================
jeBoolean jeEngine_ShutdownFonts(jeEngine *Engine)
{
	Engine_FontInfo* Fi{};

	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_None);

	Fi = &Engine->FontInfo;

	if (Fi->FontBitmap)
	{
		if (!jeEngine_RemoveBitmap(Engine, Fi->FontBitmap))
		{
			jeErrorLog_AddString(-1, "jeEngine_ShutdownFonts:  jeEngine_RemoveBitmap failed.", NULL);
			return JE_FALSE;
		}

		jeBitmap_Destroy(&Fi->FontBitmap);
	}

	return JE_TRUE;
}

//=====================================================================================
//	jeEngine_LoadLibrary
//=====================================================================================

HINSTANCE jeEngine_LoadLibrary( const char * lpLibFileName, const char *DriverDirectory)
{
	char	Buff[_MAX_PATH];
	char* StrEnd{};
	HINSTANCE	Library{};

	//-------------------------
	strcpy(Buff, DriverDirectory);
	StrEnd = Buff + strlen(Buff) - 1;
	if ( *StrEnd != '\\' && *StrEnd != '/' && *StrEnd != ':' )
	{
		strcat(Buff,"\\");
	}
	strcat(Buff, lpLibFileName);
	Library = LoadLibrary(Buff);
	if ( Library )
		return Library;

#pragma message("Engine : LoadLibrary : need jeConfig_GetDriverDir")
#ifdef LOADLIBRARY_HARDCODES
	#pragma message("Engine : using LoadLibrary HardCodes : curdir, q:\\jet, c:\\jet")

	//-------------------------

	strcpy(Buff, "q:\\jet");
	StrEnd = Buff + strlen(Buff) - 1;
	if ( *StrEnd != '\\' && *StrEnd != '/' && *StrEnd != ':' )
	{
		strcat(Buff,"\\");
	}
	strcat(Buff, lpLibFileName);
	Library = LoadLibrary(Buff);
	if ( Library )
		return Library;

	//-------------------------

	strcpy(Buff, "c:\\jet3d");
	StrEnd = Buff + strlen(Buff) - 1;
	if ( *StrEnd != '\\' && *StrEnd != '/' && *StrEnd != ':' )
	{
		strcat(Buff,"\\");
	}
	strcat(Buff, lpLibFileName);
	Library = LoadLibrary(Buff);
	if ( Library )
		return Library;
#endif

return NULL;
}
 
extern GInfo GlobalInfo;		// AHH!!!  Get rid of this!!!

//=====================================================================================
//	EngineInitDriver
//=====================================================================================

static jeBoolean Engine_InitDriver(	jeEngine		*Engine, 
									HWND			hWnd,
									jeDriver		*Driver,
									jeDriver_Mode	*DriverMode)
{
	Engine_DriverInfo* DrvInfo{};
	DRV_Hook* Hook{};
	DRV_DriverHook		DLLDriverHook{};
	DRV_Driver* RDriver{};
	jeChain_Link* Link{};

	assert(jeEngine_IsValid(Engine));
	assert(Engine->FrameState == FrameState_None);

	assert(Driver != NULL);
	assert(DriverMode != NULL);

	DrvInfo = &Engine->DriverInfo;

	// jeEngine_ShutdownDriver calls _Reset which detaches all

	if (! jeEngine_ShutdownDriver(Engine))
	{
		jeErrorLog_AddString(-1, "Engine_InitDriver:  jeEngine_ShutdownDriver failed.", NULL);
		goto Failure;
	}
	assert(!DrvInfo->RDriver);
	
	DrvInfo->CurDriver = Driver;
	DrvInfo->CurMode = DriverMode;

	if (!Driver->HookProc)
	{
		assert(Engine->DriverDirectory);
		DrvInfo->DriverHandle = (int32)jeEngine_LoadLibrary(Driver->FileName, Engine->DriverDirectory);
	
		if (!DrvInfo->DriverHandle)
		{
			jeErrorLog_Add(JE_ERR_DRIVER_NOT_FOUND, NULL);
			goto Failure;
		}
	
		Hook = (DRV_Hook*)GetProcAddress((HINSTANCE)(DrvInfo->DriverHandle), "DriverHook");		
		if (!Hook)
		{
			jeErrorLog_Add(JE_ERR_INVALID_DRIVER, NULL);
			goto Failure;
		}
	}
	else
	{
		Hook = Driver->HookProc;
	}

	if (!Hook(&DrvInfo->RDriver))
	{
		DrvInfo->RDriver = NULL;
		jeErrorLog_Add(JE_ERR_INVALID_DRIVER, NULL);
		goto Failure;
	}

	assert(DrvInfo->RDriver);

	// Get a handy pointer to the driver
	RDriver = DrvInfo->RDriver;

	if (RDriver->VersionMajor != DRV_VERSION_MAJOR || RDriver->VersionMinor != DRV_VERSION_MINOR)
	{
		jeErrorLog_Add(JE_ERR_INVALID_DRIVER, NULL);
		goto Failure;
	}

	strcpy(DLLDriverHook.AppName, Engine->AppName);

	//
	//	Setup what driver they want
	//

	DLLDriverHook.Driver = Driver->Id;
	strcpy(DLLDriverHook.DriverName, Driver->Name);
	DLLDriverHook.Mode = DriverMode->Id;
	DLLDriverHook.Width = DriverMode->Width;
	DLLDriverHook.Height = DriverMode->Height;
	DLLDriverHook.hWnd = hWnd;
	strcpy(DLLDriverHook.ModeName, DriverMode->Name);
	
	if (!RDriver->Init(&DLLDriverHook))
	{
		jeErrorLog_Add(JE_ERR_DRIVER_INIT_FAILED, NULL);
		jeErrorLog_AddString(-1, RDriver->LastErrorStr , NULL);
		goto Failure;
	}

	Engine->hWnd = hWnd;		// Store the new hWnd

#if (DEBUG_OUTPUT_LEVEL >= 1)
	OutputDebugString("BEGIN StartupDriverCB\n");
#endif

	// Call all the changedriver CB's to notify them of the new driver
	for (Link = jeChain_GetFirstLink(Engine->ChangeDriverCBChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeEngine_ChangeDriverCB* ChangeDriverCB{};

		ChangeDriverCB = (jeEngine_ChangeDriverCB*)jeChain_LinkGetLinkData(Link);
		assert(ChangeDriverCB);

		if (!ChangeDriverCB->StartupDriverCB(RDriver, ChangeDriverCB->Context))
			{
				jeErrorLog_Add(JE_ERR_DRIVER_INIT_FAILED, NULL);
				goto Failure;
			}
	}

#if (DEBUG_OUTPUT_LEVEL >= 1)
	OutputDebugString("END StartupDriverCB\n");
#endif

	return JE_TRUE;

	Failure:
	#pragma message("need better clean up on failure (restore previous mode)")
	DrvInfo->RDriver = nullptr;
	return JE_FALSE;
}

#pragma warning (default:4100)

JETAPI jeBoolean JETCC jeEngine_FlushScene(jeEngine *Engine)
{
	DRV_Driver* RDriver{};

	assert( jeEngine_IsValid(Engine) );
	assert(Engine->FrameState == FrameState_Begin);

	RDriver = Engine->DriverInfo.RDriver;
	assert(RDriver);

	RDriver->EndBatch();
	RDriver->BeginBatch();

	return JE_TRUE;
}

//===================================================================================
//	Engine_Tick
//===================================================================================
static void Engine_Tick(jeEngine *Engine)
{
	int32		i{};

	for (i=0; i< 20; i++)
	{
		if (Engine->WaveDir[i] == 1)
			Engine->WaveTable[i] += 14;
		else
			Engine->WaveTable[i] -= 14;
		if (Engine->WaveTable[i] < 50)
		{
			Engine->WaveTable[i] += 14;
			Engine->WaveDir[i] = 1;
		}
		if (Engine->WaveTable[i] > 255)
		{
			Engine->WaveTable[i] -= 14;
			Engine->WaveDir[i] = 0;
		}
	}
}

/*}{**** SECTION : Text  *********************/

//===================================================================================
//	Engine_DrawFontBuffer
//===================================================================================
static void Engine_DrawFontBuffer(jeEngine *Engine)
{
	//jeRect			Rect;
	int32			i{}, x{}, y{}, size{}, StrLength{};
	//int32           w, r,g,b;
	Engine_FontInfo* Fi{};
	char* Str{};
	int32			FontWidth{}, FontHeight{};

	assert(jeEngine_IsValid(Engine));
	assert(Engine->FrameState == FrameState_Begin);
		
	Fi = &Engine->FontInfo;

	if ( Fi->NumStrings == 0) 
		return;

	assert( Fi->FontBitmap );
	assert( jeBitmap_GetTHandle(Fi->FontBitmap) );

	FontWidth	= 8;
	FontHeight	= 15;
		
	for (i=0; i< Fi->NumStrings; i++)
	{
		uint32					color;

		x = Fi->ClientStrings[i].x;
		y = Fi->ClientStrings[i].y;
		color = Fi->ClientStrings[i].Color;
		//g = Fi->ClientStrings[i].g;
		//b = Fi->ClientStrings[i].b;

		size = Fi->ClientStrings[i].size;
		Str = Fi->ClientStrings[i].String;
		StrLength = strlen(Str);

		//jeEngine_DrawText(Engine,Str,x,y,r,g,b);
		//if (Engine->DriverInfo.RDriver->Font_Draw) 
		//{
		//	Engine->DriverInfo.RDriver->Font_Draw(Fi->ClientStrings[i].Font, x, y, color, Str);
		//}
		if (Engine->DriverInfo.RDriver->DrawText)
		{
			Engine->DriverInfo.RDriver->DrawText(Str, x, y, color);
		}
		/*else 
		{
		   for (w=0; w< StrLength; w++)
		   {
			   //Rect.left = (Fi->FontLUT1[*Str]>>16)+1;
			   //Rect.right = Rect.left+16;
			   //Rect.top = (Fi->FontLUT1[*Str]&0xffff)+1;
			   //Rect.bottom = Rect.top+16;
			   
			   Rect.Left = (Fi->FontLUT1[*Str]>>16);
			   Rect.Right = Rect.Left + FontWidth - 1;
			   Rect.Top = (Fi->FontLUT1[*Str]&0xffff);
			   Rect.Bottom = Rect.Top + FontHeight - 1;

			   if ( ! jeEngine_DrawBitmap(Engine, Fi->FontBitmap, &Rect, x, y) )
			   {
				   // this is circular : printf failed, so use printf to write an error !?
				   //jeEngine_Printf(Engine, 10, 50, "Could not draw font...\n");
				   jeErrorLog_AddString(-1,"DrawFontBuffer : Could not draw font...\n", NULL);
			   }
			   //x+= 16;
			   x += FontWidth;
			   Str++;
		   }
		}*/
	}

	Fi->NumStrings = 0;
	Fi->NumDebugStrings = 0;
}

static void SubLarge(LARGE_INTEGER *start, LARGE_INTEGER *end, LARGE_INTEGER *delta)
{
	_asm {
		mov ebx,dword ptr [start]
		mov esi,dword ptr [end]

		mov eax,dword ptr [esi+0]
		sub eax,dword ptr [ebx+0]

		mov edx,dword ptr [esi+4]
		sbb edx,dword ptr [ebx+4]

		mov ebx,dword ptr [delta]
		mov dword ptr [ebx+0],eax
		mov dword ptr [ebx+4],edx
	}
}

//===================================================================================
// jeEngine_Puts
//===================================================================================
jeBoolean jeEngine_Puts(jeEngine *Engine, jeFont *Font, int32 x, int32 y, uint32 Color, const char *String)
{
	Engine_FontInfo* Fi{};

	Fi = &Engine->FontInfo;

	if (strlen(String) >= MAX_CLIENT_STRING_LEN)
		return JE_FALSE;
					 
	if (Fi->NumStrings >= MAX_CLIENT_STRINGS)
		return JE_FALSE;

	strcpy(Fi->ClientStrings[Fi->NumStrings].String, String);

	Fi->ClientStrings[Fi->NumStrings].x = x;	
	Fi->ClientStrings[Fi->NumStrings].y = y;

	Fi->ClientStrings[Fi->NumStrings].Color = Color;
	Fi->ClientStrings[Fi->NumStrings].size = 0;
	Fi->ClientStrings[Fi->NumStrings].Font = Font;

	Fi->NumStrings++;

	return JE_TRUE;
}

//========================================================================================
//	jeEngine_Printf
//========================================================================================
JETAPI jeBoolean JETCC jeEngine_Printf(jeEngine *Engine, jeFont *Font, int32 x, int32 y, uint32 Color, const char *String, ...)
{
	va_list			ArgPtr{};
    char			TempStr[1024];

	assert(jeEngine_IsValid(Engine));
//	assert(Engine->FrameState == FrameState_Begin); // can do this anywhere

	va_start(ArgPtr, String);
    vsprintf(TempStr, String, ArgPtr);
	va_end(ArgPtr);

	return jeEngine_Puts(Engine, Font, x, y, Color, TempStr);
}


jeBoolean jeEngine_DebugPrintf(jeEngine *Engine, uint32 Color, const char *String, ...)
{
	jeBoolean ret{};
	va_list			ArgPtr{};
   	char			TempStr[1024];

	assert(jeEngine_IsValid(Engine));
//	assert(Engine->FrameState == FrameState_Begin); // can do this anywhere

	va_start(ArgPtr, String);
    vsprintf(TempStr, String, ArgPtr);
	va_end(ArgPtr);

	ret = jeEngine_Puts(Engine, Engine->FontInfo.Font, 2, 2 + 15 * Engine->FontInfo.NumDebugStrings, Color, TempStr);

	Engine->FontInfo.NumDebugStrings++;

	return ret;
}


/*}{**** SECTION : THandles  *********************/

/*}{**** SECTION : jeDriver stuff *********************/

#pragma message ("Engine : jeDriver_* : do these go here?  (jeDriver name space) :")  

//=====================================================================================
//	jeDriver_SystemGetNextDriver
//=====================================================================================
JETAPI jeDriver * JETCC jeDriver_SystemGetNextDriver(jeDriver_System *DriverSystem, jeDriver *Start)
{
	Engine_DriverInfo* DriverInfo{};
	jeDriver* Last{};

	assert(DriverSystem != NULL);
	
	DriverInfo = (Engine_DriverInfo*)DriverSystem;

	if (!DriverInfo->NumSubDrivers)
		return NULL;

	Last = &DriverInfo->SubDrivers[DriverInfo->NumSubDrivers-1];

	if (Start)							// If they have a driver, return the next one
		Start++;
	else
		Start = DriverInfo->SubDrivers;	// Else, return the first one...

	if (Start > Last)					// No more drivers left
		return NULL;

	// This must be true!!!
	assert(Start >= DriverInfo->SubDrivers && Start <= Last);

	return Start;	 // This is it...
}

//=====================================================================================
//	jeDriver_GetNextMode
//=====================================================================================
JETAPI jeDriver_Mode * JETCC jeDriver_GetNextMode(jeDriver *Driver, jeDriver_Mode *Start)
{
	jeDriver_Mode* Last{};

	Last = &Driver->Modes[Driver->NumModes-1];

	if (Start)						// If there is a start, return the next one
		Start++;
	else
		Start = Driver->Modes;		// Else, return the first

	if (Start > Last)				// No more Modes left
		return NULL;

	// This must be true...
	assert(Start >= Driver->Modes && Start <= Last);

	return Start;
}

//=====================================================================================
//	jeDriver_GetName
//=====================================================================================
JETAPI jeBoolean JETCC jeDriver_GetName(const jeDriver *Driver, const char **Name)
{
	assert(Driver);
	assert(Name);

	*Name = Driver->Name;

	return JE_TRUE;
}

//=====================================================================================
//	jeDriver_ModeGetName
//=====================================================================================
JETAPI jeBoolean JETCC jeDriver_ModeGetName(const jeDriver_Mode *Mode, const char **Name)
{
	assert(Mode);
	assert(Name);

	*Name = Mode->Name;

	return JE_TRUE;
}

//=====================================================================================
//	jeDriver_ModeGetWidthHeight
//=====================================================================================
JETAPI jeBoolean JETCC jeDriver_ModeGetWidthHeight(const jeDriver_Mode *Mode, int32 *pWidth, int32 *pHeight)
{
	assert(Mode);
	assert(pWidth);
	assert(pHeight);

	*pWidth = Mode->Width;
	*pHeight = Mode->Height;

	return JE_TRUE;
}

JETAPI jeBoolean	JETCC jeDriver_ModeGetAttributes(const jeDriver_Mode *Mode, int32 *pWidth, int32 *pHeight, int32 *pBpp)
{
	assert(Mode);
	assert(pWidth);
	assert(pHeight);

	*pWidth = Mode->Width;
	*pHeight = Mode->Height;
	*pBpp = Mode->Bpp;

	return JE_TRUE;
}

//===================================================================================
//	EnumSubDriversCB
//===================================================================================
static jeBoolean Engine_EnumSubDriversCB(int32 DriverId, char *Name, void *Context)
{
	Engine_DriverInfo* DriverInfo{};
	DriverInfo = (Engine_DriverInfo*)Context;
	DRV_Driver* RDriver{};
	jeDriver* Driver{};

	if (DriverInfo->NumSubDrivers+1 >= MAX_SUB_DRIVERS)
		return JE_FALSE;		// Stop when no more driver slots available

	Driver = &DriverInfo->SubDrivers[DriverInfo->NumSubDrivers];
	
	Driver->Id = DriverId;
	strcpy(Driver->Name, Name);
	if	(DriverInfo->CurHookProc)
		Driver->HookProc = DriverInfo->CurHookProc;
	else
		strcpy(Driver->FileName, DriverInfo->CurFileName);

	RDriver = DriverInfo->RDriver;

	// Store this, so enum modes know what driver we are working on...
	DriverInfo->CurDriver = Driver;
	
	if (!RDriver->EnumModes(Driver->Id, Driver->Name, Engine_EnumModesCB, (void*)DriverInfo))
		return FALSE;

	DriverInfo->NumSubDrivers++;

	return JE_TRUE;
}

//===================================================================================
//	EnumModesCB
//===================================================================================
static jeBoolean Engine_EnumModesCB(int32 ModeId, char *Name, int32 Width, int32 Height, int32 Bpp, void *Context)
{
	Engine_DriverInfo* DriverInfo{};
	jeDriver* Driver{};
	jeDriver_Mode* Mode{};

	DriverInfo = (Engine_DriverInfo*)Context;

	Driver = DriverInfo->CurDriver;
	
	if (Driver->NumModes+1 >= MAX_DRIVER_MODES)
		return JE_FALSE;

	Mode = &Driver->Modes[Driver->NumModes];

	Mode->Id = ModeId;
	strcpy(Mode->Name, Name);
	Mode->Width = Width;
	Mode->Height = Height;
	Mode->Bpp = Bpp;

	Driver->NumModes++;

	return JE_TRUE;
}


//===================================================================================
//	EnumSubDrivers
//===================================================================================
static jeBoolean Engine_EnumSubDrivers(Engine_DriverInfo *DriverInfo, const char *DriverDirectory)
{
	DRV_Hook* DriverHook{};
#ifdef WIN32
	HINSTANCE			Handle{};
#endif
#ifdef BUILD_BE
	image_id			Handle;
#endif

	DRV_Driver* RDriver{};
	jeVFile* DosDir{};
	jeVFile_Finder* Finder{};

	assert(DriverDirectory);

	DosDir = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_DOS, DriverDirectory, NULL, JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY);
	if	(!DosDir)
		return JE_TRUE;
	Finder = jeVFile_CreateFinder(DosDir, "*.dll");
	if	(!Finder)
	{
		jeVFile_Close(DosDir);
		return JE_FALSE;
	}

	DriverInfo->NumSubDrivers = 0;
	DriverInfo->CurHookProc = nullptr;

	while	(jeVFile_FinderGetNextFile(Finder) == JE_TRUE)
	{
		jeVFile_Properties	Properties;

		jeVFile_FinderGetProperties(Finder, &Properties);

		Handle = jeEngine_LoadLibrary(Properties.Name, DriverDirectory);

		if (!Handle)
			continue;

		strcpy(DriverInfo->CurFileName, Properties.Name);

		DriverHook = (DRV_Hook*)GetProcAddress((HINSTANCE)(Handle), "DriverHook");
		if (!DriverHook)
		{
			FreeLibrary(Handle);
			continue;
		}

		if (!DriverHook(&RDriver))
		{
			FreeLibrary(Handle);
			continue;
		}

		if (RDriver->VersionMajor != DRV_VERSION_MAJOR || RDriver->VersionMinor != DRV_VERSION_MINOR)
		{
			jeErrorLog_AddString(-1,"Engine_EnumSubDrivers : found driver of wrong vesion (non-fatal)",Properties.Name);
			FreeLibrary(Handle);
			continue;
		}

		DriverInfo->RDriver = RDriver;	// temporary storage of the RDriver pointer
		
		if (!RDriver->EnumSubDrivers(Engine_EnumSubDriversCB, (void*)DriverInfo))
		{
			jeErrorLog_AddString(-1,"Engine_EnumSubDrivers : RDriver->EnumSub failed!)",Properties.Name);
			FreeLibrary(Handle);
			continue;		// Should we return FALSE, or just continue?
							// if you change your mind and decide to return, be sure to destroy the finder.
		}

		DriverInfo->RDriver = nullptr;		// clear out the RDriver pointer!

		FreeLibrary(Handle);
	}

	jeVFile_DestroyFinder (Finder);
	jeVFile_Close(DosDir);
	return JE_TRUE;
}

//===================================================================================
//	jeEngine_RegisterDriver
//===================================================================================
JETAPI jeBoolean JETCC jeEngine_RegisterDriver(jeEngine *Engine, void* HookProc)
{
	DRV_Hook* DriverHook{};
	DRV_Driver* RDriver{};

	assert( jeEngine_IsValid(Engine) );
	assert(HookProc);

	DriverHook = (DRV_Hook *)HookProc;

	if (!DriverHook(&RDriver))
	{
		jeErrorLog_AddString(-1,"jeEngine_RegisterDriver : Hook proc failed", NULL);
		return JE_FALSE;
	}

	if (RDriver->VersionMajor != DRV_VERSION_MAJOR || RDriver->VersionMinor != DRV_VERSION_MINOR)
	{
		jeErrorLog_AddString(-1,"jeEngine_RegisterDriver : driver wrong vesion", NULL);
		return JE_FALSE;
	}

	Engine->DriverInfo.RDriver = RDriver;	// temporary storage of the RDriver pointer
	Engine->DriverInfo.CurHookProc = DriverHook;
	
	if (!RDriver->EnumSubDrivers(Engine_EnumSubDriversCB, (void*)&Engine->DriverInfo))
	{
		jeErrorLog_AddString(-1,"Engine_EnumSubDrivers : RDriver->EnumSub failed!)", NULL);
		Engine->DriverInfo.RDriver = NULL;		// clear out the RDriver pointer!
		return JE_FALSE;
	}

	Engine->DriverInfo.RDriver = nullptr;		// clear out the RDriver pointer!
	return JE_TRUE;
}

//===================================================================================
//	jeEngine_GetDeviceCaps
//===================================================================================
JETAPI jeBoolean JETCC jeEngine_GetDeviceCaps(jeEngine *pEngine, jeDeviceCaps *DeviceCaps)
{
	memset(DeviceCaps, 0, sizeof(*DeviceCaps));

	if (!pEngine->DriverInfo.RDriver)
		return JE_FALSE;

#if 0
	DeviceCaps->SuggestedDefaultRenderFlags = JE_RENDER_FLAG_BILINEAR_FILTER;
	DeviceCaps->CanChangeRenderFlags = 0xFFFFFFFF;
#else
	pEngine->DriverInfo.RDriver->GetDeviceCaps(DeviceCaps);
#endif

	return JE_TRUE;
	pEngine;
}

//===================================================================================
//	jeEngine_GetDefaultRenderFlags
//===================================================================================
JETAPI jeBoolean JETCC jeEngine_GetDefaultRenderFlags(jeEngine *pEngine, uint32 *RenderFlags)
{
	*RenderFlags = pEngine->DefaultRenderFlags;

	return JE_TRUE;
	pEngine;
}

//===================================================================================
//	jeEngine_SetDefaultRenderFlags
//===================================================================================
JETAPI jeBoolean JETCC jeEngine_SetDefaultRenderFlags(jeEngine *pEngine, uint32 RenderFlags )
{
	pEngine->DefaultRenderFlags = RenderFlags;

	return JE_TRUE;
	pEngine;
}

/*}**** SECTION : EOF *********************/

// Registers an Object given a handle to a DLL - Incarnadine
JETAPI jeBoolean JETCC jeEngine_RegisterObject(HINSTANCE DllHandle)
{
	jeBoolean (*RegisterDef)(float MajorVersion, float MinorVersion);

	assert( DllHandle );

#ifdef WIN32	
	RegisterDef = (jeBoolean (*)(float MajorVersion, float MinorVersion))GetProcAddress( DllHandle, "Object_RegisterDef" );
#endif

#ifdef BUILD_BE
		get_image_symbol(DllHandle, "Object_RegisterDef", B_SYMBOL_TYPE_TEXT, (void **)&RegisterDef);			
#endif

	return( (*RegisterDef)( JET_MAJOR_VERSION, JET_MINOR_VERSION) );
}

// Registers all Objects in a particular path. - Incarnadine
JETAPI jeBoolean JETCC jeEngine_RegisterObjects(char * DllPath)
{
	// locals
	jeVFile* DllDir{};
	jeVFile_Finder* Finder{};
	jeVFile_Properties	Properties{};
	HINSTANCE			DllHandle{};
	char* FullName{};

	// open dll directory
	DllDir = jeVFile_OpenNewSystem(
		NULL,
		JE_VFILE_TYPE_DOS,
		DllPath,
		NULL,
		JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY );
	if ( DllDir != nullptr )
	{

		// create our directory finder
		#ifndef NDEBUG
		Finder = jeVFile_CreateFinder( DllDir, "*.ddl" );
		#else
		Finder = jeVFile_CreateFinder( DllDir, "*.dll" );
		#endif
		
		if( Finder != nullptr )
		{

			// start processing files
			while ( jeVFile_FinderGetNextFile( Finder ) == JE_TRUE )
			{

				// get properties of current file
				if( jeVFile_FinderGetProperties( Finder, &Properties ) == JE_FALSE )
				{
					jeErrorLog_AddString( JE_ERR_FILEIO_READ, "InitObjects: Unable to get dll file properties.", NULL );
					goto ERROR_INITOBJECTS;
				}


				// save dll full name
				FullName = (char*)jeRam_Allocate( strlen( DllPath ) + strlen( Properties.Name ) + 2 );
				if ( FullName == nullptr )
				{
					jeErrorLog_AddString( JE_ERR_MEMORY_RESOURCE, "InitObjects: Unable to allocate dll full name.", NULL );
					goto ERROR_INITOBJECTS;
				}
				strcpy( FullName, DllPath );
				strcat( FullName, "\\" );
				strcat( FullName, Properties.Name );

				// load up the dll
#ifdef WIN32
				DllHandle = LoadLibrary( FullName );
#endif
#ifdef BUILD_BE
				DllHandle = load_add_on( FullName );
#endif

				if ( DllHandle == nullptr )
				{
					jeErrorLog_AddString( JE_ERR_FILEIO_READ, "InitObjects: Unable to load object dll.", Properties.Name );
					jeRam_Free( FullName );
					continue;
				}

				// setup the object functions
				if ( jeEngine_RegisterObject( DllHandle ) == JE_FALSE )
				{
					jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "InitObjects: failed to find get functions for object dll.", Properties.Name  );
					jeRam_Free( FullName );
					continue;
				}
				jeRam_Free( FullName );

			}

			// destroy finder
			jeVFile_DestroyFinder( Finder );
		}

		// close file system
		jeVFile_Close( DllDir );
	}
	return( JE_TRUE );

ERROR_INITOBJECTS:
	jeVFile_DestroyFinder( Finder );
	jeVFile_Close( DllDir );
	return( JE_FALSE );
}

// BSP stats accessor
JETAPI jeBoolean JETCC jeEngine_GetBSPDebugInfo(jeEngine *Engine, int32 *pNumMakeFaces, int32 *pNumMergedFaces, int32 *pNumSubdividedFaces, int32 *pNumDrawFaces)
{
   if (pNumMakeFaces)
	   *pNumMakeFaces = NumMakeFaces;
   if (pNumMergedFaces)
	   *pNumMergedFaces = NumMergedFaces;
   if (pNumSubdividedFaces)
	   *pNumSubdividedFaces = NumSubdividedFaces;
   if (pNumDrawFaces)
	   *pNumDrawFaces = NumMakeFaces+NumSubdividedFaces-NumMergedFaces;

	return JE_TRUE;
}

// Engine render mode
JETAPI jeBoolean JETCC jeEngine_SetRenderMode(jeEngine *Engine, int32 RenderMode)
{
   if (Engine) {
      Engine->RenderMode = RenderMode;
      return JE_TRUE;
   }
   return JE_FALSE;
}

// BEGIN - Bug Fix - jeEngine_FillRect() not implemented - paradoxnj
JETAPI void JETCC jeEngine_FillRect(jeEngine *Engine, const jeRect *Rect, const jeRGBA *Color)
{
	jeTLVertex		DrvVertex[4];
	DRV_Driver* RDriver{};

	RDriver = Engine->DriverInfo.RDriver;

	assert(RDriver != NULL);

#define NEARZ								0.5f

	DrvVertex[0].x = (float)Rect->Left;
	DrvVertex[0].y = (float)Rect->Top;
	DrvVertex[0].z = NEARZ;
	DrvVertex[0].u = 0.0f;
	DrvVertex[0].v = 0.0f;
	DrvVertex[0].r = Color->r;
	DrvVertex[0].g = Color->g;
	DrvVertex[0].b = Color->b;
	DrvVertex[0].a = Color->a;
	
	DrvVertex[1].x = (float)Rect->Right;
	DrvVertex[1].y = (float)Rect->Top;
	DrvVertex[1].z = NEARZ;
	DrvVertex[1].u = 0.0f;
	DrvVertex[1].v = 0.0f;
	DrvVertex[1].r = Color->r;
	DrvVertex[1].g = Color->g;
	DrvVertex[1].b = Color->b;
	DrvVertex[1].a = Color->a;

	DrvVertex[2].x = (float)Rect->Right;
	DrvVertex[2].y = (float)Rect->Bottom;
	DrvVertex[2].z = NEARZ;
	DrvVertex[2].u = 0.0f;
	DrvVertex[2].v = 0.0f;
	DrvVertex[2].r = Color->r;
	DrvVertex[2].g = Color->g;
	DrvVertex[2].b = Color->b;
	DrvVertex[2].a = Color->a;

	DrvVertex[3].x = (float)Rect->Left;
	DrvVertex[3].y = (float)Rect->Bottom;
	DrvVertex[3].z = NEARZ;
	DrvVertex[3].u = 0.0f;
	DrvVertex[3].v = 0.0f;
	DrvVertex[3].r = Color->r;
	DrvVertex[3].g = Color->g;
	DrvVertex[3].b = Color->b;
	DrvVertex[3].a = Color->a;

	if (Color->a != 255.0f)
		RDriver->RenderGouraudPoly(DrvVertex, 4, JE_RENDER_FLAG_FLUSHBATCH);
	else
		RDriver->RenderGouraudPoly(DrvVertex, 4, JE_RENDER_FLAG_ALPHA | JE_RENDER_FLAG_FLUSHBATCH);
}
// END - Bug Fix - jeEngine_FillRect() not implemented - paradoxnj

// BEGIN - Hardware T&L - paradoxnj 6/8/2005
JETAPI jeBoolean JETCC jeEngine_SetMatrix(jeEngine *Engine, uint32 Type, jeXForm3d *Matrix)
{
	if (Engine->DriverInfo.RDriver->SetMatrix)
		return Engine->DriverInfo.RDriver->SetMatrix(Type, Matrix);

	return JE_FALSE;
}

JETAPI jeBoolean JETCC jeEngine_GetMatrix(jeEngine *Engine, uint32 Type, jeXForm3d *Matrix)
{
	if (Engine->DriverInfo.RDriver->GetMatrix)
		return Engine->DriverInfo.RDriver->GetMatrix(Type, Matrix);

	return JE_FALSE;
}

JETAPI jeBoolean JETCC jeEngine_SetCamera(jeEngine *Engine, jeCamera *Camera)
{
	if (Engine->DriverInfo.RDriver->SetCamera)
		return Engine->DriverInfo.RDriver->SetCamera(Camera);

	return JE_FALSE;
}
// END - Hardware T&L - paradoxnj 6/8/2005

JETAPI jeFont * JETCC jeEngine_CreateFont(jeEngine *Engine, int32 Height, int32 Width, uint32 Weight, jeBoolean Italic, const char *facename)
{
	assert(Engine != NULL);
	
	if (Engine->DriverInfo.RDriver->Font_Create)
		return Engine->DriverInfo.RDriver->Font_Create(Height, Width, Weight, Italic, facename);

	return NULL;
}

JETAPI jeBoolean JETCC jeEngine_DestroyFont(jeEngine *Engine, jeFont **Font)
{
	assert(Engine != NULL);
	assert(Font != NULL);

	if (Engine->DriverInfo.RDriver->Font_Destroy)
		return Engine->DriverInfo.RDriver->Font_Destroy(Font);

	return JE_FALSE;
}
