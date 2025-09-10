/****************************************************************************************/
/*  RAM.C                                                                               */
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
// RAM Memory manager

//#define DO_REPORT

#ifdef BUILD_BE // Memory debugging functionality only available under Win32
#ifndef NDEBUG

#define BEOS_REDEFINE_NDEBUG
#define NDEBUG
#undef DEBUG

#endif
#endif

#include <memory.h>
#include <malloc.h>
#include <assert.h>

//#ifndef NDEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//#endif

#ifdef DO_REPORT
#include "Report.h"
REPORT_VARS(MemoryAllocations);
#endif

#include "Ram.h"

#ifndef NDEBUG
#ifndef _LOG
#define _LOG
#endif
#include "Log.h"
#endif

//#define PAD	(8)		// <> use 8 for MMX ? use 16 if on Katmai! // commented out by trilobite dec. 2021
// alternate version by trilobite dec.2021
#define PAD (16)

					// rounds up to the nearest multiple
#define PAD_SIZE(size)	((size+PAD-1)&(~(uint32)(PAD-1)))

/*
  This controls the MINIMAL_CONFIG flag.  Basically, all overflow, underflow,
  and size checking code is always enabled except when NDEBUG is defined...
*/
#ifdef NDEBUG
  // debugging's turned off, so make it minimal config
  #ifndef MINIMAL_CONFIG
	#define MINIMAL_CONFIG
  #endif
#else
  // debugging on, so do full checking
  #ifdef MINIMAL_CONFIG
	#undef MINIMAL_CONFIG
  #endif
#endif

// stupid stuff...
#ifndef JETDLLVERSION
void *StupidUnusedPointer;
#endif

// critical allocation stuff...
static int jeRam_CriticalAllocationCount = 0;

static jeRam_CriticalCallbackFunction jeRam_CriticalCallback = NULL;

/*
  increments or decrements a counter.  if the counter is >0
  the critical callback function (if set) is called for a failed memory allocation.
  add is added to the current counter value.  the new counter value is returned.
*/
JETAPI int JETCC jeRam_EnableCriticalCallback(int add)
{
	jeRam_CriticalAllocationCount += add;
	return jeRam_CriticalAllocationCount;
}


/*
  Set the critical callback function.  jeRam_Allocate will call this function
  if it's unable to allocate memory.  Returns the previous critical callback fcn.
*/
JETAPI jeRam_CriticalCallbackFunction JETCC jeRam_SetCriticalCallback
	(
	  jeRam_CriticalCallbackFunction critical_callback
	)
{
	jeRam_CriticalCallbackFunction OldCallback;

	OldCallback = jeRam_CriticalCallback;
	jeRam_CriticalCallback = critical_callback;
	return OldCallback;
}

/*
  If an allocation fails, this function will be called.  If the critical callback
  function is not NULL, then that function will be called.
*/
static int jeRam_DoCriticalCallback
	(
	  void
	)
{
	if ((jeRam_CriticalAllocationCount != 0) && (jeRam_CriticalCallback != NULL))
	{
		return jeRam_CriticalCallback ();
	}
	else
	{
		return 0;
	}
}

#ifndef JE_DEACTIVATE_JMAI

#ifdef MINIMAL_CONFIG

	JETAPI void * JETCC jeRam_AllocateClear(uint32 size)
	{
	void * mem;
		size = (size + 3)&(~(uint32)3);
		mem = jeRam_Allocate(size);
		if ( mem )
		{
			memset (mem, 0, size);
		}
	return mem;
	}
#endif

#else

	JETAPI void * JETCC jeRam_AllocateClear(uint32 size)
	{
	void * mem;
		size = (size + 3)&(~(uint32)3);
		mem = jeRam_Allocate(size);
		if ( mem )
		{
			memset (mem, 0, size);
		}
	return mem;
	}

#endif

