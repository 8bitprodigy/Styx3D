/****************************************************************************************/
/*  CSNETMGR.C                                                                          */
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
#include <assert.h>

//#define INITGUID
#include <Windows.H>
#include <objbase.h>

#include "CSNetMgr.h"
#include "NetPlay.h"

#include "BaseType.h"
#include "Ram.h"
#include "ErrorLog.h"

#include <InitGuid.h>

#pragma message(" some assertions in here would be nice:")

#define PACKET_HEADER_SIZE				1

#define NET_SERVER_ID					4
#define NET_TIMEOUT						15000		// Givem 15 secs


// {33925241-05F8-11d0-8063-00A0C90AE891}
DEFINE_GUID(JET_GUID, 
//0x33925241, 0x5f8, 0x11d0, 0x80, 0x63, 0x0, 0xa0, 0xc9, 0xa, 0xe8, 0x91);
0x33925241, 0x0, 0x11d0, 0x80, 0x63, 0x0, 0xa0, 0xc9, 0xa, 0xe8, 0x91);


static  jeBoolean	NetSession = JE_FALSE;
static	jeBoolean	WeAreTheServer = JE_FALSE;
static  DPID		OurPlayerId;
static	DPID		ServerId;					// The servers Id we are connected too

static	int32		BufferSize = 20000;

static uint8		Packet[20000];
static jeCSNetMgr_NetClient	gClient;

static BOOL jeCSNetMgr_DoSystemMessage(jeCSNetMgr *M,jeCSNetMgr_NetID IdTo, LPDPMSG_GENERIC lpMsg, DWORD dwMsgSize, jeCSNetMgr_NetMsgType *Type, jeCSNetMgr_NetClient *Client);

typedef struct jeCSNetMgr
{
	// instance data goes here
	jeCSNetMgr *Valid;
} jeCSNetMgr;


jeBoolean	jeCSNetMgr_IsValid(jeCSNetMgr *M)
{
	if ( M == NULL )
		{
			return JE_FALSE;
		}
	if ( M->Valid != M )
		{
			return JE_FALSE;
		}
	return JE_TRUE;
}

JETAPI jeCSNetMgr * JETCC		jeCSNetMgr_Create(void)
{
	jeCSNetMgr *M;

	M = JE_RAM_ALLOCATE_STRUCT(jeCSNetMgr);
	if ( M == NULL)
		{
			jeErrorLog_Add(-1, NULL); //FIXME
			return NULL;
		}

	M->Valid = M;
	return M;
}

	
JETAPI void JETCC jeCSNetMgr_Destroy(jeCSNetMgr **ppM)
{
	assert( ppM != NULL );
	assert( jeCSNetMgr_IsValid(*ppM)!=JE_FALSE );
	
	(*ppM) -> Valid = 0;
	jeRam_Free(*ppM);
	*ppM = NULL;	
};

//================================================================================
//	jeCSNetMgr_ReceiveFromServer
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_ReceiveFromServer(jeCSNetMgr *M, uint32 *Type, int32 *Size, uint8 **Data)
{
	DPID				IdTo;
	DWORD				BSize = BufferSize;

	*Size = 0;
	*Data = NULL;
	*Type = NET_MSG_USER;
	
	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );

	if (!NetSession)
		return JE_FALSE;
	/*
	if( jeCSNetMgr_ReceiveSystemMessage( M,OurPlayerId, Type, &gClient ) )
	{
		*Size = sizeof( jeCSNetMgr_NetClient );
		*Data = (uint8*)&gClient;
		*Type = 98;
		return( JE_TRUE );
	}
	*/

	// Find the client player
	IdTo = 0;
    if (NetPlayReceive(&ServerId, &IdTo, DPRECEIVE_FROMPLAYER, (uint8*)&Packet, &BSize))
	if (BSize > 0)
	{
		*Type = Packet[0];
		*Size = BSize - PACKET_HEADER_SIZE;
		*Data = (uint8*)&Packet[1];
		return JE_TRUE;
	}

    return JE_FALSE;			// No message from client
}

