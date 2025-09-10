/****************************************************************************************/
/*  TKARRAY.C																			*/
/*                                                                                      */
/*  Author: Stephen Balkum	                                                            */
/*  Description: Time-keyed events implementation.										*/
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
/* jeTKEvents
	(Time-Keyed-Events)

	jeTKEvents is a sorted array of times with an identifying descriptor.
	The descriptors are stored as strings in a separate, packed buffer.

*/

#include <assert.h>
#include <string.h>

#include "TKEvents.h"
#include "TKArray.h"
#include "Errorlog.h"
#include "Ram.h"

typedef struct
{
	jeTKEvents_TimeType EventTime;
	uint32 DataOffset;
}	EventType;

typedef struct jeTKEventsIterator 
{
	jeTKEvents_TimeType EndTime;
	int CurrentIndex;
}	jeTKEventsIterator;

typedef struct jeTKEvents
{
	jeTKArray* pTimeKeys;
	uint32 DataSize;
	char* pEventData;

	jeTKEventsIterator Iterator;
}	jeTKEvents;



// General validity test.
// Use TKE_ASSERT_VALID to test array for reasonable data.
#ifdef _DEBUG

#define TKE_ASSERT_VALID(E) jeTKEvents_Asserts(E)

// Do not call this function directly.  Use TKE_ASSERT_VALID
static void JETCC jeTKEvents_Asserts(const jeTKEvents* E)
{
	assert( (E) != NULL );
	assert( (E)->pTimeKeys != NULL );
	if(jeTKArray_NumElements((E)->pTimeKeys) == 0)
	{
		assert( (E)->pEventData == NULL );
	}
	else
	{
		assert( (E)->pEventData != NULL );
	}
}

#else // !_DEBUG

#define TKE_ASSERT_VALID(E) ((void)0)

#endif // _DEBUG

jeTKEvents* JETCC jeTKEvents_Create(void)
	// Creates a new event array.
{
	jeTKEvents* pEvents;

	pEvents = JE_RAM_ALLOCATE_STRUCT_CLEAR(jeTKEvents);
	if(!pEvents)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeTKEvents_Create.");
		return NULL;
	}

	pEvents->pTimeKeys = jeTKArray_Create(sizeof(EventType));
	if(!pEvents->pTimeKeys)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeTKEvents_Create.");
		jeRam_Free(pEvents);
		return NULL;
	}

	pEvents->DataSize = 0;
	pEvents->pEventData = NULL;

	pEvents->Iterator.CurrentIndex = 0;
	pEvents->Iterator.EndTime = -99e33f;	// you could sample here I suppose...
	
	return pEvents;
}


void JETCC jeTKEvents_Destroy(jeTKEvents** ppEvents)
	// Destroys array.
{
	jeTKEvents* pE;

	assert(ppEvents);
	pE = *ppEvents;
	assert(pE);

	if( pE->pEventData != NULL )
		{
			jeRam_Free(pE->pEventData);
		}
	
	if (pE->pTimeKeys != NULL)
		{
			jeTKArray_Destroy(&pE->pTimeKeys);
		}
	jeRam_Free(*ppEvents);
	*ppEvents = NULL;
}


