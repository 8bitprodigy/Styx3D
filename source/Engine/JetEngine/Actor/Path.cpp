/****************************************************************************************/
/*  PATH.C																				*/
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
#include <assert.h>
#include <math.h>   //fmod()

#include "Path.h"
#include "Quatern.h"
#include "Errorlog.h"
#include "Ram.h"
#include "TKArray.h"
#include "VKFrame.h"
#include "QKFrame.h"
#include "Vec3d.h"

#define min(aa,bb)  (( (aa)>(bb) ) ? (bb) : (aa) )
#define max(aa,bb)  (( (aa)>(bb) ) ? (aa) : (bb) )


#define jePath_TimeType jeFloat

typedef int8 Bool8;

typedef void (JETCC *InterpolationFunction)(
	const void *KF1,
	const void *KF2, 
	jePath_TimeType T,
	void *Result);


#define FLAG_DIRTY   (0x01)
#define FLAG_LOOPED  (0x01)
#define FLAG_OTHER	 (0x696C6345)
#define FLAG_EMPTY	 (0x21657370)

typedef enum
{
	JE_PATH_VK_LINEAR,
	JE_PATH_VK_HERMITE,
	JE_PATH_VK_HERMITE_ZERO_DERIV,
	JE_PATH_QK_LINEAR,
	JE_PATH_QK_SLERP,
	JE_PATH_QK_SQUAD,
	JE_PATH_MANY_INTERPOLATORS
} jePath_InterpolationType;

typedef struct
{
	jeTKArray *KeyList;
	// was int.. int InterpolationType;				// type of interpolation for channel
	jePath_InterpolationType InterpolationType;
	
	jePath_TimeType StartTime;			// First time in channel's path
	jePath_TimeType EndTime;			// Last time in channel's path

	// --remember keys used for last sample--
	int32 LastKey1;						// smaller key
	int32 LastKey2;						// larger key (keys may be equal)
	jePath_TimeType LastKey1Time;		// Time at LastKey1
	jePath_TimeType LastKey2Time;		// Time at LastKey2
									// if last key is not valid: LastKey1Time > LastKey2Time
} jePath_Channel;


typedef struct _jePath
{
	jePath_Channel Rotation;
	jePath_Channel Translation;
	unsigned int Dirty    : 1;						
	unsigned int Looped   : 1;
	unsigned int AllowCuts: 1;
	unsigned int RefCount :29;
} jePath;


typedef struct 
{
	InterpolationFunction InterpolationTable[JE_PATH_MANY_INTERPOLATORS];
	int32 Flags[2];
} jePath_StaticType;

jePath_StaticType jePath_Statics = 
{
	{ 	jeVKFrame_LinearInterpolation,
		jeVKFrame_HermiteInterpolation,
		jeVKFrame_HermiteInterpolation,
		jeQKFrame_LinearInterpolation,
		jeQKFrame_SlerpInterpolation,
		jeQKFrame_SquadInterpolation
	},
	{FLAG_OTHER,FLAG_EMPTY},
};



static jeVKFrame_InterpolationType JETCF jePath_PathToVKInterpolation(jePath_InterpolationType I)
{
	switch (I)
		{
			case (JE_PATH_VK_LINEAR):			  return VKFRAME_LINEAR;
			case (JE_PATH_VK_HERMITE):			  return VKFRAME_HERMITE;
			case (JE_PATH_VK_HERMITE_ZERO_DERIV): return VKFRAME_HERMITE_ZERO_DERIV;
			default: assert(0);
		}
	return VKFRAME_LINEAR;  // this is just for warning removal
}
			
static jePath_InterpolationType JETCF jePath_VKToPathInterpolation(jeVKFrame_InterpolationType I)
{
	switch (I)
		{
			case (VKFRAME_LINEAR):				return JE_PATH_VK_LINEAR;
			case (VKFRAME_HERMITE):				return JE_PATH_VK_HERMITE;
			case (VKFRAME_HERMITE_ZERO_DERIV):  return JE_PATH_VK_HERMITE_ZERO_DERIV;
			default: assert(0);
		}
	return JE_PATH_VK_LINEAR; // this is just for warning removal
}

static jeQKFrame_InterpolationType JETCF jePath_PathToQKInterpolation(jePath_InterpolationType I)
{
	switch (I)
		{
			case (JE_PATH_QK_LINEAR):	return QKFRAME_LINEAR;
			case (JE_PATH_QK_SLERP):	return QKFRAME_SLERP;
			case (JE_PATH_QK_SQUAD):	return QKFRAME_SQUAD;
			default: assert(0);
		}
	return QKFRAME_LINEAR;  // this is just for warning removal
}
			
