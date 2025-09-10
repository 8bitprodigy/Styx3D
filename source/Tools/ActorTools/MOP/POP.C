/****************************************************************************************/
/*  POP.C																				*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Path optimizer.														*/
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
#include <math.h>

#include "pop.h"
#include "path.h"

#include "MkUtil.h"   // ONLY for Interrupt!


#define xxSUPERSAMPLE

jeBoolean Pop_RotationCompare( const jePath *PLong, const jePath *PShort, jeFloat Tolerance)
{
	jeXForm3d M1;
	jeFloat T;
	#ifdef SUPERSAMPLE
		jeFloat StartTime,EndTime;
		jeFloat LastT=0.0f;
		jeFloat DT=0.0f;
	#endif
	jeQuaternion Q1,Q2;
	jeVec3d V1,V2;

	int Count;
	int i;

	assert( PLong != NULL );
	assert( PShort != NULL );
	
	Count = jePath_GetKeyframeCount(PLong,JE_PATH_ROTATION_CHANNEL);
	if (Count == 0)
		{
			return JE_TRUE;
		}
	assert( jePath_GetKeyframeCount(PShort,JE_PATH_ROTATION_CHANNEL) <= Count );

	for (i=0; i<Count; i++)
		{
			jePath_GetKeyframe(PLong,i,JE_PATH_ROTATION_CHANNEL, &T, &M1);
			jePath_SampleChannels(PShort, T,&Q1,&V1);
			jePath_SampleChannels(PLong , T,&Q2,&V2);
			if (jeQuaternion_Compare(&Q1,&Q2,Tolerance) == JE_FALSE)
				return JE_FALSE; 
			#ifdef SUPERSAMPLE
				if (i>0)
					{
						if (i>1)
							{
								if (T-LastT < DT)
									{
										DT = T-LastT;
									}
							}
						else
							{
								DT = T-LastT;
							}
					}
				LastT = T;
			#endif
		}

	#ifdef SUPERSAMPLE
		DT = DT / 3.0f;   // 'oversample' at 3 times

		if (Count>0)
			{
				jePath_GetKeyframe(PLong,0,JE_PATH_ROTATION_CHANNEL, &StartTime, &M1);
			}
		if (Count>1)
			{
				jePath_GetKeyframe(PLong,Count-1,JE_PATH_ROTATION_CHANNEL, &EndTime, &M1);
			}
		else
			{
				EndTime = StartTime;
			}

		for (T=StartTime; T<=EndTime; T+=DT)
			{
				jePath_SampleChannels(PLong,  T, &Q1, &V1);
				jePath_SampleChannels(PShort, T, &Q2, &V2);
				if (jeQuaternion_Compare(&Q1,&Q2,Tolerance) == JE_FALSE)
					{
						return JE_FALSE; 
					}
			}
	#endif

	return JE_TRUE;
}


jeBoolean Pop_ZapRotationsIfAllKeysEqual( jePath *P, jeFloat Tolerance, jeBoolean *AllEqual )
{
	jeXForm3d M1;
	jeFloat T;
	jeQuaternion Q1,Q2;
	jeVec3d V1,V2;

	int Count;
	int i;

	assert( AllEqual != NULL );
	assert( P != NULL );

	*AllEqual = JE_FALSE;

	Count = jePath_GetKeyframeCount(P,JE_PATH_ROTATION_CHANNEL);
	if (Count == 0)
		{
			return JE_TRUE;
		}
	jePath_GetKeyframe(P,0,JE_PATH_ROTATION_CHANNEL, &T, &M1);
	jePath_SampleChannels(P, T,&Q1,&V1);

	for (i=1; i<Count; i++)
		{
			jePath_GetKeyframe(P,i,JE_PATH_ROTATION_CHANNEL, &T, &M1);
			jePath_SampleChannels(P , T,&Q2,&V2);
			if (jeQuaternion_Compare(&Q1,&Q2,Tolerance) == JE_FALSE)
				return JE_TRUE; 
		}

	// if we get to here, all keys are equal.  (delete all but first and last)
	for (i=Count-2; i>0; i--)
		{
			//jePath_GetKeyframe(P,i,JE_PATH_ROTATION_CHANNEL, &T, &M1);
			if (jePath_DeleteKeyframe(P,i,JE_PATH_ROTATION_CHANNEL) == JE_FALSE)
				{
					return JE_FALSE;
				}
		}

	*AllEqual = JE_TRUE;
	return JE_TRUE;
}