jeBoolean JETCC jeTKEvents_Insert(jeTKEvents* pEvents, jeTKEvents_TimeType tKey, const char* pEventData)
{
	int nIndex;
	uint32 DataLength;
	uint32 InitialOffset;
	int nNumElements;
	EventType* pKeyInfo;
	char* pNewData;

	TKE_ASSERT_VALID(pEvents);

	if( jeTKArray_Insert(&pEvents->pTimeKeys, tKey, &nIndex) != JE_TRUE )
	{
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeTKEvents_Insert: failed to insert.");
		return JE_FALSE;
	}
	pKeyInfo = (EventType *)jeTKArray_Element(pEvents->pTimeKeys, nIndex);
	assert( pKeyInfo != NULL ); // I just successfully added it; it better be there.

	DataLength = strlen(pEventData) + 1;

	// Resize data to add new stuff
	pNewData = (char *)jeRam_Realloc(pEvents->pEventData, pEvents->DataSize + DataLength);
	if(!pNewData)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeTKEvents_Insert.");
		if( jeTKArray_DeleteElement(&pEvents->pTimeKeys, nIndex) == JE_FALSE)
		{
			// This object is now in an unstable state.
			assert(0);
		}
		// invalidate the iterator
		pEvents->Iterator.EndTime = -99e33f;	// you could sample here I suppose...
		return JE_FALSE;
	}
	pEvents->pEventData = pNewData;

	// Find where new data will go
	nNumElements = jeTKArray_NumElements(pEvents->pTimeKeys);
	assert(nIndex < nNumElements); // sanity check
	if(nIndex == nNumElements - 1)
	{
		// We were added to the end
		InitialOffset = pEvents->DataSize;
	}
	else
	{
		EventType* pNextKeyInfo = (EventType *)jeTKArray_Element(pEvents->pTimeKeys, nIndex + 1);
		assert( pNextKeyInfo != NULL );

		InitialOffset = pNextKeyInfo->DataOffset;
	}
	pKeyInfo->DataOffset = InitialOffset;

	// Add new data, moving only if necessary
	if(InitialOffset < pEvents->DataSize)
	{
		memmove(pEvents->pEventData + InitialOffset + DataLength,	// dest
				pEvents->pEventData + InitialOffset,				// src
				pEvents->DataSize - InitialOffset);					// count
	}
	memcpy(	pEvents->pEventData + InitialOffset,	// dest
			pEventData,								// src
			DataLength);							// count

	pEvents->DataSize += DataLength;

	// Bump all remaining offsets up
	nIndex++;
	while(nIndex < nNumElements)
	{
		pKeyInfo = (EventType *)jeTKArray_Element(pEvents->pTimeKeys, nIndex);
		assert( pKeyInfo != NULL );
		pKeyInfo->DataOffset += DataLength;

		nIndex++;
	}

	// invalidate the iterator
	pEvents->Iterator.EndTime = -99e33f;	// you could sample here I suppose...

	return JE_TRUE;
}


jeBoolean JETCC jeTKEvents_Delete(jeTKEvents* pEvents, jeTKEvents_TimeType tKey)
{
	int nIndex, Count;
	jeTKEvents_TimeType tFound;
	EventType* pKeyInfo;
	int DataOffset, DataSize;
	char *pNewData;

	TKE_ASSERT_VALID(pEvents);

	nIndex = jeTKArray_BSearch(pEvents->pTimeKeys, tKey);

	if( nIndex < 0 )
	{	// key wasn't found
		jeErrorLog_Add(JE_ERR_SEARCH_FAILURE, "jeTKEvents_Delete: key not found for delete.");
		return JE_FALSE;
	}

	tFound = jeTKArray_ElementTime(pEvents->pTimeKeys, nIndex);
	if(tFound < tKey - JE_TKA_TIME_TOLERANCE)
	{
		// key not found
		jeErrorLog_Add(JE_ERR_SEARCH_FAILURE, "jeTKEvents_Delete: key not found for delete.");
		return JE_FALSE;
	}

	pKeyInfo = (EventType *)jeTKArray_Element(pEvents->pTimeKeys, nIndex);
	DataOffset = pKeyInfo->DataOffset;
	if(nIndex < jeTKArray_NumElements(pEvents->pTimeKeys) - 1)
	{
		// not the last element
		pKeyInfo = (EventType *)jeTKArray_Element(pEvents->pTimeKeys, nIndex + 1);
		DataSize = pKeyInfo->DataOffset - DataOffset;

		memmove(pEvents->pEventData + DataOffset,				// dest
				pEvents->pEventData + DataOffset + DataSize,	// src
				pEvents->DataSize - DataOffset - DataSize);		// count
	}
	else
	{
		// It's the last element and no memory needs to be moved
		DataSize = pEvents->DataSize - DataOffset;
	}

	// Adjust data
	pEvents->DataSize -= DataSize;
	if (pEvents->DataSize == 0)
	{
		jeRam_Free (pEvents->pEventData);
		pEvents->pEventData = NULL;
	}
	else
	{
		pNewData = (char *)jeRam_Realloc(pEvents->pEventData, pEvents->DataSize);
		// If the reallocation failed, it doesn't really hurt.  However, it is a 
		// sign of problems ahead.
		if(pNewData)
		{
			pEvents->pEventData = pNewData;
		}
	}

	// Finally, remove this element
	jeTKArray_DeleteElement(&pEvents->pTimeKeys, nIndex);

	// Adjust the offsets
	Count = jeTKArray_NumElements(pEvents->pTimeKeys);
	while(nIndex < Count)
	{
		pKeyInfo = (EventType *)jeTKArray_Element(pEvents->pTimeKeys, nIndex);
		assert( pKeyInfo != NULL );
		pKeyInfo->DataOffset -= DataSize;
		nIndex++;
	}

	// invalidate the iterator
	pEvents->Iterator.EndTime = -99e33f;	// you could sample here I suppose...

	return JE_TRUE;
}