static jePath_InterpolationType JETCF jePath_QKToPathInterpolation(jeQKFrame_InterpolationType I)
{
	switch (I)
		{
			case (QKFRAME_LINEAR):	return JE_PATH_QK_LINEAR;
			case (QKFRAME_SLERP):	return JE_PATH_QK_SLERP;
			case (QKFRAME_SQUAD):	return JE_PATH_QK_SQUAD;
			default: assert(0);
		}
	return JE_PATH_QK_LINEAR; // this is just for warning removal
}


JETAPI void JETCC jePath_CreateRef( jePath *P )
{
	assert( P != NULL );
	P->RefCount++;
}

JETAPI void JETCC jePath_SetCutMode(jePath *P, jeBoolean Enable)
{
	assert( P != NULL );
	P->AllowCuts = Enable;
	P->Dirty = FLAG_DIRTY;
}

JETAPI jeBoolean JETCC jePath_GetCutMode(jePath *P)
{
	assert( P != NULL );
	return (P->AllowCuts);
}
	

JETAPI jePath *JETCC jePath_Create(
	jePath_Interpolator TranslationInterpolation,	// type of interpolation for translation channel
	jePath_Interpolator RotationInterpolation,	// type of interpolation for rotation channel
	jeBoolean Looped)				// JE_TRUE if end of path is connected to head
	
{
	jePath *P;

	P = (jePath *)jeRam_AllocateClear(sizeof(jePath));

	if ( P == NULL )
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jePath_Create.");
		return NULL;
	}

	P->Rotation.KeyList    = NULL;
	P->Translation.KeyList = NULL;
	
	P->RefCount  = 0;
	P->Dirty     = FLAG_DIRTY;

	if (Looped==JE_TRUE)
		P->Looped = FLAG_LOOPED;
	else
		P->Looped = 0;


	switch (RotationInterpolation)
		{
			case (JE_PATH_INTERPOLATE_LINEAR):
				P->Rotation.InterpolationType = JE_PATH_QK_LINEAR;
				break;
			case (JE_PATH_INTERPOLATE_SLERP):
				P->Rotation.InterpolationType = JE_PATH_QK_SLERP; 
				break;
			case (JE_PATH_INTERPOLATE_SQUAD):
				P->Rotation.InterpolationType = JE_PATH_QK_SQUAD;
				break;
			default:
				assert(0);
		}
	
	P->Rotation.KeyList = NULL;

	switch (TranslationInterpolation)
		{
			case (JE_PATH_INTERPOLATE_LINEAR):
				P->Translation.InterpolationType = JE_PATH_VK_LINEAR;
				break;
			case (JE_PATH_INTERPOLATE_HERMITE):
				P->Translation.InterpolationType = JE_PATH_VK_HERMITE;
				break;
			case (JE_PATH_INTERPOLATE_HERMITE_ZERO_DERIV):
				P->Translation.InterpolationType = JE_PATH_VK_HERMITE_ZERO_DERIV;
				break;
			default:
				assert(0);
		}

	P->Translation.KeyList = NULL;

	return P;
}

static jeBoolean JETCF jePath_SetupRotationKeyList(jePath *P)
{
	assert( P != NULL );
	switch (P->Rotation.InterpolationType)
		{
			case (JE_PATH_QK_LINEAR):
				P->Rotation.KeyList = jeQKFrame_LinearCreate();
				break;
			case (JE_PATH_QK_SLERP):
				P->Rotation.KeyList = jeQKFrame_SlerpCreate();
				break;
			case (JE_PATH_QK_SQUAD):
				P->Rotation.KeyList = jeQKFrame_SquadCreate();
				break;
			default:
				assert(0);
		}
	if (P->Rotation.KeyList == NULL)
		{
			return JE_FALSE;
		}
	return JE_TRUE;	
}

static jeBoolean JETCF jePath_SetupTranslationKeyList(jePath *P)
{
	assert( P != NULL );
	switch (P->Translation.InterpolationType)
		{
			case (JE_PATH_VK_LINEAR):
				P->Translation.KeyList = jeVKFrame_LinearCreate();
				break;
			case (JE_PATH_VK_HERMITE):
				P->Translation.KeyList = jeVKFrame_HermiteCreate();
				break;
			case (JE_PATH_VK_HERMITE_ZERO_DERIV):
				P->Translation.KeyList = jeVKFrame_HermiteCreate();
				break;
			default:
				assert(0);
		}
	if (P->Translation.KeyList == NULL)
		{
			return JE_FALSE;
		}
	return JE_TRUE;
}

