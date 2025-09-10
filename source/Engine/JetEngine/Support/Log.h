/****************************************************************************************/
/*  LOG.H                                                                               */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: Debugging logger interface                                             */
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
#ifndef JE_LOG_H
#define JE_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/*****

the Log protos :

void Log_Puts(	const char * string);
void Log_Printf(const char * string, ...);
void Log_TeeFile( FILE * FP );	// defaults to stdout

note : Log also writes out to ThreadLog !
use ThreadLog_Report() to dump data

*****/

#ifndef	NO_LOG

#include <stdio.h>
#include "xform3d.h"

void Log_Puts(	const char * string);
void Log_Printf(const char * string, ...);
void Log_TeeFile( FILE * FP );

// Wrapper to trace matrices - dont print the last row
void Log_PrintMatrices(jeXForm3d* pXF);

#else	// NO_LOG

#pragma warning (disable:4100)
static _inline void Log_Printf(const char * str, ...) { }
#pragma warning (default:4100)

#define Log_Puts(string)
#define Log_TeeFile(f)

#define Log_PrintMatrices(pXF)

#endif	// NO_LOG

#ifdef __cplusplus
}
#endif

#endif // LOG_H