#define TKEVENTS_FILE_VERSION 0x00F0		// Restrict to 16 bits
#define TKEVENTS_BIN_FILE_TYPE   0x42454B54 // 'TKEB'


jeTKEvents* JETCC jeTKEvents_CreateFromFile(
	jeVFile* pFile)					// stream positioned at array data
	// Creates a new array from the given stream.
{
	uint32 u;
	jeTKEvents* pEvents;

	assert( pFile != NULL );

	// Read the format/version flag
	if(jeVFile_Read(pFile, &u, sizeof(u)) == JE_FALSE)
	{
		jeErrorLog_Add(JE_ERR_FILEIO_READ, "jeTKEvents_CreateFromFile.");
		return NULL;
	}

	pEvents = JE_RAM_ALLOCATE_STRUCT_CLEAR(jeTKEvents);
	if(!pEvents)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeTKEvents_CreateFromFile.");
		return NULL;
	}
	pEvents->pEventData = NULL;
	pEvents->pTimeKeys  = NULL;

		if(u == TKEVENTS_BIN_FILE_TYPE)
			{
				if(jeVFile_Read(pFile, &u, sizeof(u)) == JE_FALSE)
					{
						jeErrorLog_Add(JE_ERR_FILEIO_READ, "jeTKEvents_CreateFromFile.");
						jeTKEvents_Destroy(&pEvents);
						return NULL;
					}
				if (u != TKEVENTS_FILE_VERSION)
					{
						jeErrorLog_AddString(JE_ERR_FILEIO_VERSION,"jeTKEvents_CreateFromFile: Failure to recognize file version", NULL);
						jeTKEvents_Destroy(&pEvents);
						return NULL;
					}

				if(jeVFile_Read(pFile, &(pEvents->DataSize), sizeof(pEvents->DataSize)) == JE_FALSE)
					{
						jeErrorLog_Add(JE_ERR_FILEIO_READ, "jeTKEvents_CreateFromFile.");
						jeTKEvents_Destroy(&pEvents);
						return NULL;
					}

				pEvents->pEventData = (char *)jeRam_AllocateClear(pEvents->DataSize);
				if(!pEvents->pEventData)
					{
						jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeTKEvents_CreateFromFile.");
						jeTKEvents_Destroy(&pEvents);
						return NULL;
					}

				if(jeVFile_Read(pFile, pEvents->pEventData, pEvents->DataSize) == JE_FALSE)
					{
						jeErrorLog_Add(JE_ERR_FILEIO_READ, "jeTKEvents_CreateFromFile.");
						jeTKEvents_Destroy(&pEvents);
						return NULL;
					}
				pEvents->pTimeKeys = jeTKArray_CreateFromFile(pFile);
				if(!pEvents->pTimeKeys)
					{
						jeErrorLog_Add(JE_ERR_FILEIO_READ, "jeTKEvents_CreateFromFile.");
						jeTKEvents_Destroy(&pEvents);
						return NULL;
					}
			}

	return pEvents;
}