JETAPI jePath *JETCC jePath_CreateCopy(const jePath *Src)
{
	jePath *P;
	jePath_TimeType Time;
	jeBoolean Looped;

	int i,Count;
	jePath_Interpolator RInterp;
	jePath_Interpolator TInterp;

	assert ( Src != NULL );

	switch (Src->Rotation.InterpolationType)
		{
			case (JE_PATH_QK_LINEAR):
				RInterp = JE_PATH_INTERPOLATE_LINEAR;
				break;
			case (JE_PATH_QK_SLERP):
				RInterp = JE_PATH_INTERPOLATE_SLERP;
				break;
			case (JE_PATH_QK_SQUAD):
				RInterp = JE_PATH_INTERPOLATE_SQUAD;
				break;
			default:
				assert(0);
		}
	
	switch (Src->Translation.InterpolationType)
		{
			case (JE_PATH_VK_LINEAR):
				TInterp = JE_PATH_INTERPOLATE_LINEAR;
				break;
			case (JE_PATH_VK_HERMITE):
				TInterp = JE_PATH_INTERPOLATE_HERMITE;
				break;
			case (JE_PATH_VK_HERMITE_ZERO_DERIV):
				TInterp = JE_PATH_INTERPOLATE_HERMITE_ZERO_DERIV;
				break;
			default:
				assert(0);
		}
	
	if (Src->Looped)
		Looped = JE_TRUE;
	else
		Looped = JE_FALSE;

	P = jePath_Create(TInterp, RInterp, Looped);	
	if (P == NULL)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_CreateCopy.");
			return NULL;
		}

	{
		jeVec3d V;
		Count = 0;
		if (Src->Translation.KeyList != NULL)
			{
				Count = jeTKArray_NumElements(Src->Translation.KeyList);
			}
		if (Count>0)
			{
				if (jePath_SetupTranslationKeyList(P)==JE_FALSE)
					{
						jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_CreateCopy.");
						jePath_Destroy(&P);
						return NULL;
					}

				for (i=0; i<Count; i++)
					{
						int Index;
						jeVKFrame_Query(Src->Translation.KeyList, i, &Time, &V);
						if (jeVKFrame_Insert(&(P->Translation.KeyList), Time, &V,&Index) == JE_FALSE)
							{
								jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_CreateCopy.");
								jePath_Destroy(&P);
								return NULL;
							}
					}
			}
	}

	{
		jeQuaternion Q;
		Count = 0;
		if (Src->Rotation.KeyList != NULL)
			{
				Count = jeTKArray_NumElements(Src->Rotation.KeyList);
			}
		if (Count>0)
			{
				if (jePath_SetupRotationKeyList(P)==JE_FALSE)
					{
						jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_CreateCopy.");
						jePath_Destroy(&P);
						return NULL;
					}

				for (i=0; i<Count; i++)
					{
						int Index;
						jeQKFrame_Query(Src->Rotation.KeyList, i, &Time, &Q);
						if (jeQKFrame_Insert(&(P->Rotation.KeyList), Time, &Q, &Index) == JE_FALSE)
							{
								jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_CreateCopy.");
								jePath_Destroy(&P);
								return NULL;
							}
					}
			}
	}
	return P;
}
	


JETAPI void JETCC jePath_Destroy(jePath **PP)
{
	jePath *P;
	
	assert( PP  != NULL );
	assert( *PP != NULL );
	
	P = *PP;

	if ( P->RefCount > 0)
		{
			P->RefCount -- ;
			return;
		}
	if ( P->Rotation.KeyList != NULL)
	{
		jeTKArray_Destroy(&(P->Rotation.KeyList));
		P->Rotation.KeyList = NULL;
	}

	if ( P->Translation.KeyList != NULL)
	{
		jeTKArray_Destroy(&(P->Translation.KeyList));
		P->Translation.KeyList = NULL;
	}

	jeRam_Free(*PP);

	*PP = NULL;
}


