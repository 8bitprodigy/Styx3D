/****************************************************************************************/
/*  PATH.H																				*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Time-indexed keyframe creation, maintenance, and sampling.				*/
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
#ifndef JE_PATH_H
#define JE_PATH_H

#include "BaseType.h"
#include "Xform3d.h"
#include "Quatern.h"
#include "VFile.h"

#ifdef __cplusplus
	extern "C" {
#endif


typedef struct _jePath jePath;

#define JE_PATH_ROTATION_CHANNEL    1
#define JE_PATH_TRANSLATION_CHANNEL 2

#define JE_PATH_ALL_CHANNELS (JE_PATH_ROTATION_CHANNEL | JE_PATH_TRANSLATION_CHANNEL)

#ifndef JE_PATH_ENUMS
	#define JE_PATH_ENUMS
	typedef enum 
	{
		JE_PATH_INTERPOLATE_LINEAR  = 0,	// linear blend for translation or rotation channel
		JE_PATH_INTERPOLATE_HERMITE,		// hermite cubic spline for translation channel
		JE_PATH_INTERPOLATE_SLERP,			// spherical-linear blend for rotation channel
		JE_PATH_INTERPOLATE_SQUAD,			// higher order blend for rotation channel 'G1' continuity
		//JE_PATH_INTEROPLATE_TRIPOD,		 // not supported yet.
		JE_PATH_INTERPOLATE_HERMITE_ZERO_DERIV = 7	// hermite cubic with zero derivative at keyframes ('easing' curve)
	}jePath_Interpolator;
#endif


#define JE_PATH_MAXIMUM_CUT_TIME (0.001f)

JETAPI void JETCC jePath_CreateRef( jePath *P );

JETAPI jePath *JETCC jePath_Create(
	jePath_Interpolator TranslationInterpolation,	// type of interpolation for translation channel
	jePath_Interpolator RotationInterpolation,	// type of interpolation for rotation channel
	jeBoolean Looped);				// True if end of path is connected to head
	// creates new jePath
	//  A looping path should have the same first & last point.  The path
	//  generator will choose arbitrarily between these points for a 
	//  sample exactly at the end of the loop.

JETAPI jePath *JETCC jePath_CreateCopy( const jePath *P );
	
JETAPI void JETCC jePath_Destroy(jePath **PP);		
	// destroys path *PP

JETAPI void JETCC jePath_SetCutMode(jePath *P, jeBoolean Enable);
	// If 'Cut' mode is enabled, keyframes with times that differ by <= JE_PATH_MAXIMUM_CUT_TIME
	// are treated as discontinuous.  This can be used to create 'cuts' or 'jumps' in the path.
	// The mode is set per path.

JETAPI jeBoolean JETCC jePath_GetCutMode(jePath *P);
	// returns the status of the 'cut' mode in the given path P

//------------------ time based keyframe operations
JETAPI jeBoolean JETCC jePath_InsertKeyframe(
	jePath *P, 
	int ChannelMask, 
	jeFloat Time, 
	const jeXForm3d *Matrix); 
	// inserts a keyframe at a specific time.
	
JETAPI jeBoolean JETCC jePath_DeleteKeyframe(
	jePath *P,
	int Index,
	int ChannelMask); 
	// deletes the nth keyframe

JETAPI jeBoolean JETCC jePath_GetTimeExtents(
	const jePath *P,
	jeFloat *StartTime, 
	jeFloat *EndTime);
	// gets the time for the first and last keys in the path (ignoring looping)
	// if there are no keys, return JE_FALSE and times are not set.
	// returns JE_TRUE if there are keys.

//----------------- index based keyframe operations
JETAPI void JETCC jePath_GetKeyframe(
	const jePath *P, 
	int Index,				// gets keyframe[index]
	int Channel,			// for this channel
	jeFloat *Time,			// returns the time of the keyframe
	jeXForm3d *Matrix);		// returns the matrix of the keyframe
	// retrieves keyframe[index], and it's time

JETAPI int JETCC jePath_GetKeyframeCount(const jePath *P,int Channel);
	// retrieves count of keyframes for a specific channel

JETAPI int JETCC jePath_GetKeyframeIndex(const jePath *P, int Channel, jeFloat Time);
	// retrieves the index of the keyframe at a specific time for a specific channel

//----------------- sampling a path  (time based)
JETAPI void JETCC jePath_Sample(const jePath *P, jeFloat Time,jeXForm3d *Matrix);
	// returns a transform matrix sampled at 'Time'.
	// p is not const because information is cached in p for next sample

JETAPI void JETCC jePath_SampleChannels(
	const jePath *P, 
	jeFloat Time, 
	jeQuaternion *Rotation, 
	jeVec3d *Translation);
	// returns a rotation and a translation for the path at 'Time'
	// p is not const because information is cached in p for next sample

JETAPI jeBoolean JETCC jePath_OffsetTimes(jePath *P, 
	int StartingIndex, int ChannelMask, jeFloat TimeOffset );
		// slides all samples in path starting with StartingIndex down by TimeOffset

JETAPI jeBoolean JETCC jePath_ModifyKeyframe(
	jePath *P,
	int Index,
	int ChannelMask,
	const jeXForm3d *Matrix);
	

//------------------ saving/loading a path
JETAPI jePath* JETCC jePath_CreateFromFile(jeVFile *F);
	// loads a file 

JETAPI jeBoolean JETCC jePath_WriteToFile(const jePath *P, jeVFile *F);
	// dumps a path to a file

#ifdef __cplusplus
	}
#endif


#endif
