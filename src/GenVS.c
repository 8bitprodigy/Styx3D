/****************************************************************************************/
/*  GenVS.c                                                                             */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description:                                                                        */
/*                                                                                      */
/*  Copyright (c) 1999 WildTangent, Inc.; All rights reserved.               */
/*                                                                                      */
/*  See the accompanying file LICENSE.TXT for terms on the use of this library.         */
/*  This library is distributed in the hope that it will be useful but WITHOUT          */
/*  ANY WARRANTY OF ANY KIND and without any implied warranty of MERCHANTABILITY        */
/*  or FITNESS FOR ANY PURPOSE.  Refer to LICENSE.TXT for more details.                 */
/*                                                                                      */
/****************************************************************************************/
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>	//_chdir()
#include <sys/time.h>

#include <SDL2/SDL.h>

#ifdef _WIN32
	#include <direct.h>	
	#include <dos.h>
	#include <mmsystem.H>
	#include <windows.H>
	#define chdir _chdir
#else
	#include <limits.h>
	#include <unistd.h>
	
	#define _MAX_PATH PATH_MAX
	#define MAX_PATH PATH_MAX
	#define CALLBACK
	#define WINAPI
	#define WinMain main
	typedef void* LRESULT;
	typedef void* WPARAM;
	typedef void* LPARAM;
	typedef union
	{
		struct {
			uint32_t LowPart;
			int32_t HighPart;
		};
		struct {
			uint32_t LowPart;
			int32_t HighPart;
		} u;
		int64_t QuadPart;
	}
	LARGE_INTEGER;
#endif

#include "AutoSelect.h"
#include "GETypes.h"
#include "Bitmap.h"
#include "Client.h"
#include "Console.h"
#include "DrvList.h"
#include "ErrorLog.h"
#include "GameMgr.h"
#include "Genesis.h"
#include "GMenu.h"
#include "Host.h"
#include "Menu.h"
#include "NetMgr.h"
#include "Text.h"
#include "XPlatUtils.h"




#define CLIP_CURSOR
//#define DO_CAPTURE

void	      	GenVS_Error(const char *Msg, ...);

LRESULT CALLBACK  WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

static char      *GetCommandText(char *CmdLine, char *Data, geBoolean Cmd);

void              ShutdownAll(void);

static geBoolean  NewKeyDown(int KeyCode);
static void       GetMouseInput(SDL_Window *hWnd, int Width, int Height);

#define STARTING_WIDTH  (500)
#define STARTING_HEIGHT (400)

// Misc global info
geBoolean				PopupD3DLog = false;
geBoolean				Running     = true;
geBoolean				GameRunning = false;
geBoolean				ResetMouse  = true;
char					AppName[256] = "Genesis3D Sample Game";

#define DEFAULT_LEVEL	"Levels\\GenVS.BSP"

geFloat					EffectScale;
geBoolean				ChangingDisplayMode = false;	
int						ChangeDisplaySelection;

// Game Manager
static GameMgr			*GMgr = NULL;

//	Genesis Objects
geEngine				*Engine = NULL;

// Misc objects
static Console_Console	*Console = NULL;
static Host_Host		*Host = NULL;

geSound_System	*SoundSys = NULL;

static char				PlayerName[MENU_MAXSTRINGSIZE];
static char				IPAddress[MENU_MAXSTRINGSIZE];

float					MainTime;

static geBoolean		MenusCreated;
ModeList			   *DriverModeList;
int						DriverModeListLength;

geBoolean				g_FogEnable = false;
geBoolean				g_FarClipPlaneEnable = false;

//=====================================================================================
//	GetCommandText
//=====================================================================================
static char *GetCommandText(char *CmdLine, char *Data, geBoolean Cmd)
{
	geBoolean	Quote;
	int32		dp;
	char		ch;

	while (1)
	{
		ch = *CmdLine;
		
		if (ch == 0 || ch == '\n')
			return NULL;

		if (ch == ' ')				// Skip white space
		{
			CmdLine++;
			continue;
		}

		if (ch == '-')
		{
			if (!Cmd)
				return NULL;		// FIXME:  Return error!!!
			CmdLine++;
			break;
		}
		else 
		{
			if (Cmd)
				return NULL;			// FIXME:  Return error!!!
			break;
		}

		CmdLine++;
	}

	if (ch == 0 || ch == '\n')
		return NULL;

	if (ch == '"')
	{
		Quote = true;
		ch = *++CmdLine;
	}
	else
		Quote = false;

	dp = 0;

	while (1)
	{
		if (Quote)
		{
			if (ch == '"')
			{
				break;
			}
			
			if (ch == '\n' || ch == 0)
				return NULL;		// Quote not found!!!
		}
		else
		{
			if (ch == ' ' || ch == '\n' || ch == 0)
				break;
		}
		
		ch = *CmdLine++;
		Data[dp++] = ch;
	}

	Data[dp-1] = 0;

	return CmdLine;
		
}