static void JETCF jePath_Recompute(jePath *P)
	// Recompute any pre-computed constants for the current path.
{
	jeBoolean Looped;
	assert(P);

	P->Dirty = 0;

	P->Translation.LastKey1Time = 0.0f;
	P->Translation.LastKey2Time = -1.0f;
	if (P->Looped)
		Looped = JE_TRUE;
	else
		Looped = JE_FALSE;

	if (P->Translation.KeyList != NULL)
	{
		if (jeTKArray_NumElements(P->Translation.KeyList) > 0 )
		{
			P->Translation.StartTime =	jeTKArray_ElementTime(P->Translation.KeyList,0);
			P->Translation.EndTime   =	jeTKArray_ElementTime(P->Translation.KeyList,
										jeTKArray_NumElements(P->Translation.KeyList) - 1);
		}
		if(P->Translation.InterpolationType == JE_PATH_VK_HERMITE)
			jeVKFrame_HermiteRecompute(Looped, JE_FALSE, P->Translation.KeyList,JE_PATH_MAXIMUM_CUT_TIME);
		else if (P->Translation.InterpolationType == JE_PATH_VK_HERMITE_ZERO_DERIV)
			jeVKFrame_HermiteRecompute(Looped, JE_TRUE, P->Translation.KeyList,JE_PATH_MAXIMUM_CUT_TIME);
	}
	
	P->Rotation.LastKey1Time = 0.0f;
	P->Rotation.LastKey2Time = -1.0f;

	if (P->Rotation.KeyList != NULL)
	{
		if (jeTKArray_NumElements(P->Rotation.KeyList) > 0 )
		{
			P->Rotation.StartTime = jeTKArray_ElementTime(P->Rotation.KeyList,0);
			P->Rotation.EndTime   = jeTKArray_ElementTime(P->Rotation.KeyList,
									jeTKArray_NumElements(P->Rotation.KeyList) - 1);
		}
		if (P->Rotation.InterpolationType == JE_PATH_QK_SQUAD)
			jeQKFrame_SquadRecompute(Looped, P->Rotation.KeyList,JE_PATH_MAXIMUM_CUT_TIME);
		else if (P->Rotation.InterpolationType == JE_PATH_QK_SLERP)
			jeQKFrame_SlerpRecompute(P->Rotation.KeyList);

	}
}	

//------------------ time based keyframe operations
JETAPI jeBoolean JETCC jePath_InsertKeyframe(
	jePath *P, 
	int ChannelMask, 
	jePath_TimeType Time, 
	const jeXForm3d *Matrix)
{
	int VIndex;
	int QIndex = 0;
	assert( P != NULL );
	assert( Matrix != NULL );
	assert( ( ChannelMask & JE_PATH_ROTATION_CHANNEL    ) ||
			( ChannelMask & JE_PATH_TRANSLATION_CHANNEL ) );
	
	if (ChannelMask & JE_PATH_ROTATION_CHANNEL)
	{	
		jeQuaternion Q;
		jeQuaternion_FromMatrix(Matrix, &Q);
		jeQuaternion_Normalize(&Q);
		if (P->Rotation.KeyList==NULL)
		{
			if (jePath_SetupRotationKeyList(P)==JE_FALSE)
				{
					jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_InsertKeyframe.");
					return JE_FALSE;
				}
		}
		if (jeQKFrame_Insert(&(P->Rotation.KeyList), Time, &Q, &QIndex) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_InsertKeyframe.");
			return JE_FALSE;
		}
	}

	
	if (ChannelMask & JE_PATH_TRANSLATION_CHANNEL)
	{
		jeBoolean ErrorOccured = JE_FALSE;
		if (P->Translation.KeyList == NULL)
			{
				if (jePath_SetupTranslationKeyList(P)==JE_FALSE)
					{
						jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_InsertKeyframe.");
						ErrorOccured = JE_TRUE;
					}
			}
		if (ErrorOccured == JE_FALSE)
			{
				if (jeVKFrame_Insert( &(P->Translation.KeyList), Time, &(Matrix->Translation), &VIndex) == JE_FALSE)
					{
						jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_InsertKeyframe.");
						ErrorOccured = JE_TRUE;
					}
			}
		if (ErrorOccured != JE_FALSE)
			{
				if (ChannelMask & JE_PATH_ROTATION_CHANNEL)
					{	// clean up previously inserted rotation
						if (jeTKArray_DeleteElement(&(P->Rotation.KeyList),QIndex)==JE_FALSE)
							{
								jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_InsertKeyframe.");
							}
					}
				P->Dirty = FLAG_DIRTY;
				return JE_FALSE;
			}
	}

	P->Dirty = FLAG_DIRTY;

	return JE_TRUE;
}
	
JETAPI jeBoolean JETCC jePath_DeleteKeyframe(
	jePath *P,
	int Index,
	int ChannelMask)
{
	int ErrorOccured= 0;

	assert( P != NULL );
	assert( ( ChannelMask & JE_PATH_ROTATION_CHANNEL    ) ||
			( ChannelMask & JE_PATH_TRANSLATION_CHANNEL ) );

	if (ChannelMask & JE_PATH_ROTATION_CHANNEL)
	{
		if (jeTKArray_DeleteElement( &(P->Rotation.KeyList), Index) == JE_FALSE)
		{
			ErrorOccured = 1;
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_DeleteKeyframe.");
		}
	}
			
	if (ChannelMask & JE_PATH_TRANSLATION_CHANNEL)
	{
		if (jeTKArray_DeleteElement( &(P->Translation.KeyList), Index) == JE_FALSE)
		{
			ErrorOccured = 1;
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePath_DeleteKeyframe.");
		}
	}

	P->Dirty = FLAG_DIRTY;


	if (ErrorOccured)
	{
		return JE_FALSE;
	}

	return JE_TRUE;
}


