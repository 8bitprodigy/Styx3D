/****************************************************************************************/
/*  System.c                                                                            */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: Friend of engine.c.  Takes care of some of the driver work.            */
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*Genesis3D Version 1.1 released November 15, 1999                            */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#include <assert.h>

#include <SDL2/SDL.h>

#include "BaseType.h"
#include "BitmapList.h"
#include "Camera.h"
#include "DCommon.h"
#include "Entities.h"
#include "ErrorLog.h"
#include "GEAssert.h"
#include "Genesis.h"
#include "Engine.h"
#include "Light.h"
#include "List.h"
#include "Plane.h"
#include "RAM.h"
#include "Sound.h"
#include "Surface.h"
#include "System.h"
#include "User.h"
#include "WBitmap.h"
#include "World.h"
#include "XPlatUtils.h"


#define DBG_OUT( Text, ... ) _DBG_OUT("System.c:" Text, ##__VA_ARGS__ )
//#define SKY_HACK
//extern BOOL GlobalReset;

//=====================================================================================
//	Local static globals
//=====================================================================================
static char DriverFileNames[][200] = 
{
	{"GlideDrv.dll"},
	{"D3DDrv.dll"},
	{"SoftDrv.dll"},
	{"SoftDrv2.dll"},
	{""}
};

//=====================================================================================
//	local static function prototypes
//=====================================================================================

static geBoolean EnumSubDrivers(  Sys_DriverInfo *DriverInfo, const char *DriverDirectory);
static geBoolean EnumSubDriversCB(S32             DriverId,         char *Name,             void *Context);
static geBoolean EnumModesCB(     S32             ModeId,           char *Name,             S32   Width,    S32 Height, void *Context);