void AdjustPriority(int adjustment)
{
	//HANDLE thread = GetCurrentThread();
	//SetThreadPriority(thread,adjustment);
}


float	GlobalMouseSpeedX;
float	GlobalMouseSpeedY;
uint32	GlobalMouseFlags;	

static void 
SubLarge(LARGE_INTEGER *start, LARGE_INTEGER *end, LARGE_INTEGER *delta)
{/*
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
	}*/
	delta->QuadPart = end->QuadPart - start->QuadPart;
}

Host_Init		HostInit;
geBoolean		ShowStats,Mute;

geVFile *			MainFS;

//=====================================================================================
//	NewKeyDown
//=====================================================================================
static geBoolean 
NewKeyDown(int KeyCode) 
{
    const uint8 *keyState = SDL_GetKeyboardState(NULL);
    return (bool)keyState[SDL_GetScancodeFromKey(KeyCode)];
}

static void 
PickMode(
	SDL_Window *hwnd, 
	geBoolean   NoSelection, 
	geBoolean   ManualSelection, 
	ModeList   *List, 
	int         ModeListLength, 
	int        *Selection
);


//=====================================================================================
//	WinMain
//=====================================================================================
/*
#pragma warning (disable: 4028)
int WINAPI 
WinMain(
	HANDLE hInstance, 
	HANDLE hPrevInstance,
	LPSTR lpszCmdParam, 
	int nCmdShow
)
*/
int 
main(int argc, char *argv[]) 
{
	//MSG              Msg;
	SDL_Event        event;
    geDriver        *Driver     = NULL;
    geDriver_Mode   *DriverMode = NULL;

    //struct timeval  SystemTime;
    char            *CmdLine = (argc > 1) ? argv[1] : NULL;
    int32           i;
    uint64          
	                Freq, 
	                OldTick, 
	                CurTick;
    char            TempName[256];
    int32           TempNameLength;
    geBoolean       ManualPick = false;

    MainTime = 0.0f;

	//AdjustPriority(THREAD_PRIORITY_NORMAL);
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		return -1;
	}
	
	for (i=0; i<255; i++) {
		NewKeyDown(i);		// Need to flush all the keys
	}
/*
	GetSystemTime(&SystemTime);

#if 0
	if	(SystemTime.wYear > 1999 || SystemTime.wMonth > 11)
	{
		MessageBox(NULL,"The time limit on this demo has expired. \n" 
			"Please contact WildTangent for more information.",
			"Notice",MB_OK | MB_TASKMODAL);
		return 0;
	}
#endif
*/

