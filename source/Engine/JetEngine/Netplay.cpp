/****************************************************************************************/
/*  NETPLAY.C                                                                           */
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
#define IDIRECTPLAY2_OR_GREATER

#include <Windows.H>

#include <dplay.h>
#include <dplobby.h>
#include <Stdio.h>

#include "netplay.h"

//#define INIT_GUID

//#define UNICODE   (do not use, not done yet.  Need to fix strings...)


// ************************************************************************************
// Misc globals all will need...
// ************************************************************************************


LPGUID							glpGuid;

SP_DESC							GlobalSP;				
SESSION_DESC*					GlobalSession;			// Global sessions availible
DWORD							gSessionCnt;
BOOL							FoundSP = FALSE;		// If the provider was found
BOOL							FoundSession = FALSE;

LPDIRECTPLAY2A					glpDP = NULL;			// directplay object pointer
LPDIRECTPLAY3A					glpDP3A = NULL;
LPDIRECTPLAYLOBBY2A				glpDPL2A = NULL;

BOOL							FoundConnection = FALSE;
LPVOID							lpConnectionBuffer = NULL;
	
// ************************************************************************************
//	Misc global functions
// ************************************************************************************
BOOL WINAPI EnumSP(LPGUID lpGuid, LPTSTR lptszDesc, DWORD dwMajorVersion,
                   DWORD dwMinorVersion, LPVOID lpv);

HRESULT DPlayCreate(LPGUID lpGuid);
HRESULT DPlayCreateSession(LPTSTR lptszSessionName, DWORD MaxPlayers);
HRESULT DPlayOpenSession(LPGUID lpSessionGuid);
BOOL WINAPI EnumSession(LPCDPSESSIONDESC2 lpDPSessionDesc, LPDWORD lpdwTimeOut, DWORD dwFlags, 
                        LPVOID lpContext);
HRESULT DPlayEnumSessions(DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumCallback, 
                          LPVOID lpContext, DWORD dwFlags);
HRESULT DPlayCreatePlayer(LPDPID lppidID, LPTSTR lptszPlayerName, HANDLE hEvent, 
                          LPVOID lpData, DWORD dwDataSize);
HRESULT DPlayDestroyPlayer(DPID pid);
HRESULT DPlaySend(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize);
HRESULT DPlayReceive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize);
HRESULT DPlayRelease(void);

static HRESULT DPlayCreate2(void );

// New dp3 Connection callback 
BOOL FAR PASCAL DPEnumConnectionsCallback(
						LPCGUID			lpguidSP,
						LPVOID			lpConnection,
						DWORD			dwSize,
						LPCDPNAME		lpName,
						DWORD			dwFlags,
						LPVOID			lpContext);
FILE *DebugF;