#ifdef MINIMAL_CONFIG

	/*
	  Minimal configuration acts almost exactly like standard malloc, free,
	  and realloc.  The only difference is the critical allocation stuff.
	*/


	/*
	  Allocate memory of the given size.  In debug mode, the memory is filled
	  with 0xA5, and we keep track of the amount of memory allocated.
	*/
	JETAPI void * JETCC jeRam_Allocate
		(
		  uint32 size
		)
	{
		void *p;

//		size = PAD_SIZE(size);

		do
		{
			p = malloc(size);
		} while ((p == NULL) && (jeRam_DoCriticalCallback ()));


		return p;
	}

	// free an allocated block
	JETAPI void JETCC jeRam_Free_
		(
		  void *ptr
		)
	{
	  free (ptr);
	}

	// reallocate a block...
	// This acts like the standard realloc
JETAPI	 void * JETCC jeRam_Realloc
		(
		  void *ptr,
		  uint32 newsize
		)
	{
		char *p;
		char * NewPtr;

		if (ptr == NULL)
		{
			return jeRam_Allocate (newsize);
		}

		// if newsize is NULL, then it's a free and return NULL
		if (newsize == 0)
		{
			jeRam_Free (ptr);
			return NULL;
		}

		p = (char *)ptr;
		do
		{
			NewPtr = (char *)realloc (p, newsize);
		} while ((NewPtr == NULL) && (jeRam_DoCriticalCallback ()));

		return NewPtr;
	}

#else  // MINIMAL_CONFIG
	 /*
	   For debugging implementations, we add a header and trailer to the
	   allocated memory blocks so that we compute memory usage, and catch
	   simple over- and under-run errors.
	 */

#ifndef JE_DEACTIVATE_JMAI
	extern void JETCC jeMemAllocInfo_Alloc(uint32 Size, void *Pointer, const char *FName, int LNr);
	extern void JETCC jeMemAllocInfo_Realloc(uint32 Size, void *Pointer, const char *FName, int LNr);
	extern void JETCC jeMemAllocInfo_Free(void *Pointer, const char *FName, int LNr);
	extern jeBoolean JETCC jeMemAllocInfo_BREAK(const void *Data);
	static int32 jeRam_jMAI_Flag = 1;					// Flag for jMAI-Memory-Calls
	static const int DataSize = sizeof (uint32);		//Icestorm:Pointer(to respective jMAI_struct)size
#endif

	// yes, this will break if we use more than 2 gigabytes of RAM...
	int32 jeRam_CurrentlyUsed	   = 0;  // total ram currently in use
	int32 jeRam_MaximumUsed		 = 0;  // max total ram allocated at any time
	int32 jeRam_NumberOfAllocations	 = 0;  // current number of blocks allocated
	int32 jeRam_MaximumNumberOfAllocations = 0;  // max number of allocations at any time

	// header and trailer stuff...
	static char MemStamp[] = {"!CHECKME!"};
	static const int MemStampSize = sizeof (MemStamp)-1;
	static const int SizeSize = sizeof (uint32);
	
	// these pads are critical !
#ifndef JE_DEACTIVATE_JMAI
	#define SIZES_SIZE		(SizeSize+DataSize)		// Icestorm: added DataSize...
	#define HEADER_SIZE		PAD_SIZE(SIZES_SIZE		+ MemStampSize)	// Icestorm: added DataSize...
	#define EXTRA_SIZE		PAD_SIZE(HEADER_SIZE	+ MemStampSize)
#else
	#define HEADER_SIZE		PAD_SIZE(SizeSize		+ MemStampSize)
	#define EXTRA_SIZE		PAD_SIZE(HEADER_SIZE	+ MemStampSize)
#endif

	static const unsigned char AllocFillerByte = (unsigned char)0xA5;
	static const unsigned char FreeFillerByte  = (unsigned char)0xB6;

#ifndef JE_DEACTIVATE_JMAI
	/*
	  A memory block is allocated that's size + (2*MemStampSize)+SizeSize+DataSize bytes.
	  It's then filled with 0xA5.  The size stamp is placed at the head of the block,
	  with the MemStamp being placed directly after the pointer at the front, and
	  also at the end of the block.  The layout is:

	  <size><jMAIPointer><MemStamp><<allocated memory>><MemStamp>
	*/
