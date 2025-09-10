/****************************************************************************************/
/*  ERRORLOG.H                                                                          */
/*                                                                                      */
/*  Author: Mike Sandige                                                                */
/*  Description: Generic error logging system interface                                 */
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
/*  
	Simple error id logger.
	errors are logged to this object using ErrorLog_Add()
	errors are retrieved using ErrorLog_Report()

	created:  Mike Sandige 1/2/98
*/


#ifndef JE_ERRORLOG_H
#define JE_ERRORLOG_H

#include "basetype.h"

//#ifndef NDEBUG 
	#define ERRORLOG_FULL_REPORTING
//#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
	Temporary structure forward, to hold place for when jeThreadQueue is fully
	implemented.
*/
typedef	struct	jeErrorLog	jeErrorLog;

typedef enum
{
//do not use these errors - use the next list
	JE_ERR_DRIVER_INIT_FAILED=500,				// Could not init Driver
//do not use these errors - use the next list
	JE_ERR_DRIVER_NOT_FOUND,				// File open error for driver
//do not use these errors - use the next list
	JE_ERR_DRIVER_NOT_INITIALIZED,			// Driver shutdown failure
	JE_ERR_INVALID_DRIVER,					// Wrong driver version, or bad driver
	JE_ERR_DRIVER_BEGIN_SCENE_FAILED,
	JE_ERR_DRIVER_END_SCENE_FAILED,
//do not use these errors - use the next list
	JE_ERR_NO_PERF_FREQ,
	JE_ERR_FILE_OPEN_ERROR,
//do not use these errors - use the next list
	JE_ERR_INVALID_PARMS,
	JE_ERR_OUT_OF_MEMORY,
} jeErrorLog_ErrorIDEnumType;

typedef enum 
{
	JE_ERR_MEMORY_RESOURCE,
	JE_ERR_DISPLAY_RESOURCE,
	JE_ERR_SOUND_RESOURCE,
	JE_ERR_SYSTEM_RESOURCE,
	JE_ERR_INTERNAL_RESOURCE,
	
	JE_ERR_FILEIO_OPEN,
	JE_ERR_FILEIO_CLOSE,
	JE_ERR_FILEIO_READ,
	JE_ERR_FILEIO_WRITE,
	JE_ERR_FILEIO_FORMAT,
	JE_ERR_FILEIO_VERSION,
	
	JE_ERR_LIST_FULL,
	JE_ERR_DATA_FORMAT,
	JE_ERR_BAD_PARAMETER,
	JE_ERR_SEARCH_FAILURE,

	JE_ERR_WINDOWS_API_FAILURE,
	JE_ERR_SUBSYSTEM_FAILURE,
	JE_ERR_SHADER_SCRIPT, //added (cyrius)
	JE_ERR_PARSE_ERROR, //added (cyrius)
	JE_ERR_PARSE_FAILURE, //added (cyrius)

} jeErrorLog_ErrorClassType;

JETAPI void JETCC jeErrorLog_Clear(void);
	// clears error history

JETAPI int  JETCC jeErrorLog_Count(void);
	// reports size of current error log

JETAPI void JETCC jeErrorLog_AddExplicit(jeErrorLog_ErrorClassType,
	const char *ErrorIDString,
	const char *ErrorFileString,
	int LineNumber,
	const char *UserString,
	const char *Context);
	// not intended to be used directly: use ErrorLog_Add or ErrorLog_AddString


#ifdef ERRORLOG_FULL_REPORTING
	// 'Debug' version includes a textual error id, and the user string

	#define jeErrorLog_Add(Error, Context) jeErrorLog_AddExplicit((jeErrorLog_ErrorClassType)(Error), #Error, __FILE__, __LINE__,"", Context)
		// logs an error.  

	#define jeErrorLog_AddString(Error,String, Context) jeErrorLog_AddExplicit((jeErrorLog_ErrorClassType)(Error), #Error, __FILE__,__LINE__, String, Context)
		// logs an error with additional identifing string.  
	
JETAPI	jeBoolean JETCC jeErrorLog_AppendStringToLastError(const char *String);// use jeErrorLog_AppendString

	#define jeErrorLog_AppendString(XXX) jeErrorLog_AppendStringToLastError(XXX)
		// adds text to the previous logged error

#else
	// 'Release' version does not include the textual error id, or the user string

	#define jeErrorLog_Add(Error, Context) jeErrorLog_AddExplicit((jeErrorLog_ErrorClassType)(Error), "", __FILE__, __LINE__,"", Context)
		// logs an error.  

	#define jeErrorLog_AddString(Error,String, Context) jeErrorLog_AddExplicit((jeErrorLog_ErrorClassType)(Error), "", __FILE__,__LINE__, "", Context)
		// logs an error with additional identifing string.  
	
	#define jeErrorLog_AppendString(XXX)
		// adds text to the previous logged error

#endif

JETAPI const char * JETCC jeErrorLog_IntToString(int Number);
	// turns Number into a string.  uses a fixed static character string.  
	// for use with the context parameter of jeErrorLog_AddString()

JETAPI jeBoolean JETCC jeErrorLog_Report(int History, jeErrorLog_ErrorClassType *Error, const char **UserString, const char **Context);
	// reports from the error log.  
	// history is 0 for most recent,  1.. for second most recent etc.
	// returns JE_TRUE if report succeeded.  JE_FALSE if it failed.

#ifdef __cplusplus
}
#endif

#endif