void DoDPError(HRESULT Hr)
{
	switch (Hr)
	{
	case CLASS_E_NOAGGREGATION:
		//AFX_CPrintfC("A non-NULL value was passed for the pUnkOuter parameter in DirectPlayCreate, DirectPlayLobbyCreate, or IDirectPlayLobby2::Connect.\n");
		break;

	case DP_OK:
		//AFX_CPrintfC("The request completed successfully.\n");
		break;

	case DPERR_ACCESSDENIED:
		//AFX_CPrintfC("The session is full or an incorrect password was supplied.\n");
		break;

	case DPERR_ACTIVEPLAYERS:
		//AFX_CPrintfC("The requested operation cannot be performed because there are existing active players.\n"); 
		break;

	case DPERR_ALREADYINITIALIZED:
		//AFX_CPrintfC("This object is already initialized. \n");
		break;

	case DPERR_APPNOTSTARTED:
		//AFX_CPrintfC("The application has not been started yet.\n"); 
		break;

	case DPERR_AUTHENTICATIONFAILED:
		//AFX_CPrintfC("The password or credentials supplied could not be authenticated. \n");
		break;

	case DPERR_BUFFERTOOLARGE:
		//AFX_CPrintfC("The data buffer is too large to store. \n");
		break;

	case DPERR_BUSY:
		//AFX_CPrintfC("A message cannot be sent because the transmission medium is busy. \n");
		break;

	case DPERR_BUFFERTOOSMALL:
		//AFX_CPrintfC("The supplied buffer is not large enough to contain the requested data. \n");
		break;

	case DPERR_CANTADDPLAYER:
		//AFX_CPrintfC("The player cannot be added to the session. \n");
		break;

	case DPERR_CANTCREATEGROUP:
		//AFX_CPrintfC("A new group cannot be created. \n");
		break;

	case DPERR_CANTCREATEPLAYER:
		//AFX_CPrintfC("A new player cannot be created. \n");
		break;

	case DPERR_CANTCREATEPROCESS:
		//AFX_CPrintfC("Cannot start the application. \n");
		break;

	case DPERR_CANTCREATESESSION:
		//AFX_CPrintfC("A new session cannot be created. \n");
		break;

	case DPERR_CANTLOADCAPI:
		//AFX_CPrintfC("No credentials were supplied and the CryptoAPI package (CAPI) to use for cryptography services cannot be loaded. \n");
		break;

	case DPERR_CANTLOADSECURITYPACKAGE:
		//AFX_CPrintfC("The software security package cannot be loaded. \n");
		break;

	case DPERR_CANTLOADSSPI:
		//AFX_CPrintfC("No credentials were supplied and the software security package (SSPI) that will prompt for credentials cannot be loaded. \n");
		break;

	case DPERR_CAPSNOTAVAILABLEYET:
		//AFX_CPrintfC("The capabilities of the DirectPlay object have not been determined yet. This error will occur if the DirectPlay object is implemented on a connectivity solution that requires polling to determine available bandwidth and latency. \n");
		break;

	case DPERR_CONNECTING:
		//AFX_CPrintfC("The method is in the process of connecting to the network. The application should keep calling the method until it returns DP_OK, indicating successful completion, or it returns a different error. \n");
		break;

	case DPERR_ENCRYPTIONFAILED:
		//AFX_CPrintfC("The requested information could not be digitally encrypted. Encryption is used for message privacy. This error is only relevant in a secure session. \n");
		break;

	case DPERR_EXCEPTION:
		//AFX_CPrintfC("An exception occurred when processing the request. \n");
		break;

	case DPERR_GENERIC:
		//AFX_CPrintfC("An undefined error condition occurred. \n");
		break;

	case DPERR_INVALIDFLAGS:
		//AFX_CPrintfC("The flags passed to this method are invalid. \n");
		break;

	case DPERR_INVALIDGROUP:
		//AFX_CPrintfC("The group ID is not recognized as a valid group ID for this game session. \n");
		break;

	case DPERR_INVALIDINTERFACE:
		//AFX_CPrintfC("The interface parameter is invalid. \n");
		break;

	case DPERR_INVALIDOBJECT:
		//AFX_CPrintfC("The DirectPlay object pointer is invalid. \n");
		break;

	case DPERR_INVALIDPARAMS: 
		//AFX_CPrintfC("One or more of the parameters passed to the method are invalid. \n");
		break;

	case DPERR_INVALIDPASSWORD: 
		//AFX_CPrintfC("An invalid password was supplied when attempting to join a session that requires a password. \n");
		break;

	case DPERR_INVALIDPLAYER: 
		//AFX_CPrintfC("The player ID is not recognized as a valid player ID for this game session. \n");
		break;
	
	case DPERR_LOGONDENIED: 
		//AFX_CPrintfC("The session could not be opened because credentials are required and either no credentials were supplied or the credentials were invalid. \n");
		break;

	case DPERR_NOCAPS:
		//AFX_CPrintfC("The communication link that DirectPlay is attempting to use is not capable of this function. \n");
		break;

	case DPERR_NOCONNECTION: 
		//AFX_CPrintfC("No communication link was established. \n");
		break;

	case DPERR_NOINTERFACE: 
		//AFX_CPrintfC("The interface is not supported. \n");
		break;

	case DPERR_NOMESSAGES:
		//AFX_CPrintfC("There are no messages in the receive queue. \n");
		break;

	case DPERR_NONAMESERVERFOUND:
		//AFX_CPrintfC("No name server (host) could be found or created. A host must exist to create a player. \n");
		break;

	case DPERR_NONEWPLAYERS: 
		//AFX_CPrintfC("The session is not accepting any new players. \n");
		break;

	case DPERR_NOPLAYERS: 
		//AFX_CPrintfC("There are no active players in the session. \n");
		break;

	case DPERR_NOSESSIONS: 
		//AFX_CPrintfC("There are no existing sessions for this game. \n");
		break;

	case DPERR_NOTLOBBIED: 
		//AFX_CPrintfC("Returned by the IDirectPlayLobby2::Connect method if the application was not started by using the IDirectPlayLobby2::RunApplication method or if there is no DPLCONNECTION structure currently initialized for this DirectPlayLobby object. \n");
		break;

	case DPERR_NOTLOGGEDIN: 
		//AFX_CPrintfC("An action cannot be performed because a player or client application is not logged in. Returned by the IDirectPlay3::Send method when the client application tries to send a secure message without being logged in. \n");
		break;

	case DPERR_OUTOFMEMORY: 
		//AFX_CPrintfC("There is insufficient memory to perform the requested operation. \n");
		break;

	case DPERR_PLAYERLOST:
		//AFX_CPrintfC("A player has lost the connection to the session. \n");
		break;

	case DPERR_SENDTOOBIG: 
		//AFX_CPrintfC("The message being sent by the IDirectPlay3::Send method is too large. \n");
		break;

	case DPERR_SESSIONLOST: 
		//AFX_CPrintfC("The connection to the session has been lost. \n");
		break;

	case DPERR_SIGNFAILED: 
		//AFX_CPrintfC("The requested information could not be digitally signed. Digital signatures are used to establish the authenticity of messages. \n");
		break;

	case DPERR_TIMEOUT: 
		//AFX_CPrintfC("The operation could not be completed in the specified time. \n");
		break;

	case DPERR_UNAVAILABLE: 
		//AFX_CPrintfC("The requested function is not available at this time. \n");
		break;

	case DPERR_UNINITIALIZED: 
		//AFX_CPrintfC("The requested object has not been initialized. \n");
		break;

	case DPERR_UNKNOWNAPPLICATION: 
		//AFX_CPrintfC("An unknown application was specified. \n");
		break;

	case DPERR_UNSUPPORTED:
		//AFX_CPrintfC("The function is not available in this implementation. Returned from IDirectPlay3::GetGroupConnectionSettings and IDirectPlay3::SetGroupConnectionSettings if they are called from a session that is not a lobby session. \n");
		break;

	case DPERR_USERCANCEL: 
		//AFX_CPrintfC("Can be returned in two ways. 1) The user canceled the connection process during a call to the IDirectPlay3::Open method. 2) The user clicked Cancel in one of the DirectPlay service provider dialog boxes during a call to IDirectPlay3::EnumSessions. \n");
		break;

	default:
		//AFX_CPrintfC("NetPlayError:  Don't know this one...\n");
		break;
	}
	
	if (Hr != DP_OK)
		//AFX_CPrintfC("NetPlayError:\n");

	return;
}