jeBoolean JETCC jeTKEvents_WriteToFile(
	const jeTKEvents* pEvents,		// sorted array to write
	jeVFile* pFile)					// stream positioned for writing
	// Writes the array to the given stream.
{
	uint32 u;
	assert( pEvents != NULL );
	assert( pFile != NULL );

	u = TKEVENTS_BIN_FILE_TYPE;
	if(jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "jeTKEvents_WriteToFile.");
			return JE_FALSE;
		}
	u = TKEVENTS_FILE_VERSION;
	// Write the version
	if(jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "jeTKEvents_WriteToFile.");
			return JE_FALSE;
		}

	if(jeVFile_Write(pFile, &pEvents->DataSize, sizeof(pEvents->DataSize)) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "jeTKEvents_WriteToFile.");
			return JE_FALSE;
		}

	if(jeVFile_Write(pFile, pEvents->pEventData, pEvents->DataSize) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "jeTKEvents_WriteToFile.");
			return JE_FALSE;
		}

	if (jeTKArray_WriteToFile(pEvents->pTimeKeys, pFile)==JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "jeTKEvents_WriteToFile.");
			return JE_FALSE;
		}
	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTKEvents_GetExtents(jeTKEvents *Events,
		jeTKEvents_TimeType *FirstEventTime,
		jeTKEvents_TimeType *LastEventTime)
{
	int Count;
	assert( Events != NULL );
	
	Count = jeTKArray_NumElements(Events->pTimeKeys);
	if (Count<0)
		{
			return JE_FALSE;
		}

	*FirstEventTime = jeTKArray_ElementTime(Events->pTimeKeys, 0);
	*LastEventTime  = jeTKArray_ElementTime(Events->pTimeKeys, Count-1);
	return JE_TRUE;
}

void JETCC jeTKEvents_SetupIterator(
	jeTKEvents* pEvents,				// Event list to iterate
	jeTKEvents_TimeType StartTime,				// Inclusive search start
	jeTKEvents_TimeType EndTime)				// Non-inclusive search stop
	// For searching or querying the array for events between two times
	// times are compaired [StartTime,EndTime), '[' is inclusive, ')' is 
	// non-inclusive.  This prepares the PathGetNextEvent() function.  
{
	jeTKEventsIterator* pTKEI;

	assert( pEvents != NULL );

	pTKEI = &pEvents->Iterator;

	pTKEI->EndTime = EndTime;

	// Initialize search with first index before StartTime
	pTKEI->CurrentIndex = jeTKArray_BSearch(pEvents->pTimeKeys, StartTime - JE_TKA_TIME_TOLERANCE);
	while( (pTKEI->CurrentIndex > -1) && 
		(jeTKArray_ElementTime(pEvents->pTimeKeys, pTKEI->CurrentIndex) >= StartTime - JE_TKA_TIME_TOLERANCE) )
	{
		pTKEI->CurrentIndex--;
	}
}


jeBoolean JETCC jeTKEvents_GetNextEvent(
	jeTKEvents* pEvents,				// Event list to iterate
	jeTKEvents_TimeType *pTime,				// Return time, if found
	const char **ppEventString)		// Return data, if found
	// Iterates from StartTime to EndTime as setup in jeTKEvents_CreateIterator()
	// and for each event between these times [StartTime,EndTime)
	// this function will return Time and EventString returned for that event
	// and the iterator will be positioned for the next search.  When there 
	// are no more events in the range, this function will return NULL (Time
	// will be 0 and ppEventString will be empty).
{
	jeTKEventsIterator* pTKEI;
	jeTKArray* pTimeKeys;
	EventType* pKeyInfo;
	int Index;

	assert(pEvents);
	assert(pTime);
	assert(ppEventString);

	pTKEI = &pEvents->Iterator;

	pTimeKeys = pEvents->pTimeKeys;

	pTKEI->CurrentIndex++;
	Index = pTKEI->CurrentIndex;
	if(Index < jeTKArray_NumElements(pTimeKeys))
	{
		*pTime = jeTKArray_ElementTime(pTimeKeys, Index);
		if(*pTime + JE_TKA_TIME_TOLERANCE < pTKEI->EndTime)
		{
			// Looks good.  Get the string and return.
			pKeyInfo = (EventType *)jeTKArray_Element(pTimeKeys, Index);
			*ppEventString = pEvents->pEventData + pKeyInfo->DataOffset;
			return JE_TRUE;
		}
	}

	// None found, clean up
	*pTime = 0.0f;
	*ppEventString = NULL;
	return JE_FALSE;
}