#if 0
jeBoolean Pop_RotationComparePortion( jePath *PLong, jePath *PShort, jeFloat Tolerance,int StartIndex,int EndIndex)
{
	jeXForm3d M1;
	jeFloat T;
	#ifdef SUPERSAMPLE
		jeFloat StartTime,EndTime;
		jeFloat LastT=-9e29f;
		jeFloat DT=9e29f;
	#endif
	jeQuaternion Q1,Q2;
	jeVec3d V1,V2;

	int Count;
	int i,n;

	assert( PLong != NULL );
	assert( PShort != NULL );
	
	Count = jePath_GetKeyframeCount(PLong,JE_PATH_ROTATION_CHANNEL);
	if (Count == 0)
		{
			return JE_TRUE;
		}
	assert( jePath_GetKeyframeCount(PShort,JE_PATH_ROTATION_CHANNEL) <= Count );


	for (i=StartIndex; i<EndIndex; i++)
		{
			n = i;
			if (n < 0)
				{
					n = Count + n;
				}
			if (n >= Count)
				{
					n = n-Count;
				}

			jePath_GetKeyframe(PLong,n,JE_PATH_ROTATION_CHANNEL, &T, &M1);
			//jeQuaternion_FromMatrix(&M1,&Q1);
			jePath_SampleChannels(PShort, T,&Q1,&V1);
			jePath_SampleChannels(PLong , T,&Q2,&V2);
			if (jeQuaternion_Compare(&Q1,&Q2,Tolerance) == JE_FALSE)
				return JE_FALSE; 
			#ifdef SUPERSAMPLE
				if (fabs(T-LastT) < DT)
					{
						DT = (jeFloat)fabs(T-LastT);
					}
				LastT = T;
			#endif
		}

	#ifdef SUPERSAMPLE
		DT = DT / 3.0f;   // 'oversample' at 3 times


		if (StartIndex < 0)
			{
				StartIndex = Count + StartIndex;
			}
		if (StartIndex >= Count)
			{
				StartIndex = StartIndex-Count;
			}
		if (EndIndex < 0)
			{
				EndIndex = Count + EndIndex;
			}
		if (EndIndex >= Count)
			{
				EndIndex = EndIndex-Count;
			}

		if (Count==1)
			{		// if count is 0, it didn't get here
					// if count is 1, there is only one possible return value, and that is checked in above loop.
				return JE_TRUE;
			}

		jePath_GetKeyframe(PLong,StartIndex,JE_PATH_ROTATION_CHANNEL, &StartTime, &M1);
		jePath_GetKeyframe(PLong,EndIndex,JE_PATH_ROTATION_CHANNEL, &EndTime, &M1);

		if (EndTime>StartTime)
			{
				for (T=StartTime; T<=EndTime; T+=DT)
					{
						jePath_SampleChannels(PLong,  T, &Q1, &V1);
						jePath_SampleChannels(PShort, T, &Q2, &V2);
						if (jeQuaternion_Compare(&Q1,&Q2,Tolerance) == JE_FALSE)
							{
								return JE_FALSE; 
							}
					}
			}
		else
			{
				jeFloat LastTime;
				jePath_GetKeyframe(PLong,Count-1,JE_PATH_ROTATION_CHANNEL, &LastTime, &M1);
				for (T=0.0f; T<=EndTime; T+=DT)
					{
						jePath_SampleChannels(PLong,  T, &Q1, &V1);
						jePath_SampleChannels(PShort, T, &Q2, &V2);
						if (jeQuaternion_Compare(&Q1,&Q2,Tolerance) == JE_FALSE)
							{
								return JE_FALSE; 
							}
					}
				for (T=StartTime; T<=LastTime; T+=DT)
					{
						jePath_SampleChannels(PLong,  T, &Q1, &V1);
						jePath_SampleChannels(PShort, T, &Q2, &V2);
						if (jeQuaternion_Compare(&Q1,&Q2,Tolerance) == JE_FALSE)
							{
								return JE_FALSE; 
							}
					}
			}
	#endif

	return JE_TRUE;
}
#endif