#else
	/*
	  A memory block is allocated that's size + (2*MemStampSize)+SizeSize bytes.
	  It's then filled with 0xA5.  The size stamp is placed at the head of the block,
	  with the MemStamp being placed directly after the size at the front, and
	  also at the end of the block.  The layout is:

	  <size><MemStamp><<allocated memory>><MemStamp>
	*/
#endif

	typedef enum 
	{
		DONT_INITIALIZE = 0, 
		INITIALIZE_MEMORY = 1
	} jeRam_MemoryInitialization;

#ifndef JE_DEACTIVATE_JMAI
	// jMAI-Breakpoint-function
	JETAPI void* JETCC _jeRam_DoBreakTest(void *Pointer)
	{
		if (jeMemAllocInfo_BREAK(*(void**)((uint32)Pointer+SizeSize-HEADER_SIZE)) == JE_TRUE)
			_asm { int 3h }
		return Pointer;
	}
#endif

	static void jeRam_SetupBlock
		  (
			char * p,
			uint32 size,
			jeRam_MemoryInitialization InitMem
		  )
	{
		if (InitMem == INITIALIZE_MEMORY)
		{
			// fill the memory block
			memset (p+HEADER_SIZE, AllocFillerByte, size);
		}

		// add the size at the front
		*((uint32 *)p) = size;

#ifndef JE_DEACTIVATE_JMAI
		*(void* *)((uint32)p+SizeSize)=NULL;	// Icestorm : DON'T CHANGE THIS!!!
												// Init. jMAI_Ptr to NULL, prevents "death-jumps to nowhere"
		// copy the memstamp to the front of the block
		memcpy (p+SIZES_SIZE, MemStamp, MemStampSize);
#else
		// copy the memstamp to the front of the block
		memcpy (p+SizeSize, MemStamp, MemStampSize);
#endif

		// and to the end of the block
		memcpy (p+HEADER_SIZE+size, MemStamp, MemStampSize);
	}

#ifndef JE_DEACTIVATE_JMAI
#ifndef NDEBUG	// Added File/Line-Support => jMAI can record "real" file/line

JETAPI void * JETCC _jeRam_DebugAllocateClear(uint32 size, const char* pFile, int line)
{
void * mem;
	size = (size + 3)&(~(uint32)3);
	mem = _jeRam_DebugAllocate(size, pFile, line);
	if ( mem )
	{
		memset (mem, 0, size);
	}
return mem;
}

#else

JETAPI void * JETCC jeRam_AllocateClear(uint32 size)
{
void * mem;
	size = (size + 3)&(~(uint32)3);
	mem = jeRam_Allocate(size);
	if ( mem )
	{
		memset (mem, 0, size);
	}
return mem;
}

#endif
#endif	// JE_DEACTIVATE_JMAI
/*
This function, (jeRam_DebugAllocate) is the source of much misery in the Jet3D engine.
It is the cause of fatal exceptions that prevent applications from initializing, or makes them buggy and unstable.
I wish I could find a way to fix it. -- trilobite dec. 2021.
*/


#ifndef NDEBUG
JETAPI 	void* JETCC _jeRam_DebugAllocate(uint32 size, const char* pFile, int line)
	{
	  char *p;

//		size = PAD_SIZE(size);

	  do
	  {
		 
		  p = (char*)malloc(size + EXTRA_SIZE);//, _NORMAL_BLOCK, pFile, line);
	  } while ((p == NULL) && jeRam_DoCriticalCallback ());

	  if (p == NULL)
	  {
		 return NULL;
	  }

	  // setup size stamps and memory overwrite checks
	  jeRam_SetupBlock (p, size, INITIALIZE_MEMORY);

		jeRam_AddAllocation(1,size);

#ifndef JE_DEACTIVATE_JMAI
		if (jeRam_jMAI_Flag)
			jeMemAllocInfo_Alloc(size, p+SizeSize, pFile, line); //Icestorm
		//NOTE:If this jeRam_Alloc is called from jMAI-Module, jMAI_Ptr is still NULL!!
		//     Also deactivated jMAI leave the jMAI_Ptr NULL.
		//     So registered and unregistered memory can be distinguished
#endif

	  return p+HEADER_SIZE;
	}