// set the currrent directory to where the exe is
	{
		int   i;
		char *PathBuf = SDL_GetBasePath();

		// get the exe's path and name
		if (!PathBuf)
		{
			GenVS_Error("Could not get exe file name.");
		}

		// strip off exe name to leave path
		for (i=strlen(PathBuf)-1; i>0; i--)
		{
			if (PathBuf[i]=='\\')
			{
				PathBuf[i]=0;
				break;
			}
		}
		if (i==0)
		{
			GenVS_Error("Could not parse exe's path from exe name.");
		}
		
		// move the current working directory to the executables directory.
		// this is a little rude
		if (chdir(PathBuf)==-1)
		{
			GenVS_Error("Could not change current working directory to exe's path.");
		}
	}
	

	// Setup the appname
	sprintf(AppName, "GTest v1.0 "__DATE__","__TIME__"");

	// Get defaults
	ShowStats = Mute = false;

	strcpy(IPAddress, "");							// Deafult our IP address to blank (LAN game)
	TempNameLength = sizeof(TempName);
	if( geGetUserName( TempName, &TempNameLength ) == 0 )
	{
		strcpy( PlayerName, "Player" );
	}
	else
	{
		strncpy( PlayerName, TempName, sizeof( PlayerName ) );
		PlayerName[ sizeof( PlayerName ) - 1 ] = '\0';
	}


	HostInit.Mode = HOST_MODE_SINGLE_PLAYER;
	HostInit.DemoMode = HOST_DEMO_NONE;
	HostInit.LevelHack[0] = 0;
	HostInit.DemoFile[0] = 0;

	geGetCurrentDir(sizeof(TempName), TempName);
	MainFS = geVFile_OpenNewSystem(NULL,
								   GE_VFILE_TYPE_DOS,
								   TempName,
								   NULL,
								   GE_VFILE_OPEN_READONLY | GE_VFILE_OPEN_DIRECTORY);
	assert(MainFS);
	

	while (1)
	{	
		char			Data[MAX_PATH];

		if (!(CmdLine = GetCommandText(CmdLine, Data, true)))
			break;

		if (!strcmp(Data, "Server"))
		{
			if (!HostInit.LevelHack[0])	
				strcpy(HostInit.LevelHack, DEFAULT_LEVEL);

			HostInit.Mode = HOST_MODE_SERVER_CLIENT;
		}
		else if (!strcmp(Data, "Dedicated"))
		{
			HostInit.Mode = HOST_MODE_SERVER_DEDICATED;
		}
		else if (!strcmp(Data, "Client"))
		{
			HostInit.Mode = HOST_MODE_CLIENT;
		}
		else if (!strcmp(Data, "BotPathDebug"))
		{
			extern geBoolean PathLight; // hacked in
			PathLight = true;
		}
		else if (!strcmp(Data, "Record"))
		{
			HostInit.DemoMode = HOST_DEMO_RECORD;

			if (!(CmdLine = GetCommandText(CmdLine, Data, false)))
				GenVS_Error("No demo name specified on command line.");

			strcpy(HostInit.DemoFile, Data);
		}
		else if (!strcmp(Data, "Play"))
		{
			HostInit.DemoMode = HOST_DEMO_PLAY;

			if (!(CmdLine = GetCommandText(CmdLine, Data, false)))
				GenVS_Error("No demo name specified on command line.");

			strcpy(HostInit.DemoFile, Data);
		}
		else if (!strcmp(Data, "Name"))
		{
			// Get name
			if (!(CmdLine = GetCommandText(CmdLine, Data, false)))
				GenVS_Error("No name specified on command line.");
			
			strcpy(PlayerName, Data);
		}
		else if (!strcmp(Data, "IP"))
		{
			// Get name
			if (!(CmdLine = GetCommandText(CmdLine, Data, false)))
				GenVS_Error("No IP Address specified on command line.");
			
			if (strlen(Data) >= NETMGR_MAX_IP_ADDRESS)
				GenVS_Error("MaxIP Address string on command line.\n");

			strcpy(IPAddress, Data);		// User wants to override IP at command line
		}
		else if (!strcmp(Data, "Map"))
		{
			// Get name
			if (!(CmdLine = GetCommandText(CmdLine, Data, false)))
				GenVS_Error("No map name specified on command line.");
			sprintf(HostInit.LevelHack, "Levels\\%s", Data);
			//sprintf(HostInit.UserLevel, "Levels\\%s", Data);
		}
		else if (!strcmp(Data, "Gamma"))
		{
			// Get name
			if (!(CmdLine = GetCommandText(CmdLine, Data, false)))
				GenVS_Error("No gamma value specified on command line.");
			
			// <>
			UserGamma = (float)atof(Data);
		}
		else if (!strcmp(Data, "ShowStats"))
		{
			ShowStats = true;
		}
		else if (!strcmp(Data, "Mute"))
		{
			Mute = true;
		}
		else if (!strcmp(Data, "VidMode"))
		{
			GenVS_Error("VidMode Parameter no longer supported");
		}
		else if (!strcmp(Data,"PickMode"))
		{
			ManualPick=true;
		}
		else if (!strcmp(Data,"Fog"))
		{
			g_FogEnable = true;
		}
		else if (!strcmp(Data,"FarClip"))
		{
			g_FarClipPlaneEnable = true;
		}
		else
			GenVS_Error("Unknown Option: %s.", Data);
	}


	
	//
	//	Create the Game Mgr
	//
	GMgr = GameMgr_Create(STARTING_WIDTH, STARTING_HEIGHT, AppName);
	
	if (!GMgr)
		GenVS_Error("Could not create the game mgr.");

	Engine = GameMgr_GetEngine(GMgr);
	if (!Engine)
		GenVS_Error("Failed to create the geEngine Object");

	DriverModeList = ModeList_Create(Engine,&DriverModeListLength);
	if (DriverModeList == NULL)
		{
			GenVS_Error("Failed to create a list of available drivers - Make sure the driver dll's are in the right directory.");
		}
	AutoSelect_SortDriverList(DriverModeList, DriverModeListLength);

	geEngine_EnableFrameRateCounter(Engine, ShowStats);

	
	do 
		{
			SDL_Window *hWnd;
			VidMode vidMode;

			// Pick mode
			PickMode(GameMgr_GethWnd(GMgr),ChangingDisplayMode, ManualPick, 
					 DriverModeList, DriverModeListLength, &ChangeDisplaySelection);
			ChangingDisplayMode = 0;		

			// Text, and Menu should go in GameMgr to make the hierarchy clean
			// Client, and Host could then look down on Menu, and grab default items out of it...
			if (!Text_Create( Engine ))
				GenVS_Error("Text_Create failed.");

			if (!GMenu_Create( Engine , DriverModeList, DriverModeListLength, ChangeDisplaySelection))
				GenVS_Error("GMenu_Create failed.");
			
			MenusCreated = true;

			// Set the text in the menus for the name/ipaddress etc...
			{
				Menu_SetStringText( GMenu_GetMenu(GMenu_NameEntry), GMenu_NameEntry, PlayerName );
				Menu_SetStringText( GMenu_GetMenu(GMenu_IPEntry), GMenu_IPEntry, IPAddress );
			}

			// Get the sound system
			SoundSys = GameMgr_GetSoundSystem(GMgr);
			//assert(SoundSys);

			if (SoundSys)
				{
					if ( Mute )
						geSound_SetMasterVolume(SoundSys, 0.0f );
				}

			// Get the console
			Console = GameMgr_GetConsole(GMgr);
			assert(Console);
			
			HostInit.hWnd = GameMgr_GethWnd(GMgr);
			strcpy(HostInit.ClientName, PlayerName);

			if (!GameMgr_ClearBackground(GMgr, 0, 0, NULL))
				GenVS_Error("GameMgr_ClearBackground failed.");

			// If a map has not been set, and not recording a demo, then play demos...
			if (HostInit.DemoMode != HOST_DEMO_RECORD && !HostInit.LevelHack[0] && HostInit.Mode != HOST_MODE_CLIENT)
				HostInit.DemoMode = HOST_DEMO_PLAY;
			else
				GMenu_SetActive(false);			// Menu should start out as non active when recording a demo...

			// If the user wants to load a level at the command prompt, or wants to play back a demo
			// then load a host now, else just let them do it through the menus later...
			if (HostInit.LevelHack[0] || HostInit.DemoMode != HOST_DEMO_NONE || HostInit.Mode == HOST_MODE_CLIENT)
			{
				if (!HostInit.LevelHack[0])	
					strcpy(HostInit.LevelHack, DEFAULT_LEVEL);

				Host = Host_Create(Engine, &HostInit, GMgr, GameMgr_GetVidMode(GMgr));

				if (!Host)
					GenVS_Error("Could not create the host!\n");
			}
			else		// Make sure a default level is set
				strcpy(HostInit.LevelHack, DEFAULT_LEVEL);

		#if 1
			// <>
			geEngine_SetGamma(Engine, UserGamma);
		#endif

			// Setup the fog (if enabled)
			if (g_FogEnable)
				geEngine_SetFogEnable(Engine, true, 200.0f, 10.0f, 10.0f, 200.0f, 1300.0f);
			
			Freq    = SDL_GetPerformanceFrequency();
			OldTick = SDL_GetPerformanceCounter();

			//SetCapture(GameMgr_GethWnd(GMgr));
			SDL_ShowCursor(SDL_DISABLE);
			
			#ifdef CLIP_CURSOR	
			{
				SDL_Rect ClipRect;
				int 
					win_x, 
					win_y, 
					win_w, 
					win_h;

				SDL_GetWindowPosition(GameMgr_GethWnd(GMgr), &win_x, &win_y);
				SDL_GetWindowSize(    GameMgr_GethWnd(GMgr), &win_w, &win_h);

				ClipRect.x = win_x;
				ClipRect.y = win_y;
				ClipRect.w = win_w;
				ClipRect.h = win_h;

				SDL_SetWindowMouseRect(GameMgr_GethWnd(GMgr), &ClipRect);
			}
			#endif
			
			Running = true;
			vidMode = GameMgr_GetVidMode(GMgr);
			hWnd    = GameMgr_GethWnd(GMgr);


			while (Running)
			{
				uint64         DeltaTick;
				float          ElapsedTime;
				geWorld       *World;
				geCamera      *Camera;

				GameRunning =  true;

				// see runtime menu options for stats and video mode changing.  (Mike)

				SDL_ShowCursor(SDL_DISABLE);
				#ifdef DO_CAPTURE
				SDL_SetRelativeMouseMode(true);
				#endif

				// Get timing info
				uint64 CurTick = SDL_GetPerformanceCounter();

				DeltaTick = CurTick - OldTick;
				OldTick = CurTick;

				if (DeltaTick > 0)
					ElapsedTime = 1.0f / ((float)SDL_GetPerformanceFrequency() / (float)DeltaTick);
				else
					ElapsedTime = 0.001f;

				//MainTime += ElapsedTime;

				// Get the mouse input (FIXME:  Move this into client?)
				{
					int Width, Height;
					SDL_GetWindowSize(hWnd, &Width, &Height);
					GetMouseInput(hWnd, Width, Height);
				}

				// Do a host frame (physics, etc)
				if (Host)
				{
					if (!Host_Frame(Host, ElapsedTime))
						GenVS_Error("Host_Frame failed...");
				}

				// Get the world, and the camera from the GameMgr
				World = GameMgr_GetWorld(GMgr);
				Camera = GameMgr_GetCamera(GMgr);;

				if (!GameMgr_Frame(GMgr, ElapsedTime))
					GenVS_Error("GameMgr_Frame failed...");

				// Begin frame
				if (g_FarClipPlaneEnable)		// If we are using a far clip plane, then clear the screen
				{
					if (!GameMgr_BeginFrame(GMgr, World, true))
						GenVS_Error("GameMgr_BeginFrame failed.\n");
				}
				else if (!GameMgr_BeginFrame(GMgr, World, false))
				{
					if (!GameMgr_BeginFrame(GMgr, World, true))
						GenVS_Error("GameMgr_BeginFrame failed.\n");
				}

				if (!Host || !World)
					GameMgr_ClearBackground(GMgr, 0, 0, NULL);

				if (Host)
				{
					if (!Host_RenderFrame(Host, ElapsedTime))
						GenVS_Error("Host_RenderFrame failed in main game loop.\n");
				}

				Console_Frame(Console, ElapsedTime);

				/*
				if (Host)
				{
					if (!World)
						GameMgr_ClearBackground(GMgr, 0, 0, NULL);

					if (Host_RenderFrame(Host, ElapsedTime)==false)
						GenVS_Error("Host_RenderFrame failed in main game loop.\n");

					Console_Frame(Console, ElapsedTime);
				}
				else
					GameMgr_ClearBackground(GMgr, 0, 0, NULL);
				*/

				if (!ShowStats && World)
				{
					Console_XYPrintf(Console,0,0,0,"Driver: %s %s",
						DriverModeList[ChangeDisplaySelection].DriverNamePtr,
						DriverModeList[ChangeDisplaySelection].ModeNamePtr);
					
					if (!SoundSys)
					{
						Console_XYPrintf(Console,0,1,0,"(No sound device found)");
					}
				}
				
				// Draw manu
				GMenu_Draw();

				//	End Engine frame
				if (!GameMgr_EndFrame(GMgr))
				{
					GenVS_Error("GameMgr_EndFrame failed.\n");
				}

				// Do the'ol message pump
				/*while (PeekMessage( &Msg, NULL, 0, 0, PM_NOREMOVE))
				{
					if (!GetMessage(&Msg, NULL, 0, 0 ))
					{
						PostQuitMessage(0);
						Running=0;
						break;
					}

					TranslateMessage(&Msg); 
					DispatchMessage(&Msg);
					if (ChangingDisplayMode)
						{
							break;
						}
				} */
				while (SDL_PollEvent(&event)) {
					switch (event.type) {
					case SDL_QUIT:
						Running = 0;
						break;
						
					case SDL_KEYDOWN:
						// Handle key presses
						break;
						
					case SDL_KEYUP:
						// Handle key releases
						break;
						
					case SDL_WINDOWEVENT:
						if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
							ChangingDisplayMode = true;
							break;
						}
					}
					
					if (ChangingDisplayMode) {
						break;
					}
				}
				

				if (ChangingDisplayMode)
					{
						Running = 0;		
						GameRunning	= false;
					}
			} 
			
				if (ChangingDisplayMode)
					{
						if (MenusCreated)
							{
								Text_Destroy();
								GMenu_Destroy();
								MenusCreated = false;
							}

						if (Host)
							{
								Host_Destroy(Host);
								Host = NULL;
							}
						
						if ( SDL_GetRelativeMouseMode() )
							{
								SDL_SetRelativeMouseMode(false);
							}
						SDL_ShowCursor(SDL_ENABLE);
					}

			
		}
	while (ChangingDisplayMode != false);

	//ShutdownAll();
	SDL_Quit();
	return (0);
}