//================================================================================
//	jeCSNetMgr_ReceiveFromClient
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_ReceiveFromClient(jeCSNetMgr *M, jeCSNetMgr_NetMsgType *Type, jeCSNetMgr_NetID *IdClient, int32 *Size, uint8 **Data)
{
	DPID				IdTo;
	DWORD				BSize = BufferSize;

	*Size = 0;
	*Data = NULL;
	*Type = NET_MSG_USER;

	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );
	
	if (!NetSession)
		return JE_FALSE;
        
	if( jeCSNetMgr_ReceiveSystemMessage(M, ServerId, Type, &gClient ) )
	{
		*Size = sizeof( jeCSNetMgr_NetClient );
		*Data = (uint8*)&gClient;
		return( JE_TRUE );
	}

	// Find the client player
	IdTo = ServerId;
    if (NetPlayReceive((DPID*)IdClient, &IdTo, DPRECEIVE_TOPLAYER, (uint8*)&Packet, &BSize))
	if (BSize > 0)
	{
		*Type = Packet[0];
		*Size = BSize - PACKET_HEADER_SIZE;
		*Data = (uint8*)&Packet[1];
		return JE_TRUE;
	}

    return JE_FALSE;			// No message from client
}

//================================================================================
//	jeCSNetMgr_ReceiveSystemMessage
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_ReceiveSystemMessage(jeCSNetMgr *M, jeCSNetMgr_NetID IdFor, jeCSNetMgr_NetMsgType *Type, jeCSNetMgr_NetClient *Client)
{
	DPID				SystemId, IdTo;
	DWORD				BSize = BufferSize;

	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );

	*Type = NET_MSG_USER;

	if (!NetSession)
		return JE_FALSE;

	// Find system messages
	SystemId = DPID_SYSMSG;
	IdTo = IdFor;
	if (NetPlayReceive(&SystemId,&IdTo, DPRECEIVE_FROMPLAYER | DPRECEIVE_TOPLAYER, &Packet, &BSize))
	if (BSize > 0)
	{
		
		//AFX_CPrintf("Receiving system message.");
		return( jeCSNetMgr_DoSystemMessage(M, IdFor, (LPDPMSG_GENERIC)&Packet, BSize, Type, Client) );
	}

    return JE_FALSE;			// No message from system
}

//================================================================================
//	Host_DoSystemMessage
//================================================================================
static jeBoolean jeCSNetMgr_DoSystemMessage( jeCSNetMgr *M, jeCSNetMgr_NetID IdTo, LPDPMSG_GENERIC lpMsg, DWORD dwMsgSize, jeCSNetMgr_NetMsgType *Type, jeCSNetMgr_NetClient *Client)
{
	DWORD dwSize;

	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );

	switch( lpMsg->dwType)
    {
		case DPSYS_CREATEPLAYERORGROUP:
        {
			if(WeAreTheServer && IdTo == ServerId)
            {
				LPDPMSG_CREATEPLAYERORGROUP lpAddMsg = (LPDPMSG_CREATEPLAYERORGROUP) lpMsg;
				
				// Don't broadcast the server being created...
				if( lpAddMsg->dpId == ServerId )
					return( FALSE );

				// Name the player, and return it as a NET_MSG_CREATE_CLIENT message
				*Type = NET_MSG_CREATE_CLIENT;
				strcpy(Client->Name, lpAddMsg->dpnName.lpszShortNameA);
				Client->Id = lpAddMsg->dpId;
				
				// The client is waiting for this message, so send it now.
				// It contains our ServerId, which the client needs...
				Packet[0] = NET_SERVER_ID;
				memcpy( &Packet[1], &ServerId, sizeof( ServerId ) );
				dwSize = sizeof( ServerId ) + PACKET_HEADER_SIZE;

				// Fire it off...
				NetPlaySend( ServerId, Client->Id, DPSEND_GUARANTEED, (void*)&Packet, dwSize );

				return( JE_TRUE );
            }
			else
				return( JE_FALSE );
        }
        break;

		case DPSYS_DESTROYPLAYERORGROUP:
        {
            if (WeAreTheServer && IdTo == ServerId)
			{
				LPDPMSG_DESTROYPLAYERORGROUP lpDestroyMsg = (LPDPMSG_DESTROYPLAYERORGROUP) lpMsg;

				Client->Id = lpDestroyMsg->dpId;
				*Type = NET_MSG_DESTROY_CLIENT;

				return JE_TRUE;
			}
			else
				return( JE_FALSE );
			
			break;
        }

		case DPSYS_HOST:
        {           
            WeAreTheServer = JE_TRUE;
			*Type = NET_MSG_HOST;
			break;
        }

		case DPSYS_SESSIONLOST:
		{
			// FIXME:  Make somthing bad happen here
			//AFX_Error("NetPlay Session was lost!");
			return JE_FALSE;
		}

    }

	return( JE_TRUE );
}

