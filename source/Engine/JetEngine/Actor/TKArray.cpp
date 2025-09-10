/****************************************************************************************/
/*  TKARRAY.C																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Time-keyed array implementation.										*/
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
/*TKArray
	(Time-Keyed-Array)
	This module is designed primarily to support path.c

	The idea is that there are these packed arrays of elements,
	sorted by a jeTKArray_TimeType key.  The key is assumed to be the 
	first field in each element.

	the jeTKArray functions operate on this very specific array type.

	Error conditions are reported to errorlog

	Michael Sandige
*/

#include <assert.h>
#include <stddef.h> // offsetof
#include <string.h>

#include "TKArray.h"
#include "Errorlog.h"
#include "Ram.h"

typedef struct jeTKArray
{
	int32 NumElements;		// number of elements in use
	int32 ElementSize;		// size of each element
	char Elements[1];		// array elements.  This list will be expanded by changing
							// the allocated size of the entire jeTKArray object
}	jeTKArray;

typedef struct 
{
	int32 NumElements;		// number of elements in use
	int32 ElementSize;		// size of each element
} jeTKArray_FileHeader;


#define TK_MAX_ARRAY_LENGTH (0x7FFFFFFF)  // NumElements is (signed) 32 bit int


#define TK_ARRAYSIZE (offsetof(jeTKArray, Elements))	// gets rid of the extra element char in the def.

// General validity test.
// Use TK_ASSERT_VALID to test array for reasonable data.
#ifdef _DEBUG

#define TK_ASSERT_VALID(A) jeTKArray_Asserts(A)

// Do not call this function directly.  Use TK_ASSERT_VALID
static void JETCC jeTKArray_Asserts(const jeTKArray* A)
{
	assert( (A) != NULL );
	assert( ((A)->NumElements == 0) ||
			(((A)->NumElements > 0) && ((A)->Elements != NULL)) );
	assert( (A)->NumElements >= 0 );
	assert( (A)->NumElements <= TK_MAX_ARRAY_LENGTH );
	assert( (A)->ElementSize > 0 );
}

#else // !_DEBUG

#define TK_ASSERT_VALID(A) ((void)0)

#endif // _DEBUG


jeTKArray *JETCC jeTKArray_Create(				
	int ElementSize)				// element size
	// Creates new array with given attributes.  The first field of the element
	// is assumed to be the jeTKArray_TimeType key.
{
	jeTKArray *A;

	// first item in each element must be the time key
	assert( ElementSize >= sizeof(jeTKArray_TimeType) );

	A = (jeTKArray *)jeRam_AllocateClear(TK_ARRAYSIZE);
	if ( A == NULL)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeTKArray_Create.");
		return NULL;
	}

	A->ElementSize = ElementSize;
	A->NumElements = 0;

	TK_ASSERT_VALID(A);

	return A;	
}

jeTKArray *JETCC jeTKArray_CreateEmpty(				
	int ElementSize,int ElementCount)				// element size
	// Creates new array with given size and count.  The first field of the element
	// is assumed to be the jeTKArray_TimeType key.
{
	jeTKArray *A;
	int32 size = TK_ARRAYSIZE + ElementCount * ElementSize;
	A = (jeTKArray*)jeRam_AllocateClear(size);
	if( A == NULL )
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE,"jeTKArray_CreateEmpty.");
		return NULL;
	}
	A->ElementSize = ElementSize;
	A->NumElements = ElementCount;

	TK_ASSERT_VALID(A);

	return A;	
}

