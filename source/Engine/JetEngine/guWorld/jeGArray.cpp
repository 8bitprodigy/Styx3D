/****************************************************************************************/
/*  JEGARRAY.C                                                                          */
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
#include <assert.h>
#include <stdio.h>
#include <memory.h>

// Public dependents
#include "jeGArray.h"

// Private dependents
#include "Ram.h"
#include "Errorlog.h"

//=======================================================================================
//=======================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

#define JE_GARRAY_MAX_REF_COUNT		65535

//=======================================================================================
//=======================================================================================
typedef struct jeGArray
{
	uint16				RefCount;

	jeGArray_Index		ActiveElements;		// Total active elements
	jeGArray_Index		MaxElements;		// Element array size (in elements)
	uint16				ElementSize;
	jeGArray_Index		LastIndex;			//
	
	uint8				*Elements;
	jeGArray_RefType	*RefCounts;

#ifdef _DEBUG
	jeGArray			*Self;
#endif

} jeGArray;

static jeBoolean jeGArray_Extend(jeGArray *Array);

//=======================================================================================
//	jeGArray_Create
//=======================================================================================
jeGArray *jeGArray_Create(int32 StartElements, int32 ElementSize)
{
	jeGArray	*Array;

	assert(StartElements < JE_GARRAY_MAX_ELEMENTS);		// This must be true
	assert(ElementSize < JE_GARRAY_MAX_ELEMENT_SIZE);

	Array = JE_RAM_ALLOCATE_STRUCT(jeGArray);

	if (!Array)	// Assume not enough ram
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeGArray_Create:  Out of ram for array.", NULL);
		return NULL;
	}

	// Clear mem for array
	ZeroMem(Array);

	// Now, create the elements
	Array->Elements = JE_RAM_ALLOCATE_ARRAY(uint8, StartElements*ElementSize);

	if (!Array->Elements)
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeGArray_Create:  Out of ram for elements.", NULL);
		goto ExitWithError;
	}

	// Clear the elements in this vert Link
	ZeroMemArray(Array->Elements, StartElements*ElementSize);

	// Create the element ref counts
	Array->RefCounts = JE_RAM_ALLOCATE_ARRAY(jeGArray_RefType, StartElements);

	if (!Array->RefCounts)
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeGArray_Create:  Out of ram for ref counts.", NULL);
		goto ExitWithError;
	}

	ZeroMemArray(Array->RefCounts, StartElements);

	// Store the number of verts that the first link in the Link has
	Array->MaxElements = (jeGArray_Index)StartElements;
	Array->ElementSize = (jeGArray_Index)ElementSize;
	Array->ActiveElements = 0;
	Array->LastIndex = 0;

	Array->RefCount = 1;

#ifdef _DEBUG
	Array->Self = Array;
#endif
   
	return Array;			// Done

	// Error
	ExitWithError:
	{
		if (Array)
		{
			if (Array->Elements)
				jeRam_Free(Array->Elements);
			if (Array->RefCounts)
				jeRam_Free(Array->RefCounts);
			jeRam_Free(Array);
		}

		return NULL;
	}
}

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
		((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))

#define JE_GARRAY_TAG			MAKEFOURCC('G', 'E', 'G', 'A')		// 'GE' 'G'eneric 'A'rray
#define JE_GARRAY_VERSION		0