// Enumerate the service providers, and everything else...
BOOL InitNetPlay(LPGUID lpGuid)
{
	HRESULT		Hr;

	glpGuid = lpGuid;
	
	FoundSP = FALSE;

	Hr = DPlayCreate2();
	if (Hr != DP_OK)
	{
		//AFX_CPrintfC("InitNetPlay:  Could not create the direct play object1.\n");
		DoDPError(Hr);
		return FALSE;
	}
	
	IDirectPlay3_EnumConnections( glpDP3A, glpGuid, DPEnumConnectionsCallback, NULL, 0);
	
	
	if (!FoundConnection)
	{
		//AFX_CPrintfC("InitNetPlay:  Could not find connection.\n");
		return FALSE;
	}


	return TRUE;
}

BOOL NetPlayEnumSession(LPSTR IPAdress, SESSION_DESC** SessionList, DWORD* SessionNum)
{	char tempBuf[1000];
	DWORD tempLng = 1000;
	HRESULT		hr;


	hr = CoCreateInstance(	CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER,
					IID_IDirectPlayLobby2A, (LPVOID *) &glpDPL2A );

 	hr = IDirectPlayLobby_CreateAddress(glpDPL2A, &DPSPGUID_TCPIP, &DPAID_INet, (LPVOID)IPAdress, strlen(IPAdress), tempBuf, &tempLng  );
	if (hr != DP_OK)
	{
		DoDPError(hr);
		return( FALSE );
	}
	if(lpConnectionBuffer ) 
	{
		free( lpConnectionBuffer );
		lpConnectionBuffer = NULL;
	}
	hr = IDirectPlay3_InitializeConnection( glpDP3A, tempBuf, 0);
	if (hr != DP_OK)
	{
		DoDPError(hr);
		return( FALSE );
	}
	if( GlobalSession )
	{
		GlobalSession = NULL;
	}
	gSessionCnt = 0;
	hr = DPlayEnumSessions(0, EnumSession, NULL, 0);
	*SessionList = GlobalSession;
	*SessionNum = gSessionCnt;
	return( TRUE );

}