static void 
PickMode(
	SDL_Window *hwnd, 
	geBoolean   NoSelection, 
	geBoolean   ManualSelection, 
	ModeList   *List, 
	int         ListLength, 
	int        *ListSelection
)
{

	assert( hwnd != 0 );
	assert( List != NULL );
	assert( ListSelection != NULL );

	GameMgr_PrepareToChangeMode(GMgr);

	if (!NoSelection && !ManualSelection)
			{
				if (AutoSelect_PickDriver(GameMgr_GethWnd(GMgr),Engine,List, ListLength, ListSelection)==false)
					{
						geErrorLog_AddString(-1,"Automatic video mode selection failed to find good mode.  Trying manual selection.",NULL);
						ManualSelection = true;
					}
			}

	
	if (NoSelection || ManualSelection)
		{
			while (1)
				{
					if (!NoSelection)
						{
							if (DrvList_PickDriver(GameMgr_GethWnd(GMgr), List, ListLength, ListSelection)==false)
								{
									geErrorLog_AddString(-1,"Driver pick dialog failed",NULL);
									ShutdownAll();
									exit(1);
								}
						}
					NoSelection = false;

					if ( *ListSelection < 0 )	// no selection made
						{
							ShutdownAll();
							exit(1);
						}

					geEngine_ShutdownDriver(Engine);

					if(List[*ListSelection].InAWindow)
						{
							GameMgr_ResetMainWindow(hwnd,List[*ListSelection].Width,List[*ListSelection].Height);
						}
					if ( (List[*ListSelection].DriverType == MODELIST_TYPE_D3D_PRIMARY)   ||
						 (List[*ListSelection].DriverType == MODELIST_TYPE_D3D_SECONDARY) ||
						 (List[*ListSelection].DriverType == MODELIST_TYPE_D3D_3DFX) )
						{
							PopupD3DLog = true;
						}
					else
						{
							PopupD3DLog = false;
						}
					if (!geEngine_SetDriverAndMode(Engine, List[*ListSelection].Driver, List[*ListSelection].Mode))
						{
							if ( SDL_GetRelativeMouseMode() )  // just in case
								{
									SDL_SetRelativeMouseMode(false);
								}
							GameMgr_ResetMainWindow(hwnd,STARTING_WIDTH,STARTING_HEIGHT);
							geErrorLog_AddString(-1, "geEngine_SetDriverAndMode failed. (continuing)", NULL);
							SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Styx Driver Error", "Driver failed to properly set the selected mode.", GameMgr_GethWnd(GMgr));
						}
					else
						{ 
							break;
						}
				}
		}



	// Set the driver and the mode through the game manager...
	if (!GameMgr_SetDriverAndMode(GMgr, List[*ListSelection].Driver, List[*ListSelection].Mode, 
										List[*ListSelection].Width,  List[*ListSelection].Height))
		GenVS_Error("GameMgr_SetDriverAndMode failed.");
	

	EffectScale	= 0.3f;	// this is always 0.3 ?

}

