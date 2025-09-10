/****************************************************************************************/
/*  MSGLOG.CPP                                                                          */
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
	
// This module Log windows messages to a file.  Then replays them using dispatch msg.

//	by trilobite jan. 2011
#include "stdafx.h"
//
#include "WndReg.h"
#include "ram.h"
#include "jwe.h"

typedef enum {
	LOGMODE_NONE,
	LOGMODE_RECORDING,
	LOGMODE_PLAYING
} LOGMODES;

typedef struct tagLogEntry
{
	MSG msg;
	int time;
	int32 WndSigniture;
}	LogEntry;

typedef struct tagMsgLog
{
	FILE * File;
	int	   Mode;
	int	   StartTime;
	LogEntry *Entries;
	int	EntryN;
	int	CurEntry;
} MsgLog;


MsgLog * MsgLog_Create()
{
	MsgLog * pMsgLog ;
	
	pMsgLog = JE_RAM_ALLOCATE_STRUCT_CLEAR( MsgLog ) ;
	if( pMsgLog == NULL )
		goto MLC_FAILURE ;

	return( pMsgLog );

MLC_FAILURE:
	if( pMsgLog != NULL )
		jeRam_Free( pMsgLog );
	return( NULL );

}

void MsgLog_Destroy( MsgLog **hMsgLog )
{
	ASSERT( hMsgLog );
	ASSERT( *hMsgLog );

	if( (*hMsgLog)->File )
		fclose( (*hMsgLog)->File  );
	
	if( (*hMsgLog)->Entries != NULL )
		jeRam_Free( (*hMsgLog)->Entries );

	jeRam_Free( (*hMsgLog) );
}

static BOOL	MsgLog_WriteRecordN( MsgLog * pMsgLog )
{


	if( fseek( pMsgLog->File, 0, SEEK_SET ) )
		return( FALSE );

	if( !fprintf( pMsgLog->File, "EntryN %04d\n", pMsgLog->EntryN ) )
		return( FALSE );

	if( fseek( pMsgLog->File, 0, SEEK_END ) )
		return( FALSE );

	return( TRUE );
}


BOOL MsgLog_StartRecord( MsgLog * pMsgLog, char * Name )
{
	ASSERT( pMsgLog );
	ASSERT( Name );

	if( pMsgLog->File )
		fclose( pMsgLog->File );

	pMsgLog->File = fopen( Name, "w+");
	if( pMsgLog->File == 0 )
		return( FALSE );
	pMsgLog->EntryN = 0;
	pMsgLog->StartTime = GetTickCount();
	pMsgLog->Mode = LOGMODE_RECORDING;
	if( !MsgLog_WriteRecordN(  pMsgLog ) )
		return( FALSE );
	return( TRUE );
}

BOOL MsgLog_IsLogMsg( MSG* pMsg )
{
	if( pMsg->message == WM_MOUSEMOVE ||
		pMsg->message == WM_LBUTTONDOWN ||
		pMsg->message == WM_LBUTTONUP ||
		pMsg->message == WM_RBUTTONDOWN ||
		pMsg->message == WM_RBUTTONUP ||
		pMsg->message == WM_KEYDOWN ||
		pMsg->message == WM_KEYUP  )
		return( TRUE );
	return( FALSE );
}

BOOL MsgLog_Record( MsgLog * pMsgLog, MSG* pMsg, int32 WndSigniture )
{
	int Time;
	char *SigStr;

	ASSERT( pMsgLog );
	ASSERT( pMsgLog->File );
	ASSERT( pMsg );
	ASSERT( pMsgLog->Mode == LOGMODE_RECORDING );

	if( !MsgLog_IsLogMsg( pMsg  ) )
		return( TRUE );

	Time = GetTickCount() - pMsgLog->StartTime;

	SigStr = (char*)&WndSigniture;
	if( !fprintf( pMsgLog->File, "message %d\n", pMsg->message ) )
		goto RECORD_ERR;
	if( !fprintf( pMsgLog->File, "wParam %d\n", pMsg->wParam ) )
		goto RECORD_ERR;
	if( !fprintf( pMsgLog->File, "lParam %d\n", pMsg->lParam ) )
		goto RECORD_ERR;
	if( !fprintf( pMsgLog->File, "x: %d\n", pMsg->pt.x ) )
		goto RECORD_ERR;
	if( !fprintf( pMsgLog->File, "y: %d\n", pMsg->pt.y ) )
		goto RECORD_ERR;
	if( !fprintf( pMsgLog->File, "Time %d\n", Time ) )
		goto RECORD_ERR;
	if( !fprintf( pMsgLog->File, "%c%c%c%c\n", SigStr[3], SigStr[2], SigStr[1], SigStr[0] ) )
		goto RECORD_ERR;
	pMsgLog->EntryN++;
	if( !MsgLog_WriteRecordN(  pMsgLog ) )
		goto RECORD_ERR;
	fflush( pMsgLog->File );

	return( TRUE );

RECORD_ERR:
	return( FALSE );
}

void MsgLog_EndRecord( MsgLog * pMsgLog )
{
	ASSERT( pMsgLog );
	ASSERT( pMsgLog->File );
	ASSERT( pMsgLog->Mode == LOGMODE_RECORDING );


	fclose( pMsgLog->File );
	pMsgLog->Mode = LOGMODE_NONE;
}

BOOL MsgLog_IsRecording(MsgLog * pMsgLog)
{
	return( pMsgLog->Mode == LOGMODE_RECORDING );
}

