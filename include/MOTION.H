/****************************************************************************************/
/*  MOTION.H	                                                                        */
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Motion interface.					                                    */
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
#ifndef JE_MOTION_H
#define JE_MOTION_H

/*	motion

	This object is a list of named Path objects

*/

#include <stdio.h>
#include "BaseType.h"
#include "Path.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

// JET_PUBLIC_APIS
typedef struct jeMotion jeMotion;

JETAPI jeMotion *JETCC jeMotion_Create(jeBoolean ManageNames);

JETAPI void JETCC jeMotion_Destroy(jeMotion **PM);

// JET_PRIVATE_APIS

JETAPI jeBoolean JETCC jeMotion_IsValid(const jeMotion *M);

	// AddPath adds a reference of P to the motion M.  Ownership is shared - The caller must destroy P.
JETAPI jeBoolean JETCC jeMotion_AddPath(jeMotion *M, jePath *P,const char *Name,int *Index);

JETAPI jeBoolean JETCC jeMotion_HasNames(const jeMotion *M);
JETAPI int32 JETCC jeMotion_GetNameChecksum(const jeMotion *M);

JETAPI jeBoolean JETCC jeMotion_RemoveNames(jeMotion *M);

JETAPI void JETCC jeMotion_SampleChannels(const jeMotion *M, int PathIndex, jeFloat Time, jeQuaternion *Rotation, jeVec3d *Translation);
JETAPI jeBoolean JETCC jeMotion_SampleChannelsNamed(const jeMotion *M, const char *PathName, jeFloat Time, jeQuaternion *Rotation, jeVec3d *Translation);

JETAPI void JETCC jeMotion_Sample(const jeMotion *M, int PathIndex, jeFloat Time, jeXForm3d *Transform);
JETAPI jeBoolean JETCC jeMotion_SampleNamed(const jeMotion *M, const char *PathName, jeFloat Time, jeXForm3d *Transform);

	// the returned Paths from _Get functions should not be destroyed.  
	// if ownership is desired, call jePath_CreateRef() to create another owner. 
	// an 'owner' has access to the object regardless of the number of other owners, and 
	// an owner must call the object's destroy method to relinquish ownership
JETAPI jePath *JETCC jeMotion_GetPathNamed(const jeMotion *M,const char *Name);
JETAPI const char *JETCC jeMotion_GetNameOfPath(const jeMotion *M, int Index);

// JET_PUBLIC_APIS
JETAPI jePath *JETCC jeMotion_GetPath(const jeMotion *M,int Index);
JETAPI int JETCC jeMotion_GetPathCount(const jeMotion *M);


JETAPI jeBoolean JETCC jeMotion_SetName(jeMotion *M, const char * Name);
JETAPI const char *JETCC jeMotion_GetName(const jeMotion *M);

// JET_PRIVATE_APIS

	// support for compound motions.  A motion can either have sub-motions, or be single motion.
	// these functions support motions that have sub-motions.
JETAPI int JETCC jeMotion_GetSubMotionCount(const jeMotion*M);

	// the returned motions from these _Get functions should not be destroyed.  
	// if ownership is desired, call jeMotion_CreateRef() to create another owner. 
	// an 'owner' has access to the object regardless of the number of other owners, and 
	// an owner must call the object's destroy method to relinquish ownership
JETAPI jeMotion *JETCC jeMotion_GetSubMotion(const jeMotion *M,int Index);
JETAPI jeMotion *JETCC jeMotion_GetSubMotionNamed(const jeMotion *M,const char *Name);
JETAPI jeBoolean JETCC jeMotion_AddSubMotion(
								jeMotion *ParentMotion,
								jeFloat TimeScale,			// Scale factor for this submotion
								jeFloat TimeOffset,			// Time in parent motion when submotion should start
								jeMotion *SubMotion,
								jeFloat StartTime,			// Blend start time (relative to submotion)
								jeFloat StartMagnitude,		// Blend start magnitude (0..1)
								jeFloat EndTime,			// Blend ending time (relative to submotion)
								jeFloat EndMagnitude,		// Blend ending magnitude (0..1)
								const jeXForm3d *Transform,	// Base transform to apply to this submotion
								int *Index);				// returned motion index

JETAPI jeMotion *JETCC  jeMotion_RemoveSubMotion(jeMotion *ParentMotion, int SubMotionIndex);