//=====================================================================================
//	ShutdownAll
//=====================================================================================
void ShutdownAll(void)
{
	// Free each object (sub objects are freed by their parents...)
	if (MenusCreated)
	{
		Text_Destroy();
		GMenu_Destroy();
		MenusCreated = false;
	}

	if (Host)
	{
		Host_Destroy(Host);
		Host = NULL;
	}

	if (GMgr)
		{
			SDL_Window *hWnd = GameMgr_GethWnd(GMgr);
			if (hWnd)
				{
					SDL_SetWindowGrab(hWnd, false);
					SDL_ShowCursor(SDL_ENABLE);
					SDL_HideWindow(hWnd);
					SDL_UpdateWindowSurface(hWnd);
					
					#ifdef CLIP_CURSOR	
						SDL_SetWindowGrab(hWnd, false);
					#endif
				}
		}

	if (GMgr)
		GameMgr_Destroy(GMgr);
	
	geVFile_Close(MainFS);

	GMgr     = NULL;
	Engine   = NULL;
	Console  = NULL;
	SoundSys = NULL;
	Host     = NULL;

	#ifdef _DEBUG
	{
		char	Str[1024];
		uint32	Count;

		Count = geBitmap_Debug_GetCount();

		sprintf(Str, "Final Bitmap count: %i\n", Count);
		OutputDebugString(Str);

		MessageBox(NULL, Str,
			"GenVS MSG",MB_OK | MB_TASKMODAL);

	}
	#endif


}