//=======================================================================================
//	jeGArray_CreateFromFile
//=======================================================================================
jeGArray *jeGArray_CreateFromFile(jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr)
{
	uint32		Tag;
	uint16		Version;
	jeGArray	*Array;

	assert(VFile);

	if (PtrMgr)
	{
		if (!jePtrMgr_ReadPtr(PtrMgr, VFile, (void **)&Array))
			return NULL;

		if (Array)
		{
			if (!jeGArray_CreateRef(Array))
				return NULL;

			return Array;		// Ptr found in stack, return it
		}
	}

	// Read header info
	if (!jeVFile_Read(VFile, &Tag, sizeof(Tag)))
		return NULL;

	if (Tag != JE_GARRAY_TAG)
		return NULL;

	if (!jeVFile_Read(VFile, &Version, sizeof(Version)))
		return NULL;

	if (Version != JE_GARRAY_VERSION)
		return NULL;

	// Read array
	Array = JE_RAM_ALLOCATE_STRUCT(jeGArray);

	if (!Array)	// Assume not enough ram
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeGArray_CreateFromFile:  Out of ram for array.", NULL);
		return NULL;
	}

	// Clear mem for array
	ZeroMem(Array);

	// Read array size
	if (!jeVFile_Read(VFile, &Array->MaxElements, sizeof(Array->MaxElements)))
		return NULL;

	// Read num active elements
	if (!jeVFile_Read(VFile, &Array->ActiveElements, sizeof(Array->ActiveElements)))
		return NULL;

	// Read size of each element
	if (!jeVFile_Read(VFile, &Array->ElementSize, sizeof(Array->ElementSize)))
		return NULL;

	// Allocate arrays
	Array->Elements = JE_RAM_ALLOCATE_ARRAY(uint8, Array->MaxElements*Array->ElementSize);

	if (!Array->Elements)
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeGArray_CreateFromFile:  Out of ram for elements.", NULL);
		goto ExitWithError;
	}

	// Allocate ref count array
	Array->RefCounts = JE_RAM_ALLOCATE_ARRAY(jeGArray_RefType, Array->MaxElements);

	if (!Array->RefCounts)
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeGArray_CreateFromFile:  Out of ram for ref counts.", NULL);
		goto ExitWithError;
	}

	// Load arrays
	// load refcount array
	if (!jeVFile_Read(VFile, Array->RefCounts, sizeof(Array->RefCounts[0])*Array->MaxElements))
		return NULL;

	// Load element array
	if (IOFunc)
	{
		int32		i;

		for (i=0; i< Array->MaxElements; i++)
		{
			if (!Array->RefCounts[i])
				continue;

			if (!IOFunc(VFile, &Array->Elements[i*Array->ElementSize], IOFuncContext))
				goto ExitWithError;
		}
	}
	else
	{
		if (!jeVFile_Read(VFile, Array->Elements, Array->ElementSize*Array->MaxElements))
			goto ExitWithError;
	}

	// Zero out the RefCount array because no one should be reffing any of the elements yet
	ZeroMemArray(Array->RefCounts, Array->MaxElements);

	Array->LastIndex = 0;

	Array->RefCount = 1;

#ifdef _DEBUG
	Array->Self = Array;
#endif
   
	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, Array))
			goto ExitWithError;
	}

	return Array;			// Done

	// Error
	ExitWithError:
	{
		if (Array)
		{
			if (Array->Elements)
				jeRam_Free(Array->Elements);
			if (Array->RefCounts)
				jeRam_Free(Array->RefCounts);
			jeRam_Free(Array);
		}

		return NULL;
	}
}


//=======================================================================================
//	jeGArray_WriteToFile
//=======================================================================================
jeBoolean jeGArray_WriteToFile(const jeGArray *Array, jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr)
{
	uint32		Tag;
	uint16		Version;

	assert(Array);
	assert(VFile);

	if (PtrMgr)
	{
		uint32		Count;

		if (!jePtrMgr_WritePtr(PtrMgr, VFile, (void*)Array, &Count))
			return JE_FALSE;

		if (Count)
			return JE_TRUE;		// Ptr was on stack, so return
	}

	// Write TAG
	Tag = JE_GARRAY_TAG;

	if (!jeVFile_Write(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	// Write version
	Version = JE_GARRAY_VERSION;

	if (!jeVFile_Write(VFile, &Version, sizeof(Version)))
		return JE_FALSE;

	// Write out array size
	if (!jeVFile_Write(VFile, &Array->MaxElements, sizeof(Array->MaxElements)))
		return JE_FALSE;

	// Write out num active elements
	if (!jeVFile_Write(VFile, &Array->ActiveElements, sizeof(Array->ActiveElements)))
		return JE_FALSE;

	// Write out size of each element
	if (!jeVFile_Write(VFile, &Array->ElementSize, sizeof(Array->ElementSize)))
		return JE_FALSE;

	// Write out ref count array
	if (!jeVFile_Write(VFile, Array->RefCounts, sizeof(Array->RefCounts[0])*Array->MaxElements))
		return JE_FALSE;

	// Write out element array
	if (IOFunc)
	{
		int32		i;

		for (i=0; i< Array->MaxElements; i++)
		{
			if (!Array->RefCounts[i])
				continue;

			if (!IOFunc(VFile, &Array->Elements[i*Array->ElementSize], IOFuncContext))
				return JE_FALSE;
		}
	}
	else
	{
		if (!jeVFile_Write(VFile, Array->Elements, Array->ElementSize*Array->MaxElements))
			return JE_FALSE;
	}

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, (void*)Array))
			return JE_FALSE;
	}

	return JE_TRUE;
}