// Get/Set submotion time offset.  The time offset is the offset into the 
// compound (parent) motion at which the submotion should start.
JETAPI jeFloat   JETCC  jeMotion_GetTimeOffset( const jeMotion *M,int SubMotionIndex );
JETAPI jeBoolean  JETCC jeMotion_SetTimeOffset( jeMotion *M,int SubMotionIndex,jeFloat TimeOffset );

// Get/Set submotion time scale.  Time scaling is applied to the submotion after the TimeOffset
// is applied.  The formula is:  (CurrentTime - TimeOffset) * TimeScale
JETAPI jeFloat   JETCC  jeMotion_GetTimeScale( const jeMotion *M,int SubMotionIndex );
JETAPI jeBoolean  JETCC jeMotion_SetTimeScale( jeMotion *M,int SubMotionIndex,jeFloat TimeScale );

// Get blending amount for a particular submotion.  The Time parameter is parent-relative.
JETAPI jeFloat    JETCC jeMotion_GetBlendAmount( const jeMotion *M, int SubMotionIndex, jeFloat Time);

// Get/Set blending path.  The keyframe times in the blend path are relative to the submotion.
JETAPI jePath    *JETCC jeMotion_GetBlendPath( const jeMotion *M,int SubMotionIndex );
JETAPI jeBoolean  JETCC jeMotion_SetBlendPath( jeMotion *M,int SubMotionIndex, jePath *Blend );

JETAPI const jeXForm3d *JETCC jeMotion_GetBaseTransform( const jeMotion *M,int SubMotionIndex );
JETAPI jeBoolean  JETCC jeMotion_SetBaseTransform( jeMotion *M,int SubMotionIndex, jeXForm3d *BaseTransform );
JETAPI jeBoolean  JETCC jeMotion_GetTransform(const jeMotion *M, jeFloat Time, jeXForm3d *Transform);
// JET_PUBLIC_APIS

	// gets time of first key and time of last key (as if motion did not loop)
	// if there are no paths in the motion: returns JE_FALSE and times are not set
	// otherwise returns JE_TRUE
	//
	// For a compound motion, GetTimeExtents will return the extents of the scaled submotions.
	// For a single motion, no scaling is applied.
JETAPI jeBoolean JETCC jeMotion_GetTimeExtents(const jeMotion *M,jeFloat *StartTime,jeFloat *EndTime);

// Only one event is allowed per time key.

JETAPI jeBoolean JETCC jeMotion_InsertEvent(jeMotion *M, jeFloat tKey, const char* String);
	// Inserts the new event and corresponding string.

JETAPI jeBoolean JETCC jeMotion_DeleteEvent(jeMotion *M, jeFloat tKey);
	// Deletes the event

JETAPI void JETCC jeMotion_SetupEventIterator(
	jeMotion *M,
	jeFloat StartTime,				// Inclusive search start
	jeFloat EndTime);				// Non-inclusive search stop
	// For searching or querying the array for events between two times
	// times are compaired [StartTime,EndTime), '[' is inclusive, ')' is 
	// non-inclusive.  This prepares the jeMotion_GetNextEvent() function.

JETAPI jeBoolean JETCC jeMotion_GetNextEvent(
	jeMotion *M,						// Event list to iterate
	jeFloat *pTime,				// Return time, if found
	const char **ppEventString);	// Return data, if found
	// Iterates from StartTime to EndTime as setup in jeMotion_SetupEventIterator()
	// and for each event between these times [StartTime,EndTime)
	// this function will return Time and EventString returned for that event
	// and the iterator will be positioned for the next search.  When there 
	// are no more events in the range, this function will return JE_FALSE (Time
	// will be 0 and ppEventString will be empty).

JETAPI jeBoolean JETCC jeMotion_GetEventExtents(const jeMotion *M,
			jeFloat *FirstEventTime,
			jeFloat *LastEventTime);
	// returns the time associated with the first and last events 
	// returns JE_FALSE if there are no events (and Times are not set)


// JET_PRIVATE_APIS
JETAPI jeMotion *JETCC jeMotion_CreateFromFile(jeVFile *f);
JETAPI jeBoolean JETCC jeMotion_WriteToFile(const jeMotion *M,jeVFile *pFile);

#ifdef __cplusplus
}
#endif


#endif
