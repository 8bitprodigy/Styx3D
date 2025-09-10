/****************************************************************************************/
/*  NETPLAY.H                                                                           */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description:                                                                        */
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
#ifndef JE_NETPLAY_H
#define JE_NETPLAY_H

#include <windows.h>
#include <dplay.h>

#pragma message("Netplay.h : includes windows.h")

#ifdef	__cplusplus
extern "C" {
#endif

// ************************************************************************************
//	Defines
// ************************************************************************************
#define NETPLAY_OPEN_CREATE		1
#define NETPLAY_OPEN_JOIN		2


typedef struct
{
	char	Desc[200];								// Description of Service provider
	GUID	Guid;									// Global Service Provider GUID
} SP_DESC;

// must match stuct AFX_SESSION in cengine.h
typedef struct
{
	char	SessionName[200];						// Description of Service provider
	GUID	Guid;									// Global Service Provider GUID
} SESSION_DESC;

extern	SP_DESC					GlobalSP;			// Global info about the sp
extern  SESSION_DESC*			GlobalSession;		// Global sessions availible
extern	LPGUID					glpGuid;
													
void DoDPError(HRESULT Hr);
BOOL InitNetPlay(LPGUID lpGuid);
BOOL NetPlayEnumSession(LPSTR IPAdress, SESSION_DESC** SessionList, DWORD* SessionNum);
BOOL NetPlayJoinSession(SESSION_DESC* SessionList);
BOOL NetPlayCreateSession(LPSTR SessionName, DWORD MaxPlayers);
BOOL NetPlayCreatePlayer(LPDPID lppidID, LPTSTR lptszPlayerName, HANDLE hEvent, 
                          LPVOID lpData, DWORD dwDataSize);
BOOL NetPlayDestroyPlayer(DPID pid);
BOOL NetPlaySend(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize);
BOOL NetPlayReceive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize);
BOOL DeInitNetPlay(void);

// HACK!!!! Function is in Engine.cpp (So NetPlay.C can call it...)
BOOL			AFX_CPrintfC(char *String);

#ifdef	__cplusplus
}
#endif

#endif