extern uint32	GlobalButtonBits;

//=====================================================================================
//	WndProc
//=====================================================================================
/* Is this even used for anything? Let's find out!
LRESULT CALLBACK 
WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage)
	{
		case WM_SYSCOMMAND:
			if (wParam == SC_SCREENSAVE)
				return 1;
			break;
		case WM_DISPLAYCHANGE:
			{
				if (DriverModeList)
					{
						if (DriverModeList[ChangeDisplaySelection].InAWindow)
							ChangingDisplayMode = true;
					}
				break;
			}
		case WM_ACTIVATEAPP:
		{
			if(Engine)
			{
				geEngine_Activate(Engine, wParam);
			}

			if(GameRunning)
			{

			#ifdef CLIP_CURSOR	
				if(wParam && GMgr)
				{
					RECT	ClipRect;
					RECT	ClientRect;
					POINT	RPoint;
						  
					GetClientRect(GameMgr_GethWnd(GMgr), &ClientRect);
					RPoint.x		=ClientRect.left;
					RPoint.y		=ClientRect.top;
					ClientToScreen(GameMgr_GethWnd(GMgr), &RPoint);
					ClipRect.left	=RPoint.x;
					ClipRect.top	=RPoint.y;

					RPoint.x		=ClientRect.right;
					RPoint.y		=ClientRect.bottom;
					ClientToScreen(GameMgr_GethWnd(GMgr), &RPoint);
					ClipRect.right	=RPoint.x;
					ClipRect.bottom	=RPoint.y;
					ClipCursor(&ClipRect);
					ResetMouse = true;
				}
				else
				{
					ResetMouse = false;
					ClipCursor(NULL);
				}
			#endif
			}
			if ( wParam )
				{
					AdjustPriority(THREAD_PRIORITY_HIGHEST);
				}
			else
				{
					AdjustPriority(THREAD_PRIORITY_NORMAL);
				}
			return	0;
		}
		case WM_MOVE:
		{
			if(GameRunning)
			{
				geRect ClipRect;
				RECT   ClientRect;
				POINT  RPoint;
				bool   Ret;

				GetClientRect(GameMgr_GethWnd(GMgr), &ClientRect);
				RPoint.x		=ClientRect.left;
				RPoint.y		=ClientRect.top;
				ClientToScreen(GameMgr_GethWnd(GMgr), &RPoint);
				ClipRect.Left	=RPoint.x;
				ClipRect.Top	=RPoint.y;

				RPoint.x		=ClientRect.right;
				RPoint.y		=ClientRect.bottom;
				ClientToScreen(GameMgr_GethWnd(GMgr), &RPoint);
				ClipRect.Right	=RPoint.x;
				ClipRect.Bottom	=RPoint.y;
				Ret = geEngine_UpdateWindow(Engine);

				assert(Ret == true);
			}
			return	0;
		}
		case WM_MOUSEMOVE:
		{
			return FALSE;
			break;
		}

		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONDOWN:
		{
			switch ( iMessage )
			{
				case WM_LBUTTONDOWN:
					GlobalMouseFlags |= 1;
					lParam = VK_LBUTTON;
					break;
				case WM_RBUTTONDOWN:
					GlobalMouseFlags |= 2;
					lParam = VK_RBUTTON;
					break;
				case WM_LBUTTONUP:
					GlobalMouseFlags &= ~1;
					lParam = VK_LBUTTON;
					break;
				case WM_RBUTTONUP:
					GlobalMouseFlags &= ~2;
					lParam = VK_RBUTTON;
					break;
			}

			// intentionally falls through if required
			if ( GMenu_IsAMenuActive() == false )
			{
				break;
			}
		}

		case (WM_KEYDOWN):
		{
			// locals
			int32			Result;

			// process keystroke result //undone
			Result = GMenu_Key( wParam, lParam );

			switch ( Result )
			{
				
				case GMenu_DoNothing:
				{
					break;
				}

				case GMenu_UserSinglePlayerGame:
				case GMenu_SinglePlayerGame:
				case GMenu_SinglePlayerGame1:
				case GMenu_SinglePlayerGame2:
				case GMenu_SinglePlayerGame3:
				case GMenu_StartGame:
				case GMenu_StartGame1:
				case GMenu_StartGame2:
				case GMenu_Connect:
				{
					char	TempString[64];

					// Init the host struct
					if (HostInit.DemoMode == HOST_DEMO_PLAY)	// Make sure we are not in demo play mode...
						HostInit.DemoMode = HOST_DEMO_NONE;

					HostInit.hWnd = GameMgr_GethWnd(GMgr);
					
					// Make sure there is a levelname just in case we are not in demo mode or somthing...
					strcpy(HostInit.LevelHack, "Levels\\GenVS.BSP");

					// zap old host
					if ( Host != NULL )
					{ 
						Host_Destroy( Host );
						Host = NULL;
					}
				
					// set host type
					switch ( Result )
					{
						case GMenu_UserSinglePlayerGame:
						{
							strcpy(HostInit.LevelHack, HostInit.UserLevel);
							HostInit.Mode = HOST_MODE_SINGLE_PLAYER;
							break;
						}

						case GMenu_SinglePlayerGame:
						{
							HostInit.Mode = HOST_MODE_SINGLE_PLAYER;
							break;
						}

						case GMenu_SinglePlayerGame1:
						{
							strcpy(HostInit.LevelHack, "Levels\\GenVS.BSP");
							HostInit.Mode = HOST_MODE_SINGLE_PLAYER;
							break;
						}

						case GMenu_SinglePlayerGame2:
						{
							strcpy(HostInit.LevelHack, "Levels\\GenVS2.BSP");
							HostInit.Mode = HOST_MODE_SINGLE_PLAYER;
							break;
						}

						/*case GMenu_SinglePlayerGame3:
						{
							strcpy(HostInit.LevelHack, "Levels\\Gallery.BSP");
							HostInit.Mode = HOST_MODE_SINGLE_PLAYER;
							break;
						}*//*

						case GMenu_StartGame:
						{
							HostInit.Mode = HOST_MODE_SERVER_CLIENT;
							break;
						}

						case GMenu_StartGame1:
						{
							strcpy(HostInit.LevelHack, "Levels\\GenVS.BSP");
							HostInit.Mode = HOST_MODE_SERVER_CLIENT;
							break;
						}

						case GMenu_StartGame2:
						{
							strcpy(HostInit.LevelHack, "Levels\\GenVS2.BSP");
							HostInit.Mode = HOST_MODE_SERVER_CLIENT;
							break;
						}

						case GMenu_Connect:
						{
							HostInit.Mode = HOST_MODE_CLIENT;
							break;
						}
					}

					// Get the client name form the menu
					Menu_GetStringText( GMenu_GetMenu(GMenu_NameEntry), GMenu_NameEntry, TempString );
					strcpy(HostInit.ClientName, TempString);

					// Get the Ip address from the menu...
					Menu_GetStringText( GMenu_GetMenu(GMenu_IPEntry), GMenu_IPEntry, TempString );
					strcpy(HostInit.IPAddress, TempString);

					// create new host
					Host = Host_Create( Engine, &HostInit, GMgr, GameMgr_GetVidMode(GMgr));
					assert( Host != NULL );
					break;
				}

				case GMenu_QuitGame:
				{
					Running = 0;
					break;
				}
			}

			switch(wParam)
			{

				case VK_F12:
				{
					int32		i;
					FILE		*f;
					char		Name[256];

					if (Engine)
					{
						for (i=0 ;i<999; i++)		// Only 999 bmps, oh well...
						{
							sprintf(Name, "Bmp%i.Bmp", i);

							f = fopen(Name, "rb");

							if (f)
							{
								fclose(f);
								continue;
							}
							
							geEngine_ScreenShot(Engine, Name);

							if (Console)
								Console_Printf(Console, "Writing Bmp: %s...\n", Name);

							break;
						}
					}
					break;
				}

				case 192:		// '~'

					if (Console)
						Console_ToggleActive(Console);
					break;

				default:
					if (Console)
						Console_KeyDown(Console, (char)wParam, true);
					break;
			}
			break;
		}
		
		case (WM_KEYUP):
		{
			break;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			return	0;


		default:
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
	return 0;
}
*/

