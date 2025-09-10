/****************************************************************************************/
/*  MOTION.C	                                                                        */
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Motion implementation.				                                    */
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

/*
	This object is a list of (named) jePath objects, 
	and an associated event list 

*/
 
#include <assert.h>
#include <string.h>		// strcmp, strnicmp

#include "BaseType.h"
#include "Ram.h"
#include "Errorlog.h"
#include "Motion.h"
#include "TKEvents.h"
#include "StrBlock.h"

#pragma warning(disable : 4201)		// we're using nameless structures

#define jePath_TimeType jeFloat

#define MIN(aa,bb)  (( (aa)>(bb) ) ? (bb) : (aa) )
#define MAX(aa,bb)  (( (aa)>(bb) ) ? (aa) : (bb) )

typedef enum { MOTION_NODE_UNDECIDED, MOTION_NODE_BRANCH, MOTION_NODE_LEAF } jeMotion_NodeType;

#define MOTION_BLEND_PART_OF_TRANSFORM(TForm)  ((TForm).Translation.X)						
#define MOTION_BLEND_PART_OF_VECTOR(Vec)  ((Vec).X)						


typedef struct jeMotion_Leaf
{
	int			PathCount;		
	int32		NameChecksum;	// checksum based on names and list order
	jeTKEvents *Events;
	jeStrBlock *NameArray;
	jePath	  **PathArray;
} jeMotion_Leaf;


typedef struct jeMotion_Mixer
{
	jeFloat   TimeScale;		// multipler for time
	jeFloat   TimeOffset;		// already scaled.
	jePath   *Blend;			// path used to interpolate blending amounts. 
	jeXForm3d Transform;		// base transform for this motion (if TransformUsed==JE_TRUE)
	jeBoolean TransformUsed;	// JE_FALSE if there is no base transform.
	jeMotion *Motion;			
} jeMotion_Mixer;

typedef struct jeMotion_Branch
{
	int				MixerCount;
	int				CurrentEventIterator;
	jeMotion_Mixer *MixerArray;
} jeMotion_Branch;


typedef struct jeMotion
{
	char			 *Name;
	int				  CloneCount;
	jeBoolean		  MaintainNames;		
	jeMotion_NodeType NodeType;
	union 
		{
			jeMotion_Leaf   Leaf;
			jeMotion_Branch Branch;
		};
	jeMotion *SanityCheck;
} jeMotion;


JETAPI jeBoolean JETCC jeMotion_IsValid(const jeMotion *M)
{
	if (M == NULL)
		return JE_FALSE;
	if (M->SanityCheck!=M)
		return JE_FALSE;
	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeMotion_SetName(jeMotion *M, const char *Name)
{
	char *NewName;

	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	NewName = (char *)jeRam_Allocate( strlen(Name)+1 );
	if (NewName == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeMotion_SetName.");
			return JE_FALSE;
		}
	if (M->Name!=NULL)
		{
			jeRam_Free(M->Name);
		}
	M->Name = NewName;
	strcpy(M->Name, Name);
	return JE_TRUE;
}

JETAPI const char * JETCC jeMotion_GetName(const jeMotion *M)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	return M->Name;
}
		
static jeBoolean JETCF jeMotion_InitNodeAsLeaf(jeMotion *M,jeBoolean SetupStringBlock)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	assert( M->NodeType == MOTION_NODE_UNDECIDED );

	M->NodeType = MOTION_NODE_LEAF;
	
	M->Leaf.PathCount     = 0;
	M->Leaf.Events        = NULL;
	M->Leaf.PathArray     = NULL;
	M->Leaf.NameChecksum  = 0;
	if ((M->MaintainNames != JE_FALSE) && (SetupStringBlock!=JE_FALSE))
		{
			M->Leaf.NameArray = jeStrBlock_Create();
			if (M->Leaf.NameArray == NULL)	
				{
					jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeMotion_InitNodeAsLeaf");
					return JE_FALSE;
				}
		}
	else
		{
			M->Leaf.NameArray  = NULL;
		}
	return JE_TRUE;
}

static jeBoolean JETCF jeMotion_InitNodeAsBranch(jeMotion *M)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	assert( M->NodeType == MOTION_NODE_UNDECIDED );

	M->NodeType = MOTION_NODE_BRANCH;
	
	M->Branch.MixerCount           = 0;
	M->Branch.CurrentEventIterator = 0;
	M->Branch.MixerArray           = NULL;
	return JE_TRUE;
}


JETAPI jeMotion * JETCC jeMotion_Create(jeBoolean WithNames)
{
	jeMotion *M;
	assert( (WithNames==JE_TRUE) || (WithNames==JE_FALSE) );

	M = JE_RAM_ALLOCATE_STRUCT_CLEAR(jeMotion);

	if ( M == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeMotion_Create.");
			return NULL;
		}

	M->Name          = NULL;
	M->CloneCount	 = 0;
	M->MaintainNames = WithNames;
	M->NodeType      = MOTION_NODE_UNDECIDED;
	M->SanityCheck   = M;
	return M;
}


JETAPI jeBoolean JETCC jeMotion_RemoveNames(jeMotion *M)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	if (M->CloneCount > 0)
		{
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER,"jeMotion_RemoveNames: Can't remove names from a cloned motion.");
			return JE_FALSE;
		}

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				break;
			case (MOTION_NODE_BRANCH):
				jeErrorLog_Add(JE_ERR_BAD_PARAMETER,"jeMotion_RemoveNames: Can't remove names from a compound motion.");
				return JE_FALSE;
				break;
			case (MOTION_NODE_LEAF):
				assert( M->Leaf.PathCount >= 0 );
				
				if ( M->Leaf.NameArray != NULL )
					{	
						jeStrBlock_Destroy(&(M->Leaf.NameArray));
					}
				M->Leaf.NameArray = NULL;
				break;
			default:
				assert(0);
		}

	M->MaintainNames = JE_FALSE;
	return JE_TRUE;
}



