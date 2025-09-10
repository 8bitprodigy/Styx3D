/****************************************************************************************/
/*  THREADLOG.C                                                                         */
/*                                                                                      */
/*  Author:  Eli Boling                                                                 */
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
#ifdef WIN32
#define	WIN32_LEAN_AND_MEAN
#include	<windows.h>
#include	<mmsystem.h>
#endif

#include 	<string.h>
#include	<stdio.h>
#include	<assert.h>
#include	<stdarg.h>
#include	<stdlib.h>

#include	"VFile.h"	// Oy!

#ifdef BUILD_BE
#include <OS.h>
area_id logEntriesArea;
#define DWORD long // not really a dword..
#define GetCurrentThreadId() find_thread(NULL)
#define timeGetTime real_time_clock
#endif

#pragma message("ThreadLog includes vfile!")

#include	"ThreadLog.h"

#define	ENABLE_LOGGING

typedef	struct	ThreadLog_Entry
{
	uint32	TimeStamp;
	uint32	ThreadId;
	char	Msg[256];
}	ThreadLog_Entry;

#define	MAXENTRIES	((1024 * 1024 * 20) / sizeof(ThreadLog_Entry))
static	ThreadLog_Entry	*	LogEntries = NULL;
static	ThreadLog_Entry *	CurrentEntry = NULL;
static	DWORD				StartTime = 0;
static  jeBoolean			Filling = JE_FALSE;

jeBoolean ThreadLog_Initialize(void)
{
#ifdef	ENABLE_LOGGING
	if ( LogEntries )
		return JE_TRUE;

#ifdef WIN32
	// <> this is a memory leak
	LogEntries = (ThreadLog_Entry*)VirtualAlloc(NULL, MAXENTRIES * sizeof(ThreadLog_Entry), MEM_COMMIT, PAGE_READWRITE);
	if	(!LogEntries)
		return JE_FALSE;
#endif
#ifdef BUILD_BE
	logEntriesArea = create_area("LogEntries Area" , (void **)&LogEntries , B_ANY_ADDRESS, MAXENTRIES * sizeof(ThreadLog_Entry), B_NO_LOCK , B_READ_AREA | B_WRITE_AREA );
#endif
	
	CurrentEntry = LogEntries;

	StartTime = timeGetTime();
#endif

	return JE_TRUE;
}

void	ThreadLog_Printf(const char *Msg, ...)
{
#ifdef	ENABLE_LOGGING
	ThreadLog_Entry *	Entry;
	va_list				ap;

	if ( ! LogEntries )
		return;

	va_start(ap, Msg);

	#pragma message ("ThreadLog is susceptible to premption err-ors here")
	/*
		We accept the risk of preemption in exchange for performance of the log.  The
		point is to rapidly log events from multiple threads, so we can get a rough idea
		of when different things happened with at least millisecond accuracy.  If we
		start using critical sections to guard this stuff, the numbers will be useless.
		That's why we allocate a HUGE log buffer at the outset.
	*/

	Entry = CurrentEntry;

	if ( Filling )
	{
		vsprintf(Entry->Msg + strlen(Entry->Msg), Msg, ap);
	}
	else
	{
		Entry->TimeStamp = timeGetTime() - StartTime;
		Entry->ThreadId = GetCurrentThreadId();
		vsprintf(Entry->Msg, Msg, ap);
	}
	va_end(ap);
	Entry->Msg[ sizeof(Entry->Msg) - 1] = 0;
	if ( strrchr(Entry->Msg,'\n') )
	{
		CurrentEntry++;
		Filling = JE_FALSE;
	}
	else
		Filling = JE_TRUE;
#endif
}

jeBoolean	ThreadLog_Report(const char *FileName)
{
#ifdef	ENABLE_LOGGING
	ThreadLog_Entry *	Entry;
	jeVFile *			Out;

	if	(!LogEntries)
		return JE_FALSE;

	Out = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_DOS, FileName, NULL, JE_VFILE_OPEN_CREATE);
	if	(!Out)
		return JE_FALSE;

	Entry = LogEntries;
	while	(Entry < CurrentEntry)
	{
		int		Length;
		char	Buff[8 + 2 + 8 + 3 + 1 + sizeof(Entry->Msg)];

		sprintf(Buff, "%8d  %8x:  %s", Entry->TimeStamp, Entry->ThreadId, Entry->Msg);
		Length = strlen(Buff);
		if	(jeVFile_Write(Out, Buff, Length) == JE_FALSE)
		{
			jeVFile_Close(Out);
			return JE_FALSE;
		}
		Entry++;
	}

	jeVFile_Close(Out);
#endif

	return JE_TRUE;
}