#else // NDEBUG

JETAPI	 void * JETCC jeRam_Allocate (uint32 size)
	{
	  char *p;

//		size = PAD_SIZE(size);

	  do
	  {
		  p = (char*)malloc (size + EXTRA_SIZE);
	  } while ((p == NULL) && jeRam_DoCriticalCallback ());

	  if (p == NULL)
	  {
		 return NULL;
	  }

	  // setup size stamps and memory overwrite checks
	  jeRam_SetupBlock (p, size, INITIALIZE_MEMORY);

		jeRam_AddAllocation(1,size);

	  return p+HEADER_SIZE;
	}
#endif // NDEBUG

	static char * ram_verify_block
		  (
			void * ptr
		  )
	{
		char * p = (char *)ptr;
		uint32 size;

		if (p == NULL)
		{
			assert (0 && "freeing NULL");
			return NULL;
		}

		// make p point to the beginning of the block
		p -= HEADER_SIZE;

		// get size from block
		size = *((uint32 *)p);

#ifndef JE_DEACTIVATE_JMAI
		// check stamp at front
		if (memcmp (p+SIZES_SIZE, MemStamp, MemStampSize) != 0)
		{
			assert (0 && "ram_verify_block:  Memory block corrupted at front");
			return NULL;
		}
#else
		// check stamp at front
		if (memcmp (p+SizeSize, MemStamp, MemStampSize) != 0)
		{
			assert (0 && "ram_verify_block:  Memory block corrupted at front");
			return NULL;
		}
#endif

		// and at back
		if (memcmp (p+HEADER_SIZE+size, MemStamp, MemStampSize) != 0)
		{
			assert (0 && "ram_verify_block:  Memory block corrupted at tail");
			return NULL;
		}

		return p;
	}

#ifndef JE_DEACTIVATE_JMAI

#ifndef NDEBUG
JETAPI	 void JETCC jeRam_DebugFree_ (void *ptr, const char* pFile, int line)
	{
		char *p;
		uint32 size;
		void *jMAI_Ptr;	// Icestorm

		// make sure it's a valid block...
		p = ram_verify_block (ptr);
		if (p == NULL)
		{
			return;
		}

		// gotta get the size before you free it
		size = *((uint32 *)p);

		jMAI_Ptr = *(void**)((uint32)p+SizeSize); // Icestorm: Get this jMAI_Ptr

		// fill it with trash...
		memset (p, FreeFillerByte, size+EXTRA_SIZE);

		// free the memory
		free (p);

		// Icestorm Begin
		if(jMAI_Ptr!=NULL)		// (un)registered memoryblock?
		{
			jMAI_Ptr=ram_verify_block(jMAI_Ptr);	//Verify jMAI_Ptr
			if(jMAI_Ptr!=NULL)
				jMAI_Ptr=(void*)((uint32)jMAI_Ptr+HEADER_SIZE);
		} 

		jeMemAllocInfo_Free(jMAI_Ptr, pFile, line);
		//Icestorm End

		// update allocations
		jeRam_NumberOfAllocations--;
		assert ((jeRam_NumberOfAllocations >= 0) && "free()d more ram than you allocated!");

		jeRam_CurrentlyUsed -= size;
		assert ((jeRam_CurrentlyUsed >= 0) && "free()d more ram than you allocated!");
	}

#else

JETAPI	 void JETCC jeRam_Free_ (void *ptr)
	{
		char *p;
		uint32 size;

		// make sure it's a valid block...
		p = ram_verify_block (ptr);
		if (p == NULL)
		{
			return;
		}

		// gotta get the size before you free it
		size = *((uint32 *)p);

		// fill it with trash...
		memset (p, FreeFillerByte, size+EXTRA_SIZE);

		// free the memory
		free (p);

		// update allocations
		jeRam_NumberOfAllocations--;
		assert ((jeRam_NumberOfAllocations >= 0) && "free()d more ram than you allocated!");

		jeRam_CurrentlyUsed -= size;
		assert ((jeRam_CurrentlyUsed >= 0) && "free()d more ram than you allocated!");
	}