JETAPI void JETCC jeMotion_Destroy(jeMotion **PM)
{
	int i;
	jeMotion *M;
	
	assert(PM   != NULL );
	assert(*PM  != NULL );
	M = *PM;
	assert( jeMotion_IsValid(M) != JE_FALSE );

	if (M->CloneCount > 0 )
		{
			M->CloneCount--;
			return;
		}

	if (M->Name != NULL)
		{
			jeRam_Free(M->Name);
			M->Name = NULL;
		}

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				break;
			case (MOTION_NODE_BRANCH):
				for (i=0; i<M->Branch.MixerCount; i++)
					{
						assert( M->Branch.MixerArray[i].Motion != NULL );
						jeMotion_Destroy( &(M->Branch.MixerArray[i].Motion));
						M->Branch.MixerArray[i].Motion = NULL;

						if (M->Branch.MixerArray[i].Blend != NULL )
							{
								jePath_Destroy( &(M->Branch.MixerArray[i].Blend));
								M->Branch.MixerArray[i].Blend = NULL;
							}
						
					}
				if (M->Branch.MixerArray != NULL)
					{
						jeRam_Free(M->Branch.MixerArray);
						M->Branch.MixerArray = NULL;
					}
				M->Branch.MixerCount = 0;
				M->Branch.CurrentEventIterator = 0;
				break;
			case (MOTION_NODE_LEAF):
				if (M->MaintainNames == JE_TRUE)
					{	
						jeBoolean Test=	jeMotion_RemoveNames(M);
						assert( Test != JE_FALSE );
						Test;
					}
				for (i=0; i< M->Leaf.PathCount; i++)
					{
						assert( M->Leaf.PathArray[i] );
						jePath_Destroy( &( M->Leaf.PathArray[i] ) );
						M->Leaf.PathArray[i] = NULL;
					}
				if (M->Leaf.PathArray!=NULL)
					{
						jeRam_Free(M->Leaf.PathArray);
						M->Leaf.PathArray = NULL;
					}
				M->Leaf.PathCount = 0;
				if ( M->Leaf.Events != NULL )
					{
						jeTKEvents_Destroy( &(M->Leaf.Events) );
					}
				break;
			default:
				assert(0);
		}
	M->NodeType = MOTION_NODE_UNDECIDED;
	jeRam_Free( *PM );
	*PM = NULL;
}

JETAPI jeBoolean JETCC jeMotion_AddPath(jeMotion *M,
	jePath *P,const char *Name,int *PathIndex)
{
	int PathCount;
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				if (jeMotion_InitNodeAsLeaf(M,JE_TRUE)==JE_FALSE)
					{
						jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_AddPath.");
						return JE_FALSE;
					}
				break;
			case (MOTION_NODE_BRANCH):
				jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeMotion_AddPath.");
				return JE_FALSE;
			case (MOTION_NODE_LEAF):
				break;
			default:
				assert(0);
		}

	assert( M->Leaf.PathCount >= 0 );

	if (Name!=NULL)
		{
			if (jeMotion_GetPathNamed( M, Name) != NULL )
				{
					jeErrorLog_AddString(JE_ERR_BAD_PARAMETER,"jeMotion_AddPath: Path already exists with same name.", Name);
					return JE_FALSE;
				}
		}

	PathCount = M->Leaf.PathCount;

	{
		jePath **NewPathArray;

		NewPathArray = (jePath**)jeRam_Realloc(M->Leaf.PathArray, (1+PathCount) * sizeof(jePath*) );

		if ( NewPathArray == NULL )
			{	
				jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeMotion_AddPath.");
				return JE_FALSE;
			}
		M->Leaf.PathArray = NewPathArray;
	}

	M->Leaf.PathArray[PathCount] = P;

	if ( M->MaintainNames == JE_TRUE )
		{
			
			assert (M->Leaf.NameArray != NULL);
			if (jeStrBlock_Append(&(M->Leaf.NameArray),Name)==JE_FALSE)
				{
					jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_AddPath.");
					assert(M->Leaf.PathArray[PathCount]);
					jePath_Destroy(&(M->Leaf.PathArray[PathCount]));
					return JE_FALSE;
				}
			M->Leaf.NameChecksum = jeStrBlock_GetChecksum(M->Leaf.NameArray);
		}						
															
	M->Leaf.PathCount = PathCount+1;
	*PathIndex = PathCount;
	jePath_CreateRef(P);
	return JE_TRUE;
}


// returns 0 if there is no name information... or if children don't all share the same checksum.
JETAPI int32 JETCC jeMotion_GetNameChecksum(const jeMotion *M)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				return 0;
			case (MOTION_NODE_BRANCH):
				{
					int i;
					int32 Checksum,FirstChecksum;
					if (M->Branch.MixerCount<1)
						return 0;
					assert( M->Branch.MixerArray[0].Motion );
					FirstChecksum = jeMotion_GetNameChecksum( M->Branch.MixerArray[0].Motion );
					
					for (i=1; i<M->Branch.MixerCount; i++)
						{
							assert( M->Branch.MixerArray[i].Motion );
							Checksum = jeMotion_GetNameChecksum( M->Branch.MixerArray[i].Motion );
							if (Checksum != FirstChecksum)
								return 0;
						}
					return FirstChecksum;
				}
			case (MOTION_NODE_LEAF):
				return M->Leaf.NameChecksum;
			default:
				assert(0);
		}
	return 0;
}	

JETAPI jeBoolean JETCC jeMotion_HasNames(const jeMotion *M)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	assert( (M->MaintainNames == JE_TRUE) || (M->MaintainNames == JE_FALSE) );
	// if M has names, all children of M have names. 
	return M->MaintainNames;
}

JETAPI jePath * JETCC jeMotion_GetPathNamed(const jeMotion *M,const char *Name)
{
	int i;

	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	
	if (M->NodeType != MOTION_NODE_LEAF)
		{	// not an error condition.
			return NULL;
		}
			
	assert( M->Leaf.PathCount >=0 );
	
	if (Name != NULL)	
		{
			if ( M->MaintainNames == JE_TRUE )
				{
					for (i=0; i<M->Leaf.PathCount; i++)
						{
							if ( strcmp(Name,jeStrBlock_GetString(M->Leaf.NameArray,i))==0 )
								{
									return M->Leaf.PathArray[i];
								}
						}
				}
		}
	return NULL;
}
			

#define LINEAR_BLEND(a,b,t)  ( (t)*((b)-(a)) + (a) )	
			// linear blend of a and b  0<t<1 where  t=0 ->a and t=1 ->b



JETAPI void JETCC jeMotion_Sample(const jeMotion *M, int PathIndex, jePath_TimeType Time, jeXForm3d *Transform)
{
	jeQuaternion Rotation;
	jeVec3d		 Translation;
	assert( M           != NULL);
	assert( jeMotion_IsValid(M) != JE_FALSE );
	assert( Transform   != NULL );

	jeMotion_SampleChannels(M,PathIndex,Time,&Rotation,&Translation);
	jeQuaternion_ToMatrix(&Rotation,Transform);
	Transform->Translation = Translation;
}