geBoolean IsAMenuActive(void)
{
	// intentionally falls through if required
	return(GMenu_IsAMenuActive());
}


//=====================================================================================
//=====================================================================================
static void GetMouseInput(SDL_Window *hWnd, int Width, int Height)
{
	int   px, py;
	int   wx, wy;
	int   dx, dy;
	int32 x,  y;
	
	assert( Width > 0 );
	assert( Height > 0 );
	SDL_GetMouseState(&px, &py);
  
	//if (
		SDL_GetWindowPosition(hWnd, &wx, &wy);
	/*) {
		geErrorLog_AddString(0,"GetMouseInput: SDL_GetWindowPosition failed",NULL); 
		return;
	}*/
	x = px;
	y = py;

	dx = ((float) (((float)Width/2.0f) - px) / 350.0f);
	dy = ((float) (((float)Height/2.0f) - py) / 350.0f);

	px = Width/2.0;
	py = Height/2.0;
	
	if(ResetMouse)
	{
		SDL_WarpMouseGlobal(px, py);
	}

	GlobalMouseSpeedX = dx;
	GlobalMouseSpeedY = dy;
}

//=====================================================================================
//	GenVS_Error
//=====================================================================================
extern geBoolean	PopupD3DLog;
static geBoolean	ErrorHandled = false;