//================================================================================
//	jeCSNetMgr_ReceiveAllMessages
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_ReceiveAllMessages(jeCSNetMgr *M, jeCSNetMgr_NetID *IdFrom, jeCSNetMgr_NetID *IdTo, jeCSNetMgr_NetMsgType *Type, int32 *Size, uint8 **Data)
{
	DWORD				BSize = BufferSize;

	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );

	*Size = 0;
	*Data = NULL;
	*Type = NET_MSG_USER;

	if (!NetSession)
		return JE_FALSE;
        
    *IdFrom = 0;
	*IdTo = 0;
	if(NetPlayReceive((DPID*)IdFrom, (DPID*)IdTo, DPRECEIVE_ALL, &Packet, &BSize))
	if (BSize > 0)
	{
		if (*IdFrom == DPID_SYSMSG   )
		{
			*Data = (uint8*)&gClient;
			*Size = sizeof( gClient );
			// IdTo used to be DPID_ALLPLAYERS...
			// Had to change to IdTo so it would work correctly.
			return( jeCSNetMgr_DoSystemMessage(M, DPID_ALLPLAYERS, (LPDPMSG_GENERIC)&Packet, BSize, Type, &gClient) );
			//return( jeCSNetMgr_DoSystemMessage(M, (DPID)*IdTo, (LPDPMSG_GENERIC)&Packet, BSize, Type, &gClient) );
		}
		else
		{	
			*Type = Packet[0];
			*Size = BSize - PACKET_HEADER_SIZE;
			*Data = (uint8*)&Packet[1];
			return JE_TRUE;
		}
	}

    return JE_FALSE;			// No message from client
}


//================================================================================
//	jeCSNetMgr_GetServerID
//================================================================================
JETAPI jeCSNetMgr_NetID JETCC jeCSNetMgr_GetServerID(jeCSNetMgr *M)
{
	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );
	return(ServerId);
}

//================================================================================
//	jeCSNetMgr_GetOurID
//================================================================================
JETAPI jeCSNetMgr_NetID JETCC jeCSNetMgr_GetOurID(jeCSNetMgr *M)
{
	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );
	return(OurPlayerId);
}

//================================================================================
//	jeCSNetMgr_GetAllPlayerID
//================================================================================
JETAPI jeCSNetMgr_NetID JETCC jeCSNetMgr_GetAllPlayerID(jeCSNetMgr *M)
{
	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );
	return(DPID_ALLPLAYERS);
}

//================================================================================
//	jeCSNetMgr_WeAreTheServer
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_WeAreTheServer(jeCSNetMgr *M)
{
	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );
	return (WeAreTheServer);
}

//================================================================================
//	jeCSNetMgr_StartSession
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_StartSession(jeCSNetMgr *M, const char *SessionName, const char* PlayerName )
{
	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );

	NetSession = JE_FALSE;
	WeAreTheServer = JE_FALSE;

	if (!InitNetPlay((LPGUID)&JET_GUID))
	{
		//AFX_CPrintf("AFX_StartSession: Could not init.\n");
		return JE_FALSE;
	}

	if (!NetPlayCreateSession((char*)SessionName, strlen(SessionName)))
	{
		//AFX_CPrintf("AFX_StartSession: Could not create session.\n");
		return JE_FALSE;
	}
	
	if (!NetPlayCreatePlayer(&ServerId, "Server", NULL, NULL, 0))
		return JE_FALSE;

	if (!NetPlayCreatePlayer(&OurPlayerId, (char*)PlayerName, NULL, NULL, 0))
		return JE_FALSE;

	WeAreTheServer = JE_TRUE;
	NetSession = JE_TRUE;

	return JE_TRUE;
} 	

//================================================================================
//	jeCSNetMgr_FindSession
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_FindSession(jeCSNetMgr *M, const char *IPAdress, jeCSNetMgr_NetSession **SessionList, int32 *SessionNum )
{
	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );

	NetSession = JE_FALSE;
	WeAreTheServer = JE_FALSE;

	if (!InitNetPlay((LPGUID)&JET_GUID))
	{
		//AFX_CPrintf("AFX_FindSession: Could not init.\n");
		return JE_FALSE;
	}
	if( !NetPlayEnumSession((char*)IPAdress, (SESSION_DESC**)SessionList, SessionNum) )
	{
		//AFX_CPrintf("AFX_FindSession: Could not enum sessions.\n");
		return JE_FALSE;
	}
	return( JE_TRUE );
}