#endif
#else  //JE_DEACTIVATE_JMAI

JETAPI	 void JETCC jeRam_Free_ (void *ptr)
	{
		char *p;
		uint32 size;

		// make sure it's a valid block...
		p = ram_verify_block (ptr);
		if (p == NULL)
		{
			return;
		}

		// gotta get the size before you free it
		size = *((uint32 *)p);

		// fill it with trash...
		memset (p, FreeFillerByte, size+EXTRA_SIZE);

		// free the memory
		free (p);

		// update allocations
		jeRam_NumberOfAllocations--;
		assert ((jeRam_NumberOfAllocations >= 0) && "free()d more ram than you allocated!");

		jeRam_CurrentlyUsed -= size;
		assert ((jeRam_CurrentlyUsed >= 0) && "free()d more ram than you allocated!");
	}

#endif  //JE_DEACTIVATE_JMAI

#ifndef NDEBUG

JETAPI	 void * JETCC _jeRam_DebugRealloc (void *ptr, uint32 newsize, const char* pFile, int line)
	{
		char *p;
		char * NewPtr;
#ifndef JE_DEACTIVATE_JMAI
		void *jMAI_Ptr;	// Icestorm
#endif
		uint32 size;

		// if realloc is called with NULL, just treat it like an alloc
		if (ptr == NULL)
		{
#ifndef JE_DEACTIVATE_JMAI //Icestorm: added Breakpoint
			return _jeRam_DoBreakTest(_jeRam_DebugAllocate(newsize, pFile, line));
#else
			return _jeRam_DebugAllocate(newsize, pFile, line);
#endif
		}

		// verify the block
		p = ram_verify_block (ptr);
		if (p == NULL)
		{
			return NULL;
		}

		// if newsize is NULL, then it's a free and return NULL
		if (newsize == 0)
		{
#ifndef JE_DEACTIVATE_JMAI 		// Icestorm: again pFile/line-correction
			jeRam_DebugFree_(ptr, pFile, line);
#else
			jeRam_Free (ptr);
#endif
			return NULL;
		}

		// gotta get the size before I realloc it...
		size = *((uint32 *)p);

#ifndef JE_DEACTIVATE_JMAI
		jMAI_Ptr = *(void**)((uint32)p+SizeSize); // Icestorm: Get this jMAI_Ptr
#endif

		do
		{
			NewPtr = (char *)realloc(p, newsize+EXTRA_SIZE);//, _NORMAL_BLOCK, pFile, line);
		} while ((NewPtr == NULL) && jeRam_DoCriticalCallback ());

		// if allocation failed, return NULL...
		if (NewPtr == NULL)
		{
			return NULL;
		}

		jeRam_SetupBlock (NewPtr, newsize, DONT_INITIALIZE);

		jeRam_AddAllocation(0,newsize - size);

#ifndef JE_DEACTIVATE_JMAI
		if(jMAI_Ptr!=NULL)		// (un)registered?
		{
			jMAI_Ptr=ram_verify_block(jMAI_Ptr);	//Check jMAI_Ptr
			if(jMAI_Ptr!=NULL)
				jMAI_Ptr=(void*)((uint32)jMAI_Ptr+HEADER_SIZE);
		} 

		*(void* *)((uint32)NewPtr+SizeSize)=jMAI_Ptr;	//Restore jMAI_Ptr or set unregistered-Mark(NULL)
		jeMemAllocInfo_Realloc(newsize, NewPtr+SizeSize, pFile, line);
#endif

		return NewPtr + HEADER_SIZE;
	}

#else // NDEBUG