jeTKArray* JETCC jeTKArray_CreateFromFile(
	jeVFile* pFile)					// stream positioned at array data
	// Creates a new array from the given stream.
{
	int32 size;
	jeTKArray* A;
	jeTKArray_FileHeader Header;

	if (jeVFile_Read(pFile, &Header, sizeof(jeTKArray_FileHeader)) == JE_FALSE)
	{
		jeErrorLog_Add(JE_ERR_FILEIO_READ,"jeTKArray_CreateFromFile: Failed to read header");
		return NULL;
	}

	size = TK_ARRAYSIZE + Header.NumElements * Header.ElementSize;
	A = (jeTKArray*)jeRam_AllocateClear(size);
	if( A == NULL )
	{
		jeErrorLog_Add(JE_ERR_FILEIO_READ,"jeTKArray_CreateFromFile.");
		return NULL;
	}


	if(jeVFile_Read(pFile, A->Elements, size - sizeof(jeTKArray_FileHeader)) == JE_FALSE)
		{
			jeRam_Free(A);
			jeErrorLog_Add(JE_ERR_FILEIO_READ,"jeTKArray_CreateFromFile.");
			return NULL;
		}

	A->NumElements = Header.NumElements;
	A->ElementSize = Header.ElementSize;

	return A;
}


jeBoolean JETCC jeTKArray_SamplesAreTimeLinear(const jeTKArray *Array,jeFloat Tolerance)
{
	int i;

	jeTKArray_TimeType Delta,Nth,LastNth,NthDelta;
			
	if (Array->NumElements < 2)
		return JE_TRUE;

	LastNth = jeTKArray_ElementTime(Array, 0);
	Nth     = jeTKArray_ElementTime(Array, 1);
	Delta   =  Nth - LastNth;
	LastNth = Nth;
	
	for (i=2; i< Array->NumElements; i++)
		{
			Nth = jeTKArray_ElementTime(Array, i);
			NthDelta = (Nth-LastNth)-Delta;
			if (NthDelta<0.0f) NthDelta = -NthDelta;
			if (NthDelta>Tolerance)
				{
					return JE_FALSE;
				}
			LastNth = Nth;
		}
	return JE_TRUE;
}

jeBoolean JETCC jeTKArray_WriteToFile(
	const jeTKArray* Array,			// sorted array to write
	jeVFile* pFile)					// stream positioned for writing
	// Writes the array to the given stream.
{
	int size;
	
	size = TK_ARRAYSIZE + Array->NumElements * Array->ElementSize;
	if(jeVFile_Write(pFile, Array, size) == JE_FALSE)
	{
		jeErrorLog_Add(JE_ERR_FILEIO_WRITE,"jeTKArray_WriteToFile.");
		return JE_FALSE;
	}
	return JE_TRUE;
}

void JETCC jeTKArray_Destroy(jeTKArray **PA)
	// destroys array
{
	assert( PA  != NULL );
	TK_ASSERT_VALID(*PA);

	jeRam_Free(*PA);
	*PA = NULL;
}


int JETCC jeTKArray_BSearch(
	const jeTKArray *A,				// sorted array to search
	jeTKArray_TimeType Key)			// searching for this key
	// Searches for key in the Array.   A is assumed to be sorted
	// if key is found (within +-tolarance), the index to that element is returned.
	// if key is not found, the index to the key just smaller than the 
	// given key is returned.  (-1 if the key is smaller than the first element)
{
	int low,hi,mid;
	int ElementSize;
	const char *Array;
	jeTKArray_TimeType test;

	TK_ASSERT_VALID(A);
	
	low = 0;
	hi = A->NumElements - 1;
	Array = A->Elements;
	ElementSize = A->ElementSize;
	
	while ( low<=hi )
		{
			mid = (low+hi)/2;
			test = *(jeTKArray_TimeType *)(Array + mid*ElementSize);
			if ( Key > test )
				{
					low = mid+1;
				}
			else
				{
					if ( Key < test )
						{
							hi = mid-1;
						}
					else
						{
							return mid;
						}
				}
		}
	return hi;
}