//================================================================================
//	jeCSNetMgr_JoinSession
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_JoinSession(jeCSNetMgr *M, const char *Name, const jeCSNetMgr_NetSession* Session)
{

	uint32	StartTime;
	DPID	IdFrom, IdTo;
	DWORD	BSize = BufferSize;

	WeAreTheServer = JE_FALSE;
	NetSession = JE_FALSE;

	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );

	if (!NetPlayJoinSession( (SESSION_DESC*)Session) )
	{
		//AFX_CPrintf("AFX_JoinSession: Could not join session.\n");
		return JE_FALSE;
	}
	if (!NetPlayCreatePlayer(&OurPlayerId, (char*)Name, NULL, NULL, 0))
	{
		//AFX_CPrintf("AFX_StartSession: Could not create player for NetPlay session.\n");
		return JE_FALSE;
	}
	

	//Clients must wait until they get a Server Id.
	//  All other System messages. are ignored, until this happens.
	//  The only system message that should come before this is
	//  Create Client message.  Since Clients don't need this message
	//  this should not be a problem.

	StartTime = timeGetTime();
	while( NET_TIMEOUT > (timeGetTime() -  StartTime) )
	{
		if(NetPlayReceive(&IdFrom, &IdTo, DPRECEIVE_ALL, &Packet, &BSize))
		{
			if (BSize > 0)
			if( (IdFrom != DPID_SYSMSG) &&(Packet[0] == NET_SERVER_ID)  )
			{
				//AFX_CPrintf("Received SeverId");
 				memcpy( &ServerId, &Packet[1], sizeof( ServerId ) );
				NetSession = JE_TRUE;
				return JE_TRUE;
			}
		}
	}
	return( JE_FALSE );
} 	

//================================================================================
//	jeCSNetMgr_StopSession
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_StopSession(jeCSNetMgr *M)
{
	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );

	if (NetSession)
	{
		NetSession = JE_FALSE;
	
		if (WeAreTheServer)		// If we are the server, then free the server player
		{
			if (!NetPlayDestroyPlayer(ServerId))
				return JE_FALSE;
		}

		if (!NetPlayDestroyPlayer(OurPlayerId))
			return JE_FALSE;

		if (!DeInitNetPlay())
		{
			//AFX_CPrintf("AFX_DestroyNetSession: There was a problem while De-Intializing NetPlay.\n");
			return JE_FALSE;
		}

	}

	WeAreTheServer = JE_FALSE;
	return JE_TRUE;
}

//================================================================================
//	jeCSNetMgr_SendToServer
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_SendToServer(jeCSNetMgr *M,  BOOL Guaranteed, uint8 *Data, int32 DataSize)
{
    DWORD           dwFlags = 0;

	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );
	
	if (!NetSession)
		return JE_FALSE;

	if( DataSize > BufferSize )
		return JE_FALSE;
	
	if (Guaranteed)
		dwFlags |= DPSEND_GUARANTEED;

	memcpy( &Packet[1], Data, DataSize );
	DataSize += PACKET_HEADER_SIZE;
	Packet[0] = NET_MSG_USER;

	if (!NetPlaySend(OurPlayerId, ServerId, dwFlags, (void*)&Packet, DataSize))
	{
		//AFX_CPrintf("AFX_SendToServer:  Could not send message");
		return JE_FALSE;
	}
	
	return JE_TRUE;
}

//================================================================================
//	jeCSNetMgr_SendToClient
//================================================================================
JETAPI jeBoolean JETCC jeCSNetMgr_SendToClient(jeCSNetMgr *M, jeCSNetMgr_NetID To, BOOL Guaranteed, uint8 *Data, int32 DataSize)
{
    DWORD           dwFlags = 0;

	assert( jeCSNetMgr_IsValid(M)!=JE_FALSE );
	
	if (!NetSession)
		return JE_FALSE;
	
	if( DataSize > BufferSize )
		return JE_FALSE;

	if (Guaranteed)
		dwFlags |= DPSEND_GUARANTEED;
	
	memcpy( &Packet[1], Data, DataSize );
	DataSize += PACKET_HEADER_SIZE;
	Packet[0] = NET_MSG_USER;

	if (!NetPlaySend(ServerId, To, dwFlags, (void*)&Packet, DataSize))
	{
		//AFX_CPrintf("AFX_SendToClient:  Could not send message");
		return JE_FALSE;
	}
	
	return JE_TRUE;
}