JETAPI void JETCC jeMotion_SampleChannels(const jeMotion *M, int PathIndex, jePath_TimeType Time, jeQuaternion *Rotation, jeVec3d *Translation)
{
	assert( M           != NULL);
	assert( Rotation    != NULL );
	assert( Translation != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				assert(0);
				break;
			case (MOTION_NODE_BRANCH):
				{
					jeQuaternion R;
					jeVec3d      T;
					jeMotion_Mixer *Mixer;
					int i;

					if ( M->Branch.MixerCount == 0 )
						{
							jeVec3d_Clear(Translation);
							jeQuaternion_SetNoRotation(Rotation);
							return;
						}

					assert( M->Branch.MixerCount > 0);
					Mixer = &(M->Branch.MixerArray[0]);
					
					assert(Mixer->Motion != NULL );
					jeMotion_SampleChannels(Mixer->Motion,PathIndex,
											(Time - Mixer->TimeOffset) * Mixer->TimeScale,
											Rotation,Translation);
				
					for (i=1; i<M->Branch.MixerCount; i++)
						{
							jeFloat BlendAmount;
							jeFloat MixTime;

							Mixer = &(M->Branch.MixerArray[i]);

							assert( Mixer->Motion != NULL );
							assert( Mixer->Blend  != NULL );

							MixTime = (Time - Mixer->TimeOffset) * Mixer->TimeScale;

							jeMotion_SampleChannels(Mixer->Motion,PathIndex,MixTime,&R,&T);
							{
								jeVec3d BlendVector;
								jeQuaternion Dummy;
								jePath_SampleChannels(Mixer->Blend,MixTime,&Dummy,&BlendVector);
								BlendAmount = MOTION_BLEND_PART_OF_VECTOR(BlendVector);
							}
							jeQuaternion_Slerp(Rotation,&R,BlendAmount,Rotation);
							Translation->X = LINEAR_BLEND(Translation->X,T.X,BlendAmount);
							Translation->Y = LINEAR_BLEND(Translation->Y,T.Y,BlendAmount);
							Translation->Z = LINEAR_BLEND(Translation->Z,T.Z,BlendAmount);
						}
				}
				break;
			case (MOTION_NODE_LEAF):
				{
					jePath *P;
					assert( ( PathIndex >=0 ) && ( PathIndex < M->Leaf.PathCount ) );
					P= M->Leaf.PathArray[PathIndex];
					assert( P != NULL );
					jePath_SampleChannels(P,Time,Rotation,Translation);
				}
				break;
			default:
				assert(0);
		}
}		

JETAPI jeBoolean JETCC jeMotion_SampleNamed(const jeMotion *M, const char *PathName, jePath_TimeType Time, jeXForm3d *Transform)
{
	jeQuaternion Rotation;
	jeVec3d		 Translation;
	assert( M           != NULL);
	assert( jeMotion_IsValid(M) != JE_FALSE );
	assert( Transform   != NULL );

	if (jeMotion_SampleChannelsNamed(M,PathName,Time,&Rotation,&Translation)==JE_FALSE)
		{
			return JE_FALSE;
		}

	jeQuaternion_ToMatrix(&Rotation,Transform);
	Transform->Translation = Translation;
	return JE_TRUE;
}



JETAPI jeBoolean JETCC jeMotion_SampleChannelsNamed(const jeMotion *M, const char *PathName, jePath_TimeType Time, jeQuaternion *Rotation, jeVec3d *Translation)
{
	jeBoolean AnyChannels=JE_FALSE;
	assert( M           != NULL);
	assert( Rotation    != NULL );
	assert( Translation != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				return JE_FALSE;
				break;
			case (MOTION_NODE_BRANCH):
				{
					int i;
					jeQuaternion R;
					jeVec3d T;
					jeMotion_Mixer *Mixer;

					if ( M->Branch.MixerCount == 0 )
						{
							jeVec3d_Clear(Translation);
							jeQuaternion_SetNoRotation(Rotation);
							return JE_TRUE;
						}

					assert( M->Branch.MixerCount > 0 );

					for (i=0; i<M->Branch.MixerCount; i++)
						{
							jeFloat BlendAmount;
							jeFloat MixTime;

							Mixer = &(M->Branch.MixerArray[i]);

							assert( Mixer->Motion != NULL );
							assert( Mixer->Blend  != NULL );

							MixTime = (Time - Mixer->TimeOffset) * Mixer->TimeScale;

							// hmm. is BlendAmount still good if there is no path?
							if ( jeMotion_SampleChannelsNamed(Mixer->Motion,PathName,MixTime,&R,&T)
								 != JE_FALSE )
								{
									if (AnyChannels != JE_FALSE)
										{
											{
												jeVec3d BlendVector;
												jeQuaternion Dummy;
												jePath_SampleChannels(Mixer->Blend,MixTime,&Dummy,&BlendVector);
												BlendAmount = MOTION_BLEND_PART_OF_VECTOR(BlendVector);
											}
											jeQuaternion_Slerp(Rotation,&R,BlendAmount,Rotation);
											Translation->X = LINEAR_BLEND(Translation->X,T.X,BlendAmount);
											Translation->Y = LINEAR_BLEND(Translation->Y,T.Y,BlendAmount);
											Translation->Z = LINEAR_BLEND(Translation->Z,T.Z,BlendAmount);
										}
									else
										{
											*Rotation = R;
											*Translation = T;
											AnyChannels = JE_TRUE;
										}
								}
						}
				}
				break;
			case (MOTION_NODE_LEAF):
				{
					jePath *P;
					P = jeMotion_GetPathNamed(M, PathName);
					if (P == NULL)
						{
							return JE_FALSE;
						}
					jePath_SampleChannels(P,Time,Rotation,Translation);
					AnyChannels = JE_TRUE;
				}
				break;
			default:
				assert(0);
		}
	return AnyChannels;
}		


JETAPI jePath * JETCC jeMotion_GetPath(const jeMotion *M,int Index)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
 	
	if (M->NodeType != MOTION_NODE_LEAF)
		{	// not an error condition.
			return NULL;
		}
			
	assert( M->Leaf.PathCount >=0 );
	assert( Index <= M->Leaf.PathCount );
	assert( Index >= 0 );

	return M->Leaf.PathArray[Index];
}

JETAPI const char * JETCC jeMotion_GetNameOfPath(const jeMotion *M, int Index)
{
	jePath *P;
	assert( M != NULL );

	if (M->NodeType!=MOTION_NODE_LEAF)
		{
			return NULL;
		}
	if (jeMotion_HasNames(M)==JE_FALSE)
		{
			return NULL;
		}

	P = jeMotion_GetPath(M,Index);
	if (P==NULL)
		{
			return NULL;
		}
	assert( M->Leaf.NameArray!=NULL );

	return jeStrBlock_GetString(M->Leaf.NameArray,Index);

}
			
	