static BOOL MsgLog_ReadRecord( FILE * File, LogEntry * pLogEntry )
{

	char SigStr[4];
	char Buff[1000];
	char Label[1000];


	
	if( fgets( Buff, 1000, File ) == NULL )
		goto READRECORD_ERR;
	if( sscanf( Buff, "%s %d", Label, &pLogEntry->msg.message ) != 2)
		goto READRECORD_ERR;
		

	if( fgets( Buff, 1000, File ) == NULL )
		goto READRECORD_ERR;
	if( sscanf( Buff, "%s %d", Label, &pLogEntry->msg.wParam) != 2)
		goto READRECORD_ERR;

	if( fgets( Buff, 1000, File ) == NULL )
		goto READRECORD_ERR;
	if( sscanf( Buff, "%s %d", Label, &pLogEntry->msg.lParam) != 2)
		goto READRECORD_ERR;


	if( fgets( Buff, 1000, File ) == NULL )
		goto READRECORD_ERR;
	if( sscanf( Buff, "%s %d", Label, &pLogEntry->msg.pt.x) != 2)
		goto READRECORD_ERR;
		
		
	if( fgets( Buff, 1000, File ) == NULL )
		goto READRECORD_ERR;
	if( sscanf( Buff, "%s %d", Label, &pLogEntry->msg.pt.y) != 2)
		goto READRECORD_ERR;


	if( fgets( Buff, 1000, File ) == NULL )
		goto READRECORD_ERR;
	if( sscanf( Buff, "%s %d", Label, &pLogEntry->time) != 2)
		goto READRECORD_ERR;

	if( fgets( Buff, 1000, File ) == NULL )
		goto READRECORD_ERR;
	if( sscanf( Buff, "%c%c%c%c", &SigStr[3], &SigStr[2], &SigStr[1], &SigStr[0]) != 4)
		goto READRECORD_ERR;

	pLogEntry->WndSigniture = *((int32*)&SigStr);

	return( TRUE );
READRECORD_ERR:
	return( FALSE );
}

static BOOL MsgLog_Read( MsgLog * pMsgLog )
{
	int i;
	char Buff[1000];
	char Label[1000];

	if( !fgets( Buff, 1000, pMsgLog->File) )
		return( FALSE );

	sscanf( Buff, "%s%d", Label, &pMsgLog->EntryN );

	if( pMsgLog->Entries != NULL )
		jeRam_Free( pMsgLog->Entries );
	
	pMsgLog->Entries = JE_RAM_ALLOCATE_ARRAY_CLEAR( LogEntry, pMsgLog->EntryN );
	if( pMsgLog->Entries == NULL )
	{
		pMsgLog->EntryN = 0;
		return( FALSE );
	}
		
	for( i = 0; i < pMsgLog->EntryN; i++ )
	{
		if( ! MsgLog_ReadRecord( pMsgLog->File, &pMsgLog->Entries[i] ) )
			return( FALSE );
	}
	return( TRUE );
}



BOOL MsgLog_StartPlay( MsgLog * pMsgLog, char * Name )
{
	ASSERT( pMsgLog );
	ASSERT( Name );

	if( pMsgLog->File )
		fclose( pMsgLog->File );

	pMsgLog->File = fopen( Name, "r+");
	if( pMsgLog->File == 0 )
		return( FALSE );
	pMsgLog->StartTime = GetTickCount();

	if( !MsgLog_Read( pMsgLog ) )
	{
		fclose( pMsgLog->File );
		return( FALSE );
	}

	fclose( pMsgLog->File );
	pMsgLog->Mode = LOGMODE_PLAYING;
	pMsgLog->StartTime = GetTickCount();
	pMsgLog->CurEntry = 0;
	return( TRUE );
}

void MsgLog_EndPlay(MsgLog * pMsgLog)
{
	ASSERT( pMsgLog );
	ASSERT( pMsgLog->Mode == LOGMODE_PLAYING );

	if( pMsgLog->Entries != NULL )
	{
		jeRam_Free( pMsgLog->Entries );
		pMsgLog->Entries = NULL ;
	}
	pMsgLog->CurEntry = 0;
	pMsgLog->EntryN = 0;

	pMsgLog->Mode = LOGMODE_NONE;
}

void MsgLog_Play( MsgLog * pMsgLog )
{
	int Time;
	int CurEntry;

	ASSERT( pMsgLog );
	ASSERT( pMsgLog->Mode == LOGMODE_PLAYING );
	ASSERT(  pMsgLog->Entries );
	ASSERT(  pMsgLog->CurEntry < pMsgLog->EntryN );


	Time = GetTickCount();
	CurEntry = pMsgLog->CurEntry;
	while( Time - pMsgLog->StartTime > pMsgLog->Entries[CurEntry].time )
	{
		pMsgLog->Entries[CurEntry].msg.time = Time;
		pMsgLog->Entries[CurEntry].msg.hwnd = WndReg_GetWindow(pMsgLog->Entries[CurEntry].WndSigniture);
		if( pMsgLog->Entries[CurEntry].msg.hwnd != NULL )
		{
			if( pMsgLog->Entries[CurEntry].msg.message == WM_MOUSEMOVE )
				SetCursorPos( pMsgLog->Entries[CurEntry].msg.pt.x, pMsgLog->Entries[CurEntry].msg.pt.y);
			DispatchMessage( &pMsgLog->Entries[CurEntry].msg );
		}
		CurEntry++;
		if( CurEntry == pMsgLog->EntryN )
			break;
		Time = GetTickCount();
	}
	pMsgLog->CurEntry = CurEntry;
	if( CurEntry == pMsgLog->EntryN )
		MsgLog_EndPlay(pMsgLog);
}


BOOL MsgLog_IsPlaying(MsgLog * pMsgLog )
{
	return( pMsgLog->Mode == LOGMODE_PLAYING);
}