BOOL NetPlayCreateSession(LPSTR SessionName, DWORD MaxPlayers)
{
	HRESULT	Hr;

	//if (!FoundSP) 
	//	return FALSE;

	Hr = IDirectPlay3_InitializeConnection( glpDP3A, lpConnectionBuffer, 0);
	if (Hr != DP_OK)
	{
		DoDPError(Hr);
		return( FALSE );
	}
	Hr = DPlayCreateSession(SessionName, MaxPlayers);

	if (Hr != DP_OK)
	{
		DoDPError(Hr);
		//AFX_CPrintfC("NetPlayCreateSession:  Could not create the direct play session.\n");
		return FALSE;
	}

	return TRUE;
}

BOOL NetPlayJoinSession(SESSION_DESC* Session)
{
    HRESULT Hr;
	    

	
	Hr = DPlayOpenSession(&Session->Guid);

	if (Hr != DP_OK)
	{
		DoDPError(Hr);
		return FALSE;
	}

	return TRUE;
}

//
// Creates a player for session
//
BOOL NetPlayCreatePlayer(LPDPID lppidID, LPTSTR lptszPlayerName, HANDLE hEvent, 
                          LPVOID lpData, DWORD dwDataSize)
{
	HRESULT Hr;
	
	Hr = DPlayCreatePlayer(lppidID, lptszPlayerName, hEvent,lpData, dwDataSize);

	if (Hr != DP_OK)
	{
		DoDPError(Hr);
		return FALSE;
	}

	return TRUE;
}

BOOL NetPlayDestroyPlayer(DPID pid)
{
	HRESULT Hr = DPlayDestroyPlayer(pid);
	
	if (Hr != DP_OK)
	{
		DoDPError(Hr);
		return FALSE;
	}

	return TRUE;
}

BOOL NetPlayReceive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{
    HRESULT Hr = E_FAIL;

    if (glpDP)
        Hr = IDirectPlay3_Receive(glpDP3A, lpidFrom, lpidTo, dwFlags, lpData, lpdwDataSize);
    
	if (Hr != DP_OK)
	{
	//	DoDPError(Hr);
		return FALSE;
	}
			
	return TRUE;
}

