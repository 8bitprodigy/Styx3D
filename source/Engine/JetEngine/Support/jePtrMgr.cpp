/****************************************************************************************/
/*  JEPTRMGR.C                                                                          */
/*                                                                                      */
/*  Author:  John Pollard                                                               */
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
#include <string.h>
#include <assert.h>

// Public Dependents
#include "jePtrMgr.h"

// Private dependents
#include "Ram.h"
#include "Errorlog.h"
#include "jePtrMgr._h"

//========================================================================================
//========================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

static jePtrMgr_SEntry *PushSEntry(jePtrMgr *PtrMgr);
static jePtrMgr_Index FindIndexFromPtr(const jePtrMgr *PtrMgr, void *Ptr);

//========================================================================================
//	jePtrMgr_Create
//========================================================================================
JETAPI jePtrMgr * JETCC jePtrMgr_Create(void)
{
	jePtrMgr		*PtrMgr;

	PtrMgr = JE_RAM_ALLOCATE_STRUCT(jePtrMgr);

	if (!PtrMgr)
	{
		jeErrorLog_AddString(-1, "jePtrMgr_Create:  JE_RAM_ALLOCATE_STRUCT(jePtrMgr) failed.", "PtrMgr");
		return NULL;
	}

	ZeroMem(PtrMgr);

	PtrMgr->StackSize = JE_PTRMGR_START_SIZE;

	PtrMgr->PtrStack = JE_RAM_ALLOCATE_ARRAY(jePtrMgr_SEntry, PtrMgr->StackSize);

	if (!PtrMgr->PtrStack)
	{
		jeErrorLog_AddString(-1, "jePtrMgr_Create:  JE_RAM_ALLOCATE_ARRAY failed.", "PtrMgr->PtrStack");
		jeRam_Free(PtrMgr);
		return NULL;
	}

#ifdef _DEBUG
	PtrMgr->Signature1 = PtrMgr;
	PtrMgr->Signature2 = PtrMgr;
#endif

	PtrMgr->RefCount = 1;

	return PtrMgr;
}