JETAPI void JETCC jePath_GetKeyframe(
	const jePath *P, 
	int Index,				// gets keyframe[index]
	int Channel,			// for this channel
	jePath_TimeType *Time,	// returns the time of the keyframe
	jeXForm3d *Matrix) 		// returns the matrix of the keyframe
{
	assert( P != NULL );
	assert( Index >= 0 );
	assert( Time != NULL );
	assert( Matrix != NULL );

	jeXForm3d_SetIdentity(Matrix);

	switch (Channel)
	{
	case (JE_PATH_ROTATION_CHANNEL):
		{
			jeQuaternion Q;
			assert( Index < jeTKArray_NumElements(P->Rotation.KeyList) );
			jeQKFrame_Query(P->Rotation.KeyList, Index, Time, &Q);
			jeQuaternion_ToMatrix(&Q, Matrix);
		}
		break;

	case (JE_PATH_TRANSLATION_CHANNEL):
		{
			assert( Index < jeTKArray_NumElements(P->Translation.KeyList) );
			jeVKFrame_Query(P->Translation.KeyList, Index, Time, &(Matrix->Translation));
		}
		break;

	default:
		assert(0);
	}
}

JETAPI jeBoolean JETCC jePath_ModifyKeyframe(
	jePath *P, 
	int Index,						// keyframe[index]
	int ChannelMask,				// for this channel
	const jeXForm3d *Matrix) 		// new matrix for the keyframe
{
	assert( P != NULL );
	assert( Index >= 0 );
	assert( Matrix != NULL );
	assert( ( ChannelMask & JE_PATH_ROTATION_CHANNEL    ) ||
			( ChannelMask & JE_PATH_TRANSLATION_CHANNEL ) );


	if (ChannelMask & JE_PATH_ROTATION_CHANNEL)
		{
			jeQuaternion Q;
			assert( Index < jeTKArray_NumElements(P->Rotation.KeyList) );
			jeQuaternion_FromMatrix(Matrix, &Q);
			jeQuaternion_Normalize(&Q);
			jeQKFrame_Modify(P->Rotation.KeyList, Index, &Q);
		}

	if (ChannelMask & JE_PATH_TRANSLATION_CHANNEL)
		{
			assert( Index < jeTKArray_NumElements(P->Translation.KeyList) );
			jeVKFrame_Modify(P->Translation.KeyList, Index, &(Matrix->Translation));
		}

	P->Dirty = FLAG_DIRTY;
	return JE_TRUE;
}


JETAPI int JETCC jePath_GetKeyframeCount(const jePath *P, int Channel)
{
	assert( P != NULL );

	switch (Channel)
	{
		case (JE_PATH_ROTATION_CHANNEL):
			if (P->Rotation.KeyList!=NULL)
				{
					return jeTKArray_NumElements(P->Rotation.KeyList);
				}
			else
				{
					return 0;
				}
			break;

		case (JE_PATH_TRANSLATION_CHANNEL):
			if (P->Translation.KeyList!=NULL)
				{
					return jeTKArray_NumElements(P->Translation.KeyList);
				}
			else
				{
					return 0;
				}
			break;

		default:
			assert(0);
	}
	return 0; // this is just for warning removal
}

JETAPI int JETCC jePath_GetKeyframeIndex(const jePath *P, int Channel, jeFloat Time)
	// retrieves the index of the keyframe at a specific time for a specific channel
{
	int KeyIndex;
	jeTKArray *Array = NULL;

	assert ((Channel == JE_PATH_TRANSLATION_CHANNEL) ||
			(Channel == JE_PATH_ROTATION_CHANNEL));

	switch (Channel)
	{
		case JE_PATH_ROTATION_CHANNEL :
			Array = P->Rotation.KeyList;
			break;

		case JE_PATH_TRANSLATION_CHANNEL :
			Array = P->Translation.KeyList;
			break;
	}

	// find the time in the channel's array
	KeyIndex = jeTKArray_BSearch (Array, Time);
	if (KeyIndex != -1)
	{
		// since jeTKArray_BSearch will return the "closest" key,
		// I need to make sure that it's exact...
		if (fabs (Time - jeTKArray_ElementTime (Array, KeyIndex)) > JE_TKA_TIME_TOLERANCE)
		{
			KeyIndex = -1;
		}
	}

	return KeyIndex;
}