jeBoolean Pop_TranslationCompare( const jePath *PLong, const jePath *PShort, jeFloat Tolerance)
{
	jeXForm3d M1;
	jeFloat T;
	#ifdef SUPERSAMPLE
		jeFloat StartTime,EndTime;
		jeFloat LastT=0.0f;
		jeFloat DT=0.0f;
	#endif
	jeQuaternion Q1;
	//jeQuaternion Q2;
	jeVec3d V1;
	//veVec3d V2;

	int Count;
	int i;

	assert( PLong != NULL );
	assert( PShort != NULL );
	
	Count = jePath_GetKeyframeCount(PLong,JE_PATH_TRANSLATION_CHANNEL);
	if (Count == 0)
		{
			return JE_TRUE;
		}
	assert( jePath_GetKeyframeCount(PShort,JE_PATH_TRANSLATION_CHANNEL) <= Count );

	for (i=0; i<Count; i++)
		{
			jePath_GetKeyframe(PLong,i,JE_PATH_TRANSLATION_CHANNEL, &T, &M1);
			jePath_SampleChannels(PShort, T,&Q1,&V1);
			//jePath_SampleChannels(PLong , T,&Q2,&V2);
			if (jeVec3d_Compare(&V1, &(M1.Translation),Tolerance) == JE_FALSE)
				{
					return JE_FALSE; 
				}
			#ifdef SUPERSAMPLE
				if (i>0)
					{
						if (i>1)
							{
								if (T-LastT < DT)
									{
										DT = T-LastT;
									}
							}
						else
							{
								DT = T-LastT;
							}
					}
				LastT = T;
			#endif
		}

	#ifdef SUPERSAMPLE

		DT = DT / 3.0f;   // 'oversample' at 3 times

		if (Count>0)
			{
				jePath_GetKeyframe(PLong,0,JE_PATH_TRANSLATION_CHANNEL, &StartTime, &M1);
			}
		if (Count>1)
			{
				jePath_GetKeyframe(PLong,Count-1,JE_PATH_TRANSLATION_CHANNEL, &EndTime, &M1);
			}
		else
			{
				EndTime = StartTime;
			}

		for (T=StartTime; T<=EndTime; T+=DT)
			{
				jePath_SampleChannels(PLong,  T, &Q1, &V1);
				jePath_SampleChannels(PShort, T, &Q2, &V2);
				if (jeVec3d_Compare(&V1, &V2,Tolerance) == JE_FALSE)
					{
						return JE_FALSE; 
					}
			}

	#endif
	return JE_TRUE;
}

