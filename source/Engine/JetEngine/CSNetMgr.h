/****************************************************************************************/
/*  CSNETMGR.H                                                                          */
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
#ifndef JE_CSNETMGR_H
#define JE_CSNETMGR_H

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif


//================================================================================
//	Structure defines
//================================================================================

// JET_PUBLIC_APIS

typedef struct		jeCSNetMgr	jeCSNetMgr;

typedef uint32		jeCSNetMgr_NetID;

// Types for messages received from _ReceiveSystemMessage
typedef enum 
{
	NET_MSG_USER,					// User message
	NET_MSG_CREATE_CLIENT,			// A new client has joined in
	NET_MSG_DESTROY_CLIENT,			// An existing client has left
	NET_MSG_HOST					// We are the server now
} jeCSNetMgr_NetMsgType;

typedef struct
{
	char		Name[32];
	jeCSNetMgr_NetID	Id;
} jeCSNetMgr_NetClient;


#ifdef _INC_WINDOWS
	// Windows.h must be included previously for this api to be exposed.

	typedef struct jeCSNetMgr_NetSession
	{
		char		SessionName[200];					// Description of Service provider
		GUID		Guid;								// Service Provider GUID
		#pragma message("define a jeGUID?.. wouldn't need a windows dependency here...")
	} jeCSNetMgr_NetSession;

JETAPI jeBoolean		JETCC jeCSNetMgr_FindSession(jeCSNetMgr *M, const char *IPAdress, jeCSNetMgr_NetSession **SessionList, int32 *SessionNum );
JETAPI jeBoolean		JETCC jeCSNetMgr_JoinSession(jeCSNetMgr *M, const char *Name, const jeCSNetMgr_NetSession* Session);
#endif

JETAPI jeCSNetMgr *		JETCC jeCSNetMgr_Create(void);
JETAPI void				JETCC jeCSNetMgr_Destroy(jeCSNetMgr **ppM);
JETAPI jeCSNetMgr_NetID	JETCC jeCSNetMgr_GetServerID(jeCSNetMgr *M);
JETAPI jeCSNetMgr_NetID	JETCC jeCSNetMgr_GetOurID(jeCSNetMgr *M);
JETAPI jeCSNetMgr_NetID	JETCC jeCSNetMgr_GetAllPlayerID(jeCSNetMgr *M);
JETAPI jeBoolean JETCC		jeCSNetMgr_ReceiveFromServer(jeCSNetMgr *M, uint32 *Type, int32 *Size, uint8 **Data);
JETAPI jeBoolean JETCC		jeCSNetMgr_ReceiveFromClient(jeCSNetMgr *M, jeCSNetMgr_NetMsgType *Type, jeCSNetMgr_NetID *IdClient, int32 *Size, uint8 **Data);
JETAPI jeBoolean JETCC		jeCSNetMgr_ReceiveSystemMessage(jeCSNetMgr *M, jeCSNetMgr_NetID IdFor, jeCSNetMgr_NetMsgType *Type, jeCSNetMgr_NetClient *Client);
JETAPI jeBoolean JETCC		jeCSNetMgr_ReceiveAllMessages(jeCSNetMgr *M, jeCSNetMgr_NetID *IdFrom, jeCSNetMgr_NetID *IdTo, jeCSNetMgr_NetMsgType *Type, int32 *Size, uint8 **Data);
JETAPI jeBoolean JETCC		jeCSNetMgr_WeAreTheServer(jeCSNetMgr *M);
JETAPI jeBoolean JETCC		jeCSNetMgr_StartSession(jeCSNetMgr *M, const char *SessionName, const char *PlayerName );
JETAPI jeBoolean JETCC		jeCSNetMgr_StopSession(jeCSNetMgr *M);
JETAPI jeBoolean JETCC		jeCSNetMgr_SendToServer(jeCSNetMgr *M, jeBoolean Guaranteed, uint8 *Data, int32 DataSize);
JETAPI jeBoolean JETCC		jeCSNetMgr_SendToClient(jeCSNetMgr *M, jeCSNetMgr_NetID To, jeBoolean Guaranteed, uint8 *Data, int32 DataSize);


// JET_PRIVATE_APIS
#ifdef __cplusplus
}
#endif

#endif