//========================================================================================
//	jePtrMgr_IsValid
//========================================================================================
JETAPI jeBoolean JETCC jePtrMgr_IsValid(const jePtrMgr *PtrMgr)
{
	if (!PtrMgr)
	{
		jeErrorLog_AddString(-1, "jePtrMgr_IsValid:  PtrMgr == NULL", NULL);
		return JE_FALSE;
	}

#ifdef _DEBUG
	if (PtrMgr->Signature1 != PtrMgr)
	{
		jeErrorLog_AddString(-1, "jePtrMgr_IsValid:  PtrMgr->Signature1 != PtrMgr", NULL);
		return JE_FALSE;
	}

	if (PtrMgr->Signature2 != PtrMgr)
	{
		jeErrorLog_AddString(-1, "jePtrMgr_IsValid:  PtrMgr->Signature2 != PtrMgr", NULL);
		return JE_FALSE;
	}
#endif

	if (PtrMgr->RefCount <= 0)
	{
		jeErrorLog_AddString(-1, "jePtrMgr_IsValid:  PtrMgr->RefCount <= 0", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	jePtrMgr_CreateRef
//========================================================================================
JETAPI jeBoolean JETCC jePtrMgr_CreateRef(jePtrMgr *PtrMgr)
{
	assert(jePtrMgr_IsValid(PtrMgr) == JE_TRUE);

	if (PtrMgr->RefCount >= JE_PTRMGR_MAX_REF_COUNTS)
	{
		jeErrorLog_AddString(-1, "jePtrMgr_CreateRef:  PtrMgr->RefCount >= ((0xFFFFFFFF>>1)-1)", NULL);
		assert(0);		// just in case they are not checking return value (I dunno if this is legit though...)
		return JE_FALSE;
	}

	PtrMgr->RefCount++;

	return JE_TRUE;
}

//========================================================================================
//	jePtrMgr_Destroy
//========================================================================================
JETAPI void JETCC jePtrMgr_Destroy(jePtrMgr **PtrMgr)
{
	assert(PtrMgr);
	assert(jePtrMgr_IsValid(*PtrMgr) == JE_TRUE);

	(*PtrMgr)->RefCount--;

	if ((*PtrMgr)->RefCount == 0)
	{
		assert((*PtrMgr)->PtrStack);
		jeRam_Free((*PtrMgr)->PtrStack);

		jeRam_Free(*PtrMgr);
	}

	*PtrMgr = NULL;
}

//========================================================================================
//	jePtrMgr_ReadPtr
//========================================================================================
JETAPI jeBoolean JETCC jePtrMgr_ReadPtr(jePtrMgr *PtrMgr, jeVFile *VFile, void **Ptr)
{
	jePtrMgr_Header		Header;
	jePtrMgr_SEntry		*SEntry;

	assert(jePtrMgr_IsValid(PtrMgr) == JE_TRUE);
	assert(VFile);
	assert(Ptr);

	// Read the header
	if (!jeVFile_Read(VFile, &Header, sizeof(Header)))
	{
		jeErrorLog_AddString(-1, "jePtrMgr_Read:  jeVFile_Read failed.", "Header");
		return JE_FALSE;
	}

	if (Header.Index == JE_PTRMGR_NULL_INDEX)
	{
		*Ptr = NULL;		// They need to load this object
		return JE_TRUE;
	}
	
	// Object has been loaded by a previous read, return a ptr to it
	assert(Header.Index >= 0 && Header.Index < PtrMgr->StackLoc);	// This assert usually means that the ptr has not actually been read yet

	SEntry = &PtrMgr->PtrStack[Header.Index];

	// Assert that it HAS been loaded (assuming they are loading in the EXACT order they saved)
	assert(SEntry->Ptr);
	assert(SEntry->RefCount > 0);
	
	SEntry->RefCount++;		// Ref it

	PtrMgr->TotalPtrRefs++;

	*Ptr = SEntry->Ptr;

	return JE_TRUE;
}

//========================================================================================
//	jePtrMgr_PushPtr
//========================================================================================
JETAPI jeBoolean JETCC jePtrMgr_PushPtr(jePtrMgr *PtrMgr, void *Ptr)
{
	jePtrMgr_SEntry		*SEntry;

	assert(jePtrMgr_IsValid(PtrMgr) == JE_TRUE);
	assert(Ptr);

	SEntry = PushSEntry(PtrMgr);

	if (!SEntry)
		return JE_FALSE;

	assert(SEntry->RefCount == 0);
	assert(!SEntry->Ptr);

	// Ref the entry
	SEntry->RefCount++;
	SEntry->Ptr = Ptr;

	PtrMgr->TotalPtrRefs++;

	return JE_TRUE;
}

//========================================================================================
//	jePtrMgr_WritePtr
//========================================================================================
JETAPI jeBoolean JETCC jePtrMgr_WritePtr(jePtrMgr *PtrMgr, jeVFile *VFile, void *Ptr, uint32 *Count)
{
	jePtrMgr_Header		Header;
	jePtrMgr_SEntry		*SEntry;

	assert(jePtrMgr_IsValid(PtrMgr) == JE_TRUE);
	assert(VFile);
	assert(Ptr);
	assert(Count);

	// Try to find the ptr in the PtrStack
	Header.Index = FindIndexFromPtr(PtrMgr, Ptr);
	
	if (Header.Index != JE_PTRMGR_NULL_INDEX)
	{
		SEntry = &PtrMgr->PtrStack[Header.Index];
		assert(SEntry->RefCount > 0);
		assert(SEntry->Ptr == Ptr);

		// Get the count of this entry, BEFORE it was ref'd
		*Count = SEntry->RefCount++;
		PtrMgr->TotalPtrRefs++;
	}
	else
		*Count = 0;		// Not in PtrStack yet

	// Save the header
	if (!jeVFile_Write(VFile, &Header, sizeof(Header)))
	{
		jeErrorLog_AddString(-1, "jePtrMgr_WritePtr:  jeVFile_Read failed.", "Header");
		return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	jePtrMgr_PopPtr
//========================================================================================
JETAPI void JETCC jePtrMgr_PopPtr(jePtrMgr *PtrMgr, void *Ptr)
{
	jePtrMgr_SEntry		*SEntry;

	assert(jePtrMgr_IsValid(PtrMgr) == JE_TRUE);
	assert(Ptr);
	
	assert(PtrMgr->StackLoc > 0);
	PtrMgr->StackLoc--;

	SEntry = &PtrMgr->PtrStack[PtrMgr->StackLoc];

	assert(SEntry->RefCount > 0);
	assert(SEntry->Ptr == Ptr);
		
	SEntry->RefCount--;
	PtrMgr->TotalPtrRefs--;
}

//========================================================================================
//	jePtrMgr_GetPtrCount
//========================================================================================
JETAPI jeBoolean JETCC jePtrMgr_GetPtrCount(const jePtrMgr *PtrMgr, int32 *PtrCount)
{
	*PtrCount = PtrMgr->StackLoc;		// Ptr count is current stack loc...

	return JE_TRUE;
}

//========================================================================================
//	jePtrMgr_GetPtrRefs
//========================================================================================
JETAPI jeBoolean JETCC jePtrMgr_GetPtrRefs(const jePtrMgr *PtrMgr, int32 *PtrRefs)
{
	*PtrRefs = PtrMgr->TotalPtrRefs;

	return JE_TRUE;
}

//========================================================================================
//	PushSEntry
//========================================================================================
static jePtrMgr_SEntry *PushSEntry(jePtrMgr *PtrMgr)
{
	jePtrMgr_SEntry		*SEntry;
	
	if (PtrMgr->StackLoc >= PtrMgr->StackSize)
	{
		uint32		NewSize;

		if (PtrMgr->StackSize >= JE_PTRMGR_MAX_STACK_SIZE)
			return NULL;		// No room to grow anymore...

		// At this point, there is room to grow at least one element, so it should not fail
		//	unless we run out of memory

		NewSize = PtrMgr->StackSize + JE_PTRMGR_EXTEND_AMOUNT;

		if (NewSize < PtrMgr->StackSize)				// Must have wrapped, clamp to MAXSIZE
			NewSize = JE_PTRMGR_MAX_STACK_SIZE;

		if (NewSize > JE_PTRMGR_MAX_STACK_SIZE)
			NewSize = JE_PTRMGR_MAX_STACK_SIZE;

		assert(NewSize > PtrMgr->StackSize);

		PtrMgr->StackSize = NewSize;

		PtrMgr->PtrStack = (jePtrMgr_SEntry *)jeRam_Realloc(PtrMgr->PtrStack, PtrMgr->StackSize*sizeof(jePtrMgr_SEntry));

		if (!PtrMgr->PtrStack)
			return NULL;			// Out of memory
	}

	SEntry = &PtrMgr->PtrStack[PtrMgr->StackLoc++];

	ZeroMem(SEntry);

	return SEntry;
}

//========================================================================================
//	FindIndexFromPtr
//========================================================================================
static jePtrMgr_Index FindIndexFromPtr(const jePtrMgr *PtrMgr, void *Ptr)
{
	uint32				i;
	jePtrMgr_SEntry		*SEntry;

	assert(jePtrMgr_IsValid(PtrMgr) == JE_TRUE);
	assert(Ptr);

	for (SEntry = PtrMgr->PtrStack, i=0; i< PtrMgr->StackLoc; i++, SEntry++)
	{
		if (SEntry->Ptr == Ptr)
		{
			assert(SEntry->RefCount > 0);
			return i;
		}
	}

	return JE_PTRMGR_NULL_INDEX;
}

// Krouer : ResourceMgr accessor
JETAPI jeResourceMgr* JETCC jePtrMgr_GetResourceMgr(const jePtrMgr *PtrMgr)
{
	assert(PtrMgr);
	return PtrMgr->pResMgr;
}