jeBoolean Pop_ZapTranslationsIfAllKeysEqual( jePath *P, jeFloat Tolerance, jeBoolean *AllEqual )
{
	jeXForm3d M1;
	jeFloat T;
	jeQuaternion Q1,Q2;
	jeVec3d V1,V2;

	int Count;
	int i;

	assert( P != NULL );
	assert( AllEqual != NULL );

	*AllEqual = JE_FALSE;
	
	Count = jePath_GetKeyframeCount(P,JE_PATH_TRANSLATION_CHANNEL);
	if (Count == 0)
		{
			return JE_TRUE;
		}
	jePath_GetKeyframe(P,0,JE_PATH_TRANSLATION_CHANNEL, &T, &M1);
	jePath_SampleChannels(P, T,&Q1,&V1);

	for (i=1; i<Count; i++)
		{
			jePath_GetKeyframe(P,i,JE_PATH_TRANSLATION_CHANNEL, &T, &M1);
			jePath_SampleChannels(P , T,&Q2,&V2);
			if (jeVec3d_Compare(&V1, &V2,Tolerance) == JE_FALSE)
				return JE_TRUE; 
		}

	// if we get to here, all keys are equal.  (delete all but first and last)
	for (i=Count-2; i>0; i--)
		{
			//jePath_GetKeyframe(P,i,JE_PATH_TRANSLATION_CHANNEL, &T, &M1);
			if (jePath_DeleteKeyframe(P,i,JE_PATH_TRANSLATION_CHANNEL) == JE_FALSE)
				{
					return JE_FALSE;
				}
		}

	*AllEqual = JE_TRUE;
	return JE_TRUE;
}

#if 0
jeBoolean Pop_TranslationComparePortion( jePath *PLong, jePath *PShort, jeFloat Tolerance,int StartIndex,int EndIndex)
{
	jeXForm3d M1;
	jeFloat T;
	#ifdef SUPERSAMPLE
		jeFloat StartTime,EndTime;
		jeFloat LastT=-9e29f;
		jeFloat DT=9e29f;
	#endif
	jeQuaternion Q1,Q2;
	jeVec3d V1,V2;

	int Count;
	int i,n;

	assert( PLong != NULL );
	assert( PShort != NULL );
	
	Count = jePath_GetKeyframeCount(PLong,JE_PATH_TRANSLATION_CHANNEL);
	if (Count == 0)
		{
			return JE_TRUE;
		}
	assert( jePath_GetKeyframeCount(PShort,JE_PATH_TRANSLATION_CHANNEL) <= Count );


	for (i=StartIndex; i<EndIndex; i++)
		{
			n = i;
			if (n < 0)
				{
					n = Count + n;
				}
			if (n >= Count)
				{
					n = n-Count;
				}

			jePath_GetKeyframe(PLong,n,JE_PATH_TRANSLATION_CHANNEL, &T, &M1);
			//jeQuaternion_FromMatrix(&M1,&Q1);
			jePath_SampleChannels(PShort, T,&Q1,&V1);
			jePath_SampleChannels(PLong , T,&Q2,&V2);
			if (jeVec3d_Compare(&V1, &V2,Tolerance) == JE_FALSE)
				return JE_FALSE; 
			#ifdef SUPERSAMPLE
				if (fabs(T-LastT) < DT)
					{
						DT = (jeFloat)fabs(T-LastT);
					}
				LastT = T;
			#endif
		}

	#ifdef SUPERSAMPLE

		DT = DT / 3.0f;   // 'oversample' at 3 times


		if (StartIndex < 0)
			{
				StartIndex = Count + StartIndex;
			}
		if (StartIndex >= Count)
			{
				StartIndex = StartIndex-Count;
			}
		if (EndIndex < 0)
			{
				EndIndex = Count + EndIndex;
			}
		if (EndIndex >= Count)
			{
				EndIndex = EndIndex-Count;
			}

		if (Count==1)
			{		// if count is 0, it didn't get here
					// if count is 1, there is only one possible return value, and that is checked in above loop.
				return JE_TRUE;
			}


		jePath_GetKeyframe(PLong,StartIndex,JE_PATH_TRANSLATION_CHANNEL, &StartTime, &M1);
		jePath_GetKeyframe(PLong,EndIndex,JE_PATH_TRANSLATION_CHANNEL, &EndTime, &M1);

		if (EndTime>StartTime)
			{
				for (T=StartTime; T<=EndTime; T+=DT)
					{
						jePath_SampleChannels(PLong,  T, &Q1, &V1);
						jePath_SampleChannels(PShort, T, &Q2, &V2);
						if (jeVec3d_Compare(&V1, &V2,Tolerance) == JE_FALSE)
							{
								return JE_FALSE; 
							}
					}
			}
		else
			{
				jeFloat LastTime;
				jePath_GetKeyframe(PLong,Count-1,JE_PATH_TRANSLATION_CHANNEL, &LastTime, &M1);
				for (T=0.0f; T<=EndTime; T+=DT)
					{
						jePath_SampleChannels(PLong,  T, &Q1, &V1);
						jePath_SampleChannels(PShort, T, &Q2, &V2);
						if (jeVec3d_Compare(&V1, &V2,Tolerance) == JE_FALSE)
							{
								return JE_FALSE; 
							}
					}
				for (T=StartTime; T<=LastTime; T+=DT)
					{
						jePath_SampleChannels(PLong,  T, &Q1, &V1);
						jePath_SampleChannels(PShort, T, &Q2, &V2);
						if (jeVec3d_Compare(&V1, &V2,Tolerance) == JE_FALSE)
							{
								return JE_FALSE; 
							}
					}
			}
	#endif

	return JE_TRUE;
}
#endif