static jePath_TimeType JETCF jePath_AdjustTimeForLooping(
	jeBoolean Looped,
	jePath_TimeType Time, 
	jePath_TimeType TStart, 
	jePath_TimeType TEnd)
{
	if (Looped!=JE_FALSE)
	{
		if (Time < TStart)
		{
			return (jePath_TimeType)fmod(Time - TStart, TEnd - TStart) + TStart + TEnd;
		}
		else
		{
			if (Time >= TEnd)
			{
				if(TStart + JE_TKA_TIME_TOLERANCE > TEnd)
					return TStart;

				return (jePath_TimeType)fmod(Time - TStart, TEnd - TStart) + TStart;
			}
			else
			{
				return Time;
			}
		}
	}
	else
	{
		return Time;
	}
}


static jeBoolean JETCF jePath_SampleChannel(
	const jePath_Channel *Channel,			// channel to sample
	jeBoolean Looped,
	jeBoolean AllowCuts,
	jePath_TimeType Time, 
	void *Result)
				// return JE_TRUE if sample was made,
				// return JE_FALSE if no sample was made (no keyframes)
{
	int Index1,Index2;				// index of keyframe just before and after Time
	jePath_TimeType Time1, Time2;	// Times in those keyframes	
	jePath_TimeType T;				// 0..1 blending factor
	jePath_TimeType AdjTime;		// parameter Time adjusted for looping.
	int Length;
	
	assert( Channel != NULL );
	assert( Result != NULL );

	if (Channel->KeyList == NULL)	
		return JE_FALSE;
	
	Length = jeTKArray_NumElements( Channel->KeyList );
			
	if ( Length == 0 )
	{
		//Interpolate(Channel,NULL,NULL,Time,Result);
		return JE_FALSE;
	}

	AdjTime = jePath_AdjustTimeForLooping(Looped,Time,
			Channel->StartTime,Channel->EndTime);

	if (	( Channel->LastKey1Time <= AdjTime ) && 
			( AdjTime < Channel->LastKey2Time  ) )
	{  
		Index1 = Channel->LastKey1;
		Index2 = Channel->LastKey2;
		Time1  = Channel->LastKey1Time;
		Time2  = Channel->LastKey2Time;
	}
	else
	{
		Index1 = jeTKArray_BSearch( Channel->KeyList,
								AdjTime);
		Index2 = Index1 + 1;

		// edje conditions: if Time is off end of path's time, use end point twice
		if ( Index1 < 0 )	
		{
			if (Looped!=JE_FALSE) 
			{
				Index1 = Length -1;
			}
			else
			{
				Index1 = 0;
			}
		}
		if ( Index2 >= Length )
		{
			if (Looped!=JE_FALSE)
			{
				Index2 = 0;
			}
			else
			{
				Index2 = Length - 1;
			}
		}
		((jePath_Channel *)Channel)->LastKey1 = Index1;
		((jePath_Channel *)Channel)->LastKey2 = Index2;
		Time1 = ((jePath_Channel *)Channel)->LastKey1Time = jeTKArray_ElementTime(Channel->KeyList, Index1);
		Time2 = ((jePath_Channel *)Channel)->LastKey2Time = jeTKArray_ElementTime(Channel->KeyList, Index2);
	}
	
	if (Index1 == Index2)
		T=0.0f;			// Time2 == Time1 !
	else
		{
			if (AllowCuts && ((Time2-Time1)<JE_PATH_MAXIMUM_CUT_TIME))
				{
					T=0.0f;
				}
			else	
				{
					T = (AdjTime-Time1) / (Time2 - Time1);
				}
		}
	
	jePath_Statics.InterpolationTable[Channel->InterpolationType](
				jeTKArray_Element(Channel->KeyList,Index1),
				jeTKArray_Element(Channel->KeyList,Index2),
				T,Result);

	return JE_TRUE;
}


JETAPI void JETCC jePath_Sample(const jePath *P, jePath_TimeType Time, jeXForm3d *Matrix)
{
	jeQuaternion	Rotation;
	jeVec3d		Translation;

	assert( P != NULL );
	assert( Matrix != NULL );


	if (P->Dirty)
		{
			jePath_Recompute((jePath *)P);
		}

	if(jePath_SampleChannel(&(P->Rotation), P->Looped, P->AllowCuts, Time, (void*)&Rotation) == JE_TRUE)
	{
		jeQuaternion_ToMatrix(&Rotation, Matrix);
	}
	else
	{
		jeXForm3d_SetIdentity(Matrix);
	}

	if(jePath_SampleChannel(&(P->Translation), P->Looped, P->AllowCuts, Time, (void*)&Translation) == JE_TRUE)
	{
		Matrix->Translation = Translation;
	}
	else
	{
		Matrix->Translation.X = Matrix->Translation.Y = Matrix->Translation.Z = 0.0f;
	}

}

