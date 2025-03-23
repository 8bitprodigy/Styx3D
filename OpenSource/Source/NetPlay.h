/****************************************************************************************/
/*  NetPlay.h                                                                           */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: DirectPlay wrapper                                                     */
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
/*  Genesis3D Version 1.1 released November 15, 1999                                 */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef GE_NETPLAY_H
#define GE_NETPLAY_H

#ifdef _WIN32
	#include <windows.h>
	#include <dplay.h>
#else /* _WIN32 */
	#include <stdint.h>
	#include <uuid/uuid.h>
	
	typedef uint32_t  DWORD;
	typedef char*     LPSTR;
	typedef char*     LPTSTR;
	typedef uint32_t  DPID;
	typedef uuid_t    GUID;
	typedef GUID*     LPGUID;
	typedef uint32_t* LPDPID;
	typedef uint32_t* LPDWORD;
	typedef int       HRESULT;
	#define S_OK      0
	#define E_FAIL   -1
	typedef void*     HANDLE;
	typedef void*     LPVOID;
	typedef uint32_t  DPID;  // DirectPlay ID
	typedef DPID*     LPDPID;

	#define DPID_SYSMSG                 0xFFFFFFFF
	#define DPID_ALLPLAYERS             0xFFFFFFFE

	#define DP_OK                         0   // Success
	#define DPERR_NOMESSAGES              1   // No messages available

	#define DPRECEIVE_ALL               0x0001
	#define DPRECEIVE_FROMPLAYER        0x0002
	#define DPRECEIVE_TOPLAYER          0x0004

	#define DPSEND_GUARANTEED           0x0008

	// Message Types
	#define DPSYS_CREATEPLAYERORGROUP   0x0001
	#define DPSYS_DESTROYPLAYERORGROUP  0x0002
	#define DPSYS_ADDPLAYERTOGROUP      0x0003
	#define DPSYS_DELETEPLAYERFROMGROUP 0x0004
	#define DPSYS_SESSIONLOST           0x0005

	// Generic DirectPlay message structure
	typedef struct 
	{
		uint32_t dwType;
		char     data[256]; // Dummy payload
	} 
	DPMSG_GENERIC, *
	LPDPMSG_GENERIC;

	typedef struct 
	{
		char *lpszShortNameA;
	} 
	DPNNAME;
	// Player creation message structure
	typedef struct 
	{
		uint32_t dwType;
		DPID     dpId;
		char     name[64]; // Dummy player name
		DPNNAME  dpnName;
	}*
	LPDPMSG_CREATEPLAYERORGROUP;

	typedef struct 
	{
		DWORD               dwSize;              // Size of this structure
		DWORD               dwFlags;             // Flags related to the operation
		DPID                dpId;                // The DirectPlay ID of the player or group to destroy
		DWORD               dwReserved;          // Reserved for future use
	}*
	LPDPMSG_DESTROYPLAYERORGROUP;
	
	typedef struct 
	{
		DWORD               dwSize;          // Size of this structure
		DWORD               dwFlags;         // Flags to indicate host-specific options
		DPID                dpidHost;        // The DirectPlay ID of the host player
		DWORD               dwReserved;      // Reserved for future use
	}*
	LPDPMSG_HOST;

	typedef enum 
	{
		DPMSG_CREATEPLAYERORGROUP,
		DPMSG_DESTROYPLAYERORGROUP,
		DPSYS_HOST,
		DPSYS_SESSIONLOST,
	} 
	DPSYS_MESSAGE_TYPE;
#endif

#include "BaseType.h"

#ifdef	__cplusplus
extern "C" {
#endif

// ************************************************************************************
//	Defines
// ************************************************************************************
#define NETPLAY_OPEN_CREATE 1
#define NETPLAY_OPEN_JOIN   2


typedef struct
{
	char Desc[200]; // Description of Service provider
	GUID Guid;      // Global Service Provider GUID
} SP_DESC;

// must match stuct AFX_SESSION in cengine.h
typedef struct
{
	char SessionName[200]; // Description of Service provider
	GUID Guid;             // Global Service Provider GUID
} SESSION_DESC;

extern SP_DESC       GlobalSP;      // Global info about the sp
extern SESSION_DESC *GlobalSession; // Global sessions availible
extern LPGUID        glpGuid;
													
void      DoDPError(           HRESULT       Hr);
geBoolean InitNetPlay(         LPGUID        lpGuid);
geBoolean NetPlayEnumSession(  LPSTR         IPAdress,    SESSION_DESC **SessionList,     DWORD *SessionNum);
geBoolean NetPlayJoinSession(  SESSION_DESC *SessionList);
geBoolean NetPlayCreateSession(LPSTR         SessionName, DWORD          MaxPlayers);
geBoolean NetPlayCreatePlayer( LPDPID        lppidID,     LPTSTR         lptszPlayerName, HANDLE  hEvent,  LPVOID lpData, DWORD   dwDataSize,    geBoolean ServerPlayer);
geBoolean NetPlayDestroyPlayer(DPID          pid);
HRESULT   NetPlaySend(         DPID          idFrom,      DPID           idTo,            DWORD   dwFlags, LPVOID lpData, DWORD   dwDataSize);
HRESULT   NetPlayReceive(      LPDPID        lpidFrom,    LPDPID         lpidTo,          DWORD   dwFlags, LPVOID lpData, LPDWORD lpdwDataSize);
geBoolean DeInitNetPlay(       void);

// HACK!!!! Function is in Engine.cpp (So NetPlay.C can call it...)
geBoolean AFX_CPrintfC(        char         *String);

#ifdef	__cplusplus
}
#endif

#endif /* GE_NETPLAY_H */