jeBoolean pop_ZapLastKey(const jePath *P, jePath *Popt, int Channel1, int Channel2, jeFloat Tolerance)
{
	assert( Popt != NULL );
	assert( Channel1 != Channel2 );
	assert( Channel1 == JE_PATH_ROTATION_CHANNEL || Channel1 == JE_PATH_TRANSLATION_CHANNEL );
	assert( Channel2 == JE_PATH_ROTATION_CHANNEL || Channel2 == JE_PATH_TRANSLATION_CHANNEL );
	
	if (jePath_GetKeyframeCount(Popt, Channel1)==2)
		{
			if (jePath_GetKeyframeCount(Popt, Channel2)>=2)
				{
					jeFloat T1,T2,StartTime1,EndTime1,StartTime2,EndTime2;
					jeXForm3d M;
					jeQuaternion Q1,Q2;
					jeVec3d V1,V2;

					if (jePath_GetTimeExtents(Popt, &StartTime1, &EndTime1)==JE_FALSE)
						{
							return JE_FALSE;
						}
			
					jePath_GetKeyframe(Popt,0,Channel1, &T1, &M);
					jePath_GetKeyframe(Popt,1,Channel1, &T2, &M);
					
					jePath_SampleChannels(Popt, T1, &Q1, &V1);
					jePath_SampleChannels(Popt, T2, &Q2, &V2);
					
					// first and last key have to be equal!
					if (Channel1 == JE_PATH_ROTATION_CHANNEL)
						{
							if (jeQuaternion_Compare(&Q1,&Q2,Tolerance) == JE_FALSE)
								{
									return JE_TRUE;
								}
						}
					else
						{
							if (jeVec3d_Compare(&V1, &V2,Tolerance) == JE_FALSE)
								{
									return JE_TRUE; 
								}
						}

					if (jePath_DeleteKeyframe(Popt,1,Channel1) == JE_FALSE)
						{
							return JE_FALSE;
						}
					if (jePath_GetTimeExtents(Popt,&StartTime2,&EndTime2)==JE_FALSE)
						{	// cant get extents: try to reverse change and bail out
							if (jePath_InsertKeyframe(Popt,Channel1,T2,&M) == JE_FALSE)
								{
									return JE_FALSE;
								}
							return JE_FALSE;
						}
					if (      (fabs(StartTime1-StartTime2) > Tolerance)
						  ||  (fabs(EndTime1-EndTime2) > Tolerance)     )
						{	// new extents are bad: reverse change and bail out
							if (jePath_InsertKeyframe(Popt,Channel1,T2,&M) == JE_FALSE)
								{
									return JE_FALSE;
								}
							return JE_FALSE;
						}

						
					if (Channel1 == JE_PATH_ROTATION_CHANNEL)
						{
							if ( Pop_RotationCompare   (P,Popt,Tolerance)==JE_FALSE )
								{	// new path has too much error: reverse change and bail out
									if (jePath_InsertKeyframe(Popt,Channel1,T2,&M) == JE_FALSE)
										{
											return JE_FALSE;
										}
									return JE_FALSE;
								}
						}
					else
						{
							if (Pop_TranslationCompare(P,Popt,Tolerance)==JE_FALSE )
								{
									if (jePath_InsertKeyframe(Popt,Channel1,T2,&M) == JE_FALSE)
										{
											return JE_FALSE;
										}
									return JE_FALSE;
								}
						}
					return JE_TRUE;
				}	
		}
	return JE_TRUE;
}