JETAPI int JETCC jeMotion_GetPathCount(const jeMotion *M)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	if (M->NodeType != MOTION_NODE_LEAF)
		{	// not an error condition.
			return 0;
		}
	assert( M->Leaf.PathCount >=0 );
	return M->Leaf.PathCount;
}


JETAPI jeBoolean JETCC jeMotion_GetTimeExtents(const jeMotion *M,jePath_TimeType *StartTime,jePath_TimeType *EndTime)
{
	int i,found;
	jePath_TimeType Start,End;
	assert( M != NULL );
	assert( StartTime != NULL );
	assert( EndTime != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	found = 0;

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				break;
			case (MOTION_NODE_BRANCH):
				for (i=0; i<M->Branch.MixerCount; i++)
					{
						if (jeMotion_GetTimeExtents(M->Branch.MixerArray[i].Motion,&Start,&End)!=JE_FALSE)
							{
								found++;

								// Assertions in AddSubMotion and SetTimeScale prevent TimeScale from being 0.
								End = M->Branch.MixerArray[i].TimeOffset + ((End - Start) / M->Branch.MixerArray[i].TimeScale);
								Start += M->Branch.MixerArray[i].TimeOffset;

								// If time scale is negative, then End will be < Start, which violates
								// the entire idea of extents.  So we'll swap them.
								if (End < Start)
								{
									jeFloat Temp = Start;
									Start = End;
									End = Temp;
								}
								if (found==1)
									{
										*StartTime = Start;
										*EndTime   = End;
									}
								else
									{	//found>1
										*StartTime = MIN(*StartTime,Start);
										*EndTime   = MAX(*EndTime,End);
									}
							}
					}										
				break;			
			case (MOTION_NODE_LEAF):
				found = 0;
				for (i=0; i<M->Leaf.PathCount; i++)
					{
						if (jePath_GetTimeExtents(M->Leaf.PathArray[i],&Start,&End)!=JE_FALSE)
							{
								found++;
								if (found==1)
									{
										*StartTime = Start;
										*EndTime   = End;
									}
								else
									{	//found>1
										*StartTime = MIN(*StartTime,Start);
										*EndTime   = MAX(*EndTime,End);
									}
							}
					}
				break;
			default:
				assert(0);
		}
	if (found>0)
		{
			return JE_TRUE;
		}
	return JE_FALSE;
}			


JETAPI int JETCC jeMotion_GetSubMotionCount(const jeMotion *M)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			return M->Branch.MixerCount;
		}
	return 0;
}


#pragma message ("do we want to copy these before returning them?")
JETAPI jeMotion * JETCC jeMotion_GetSubMotion(const jeMotion *M,int SubMotionIndex)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	if (M->NodeType != MOTION_NODE_BRANCH )
		{
			return NULL;
		}
	assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
	assert( M->Branch.MixerArray != NULL );

	return M->Branch.MixerArray[SubMotionIndex].Motion;
}

JETAPI jeMotion * JETCC jeMotion_GetSubMotionNamed(const jeMotion *M,const char *Name)
{
	int i;
	assert( M != NULL);	
	assert( Name != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	if (M->NodeType != MOTION_NODE_BRANCH)
		{
			return NULL;
		}
	assert( M->Branch.MixerArray != NULL );
	for (i=0; i<M->Branch.MixerCount; i++)
		{
			jeMotion *MI = M->Branch.MixerArray[i].Motion;
			assert( MI != NULL );
			if (MI->Name!=NULL)
				{
					if (strcmp(MI->Name,Name)==0)
						{
							return MI;
						}
				}
		}
	return NULL;
}

static jeBoolean JETCF jeMotion_SearchForSubMotion(const jeMotion *Parent, const jeMotion*Child)
{
	int i;
	assert( Parent != NULL );
	assert( Child  != NULL );
	assert( jeMotion_IsValid(Parent) != JE_FALSE );
	assert( jeMotion_IsValid(Child) != JE_FALSE );

	if (Parent == Child)
		return JE_TRUE;

	if (Parent->NodeType != MOTION_NODE_BRANCH)
		return JE_FALSE;

	assert( Parent->Branch.MixerArray != NULL );

	for (i=0; i<Parent->Branch.MixerCount; i++)
		{
			assert( Parent->Branch.MixerArray[i].Motion != NULL );
			if (jeMotion_SearchForSubMotion(Parent->Branch.MixerArray[i].Motion,Child)==JE_TRUE)
				return JE_TRUE;
		}
	return JE_FALSE;
}

JETAPI jeBoolean JETCC jeMotion_AddSubMotion(jeMotion *ParentMotion, 
								jeFloat TimeScale, 
								jeFloat TimeOffset,
								jeMotion *SubMotion, 
								jeFloat StartTime, jeFloat StartMagnitude,
								jeFloat EndTime,   jeFloat EndMagnitude,
								const jeXForm3d *Transform,
								int *Index)

{

	int Count;
	jeMotion_Mixer *NewMixerArray;
	assert( ParentMotion != NULL );
	assert( TimeScale	 != 0.0f );
	assert( SubMotion    != NULL );
	assert( Index        != NULL );
	//assert( Transform    != NULL );
	assert( ( StartMagnitude >= 0.0f) && ( StartMagnitude <=1.0f ));
	assert( ( EndMagnitude   >= 0.0f) && ( EndMagnitude   <=1.0f ));
	assert( jeMotion_IsValid(ParentMotion) != JE_FALSE );
	assert( jeMotion_IsValid(SubMotion) != JE_FALSE );

	switch (ParentMotion->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				if (jeMotion_InitNodeAsBranch(ParentMotion)==JE_FALSE)
					{
						jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_AddSubMotion.");
						return JE_FALSE;
					}
				break;
			case (MOTION_NODE_LEAF):
				{
					jeErrorLog_Add(JE_ERR_BAD_PARAMETER, "jeMotion_AddSubMotion: Can't add a submotion to a Leaf motion.");
					return JE_FALSE;
				}
			case (MOTION_NODE_BRANCH):
				break;
			default:
				assert(0);
		}

	if (ParentMotion->MaintainNames != SubMotion->MaintainNames)
		{
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER, "jeMotion_AddSubMotion: Can't add a submotion with different MaintainNames convention.");  
			return JE_FALSE;
		}
		
	if (jeMotion_SearchForSubMotion(SubMotion,ParentMotion)!=JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER, "jeMotion_AddSubMotion: Can't add - would create a circular loop of submotions.");
			return JE_FALSE;
		}
			
	Count = ParentMotion->Branch.MixerCount;
	NewMixerArray = (jeMotion_Mixer *)jeRam_Realloc(ParentMotion->Branch.MixerArray, (1+Count) * sizeof(jeMotion_Mixer) );
	if ( NewMixerArray == NULL )
		{	
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeMotion_AddSubMotion.");
			return JE_FALSE;
		}
		
	ParentMotion->Branch.MixerArray = NewMixerArray;
	{
		jeMotion_Mixer *Mixer;
		jeXForm3d BlendKeyTransform;
		Mixer = &(ParentMotion->Branch.MixerArray[Count]);
	
		Mixer->Motion     = SubMotion;
		Mixer->TimeScale  = TimeScale;
		Mixer->TimeOffset = TimeOffset;
		
		Mixer->Blend = jePath_Create(JE_PATH_INTERPOLATE_HERMITE_ZERO_DERIV,JE_PATH_INTERPOLATE_SLERP,JE_FALSE);
		if (Mixer->Blend==NULL)
			{	
				jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_AddSubMotion.");
				return JE_FALSE;
			}
		MOTION_BLEND_PART_OF_TRANSFORM(BlendKeyTransform) = StartMagnitude;
		if (jePath_InsertKeyframe(Mixer->Blend,
						JE_PATH_TRANSLATION_CHANNEL,StartTime,&BlendKeyTransform)==JE_FALSE)
			{
				jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_AddSubMotion.");
				jePath_Destroy(&(Mixer->Blend));
				return JE_FALSE;
			}

		MOTION_BLEND_PART_OF_TRANSFORM(BlendKeyTransform) = EndMagnitude;
		if (jePath_InsertKeyframe(Mixer->Blend,
						JE_PATH_TRANSLATION_CHANNEL,EndTime,&BlendKeyTransform)==JE_FALSE)
			{
				jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_AddSubMotion.");
				jePath_Destroy(&(Mixer->Blend));
				return JE_FALSE;
			}
		if (Transform == NULL)
			{
				Mixer->TransformUsed = JE_FALSE;
			}
		else
			{
				Mixer->TransformUsed = JE_TRUE;
				Mixer->Transform = *Transform;
			}
	}
	
	*Index = Count;
	SubMotion->CloneCount++;
	ParentMotion->Branch.MixerCount++;

	return JE_TRUE;
}

