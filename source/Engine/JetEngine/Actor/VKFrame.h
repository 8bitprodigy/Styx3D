/****************************************************************************************/
/*  VKFRAME.H																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Vector keyframe interface.												*/
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
/* VKFrame (Vector-Keyframe)
	This module handles interpolation for keyframes that contain a vector (a jeVec3d)
	This is intended to support Path.c
	jeTKArray supplies general support for a time-keyed array, and this supplements
	that support to include the two specific time-keyed arrays:
	  An array of jeVec3d interpolated linearly
	  An array of jeVec3d interpolated with hermite blending
	These are phycially separated and have different base structures because:
		linear blending requires less data.
		future blending might require more data.
	The two types of lists are created with different creation calls,
	interpolated with different calls, but insertion and queries share a call.

	Hermite interpolation requires additional computation after changes are
	made to the keyframe list.  Call jeVKFrame_HermiteRecompute() to update the
	calculations.
*/


#ifndef JE_VKFRAME_H
#define JE_VKFRAME_H

#include "TKArray.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	VKFRAME_LINEAR,
	VKFRAME_HERMITE,
	VKFRAME_HERMITE_ZERO_DERIV,
} jeVKFrame_InterpolationType;


jeTKArray *JETCC jeVKFrame_LinearCreate(void);
	// creates a frame list for linear interpolation

jeTKArray *JETCC jeVKFrame_HermiteCreate(void);
	// creates a frame list for hermite interpolation


jeBoolean JETCC jeVKFrame_Insert(
	jeTKArray **KeyList,			// keyframe list to insert into
	jeTKArray_TimeType Time,		// time of new keyframe
	const jeVec3d *V,				// vector at new keyframe
	int *Index);					// indx of new key
	// inserts a new keyframe with the given time and vector into the list.

void JETCC jeVKFrame_Query(
	const jeTKArray *KeyList,		// keyframe list
	int Index,						// index of frame to return
	jeTKArray_TimeType *Time,		// time of the frame is returned
	jeVec3d *V);						// vector from the frame is returned
	// returns the vector and the time at keyframe[index] 

void JETCC jeVKFrame_Modify(
	jeTKArray *KeyList,				// keyframe list
	int Index,						// index of frame to change
	const jeVec3d *V);				// vector for the key
	// changes the vector at keyframe[index] 

void JETCC jeVKFrame_LinearInterpolation(
	const void *KF1,		// pointer to first keyframe
	const void *KF2,		// pointer to second keyframe
	jeFloat T,				// 0 <= T <= 1   blending parameter
	void *Result);			// put the result in here (jeVec3d)
		// interpolates to get a vector between the two vectors at the two
		// keyframes where T==0 returns the vector for KF1 
		// and T==1 returns the vector for KF2
		// interpolates linearly

void JETCC jeVKFrame_HermiteInterpolation(
	const void *KF1,		// pointer to first keyframe
	const void *KF2,		// pointer to second keyframe
	jeFloat T,				// 0 <= T <= 1   blending parameter
	void *Result);			// put the result in here (jeVec3d)
		// interpolates to get a vector between the two vectors at the two
		// keyframes where T==0 returns the vector for KF1 
		// and T==1 returns the vector for KF2
		// interpolates using 'hermite' blending


void JETCC jeVKFrame_HermiteRecompute(
	int Looped,					// if keylist has the first key connected to last key
	jeBoolean ZeroDerivative,	// if each key should have a zero derivatives (good for 2 point S curves)
	jeTKArray *KeyList,			// list of keys to recompute hermite values for
								// rebuild precomputed data for keyframe list.
	jeFloat CutInterval);		// intervals <= CutInterval are to be treated as discontinuous


jeBoolean JETCC jeVKFrame_WriteToFile(
	jeVFile						*pFile, 
	jeTKArray					*jeVKFrame, 
	jeVKFrame_InterpolationType InterpolationType, 
	int							Looping);

jeTKArray *JETCC jeVKFrame_CreateFromFile(
	jeVFile					  *pFile, 
	jeVKFrame_InterpolationType *InterpolationType, 
	int						  *Looping,
	jeFloat					  CutInterval);	// intervals <= CutInterval are to be treated as discontinuous

#ifdef __cplusplus
}
#endif

#endif