//=======================================================================================
//	jeGArray_CreateRef
//=======================================================================================
jeBoolean jeGArray_CreateRef(jeGArray *Array)
{
	assert(jeGArray_IsValid(Array) == JE_TRUE);
	assert(Array->RefCount >= 0);

	Array->RefCount++;

	if (Array->RefCount >= JE_GARRAY_MAX_REF_COUNT)
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeGArray_Destroy
//=======================================================================================
void jeGArray_Destroy(jeGArray **Array)
{
	assert(Array);
	assert(jeGArray_IsValid(*Array) == JE_TRUE);
	assert((*Array)->RefCount > 0);

	(*Array)->RefCount--;

	if ((*Array)->RefCount == 0)		// Don't destroy until refcount == 0
	{
		if ((*Array)->Elements)				// Free the elements
		{
			assert((*Array)->MaxElements > 0);
			assert((*Array)->RefCounts);

			#ifdef _DEBUG		
			{
				int32		i;
	
				for (i=0; i< (*Array)->MaxElements; i++)
				{
					assert((*Array)->RefCounts[i] == 0);
				}
			}
			#endif

			jeRam_Free((*Array)->Elements);
			jeRam_Free((*Array)->RefCounts);
		}
		else
		{
			assert((*Array)->MaxElements == 0);
			assert(!(*Array)->RefCounts);
		}

		jeRam_Free(*Array);				// Finally, free the Array itself
	}

	*Array = NULL;
}

//=======================================================================================
//	jeGArray_IsValid
//=======================================================================================
jeBoolean jeGArray_IsValid(const jeGArray *Array)
{
	if (!Array)
		return JE_FALSE;

	if (Array->RefCount <= 0)
		return JE_FALSE;

#ifdef _DEBUG
	if (Array->Self != Array)
		return JE_FALSE;
#endif
	return JE_TRUE;
}

//=======================================================================================
//	jeGArray_Extend
//=======================================================================================
static jeBoolean jeGArray_Extend(jeGArray *Array)
{
	int32				NewSize;
	jeGArray_RefType	*Refs;

	NewSize = Array->MaxElements<<1;

	if (NewSize > JE_GARRAY_MAX_ELEMENTS)
	{
		NewSize = JE_GARRAY_MAX_ELEMENTS;

		assert(NewSize > Array->MaxElements);	// Make sure it grows past original size!

		if (NewSize <= Array->MaxElements)		// Make sure it grows past original size!
			return JE_FALSE;			// Out of index space
	}
	
	Array->Elements = (uint8 *)jeRam_Realloc(Array->Elements, NewSize*Array->ElementSize);

	if (!Array->Elements)
		return JE_FALSE;

	Array->RefCounts = (jeGArray_RefType *)jeRam_Realloc(Array->RefCounts, NewSize*sizeof(jeGArray_RefType));

	if (!Array->RefCounts)
	{
		jeRam_Free(Array->Elements);
		return JE_FALSE;
	}

	Refs = &Array->RefCounts[Array->MaxElements];

	// Clear new memory allocated in the refs section...
	memset(Refs, 0, sizeof(jeGArray_RefType)*(NewSize-Array->MaxElements));

	Array->MaxElements = (jeGArray_Index)NewSize;	// Get the new number of max elements

	return JE_TRUE;
}

//=======================================================================================
//	jeGArray_AddElement
//=======================================================================================
jeGArray_Index jeGArray_AddElement(jeGArray *Array, const jeGArray_Element *Element)
{
	int32	i2;

	assert(jeGArray_IsValid(Array) == JE_TRUE);
	assert(Element);

	if (Array->ActiveElements >= Array->MaxElements)
	{
		i2 = Array->MaxElements;

		if (!jeGArray_Extend(Array))
			return JE_GARRAY_NULL_INDEX;
	}
	else 
	{
		jeGArray_Index			i;

		i2 = Array->LastIndex;

		for (i=0; i<Array->MaxElements; i++, i2++)
		{
			if (i2 == Array->MaxElements)				// Wrap
				i2 = 0;

			if (!Array->RefCounts[i2])
				break;
		}
	}

	assert(Array->RefCounts[i2] == 0);

	Array->RefCounts[i2]++;
	Array->ActiveElements++;

	memcpy(&Array->Elements[i2*Array->ElementSize], Element, Array->ElementSize);

	Array->LastIndex = (jeGArray_Index)(i2+1);

	return (jeGArray_Index)i2;
}

//=======================================================================================
//	jeGArray_RefElement
//=======================================================================================
jeBoolean jeGArray_RefElement(jeGArray *Array, jeGArray_Index Index)
{
	assert(jeGArray_IsValid(Array) == JE_TRUE);
	assert(Index != JE_GARRAY_NULL_INDEX);
	assert(Array->ActiveElements > 0);
	assert(Array->RefCounts[Index] >= 0);
	assert(Array->RefCounts[Index] < JE_GARRAY_MAX_ELEMENT_REFCOUNT);

	Array->RefCounts[Index]++;

	return JE_TRUE;
}

//=======================================================================================
//	jeGArray_RemoveElement
//=======================================================================================
void jeGArray_RemoveElement(jeGArray *Array, jeGArray_Index *Index)
{
	assert(jeGArray_IsValid(Array) == JE_TRUE);
	assert(*Index != JE_GARRAY_NULL_INDEX);
	assert(Array->ActiveElements > 0);
	assert(Array->RefCounts[*Index] > 0);

	Array->RefCounts[*Index]--;

	if (Array->RefCounts[*Index] == 0)
		Array->ActiveElements--;

	*Index = JE_GARRAY_NULL_INDEX;	// They should not use this element index again
}

//=======================================================================================
//	jeGArray_GetSize
//=======================================================================================
int32 jeGArray_GetSize(const jeGArray *Array)
{
	assert(jeGArray_IsValid(Array) == JE_TRUE);

	return (int32)Array->MaxElements;
}

//=======================================================================================
//	jeGArray_GetElements
//=======================================================================================
jeGArray_Element *jeGArray_GetElements(const jeGArray *Array)
{
	assert(jeGArray_IsValid(Array) == JE_TRUE);
	return Array->Elements;
}

//=======================================================================================
//	jeGArray_GetRefCounts
//=======================================================================================
jeGArray_RefType *jeGArray_GetRefCounts(const jeGArray *Array)
{
	assert(jeGArray_IsValid(Array) == JE_TRUE);
	return Array->RefCounts;
}

//=======================================================================================
//	jeGArray_GetElementRefCountByIndex
//=======================================================================================
const jeGArray_RefType jeGArray_GetElementRefCountByIndex(const jeGArray *Array, jeGArray_Index Index)
{
	assert(jeGArray_IsValid(Array) == JE_TRUE);
	assert(Index >= 0 && Index < Array->MaxElements);
	assert(Index != JE_GARRAY_NULL_INDEX);
	assert(Array->RefCounts[Index] >= 0);

	return Array->RefCounts[Index];
}
/*
//=======================================================================================
//	jeGArray_GetNextElement
//=======================================================================================
jeGArray_Element *jeGArray_GetNextElement(jeGarray *Array, jeGArray_Element *Start)
{
	assert(Array);
	assert(Start);

	if (!Start)
		Start = Array->Elements;

	Index = Array->Elements

	while (
}
*/
//=======================================================================================
//	jeGArray_SetElementByIndex
//=======================================================================================
void jeGArray_SetElementByIndex(jeGArray *Array, jeGArray_Index Index, const jeGArray_Element *Element)
{
	assert(jeGArray_IsValid(Array) == JE_TRUE);
	assert(Index != JE_GARRAY_NULL_INDEX);
	assert(Index >= 0 && Index < Array->MaxElements);
	assert(Array->RefCounts[Index] > 0);

	memcpy(&Array->Elements[Index*Array->ElementSize], Element, Array->ElementSize);
}

//=======================================================================================
//	jeGArray_GetElementByIndex
//=======================================================================================
const jeGArray_Element *jeGArray_GetElementByIndex(const jeGArray *Array, jeGArray_Index Index)
{
	assert(jeGArray_IsValid(Array) == JE_TRUE);
	assert(Index != JE_GARRAY_NULL_INDEX);
	assert(Index >= 0 && Index < Array->MaxElements);
	assert(Array->RefCounts[Index] > 0);

	return &Array->Elements[Index*Array->ElementSize];
}