JETAPI jeMotion * JETCC jeMotion_RemoveSubMotion(jeMotion *ParentMotion, int SubMotionIndex)
{
	int Count;
	jeMotion *M;
	assert( ParentMotion != NULL );
	assert( jeMotion_IsValid(ParentMotion) != JE_FALSE );

	if (ParentMotion->NodeType != MOTION_NODE_BRANCH)
		{
			return NULL;
		}
	
	Count = ParentMotion->Branch.MixerCount;
	assert( (SubMotionIndex>=0) && (SubMotionIndex<Count));
	
	M = ParentMotion->Branch.MixerArray[SubMotionIndex].Motion;
	assert( ParentMotion->Branch.MixerArray[SubMotionIndex].Blend != NULL );
	jePath_Destroy( &(ParentMotion->Branch.MixerArray[SubMotionIndex].Blend) );
	
	if (Count>1)
		{
			memcpy( &(ParentMotion->Branch.MixerArray[SubMotionIndex]),
					&(ParentMotion->Branch.MixerArray[SubMotionIndex+1]),
					sizeof(jeMotion_Mixer) * (Count-(SubMotionIndex+1)));
		}
	ParentMotion->Branch.MixerCount--;

	{
		jeMotion_Mixer *NewMixerArray;
		if (ParentMotion->Branch.MixerCount == 0)
			{
				jeRam_Free(ParentMotion->Branch.MixerArray);
				ParentMotion->Branch.MixerArray = NULL;
			}
		else
			{
				NewMixerArray = (jeMotion_Mixer *)jeRam_Realloc(ParentMotion->Branch.MixerArray, 
									(ParentMotion->Branch.MixerCount) * sizeof(jeMotion_Mixer) );
				if ( NewMixerArray != NULL )
					{	
						ParentMotion->Branch.MixerArray = NewMixerArray;
					}
			}
	}
	jeMotion_Destroy( &M );
	return M;
}
	


JETAPI jeFloat   JETCC jeMotion_GetTimeOffset( const jeMotion *M,int SubMotionIndex )
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	// wrong node type is neither error nor invalid.  return value is just 0

	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
			return M->Branch.MixerArray[SubMotionIndex].TimeOffset;
		}
	return 0.0f;
}

JETAPI jeBoolean  JETCC jeMotion_SetTimeOffset( jeMotion *M,int SubMotionIndex,jeFloat TimeOffset )
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
			M->Branch.MixerArray[SubMotionIndex].TimeOffset = TimeOffset;
			return JE_TRUE;
		}
	return JE_FALSE;
}

JETAPI jeFloat   JETCC jeMotion_GetTimeScale( const jeMotion *M,int SubMotionIndex )
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	// wrong node type is neither error nor invalid.  return value is just 1

	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
			return M->Branch.MixerArray[SubMotionIndex].TimeScale;
		}
	return 1.0f;
}

JETAPI jeBoolean  JETCC jeMotion_SetTimeScale( jeMotion *M,int SubMotionIndex,jeFloat TimeScale )
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	assert( TimeScale != 0.0f);

	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
			M->Branch.MixerArray[SubMotionIndex].TimeScale = TimeScale;
			return JE_TRUE;
		}
	return JE_FALSE;
}

JETAPI jeFloat    JETCC jeMotion_GetBlendAmount( const jeMotion *M, int SubMotionIndex, jeFloat Time)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	// wrong node type is neither error nor invalid.  return value is just 0

	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			jeQuaternion Dummy;
			jeVec3d BlendVector;
			jeFloat BlendAmount;

			assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
			assert( M->Branch.MixerArray[SubMotionIndex].Blend != NULL );
			jePath_SampleChannels(M->Branch.MixerArray[SubMotionIndex].Blend,
								  ( Time - M->Branch.MixerArray[SubMotionIndex].TimeOffset )
								    * M->Branch.MixerArray[SubMotionIndex].TimeScale,
								   &Dummy,&BlendVector);
			BlendAmount = MOTION_BLEND_PART_OF_VECTOR(BlendVector);
			return BlendAmount;
		}
	return 0.0f;
}

JETAPI jePath    * JETCC jeMotion_GetBlendPath( const jeMotion *M,int SubMotionIndex )
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	// wrong node type is neither error nor invalid.  return value is just NULL

	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
			return M->Branch.MixerArray[SubMotionIndex].Blend;
		}
	return NULL;
}

