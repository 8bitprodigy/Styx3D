/****************************************************************************************/
/*  RAM.H                                                                               */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: Replacement for malloc, realloc and free                               */
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
// RAM memory manager

#ifndef JE_RAM_H
#define JE_RAM_H

// Memory debugging functionality only supported under Windows..
#ifndef WIN32
#ifndef NDEBUG
	#define REDEFINE_NDEBUGANDDEBUG
	#define NDEBUG
	#undef DEBUG
#endif
#endif

#include "BaseType.h"

#include "jeMemAllocInfo.h"	// Added by Icestorm

#ifdef __cplusplus
extern "C" {
#endif

/*******

CB note : do NOT do jeRam_Allocate then memset(mem,0,len) !!!
	
use jeRam_AllocateClear !

This function uses a very fast memory clearer!  The normal memset
causes L2 cache misses!

*******/

typedef int (* jeRam_CriticalCallbackFunction)(void);

/*
  Set the critical callback function.  ram_allocate will call the critical
  callback function if it's unable to allocate memory.
*/
JETAPI jeRam_CriticalCallbackFunction JETCC jeRam_SetCriticalCallback
    (
      jeRam_CriticalCallbackFunction callback
    );

/*
  increments or decrements a counter .  if the counter is >0
  the critical callback function (if set) is called for a failed memory allocation.
  add is added to the current counter value.  the new counter value is returned.
*/
JETAPI int JETCC jeRam_EnableCriticalCallback(int add);


/*
  Allocate memory of the given size.  In debug mode, the memory is filled
  with 0xA5, and we keep track of the amount of memory allocated.  Also, in debug
  mode, we track where the memory was allocated and can optionally provide a
  report of allocated blocks.  See jeRam_ReportAllocations.
*/

#ifndef NDEBUG

#ifndef JE_DEACTIVATE_JMAI	// Icestorm: Added jMAI+Breakpoint-Support

#define jeRam_Allocate(size) (_jeRam_DoBreakTest(_jeRam_DebugAllocate(size, __FILE__, __LINE__)))

// Do not call _jeRam_DebugAllocate directly.
JETAPI void* _jeRam_DebugAllocate(uint32 size, const char* pFile, int line);
JETAPI void* JETCC _jeRam_DoBreakTest(void *Pointer);	// Icestorm

#else // JE_DEACTIVATE_JMAI

#define jeRam_Allocate(size) _jeRam_DebugAllocate(size, __FILE__, __LINE__)

// Do not call _jeRam_DebugAllocate directly.
JETAPI void* JETCC _jeRam_DebugAllocate(uint32 size, const char* pFile, int line);

#endif // JE_DEACTIVATE_JMAI

#else

JETAPI void * JETCC jeRam_Allocate(uint32 size);

#endif

// allocate the ram & clear it. (calloc)
#ifndef JE_DEACTIVATE_JMAI //Icestorm : FILE/LINE correction for jMAI

#ifndef NDEBUG

#define jeRam_AllocateClear(size) (_jeRam_DoBreakTest(_jeRam_DebugAllocateClear(size, __FILE__, __LINE__)))

JETAPI void * JETCC _jeRam_DebugAllocateClear(uint32 size, const char* pFile, int line);

#else

JETAPI void * JETCC jeRam_AllocateClear(uint32 size);

#endif

#ifndef NDEBUG

JETAPI void JETCC jeRam_DebugFree_(void *ptr, const char* pFile, int line);

#define jeRam_Free(ptr) {jeRam_DebugFree_(ptr, __FILE__, __LINE__);(ptr)=NULL;}

#else

JETAPI void JETCC jeRam_Free_(void *ptr);

#define jeRam_Free(xxx) {jeRam_Free_(xxx);(xxx)=NULL;}

#endif

#else // JE_DEACTIVATE_JMAI

// allocate the ram & clear it. (calloc)
JETAPI void * JETCC jeRam_AllocateClear(uint32 size);

/*
  Free an allocated memory block.
*/
JETAPI void JETCC jeRam_Free_(void *ptr);

#define jeRam_Free(xxx) {jeRam_Free_(xxx);(xxx)=NULL;}

#endif // JE_DEACTIVATE_JMAI
/*
  Reallocate memory.  This function supports shrinking and expanding blocks,
  and will also act like ram_allocate if the pointer passed to it is NULL.
  It won't, however, free the memory if you pass it a 0 size.
*/
#ifndef NDEBUG

#define jeRam_Realloc(ptr, newsize) _jeRam_DebugRealloc(ptr, newsize, __FILE__, __LINE__)

// Do not call _jeRam_DebugRealloc directly.
JETAPI void* JETCC _jeRam_DebugRealloc(void* ptr, uint32 size, const char* pFile, int line);

#else

JETAPI void * JETCC jeRam_Realloc(void *ptr,uint32 newsize);

#endif

#ifndef NDEBUG

#include <stdio.h>

JETAPI void JETCC jeRam_ReportAllocations(void);

JETAPI void JETCC jeRam_ShowStats(FILE * ToFile);

#else

#define jeRam_ReportAllocations() 

#endif

#ifndef NDEBUG
    extern int32 jeRam_CurrentlyUsed;
    extern int32 jeRam_NumberOfAllocations;
    extern int32 jeRam_MaximumUsed;
    extern int32 jeRam_MaximumNumberOfAllocations;

JETAPI     void JETCC jeRam_AddAllocation(int n,uint32 size);
#else
    #define jeRam_AddAllocation(n,s)
#endif

#define JE_RAM_ALLOCATE_STRUCT(type)			(type *)jeRam_Allocate (sizeof (type))
#define JE_RAM_ALLOCATE_STRUCT_CLEAR(type)      (type *)jeRam_AllocateClear(sizeof (type))
#define JE_RAM_ALLOCATE_ARRAY(type,count)		(type *)jeRam_Allocate (sizeof (type) * (count))
#define JE_RAM_ALLOCATE_ARRAY_CLEAR(type,count)	(type *)jeRam_AllocateClear(sizeof (type) * (count))

#if 0 //{@@

#ifndef NDEBUG	// <> CB note : what the @*#$ is this XX ? This is a bad line, regardless!
#define JE_RAM_REALLOC_ARRAY(ptr,type,count)  (type *)jeRam_Realloc(  (ptr), sizeof(type) * (count) );{type *XX=(ptr);}
#else
#define JE_RAM_REALLOC_ARRAY(ptr,type,count)  (type *)jeRam_Realloc(  (ptr), sizeof(type) * (count) )
#endif

#else

#define JE_RAM_REALLOC_ARRAY(ptr,type,count)  (type *)jeRam_Realloc(  (ptr), sizeof(type) * (count) )

#endif //}

#ifdef NDEBUG
#define jeRam_IsValidPtr(ptr)	(JE_TRUE)
#else
jeBoolean jeRam_IsValidPtr(const void *ptr);
#endif

#ifdef __cplusplus
  }
#endif

#ifdef REDEFINE_NDEBUGANDDEBUG
#undef NDEBUG
#define DEBUG
#endif

#endif