jeBoolean pop_ZapIdentityKey(jePath *P, jePath *Popt,int Channel,jeFloat Tolerance)
{
	jeFloat T;
	jeXForm3d M;
	jeQuaternion Q;
	jeVec3d V;

	assert( Popt != NULL );
	assert( Channel == JE_PATH_ROTATION_CHANNEL || Channel == JE_PATH_TRANSLATION_CHANNEL );
	
	if (jePath_GetKeyframeCount(Popt, Channel)==1)
		{
			jePath_GetKeyframe(Popt,0,Channel, &T, &M);
			jePath_SampleChannels(Popt, T,&Q,&V);

			switch (Channel)
				{
					case (JE_PATH_ROTATION_CHANNEL):
						{
							jeQuaternion QI;
							jeQuaternion_SetNoRotation(&QI);
							if (jeQuaternion_Compare(&Q,&QI, Tolerance) == JE_TRUE)
								{
									if (jePath_DeleteKeyframe(Popt,0,Channel) == JE_FALSE)
										{
											return JE_FALSE;
										}
									if ( Pop_RotationCompare   (P,Popt,Tolerance)==JE_FALSE )
										{
											if (jePath_InsertKeyframe(Popt,JE_PATH_ROTATION_CHANNEL,T,&M) == JE_FALSE)
												{
													return JE_FALSE;
												}
										}
									assert( Pop_RotationCompare   (P,Popt,Tolerance)!=JE_FALSE );
								}
						}
						break;
					case (JE_PATH_TRANSLATION_CHANNEL):
						{
							jeVec3d VI;
							jeVec3d_Clear(&VI);
							if (jeVec3d_Compare(&V, &VI ,Tolerance) == JE_TRUE)
								{
									if (jePath_DeleteKeyframe(Popt,0,Channel) == JE_FALSE)
										{
											return JE_FALSE;
										}
									if ( Pop_TranslationCompare(P,Popt,Tolerance)==JE_FALSE )
										{
											if (jePath_InsertKeyframe(Popt,JE_PATH_TRANSLATION_CHANNEL,T,&M) == JE_FALSE)
												{
													return JE_FALSE;
												}
										}
									assert( Pop_TranslationCompare(P,Popt,Tolerance)!=JE_FALSE );
								}
						}
						break;
					default:
						assert(0);
				}
		}

	return JE_TRUE;
}

#if 0
jePath *pop_CreateCopy(jePath *Src)
{
	jePath *P;
	
	int i,Count;
	int RInterp=0;
	int TInterp=0;

	assert ( Src != NULL );

	P = jePath_Create(JE_PATH_INTERPOLATE_HERMITE, JE_PATH_INTERPOLATE_SQUAD,JE_FALSE);	
	if (P == NULL)
		{
			return NULL;
		}

	{
		Count = jePath_GetKeyframeCount(Src,JE_PATH_TRANSLATION_CHANNEL);
		if (Count>0)
			{
				jeFloat Time;
				jeXForm3d M;

				for (i=0; i<Count; i++)
					{
						jePath_GetKeyframe(Src,i,JE_PATH_TRANSLATION_CHANNEL,&Time,&M);
						if (jePath_InsertKeyframe(P,JE_PATH_TRANSLATION_CHANNEL,Time,&M)==JE_FALSE)
							{
								jePath_Destroy(&P);
								return NULL;
							}
					}
			}
	}

	{
		Count = jePath_GetKeyframeCount(Src,JE_PATH_ROTATION_CHANNEL);
		if (Count>0)
			{
				jeFloat Time;
				jeXForm3d M;

				for (i=0; i<Count; i++)
					{
						jePath_GetKeyframe(Src,i,JE_PATH_ROTATION_CHANNEL,&Time,&M);
						if (jePath_InsertKeyframe(P,JE_PATH_ROTATION_CHANNEL,Time,&M)==JE_FALSE)
							{
								jePath_Destroy(&P);
								return NULL;
							}
					}
			}
	}

		return P;
}
#endif
					