JETAPI jeBoolean  JETCC jeMotion_SetBlendPath( jeMotion *M,int SubMotionIndex, jePath *Blend )
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			jePath *P;
			assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
			assert( Blend != NULL );
			P = M->Branch.MixerArray[SubMotionIndex].Blend;
			jePath_Destroy(&P);
			P = jePath_CreateCopy(Blend);
			if ( P == NULL )
				{
					jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_SetBlendPath.");
					return JE_FALSE;
				}
			M->Branch.MixerArray[SubMotionIndex].Blend = P;
			return JE_TRUE;
		}
	return JE_FALSE;
}


JETAPI const jeXForm3d * JETCC jeMotion_GetBaseTransform( const jeMotion *M,int SubMotionIndex )
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	// wrong node type is neither error nor invalid.  return value is just NULL

	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
			if (M->Branch.MixerArray[SubMotionIndex].TransformUsed != JE_FALSE)
				{
					return &(M->Branch.MixerArray[SubMotionIndex].Transform);
				}
			else
				{
					return NULL;
				}
		}
	return NULL;
}

JETAPI jeBoolean  JETCC jeMotion_SetBaseTransform( jeMotion *M,int SubMotionIndex, jeXForm3d *BaseTransform )
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );

	if (M->NodeType == MOTION_NODE_BRANCH)
		{
			assert( (SubMotionIndex>=0) && (SubMotionIndex<M->Branch.MixerCount));
			assert( BaseTransform != NULL );
			if (BaseTransform!=NULL)
				{
					M->Branch.MixerArray[SubMotionIndex].Transform     = *BaseTransform;
					M->Branch.MixerArray[SubMotionIndex].TransformUsed = JE_TRUE;
				}
			else
				{
					M->Branch.MixerArray[SubMotionIndex].TransformUsed = JE_FALSE;
				}
					
			return JE_TRUE;
		}
	return JE_FALSE;
}


#pragma warning( disable : 4701)	// don't want to set Translation until we are ready
JETAPI jeBoolean JETCC jeMotion_GetTransform( const jeMotion *M, jeFloat Time, jeXForm3d *Transform)
{
	assert( M         != NULL);
	assert( jeMotion_IsValid(M) != JE_FALSE );

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				{
					return JE_FALSE;
				}
				break;
			case (MOTION_NODE_BRANCH):
				{
					jeQuaternion R,Rotation;
					jeVec3d      T,Translation;
					jeMotion_Mixer *Mixer;
					jeFloat MixTime;
					int i;
					int MixCount=0;

					if ( M->Branch.MixerCount == 0 )
						{
							return JE_FALSE;
						}
					assert( M->Branch.MixerCount > 0 );

					for (i=0; i<M->Branch.MixerCount; i++)
						{
							jeFloat BlendAmount;
							jeBoolean DoMix=JE_FALSE;

							Mixer = &(M->Branch.MixerArray[i]);

							assert( Mixer->Motion != NULL );
							assert( Mixer->Blend  != NULL );
							
							MixTime = (Time - Mixer->TimeOffset) * Mixer->TimeScale;
							if (jeMotion_GetTransform(Mixer->Motion,MixTime,Transform)!=JE_FALSE)
								{
									DoMix=JE_TRUE;
									if (Mixer->TransformUsed!=JE_FALSE)
										{
											jeXForm3d_Multiply(&(Mixer->Transform),Transform,Transform);
										}
								}
							else
								{
									if (Mixer->TransformUsed!=JE_FALSE)
										{
											DoMix = JE_TRUE;
											*Transform = Mixer->Transform;
										}
								}
							if (DoMix!=JE_FALSE)
								{
									if (MixCount==0)
										{
											jeQuaternion_FromMatrix(Transform,&Rotation);
											Translation = Transform->Translation;
										}
									else
										{
											jeQuaternion_FromMatrix(Transform,&R);
											T = Transform->Translation;
											{
												jeVec3d BlendVector;
												jeQuaternion Dummy;
												jePath_SampleChannels(Mixer->Blend,MixTime,&Dummy,&BlendVector);
												BlendAmount = MOTION_BLEND_PART_OF_VECTOR(BlendVector);
											}
											jeQuaternion_Slerp(&Rotation,&R,BlendAmount,&Rotation);
											Translation.X = LINEAR_BLEND(Translation.X,T.X,BlendAmount);
											Translation.Y = LINEAR_BLEND(Translation.Y,T.Y,BlendAmount);
											Translation.Z = LINEAR_BLEND(Translation.Z,T.Z,BlendAmount);
										}
									
									MixCount++;
								}
						}
					if (MixCount>0)
						{
							jeQuaternion_ToMatrix(&Rotation,Transform);
							Transform->Translation = Translation;
							return JE_TRUE;
						}
					return JE_FALSE;
				}
				break;
			case (MOTION_NODE_LEAF):
				{
					return JE_FALSE;
				}
				break;
			default:
				assert(0);
		}
	return JE_FALSE;
}
#pragma warning( default : 4701)	


//--------------------------------------------------------------------------------------------
//   Event Support

JETAPI jeBoolean JETCC jeMotion_GetEventExtents(const jeMotion *M,jeFloat *FirstEventTime,jeFloat *LastEventTime)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	assert( FirstEventTime != NULL );
	assert( LastEventTime != NULL );

	return jeTKEvents_GetExtents(M->Leaf.Events,FirstEventTime,LastEventTime);
}	


	// Inserts the new event and corresponding string.
JETAPI jeBoolean JETCC jeMotion_InsertEvent(jeMotion *M, jePath_TimeType tKey, const char* String)
{
	assert( M != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );
	assert( String != NULL );

	if (M->NodeType != MOTION_NODE_LEAF )
		{
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER, "jeMotion_InsertEvent: Motion not a leaf - Can only insert events in a leaf.");
			return JE_FALSE;
		}

	if (M->Leaf.Events == NULL)
		{
			M->Leaf.Events = jeTKEvents_Create();
			if ( M->Leaf.Events == NULL )
				{
					jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_InsertEvent.");
					return JE_FALSE;
				}
		}
	if (jeTKEvents_Insert(M->Leaf.Events, tKey,String)==JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_InsertEvent.");
			return JE_FALSE;
		};
	return JE_TRUE;
}
	

			
	// Deletes the event
JETAPI jeBoolean JETCC jeMotion_DeleteEvent(jeMotion *M, jePath_TimeType tKey)
{
	assert( M != NULL);
	assert( jeMotion_IsValid(M) != JE_FALSE );

	if (M->NodeType != MOTION_NODE_LEAF )
		{
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER, "jeMotion_DeleteEvent: Motion not a leaf - Can only delete events in a leaf.");
			return JE_FALSE;
		}
	if ( M->Leaf.Events == NULL )
		{
			jeErrorLog_Add(JE_ERR_SEARCH_FAILURE, "jeMotion_DeleteEvent: no events in motion.");
			return JE_FALSE;
		}
	if (jeTKEvents_Delete(M->Leaf.Events,tKey)==JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_DeleteEvent.");
			return JE_FALSE;
		}
	return JE_TRUE;
}