JETAPI	 void * jeRam_Realloc (void *ptr, uint32 newsize)
	{
		char *p;
		char * NewPtr;
		uint32 size;

		// if realloc is called with NULL, just treat it like an alloc
		if (ptr == NULL)
		{
			return jeRam_Allocate (newsize);
		}

		// verify the block
		p = ram_verify_block (ptr);
		if (p == NULL)
		{
			return NULL;
		}

		// if newsize is NULL, then it's a free and return NULL
		if (newsize == 0)
		{
			jeRam_Free (ptr);
			return NULL;
		}

		// gotta get the size before I realloc it...
		size = *((uint32 *)p);

		do
		{
			NewPtr = (char *)realloc (p, newsize+EXTRA_SIZE);
		} while ((NewPtr == NULL) && jeRam_DoCriticalCallback ());

		// if allocation failed, return NULL...
		if (NewPtr == NULL)
		{
			return NULL;
		}

		jeRam_SetupBlock (NewPtr, newsize, DONT_INITIALIZE);

		jeRam_AddAllocation(0,newsize - size);

		return NewPtr + HEADER_SIZE;
	}

#endif // NDEBUG

#ifndef NDEBUG

JETAPI void JETCC jeRam_ReportAllocations(void)
{
//	_CrtDumpMemoryLeaks();
}

#include <stdio.h>

JETAPI void JETCC jeRam_ShowStats(FILE * ToFile)
{
	if ( ! ToFile ) ToFile = stdin;

#ifdef DO_REPORT
	reportFP = ToFile;
	REPORT_REPORT(MemoryAllocations);
#endif

	Log_TeeFile(ToFile);
	Log_Printf("jeRam : Used : Currently = %d, Max = %d\n",
		jeRam_CurrentlyUsed,jeRam_MaximumUsed);
	Log_Printf("jeRam : NumAllocs : Currently = %d, Max = %d\n",
		jeRam_NumberOfAllocations,jeRam_MaximumNumberOfAllocations);
}

#endif

	// for external programs that allocate memory some other way.
	// Here they can use ram to keep track of the memory.
JETAPI	 void JETCC jeRam_AddAllocation (int n, uint32 size)
{
	// and update the allocations stuff
	jeRam_NumberOfAllocations += n;
	jeRam_CurrentlyUsed += size;

	if (jeRam_NumberOfAllocations > jeRam_MaximumNumberOfAllocations)
	{
		jeRam_MaximumNumberOfAllocations = jeRam_NumberOfAllocations;
	}
	if (jeRam_CurrentlyUsed > jeRam_MaximumUsed)
	{
		jeRam_MaximumUsed = jeRam_CurrentlyUsed;
	}
	
	assert ((jeRam_CurrentlyUsed >= 0) && "free()d more ram than you allocated!");

#ifdef DO_REPORT
	{
	int MemoryAllocations;
		MemoryAllocations = size;
		REPORT_ADD(MemoryAllocations);
	}
#endif
}

#endif // MINIMAL_CONFIG


#ifndef JE_DEACTIVATE_JMAI
void jeRam_jMAI_Lock()
{
	jeRam_jMAI_Flag=0;
}

void jeRam_jMAI_UnLock()
{
	jeRam_jMAI_Flag=1;
}
#endif //JE_DEACTIVATE_JMAI


#ifndef NDEBUG
jeBoolean jeRam_IsValidPtr(const void *ptr)
{
const char * p = (const char *)ptr;
uint32 size;

	if (p == NULL) return JE_FALSE;

	// make p point to the beginning of the block
	p -= HEADER_SIZE;

	// get size from block
	size = *((uint32 *)p);

#ifndef JE_DEACTIVATE_JMAI
	// check stamp at front
	if (memcmp (p+SIZES_SIZE, MemStamp, MemStampSize) != 0)
	{
		return JE_FALSE;
	}
#else
	// check stamp at front
	if (memcmp (p+SizeSize, MemStamp, MemStampSize) != 0)
	{
		return JE_FALSE;
	}
#endif // JE_DEACTIVATE_JMAI

	// and at back
	if (memcmp (p+HEADER_SIZE+size, MemStamp, MemStampSize) != 0)
	{
		return JE_FALSE;
	}

return JE_TRUE;
}
#endif

#ifdef BEOS_REDEFINE_NDEBUG
#undef NDEBUG
#define DEBUG
#endif