BOOL NetPlaySend(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize)
{
    HRESULT Hr = E_FAIL;

	if (glpDP3A)
       Hr = IDirectPlay3_Send(glpDP3A, idFrom, idTo, dwFlags, lpData, dwDataSize);
    
    if (Hr != DP_OK)
	{
		DoDPError(Hr);
		return FALSE;
	}
	
	return TRUE;
}

// 
// Release DirectPlay and free all
//
BOOL DeInitNetPlay(void)
{
	HRESULT Hr;

	if (lpConnectionBuffer)
	{
		free(lpConnectionBuffer);
		lpConnectionBuffer = NULL;
	}

	FoundConnection = FALSE;
	FoundSP = FALSE;
	
	Hr = DPlayRelease();
	
	if (Hr != DP_OK)
	{
		DoDPError(Hr);
		return FALSE;
	}

	return TRUE;
}

// ************************************************************************************
// WRAPPER Code sarts here...
// ************************************************************************************

BOOL WINAPI EnumSP(LPGUID lpGuid, LPTSTR lptszDesc, DWORD dwMajorVersion,
                   DWORD dwMinorVersion, LPVOID lpv)
{
	LPSTR Str = lptszDesc;

	//fprintf(DebugF, "Found: %s\n", lptszDesc);
	// Loop through and try to see if this is the service provider we want (TCP/IP for now...)
	while (strlen(Str) > 0)
	{
		if (!strnicmp(Str, "TCP", 3))
		//if (!strnicmp(Str, "Serial", 6))
		{
			//fprintf(DebugF, " **Using: %s\n", lptszDesc);
			GlobalSP.Guid = *lpGuid;
			strcpy(GlobalSP.Desc,lptszDesc);
			FoundSP = TRUE;
			break;
		}
		Str++;
	}
    return(TRUE);
}

//
// Creates the directPlay object
//
HRESULT DPlayCreate(LPGUID lpGuid)
{
    HRESULT hr=E_FAIL;
    LPDIRECTPLAY lpDP=NULL;

	typedef HRESULT (WINAPI *DP_CREATE_FUNC)(LPGUID, LPDIRECTPLAY *, IUnknown *);
	DP_CREATE_FUNC pDirectPlayCreate;
	HMODULE hmodDirectPlay;

	hmodDirectPlay = LoadLibrary ("DPLAYX.DLL");
	if (hmodDirectPlay == NULL)
	{
		return DPERR_GENERIC;	// ugly, huh?  maybe DPERR_UNAVAILABLE?
	}

	pDirectPlayCreate = (DP_CREATE_FUNC)GetProcAddress (hmodDirectPlay, "DirectPlayCreate");
	if (pDirectPlayCreate == NULL)
	{
		FreeLibrary (hmodDirectPlay);
		return DPERR_GENERIC;
	}

    // create a DirectPlay1 interface
    if ((hr = pDirectPlayCreate(lpGuid, &lpDP, NULL)) == DP_OK)
    {
        if (lpDP)
        {
            // query for a DirectPlay2(A) interface
#ifdef UNICODE
            hr = IDirectPlay_QueryInterface(lpDP,&IID_IDirectPlay2,(LPVOID *)&glpDP);
#else
			hr = IDirectPlay_QueryInterface(lpDP,&IID_IDirectPlay2A,(LPVOID *)&glpDP);
#endif
            // no longer need the DirectPlay1 interface
            IDirectPlay_Release(lpDP);
        }
    }

	FreeLibrary (hmodDirectPlay);
    return hr;
}

//
// Creates the directPlay object
//
HRESULT DPlayCreate2( void )
{
    HRESULT hr=E_FAIL;
    LPDIRECTPLAY lpDP=NULL;

	CoInitialize( NULL );

	hr = CoCreateInstance(	&CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER,
							&IID_IDirectPlay3A, (LPVOID *) &glpDP3A );

    // create a DirectPlay1 interface
    if (hr == DP_OK)
    {
            // query for a DirectPlay2(A) interface
		#ifdef UNICODE
            hr = IDirectPlay_QueryInterface(glpDP3A,&IID_IDirectPlay2,(LPVOID *)&glpDP);
		#else
			hr = IDirectPlay_QueryInterface(glpDP3A,&IID_IDirectPlay2A,(LPVOID *)&glpDP);
		#endif
    }
	else
		DoDPError(hr);

    return hr;
}