JETAPI void JETCC jeMotion_SetupEventIterator(
	jeMotion *M,
	jePath_TimeType StartTime,				// Inclusive search start
	jePath_TimeType EndTime)				// Non-inclusive search stop
	// For searching or querying the array for events between two times
	// times are compaired [StartTime,EndTime), '[' is inclusive, ')' is 
	// non-inclusive.  This prepares the jeMotion_GetNextEvent() function.
{
	int i;
	assert( M != NULL);
	assert( jeMotion_IsValid(M) != JE_FALSE );

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				break;
			case (MOTION_NODE_LEAF):
				if ( M->Leaf.Events != NULL )
					{
						jeTKEvents_SetupIterator(M->Leaf.Events,StartTime,EndTime);
					}	
				break;
			case (MOTION_NODE_BRANCH):
				for (i=0; i<M->Branch.MixerCount; i++)
					{
						jeMotion_Mixer *Mixer;
				
						Mixer = &(M->Branch.MixerArray[i]);

						jeMotion_SetupEventIterator(Mixer->Motion,
							(StartTime - Mixer->TimeOffset) * Mixer->TimeScale,
							(EndTime - Mixer->TimeOffset) * Mixer->TimeScale);
					}
				M->Branch.CurrentEventIterator =0;
				break;
			default:
				assert(0);
		}
}		


JETAPI jeBoolean JETCC jeMotion_GetNextEvent(
	jeMotion *M,						// Event list to iterate
	jePath_TimeType *pTime,				// Return time, if found
	const char **ppEventString)		// Return data, if found
	// Iterates from StartTime to EndTime as setup in jeMotion_SetupEventIterator()
	// and for each event between these times [StartTime,EndTime)
	// this function will return Time and EventString returned for that event
	// and the iterator will be positioned for the next search.  When there 
	// are no more events in the range, this function will return JE_FALSE (Time
	// will be 0 and ppEventString will be empty).
{
	assert( M != NULL);
	assert( jeMotion_IsValid(M) != JE_FALSE );

	assert( pTime != NULL );
	assert( ppEventString != NULL );

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				{
					return JE_FALSE;
				}
				break;
			case (MOTION_NODE_LEAF):
				if ( M->Leaf.Events != NULL )
					{
						return jeTKEvents_GetNextEvent(M->Leaf.Events,pTime,ppEventString);
					}	
				break;
			case (MOTION_NODE_BRANCH):
				while (M->Branch.CurrentEventIterator < M->Branch.MixerCount)
					{
						if (jeMotion_GetNextEvent(
									M->Branch.MixerArray[M->Branch.CurrentEventIterator].Motion,
									pTime,ppEventString) !=JE_FALSE)
							return JE_TRUE;
						M->Branch.CurrentEventIterator++;
					}
				break;
			default:
				assert(0);
		}

	return JE_FALSE;
}
	
//------------------------------------------------------------------------------------------------------
//    Read/Write support

#define MOTION_BIN_FILE_TYPE 0x424E544D 	// 'MTNB'
#define MOTION_FILE_VERSION 0x00F0			// Restrict version to 16 bits

typedef struct
{
	int PathCount;
	int32 NameChecksum;
	uint32 Flags;
} jeMotion_FileLeafHeader;

static jeBoolean JETCF jeMotion_ReadBranch(jeMotion *M, jeVFile *pFile)
{
	assert( M != NULL );
	assert( pFile != NULL );
	if (jeMotion_InitNodeAsBranch(M)==JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeMotion_ReadBranch.");
			return JE_FALSE;
		}

	#pragma message("finish this")
	jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "jeMotion_ReadBranch: not implemented.");
	return JE_FALSE;
}

static jeBoolean JETCF jeMotion_ReadLeaf(jeMotion *M, jeVFile *pFile)
{
	int i;
	jeMotion_FileLeafHeader Header;
	assert( M != NULL );
	assert( pFile != NULL );
	if (jeMotion_InitNodeAsLeaf(M,JE_FALSE)==JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE,"jeMotion_ReadLeaf.");
			return JE_FALSE;
		}

	if (jeVFile_Read(pFile, &Header, sizeof(jeMotion_FileLeafHeader)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ,"jeMotion_ReadLeaf: failed to read leaf header."); 
			return JE_FALSE; 
		}
	M->Leaf.NameChecksum = Header.NameChecksum;
	
	if (Header.Flags & 0x1)
		{
			M->Leaf.Events = jeTKEvents_CreateFromFile(pFile);
			if (M->Leaf.Events == NULL )
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE,"jeMotion_ReadLeaf.");
					return JE_FALSE; 
				}
		}
	else
		{
			M->Leaf.Events = NULL;
		}

	if (Header.Flags & 0x2)
		{
			M->Leaf.NameArray = jeStrBlock_CreateFromFile(pFile);
			if (M->Leaf.NameArray == NULL)
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE,"jeMotion_ReadLeaf.");
					return JE_FALSE; 
				}
		}
	else
		{
			M->Leaf.NameArray = NULL;
		}

	M->Leaf.PathCount = 0;
	M->Leaf.PathArray = (jePath **)jeRam_Allocate( Header.PathCount * sizeof(jePath*) );

	if ( M->Leaf.PathArray == NULL )
		{	
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE,"jeMotion_ReadLeaf.");
			return JE_TRUE;
		}

	for (i=0; i<Header.PathCount; i++)
		{
			M->Leaf.PathArray[i] = jePath_CreateFromFile(pFile);
			if (M->Leaf.PathArray[i] == NULL )
				{
					jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE,"jeMotion_ReadLeaf: failed to read path.",jeErrorLog_IntToString(i));
					return JE_FALSE; 
				}
			M->Leaf.PathCount++;
		}
	return JE_TRUE;
}

