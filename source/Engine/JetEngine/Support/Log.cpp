/****************************************************************************************/
/*  LOG.C                                                                               */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: Debugging logger implementation                                        */
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
#include <windows.h>
#endif

#ifdef BUILD_BE
#include <Debug.h>
#define OutputDebugString DEBUGGER
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "Xform3d.h"
#include "ThreadLog.h"
#include "Log.h"

static FILE * TeeFP = NULL; // should default to null ?

void Log_Out(const char * string)
{
	OutputDebugString(string);
	ThreadLog_Printf(string);
	if ( TeeFP )
		fprintf(TeeFP,string);
}


void Log_Puts(const char * string)
{
	Log_Out(string);
	Log_Out("\n");
}

void Log_Printf(const char * String, ...)
{
	va_list			ArgPtr;
    char			TempStr[4096];

	va_start(ArgPtr, String);
    vsprintf(TempStr, String, ArgPtr);
	va_end(ArgPtr);

	Log_Out(TempStr);
}

void Log_PrintMatrices(jeXForm3d* pXF)
{
    char			TempStr[4096];

	sprintf(TempStr, "Matrice:\n| %0.4f | %0.4f | %0.4f | %0.4f |\n| %0.4f | %0.4f | %0.4f | %0.4f |\n| %0.4f | %0.4f | %0.4f | %0.4f |\n",
		pXF->AX, pXF->AY, pXF->AZ, pXF->Translation.X,
		pXF->BX, pXF->BY, pXF->BZ, pXF->Translation.Y,
		pXF->CX, pXF->CY, pXF->CZ, pXF->Translation.Z);

	Log_Out(TempStr);
}

void Log_TeeFile( FILE * FP )
{
	TeeFP = FP;
}

#ifndef NO_LOG
#pragma message("LOG on")
#endif