JETAPI void JETCC jePath_SampleChannels(const jePath *P, jePath_TimeType Time, jeQuaternion *Rotation, jeVec3d *Translation)
{
	jeBoolean Looped;
	assert( P != NULL );
	assert( Rotation != NULL );
	assert( Translation != NULL );

	if (P->Dirty)
		{
			jePath_Recompute((jePath *)P);
		}

	if (P->Looped)
		Looped = JE_TRUE;
	else
		Looped = JE_FALSE;
	
	if(jePath_SampleChannel(&(P->Rotation), Looped, P->AllowCuts, Time, (void*)Rotation) == JE_FALSE)
	{
		jeQuaternion_SetNoRotation(Rotation);
	}

	if(jePath_SampleChannel(&(P->Translation), Looped, P->AllowCuts, Time, (void*)Translation) == JE_FALSE)
	{
		Translation->X  = Translation->Y = Translation->Z = 0.0f;
	}
}


JETAPI jeBoolean JETCC jePath_GetTimeExtents(const jePath *P, jePath_TimeType *StartTime, jePath_TimeType *EndTime)
	// returns false and times are unchanged if there is no extent (no keys)
{
	jePath_TimeType TransStart,TransEnd,RotStart,RotEnd;

	int RCount,TCount;
	assert( P != NULL );
	assert( StartTime != NULL );
	assert( EndTime != NULL );
	// this is a pain because each channel may have 0,1, or more keys
	
	if (P->Rotation.KeyList!=NULL)
		RCount = jeTKArray_NumElements( P->Rotation.KeyList );
	else
		RCount = 0;

	if (P->Translation.KeyList!=NULL)
		TCount = jeTKArray_NumElements( P->Translation.KeyList );
	else
		TCount = 0;
	
	if (RCount>0)
		{	
			RotStart = jeTKArray_ElementTime(P->Rotation.KeyList, 0);
			if (RCount>1)
				{
					RotEnd = jeTKArray_ElementTime(P->Rotation.KeyList, RCount-1);
				}
			else
				{
					RotEnd = RotStart;
				}
			if (TCount>0)
				{	// Rotation and Translation keys
					TransStart = jeTKArray_ElementTime(P->Translation.KeyList, 0);
					if (TCount>1)
						{
							TransEnd = jeTKArray_ElementTime(P->Translation.KeyList,TCount-1);
						}
					else
						{
							TransEnd = TransStart;
						}

					*StartTime = min(TransStart,RotStart);
					*EndTime   = max(TransEnd,RotEnd);
				}
			else
				{	// No Translation Keys
					*StartTime = RotStart;
					*EndTime   = RotEnd;
				}
		}
	else
		{  // No Rotation Keys
			if (TCount>0)
				{
					*StartTime = jeTKArray_ElementTime(P->Translation.KeyList, 0);
					if (TCount>1)
						{
							*EndTime = jeTKArray_ElementTime(P->Translation.KeyList,TCount-1);
						}
					else
						{
							*EndTime = *StartTime;
						}
				}
			else
				{	// No Rotation or Translation keys
					return JE_FALSE;
				}
		}
	return JE_TRUE;	
}


#define JE_PATH_FILE_VERSION 0x1002		//15 bits!

/*
	file header:
	 15 bit version id, 
	 7 bit Rotation InterpolationType,
	 7 bit Translation InterpolationType, 
	 1 bit for translation keys exist, 
	 1 bit for rotation keys exist
	 1 bit for allow cuts
*/
#define JE_PATH_MAX_INT_TYPE_COUNT      (127)		// 7 bits 
#define JE_PATH_TRANS_SHIFT_INTO_HEADER (10)		// 7 bits shifted into bits 10..
#define JE_PATH_ROT_SHIFT_INTO_HEADER   (3)			// 7 bits shifted into bits 3..