JETAPI jeMotion* JETCC jeMotion_CreateFromFile(jeVFile* pFile)
{
	uint32 u;	
	jeBoolean MaintainNames;
	int NodeType;
	int NameLength;
	jeMotion *M;

	assert( pFile != NULL );

	if(jeVFile_Read(pFile, &u, sizeof(u)) == JE_FALSE)
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeMotion_CreateFromFile: failed to read motion header.");
		return NULL;
	}

	if(u != MOTION_BIN_FILE_TYPE)
	{
		jeErrorLog_Add( JE_ERR_FILEIO_FORMAT , "jeMotion_CreateFromFile: wrong file type - not a motion.");
		return NULL;
	}
	
	if(jeVFile_Read(pFile, &u, sizeof(u)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeMotion_CreateFromFile: failed to read version number.");
			return NULL;
		}
	if (u!=MOTION_FILE_VERSION)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_VERSION , "jeMotion_CreateFromFile: bad or old version number.");
			return NULL;
		}
	if(jeVFile_Read(pFile, &u, sizeof(u)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeMotion_CreateFromFile: failed to read motion flags.");
			return NULL;
		}

	if (u & (1<<16)) 
		{
			MaintainNames = JE_TRUE;
		}
	else
		{
			MaintainNames = JE_FALSE;
		}

	NameLength = (u & 0xFFFF);
	NodeType   = (u >> 24);
	M = jeMotion_Create(MaintainNames);
	if ( M == NULL )
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_CreateFromFile.");
			return NULL;
		}
	if (NameLength>0)
		{
			M->Name = (char *)jeRam_Allocate(NameLength);
			if ( M->Name == NULL )
				{
					jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeMotion_CreateFromFile.");
					jeMotion_Destroy(&M);
					return NULL;
				}
 			if ( jeVFile_Read (pFile, M->Name, NameLength ) == JE_FALSE )
				{
					jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeMotion_CreateFromFile: failed to read motion name.");
					jeMotion_Destroy(&M);
					return NULL;
				}
		}
	else
		{
			M->Name = NULL;
		}
	switch (NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				break;
			case (MOTION_NODE_BRANCH):
				if (jeMotion_ReadBranch(M,pFile)==JE_FALSE)
					{
						jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_CreateFromFile.");
						jeMotion_Destroy(&M);
						return NULL;
					}
				break;
			case (MOTION_NODE_LEAF):
				if (jeMotion_ReadLeaf(M,pFile)==JE_FALSE)
					{
						jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_CreateFromFile.");
						jeMotion_Destroy(&M);
						return NULL;
					}
				break;
			default:
				assert(0);
				break;
		}
	return M;
}


static jeBoolean JETCF jeMotion_WriteLeaf(const jeMotion *M, jeVFile *pFile)
{
	int i;
	jeMotion_FileLeafHeader Header;

	#define MOTION_LEAF_EVENTS_FLAG    (1)
	#define MOTION_LEAF_NAMEARRAY_FLAG (2)

	assert( M != NULL );
	assert( pFile != NULL );
	assert( M->NodeType == MOTION_NODE_LEAF);
	assert( jeMotion_IsValid(M) != JE_FALSE );

	Header.PathCount = M->Leaf.PathCount;
	Header.NameChecksum = M->Leaf.NameChecksum;
	Header.Flags = 0;

	if (M->Leaf.Events != NULL)
		{
			Header.Flags |= MOTION_LEAF_EVENTS_FLAG;
		}

	if (M->Leaf.NameArray != NULL)
		{
			Header.Flags |= MOTION_LEAF_NAMEARRAY_FLAG;
		}
		
		
	if (jeVFile_Write(pFile, &Header, sizeof(jeMotion_FileLeafHeader)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "jeMotion_WriteLeaf: failed to write leaf header."); 
			return JE_FALSE; 
		}
			

	if (Header.Flags & MOTION_LEAF_EVENTS_FLAG)
		{
			if (jeTKEvents_WriteToFile(M->Leaf.Events,pFile)==JE_FALSE)
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_WriteLeaf."); 
					return JE_FALSE; 
				}
		}

	
	if (Header.Flags & MOTION_LEAF_NAMEARRAY_FLAG)
		{
			if (jeStrBlock_WriteToFile(M->Leaf.NameArray,pFile)==JE_FALSE)
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_WriteLeaf."); 
					return JE_FALSE; 
				}
		}

	for (i=0; i<M->Leaf.PathCount; i++)
		{
			if (jePath_WriteToFile(M->Leaf.PathArray[i],pFile) == JE_FALSE)
				{
					jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_WriteLeaf: failed to write path", jeErrorLog_IntToString(i)); 
					return JE_FALSE; 
				}
		}
	return JE_TRUE;
}

static jeBoolean JETCF jeMotion_WriteBranch(const jeMotion *M, jeVFile *pFile)
{
	assert( M != NULL );
	assert( pFile != NULL );
	assert( M->NodeType == MOTION_NODE_BRANCH);
	assert( jeMotion_IsValid(M) != JE_FALSE );
	#pragma message("finish this")
	jeErrorLog_Add(JE_ERR_INTERNAL_RESOURCE,"jeMotion_WriteBranch: saving of compound motions not implemented.");
	return JE_FALSE;
}


JETAPI jeBoolean JETCC jeMotion_WriteToFile(const jeMotion *M,jeVFile *pFile)
{
	uint32 u;

	assert( M != NULL );
	assert( pFile != NULL );
	assert( jeMotion_IsValid(M) != JE_FALSE );


	// Write the format flag
	u = MOTION_BIN_FILE_TYPE;
	if(jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "jeMotion_WriteToFile: failed to write motion header.");
			return JE_FALSE;
		}

	u = MOTION_FILE_VERSION;
	if (jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "jeMotion_WriteToFile: failed to write motion version number.");
			return JE_FALSE;
		}
	if ( M->Name != NULL )
		{
			u = strlen(M->Name)+1;
		}
	else
		{
			u = 0;
		}
	assert( u < 0xFFFF );
	
	if (M->MaintainNames != JE_FALSE)
		{
			u |= (1<<16);
		}
	assert( M->NodeType < 0xFF );
	u |= (M->NodeType << 24);
	if (jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "jeMotion_WriteToFile: failed to write motion flags.");
			return JE_FALSE;
		}
	if ((u&0xFFFF) > 0)
		{
			if (jeVFile_Write(pFile, M->Name, (u&0xFFFF)) == JE_FALSE)
				{
					jeErrorLog_AddString( JE_ERR_FILEIO_WRITE, "jeMotion_WriteToFile: failed to write motion name", M->Name);
					return JE_FALSE;
				}
		}

	switch (M->NodeType)
		{
			case (MOTION_NODE_UNDECIDED):
				break;
			case (MOTION_NODE_BRANCH):
				if (jeMotion_WriteBranch(M,pFile)==JE_FALSE)
					{
						jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_WriteToFile.");
						return JE_FALSE;
					}
				break;
			case (MOTION_NODE_LEAF):
				if (jeMotion_WriteLeaf(M,pFile)==JE_FALSE)
					{
						jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeMotion_WriteToFile.");
						return JE_FALSE;
					}
				break;
			default:
				assert(0);
				break;
		}
	return JE_TRUE;
}