void GenVS_Error(const char *Msg, ...)
{
	va_list		ArgPtr;
    char		TempStr[1024];
    char		TempStr2[1024];
	FILE		*f;

	if (ErrorHandled)
		return;

	ErrorHandled = true;

	va_start (ArgPtr, Msg);
    vsprintf (TempStr, Msg, ArgPtr);
	va_end (ArgPtr);

	ShutdownAll();

	f = fopen("GTest.Log", "wt");

	if (f)
	{
		int32		i, NumErrors;

		NumErrors = geErrorLog_Count();

		fprintf(f, "Error#:%3i, Code#:%3i, Info: %s\n", NumErrors, 0, TempStr);

		for (i=0; i<NumErrors; i++)
		{
			geErrorLog_ErrorClassType	Error;
			const char						*String;

			if (geErrorLog_Report(NumErrors-i-1, &Error, &String))
			{
				fprintf(f, "Error#:%3i, Code#:%3i, Info:%s\n", NumErrors-i-1, Error, String);
			}
		}

		fclose(f);
		
		sprintf(TempStr2, "%s\nPlease refer to GTest.Log for more info.", TempStr);

		//MessageBox(0, TempStr2, "** Genesis3D Virtual System Error **", MB_OK);
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "** Genesis3D Virtual System Error **", TempStr, GameMgr_GethWnd(GMgr));
		SDL_OpenURL( "GTest.Log" );
		if (PopupD3DLog)
			{
				SDL_OpenURL( "d3ddrv.log" );
			}
	}
	else
	{
		sprintf(TempStr2, "%s\nCould NOT output GTest.log!!!", TempStr);

		//MessageBox(0, TempStr2, "** Genesis3D Virtual System Error **", MB_OK);
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "** Genesis3D Virtual System Error **", TempStr, GameMgr_GethWnd(GMgr));
	}

	_exit(1);
}