jeBoolean JETCC jeTKArray_Insert(
	jeTKArray **PtrA,				// sorted array to insert into
	jeTKArray_TimeType Key,			// key to insert
	int *Index)						// new element index
	// inserts a new element into Array.
	// sets only the key for the new element - the rest is junk
	// returns JE_TRUE if the insertion was successful.
	// returns JE_FALSE if the insertion failed. 
	// if Array is empty (no elements, NULL pointer) it is allocated and filled 
	// with the one Key element
	// Index is the index of the new element 
{
	int n;
	jeTKArray *ChangedA;
	jeTKArray *A;
	jeTKArray_TimeType Found;

	assert( PtrA );
	A = *PtrA;
	TK_ASSERT_VALID(A);

	n = jeTKArray_BSearch(A,Key);
	// n is the element just prior to the location of the new element

	if(Index)
		*Index = n+1;

	if (n >= 0)
	{
		Found =  *(jeTKArray_TimeType *)(A->Elements + (n * (A->ElementSize)) );
		// Found <= Key  (within +-JE_TKA_TIME_TOLERANCE)
		if (Found > Key - JE_TKA_TIME_TOLERANCE)
		{	// if Found==Key, bail.  Can't have two identical keys.
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER, "jeTKArray_Insert: Identical keys not allowed.");
			return JE_FALSE;
		}
	}

	if (A->NumElements >= TK_MAX_ARRAY_LENGTH)
	{
		jeErrorLog_Add(JE_ERR_LIST_FULL, "jeTKArray_Insert: Too many keys.");
		return JE_FALSE;
	}

	ChangedA = (jeTKArray *)jeRam_Realloc(A, 
				TK_ARRAYSIZE + (A->NumElements + 1) * A->ElementSize);

	if ( ChangedA == NULL )
	{	
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeTKArray_Insert.");
		return JE_FALSE;
	}
	A = ChangedA;

	// advance n to new element's position
	n++;
	
	// move elements as necessary
	if(n < A->NumElements)
	{
		memmove( A->Elements + (n + 1) * A->ElementSize,	// dest
				 A->Elements + n * A->ElementSize,			// src
				 (A->NumElements - n) * A->ElementSize);	// count
	}

	*(jeTKArray_TimeType *)((A->Elements) + ((n) * (A->ElementSize)) ) = Key;
	A->NumElements++;
	*PtrA = A;

	return JE_TRUE;
}


jeBoolean JETCC jeTKArray_DeleteElement(
	jeTKArray **PtrA,				// sorted array to delete from
	int N)							// element to delete
	// deletes an element from Array.
	// returns JE_TRUE if the deletion was successful. 
	// returns JE_FALSE if the deletion failed. (key not found or realloc failed)
{
	jeTKArray *A;
	jeTKArray *ChangedA;
	
	assert( PtrA != NULL);
	A = *PtrA;
	TK_ASSERT_VALID(A);
	assert(N >= 0);
	assert(N < A->NumElements);
	
	memmove( (A->Elements) + (N) * (A->ElementSize),  //dest
			 (A->Elements) + (N+1) * (A->ElementSize),  //src
			 ((A->NumElements) - (N+1))* (A->ElementSize) );

	A->NumElements--;
	ChangedA = (jeTKArray *)jeRam_Realloc(A, 
				TK_ARRAYSIZE + A->NumElements * A->ElementSize);
	if ( ChangedA != NULL ) 
	{	
		// if realloc fails to shrink block. no real error.
		A = ChangedA;
	}

	*PtrA = A;

	return JE_TRUE;
}


void *JETCC jeTKArray_Element(const jeTKArray *A, int N)
	// returns the Nth element 
{
	TK_ASSERT_VALID(A);
	assert(N >= 0);
	assert(N < A->NumElements);

	return (void *)( (A->Elements) + (N * (A->ElementSize)) );
}


jeTKArray_TimeType JETCC jeTKArray_ElementTime(const jeTKArray *A, int N)
	// returns the time key for the Nth element 
{
	TK_ASSERT_VALID(A);
	assert(N >= 0);
	assert(N < A->NumElements);
	
	return *(jeTKArray_TimeType *)((A->Elements) + (N * (A->ElementSize)) );
}


int JETCC jeTKArray_NumElements(const jeTKArray *A)
	// returns the number of elements in the array
{
	TK_ASSERT_VALID(A);
	return A->NumElements;
}


int JETCC jeTKArray_ElementSize(const jeTKArray *A)
	// returns the size of each element in the array
{
	TK_ASSERT_VALID(A);
	return A->ElementSize;
}