//
// Creates the directplay session
//
HRESULT DPlayCreateSession(LPTSTR lptszSessionName, DWORD MaxPlayers)
{
    HRESULT hr = E_FAIL;
    DPSESSIONDESC2 dpDesc;

    ZeroMemory(&dpDesc, sizeof(dpDesc));
    dpDesc.dwSize = sizeof(dpDesc);
    dpDesc.dwFlags = DPSESSION_KEEPALIVE;
    //dpDesc.dwFlags = DPSESSION_MIGRATEHOST | DPSESSION_KEEPALIVE;
	dpDesc.dwMaxPlayers = MaxPlayers;

	//AFX_CPrintfC("Starting to open");

#ifdef UNICODE
    dpDesc.lpszSessionName = lptszSessionName;
#else
    dpDesc.lpszSessionNameA = lptszSessionName;
#endif

    // set the application guid
    if (glpGuid)
        dpDesc.guidApplication = *glpGuid;

    if (glpDP)
        hr = IDirectPlay3_Open(glpDP3A, &dpDesc, DPOPEN_CREATE);

	if (hr != DP_OK)
	{
		DoDPError(hr);
	}
	
	//AFX_CPrintfC("Ending open");

    return hr;
}

//
// Joins an existing session
//
HRESULT DPlayOpenSession(LPGUID lpSessionGuid)
{
    HRESULT hr = E_FAIL;
    DPSESSIONDESC2 dpDesc;

    ZeroMemory(&dpDesc, sizeof(dpDesc));
    dpDesc.dwSize = sizeof(dpDesc);

    // set the session guid
    if (lpSessionGuid)
        dpDesc.guidInstance = *lpSessionGuid;
    // set the application guid
    if (glpGuid)
        dpDesc.guidApplication = *glpGuid;

    // open it
    if (glpDP)
        hr = IDirectPlay3_Open(glpDP3A, &dpDesc, DPOPEN_JOIN);

    return hr;
}

//
//	Enums all the availible sessions, and saves them for joining...
//
HRESULT DPlayEnumSessions(DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumCallback, 
                          LPVOID lpContext, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    DPSESSIONDESC2 dpDesc;

    ZeroMemory(&dpDesc, sizeof(dpDesc));
    dpDesc.dwSize = sizeof(dpDesc);
    if (glpGuid)
        dpDesc.guidApplication = *glpGuid;

    if (glpDP)
        hr = IDirectPlay3_EnumSessions(glpDP3A, &dpDesc, dwTimeout, lpEnumCallback,
                                        lpContext, dwFlags);
    return hr;
}

//
//	Callback for enum session
//
BOOL WINAPI EnumSession(LPCDPSESSIONDESC2 lpDPSessionDesc, LPDWORD lpdwTimeOut, DWORD dwFlags, 
                        LPVOID lpContext)
{
    HWND hWnd = (HWND) lpContext;
	LPSTR Str = NULL;
    //LPGUID lpGuid;

    if(dwFlags & DPESC_TIMEDOUT) 
		return FALSE;       // don't try again

    //if (hWnd == NULL) return FALSE;

    // allocate memory to remember the guid
    //lpGuid = (LPGUID) malloc(sizeof(GUID));
    //if (!lpGuid) return FALSE;

    //*lpGuid = lpDPSessionDesc->guidInstance;

	gSessionCnt++;
	if( GlobalSession )
		GlobalSession = realloc( GlobalSession, gSessionCnt * sizeof( SESSION_DESC ) );
	else
		GlobalSession = malloc( sizeof( SESSION_DESC ) );
	GlobalSession[gSessionCnt-1].Guid = lpDPSessionDesc->guidInstance;
#ifdef UNICODE
	strcpy(GlobalSession[gSessionCnt-1].SessionName, lpDPSessionDesc->lpszSessionName);
#else
	strcpy(GlobalSession[gSessionCnt-1].SessionName, lpDPSessionDesc->lpszSessionNameA);
#endif

    return(TRUE);
}

