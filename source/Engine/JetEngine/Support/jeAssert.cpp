/****************************************************************************************/
/*  GEASSERT.C                                                                          */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: Replacement for assert implementation                                  */
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
#include "jeAssert.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
	   
// See jeAssert.h for details.

//void jeAssertDefault(void *, void *, unsigned);
jeAssertCallbackFn *jeAssertCallback = &jeAssertDefault;

jeAssertCallbackFn *jeSetAssertCallback( jeAssertCallbackFn *newAssertCallback )
{
	jeAssertCallbackFn *oldCallback = jeAssertCallback;
	jeAssertCallback = newAssertCallback;
	return oldCallback;
}

void jeAssertDefault( void *exp, void *file, unsigned long line )
{
#ifdef _DEBUG
	//	by trilobite	Jan. 2011
	//_assert( (const char*)exp, (const char*)file, line );	
	assert( (const char*)exp, (const char*)file, line );	
#endif
}


// This might seem a little redundant, but I needed a unique name
// for the place that all jeAsserts would begin.  From here, I
// call the jeAssertCallback routine, whatever it has been 
// assigned to be. -Ken
void jeAssertEntryPoint( void *exp, void *file, unsigned line )
{
	jeAssertCallback( exp, file, line );
}

//-------------------------------------------

static jeAssert_CriticalShutdownCallback CriticalCallBack = NULL;
static uint32 CriticalCallBackContext = 0;

void jeAssert_SetCriticalShutdownCallback( jeAssert_CriticalShutdownCallback CB ,uint32 Context,
												jeAssert_CriticalShutdownCallback * pOldCB , uint32 * pOldContext)
{
	if ( pOldCB )
		*pOldCB = CriticalCallBack;
	if ( pOldContext )
		*pOldContext = CriticalCallBackContext;
	CriticalCallBack = CB;
	CriticalCallBackContext = Context;
}

/*#ifndef NDEBUG

#ifdef WIN32
#include <windows.h>
#endif

#include <signal.h>

#define MAX_ASSERT_STRING_LENGTH 4096

void __cdecl _assert (void *expr,void *filename,unsigned lineno)
{
int nCode;
char assertbuf[MAX_ASSERT_STRING_LENGTH];	
static int in_assert_cnt = 0; // a semaphore

	if ( in_assert_cnt )
		return;
	in_assert_cnt++;

	if ( (strlen((char *)expr) + strlen((char *)filename) + 100) < MAX_ASSERT_STRING_LENGTH )
		sprintf(assertbuf,"assert(%s) \n FILE %s : LINE : %d\n",(char *)expr,(char *)filename,(int)lineno);
	else
		sprintf(assertbuf," assert string longer than %d characters!\n",MAX_ASSERT_STRING_LENGTH);

#ifdef WIN32
    nCode = MessageBox(NULL,assertbuf,
        "Jet3D Exception",
        MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_SYSTEMMODAL);
#endif

#ifdef BUILD_BE
	BAlert* pAlert = new BAlert("Jet3d Exception" , assertbuf, "Abort" , "Retry", "Ignore");
	nCode = pAlert->Go();
#endif

    if (nCode == IDIGNORE)
	{
		in_assert_cnt --;
        return;
	}

    // Abort: abort the program
    if (nCode == IDABORT)
    {
		// CriticalCallBack does things like shut down the driver
		// if we retry or ignore, don't do it, so..
		if ( CriticalCallBack )
		{
			CriticalCallBack(CriticalCallBackContext);
		}

       // raise abort signal
       raise(SIGABRT);

        // We usually won't get here, but it's possible that
        //   SIGABRT was ignored.  So exit the program anyway.

        _exit(3);
    }

    // Retry: call the debugger
	// minimal code from here out so that the debugger can easily step back
	//	to the asserting line of code :

#ifdef WIN32
    if (nCode == IDRETRY)
        __asm { int 3 };
#endif
#ifdef BUILD_BE
	if (nCode == IDRETRY)
		__asm__ ("int $3");
#endif

	in_assert_cnt --;
}
#endif	// NDEBUG
*/