//=====================================================================================
//	geDriver_SystemGetNextDriver
//=====================================================================================
GENESISAPI geDriver *
geDriver_SystemGetNextDriver(geDriver_System *DriverSystem, geDriver *Start)
{
	Sys_DriverInfo	*DriverInfo;
	geDriver		*Last;

	assert(DriverSystem != NULL);
	
	DriverInfo = (Sys_DriverInfo*)DriverSystem;

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
//	geDriver_GetNextMode
//=====================================================================================
GENESISAPI geDriver_Mode *
geDriver_GetNextMode(geDriver *Driver, geDriver_Mode *Start)
{
	geDriver_Mode	*Last;

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
//	geDriver_GetName
//=====================================================================================
GENESISAPI geBoolean 
geDriver_GetName(geDriver *Driver, const char **Name)
{
	assert(Driver);
	assert(Name);

	*Name = Driver->Name;

	return GE_TRUE;
}

//=====================================================================================
//	geDriver_ModeGetName
//=====================================================================================
GENESISAPI geBoolean 
geDriver_ModeGetName(geDriver_Mode *Mode, const char **Name)
{
	assert(Mode);
	assert(Name);

	*Name = Mode->Name;

	return GE_TRUE;
}

//=====================================================================================
//	geDriver_ModeGetWidthHeight
//=====================================================================================
GENESISAPI geBoolean 
geDriver_ModeGetWidthHeight(geDriver_Mode *Mode, int32 *Width, int32 *Height)
{
	assert(Mode);
	assert(Width);
	assert(Height);

	*Width = Mode->Width;
	*Height = Mode->Height;

	return GE_TRUE;
}

//=====================================================================================
//	Sys_EngineCreate
//	<> geEngine_Create
//=====================================================================================

const uint32 geEngine_Version = GE_VERSION;
const uint32 geEngine_Version_OldestSupported = 
	( (GE_VERSION_MAJOR << GE_VERSION_MAJOR_SHIFT) + GE_VERSION_MINOR_MIN );

geEngine *
Sys_EngineCreate(SDL_Window *hWnd, const char *AppName, const char *DriverDirectory, uint32 Version)
{
	int32     i;
	geEngine *NewEngine;
	int       Length;

	if (( Version & GE_VERSION_MAJOR_MASK ) != ( geEngine_Version & GE_VERSION_MAJOR_MASK )) {
		geErrorLog_AddString(-1,"Genesis Engine has wrong major version!", NULL);
		return NULL;
	}
	DBG_OUT("Sys_EngineCreate()\tPassed Version Check 1.");
	
	if ( Version > geEngine_Version ) {
		char str[1024];
		sprintf(str,"%d - %d",Version,geEngine_Version);
		geErrorLog_AddString(-1,"Genesis Engine is older than application; aborting!", str);
		return NULL;
	}
	DBG_OUT("Sys_EngineCreate()\tPassed Version Check 2.");
	
	if ( Version < geEngine_Version_OldestSupported ) {
		char str[1024];
		sprintf(str,"%d - %d",Version,geEngine_Version);
		geErrorLog_AddString(-1,"Genesis Engine does not support the old version!", str);
		return NULL;
	}
	DBG_OUT("Sys_EngineCreate()\tPassed Version Check 3.");

	//	Attempt to create a new engine object
	NewEngine = GE_RAM_ALLOCATE_STRUCT(geEngine);

	if (!NewEngine) {
		geErrorLog_Add(GE_ERR_OUT_OF_MEMORY, NULL);
		goto ExitWithError;
	}
	DBG_OUT("Sys_EngineCreate()\tNew Engine allocated.");
	
	// Clear the engine structure...
	memset(NewEngine, 0, sizeof(geEngine));

	if ( ! List_Start() ) {
		geErrorLog_Add(GE_ERR_OUT_OF_MEMORY, NULL);
		DBG_OUT("Sys_EngineCreate()\tList_Start() failed.");
		goto ExitWithError;
	}	

	Length = strlen(DriverDirectory) + 1;
	NewEngine->DriverDirectory = geRam_Allocate(Length);

	if (!NewEngine->DriverDirectory) goto ExitWithError;
	DBG_OUT("Sys_EngineCreate()\tNewEngine->DriverDirectory allocated.");

	memcpy(NewEngine->DriverDirectory, DriverDirectory, Length);
	
	NewEngine->hWnd = hWnd;
	strcpy(NewEngine->AppName, AppName);

	// Get cpu info
	if (!Sys_GetCPUFreq(&NewEngine->CPUInfo)) goto ExitWithError;
	DBG_OUT("Sys_EngineCreate()\tNewEngine->CPUInfo set.");
	
	// Build the wavetable
	for (i = 0; i < 20; i++)
		NewEngine->WaveTable[i] = ((i * 65)%200) + 50;

	if (!EnumSubDrivers(&NewEngine->DriverInfo, DriverDirectory)) goto ExitWithError;
	DBG_OUT("Sys_EngineCreate()\tSubDrivers Enumerated.");

	if (!geEngine_BitmapListInit(NewEngine)) goto ExitWithError;
	DBG_OUT("Sys_EngineCreate()\tBitmapList initialized.");

	if (!Light_EngineInit(NewEngine)) goto ExitWithError;
	DBG_OUT("Sys_EngineCreate()\tLighting initialized.");

	if (!User_EngineInit(NewEngine)) goto ExitWithError;
	DBG_OUT("Sys_EngineCreate()\tUser initialized.");

	if (!geEngine_InitFonts(NewEngine))		// must be after BitmapList
		goto ExitWithError;
	DBG_OUT("Sys_EngineCreate()\tFonts initialized.");

	NewEngine->Changed = GE_TRUE;			// Force a first time driver upload

	NewEngine->DisplayFrameRateCounter = GE_TRUE;	// Default to showing the FPS counter

	geAssert_SetCriticalShutdownCallback( geEngine_ShutdownDriver , NewEngine );
	
	NewEngine->CurrentGamma = 1.0f;

	//geEngine_SetFogEnable(NewEngine, GE_TRUE, 255.0f, 0.0f, 0.0f, 250.0f, 1000.0f);
	geEngine_SetFogEnable(NewEngine, GE_FALSE, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	DBG_OUT("Sys_EngineCreate()\tFog set.");

	return NewEngine;

	// Error cleanup
	ExitWithError:
	{
		DBG_OUT("Sys_EngineCreate()\tExited with Error.");
		if (NewEngine) {
			if (NewEngine->DriverDirectory) geRam_Free(NewEngine->DriverDirectory);

			geRam_Free(NewEngine);
		}

		return NULL;
	}
}


//=====================================================================================
//	Sys_EngineFree
//	<> geEngine_Destroy()
//=====================================================================================
void 
Sys_EngineFree(geEngine *Engine)
{
	geBoolean		Ret;

	assert(Engine != NULL);

	if (!Engine)
		return;

	Ret = geEngine_RemoveAllWorlds(Engine);
	assert(Ret);

	// Call upon modules to free allocated data in the engine
	Light_EngineShutdown(Engine);
	User_EngineShutdown(Engine);

	Ret = geEngine_ShutdownFonts(Engine);
	assert(Ret == GE_TRUE);

	Ret = geEngine_ShutdownDriver(Engine);
	assert(Ret == GE_TRUE);

	Ret = geEngine_BitmapListShutdown(Engine);
	assert(Ret == GE_TRUE);

	geRam_Free(Engine->DriverDirectory);

	List_Stop();

	geRam_Free(Engine);
}

//=====================================================================================
//	SysGetCPUFreq
//=====================================================================================
geBoolean 
Sys_GetCPUFreq(Sys_CPUInfo *Info)
{
	int64 Freq;

	assert(Info != NULL);

	if (!(Freq = SDL_GetPerformanceFrequency(), Freq))
	{
		geErrorLog_Add(GE_ERR_NO_PERF_FREQ, NULL);
		return GE_FALSE;
	}

	Info->Freq = Freq;

	return GE_TRUE;
}

#ifdef	MESHES
//===================================================================================
//	Sys_WorldCreateMesh
//	Create a mesh definition object
//===================================================================================
Mesh_MeshDef *
Sys_WorldCreateMesh(geWorld *World, const char *BitmapPath, const char *FileName)
{
	Mesh_MeshDef	*MeshDef;

	assert(World != NULL);
	assert(BitmapPath != NULL);
	assert(FileName != NULL);

	MeshDef = Mesh_WorldCreateMesh(World, BitmapPath, FileName);

	return MeshDef;
}

//===================================================================================
//	Sys_WorldFreeMesh
//===================================================================================
void 
Sys_WorldFreeMesh(geWorld *World, Mesh_MeshDef *MeshDef)
{
	assert(World != NULL);
	assert(MeshDef != NULL);

	Mesh_WorldFreeMesh(World, MeshDef);
}
#endif

//===================================================================================
//	EnumSubDriversCB
//===================================================================================
static geBoolean 
EnumSubDriversCB(S32 DriverId, char *Name, void *Context)
{
	Sys_DriverInfo *DriverInfo = (Sys_DriverInfo*)Context;
	DRV_Driver     *RDriver;
	geDriver       *Driver;

	if (strlen(Name) >=	DRV_STR_SIZE)
		return GE_TRUE;		// Ignore
	
	if (DriverInfo->NumSubDrivers+1 >= MAX_SUB_DRIVERS)
		return GE_FALSE;		// Stop when no more driver slots available

	Driver = &DriverInfo->SubDrivers[DriverInfo->NumSubDrivers];
	
	Driver->Id = DriverId;
	strcpy(Driver->Name, Name);
	strcpy(Driver->FileName, DriverInfo->CurFileName);

	RDriver = DriverInfo->RDriver;

	// Store this, so enum modes know what driver we are working on...
	DriverInfo->CurDriver = Driver;
	
	if (!RDriver->EnumModes(Driver->Id, Driver->Name, EnumModesCB, (void*)DriverInfo))
		return GE_FALSE;

	DriverInfo->NumSubDrivers++;

	return GE_TRUE;
}

//===================================================================================
//	EnumModesCB
//===================================================================================
static geBoolean 
EnumModesCB(S32 ModeId, char *Name, S32 Width, S32 Height, void *Context)
{
	Sys_DriverInfo	*DriverInfo;
	geDriver		*Driver;
	geDriver_Mode	*Mode;

	if (strlen(Name) >=	DRV_MODE_STR_SIZE)
		return GE_TRUE;		// Ignore

	DriverInfo = (Sys_DriverInfo*)Context;

	Driver = DriverInfo->CurDriver;
	
	if (Driver->NumModes+1 >= MAX_DRIVER_MODES)
		return GE_FALSE;

	Mode = &Driver->Modes[Driver->NumModes];

	Mode->Id = ModeId;
	strcpy(Mode->Name, Name);
	Mode->Width = Width;
	Mode->Height = Height;

	Driver->NumModes++;

	return GE_TRUE;
}

//===================================================================================
//	EnumSubDrivers
//===================================================================================
static geBoolean 
EnumSubDrivers(Sys_DriverInfo *DriverInfo, const char *DriverDirectory)
{
	int32		i;
	DRV_Hook	*DriverHook;
	HINSTANCE	Handle;
	DRV_Driver	*RDriver;
	geBoolean	GlideFound;

	DriverInfo->NumSubDrivers = 0;

	GlideFound = GE_FALSE;

	for (i=0; DriverFileNames[i][0]!=0; i++)
	{
		if (!strcmp(DriverFileNames[i], "D3DDrv.dll") && GlideFound)
			continue;			// Skip D3D if we found a glidedrv

		Handle = geEngine_LoadLibrary(DriverFileNames[i], DriverDirectory);

		if (!Handle)
			continue;

		DriverInfo->CurFileName = DriverFileNames[i];

		DriverHook = (DRV_Hook*)SDL_LoadFunction(Handle, "DriverHook");

		if (!DriverHook)
		{
			SDL_UnloadObject(Handle);
			continue;
		}

		if (!DriverHook(&RDriver))
		{
			SDL_UnloadObject(Handle);
			continue;
		}

		if (RDriver->VersionMajor != DRV_VERSION_MAJOR || RDriver->VersionMinor != DRV_VERSION_MINOR)
		{
			SDL_UnloadObject(Handle);
			geErrorLog_AddString(-1,"EnumSubDrivers : found driver of different version; ignoring; non-fatal",DriverFileNames[i]);
			continue;
		}

		DriverInfo->RDriver = RDriver;
		
		if (!RDriver->EnumSubDrivers(EnumSubDriversCB, (void*)DriverInfo))
		{
			SDL_UnloadObject(Handle);
			continue;		// Should we return FALSE, or just continue?
		}

		SDL_UnloadObject(Handle);

		if (!strcmp(DriverFileNames[i], "GlideDrv.dll"))
			GlideFound = GE_TRUE;
	}

	DriverInfo->RDriver = NULL;	// Reset this

	return GE_TRUE;
}