//
//	Creates a player 
//
HRESULT DPlayCreatePlayer(LPDPID lppidID, LPTSTR lptszPlayerName, HANDLE hEvent, 
                          LPVOID lpData, DWORD dwDataSize)
{
    HRESULT hr=E_FAIL;
    DPNAME name;
    
    ZeroMemory(&name,sizeof(name));
    name.dwSize = sizeof(DPNAME);

#ifdef UNICODE
    name.lpszShortName = lptszPlayerName;
#else
    name.lpszShortNameA = lptszPlayerName;
#endif

    if (glpDP)
        hr = IDirectPlay3_CreatePlayer(glpDP3A, lppidID, &name, hEvent, lpData, 
                                      dwDataSize, 0);
                                    
    return hr;
}

//
//	Destroys a player
//
HRESULT DPlayDestroyPlayer(DPID pid)
{
	HRESULT hr=E_FAIL;
	
    if (glpDP)
		hr = IDirectPlay3_DestroyPlayer(glpDP3A, pid);

	return hr;
}

//
// Releases the Dplay object
//
HRESULT DPlayRelease(void)
{
    HRESULT hr = E_FAIL;

    if (glpDP)
    {
        // free session desc, if any
		/*
        if (glpdpSD) 
        {
            free(glpdpSD);
            glpdpSD = NULL;
        }

        // free connection settings structure, if any (lobby stuff)
        if (glpdplConnection)
        {
            free(glpdplConnection);
            glpdplConnection = NULL;
        }
		*/
        // release dplay
        hr = IDirectPlay2_Release(glpDP);
        glpDP = NULL;
    }

	if (glpDP3A)
	{
		IDirectPlay3_Close(glpDP3A );
	//	hr = IDirectPlay3_Release(glpDP3A);
		glpDP3A = NULL;
	}
	
	CoUninitialize();

    return hr;
}

HRESULT DPlaySend(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize)
{
    HRESULT hr = E_FAIL;

    if (glpDP)
        hr = IDirectPlay2_Send(glpDP, idFrom, idTo, dwFlags, lpData, dwDataSize);
    
    return hr;
}
HRESULT DPlayReceive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{
    HRESULT hr = E_FAIL;

    if (glpDP)
        hr = IDirectPlay2_Receive(glpDP, lpidFrom, lpidTo, dwFlags, lpData, lpdwDataSize);
    
    return hr;
}

BOOL FAR PASCAL DPEnumConnectionsCallback(
						LPCGUID			lpguidSP,
						LPVOID			lpConnection,
						DWORD			dwSize,
						LPCDPNAME		lpName,
						DWORD			dwFlags,
						LPVOID			lpContext)
{
	LPSTR Str;
	
	if (FoundConnection)
		return TRUE;

	Str = lpName->lpszShortNameA;
	// Loop through and try to see if this is the service provider we want (TCP/IP for now...)
	while (strlen(Str) > 0)
	{
		if (!strnicmp(Str, "TCP", 3))
		//if (!strnicmp(Str, "Serial", 6))
		{
			// Make sure it's deleted
			if (lpConnectionBuffer)
			{
				free(lpConnectionBuffer);
				lpConnectionBuffer = NULL;
			}
	
			// make space for Connection Shortcut
			lpConnectionBuffer = (char*)malloc(dwSize);
			if (lpConnectionBuffer == NULL)
				goto FAILURE;

			memcpy(lpConnectionBuffer, lpConnection, dwSize);
			FoundConnection = TRUE;
			break;
		}
		Str++;
	}

FAILURE:
    return (TRUE);
}
