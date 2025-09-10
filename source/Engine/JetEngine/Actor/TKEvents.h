/****************************************************************************************/
/*  TKARRAY.H																			*/
/*                                                                                      */
/*  Author: Stephen Balkum	                                                            */
/*  Description: Time-keyed events interface.											*/
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
#ifndef JE_TKEVENTS_H
#define JE_TKEVENTS_H
/* TKEvents
	(Time-Keyed-Events)
	This module is designed primarily to support motion.c

	jeTKEvents is a sorted array of times with an identifying descriptor.
	The descriptors are stored as strings in a separate, packed buffer.

*/

#include "BaseType.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeTKEvents jeTKEvents;
typedef jeFloat jeTKEvents_TimeType;

jeTKEvents* JETCC jeTKEvents_Create(void);
	// Creates a new event array.

void JETCC jeTKEvents_Destroy(jeTKEvents** pEvents);
	// Destroys array.

jeBoolean JETCC jeTKEvents_Insert(jeTKEvents* pEvents, jeTKEvents_TimeType tKey, const char* pEventData);
	// Inserts the new key and corresponding data.

jeBoolean JETCC jeTKEvents_Delete(jeTKEvents* pEvents, jeTKEvents_TimeType tKey);
	// Deletes the key 

jeTKEvents* JETCC jeTKEvents_CreateFromFile(
	jeVFile* pFile);					// stream positioned at array data
	// Creates a new array from the given stream.

jeBoolean JETCC jeTKEvents_WriteToFile(
	const jeTKEvents* pEvents,		// sorted array to write 
	jeVFile* pFile);					// stream positioned for writing
	// Writes the array to the given stream.
//---------------------------------------------------------------------------
// Event Iteration

void JETCC jeTKEvents_SetupIterator(
	jeTKEvents* pEvents,				// Event list to iterate
	jeTKEvents_TimeType StartTime,				// Inclusive search start
	jeTKEvents_TimeType EndTime);				// Non-inclusive search stop
	// For searching or querying the array for events between two times
	// times are compaired [StartTime,EndTime), '[' is inclusive, ')' is 
	// non-inclusive.  This prepares the PathGetNextEvent() function.

jeBoolean JETCC jeTKEvents_GetNextEvent(
	jeTKEvents* pEvents,				// Event list to iterate
	jeTKEvents_TimeType *pTime,				// Return time, if found
	const char **ppEventString);	// Return data, if found
	// Iterates from StartTime to EndTime as setup in jeTKEvents_CreateIterator()
	// and for each event between these times [StartTime,EndTime)
	// this function will return Time and EventString returned for that event
	// and the iterator will be positioned for the next search.  When there 
	// are no more events in the range, this function will return JE_FALSE (Time
	// will be 0 and ppEventString will be empty).

JETAPI jeBoolean JETCC jeTKEvents_GetExtents(
		jeTKEvents *Events,
		jeTKEvents_TimeType *FirstEventTime,	// time of first event
		jeTKEvents_TimeType *LastEventTime);	// time of last event

#ifdef __cplusplus
}
#endif



#endif
