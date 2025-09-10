/****************************************************************************************/
/*  TKARRAY.H																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Time-keyed array interface.											*/
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
#ifndef JE_TKARRAY_H
#define JE_TKARRAY_H
/* TKArray
	(Time-Keyed-Array)
	This module is designed primarily to support path.c

	The idea is that there are these packed arrays of elements,
	sorted by a jeTKArray_TimeType key.  The key is assumed to be the 
	first field in each element.

	the TKArray functions operate on this very specific array type.

	Error conditions are reported to errorlog
	
	Michael Sandige

	01-28-98 [SLB]: style consistency changes, added jeTKArray_CreateFromFile

*/

#include "BaseType.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef jeFloat jeTKArray_TimeType;

#define JE_TKA_TIME_TOLERANCE (0.00001f)

typedef struct jeTKArray jeTKArray;

jeTKArray *JETCC jeTKArray_Create(int ElementSize);
	// creates new array with given attributes

jeTKArray *JETCC jeTKArray_CreateEmpty(int ElementSize,int ElementCount);
	// creates new array with given element size and given count of uninitialized members

jeTKArray* JETCC jeTKArray_CreateFromFile(
	jeVFile* pFile);					// stream positioned at array data
	// Creates a new array from the given stream.

jeBoolean JETCC jeTKArray_WriteToFile(
	const jeTKArray* Array,			// sorted array to write
	jeVFile* pFile);					// stream positioned for writing
	// Writes the array to the given stream.


int JETCC jeTKArray_BSearch(
	const jeTKArray *Array,			// sorted array to search
	jeTKArray_TimeType Key);		// searching for this time
	// Searches for key in the Array. (assumes array is sorted) 
	// if key is found (within +-tolerance), the index to that element is returned.
	// if key is not found, the index to the key just smaller than the 
	// given key is returned.  (-1 if the key is smaller than the first element)
	// search is only accurate to 2*TKA_TIME_TOLERANCE.  
	// if multiple keys exist within 2*TKA_TIME_TOLERANCE, this will find an arbitrary one of them.

jeBoolean JETCC jeTKArray_Insert(
	jeTKArray **Array,
	jeTKArray_TimeType Key,			// time to insert
	int *Index);					// new element index
	// inserts a new element into Array.
	// sets only the key for the new element - the rest is junk
	// returns TRUE if the insertion was successful.
	// returns FALSE if the insertion failed. 
	// if Array is empty (no elements, NULL pointer) it is allocated and filled 
	// with the one Key element
	// Index is the index of the new element 

jeBoolean JETCC jeTKArray_DeleteElement(
	jeTKArray **Array,
	int N);							// element to delete
	// deletes an element from Array.
	// returns TRUE if the deletion was successful. 
	// returns FALSE if the deletion failed. (key not found or realloc failed)

void JETCC jeTKArray_Destroy(
	jeTKArray **Array);	
	// destroys array

void *JETCC jeTKArray_Element(
	const jeTKArray *Array,
	int N);
	// returns a pointer to the Nth element of the array.

int JETCC jeTKArray_NumElements(
	const jeTKArray *Array);
	// returns the number of elements in the array

jeTKArray_TimeType JETCC jeTKArray_ElementTime(
	const jeTKArray *Array, 
	int N);
	// returns the Time associated with the Nth element of the array

int JETCC jeTKArray_ElementSize(
	const jeTKArray *A);
	// returns the size of each element in the array

jeBoolean JETCC jeTKArray_SamplesAreTimeLinear(const jeTKArray *Array,jeFloat Tolerance);
	// returns true if the samples are linear in time within a tolerance

#ifdef __cplusplus
}
#endif



#endif