JETAPI jeBoolean JETCC jePath_WriteToFile(const jePath *P, jeVFile *F)
{
	uint32 Header;
	int C,R,T,Looped;

	assert( F != NULL );
	assert( P != NULL );
	assert( JE_PATH_FILE_VERSION < 0xFFFF );

	C=R=T=0;

	if (P->Rotation.KeyList != NULL)
		{
			if (jeTKArray_NumElements(P->Rotation.KeyList)>0)
				{
					R = JE_TRUE;
				}
		}
				
	if (P->Translation.KeyList != NULL)
		{
			if (jeTKArray_NumElements(P->Translation.KeyList)>0)
				{
					T = JE_TRUE;
				}
		}

	if (P->AllowCuts)
		C=1;

	if (P->Looped)
		Looped = 1;
	else
		Looped = 0;
	assert( P->Translation.InterpolationType <= JE_PATH_MAX_INT_TYPE_COUNT);	
	assert( P->Rotation.InterpolationType <= JE_PATH_MAX_INT_TYPE_COUNT);		

	Header = 
		(JE_PATH_FILE_VERSION << 17) |
		(C)     | 
		(T<<1)  | 
		(R<<2) 	| 
		(P->Translation.InterpolationType << JE_PATH_TRANS_SHIFT_INTO_HEADER) | 
		(P->Rotation.InterpolationType    << JE_PATH_ROT_SHIFT_INTO_HEADER  );

	if	(jeVFile_Write(F, &Header,sizeof(uint32)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_WRITE ,"jePath_WriteToFile: Failure to write Path File Header.");
			return JE_FALSE;
		}

	if (T==1)
		{
			if (jeVKFrame_WriteToFile( F, P->Translation.KeyList, 
										jePath_PathToVKInterpolation(P->Translation.InterpolationType),
										Looped)==JE_FALSE)
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE ,"jePath_WriteToFile.");
					return JE_FALSE;
				}
		}
	if (R==1)
		{
			if (jeQKFrame_WriteToFile( F, P->Rotation.KeyList, 
										jePath_PathToQKInterpolation(P->Rotation.InterpolationType),
										Looped)==JE_FALSE)
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE ,"jePath_WriteToFile.");
					return JE_FALSE;
				}
		}
	
	return JE_TRUE;
}


JETAPI jePath* JETCC jePath_CreateFromFile(jeVFile* F)
{
	jePath *P;
	int Looping;//int Interp,Looping;
	jeVKFrame_InterpolationType Interp;
	
	uint32 Header;

	assert( F != NULL );
	
	if(jeVFile_Read(F, &Header, sizeof(Header)) == JE_FALSE)
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ , "jePath_CreateFromFile.");
		return NULL;
	}

	if ((Header>>17) != JE_PATH_FILE_VERSION)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_VERSION, "jePath_CreateFromFile: Bad path file version.");
			return NULL;
		}

	P = (jePath *)jeRam_AllocateClear(sizeof(jePath));
	if (P == NULL)
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "jePath_CreateFromFile.");
			return NULL;
		}
	P->Translation.KeyList = NULL;
	P->Rotation.KeyList = NULL;
	
	P->Translation.InterpolationType = (jePath_InterpolationType) ((int)(Header >> JE_PATH_TRANS_SHIFT_INTO_HEADER) & JE_PATH_MAX_INT_TYPE_COUNT);
	P->Rotation.InterpolationType    = (jePath_InterpolationType) ((int)(Header >> JE_PATH_ROT_SHIFT_INTO_HEADER) & JE_PATH_MAX_INT_TYPE_COUNT);
	// this will be replaced by the path reader (if the path has keys)

	P->Translation.LastKey1Time = 0.0f;
	P->Translation.LastKey2Time = -1.0f;

	P->Rotation.LastKey1Time = 0.0f;
	P->Rotation.LastKey2Time = -1.0f;
	P-> Dirty    = 0;
	P-> Looped   = 0;
	P-> RefCount = 0;

	if ((Header >> 1) & 0x1)
		{
			P->Translation.KeyList = jeVKFrame_CreateFromFile(F,&Interp,&Looping,JE_PATH_MAXIMUM_CUT_TIME);
			if (P->Translation.KeyList == NULL)
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jePath_CreateFromFile.");
					jeRam_Free(P);
					return NULL;
				}
			P->Translation.InterpolationType = jePath_VKToPathInterpolation(Interp);
			if( Looping != 0 )
				P->Looped = FLAG_LOOPED;
		}

	if ((Header >> 2) & 0x1)
		{
			P->Rotation.KeyList = jeQKFrame_CreateFromFile(F,(jeQKFrame_InterpolationType *)&Interp,&Looping,JE_PATH_MAXIMUM_CUT_TIME);
			if (P->Rotation.KeyList == NULL)
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jePath_CreateFromFile.");
					if (P->Translation.KeyList != NULL)
						{
							jeTKArray_Destroy(&P->Translation.KeyList);
						}
					jeRam_Free(P);
					return NULL;
				}
			P->Rotation.InterpolationType = jePath_QKToPathInterpolation((jeQKFrame_InterpolationType)Interp);
			if( Looping != 0 )
				P->Looped = FLAG_LOOPED;

		}
	P->AllowCuts = (Header & 0x1);
	P->Dirty = FLAG_DIRTY;
	return P;
}