jePath *Pop_PathOptimize( jePath *P, jeFloat Tolerance)
{
	int Count;
	jePath *Popt;
	int i,pass;
	jeBoolean AnyRemoved;
	jeFloat T;
	jeXForm3d M;
	jeBoolean AllEqualRotations,AllEqualTranslations;
	

	assert( P != NULL );

	Popt = jePath_CreateCopy(P);
	if (Popt==NULL)
		{
			return NULL;
		}


	AnyRemoved = JE_TRUE;
	pass = 0;

	if (Pop_ZapRotationsIfAllKeysEqual(Popt,Tolerance,&AllEqualRotations) == JE_FALSE)
		{
			jePath_Destroy(&Popt);
			return NULL;
		}
	//assert( Pop_RotationCompare   (P,Popt,Tolerance)!=JE_FALSE );
	
	if (Pop_ZapTranslationsIfAllKeysEqual(Popt,Tolerance,&AllEqualTranslations) == JE_FALSE)
		{
			jePath_Destroy(&Popt);
			return NULL;
		}
	//assert( Pop_TranslationCompare(P,Popt,Tolerance)!=JE_FALSE );

		
	if (AllEqualRotations)
		{
			// Try to kill last 2 keys, if the extent of the path is defined by the other channel...
			if (pop_ZapLastKey(P,Popt,JE_PATH_ROTATION_CHANNEL,JE_PATH_TRANSLATION_CHANNEL,Tolerance)==JE_FALSE)
				{
					jePath_Destroy(&Popt);
					return NULL;
				}
		}
	else
		{
			Count = jePath_GetKeyframeCount(Popt,JE_PATH_ROTATION_CHANNEL);
			//for (i=1; i< Count-1; i++)
			for (i=Count-2; i>=1; i--)
				{
					jePath_GetKeyframe(P,i,JE_PATH_ROTATION_CHANNEL, &T, &M);
					if (MkUtil_Interrupt())
						{
							jePath_Destroy(&Popt);
							return NULL;
						}
				
					if (jePath_DeleteKeyframe(Popt,i,JE_PATH_ROTATION_CHANNEL) == JE_FALSE)
						{
							jePath_Destroy(&Popt);
							return NULL;
						}
					//if ( Pop_RotationComparePortion(P,Popt,Tolerance,i-2,i+2)!=JE_FALSE )
					if ( Pop_RotationCompare   (P,Popt,Tolerance)!=JE_FALSE )
						{
							AnyRemoved = JE_TRUE;
						}
					else
						{
							//jePath_GetKeyframe(P,i,JE_PATH_ROTATION_CHANNEL, &T, &M);
							if (jePath_InsertKeyframe(Popt,JE_PATH_ROTATION_CHANNEL,T,&M) == JE_FALSE)
								{
									jePath_Destroy(&Popt);
									return NULL;
								}
						}
					/*
					if (Pop_RotationCompare   (P,Popt,Tolerance)==JE_FALSE )
						{
							jePath_GetKeyframe(P,i,JE_PATH_ROTATION_CHANNEL, &T, &M);
							Pop_RotationCompare   (P,Popt,Tolerance);
						}
					*/
					assert( Pop_RotationCompare   (P,Popt,Tolerance)!=JE_FALSE );
				}

			assert( Pop_RotationCompare   (P,Popt,Tolerance)!=JE_FALSE );
			assert( Pop_TranslationCompare(P,Popt,Tolerance)!=JE_FALSE );
			// Try to kill last 2 keys, if the extent of the path is defined by the other channel...
			if (pop_ZapLastKey(P,Popt,JE_PATH_ROTATION_CHANNEL,JE_PATH_TRANSLATION_CHANNEL,Tolerance)==JE_FALSE)
				{
					jePath_Destroy(&Popt);
					return NULL;
				}
			assert( Pop_RotationCompare   (P,Popt,Tolerance)!=JE_FALSE );
			if (pop_ZapIdentityKey(P,Popt,JE_PATH_ROTATION_CHANNEL,Tolerance)==JE_FALSE)
				{
					jePath_Destroy(&Popt);
					return NULL;
				}
			assert( Pop_RotationCompare   (P,Popt,Tolerance)!=JE_FALSE );
		}
	
	if (AllEqualTranslations)
		{
			// Try to kill last 2 keys, if the extent of the path is defined by the other channel...
			if (pop_ZapLastKey(P,Popt,JE_PATH_TRANSLATION_CHANNEL,JE_PATH_ROTATION_CHANNEL,Tolerance)==JE_FALSE)
				{
					jePath_Destroy(&Popt);
					return NULL;
				}
		}
	else
		{
			Count = jePath_GetKeyframeCount(Popt,JE_PATH_TRANSLATION_CHANNEL);
			//for (i=1; i< Count-1; i++)
			for (i=Count-2; i>=1; i--)
				{
					jePath_GetKeyframe(P,i,JE_PATH_TRANSLATION_CHANNEL, &T, &M);
					if (MkUtil_Interrupt())
						{
							jePath_Destroy(&Popt);
							return NULL;
						}
					if (jePath_DeleteKeyframe(Popt,i,JE_PATH_TRANSLATION_CHANNEL) == JE_FALSE)
						{
							jePath_Destroy(&Popt);
							return NULL;
						}
					//if ( Pop_TranslationComparePortion(P,Popt,Tolerance,i-2,i+2)!=JE_FALSE )
					if ( Pop_TranslationCompare(P,Popt,Tolerance)!=JE_FALSE )
						{
							AnyRemoved = JE_TRUE;
						}
					else
						{
							//jePath_GetKeyframe(P,i,JE_PATH_TRANSLATION_CHANNEL, &T, &M);
							if (jePath_InsertKeyframe(Popt,JE_PATH_TRANSLATION_CHANNEL,T,&M) == JE_FALSE)
								{
									jePath_Destroy(&Popt);
									return NULL;
								}
						}
					assert( Pop_TranslationCompare(P,Popt,Tolerance)!=JE_FALSE );
				}

			assert( Pop_TranslationCompare(P,Popt,Tolerance)!=JE_FALSE );
			assert( Pop_RotationCompare   (P,Popt,Tolerance)!=JE_FALSE );
			// Try to kill last 2 keys, if the extent of the path is defined by the other channel...
			if (pop_ZapLastKey(P,Popt,JE_PATH_TRANSLATION_CHANNEL,JE_PATH_ROTATION_CHANNEL,Tolerance)==JE_FALSE)
				{
					jePath_Destroy(&Popt);
					return NULL;
				}
			assert( Pop_TranslationCompare(P,Popt,Tolerance)!=JE_FALSE );
			if (pop_ZapIdentityKey(P,Popt,JE_PATH_TRANSLATION_CHANNEL,Tolerance)==JE_FALSE)
				{
					jePath_Destroy(&Popt);
					return NULL;
				}
			assert( Pop_TranslationCompare(P,Popt,Tolerance)!=JE_FALSE );
		}

			
	return Popt;
}





