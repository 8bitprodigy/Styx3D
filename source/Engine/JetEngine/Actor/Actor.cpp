/****************************************************************************************/
/*  ACTOR.C                                                                             */
/*                                                                                      */
/*  Authors: Mike Sandige	                                                            */
/*				  Aaron Oneal (Incarnadine) - aoneal@ij.net                             */
/*  Description:  Actor implementation                                                  */
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

	TODO:
	  make cued motions keyed to a 'root' bone.  Register the root bone, and then 
	  all requests are relative to that bone, rather than the current 'anchor' point.
	  actually, this doesn't really change much, just _AnimationCue() - and it allows
	  a more efficient _TestStep()

	  convert BoundingBoxMinCorner, BoundingBoxMaxCorner to use extbox.
	
*/

#include <assert.h>
#include <string.h>  // _stricmp()
#include <malloc.h> // free
#include <math.h>

#include "Actor.h"
#include "Ram.h"
#include "Puppet.h"
#include "Body.h"
#include "Motion.h"
#include "Errorlog.h"
#include "StrBlock.h"
#include "Log.h"

#include "Actor._h"

#ifdef WIN32
#pragma warning ( disable : 4115 )
#include <windows.h>
#pragma warning ( default : 4115 )
#endif

#ifdef BUILD_BE
#define _stricmp strcasecmp
#endif

#define ACTOR_MOTIONS_MAX 0x0FFFF		// really arbitrary. just for sanity checking
#define ACTOR_CUES_MAX    0x0FFFF		// arbitrary. 

/*
typedef struct ActorObj {
		char *ActorDefName, *MotionName;
		jeBoolean CollisionExtBoxDisplay;
		jeExtBox		CollisionExtBox;		
		jeBoolean RenderExtBoxDisplay;		
		jeExtBox			RenderHintExtBox;
		char				**MotionList;
		int				MotionListSize;
		jeMotion		*Motion;
		float MotionTime;
		float ScaleX,ScaleY,ScaleZ;
		float			MotionTimeScale;
		jeWorld *World;
		jeEngine *Engine;
		jeResourceMgr *ResourceMgr;
} ActorObj;*/

	// these are useful globals to monitor resources
int jeActor_Count       = 0;
int jeActor_RefCount    = 0;
int jeActor_DefCount    = 0;
int jeActor_DefRefCount = 0;

//	[MacroArt::Begin]
//	Thanks Dee(cryscan@home.net)	
JETAPI float JETCC jeActor_GetAlpha(const jeActor *A)
{
	assert( A != NULL ) ;
	assert( A->Puppet != NULL ) ;
	return jePuppet_GetAlpha(A->Puppet);
}

JETAPI void JETCC jeActor_SetAlpha(jeActor *A, float Alpha)
{
	assert( A != NULL ) ;
	assert( A->Puppet != NULL ) ;
	jePuppet_SetAlpha( A->Puppet, Alpha ) ;
}
//	[MacroArt::End]

	// returns number of actors that are currently created.
JETAPI int32 JETCC jeActor_GetCount(void)
{
	return jeActor_Count;
}

JETAPI jeBoolean JETCC jeActor_IsValid(const jeActor *A)
{
	if (A==NULL)
		return JE_FALSE;

	if(A->ActorDefinition)
	{
		if (A->Pose == NULL)
			return JE_FALSE;
		if (A->CueMotion == NULL)
			return JE_FALSE;
		if (jeActor_DefIsValid(A->ActorDefinition)==JE_FALSE)
			return JE_FALSE;
		if (jeBody_IsValid(A->ActorDefinition->Body) == JE_FALSE )
			return JE_FALSE;
	}
	return JE_TRUE;
}
	
JETAPI jeBoolean JETCC jeActor_DefIsValid(const jeActor_Def *A)
{
	if (A==NULL)
		return JE_FALSE;
	if (A->ValidityCheck != A)
		return JE_FALSE;	

	return JE_TRUE;
}

static jeBoolean JETCF jeActor_GetBoneIndex(const jeActor *A, const char *BoneName, int *BoneIndex)
{
	jeXForm3d Dummy;
	int ParentBoneIndex;
	assert( jeActor_IsValid(A) != JE_FALSE);	
	assert( jeActor_DefIsValid(A->ActorDefinition) != JE_FALSE );
	assert( jeBody_IsValid(A->ActorDefinition->Body) != JE_FALSE );
	assert( BoneIndex != NULL );	

	if ( BoneName != NULL )
	{
		if (jeBody_GetBoneByName(A->ActorDefinition->Body,
								 BoneName,
								 BoneIndex,
								 &Dummy,
								 &ParentBoneIndex) ==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_GetBoneIndex: Named bone not found:", BoneName);
			return JE_FALSE;			
		}
	}
	else
	{
		*BoneIndex = JE_POSE_ROOT_JOINT;
	}
	return JE_TRUE;
}


JETAPI jeActor_Def *JETCC jeActor_GetActorDef(const jeActor *A)
{
	assert( jeActor_DefIsValid(A->ActorDefinition) != JE_FALSE );
	return A->ActorDefinition;
}

JETAPI void JETCC jeActor_DefCreateRef(jeActor_Def *A)
{
	assert( jeActor_DefIsValid(A) != JE_FALSE );
	A->RefCount++;
	jeActor_DefRefCount++;
}

JETAPI jeActor_Def *JETCC jeActor_DefCreate(void)
{
	jeActor_Def *Ad;

	Ad = JE_RAM_ALLOCATE_STRUCT_CLEAR( jeActor_Def );
	if ( Ad == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE,"jeActor_DefCreateRef: Failed to allocate Actor_Def");
			return NULL;
		}

	Ad->Body				= NULL;
	Ad->MotionCount			= 0;
	Ad->MotionArray			= NULL;
	Ad->ValidityCheck		= Ad;
	Ad->RefCount            = 0;
	jeActor_DefCount++;
	return Ad;
}

JETAPI void JETCC jeActor_CreateRef(jeActor *Actor)
{
	assert( jeActor_IsValid(Actor) );	
	Actor->RefCount ++;
	jeActor_RefCount++;
}

JETAPI jeActor * JETCC jeActor_CreateFromDef(jeActor_Def *ActorDefinition)
{
	jeActor *A;	

	A = JE_RAM_ALLOCATE_STRUCT_CLEAR( jeActor );
	if ( A == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeActor_Create: Failed to allocate jeActor struct");
			return NULL;
		}

	A->Puppet = NULL;
	A->Pose   = NULL;
	A->CueMotion = NULL;
	A->ActorDefinition = NULL;	
	A->RefCount          = 0;
	A->CanFree = JE_TRUE;
	jeExtBox_Set(&(A->CollisionExtBox), 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
	jeExtBox_Set(&(A->RenderHintExtBox), 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
	jeXForm3d_SetIdentity(&A->Xf);
	jeActor_Count++;

	if(ActorDefinition != NULL)
		jeActor_SetActorDef(A,ActorDefinition);

	return A;
}

JETAPI jeActor *JETCC jeActor_Create()
{
	jeActor *A;	

	A = JE_RAM_ALLOCATE_STRUCT_CLEAR( jeActor );
	if ( A == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeActor_Create: Failed to allocate jeActor struct");
		return NULL;
	}

	A->Puppet = NULL;
	A->Pose   = NULL;
	A->CueMotion = NULL;
	A->ActorDefinition = NULL;	
	A->RefCount          = 0;
	A->CanFree = JE_TRUE;
	A->RenderNextTime = JE_TRUE;
	InitializeCriticalSection(&A->RenderLock);
	jeExtBox_Set(&(A->CollisionExtBox), 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
	jeExtBox_Set(&(A->RenderHintExtBox), 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
	jeXForm3d_SetIdentity(&A->Xf);
	jeActor_Count++;

	return A;
}

JETAPI jeBoolean JETCC jeActor_DefDestroy(jeActor_Def **pActorDefinition)
{
	int i;
	jeActor_Def *Ad;
	assert(  pActorDefinition != NULL );
	assert( *pActorDefinition != NULL );
	assert( jeActor_DefIsValid( *pActorDefinition ) != JE_FALSE );

	Ad = *pActorDefinition;

	if (Ad->RefCount > 0)
	{
		Ad->RefCount--;
		jeActor_DefRefCount--;
		return JE_FALSE;
	}

	if (Ad->Body != NULL)
	{
		jeBody_Destroy( &(Ad->Body) );
		Ad->Body = NULL;
	}
	if (Ad->MotionArray != NULL)
	{
		for (i=0; i<Ad->MotionCount; i++)
		{
			if (Ad->MotionArray[i]!=NULL)
			{
				jeMotion_Destroy( &(Ad->MotionArray[i]) );
			}
			Ad->MotionArray[i] = NULL;
		}
		jeRam_Free( Ad->MotionArray );
		Ad->MotionArray = NULL;
	}
				
	Ad->MotionCount = 0;

	jeRam_Free(*pActorDefinition);
	*pActorDefinition = NULL;
	jeActor_DefCount--;
	return JE_TRUE;
}


JETAPI jeBoolean JETCC jeActor_Destroy(jeActor **pA)
{
	jeActor *A;
	jeChain_Link *Link;

	assert(  pA != NULL );
	assert( *pA != NULL );
	assert( jeActor_IsValid(*pA) != JE_FALSE );	
	
	A = *pA;
	if (A->RefCount > 0)
	{
		A->RefCount --;
		jeActor_RefCount--;
		return JE_FALSE;
	}

	if (A->Puppet != NULL)
	{
		jePuppet_Destroy( &(A->Puppet) );
		A->Puppet = NULL;
	}
	if ( A->Pose != NULL )
	{
		jePose_Destroy( &( A->Pose ) );
		A->Pose = NULL;
	}
	if ( A->CueMotion != NULL )
	{
		jeMotion_Destroy(&(A->CueMotion));
		A->CueMotion = NULL;
	}

	// destroy bone collision list -- Incarnadine
	if(A->BoneCollisionChain != NULL)
	{
		for (Link = jeChain_GetFirstLink(A->BoneCollisionChain); Link; Link = jeChain_LinkGetNext(Link))
		{
			// Icestorm: Added BoneExtBoxes
			jeCollisionBone *Bone;
							
			Bone = (jeCollisionBone*)jeChain_LinkGetLinkData( Link );
			if(Bone)
			{
				free(Bone->BoneName);
				jeRam_Free(Bone->CurrExtBox);
				jeRam_Free(Bone->PrevExtBox);
				jeRam_Free(Bone);
			}
		}

		// destroy object chain
		jeChain_Destroy( &( A->BoneCollisionChain ) );
		A->BoneCollisionChain = NULL;
	}
		
	if(A->ActorDefinition != NULL)
	{
		jeActor_DefDestroy(&(A->ActorDefinition));
		A->ActorDefinition = NULL;
	}

	if(A->Object != NULL)
	{
		jeRam_Free(A->Object);
		A->Object = NULL;
	}

	DeleteCriticalSection(&A->RenderLock);
	
	if(A->CanFree == JE_TRUE)
	{
		jeActor_Count--;
		jeRam_Free(*pA);	
		*pA = NULL;
	}


	return JE_TRUE;
}

// Incarnadine
// INCNOTE: This function isn't right, it needs to totally kill the actor
// Do a new create basically.  Look into making a separate function
// that calls the old actor create function.
JETAPI void JETCC jeActor_SetActorDef(jeActor *A, jeActor_Def *Def)
{
	ActorObj *pObj;

	assert(A);
	assert(Def);
	assert( jeActor_DefIsValid(Def) != JE_FALSE );
	assert( jeBody_IsValid(Def->Body)  != JE_FALSE );
		
	if(A->Pose != NULL)
	{
		pObj = A->Object;
		A->Object = NULL;
		A->CanFree = JE_FALSE;
		jeActor_Destroy(&A);
		A->CanFree = JE_TRUE;		
		A->Object = pObj;
	}	
	
	A->ActorDefinition = Def;

	jeExtBox_Set(&(A->CollisionExtBox), 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
	jeExtBox_Set(&(A->RenderHintExtBox), 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
	jeXForm3d_SetIdentity(&A->Xf);

	A->Pose = jePose_Create();
	if (A->Pose == NULL)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE , "jeActor_Create: Failed to allocate Pose");
		goto ActorCreateFailure;
	}
	
	A->CueMotion		 = jeMotion_Create(JE_TRUE);
	
	A->BlendingType		 = JE_ACTOR_BLEND_HERMITE;
	A->BoundingBoxCenterBoneIndex = JE_POSE_ROOT_JOINT;
	A->RenderHintExtBoxCenterBoneIndex = JE_POSE_ROOT_JOINT;
	A->RenderHintExtBoxEnabled = JE_FALSE;
	A->StepBoneIndex     = JE_POSE_ROOT_JOINT;

	if (A->CueMotion == NULL)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE , "jeActor_Create: Failed to allocate CueMotion");
		goto ActorCreateFailure;
	}


	A->BoneCollisionChain = jeChain_Create(); // Incarnadine
	A->CollisionFlags = COLLIDE_SOLID; // Incarnadine
	A->LastUsedCollisionBone=NULL;	// Icestorm
	A->LastUsedCollisionBoneName=NULL;	// Icestorm
	A->needsRelighting = JE_TRUE;

	assert( jeActor_IsValid(A) != JE_FALSE );		
	
	{
		int i; 
		int BoneCount;

		BoneCount = jeBody_GetBoneCount(A->ActorDefinition->Body);
		for (i=0; i<BoneCount; i++)
		{
			const char *Name;
			jeXForm3d Attachment;
			int ParentBone;
			int Index;
			jeBody_GetBone( A->ActorDefinition->Body, i, &Name,&Attachment, &ParentBone );
			if (jePose_AddJoint( A->Pose,
								ParentBone,Name,&Attachment,&Index)==JE_FALSE)
			{
				jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_Create: jePose_AddJoint failed");
				A->ActorDefinition;
				return;
			}
		}
	}

	jeActor_DefCreateRef(Def);

	if(A->Object && A->Object->Engine)
		jeActor_AttachEngine(A, A->Object->Engine );

	return;

ActorCreateFailure:
	if ( A!= NULL)
	{
		if (A->Pose != NULL)
			jePose_Destroy(&(A->Pose));
		if (A->CueMotion != NULL)
			jeMotion_Destroy(&(A->CueMotion));
		jeRam_Free( A );
	}
}

JETAPI jeBoolean JETCC jeActor_SetBody( jeActor_Def *ActorDefinition, jeBody *BodyGeometry)
{
	assert( jeBody_IsValid(BodyGeometry) != JE_FALSE );
	
	if (ActorDefinition->RefCount > 0)
	{	
		jeErrorLog_Add(JE_ERR_BAD_PARAMETER,"jeActor_SetBody: ActorDef in use, can't modify body");
		return JE_FALSE;
	}

	if (ActorDefinition->Body != NULL)
	{
		jeBody_Destroy( &(ActorDefinition->Body) );
	}
	
	ActorDefinition->Body          = BodyGeometry;
	return JE_TRUE;
}


#pragma message ("consider removing this and related parameters to setpose")
JETAPI void JETCC jeActor_SetBlendingType( jeActor *A, jeActor_BlendingType BlendingType )
{
	assert( jeActor_IsValid(A) != JE_FALSE );

	assert( (BlendingType == JE_ACTOR_BLEND_LINEAR) || 
			(BlendingType == JE_ACTOR_BLEND_HERMITE) );

	if (BlendingType == JE_ACTOR_BLEND_LINEAR)
		{
			A->BlendingType = (jeActor_BlendingType)JE_POSE_BLEND_LINEAR;
		}
	else
		{
			A->BlendingType = (jeActor_BlendingType)JE_POSE_BLEND_HERMITE;
		}
}

jeVFile *jeActor_DefGetFileContext(const jeActor_Def *A)
{
	assert( jeActor_DefIsValid(A) != JE_FALSE );
	return A->TextureFileContext;
}



JETAPI jeBoolean JETCC jeActor_AddMotion(jeActor_Def *Ad, jeMotion *NewMotion, int32 *Index)
{
	jeMotion **NewMArray;
	assert( jeActor_DefIsValid(Ad) != JE_FALSE );
	assert( NewMotion != NULL );
	assert( Index != NULL );

	if (Ad->MotionCount >= ACTOR_MOTIONS_MAX)
		{
			jeErrorLog_Add(JE_ERR_LIST_FULL,"jeActor_AddMotion: Too many motions");
			return JE_FALSE;
		}
	NewMArray = JE_RAM_REALLOC_ARRAY( Ad->MotionArray, jeMotion*, Ad->MotionCount +1 );
	if ( NewMArray == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE,"jeActor_AddMotion: Failed to reallocate motion array");
			return JE_FALSE;
		}

	Ad->MotionArray = NewMArray;

	Ad->MotionArray[Ad->MotionCount]= NewMotion;
	Ad->MotionCount++;
	*Index = Ad->MotionCount;
	return JE_TRUE;
};

JETAPI void JETCC jeActor_ClearPose(jeActor *A, const jeXForm3d *Transform)
{
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert ( (Transform==NULL) || (jeXForm3d_IsOrthonormal(Transform) != JE_FALSE) );
	jePose_Clear( A->Pose ,Transform);
	A->Xf = *Transform; //Incarnadine
	A->needsRelighting = JE_TRUE;
}

JETAPI void JETCC jeActor_SetPose(jeActor *A, const jeMotion *M, 
								jeFloat Time, const jeXForm3d *Transform)
{
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert( M != NULL );
	assert ( (Transform==NULL) || (jeXForm3d_IsOrthonormal(Transform) != JE_FALSE) );

	jePose_SetMotion( A->Pose,M,Time,Transform);
	A->Xf = *Transform; //Incarnadine
	A->needsRelighting = JE_TRUE;
}

JETAPI void JETCC jeActor_BlendPose(jeActor *A, const jeMotion *M, 
								jeFloat Time,  
								const jeXForm3d *Transform,
								jeFloat BlendAmount)
{
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert( M != NULL );
	assert ( (Transform==NULL) || (jeXForm3d_IsOrthonormal(Transform) != JE_FALSE) );

	jePose_BlendMotion( A->Pose,M,Time,Transform,
						BlendAmount,(jePose_BlendingType)A->BlendingType);
	A->Xf = *Transform; //Incarnadine
	A->needsRelighting = JE_TRUE;
}


JETAPI int32 JETCC jeActor_GetMotionCount(const jeActor_Def *Ad)
{
	assert( jeActor_DefIsValid(Ad) != JE_FALSE );
	return Ad->MotionCount;
}
	

JETAPI jeMotion *JETCC jeActor_GetMotionByIndex(const jeActor_Def *Ad, int32 Index )
{
	assert( jeActor_DefIsValid(Ad) != JE_FALSE );
	assert( Index >= 0 );
	assert( Index < Ad->MotionCount );
	assert( Ad->MotionArray != NULL );

	return Ad->MotionArray[Index];
}

JETAPI jeMotion *JETCC jeActor_GetMotionByName(const jeActor_Def *Ad, const char *Name )
{
	int i;
	const char *TestName;
	assert( jeActor_DefIsValid(Ad) != JE_FALSE );
	assert( Name != NULL );
	for (i=0; i<Ad->MotionCount; i++)
		{
			TestName = jeMotion_GetName(Ad->MotionArray[i]);
			if (TestName != NULL)
				{
					if (_stricmp(TestName,Name)==0) // Case insensitive compare -- Incarnadine
						return Ad->MotionArray[i];
				}

		}
	return NULL;
}

JETAPI const char *JETCC jeActor_GetMotionName(const jeActor_Def *Ad, int32 Index )
{
	assert( jeActor_DefIsValid(Ad) != JE_FALSE );
	assert( Index >= 0 );
	assert( Index < Ad->MotionCount );
	assert( Ad->MotionArray != NULL );
	return jeMotion_GetName(Ad->MotionArray[Index]);
}
	
JETAPI jeBody *JETCC jeActor_GetBody(const jeActor_Def *Ad)
{
	assert( jeActor_DefIsValid(Ad) != JE_FALSE );
	return Ad->Body;
}
	
#pragma message ("consider removing the function: jeActor_DefHasBoneNamed")
// Returns JE_TRUE if the actor definition has a bone named 'Name'
JETAPI jeBoolean JETCC jeActor_DefHasBoneNamed(const jeActor_Def *Ad, const char *Name )
{
	int DummyIndex,DummyParent;
	jeXForm3d DummyAttachment;

	assert( jeActor_DefIsValid(Ad) != JE_FALSE );
	assert( Name != NULL );
	if (jeBody_GetBoneByName(jeActor_GetBody(Ad),Name,
			&DummyIndex, &DummyAttachment, &DummyParent ) == JE_FALSE )
		{
			return JE_FALSE;
		}
	return JE_TRUE;
}


#define JE_ACTOR_BODY_NAME       "Body"
#define JE_ACTOR_HEADER_NAME     "Header"
#define JE_MOTION_DIRECTORY_NAME "Motions"

#define ACTOR_FILE_TYPE 0x52544341      // 'ACTR'
#define ACTOR_FILE_VERSION 0x00F2		// Restrict version to 16 bits



static jeActor_Def * JETCF jeActor_DefCreateHeader(jeVFile *pFile, jeBoolean *HasBody)
{
	uint32 u;
	uint32 version;
	jeActor_Def *Ad;

	assert( pFile != NULL );
	assert( HasBody != NULL );

	if( ! jeVFile_Read(pFile, &u, sizeof(u)) )
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeActor_DefCreateHeader - Failed to read header tag");	jeActor_DefDestroy(&Ad); return NULL;}

	if (u != ACTOR_FILE_TYPE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_FORMAT , "jeActor_DefCreateHeader - Failed to recognize format");	return NULL;}

	if(jeVFile_Read(pFile, &version, sizeof(version)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeActor_DefCreateHeader - Read Failed");	return NULL;}
	if ( (version != ACTOR_FILE_VERSION) )
		{	jeErrorLog_Add( JE_ERR_FILEIO_VERSION , "jeActor_DefCreateHeader - Bad version");	return NULL;}

	Ad = jeActor_DefCreate();
	if (Ad==NULL)
		{	jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "jeActor_DefCreateHeader - Failed to create Def struct"); return NULL; }

	if(jeVFile_Read(pFile, HasBody, sizeof(*HasBody)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeActor_DefCreateHeader - Read Failed");	jeActor_DefDestroy(&Ad); return NULL;}

	if(jeVFile_Read(pFile, &(Ad->MotionCount), sizeof(Ad->MotionCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeActor_DefCreateHeader - Read failed");	jeActor_DefDestroy(&Ad); return NULL;}

	return Ad;
}


static jeBoolean JETCF jeActor_DefWriteHeader(const jeActor_Def *Ad, jeVFile *pFile)
{
	uint32 u;
	jeBoolean Flag;
	
	assert( jeActor_DefIsValid(Ad) != JE_FALSE );
	assert( pFile != NULL );

	// Write the format flag
	u = ACTOR_FILE_TYPE;
	if(jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteHeader - Write failed");	return JE_FALSE; }


	u = ACTOR_FILE_VERSION;
	if(jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteHeader - Write failed");	return JE_FALSE;}

	if (Ad->Body != NULL)
		Flag = JE_TRUE;
	else 
		Flag = JE_FALSE;

	if(jeVFile_Write(pFile, &Flag, sizeof(Flag)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteHeader - Write failed");	return JE_FALSE;}

	if(jeVFile_Write(pFile, &(Ad->MotionCount), sizeof(Ad->MotionCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteHeader - Write failed");	return JE_FALSE;}

	#ifdef COUNT_HEADER_SIZES
		Header_Sizes += 16;
	#endif

	return JE_TRUE;
}
	


JETAPI jeActor_Def *JETCC jeActor_DefCreateFromFile(jeVFile *pFile)
{
	int i;
	jeActor_Def *Ad   = NULL;
	jeVFile *VFile    = NULL;
	jeVFile *SubFile  = NULL;
	jeVFile *MotionDirectory = NULL;	
	jeBoolean HasBody = JE_FALSE;
	jeBody * Body     = NULL;
			
	assert( pFile != NULL );

	VFile = jeVFile_OpenNewSystem(pFile,JE_VFILE_TYPE_VIRTUAL, NULL, 
									NULL, JE_VFILE_OPEN_DIRECTORY | JE_VFILE_OPEN_READONLY);
	if (VFile == NULL)
		{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeActor_DefCreateFromFile - Failed to open actor vfile system");	goto CreateError;}
	

	SubFile = jeVFile_Open(VFile,JE_ACTOR_HEADER_NAME,JE_VFILE_OPEN_READONLY);
	if (SubFile == NULL)
		{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeActor_DefCreateFromFile - Failed to open header subfile");	goto CreateError;}

#if 1
	{
		jeVFile * HFile;
		HFile = jeVFile_GetHintsFile(SubFile);
		if ( ! HFile ) // <> backwards compatibility
			HFile = SubFile;
		Ad = jeActor_DefCreateHeader(HFile, &HasBody);
	}
#else
	Ad = jeActor_DefCreateHeader( SubFile, &HasBody);
#endif

	if (Ad == NULL)
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeActor_DefCreateFromFile -");
		goto CreateError;
	}
	if (!jeVFile_Close(SubFile))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_CLOSE ,"jeActor_DefCreateFromFile - Failed to close header subfile");
		goto CreateError;
	}
		

	Ad->TextureFileContext = VFile;
	assert(Ad->TextureFileContext);

	if (HasBody != JE_FALSE)
	{
		SubFile = jeVFile_Open(VFile,JE_ACTOR_BODY_NAME,JE_VFILE_OPEN_READONLY);
		if (SubFile == NULL)
			{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeActor_DefCreateFromFile - Read failed");	goto CreateError;}

		Body = jeBody_CreateFromFile(SubFile);
		if (Body == NULL)
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeActor_DefCreateFromFile - Read failed");
			goto CreateError;
		}
		if (jeActor_SetBody(Ad,Body)==JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeActor_DefCreateFromFile -");
			goto CreateError;
		}
		jeVFile_Close(SubFile);
	}

	MotionDirectory = jeVFile_Open(VFile,JE_MOTION_DIRECTORY_NAME, 
									JE_VFILE_OPEN_DIRECTORY | JE_VFILE_OPEN_READONLY);
	if (MotionDirectory == NULL)
		{	jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE,"jeActor_DefCreateFromFile -");	return NULL;}

	if (Ad->MotionCount>0)
		{
			Ad->MotionArray = JE_RAM_ALLOCATE_ARRAY( jeMotion*, Ad->MotionCount);
			if (Ad->MotionArray == NULL)
				{	jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE,"jeActor_DefCreateFromFile - Failed to allocate motion array");	return NULL; }
			for (i=0; i<Ad->MotionCount; i++)
				Ad->MotionArray[i] = NULL;
	
			for (i=0; i<Ad->MotionCount; i++)
				{
					char FName[1000];
					sprintf(FName,"%d",i);

					SubFile = jeVFile_Open(MotionDirectory,FName,JE_VFILE_OPEN_READONLY);
					if (SubFile == NULL)
						{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN ,"jeActor_DefCreateFromFile - Failed to open motion subdirectory");	goto CreateError;}

					#if 0	
						Ad->MotionArray[i] = jeMotion_CreateFromFile(SubFile);
					#else
						{
							jeVFile * LZFS;

							LZFS = jeVFile_OpenNewSystem(SubFile,JE_VFILE_TYPE_LZ, NULL, NULL,JE_VFILE_OPEN_READONLY);
			
							if ( !LZFS )
								{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN ,"jeActor_DefCreateFromFile - Failed to open compressed vfile");	goto CreateError;}
					
							Ad->MotionArray[i] = jeMotion_CreateFromFile(LZFS);

							Log_Printf("Actor : Motions : ");
							if ( ! jeVFile_Close(LZFS) )
								{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE ,"jeActor_DefCreateFromFile - Failed to close compressed vfile");	goto CreateError;}

						}
					#endif

					if (Ad->MotionArray[i] == NULL)
						{	jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE,"jeActor_DefCreateFromFile - Failed to read motion #",jeErrorLog_IntToString(i));	goto CreateError;}

					if (!jeVFile_Close(SubFile) )
						{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE ,"jeActor_DefCreateFromFile - Failed to close sub motion file");	goto CreateError;}

				}
		}
	else
		{
			Ad->MotionArray = NULL;
		}
	if (!jeVFile_Close(MotionDirectory))
		{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE ,"jeActor_DefCreateFromFile - Failed to close motion directory");	goto CreateError;}

	if (!jeVFile_Close(VFile))
		{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE ,"jeActor_DefCreateFromFile - Failed to close actor vfile system");	goto CreateError;}

	return Ad;

	CreateError:
		if (SubFile != NULL)
			jeVFile_Close(SubFile);
		if (MotionDirectory != NULL)
			jeVFile_Close(MotionDirectory);
		if (VFile != NULL)
			jeVFile_Close(VFile);
		if (Ad != NULL)
			jeActor_DefDestroy(&Ad);
		return NULL;
}


JETAPI jeBoolean JETCC jeActor_DefWriteToFile(const jeActor_Def *Ad, jeVFile *pFile)
{
	int i;
	jeVFile *VFile;
	jeVFile *SubFile;
	jeVFile *MotionDirectory;

	assert( jeActor_DefIsValid(Ad) != JE_FALSE );
	assert( pFile != NULL );

	VFile = jeVFile_OpenNewSystem(pFile,JE_VFILE_TYPE_VIRTUAL, NULL, 
									NULL, JE_VFILE_OPEN_DIRECTORY | JE_VFILE_OPEN_CREATE);
	if (VFile == NULL)
		{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeActor_DefWriteToFile - Failed to open actor vfile system");	goto WriteError;}
	
	SubFile = jeVFile_Open(VFile,JE_ACTOR_HEADER_NAME,JE_VFILE_OPEN_CREATE);
	if (SubFile == NULL)
		{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeActor_DefWriteToFile - Failed to open actor header subfile");	goto WriteError;}

#if 1
	{
	jeVFile * HFile;
	HFile = jeVFile_GetHintsFile(SubFile);
	if (jeActor_DefWriteHeader(Ad,HFile)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteToFile - Failed to write hints");	goto WriteError;}
	}
#else
	if (jeActor_DefWriteHeader(Ad,SubFile)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteToFile - Failed to write header");	goto WriteError;}
#endif

	if (jeVFile_Close(SubFile)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeActor_DefWriteToFile - Failed to close header");	goto WriteError;}

	if (Ad->Body != NULL)
		{
			SubFile = jeVFile_Open(VFile,JE_ACTOR_BODY_NAME,JE_VFILE_OPEN_CREATE);
			if (SubFile == NULL)
				{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteToFile - Failed to open body subfile");	goto WriteError;}

			if (jeBody_WriteToFile(Ad->Body,SubFile)==JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeActor_DefWriteToFile - Failed to write body");	goto WriteError;}

			if (jeVFile_Close(SubFile)==JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeActor_DefWriteToFile - Failed to close body subfile");	goto WriteError;}
		}

	MotionDirectory = jeVFile_Open(VFile,JE_MOTION_DIRECTORY_NAME, 
									JE_VFILE_OPEN_DIRECTORY | JE_VFILE_OPEN_CREATE);
	if (MotionDirectory == NULL)
		{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeActor_DefWriteToFile - Failed to open motion subdirectory");	goto WriteError;}
	
	// <> CB note : could save some by combining these motions in one LZ file

	for (i=0; i<Ad->MotionCount; i++)
	{
		char FName[1000];
		sprintf(FName,"%d",i);

		SubFile = jeVFile_Open(MotionDirectory,FName,JE_VFILE_OPEN_CREATE);
		if (SubFile == NULL)
			{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeActor_DefWriteToFile - Failed to open motion subfile");	goto WriteError;}

		#if 0 //{
				if (jeMotion_WriteToFile(Ad->MotionArray[i],SubFile)==JE_FALSE)
					{	jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE , "jeActor_DefWriteToFile - Failed to write motion");	goto WriteError;}
		#else //}{
			{
				jeVFile * LZFS;

				LZFS = jeVFile_OpenNewSystem(SubFile,JE_VFILE_TYPE_LZ, NULL, NULL, JE_VFILE_OPEN_CREATE);
				if ( ! LZFS )
					{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeActor_DefWriteToFile - Failed to open compressed system");	goto WriteError;}

				if (jeMotion_WriteToFile(Ad->MotionArray[i],LZFS)==JE_FALSE)
					{	jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeActor_DefWriteToFile - Failed to write motion");	goto WriteError;}

				if ( ! jeVFile_Close(LZFS) )
					{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeActor_DefWriteToFile - Failed to close compressed system");	goto WriteError;}

			}
		#endif //}

		if (jeVFile_Close(SubFile)==JE_FALSE)
			{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteToFile - Failed to close motion subfile");	goto WriteError;}
	}

	if (jeVFile_Close(MotionDirectory)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteToFile - Failed to close motion subdirectory");	goto WriteError;}
	if (jeVFile_Close(VFile)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeActor_DefWriteToFile - Failed to close actor subsystem");	goto WriteError;}
	
	return JE_TRUE;
	WriteError:
		return JE_FALSE;
}

JETAPI jeBoolean JETCC jeActor_GetBoneTransform(const jeActor *A, const char *BoneName, jeXForm3d *Transform)
{
	int BoneIndex;

	assert( jeActor_IsValid(A)!=JE_FALSE );
	assert( Transform!= NULL );
	
	if (jeActor_GetBoneIndex(A,BoneName,&BoneIndex)==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_GetBoneTransform: Named bone not found", BoneName);
			return JE_FALSE;
		}

	jePose_GetJointTransform(   A->Pose, BoneIndex,	Transform);
	assert ( jeXForm3d_IsOrthonormal(Transform) != JE_FALSE );

	return JE_TRUE;
}


static void JETCF jeActor_AccumulateMinMax(
	jeVec3d *P,jeVec3d *Mins,jeVec3d *Maxs)
{
	assert( jeVec3d_IsValid( P  ) != JE_FALSE );
	assert( jeVec3d_IsValid(Mins) != JE_FALSE );
	assert( jeVec3d_IsValid(Maxs) != JE_FALSE );
	
	if (P->X < Mins->X) Mins->X = P->X;
	if (P->Y < Mins->Y) Mins->Y = P->Y;
	if (P->Z < Mins->Z) Mins->Z = P->Z;

	if (P->X > Maxs->X) Maxs->X = P->X;
	if (P->Y > Maxs->Y) Maxs->Y = P->Y;
	if (P->Z > Maxs->Z) Maxs->Z = P->Z;
}



static jeBoolean JETCF jeActor_GetBoneBoundingBoxByIndex(
	const jeActor *A, 
	int BoneIndex,
	jeVec3d   *Corner,
	jeVec3d   *DX,
	jeVec3d   *DY,
	jeVec3d   *DZ)
{
	jeVec3d Min,Max;
	jeVec3d Orientation;
	jeXForm3d Transform;
	
	assert( jeActor_IsValid(A) != JE_FALSE );	
	assert( jeActor_DefIsValid(A->ActorDefinition) != JE_FALSE );
	assert( A->ActorDefinition->Body   != NULL );

	assert( Corner      );
	assert( DX          );
	assert( DY          );
	assert( DZ          );
	assert( (BoneIndex < jePose_GetJointCount(A->Pose)) || (BoneIndex ==JE_POSE_ROOT_JOINT));
	assert( (BoneIndex >=0)                             || (BoneIndex ==JE_POSE_ROOT_JOINT));
	
	if (jeBody_GetBoundingBox( A->ActorDefinition->Body, BoneIndex, &Min, &Max )==JE_FALSE)
		{
			// not probably a real error.  It's possible that the bone has no geometry, so it
			// has no bounding box.
			return JE_FALSE;
		}

	// scale bounding box:
	{
		jeVec3d Scale;
		jePose_GetScale(A->Pose, &Scale);
		assert( jeVec3d_IsValid(&Scale) != JE_FALSE );

		Min.X *= Scale.X;
		Min.Y *= Scale.Y;
		Min.Z *= Scale.Z;
		
		Max.X *= Scale.X;
		Max.Y *= Scale.Y;
		Max.Z *= Scale.Z;
	}


	jePose_GetJointTransform(A->Pose,BoneIndex,&(Transform));

	jeVec3d_Subtract(&Max,&Min,&Orientation);
			
	DX->X = Orientation.X;	DX->Y = DX->Z = 0.0f;
	DY->Y = Orientation.Y;	DY->X = DY->Z = 0.0f;
	DZ->Z = Orientation.Z;	DZ->X = DZ->Y = 0.0f;
			
	// transform into world space
	jeXForm3d_Transform(&(Transform),&Min,&Min);
	jeXForm3d_Rotate(&(Transform),DX,DX);
	jeXForm3d_Rotate(&(Transform),DY,DY);
	jeXForm3d_Rotate(&(Transform),DZ,DZ);

	*Corner = Min;
	return JE_TRUE;
}



static jeBoolean JETCF jeActor_GetBoneExtBoxByIndex(
	const jeActor *A, 
	int BoneIndex,
	jeExtBox *ExtBox)
{
	jeVec3d Min;
	jeVec3d DX,DY,DZ,Corner;

	assert( ExtBox );
		
	if (jeActor_GetBoneBoundingBoxByIndex(A,BoneIndex,&Min,&DX,&DY,&DZ)==JE_FALSE)
		{
			// Commented out by Incarnadine:  This happens frequently when dealing
			// with boned objects if a bone has no geometry (like any physiqued object
		    // with BIP01).  It was slowing things down to make this call several times / tick.
			//jeErrorLog_Add(JE_ERR_BAD_PARAMETER,"jeActor_GetBoneExtBoxByIndex - ");			
			return JE_FALSE;
		}

	ExtBox->Min = Min;
	ExtBox->Max = Min;
	Corner = Min;
	// should use extent box (extbox) methods rather than this
	jeVec3d_Add(&Corner,&DX,&Corner);
	jeActor_AccumulateMinMax(&Corner,&(ExtBox->Min),&(ExtBox->Max));
	jeVec3d_Add(&Corner,&DZ,&Corner);
	jeActor_AccumulateMinMax(&Corner,&(ExtBox->Min),&(ExtBox->Max));
	jeVec3d_Subtract(&Corner,&DX,&Corner);
	jeActor_AccumulateMinMax(&Corner,&(ExtBox->Min),&(ExtBox->Max));
	jeVec3d_Add(&Corner,&DY,&Corner);
	jeActor_AccumulateMinMax(&Corner,&(ExtBox->Min),&(ExtBox->Max));
	jeVec3d_Add(&Corner,&DX,&Corner);
	jeActor_AccumulateMinMax(&Corner,&(ExtBox->Min),&(ExtBox->Max));
	jeVec3d_Subtract(&Corner,&DZ,&Corner);
	jeActor_AccumulateMinMax(&Corner,&(ExtBox->Min),&(ExtBox->Max));
	jeVec3d_Subtract(&Corner,&DX,&Corner);
	jeActor_AccumulateMinMax(&Corner,&(ExtBox->Min),&(ExtBox->Max));

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeActor_GetBoneExtBox(const jeActor *A,
									 const char *BoneName,
									 jeExtBox *ExtBox)
{
	int BoneIndex;
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( ExtBox != NULL );
	
	if (jeActor_GetBoneIndex(A,BoneName,&BoneIndex)==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_GetBoneExtBox: Named bone for bounding box not found: ", BoneName);
			return JE_FALSE;
		}
	return jeActor_GetBoneExtBoxByIndex(A,BoneIndex,ExtBox);
}


JETAPI jeBoolean JETCC jeActor_GetBoneBoundingBox(const jeActor *A,
								 const char *BoneName,
								 jeVec3d *Corner,
								 jeVec3d *DX,
								 jeVec3d *DY,
								 jeVec3d *DZ)
{
	int BoneIndex;
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( Corner    != NULL );
	assert( DX        != NULL );
	assert( DY        != NULL );
	assert( DZ        != NULL );

	if (jeActor_GetBoneIndex(A,BoneName,&BoneIndex)==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_GetBoneBoundingBox - Named bone for bounding box not found: ", BoneName);
			return JE_FALSE;
		}
	if (jeActor_GetBoneBoundingBoxByIndex(A,BoneIndex,Corner,DX,DY,DZ)==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_GetBoneBoundingBox - Failed to get bounding box named: ", BoneName);
			//jeErrorLog_AppendString(BoneName);
			return JE_FALSE;
		}
	return JE_TRUE;
}



JETAPI jeBoolean JETCC jeActor_GetExtBox(const jeActor *A, jeExtBox *ExtBox)
{
	jeXForm3d Transform;
	
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( ExtBox != NULL );
	
	jePose_GetJointTransform(   A->Pose,
								A->BoundingBoxCenterBoneIndex,
								&Transform);	
	assert ( jeXForm3d_IsOrthonormal(&Transform) != JE_FALSE );
	jeVec3d_Add( &(Transform.Translation), &(A->CollisionExtBox.Min), &(ExtBox->Min));
	jeVec3d_Add( &(Transform.Translation), &(A->CollisionExtBox.Max), &(ExtBox->Max));
	return JE_TRUE;
}


JETAPI jeBoolean JETCC jeActor_SetExtBox(jeActor *A,
												 const jeExtBox *ExtBox,
												 const char *CenterOnThisNamedBone)
{
	
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( jeExtBox_IsValid(ExtBox) != JE_FALSE);
	
	A->CollisionExtBox.Min = ExtBox->Min;
	A->CollisionExtBox.Max = ExtBox->Max;
	
	if (jeActor_GetBoneIndex(A,CenterOnThisNamedBone,&(A->BoundingBoxCenterBoneIndex))==JE_FALSE)
	{
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_SetExtBox: Named bone for bounding box not found ", CenterOnThisNamedBone);
		return JE_FALSE;
	}
	
	return JE_TRUE;
}


	// Gets the rendering hint bounding box from the actor
JETAPI jeBoolean JETCC jeActor_GetRenderHintExtBox(const jeActor *A, jeExtBox *Box, jeBoolean *Enabled)
{
	jeXForm3d Transform;

	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( Box != NULL );
	assert( Enabled != NULL );

	jePose_GetJointTransform( A->Pose,
								A->RenderHintExtBoxCenterBoneIndex,
								&Transform);	
	assert ( jeXForm3d_IsOrthonormal(&Transform) != JE_FALSE );

	*Box = A->RenderHintExtBox;
	jeExtBox_Translate ( Box, Transform.Translation.X,
							  Transform.Translation.Y,
							  Transform.Translation.Z );
	
	*Enabled = A->RenderHintExtBoxEnabled;
	return JE_TRUE;
}

	// Sets a rendering hint bounding box from the actor
JETAPI jeBoolean JETCC jeActor_SetRenderHintExtBox(jeActor *A, const jeExtBox *Box, 
												const char *CenterOnThisNamedBone)
{
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( Box != NULL );
	assert( Box->Max.X >= Box->Min.X );
	assert( Box->Max.Y >= Box->Min.Y );
	assert( Box->Max.Z >= Box->Min.Z );
	
	if (jeActor_GetBoneIndex(A,CenterOnThisNamedBone,&(A->RenderHintExtBoxCenterBoneIndex))==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_SetRenderHintExtBox: Named bone for render hint box not found: ", CenterOnThisNamedBone);
			return JE_FALSE;
		}
		
	A->RenderHintExtBox = *Box;
	if (   (Box->Min.X == 0.0f) && (Box->Max.X == 0.0f)
		&& (Box->Min.Y == 0.0f) && (Box->Max.Y == 0.0f) 
		&& (Box->Min.Z == 0.0f) && (Box->Max.Z == 0.0f) )
		{
			A->RenderHintExtBoxEnabled = JE_FALSE;
		}
	else
		{
			A->RenderHintExtBoxEnabled = JE_TRUE;
		}

	return JE_TRUE;
}


JETAPI void *JETCC jeActor_GetUserData(const jeActor *A)
{
	assert( jeActor_IsValid(A) != JE_FALSE);
	return A->UserData;
}

JETAPI void JETCC jeActor_SetUserData(jeActor *A, void *UserData)
{
	assert( jeActor_IsValid(A) != JE_FALSE);
	A->UserData = UserData;
}

#define MAX(aa,bb)   ( (aa)>(bb)?(aa):(bb) )
#define MIN(aa,bb)   ( (aa)<(bb)?(aa):(bb) )

static void JETCF jeActor_StretchBoundingBox( jeVec3d *Min, jeVec3d *Max,
							const jeVec3d *Corner, 
							const jeVec3d *DX, const jeVec3d *DY, const jeVec3d *DZ)
{
	jeVec3d P;

	P = *Corner;
	Min->X = MIN(Min->X,P.X),	Min->Y = MIN(Min->Y,P.Y),	Min->Z = MIN(Min->Z,P.Z);
	Max->X = MAX(Max->X,P.X);	Max->Y = MAX(Max->Y,P.Y);	Max->Z = MAX(Max->Z,P.Z);

	jeVec3d_Add     (Corner ,DX,&P);
	Min->X = MIN(Min->X,P.X);	Min->Y = MIN(Min->Y,P.Y);	Min->Z = MIN(Min->Z,P.Z);
	Max->X = MAX(Max->X,P.X);	Max->Y = MAX(Max->Y,P.Y);	Max->Z = MAX(Max->Z,P.Z);

	jeVec3d_Add     (&P, DZ,&P);
	Min->X = MIN(Min->X,P.X);	Min->Y = MIN(Min->Y,P.Y);	Min->Z = MIN(Min->Z,P.Z);
	Max->X = MAX(Max->X,P.X);	Max->Y = MAX(Max->Y,P.Y);	Max->Z = MAX(Max->Z,P.Z);

	jeVec3d_Subtract(&P,DX,&P);
	Min->X = MIN(Min->X,P.X);	Min->Y = MIN(Min->Y,P.Y);	Min->Z = MIN(Min->Z,P.Z);
	Max->X = MAX(Max->X,P.X);	Max->Y = MAX(Max->Y,P.Y);	Max->Z = MAX(Max->Z,P.Z);

	jeVec3d_Add     (&P,DY,&P);
	Min->X = MIN(Min->X,P.X);	Min->Y = MIN(Min->Y,P.Y);	Min->Z = MIN(Min->Z,P.Z);
	Max->X = MAX(Max->X,P.X);	Max->Y = MAX(Max->Y,P.Y);	Max->Z = MAX(Max->Z,P.Z);

	jeVec3d_Add     (&P,DX,&P);
	Min->X = MIN(Min->X,P.X);	Min->Y = MIN(Min->Y,P.Y);	Min->Z = MIN(Min->Z,P.Z);
	Max->X = MAX(Max->X,P.X);	Max->Y = MAX(Max->Y,P.Y);	Max->Z = MAX(Max->Z,P.Z);

	jeVec3d_Subtract(&P,DZ,&P);
	Min->X = MIN(Min->X,P.X);	Min->Y = MIN(Min->Y,P.Y);	Min->Z = MIN(Min->Z,P.Z);
	Max->X = MAX(Max->X,P.X);	Max->Y = MAX(Max->Y,P.Y);	Max->Z = MAX(Max->Z,P.Z);

	jeVec3d_Subtract(&P,DX,&P);
	Min->X = MIN(Min->X,P.X);	Min->Y = MIN(Min->Y,P.Y);	Min->Z = MIN(Min->Z,P.Z);
	Max->X = MAX(Max->X,P.X);	Max->Y = MAX(Max->Y,P.Y);	Max->Z = MAX(Max->Z,P.Z);
}

JETAPI jeBoolean JETCC jeActor_GetDynamicExtBox( const jeActor *A, jeExtBox *ExtBox)
{
#define JE_ACTOR_REALLY_BIG_NUMBER (9e9f)

	jeVec3d Corner;
	jeVec3d DX;
	jeVec3d DY;
	jeVec3d DZ;
	int Count,i,BCount;

	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( A->ActorDefinition->Body   != NULL );

	jeVec3d_Set(&(ExtBox->Min),
			JE_ACTOR_REALLY_BIG_NUMBER,JE_ACTOR_REALLY_BIG_NUMBER,JE_ACTOR_REALLY_BIG_NUMBER);
	jeVec3d_Set(&(ExtBox->Max),
			-JE_ACTOR_REALLY_BIG_NUMBER,-JE_ACTOR_REALLY_BIG_NUMBER,-JE_ACTOR_REALLY_BIG_NUMBER);
		
	BCount = 0;
	Count = jeBody_GetBoneCount( A->ActorDefinition->Body );
	for (i=0; i< Count; i++)
		{
			if (jeActor_GetBoneBoundingBoxByIndex(A,i,&Corner,&DX,&DY,&DZ)!=JE_FALSE)
				{
					jeActor_StretchBoundingBox( &(ExtBox->Min),
												&(ExtBox->Max),&Corner,&DX,&DY,&DZ);
					BCount ++;
				}
		}
	if (BCount>0)
		{
			return JE_TRUE;
		}
	return JE_FALSE;
}



JETAPI jeBoolean JETCC jeActor_Attach( jeActor *Slave,  const char *SlaveBoneName,
						const jeActor *Master, const char *MasterBoneName, 
						const jeXForm3d *Attachment)
{
	int SlaveBoneIndex,MasterBoneIndex;

	assert( jeActor_IsValid(Slave) != JE_FALSE);
	assert( jeActor_IsValid(Master) != JE_FALSE);
	assert( jeXForm3d_IsOrthonormal(Attachment) != JE_FALSE );
	
	assert( MasterBoneName != NULL );		// might this be possible?
	
	if (jeActor_GetBoneIndex(Slave,SlaveBoneName,&(SlaveBoneIndex))==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_Attach: Named bone for slave not found: ", SlaveBoneName);
			return JE_FALSE;
		}
	
	if (jeActor_GetBoneIndex(Master,MasterBoneName,&(MasterBoneIndex))==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"Named bone for master not found: ", MasterBoneName);
			return JE_FALSE;
		}
	
	return jePose_Attach(   Slave->Pose,   SlaveBoneIndex,
							Master->Pose, MasterBoneIndex, 
							Attachment);
}


JETAPI void JETCC jeActor_Detach(jeActor *A)
{
	assert( jeActor_IsValid(A) != JE_FALSE);

	jePose_Detach( A->Pose );
}


JETAPI jeBoolean JETCC jeActor_GetBoneAttachment(const jeActor *A,
								const char *BoneName,
								jeXForm3d *Attachment)
{

	int BoneIndex;
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( Attachment != NULL );
	
	if (jeActor_GetBoneIndex(A,BoneName,&(BoneIndex))==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_GetBoneAttachment: Named bone not found: ", BoneName);
			return JE_FALSE;
		}
	
	jePose_GetJointAttachment(A->Pose,BoneIndex, Attachment);
	assert ( jeXForm3d_IsOrthonormal(Attachment) != JE_FALSE );

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeActor_SetBoneAttachment(jeActor *A,
								const char *BoneName,
								jeXForm3d *Attachment)
{

	int BoneIndex;

	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( jeXForm3d_IsOrthonormal(Attachment) != JE_FALSE );
	
	if (jeActor_GetBoneIndex(A,BoneName,&(BoneIndex))==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_SetBoneAttachment: Named bone not found: ", BoneName);
			return JE_FALSE;
		}
	
	jePose_SetJointAttachment(A->Pose,BoneIndex, Attachment);
	return JE_TRUE;
}


//-------------------------------------------------------------------------------------------------
// Actor Cuing system
//-------------------------------------------------------------------------------------------------
#define ACTOR_CUE_MINIMUM_BLEND (0.0001f)
#define ACTOR_CUE_MAXIMUM_BLEND (0.999f)


static jeBoolean JETCF jeActor_IsAnimationCueDead(jeActor *A, int Index)
{
	jeBoolean Kill= JE_FALSE;
	jeFloat BlendAmount;
	jeMotion *M;
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( (Index>=0) && (Index<jeMotion_GetSubMotionCount(A->CueMotion)));

	M = A->CueMotion;

	BlendAmount = jeMotion_GetBlendAmount(M,Index,0.0f);
	if (BlendAmount <= ACTOR_CUE_MINIMUM_BLEND)
		{
			int KeyCount;
			jePath *P; 
			jeFloat KeyTime;

			P = jeMotion_GetBlendPath(M,Index);
			assert( P != NULL );
			KeyCount = jePath_GetKeyframeCount(P,JE_PATH_TRANSLATION_CHANNEL);
			if (KeyCount>0)
				{
					jeXForm3d Dummy;
					jeFloat TimeOffset = -jeMotion_GetTimeOffset( M, Index);
					jePath_GetKeyframe( P, KeyCount-1, JE_PATH_TRANSLATION_CHANNEL, &KeyTime, &Dummy );
	
					if ( KeyTime <= TimeOffset )
						{
							Kill = JE_TRUE;
						}
				}
			else
				{
					Kill = JE_TRUE;
				}
		}
	return Kill;
}


static void JETCF jeActor_KillCue( jeActor *A, int Index )
{
	jeMotion *M;
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( (Index>=0) && (Index<jeMotion_GetSubMotionCount(A->CueMotion)));
	M  = jeMotion_RemoveSubMotion(A->CueMotion,Index);
}

JETAPI jeBoolean JETCC jeActor_AnimationNudge(jeActor *A, jeXForm3d *Offset)
{
	jeMotion *M;
	int i,Count;
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( jeXForm3d_IsOrthonormal(Offset) != JE_FALSE );
	M = A->CueMotion;
	Count = jeMotion_GetSubMotionCount(M);
	
	for (i=Count-1; i>=0; i--)	
		{
			jeXForm3d Transform;
			const jeXForm3d *pTransform;
			pTransform = jeMotion_GetBaseTransform( M, i );
			if ( pTransform != NULL )
				{
					Transform = *pTransform;
			
					jeXForm3d_Multiply(Offset,&Transform,&Transform);
					jeXForm3d_Orthonormalize(&Transform);

					jeMotion_SetBaseTransform( M, i, &Transform);
				}
		}
	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeActor_AnimationRemoveLastCue( jeActor *A )
{
	int Count;
	assert( jeActor_IsValid(A) != JE_FALSE);
	Count = jeMotion_GetSubMotionCount(A->CueMotion);
	if (Count>0)
		{
			jeActor_KillCue( A, Count-1 );
			return JE_TRUE;
		}
	return JE_FALSE;
}

JETAPI jeBoolean JETCC jeActor_AnimationCue( jeActor *A, 
								jeMotion *Motion,
								jeFloat TimeScaleFactor,
								jeFloat TimeIntoMotion,
								jeFloat BlendTime, 
								jeFloat BlendFromAmount, 
								jeFloat BlendToAmount,
								const jeXForm3d *MotionTransform)
{
	int Index;

	assert( jeActor_IsValid(A) != JE_FALSE);

	assert( (BlendFromAmount>=0.0f) && (BlendFromAmount<=1.0f));
	assert( (  BlendToAmount>=0.0f) && (  BlendToAmount<=1.0f));
	assert( (MotionTransform==NULL) || (jeXForm3d_IsOrthonormal(MotionTransform)) != JE_FALSE );

	assert( Motion != NULL );
	
	assert( BlendTime >= 0.0f);
	if (BlendTime==0.0f)
		{
			BlendFromAmount = BlendToAmount;
			BlendTime = 1.0f;	// anything that is > JE_TKA_TIME_TOLERANCE
		}

	if (jeMotion_AddSubMotion( A->CueMotion, TimeScaleFactor, -TimeIntoMotion, Motion, 
							TimeIntoMotion, BlendFromAmount, 
							TimeIntoMotion + BlendTime, BlendToAmount, 
							MotionTransform, &Index )==JE_FALSE)
		{	
			return JE_FALSE;
		}
		
	return JE_TRUE;
}


JETAPI jeBoolean JETCC jeActor_AnimationStep(jeActor *A, jeFloat DeltaTime )
{
	int i,Coverage,Count;
	jeMotion *M;
	jeMotion *SubM;
	
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( DeltaTime >= 0.0f );
	
	jePose_ClearCoverage(A->Pose,0);

	M = A->CueMotion;

	Count = jeMotion_GetSubMotionCount(M);

	for (i=Count-1; i>=0; i--)	
		{
			jeFloat TimeOffset = jeMotion_GetTimeOffset( M, i );
			TimeOffset = TimeOffset - DeltaTime;
			jeMotion_SetTimeOffset( M, i, TimeOffset);

			if (jeActor_IsAnimationCueDead(A,i))
				{
					jeActor_KillCue(A,i);
				}
			else
				{
					jeBoolean SetWithBlending= JE_TRUE;
					jeFloat BlendAmount;
					
					SubM = jeMotion_GetSubMotion(M,i);
					assert( SubM != NULL );
					
					BlendAmount = jeMotion_GetBlendAmount( M,i,0.0f );
					
					if (BlendAmount >= ACTOR_CUE_MAXIMUM_BLEND)
						{
							SetWithBlending = JE_FALSE;
						}
					Coverage = jePose_AccumulateCoverage(A->Pose,SubM, SetWithBlending);
					if ( Coverage == 0 )
						{
							jeActor_KillCue(A,i);
						}
				}
		}

	jePose_SetMotion( A->Pose, M, 0.0f, NULL );
	jeMotion_SetupEventIterator(M,-DeltaTime,0.0f);

	return JE_TRUE;
}


JETAPI jeBoolean JETCC jeActor_AnimationStepBoneOptimized(jeActor *A, jeFloat DeltaTime, const char *BoneName )
{
	int i,Coverage,Count;
	jeMotion *M;
	jeMotion *SubM;
	
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( DeltaTime >= 0.0f );
	
	if (BoneName == NULL)
		{
			A->StepBoneIndex = JE_POSE_ROOT_JOINT;
		}
	else
		{
			jeBoolean LookupBoneName= JE_TRUE;
			const char *LastBoneName;
			jeXForm3d Attachment;
			int LastParentBoneIndex;
			if (A->StepBoneIndex >= 0)
				{
					jeBody_GetBone(	A->ActorDefinition->Body,A->StepBoneIndex,&LastBoneName,&Attachment,&LastParentBoneIndex);
					if (  (LastBoneName != NULL) )
						if (_stricmp(LastBoneName,BoneName)==0)  // Case insensitive compare -- Incarnadine
							LookupBoneName = JE_FALSE;
				}
			if (LookupBoneName != JE_FALSE)
				{
					if (jeActor_GetBoneIndex(A,BoneName,&(A->StepBoneIndex))==JE_FALSE)
						{
							jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_AnimationStepBoneOptimized: Named bone not found: ", BoneName);
							return JE_FALSE;
						}
				}
		}
			

	jePose_ClearCoverage(A->Pose,0);

	M = A->CueMotion;

	Count = jeMotion_GetSubMotionCount(M);

	for (i=Count-1; i>=0; i--)	
		{
			jeFloat TimeOffset = jeMotion_GetTimeOffset( M, i );
			TimeOffset = TimeOffset - DeltaTime;
			jeMotion_SetTimeOffset( M, i, TimeOffset);

			if (jeActor_IsAnimationCueDead(A,i))
				{
					jeActor_KillCue(A,i);
				}
			else
				{
					jeBoolean SetWithBlending= JE_TRUE;
					jeFloat BlendAmount;
					
					SubM = jeMotion_GetSubMotion(M,i);
					assert( SubM != NULL );
					
					BlendAmount = jeMotion_GetBlendAmount( M,i,0.0f );
					
					if (BlendAmount >= ACTOR_CUE_MAXIMUM_BLEND)
						{
							SetWithBlending = JE_FALSE;
						}
					Coverage = jePose_AccumulateCoverage(A->Pose,SubM, SetWithBlending);
					if ( Coverage == 0 )
						{
							jeActor_KillCue(A,i);
						}
				}
		}

	jePose_SetMotionForABone( A->Pose, M, 0.0f, NULL, A->StepBoneIndex );
	jeMotion_SetupEventIterator(M,-DeltaTime,0.0f);

	return JE_TRUE;
}


		
JETAPI jeBoolean JETCC jeActor_AnimationTestStep(jeActor *A, jeFloat DeltaTime)
{
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( DeltaTime >= 0.0f );

	jePose_SetMotion( A->Pose, A->CueMotion , DeltaTime, NULL );

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeActor_AnimationTestStepBoneOptimized(jeActor *A, jeFloat DeltaTime, const char *BoneName)
{
	assert( jeActor_IsValid(A) != JE_FALSE);
	assert( DeltaTime >= 0.0f );

	if (BoneName == NULL)
		{
			A->StepBoneIndex = JE_POSE_ROOT_JOINT;
		}
	else
		{
			jeBoolean LookupBoneName= JE_TRUE;
			const char *LastBoneName;
			jeXForm3d Attachment;
			int LastParentBoneIndex;
			if (A->StepBoneIndex >= 0)
				{
					jeBody_GetBone(	A->ActorDefinition->Body,A->StepBoneIndex,&LastBoneName,&Attachment,&LastParentBoneIndex);
					if (  (LastBoneName != NULL) )
						if (_stricmp(LastBoneName,BoneName)==0)  // Case insensitive compare -- Incarnadine
							LookupBoneName = JE_FALSE;
				}
			if (LookupBoneName != JE_FALSE)
				{
					if (jeActor_GetBoneIndex(A,BoneName,&(A->StepBoneIndex))==JE_FALSE)
						{
							jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_AnimationTestStepBoneOptimized: Named bone not found:", BoneName);
							return JE_FALSE;
						}
				}
		}
	jePose_SetMotionForABone( A->Pose, A->CueMotion , DeltaTime, NULL,A->StepBoneIndex );

	return JE_TRUE;
}



JETAPI jeBoolean JETCC jeActor_GetAnimationEvent(
	jeActor *A,						
	const char **ppEventString)		// Return data, if found
	// returns the event string for the 'next' event that occured during the last 
	// animation step time delta.
	// if the return value is JE_FALSE, there are no more events, and ppEventString will be Empty
{
	jeFloat Time;
	assert( jeActor_IsValid(A) != JE_FALSE);

	return jeMotion_GetNextEvent(A->CueMotion, &Time, ppEventString );
}

JETAPI jeBoolean JETCC jeActor_GetLightingOptions(const jeActor *Actor,
	jeBoolean *UseFillLight,
	jeVec3d *FillLightNormal,
	jeFloat *FillLightRed,				
	jeFloat *FillLightGreen,				
	jeFloat *FillLightBlue,				
	jeFloat *AmbientLightRed,			
	jeFloat *AmbientLightGreen,			
	jeFloat *AmbientLightBlue,			
	jeBoolean *UseAmbientLightFromFloor,
	int32 *MaximumDynamicLightsToUse,
	int32 *MaximumStaticLightsToUse,		
	const char **LightReferenceBoneName,
	jeBoolean *PerBoneLighting)
{
	int32 BoneIndex;
	assert( jeActor_IsValid(Actor)!=JE_FALSE );

	assert( UseFillLight != NULL );
	assert( FillLightNormal != NULL );
	assert( FillLightRed != NULL );	
	assert( FillLightGreen != NULL );	
	assert( FillLightBlue != NULL );	
	assert( AmbientLightRed != NULL );
	assert( AmbientLightGreen != NULL );			
	assert( AmbientLightBlue != NULL );			
	assert( UseAmbientLightFromFloor != NULL );
	assert( MaximumDynamicLightsToUse != NULL );	
	assert( LightReferenceBoneName != NULL );

	assert( Actor->Puppet );
	
	jePuppet_GetLightingOptions(	Actor->Puppet,
									UseFillLight,
									FillLightNormal,
									FillLightRed,	
									FillLightGreen,	
									FillLightBlue,	
									AmbientLightRed,
									AmbientLightGreen,		
									AmbientLightBlue,		
									UseAmbientLightFromFloor,
									MaximumDynamicLightsToUse,
									MaximumStaticLightsToUse,
									&BoneIndex,
									PerBoneLighting);

	if (BoneIndex>=0 && (BoneIndex < jeBody_GetBoneCount(Actor->ActorDefinition->Body)))
		{
			jeXForm3d DummyAttachment;
			int DummyParentBoneIndex;
			jeBody_GetBone(	Actor->ActorDefinition->Body,
							BoneIndex,
							LightReferenceBoneName,
							&DummyAttachment,
							&DummyParentBoneIndex);
		}
	else
		{
			LightReferenceBoneName = NULL;
		}

	return JE_TRUE; // CB
}

JETAPI jeBoolean JETCC jeActor_SetLightingOptions(jeActor *A,
	jeBoolean UseFillLight,
	const jeVec3d *FillLightNormal,
	jeFloat FillLightRed,				// 0 .. 255
	jeFloat FillLightGreen,				// 0 .. 255
	jeFloat FillLightBlue,				// 0 .. 255
	jeFloat AmbientLightRed,			// 0 .. 255
	jeFloat AmbientLightGreen,			// 0 .. 255
	jeFloat AmbientLightBlue,			// 0 .. 255
	jeBoolean AmbientLightFromFloor,
	int32 MaximumDynamicLightsToUse,		// 0 for none
	int32 MaximumStaticLightsToUse, // 0 for none
	const char *LightReferenceBoneName,
	jeBoolean PerBoneLighting	)
{
	int BoneIndex;

	assert( jeActor_IsValid(A)!=JE_FALSE );
	assert( FillLightNormal != NULL );
	assert( A->Puppet );

	if (LightReferenceBoneName && strcmp(LightReferenceBoneName, "< none >") == 0)
	{
		jeActor_GetBoneIndex(A,NULL,&BoneIndex);
	}
	else
	if (jeActor_GetBoneIndex(A,LightReferenceBoneName,&BoneIndex)==JE_FALSE)
	{
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_SetLightingOptions: Named bone for light reference not found: ", LightReferenceBoneName);
		return JE_FALSE;
	}
	if (!jeVec3d_IsNormalized(FillLightNormal)) 
	{
		jeVec3d_Normalize((jeVec3d*)FillLightNormal);
	}

	jePuppet_SetLightingOptions(	A->Puppet,
									UseFillLight,
									FillLightNormal,
									FillLightRed,	
									FillLightGreen,	
									FillLightBlue,	
									AmbientLightRed,
									AmbientLightGreen,		
									AmbientLightBlue,		
									AmbientLightFromFloor,
									MaximumDynamicLightsToUse,
									MaximumStaticLightsToUse,
									BoneIndex,
									PerBoneLighting);
	return JE_TRUE;
}

JETAPI void JETCC jeActor_SetScale(jeActor *A, jeFloat ScaleX,jeFloat ScaleY,jeFloat ScaleZ)
{
	jeVec3d S;
	assert( A != NULL );
		
	jeVec3d_Set(&S,ScaleX,ScaleY,ScaleZ);
	jePose_SetScale(A->Pose,&S);

	A->needsRelighting = JE_TRUE;
}



JETAPI jeBoolean JETCC jeActor_SetShadow(jeActor *A, 
		jeBoolean DoShadow, 
		jeFloat Radius,
		const jeMaterialSpec *ShadowMap,
		const char *BoneName)
{
	int BoneIndex;

	assert( jeActor_IsValid(A)!=JE_FALSE );
	assert( (DoShadow==JE_FALSE) || (DoShadow==JE_TRUE));
	assert( Radius >= 0.0f);
	assert( A->Puppet );
	
	if (jeActor_GetBoneIndex(A,BoneName,&BoneIndex)==JE_FALSE)
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_SetShadow: Named bone for shadow not found: ", BoneName);
			return JE_FALSE;
		}

	jePuppet_SetShadow(A->Puppet,DoShadow,Radius,ShadowMap,BoneIndex);

	return JE_TRUE;
}


JETAPI jeBoolean JETCC jeActor_AttachEngine(jeActor *A, jeEngine *pEngine)
{
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert( jeActor_DefIsValid(A->ActorDefinition) != JE_FALSE );
	assert( jeBody_IsValid(A->ActorDefinition->Body) != JE_FALSE );
	assert(pEngine);


	if (A->Puppet!=NULL)
	{
		jeEngine* pe = jePuppet_GetEngine(A->Puppet);
		if (pEngine == pe) {
			OutputDebugString("No Attach engine twice\n");
			return JE_TRUE;
		}
		jePuppet_Destroy(&(A->Puppet));
		A->Puppet =NULL;
	}
		
	A->Puppet = jePuppet_Create(A->ActorDefinition->TextureFileContext, A->ActorDefinition->Body, pEngine);

	if ( A->Puppet == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeActor_AttachEngine: failed to create puppet");
		return JE_FALSE;
	}
	
	// Get box of puppet
	{
		jeExtBox EB;

		if (jeActor_GetBoneExtBoxByIndex(A,JE_POSE_ROOT_JOINT,&EB) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeActor_AttachEngine: failed to get root extent box");
			return JE_FALSE;			
		}
		
		A->CollisionExtBox.Min = EB.Min;
		A->CollisionExtBox.Max = EB.Max;
	}

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeActor_DetachEngine(jeActor *A, jeEngine *pEngine)
{
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert( jeActor_DefIsValid(A->ActorDefinition) != JE_FALSE );
	assert( jeBody_IsValid(A->ActorDefinition->Body) != JE_FALSE );

	assert(pEngine);

	assert(A->Puppet);

	jePuppet_Destroy(&A->Puppet);

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeActor_RenderThroughFrustum(
		const jeActor	*A, 
		jeEngine		*Engine, 
		jeWorld			*World, 
		jeCamera		*Camera, 
		const jeFrustum *Frustum)
{
	jeExtBox	Box;
	jeBoolean	Enabled;
		
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert( A->Puppet != NULL );

//	tom morris feb 2005
//	this switch commented out because it prevents rendering of actors in jDesigner3d
	if (!A->RenderNextTime) {
//		Log_Printf("KROUER: jeActor_RenderThroughFrustum: Actor in invisible area");
		return JE_TRUE;
	}
//	end tom morris feb 2005

	if (A->RenderHintExtBoxEnabled)
	{
		if (jeActor_GetRenderHintExtBox(A, &Box, &Enabled)==JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeActor_RenderThroughFrustum: Failed to get render hint box");
			return JE_FALSE;
		}
	}
	else
	{
		if (!jeActor_GetDynamicExtBox( A, &Box))
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeActor_RenderThroughFrustum: Failed to get dynamic ext box.");
			return JE_FALSE;
		}
	}

	if (A->needsRelighting)
	{
		if (jePuppet_RenderThroughFrustum( A->Puppet, A->Pose, &Box, Engine, World, Camera, Frustum, JE_TRUE)==JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeActor_RenderThroughFrustum: Failed to render puppet");
			return JE_FALSE;
		}

		((jeActor *)A)->needsRelighting = JE_FALSE;
	}
	else
	{
		if (jePuppet_RenderThroughFrustum( A->Puppet, A->Pose, &Box, Engine, World, Camera, Frustum, JE_FALSE)==JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeActor_RenderThroughFrustum: Failed to render puppet");
			return JE_FALSE;
		}
	}

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeActor_Render(
		const jeActor	*A, 
		jeEngine		*Engine, 
		jeWorld			*World, 
		const jeCamera	*Camera)
{
	jeExtBox Box;
	jeExtBox *pBox = &Box;
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert( A->Puppet != NULL );


//	tom morris feb 2005
//	this switch commented out because it prevents rendering of actors in jDesigner3d
	if (!A->RenderNextTime) {
//		//Log_Printf("KROUER: jeActor_Render: Actor in invisible area\n");
		return JE_TRUE;
	}
//	end tom morris feb 2005

	if (A->RenderHintExtBoxEnabled)
	{
		jeBoolean Enabled;
		if (jeActor_GetRenderHintExtBox(A, pBox, &Enabled)==JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeActor_Render: failed to get render hint box");	
			return JE_FALSE;
		}
	}
	else
		pBox = NULL;


	if (A->needsRelighting)
	{	
		if (jePuppet_Render( A->Puppet, A->Pose, Engine, World, Camera, pBox, JE_TRUE)==JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeActor_Render: failed to render puppet");
			return JE_FALSE;
		}

		((jeActor *)A)->needsRelighting = JE_FALSE;
	}

	else
	{
		if (jePuppet_Render( A->Puppet, A->Pose, Engine, World, Camera, pBox, JE_FALSE)==JE_FALSE)
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeActor_Render: failed to render puppet");
			return JE_FALSE;
		}
	}

	return JE_TRUE;
}

// force the actor to be re-lit with static lighting
JETAPI void JETCC jeActor_ForceStaticRelighting(jeActor* pActor)
{
	assert(jeActor_IsValid(pActor) != JE_FALSE);

	pActor->needsRelighting = JE_TRUE;
}


JETAPI int32 JETCC jeActor_GetMaterialCount(const jeActor *A)
{
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert( A->Puppet != NULL );

	return jePuppet_GetMaterialCount( A->Puppet );
}

JETAPI jeBoolean JETCC jeActor_GetMaterial(const jeActor *A, int32 MaterialIndex,
										jeMaterialSpec **Bitmap, jeFloat *Red, jeFloat *Green, jeFloat *Blue, jeUVMapper* pMapper)
{
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert( A->Puppet != NULL );

	return jePuppet_GetMaterial(A->Puppet, MaterialIndex, Bitmap, Red, Green, Blue, pMapper);
}


JETAPI jeBoolean JETCC jeActor_SetMaterial(jeActor *A, int32 MaterialIndex,
										jeMaterialSpec *Bitmap,  jeFloat Red,  jeFloat Green,  jeFloat Blue, jeUVMapper Mapper)
{
	assert( jeActor_IsValid(A) != JE_FALSE );
	assert( A->Puppet != NULL );

	return jePuppet_SetMaterial(A->Puppet,MaterialIndex, Bitmap, Red, Green, Blue, Mapper);
}

///////////////////////////////////////////////////////////////////////////////////////
// exposed geometry APIs

JETAPI jeBoolean JETCC jeActor_GetIndexedBoneWorldSpaceVertexLocations(const jeActor* pActor, int32 boneIndex, int32 aSize,
	jeVec3d* pVerts)
{
	int i;
	jeXForm3d xform;

	if (! jeBody_GetIndexedBoneVertexLocations(pActor->ActorDefinition->Body, boneIndex, aSize, pVerts))
		return JE_FALSE;

	jePose_GetJointTransform(pActor->Pose, boneIndex, &xform);

	for (i = 0; i < aSize; i ++)
	{
		jeXForm3d_Transform(&xform, &pVerts[i], &pVerts[i]);
	}

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeActor_GetNamedBoneWorldSpaceVertexLocations(const jeActor* pActor, const char* pBoneName, int32 aSize,
	jeVec3d* pVerts)
{
	int i;
	jeXForm3d xform;

	if (! jeBody_GetNamedBoneVertexLocations(pActor->ActorDefinition->Body, pBoneName, aSize, pVerts))
		return JE_FALSE;

	jeActor_GetBoneTransform(pActor, pBoneName, &xform);

	for (i = 0; i < aSize; i ++)
	{
		jeXForm3d_Transform(&xform, &pVerts[i], &pVerts[i]);
	}

	return JE_TRUE;
}

//========================================================================================
// --- Incarnadine: Begin Actor Collision ---

//========================================================================================
//	jeActor_Collision
//      Tests to see if a Box moving along a path from Front to Back collides with the Actor.
//  The Actor's bounding box must have previously been set and Box must be relative to
//  the path (meaning not in world-space coordinates).
//
//  Bonelevel collision is set is specified, a bone level
//  collision is done based on bones that have been added with AddCollisionBone().
//  CollidedBone gets set if there is a collision.
//========================================================================================
JETAPI jeBoolean JETCC jeActor_Collision( jeActor	*Actor, const jeWorld* World, const jeExtBox	*Box, 
		const jeVec3d *Front, const jeVec3d *Back, jeCollisionInfo *CollisionInfo)
{
	jeExtBox ActorBox, BoneBox, xSweepBox;
	jeFloat T, BestT;
	jeVec3d Normal, BestNormal, Correction;
	char *BoneName = NULL, *BestBoneName = NULL;	

	assert(Actor != NULL);
	assert(Front != NULL);
	assert(Back != NULL);
		
	// Get actor box and make sure it's valid
	jeActor_GetExtBox(Actor,&ActorBox);
	if(!jeExtBox_IsValid(&ActorBox)) return JE_FALSE;

	if(jeWorld_GetCollisionLevel(World) != COLLIDE_BONES  || jeActor_GetNextCollisionBone(Actor, NULL) == NULL)
	{
		// Not bone level collision
		if (CollisionInfo)
		{
			if(jeExtBox_Collision(&ActorBox, Box, Front, Back, &T, &Normal))
			{
				// Get path vector
				jeVec3d_Subtract(Back, Front, &CollisionInfo->Impact);

				// Impact is too precise, back off a little
				Correction = CollisionInfo->Impact;
				jeVec3d_Normalize(&Correction);
				jeVec3d_Scale(&Correction,-1.5f,&Correction);

				// Scale the path vector based on impact
				jeVec3d_Scale(&CollisionInfo->Impact, T, &CollisionInfo->Impact);

				// Calculate the plane info from this
				CollisionInfo->Plane.Dist = jeVec3d_Length(&CollisionInfo->Impact);
				CollisionInfo->Plane.Normal = Normal;
				CollisionInfo->Plane.Type = Type_Any;

				// Get the true impact point by adding the start position and correction
				jeVec3d_Add(Front, &CollisionInfo->Impact, &CollisionInfo->Impact);	
				jeVec3d_Add(&Correction, &CollisionInfo->Impact, &CollisionInfo->Impact);	

				CollisionInfo->IsValid = JE_TRUE;
				jeActor_SetCollidedBone(Actor,NULL);
		
				return JE_TRUE;
			}
		} else
			return jeExtBox_Collision(&ActorBox, Box, Front, Back, NULL, NULL);
	}
	else
	{
		// Verify the box sweep intersects the actor box		
		jeExtBox_LinearSweep(Box, Front, Back, &xSweepBox);
		if(!jeExtBox_Intersection(&ActorBox, &xSweepBox, NULL)) return JE_FALSE;

		// Bone level collision
		jeActor_RecalcCollisionBones(Actor);
		BoneName = NULL;				
		while ( (BoneName = jeActor_GetNextCollisionBoneWithExtBoxes(Actor, BoneName, NULL, &BoneBox)) != NULL )
		{
			assert(jeExtBox_IsValid( &BoneBox) && !jeExtBox_IsPoint(&BoneBox));
			if(CollisionInfo)
			{
				if(jeExtBox_Collision(&BoneBox, Box, Front, Back, &T, &Normal))
				{
					if(BestBoneName == NULL || (BestBoneName != NULL && T < BestT))
					{
						BestBoneName = BoneName;
						BestT = T;
						BestNormal = Normal;
					}
				}
			} else
				if(jeExtBox_Collision(&BoneBox, Box, Front, Back, NULL, NULL))
					return JE_TRUE;
		}		

		if(BestBoneName != NULL)
		{
			T = BestT;
			Normal = BestNormal;

			// Get path vector
			jeVec3d_Subtract(Back, Front, &CollisionInfo->Impact);

			// Impact is too precise, back off a little
			Correction = CollisionInfo->Impact;
			jeVec3d_Normalize(&Correction);
			jeVec3d_Scale(&Correction,-1.5f,&Correction);

			// Scale the path vector based on impact
			jeVec3d_Scale(&CollisionInfo->Impact, T, &CollisionInfo->Impact);

			// Calculate the plane info from this
			CollisionInfo->Plane.Dist = jeVec3d_Length(&CollisionInfo->Impact);
			CollisionInfo->Plane.Normal = Normal;
			CollisionInfo->Plane.Type = Type_Any;

			// Get the true impact point by adding the start position and correction
			jeVec3d_Add(Front, &CollisionInfo->Impact, &CollisionInfo->Impact);	
			jeVec3d_Add(&Correction, &CollisionInfo->Impact, &CollisionInfo->Impact);	
			CollisionInfo->IsValid = JE_TRUE;

			jeActor_SetCollidedBone(Actor,BestBoneName);

			return JE_TRUE;
		}		
	}

	return JE_FALSE;
}

// Added by Icestorm
//========================================================================================
//	jeActor_ChangeBoxCollision
//      Tests to see if a Box changing from FrontBox to BackBox collides with the Actor.
//  The Actor's bounding box must have previously been set and boxes must be relative to
//  the path (meaning not in world-space coordinates).
//
//  Bonelevel collision is set is specified, a bone level
//  collision is done based on bones that have been added with AddCollisionBone().
//  CollidedBone gets set if there is a collision.
//========================================================================================
JETAPI jeBoolean JETCC jeActor_ChangeBoxCollision( jeActor	*Actor, const jeWorld *World, const jeVec3d *Pos, const jeExtBox	*FrontBox,
	const jeExtBox	*BackBox, jeChangeBoxCollisionInfo *CollisionInfo)
{
	jeExtBox ActorBox, BoneBox, xChangeBox;
	jeFloat T, BestT;
	jeVec3d Normal, BestNormal, Impact, BestImpact;
	char *BoneName = NULL, *BestBoneName = NULL;	

	assert(Actor != NULL);
	assert(FrontBox != NULL);
	assert(BackBox != NULL);
	assert(Pos != NULL);
	
	// Get actor box and make sure it's valid
	jeActor_GetExtBox(Actor,&ActorBox);
	if(!jeExtBox_IsValid(&ActorBox)||jeExtBox_IsPoint(&ActorBox)) return JE_FALSE;

	if(jeWorld_GetCollisionLevel(World) != COLLIDE_BONES  || jeActor_GetNextCollisionBone(Actor, NULL) == NULL)
	{
		// Not bone level collision
		if (CollisionInfo)
		{
			if(jeExtBox_ChangeBoxCollision(&ActorBox, Pos, FrontBox, BackBox, &T, &Normal, &Impact))
			{
				CollisionInfo->ImpactBox.Min.X=FrontBox->Min.X+T*(BackBox->Min.X-FrontBox->Min.X);
				CollisionInfo->ImpactBox.Min.Y=FrontBox->Min.Y+T*(BackBox->Min.Y-FrontBox->Min.Y);
				CollisionInfo->ImpactBox.Min.Z=FrontBox->Min.Z+T*(BackBox->Min.Z-FrontBox->Min.Z);

				CollisionInfo->ImpactBox.Max.X=FrontBox->Max.X+T*(BackBox->Max.X-FrontBox->Max.X);
				CollisionInfo->ImpactBox.Max.Y=FrontBox->Max.Y+T*(BackBox->Max.Y-FrontBox->Max.Y);
				CollisionInfo->ImpactBox.Max.Z=FrontBox->Max.Z+T*(BackBox->Max.Z-FrontBox->Max.Z);

				jeActor_SetCollidedBone(Actor,NULL);

				CollisionInfo->Plane.Normal=Normal;
				CollisionInfo->Plane.Type=Type_Any;
				CollisionInfo->Plane.Dist=jeVec3d_DotProduct(&Normal,&Impact)+1.5f;
		
				return JE_TRUE;
			}
		} else
			return jeExtBox_ChangeBoxCollision(&ActorBox, Pos, FrontBox, BackBox, NULL, NULL, NULL);
	}
	else
	{
		// Verify the box "sweep" intersects the actor box		
		jeExtBox_Union(FrontBox,BackBox,&xChangeBox);
		if(!jeExtBox_Intersection(&ActorBox, &xChangeBox, NULL)) return JE_FALSE;

		// Bone level collision
		jeActor_RecalcCollisionBones(Actor);
		BoneName = NULL;				
		while ( (BoneName = jeActor_GetNextCollisionBoneWithExtBoxes(Actor, BoneName, NULL, &BoneBox)) != NULL )
		{
			assert(jeExtBox_IsValid(&BoneBox) && !jeExtBox_IsPoint(&BoneBox));
			if(jeExtBox_ChangeBoxCollision(&BoneBox, Pos, FrontBox, BackBox, &T, &Impact, &Normal))
			{
				if(BestBoneName == NULL || (BestBoneName != NULL && T < BestT))
				{
					BestBoneName = BoneName;
					BestT = T;
					BestNormal = Normal;
					BestImpact = Impact;
				}
			} else
				if (jeExtBox_ChangeBoxCollision(&BoneBox, Pos, FrontBox, BackBox, NULL, NULL, NULL))
					return JE_TRUE;
		}		

		if(BestBoneName != NULL)
		{
			T = BestT;
			Normal = BestNormal;

			CollisionInfo->ImpactBox.Min.X=FrontBox->Min.X+T*(BackBox->Min.X-FrontBox->Min.X);
			CollisionInfo->ImpactBox.Min.Y=FrontBox->Min.Y+T*(BackBox->Min.Y-FrontBox->Min.Y);
			CollisionInfo->ImpactBox.Min.Z=FrontBox->Min.Z+T*(BackBox->Min.Z-FrontBox->Min.Z);

			CollisionInfo->ImpactBox.Max.X=FrontBox->Max.X+T*(BackBox->Max.X-FrontBox->Max.X);
			CollisionInfo->ImpactBox.Max.Y=FrontBox->Max.Y+T*(BackBox->Max.Y-FrontBox->Max.Y);
			CollisionInfo->ImpactBox.Max.Z=FrontBox->Max.Z+T*(BackBox->Max.Z-FrontBox->Max.Z);

			jeActor_SetCollidedBone(Actor,BestBoneName);

			CollisionInfo->Plane.Normal=Normal;
			CollisionInfo->Plane.Type=Type_Any;
			CollisionInfo->Plane.Dist=jeVec3d_DotProduct(&Normal,&BestImpact)+1.5f;

			return JE_TRUE;
		}		
	}

	return JE_FALSE;
}

//========================================================================================
//	jeActor_AddCollisionBone
//========================================================================================
JETAPI void JETCC jeActor_AddCollisionBone(jeActor *Actor, const char* BoneName)
{
	jeExtBox BExtBox;
	assert(Actor != NULL);
	assert(Actor->BoneCollisionChain != NULL);
	assert(BoneName != NULL);

	if(!jeActor_HasCollisionBone(Actor,BoneName))
		//Icestorm: Better don't add non-valid-collision bones ;)
		if(jeActor_GetBoneExtBox(Actor,BoneName,&BExtBox)&&
			jeExtBox_IsValid(&BExtBox) && !jeExtBox_IsPoint(&BExtBox))
		{
			// Icestorm: Save ExtBox
			jeCollisionBone		*Bone;

			Bone=JE_RAM_ALLOCATE_STRUCT(jeCollisionBone);
			Bone->BoneName=strdup(BoneName);
			jeActor_GetBoneIndex(Actor, BoneName, &(Bone->BoneIndex) );
			*(Bone->CurrExtBox=JE_RAM_ALLOCATE_STRUCT(jeExtBox))=BExtBox;
			*(Bone->PrevExtBox=JE_RAM_ALLOCATE_STRUCT(jeExtBox))=BExtBox;
			jeChain_AddLinkData(Actor->BoneCollisionChain, (void*)(Bone));	
		}
}

//========================================================================================
//	jeActor_RemoveCollisionBone
//========================================================================================
JETAPI void JETCC jeActor_RemoveCollisionBone(jeActor *Actor, const char* BoneName)
{
	jeChain_Link *Link;	

	assert(Actor != NULL);
	assert(Actor->BoneCollisionChain != NULL);
	assert(BoneName != NULL);
	
	for (Link = jeChain_GetFirstLink(Actor->BoneCollisionChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		// locals
		jeCollisionBone		*LinkBone;
			
		LinkBone = (jeCollisionBone*)jeChain_LinkGetLinkData( Link );		
		if(LinkBone && _stricmp(BoneName,LinkBone->BoneName)==0)
		{
			// Icestorm: Free ExtBoxes (& Name)
			if (Link==Actor->LastUsedCollisionBone)
			{
				Actor->LastUsedCollisionBone=NULL;
				Actor->LastUsedCollisionBoneName=NULL;
			}
			free(LinkBone->BoneName);
			jeRam_Free(LinkBone->CurrExtBox);
			jeRam_Free(LinkBone->PrevExtBox);
			jeRam_Free(LinkBone);
			jeChain_RemoveLink(Actor->BoneCollisionChain, Link);
			return;
		}
	}				
}

//========================================================================================
//	jeActor_HasCollisionBone
//========================================================================================
JETAPI jeBoolean JETCC jeActor_HasCollisionBone(const jeActor *Actor, const char* BoneName)
{
	jeChain_Link *Link;	

	assert(Actor != NULL);
	assert(Actor->BoneCollisionChain != NULL);
	assert(BoneName != NULL);
	
	for (Link = jeChain_GetFirstLink(Actor->BoneCollisionChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		// locals
		jeCollisionBone		*LinkBone;
			
		LinkBone = (jeCollisionBone *)jeChain_LinkGetLinkData( Link );		
		if(LinkBone && _stricmp(BoneName,LinkBone->BoneName)==0)
			return JE_TRUE;
	}

	return JE_FALSE;
}

//========================================================================================
//	jeActor_GetNextCollisionBone ( rewritten by Icestorm [hybrid:jeChain_GetNextLinkData] )
//========================================================================================
JETAPI char* JETCC jeActor_GetNextCollisionBone(jeActor *Actor, char *BoneName)
{
	jeChain_Link *Link;	
	jeCollisionBone		*LinkBone=NULL;

	assert(Actor != NULL);
	assert(Actor->BoneCollisionChain != NULL);
	
	if(BoneName == NULL)
		Link = jeChain_GetFirstLink(Actor->BoneCollisionChain);
	else
	// Icestorm: _stricmp would be more general, but for lin. search this is better ;)
	if(BoneName==Actor->LastUsedCollisionBoneName)
		Link = jeChain_LinkGetNext(Actor->LastUsedCollisionBone);
	else
		for (Link = jeChain_GetFirstLink(Actor->BoneCollisionChain); Link; Link = jeChain_LinkGetNext(Link))
		{
			// locals
			
			LinkBone = (jeCollisionBone *)jeChain_LinkGetLinkData( Link );		
			if(LinkBone && _stricmp(BoneName,LinkBone->BoneName)==0)
			{
				Link = jeChain_LinkGetNext(Link);
				break;
			}
		}
	
	if (Link)
	{
		LinkBone=(jeCollisionBone*)jeChain_LinkGetLinkData(Link);
		Actor->LastUsedCollisionBone=Link;
		Actor->LastUsedCollisionBoneName=LinkBone->BoneName;
		return(LinkBone->BoneName);
	}
	else
		return NULL;
}

//========================================================================================
//	jeActor_GetNextCollisionBoneWithExtBoxes - By Icestorm
//========================================================================================
JETAPI char* JETCC jeActor_GetNextCollisionBoneWithExtBoxes(	jeActor		*Actor,
																char		*BoneName,
																jeExtBox	*PrevBox,
																jeExtBox	*CurrBox)
{
	jeChain_Link *Link;	
	jeCollisionBone		*LinkBone=NULL;

	assert(Actor != NULL);
	assert(Actor->BoneCollisionChain != NULL);
	
	if(BoneName == NULL)
		Link = jeChain_GetFirstLink(Actor->BoneCollisionChain);
	else
	// Icestorm: _stricmp would be more general, but for lin. search this is better ;)
	if(BoneName==Actor->LastUsedCollisionBoneName)
		Link = jeChain_LinkGetNext(Actor->LastUsedCollisionBone);
	else
		for (Link = jeChain_GetFirstLink(Actor->BoneCollisionChain); Link; Link = jeChain_LinkGetNext(Link))
		{
			LinkBone = (jeCollisionBone *)jeChain_LinkGetLinkData( Link );		
			if(LinkBone && _stricmp(BoneName,LinkBone->BoneName)==0)
			{
				Link = jeChain_LinkGetNext(Link);
				break;
			}
		}
	
	if (Link)
	{
		LinkBone=(jeCollisionBone*)jeChain_LinkGetLinkData(Link);
		Actor->LastUsedCollisionBone=Link;
		Actor->LastUsedCollisionBoneName=LinkBone->BoneName;
		if (PrevBox) *PrevBox=*(LinkBone->PrevExtBox);
		if (CurrBox) *CurrBox=*(LinkBone->CurrExtBox);
		return(LinkBone->BoneName);
	}
	else
		return NULL;
}

//========================================================================================
//	jeActor_RecalcCollisionBones - By Icestorm
//========================================================================================
JETAPI void JETCC jeActor_RecalcCollisionBones(jeActor *Actor)
{
	jeChain_Link		*Link;	
	jeCollisionBone		*LinkBone=NULL;
	jeExtBox			*BoneBox;

	assert(Actor != NULL);
	assert(Actor->BoneCollisionChain != NULL);
	
	for (Link = jeChain_GetFirstLink(Actor->BoneCollisionChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		LinkBone = (jeCollisionBone *)jeChain_LinkGetLinkData( Link );		
		if (LinkBone)
		{
			BoneBox=LinkBone->PrevExtBox;
			LinkBone->PrevExtBox=LinkBone->CurrExtBox;
			LinkBone->CurrExtBox=BoneBox;
			#ifndef NDEBUG
				assert(jeActor_GetBoneExtBoxByIndex(Actor, LinkBone->BoneIndex, BoneBox));
			#else
				jeActor_GetBoneExtBoxByIndex(Actor, LinkBone->BoneIndex, BoneBox);
			#endif
		}
	}
}

//========================================================================================
//	jeActor_MoveCollision - By Incarnadine
//========================================================================================
#define LARGE_NUMBER 9999999.0f

// This function checks an Actor to see if it collides with other Actors in the world or
// with the world itself.  The current Actor position is irrelevant as it assumes the actor
// is moving on a path from Front to Back.  The actor's collision box must have previously
// been properly set.
// (Modified by Icestorm)
JETAPI jeBoolean JETCC jeActor_MoveCollision(	jeActor *Actor,
											const jeWorld *World, 
											const jeVec3d *Front, 
											const jeVec3d *Back, 											
											jeCollisionInfo *CollisionInfo)
{
	char *BoneName;
	jeExtBox ActorBox, BoneBox;
	jeVec3d vActorBoxPos;
	jeXForm3d ActorTransform;	
	jeCollisionInfo WorldCollisionInfo;
	jeFloat BestDist = LARGE_NUMBER, Dist;	
	jeBoolean Hit = JE_FALSE;
	int32 iContents;
	jeObject *Obj;

	// Get the name of the first collision bone (if there is one)
	BoneName = jeActor_GetNextCollisionBone(Actor, NULL);	

	// Get the actor's bounding box
	jeActor_GetExtBox(Actor,&ActorBox);

	// We need the actor's box to be relative to the path Front-->Back
	jeExtBox_GetTranslation(&ActorBox,&vActorBoxPos);  // Get box pos (world)
	jeActor_GetBoneTransform(Actor, NULL, &ActorTransform); // Get actor pos (world)
	jeVec3d_Subtract(&vActorBoxPos, &ActorTransform.Translation, &vActorBoxPos); // New box pos (relative)
	jeExtBox_SetTranslation(&ActorBox,&vActorBoxPos); // Set the box to relative pos

	// Make the actor empty so it doesn't collide with itself	
	Obj = NULL;	
	while( ( Obj = jeWorld_GetNextObject((jeWorld*)World, Obj) ) != NULL)
		if(jeObject_GetInstance(Obj) == Actor) break;
		
	if(Obj != NULL)
	{
		iContents = jeObject_GetContents(Obj);
		jeObject_SetContents(Obj, CONTENTS_EMPTY);
	}

	if( jeWorld_GetCollisionLevel(World) != COLLIDE_BONES || BoneName == NULL )
	{
		if(jeWorld_Collision(World, &ActorBox, Front, Back, CollisionInfo))
			Hit = JE_TRUE;
	}
	else
	{				
		BestDist = LARGE_NUMBER;

		jeActor_RecalcCollisionBones(Actor);
		jeActor_GetBoneTransform(Actor, NULL, &ActorTransform); // Get actor pos (world)

		BoneName = NULL;			

		// Get the name of the first collision bone (if there is one)
		while( (BoneName = jeActor_GetNextCollisionBoneWithExtBoxes(Actor, BoneName, NULL, &BoneBox)) != NULL )
		{
			assert(jeExtBox_IsValid(&BoneBox) && !jeExtBox_IsPoint(&BoneBox));
			jeExtBox_SetNewOrigin(&BoneBox, &ActorTransform.Translation);

			if (CollisionInfo)
			{
				if(jeWorld_Collision(World, &BoneBox, Front, Back, &WorldCollisionInfo))
				{
					Dist = jeVec3d_DistanceBetween(Front, &WorldCollisionInfo.Impact);

					if(Dist < BestDist)
					{
						BestDist = Dist;
						*CollisionInfo = WorldCollisionInfo;
						jeActor_SetCollidedBone(Actor,BoneName);
						Hit = JE_TRUE;
					}
				}
			} else
				if(jeWorld_Collision(World, &BoneBox, Front, Back, NULL))
				{
					// Restore actor contents
					if(Obj != NULL)
						jeObject_SetContents(Obj, iContents);

					return JE_TRUE;
				}
		} 
	} 

	// Restore actor contents
	if(Obj != NULL)
		jeObject_SetContents(Obj, iContents);

	return Hit;
}

//Icestorm: Need this for pushing a growing extbox
static void MoveBoxForPlaneOut(const jeVec3d *Pos, const jeExtBox *Box, const jePlane *Plane, jeVec3d *MoveVec)
{
	jeVec3d		Normal;
	jeFloat		Dist;

	Normal = Plane->Normal;
	Dist=0.0f;
	
	if (Normal.X < 0)
		Dist += Normal.X * Box->Max.X;
	else	 
		Dist += Normal.X * Box->Min.X;
	
	if (Normal.Y < 0)
		Dist += Normal.Y * Box->Max.Y;
	else
		Dist += Normal.Y * Box->Min.Y;

	if (Normal.Z < 0)
		Dist += Normal.Z * Box->Max.Z;
	else							 
		Dist += Normal.Z * Box->Min.Z;
	Dist+=jePlane_PointDistanceFast(Plane,Pos);

	MoveVec->X=-Dist*Plane->Normal.X;
	MoveVec->Y=-Dist*Plane->Normal.Y;
	MoveVec->Z=-Dist*Plane->Normal.Z;
}

//========================================================================================
//	jeActor_RotateCollision
//     This function checks an Actor to see if it collides with other Actors in the world or
// with the world itself during a rotation (orientation) change.  This isn't really needed if 
// you're not using bone level collisions.  If you are, it's very necessary in order to keep 
// your actors in the world.
// (Modified by Icestorm)
//========================================================================================
JETAPI jeBoolean JETCC jeActor_RotateCollision(jeActor *Actor,
											const jeWorld *World,											
											const jeXForm3d *NewTransform, 
											jeCollisionInfo *CollisionInfo)
{
	char *BoneName;
	jeExtBox BoneBox, NewBoneBox;
	jeVec3d vPos, vBoneBoxPos, vNewBoneBoxPos, vNormal, vMoved;
	jeXForm3d ActorTransform, OldTransform;	
	jeCollisionInfo WorldCollisionInfo;
	jeChangeBoxCollisionInfo WorldChangeBoxCollisionInfo;
	jeFloat Dist;	
	jeBoolean Hit = JE_FALSE;
	int32 iContents;
	jeObject *Obj;

	// Get the name of the first collision bone (if there is one)
	BoneName = jeActor_GetNextCollisionBone(Actor, NULL);	

	// Make the actor empty so it doesn't collide with itself	
	Obj = NULL;	
	while( ( Obj = jeWorld_GetNextObject((jeWorld*)World, Obj) ) != NULL)
		if(jeObject_GetInstance(Obj) == Actor) break;
		
	if(Obj != NULL)
	{
		iContents = jeObject_GetContents(Obj);
		jeObject_SetContents(Obj, CONTENTS_EMPTY);
	}


	if( jeWorld_GetCollisionLevel(World) != COLLIDE_BONES || BoneName == NULL )
	{
			Hit = JE_FALSE;
	}
	else
	{				
		jeActor_GetBoneTransform(Actor, NULL, &OldTransform); // Get actor pos (world)	
		jeVec3d_Set(&vMoved, 0.0f, 0.0f, 0.0f);

		// --- Setup for old pos
		// Initialize actor's orientation
		jeActor_ClearPose(Actor,&OldTransform);
		jeActor_AnimationTestStep(Actor, 0.0f);
		// GetBoneExtBoxes
		jeActor_RecalcCollisionBones(Actor);

		// ---- Setup for new pos
		// Initialize actor's orientation
		ActorTransform = *NewTransform;
		ActorTransform.Translation = OldTransform.Translation;
		jeActor_ClearPose(Actor,&ActorTransform);
		jeActor_AnimationTestStep(Actor, 0.0f);
		// GetNewBoneExtBoxes
		jeActor_RecalcCollisionBones(Actor);

		BoneName = NULL;			

		// Get the name of the first collision bone (if there is one)
		while( (BoneName = jeActor_GetNextCollisionBoneWithExtBoxes(Actor, BoneName, &BoneBox, &NewBoneBox)) != NULL )
		{
			assert(jeExtBox_IsValid(&BoneBox)&&!jeExtBox_IsPoint(&BoneBox));
			assert(jeExtBox_IsValid(&NewBoneBox)&&!jeExtBox_IsPoint(&NewBoneBox));
							
			// Get Position of ExtBoxes and make them relative
			jeExtBox_TranslateAndMoveToOrigin(&BoneBox, &vMoved, &vBoneBoxPos);
			jeExtBox_TranslateAndMoveToOrigin(&NewBoneBox, &vMoved, &vNewBoneBoxPos);
			
			// Grow BoneBox to NewBoneBox and test on collision
			if (CollisionInfo)
			{
				if (jeWorld_ChangeBoxCollision(World, &vBoneBoxPos, &BoneBox, &NewBoneBox, &WorldChangeBoxCollisionInfo))
				{
					// Modify vBoneBoxPos to new pos. and get movedist.
					MoveBoxForPlaneOut(&vBoneBoxPos, &NewBoneBox, &WorldChangeBoxCollisionInfo.Plane, &vPos);
					jeVec3d_Add(&vBoneBoxPos, &vPos, &vBoneBoxPos);
					jeVec3d_Add(&vNewBoneBoxPos, &vPos, &vNewBoneBoxPos);
					jeVec3d_Add(&vMoved, &vPos, &vMoved);

					jeActor_SetCollidedBone(Actor,BoneName);
					CollisionInfo->Plane  = WorldChangeBoxCollisionInfo.Plane;
					CollisionInfo->Object = WorldChangeBoxCollisionInfo.Object;
					Hit = JE_TRUE;
				}
				// Now test on movingcollision
				if(jeWorld_Collision(World, &NewBoneBox, &vBoneBoxPos, &vNewBoneBoxPos, &WorldCollisionInfo))
				{
					// Adjust impact point to where we could complete the rotation
					vNormal = WorldCollisionInfo.Plane.Normal;										
					jeVec3d_Subtract(&WorldCollisionInfo.Impact,&vNewBoneBoxPos,&vPos);						
					Dist = jeVec3d_DotProduct(&vNormal,&vPos);
					jeVec3d_AddScaled(&vMoved, &vNormal, Dist, &vMoved);
					
					*CollisionInfo = WorldCollisionInfo;
					jeActor_SetCollidedBone(Actor,BoneName);
					
					Hit = JE_TRUE;
				}							 
			} else
				if (jeWorld_ChangeBoxCollision(World, &vBoneBoxPos, &BoneBox, &NewBoneBox, NULL) ||
					jeWorld_Collision(World, &NewBoneBox, &vBoneBoxPos, &vNewBoneBoxPos, NULL))
				{
					// Restore actor contents
					if(Obj != NULL)
						jeObject_SetContents(Obj, iContents);

					return JE_TRUE;
				}

		}

		if (Hit)
		{
			// Icestorm: Can we move to there?
			jeVec3d Front,Back;

			Front = OldTransform.Translation;
			jeVec3d_Add(&Front, &vMoved, &Back);

			jeVec3d_Add(&OldTransform.Translation, &vMoved, &CollisionInfo->Impact);
			CollisionInfo->IsValid = JE_TRUE;

			BoneName = NULL;

			// Get the name of the first collision bone (if there is one)
			while( (BoneName = jeActor_GetNextCollisionBoneWithExtBoxes(Actor, BoneName, &BoneBox, NULL)) != NULL )
			{
			
				assert(jeExtBox_IsValid(&BoneBox)&&!jeExtBox_IsPoint(&BoneBox));
				jeExtBox_SetNewOrigin(&BoneBox, &OldTransform.Translation);
	
				if(jeWorld_Collision(World, &BoneBox, &Front, &Back, NULL))
				{
					CollisionInfo->IsValid = JE_FALSE;
					break;
				}
			}

			// Icestorm: Can we rotate there?

			if (CollisionInfo->IsValid!=JE_FALSE)
			{
				BoneName = NULL;			

				// Get the name of the first collision bone (if there is one)
				while( (BoneName = jeActor_GetNextCollisionBoneWithExtBoxes(Actor, BoneName, &BoneBox, &NewBoneBox)) != NULL )
				{
					assert(jeExtBox_IsValid(&BoneBox)&&!jeExtBox_IsPoint(&BoneBox));
					assert(jeExtBox_IsValid(&NewBoneBox)&&!jeExtBox_IsPoint(&NewBoneBox));
							
					jeExtBox_TranslateAndMoveToOrigin(&BoneBox, &vMoved, &vBoneBoxPos);
					jeExtBox_TranslateAndMoveToOrigin(&NewBoneBox, &vMoved, &vNewBoneBoxPos);
			
					// Grow BoneBox to NewBoneBox and test on collision
					// And test on movingcollision
					if (jeWorld_ChangeBoxCollision(World, &vBoneBoxPos, &BoneBox, &NewBoneBox, NULL)||
					    jeWorld_Collision(World, &NewBoneBox, &vBoneBoxPos, &vNewBoneBoxPos, NULL))
					{
						CollisionInfo->IsValid = JE_FALSE;
						break;
					}
				}
			}
		}

		// Initialize actor's orientation
		jeActor_ClearPose(Actor,&OldTransform);
		jeActor_AnimationTestStep(Actor, 0.0f);
	}

	// Restore actor contents
	if(Obj != NULL)
		jeObject_SetContents(Obj, iContents);

	return Hit;
}

//========================================================================================
//	jeActor_AnimationCollision
//     This function checks an Actor to see if it collides with other Actors in the world or
// with the world itself during an animation change.  This isn't really needed if 
// you're not using bone level collisions.  If you are, it's very necessary in order to keep 
// your actors in the world.
//
// (Modified by Icestorm)
//========================================================================================
JETAPI jeBoolean JETCC jeActor_AnimationCollision(jeActor *Actor,
											const jeWorld *World,											
											const jeFloat DeltaTime, 
											jeCollisionInfo *CollisionInfo)
{
	char *BoneName;
	jeExtBox BoneBox, NewBoneBox;
	jeVec3d vPos, vBoneBoxPos, vNewBoneBoxPos, vNormal, vMoved;
	jeXForm3d OldTransform;	
	jeCollisionInfo WorldCollisionInfo;
	jeChangeBoxCollisionInfo WorldChangeBoxCollisionInfo;
	jeFloat Dist;	
	jeBoolean Hit = JE_FALSE;
	int32 iContents;
	jeObject *Obj;

	// Get the name of the first collision bone (if there is one)
	BoneName = jeActor_GetNextCollisionBone(Actor, NULL);	

	// Make the actor empty so it doesn't collide with itself	
	Obj = NULL;	
	while( ( Obj = jeWorld_GetNextObject((jeWorld*)World, Obj) ) != NULL)
		if(jeObject_GetInstance(Obj) == Actor) break;
		
	if(Obj != NULL)
	{
		iContents = jeObject_GetContents(Obj);
		jeObject_SetContents(Obj, CONTENTS_EMPTY);
	}


	if( jeWorld_GetCollisionLevel(World) != COLLIDE_BONES || BoneName == NULL )
	{
			Hit = JE_FALSE;
	}
	else
	{			
		jeActor_GetBoneTransform(Actor, NULL, &OldTransform); // Get actor pos (world)
		jeVec3d_Set(&vMoved, 0.0f, 0.0f, 0.0f);

		// --- Setup for old pos
		// Initialize actor's animation		
		jeActor_ClearPose(Actor,&OldTransform);
		jeActor_AnimationTestStep(Actor, 0.0f);
		jeActor_RecalcCollisionBones(Actor);

		// ---- Setup for new pos
		// Initialize actor's animation
		jeActor_AnimationTestStep(Actor, DeltaTime);
		jeActor_RecalcCollisionBones(Actor);

		BoneName = NULL;			

		// Get the name of the first collision bone (if there is one)
		while( (BoneName = jeActor_GetNextCollisionBoneWithExtBoxes(Actor, BoneName, &BoneBox, &NewBoneBox)) != NULL )
		{
			
			assert(jeExtBox_IsValid(&BoneBox)&&!jeExtBox_IsPoint(&BoneBox));
			assert(jeExtBox_IsValid(&NewBoneBox)&&!jeExtBox_IsPoint(&NewBoneBox));
							
			jeExtBox_TranslateAndMoveToOrigin(&BoneBox, &vMoved, &vBoneBoxPos);
			jeExtBox_TranslateAndMoveToOrigin(&NewBoneBox, &vMoved, &vNewBoneBoxPos);
	
			if (CollisionInfo)
			{
				// Grow BoneBox to NewBoneBox and test on collision
				if (jeWorld_ChangeBoxCollision(World, &vBoneBoxPos, &BoneBox, &NewBoneBox, &WorldChangeBoxCollisionInfo))
				{
					// Modify vBoneBoxPos to new pos. and get movedist.
					MoveBoxForPlaneOut(&vBoneBoxPos, &NewBoneBox, &WorldChangeBoxCollisionInfo.Plane, &vPos);
					jeVec3d_Add(&vBoneBoxPos, &vPos, &vBoneBoxPos);
					jeVec3d_Add(&vNewBoneBoxPos, &vPos, &vNewBoneBoxPos);
					jeVec3d_Add(&vMoved, &vPos, &vMoved);

					jeActor_SetCollidedBone(Actor,BoneName);
					CollisionInfo->Plane  = WorldChangeBoxCollisionInfo.Plane;
						
					Hit = JE_TRUE;
				}
				// Now test on movingcollision
				if(jeWorld_Collision(World, &NewBoneBox, &vBoneBoxPos, &vNewBoneBoxPos, &WorldCollisionInfo))
				{
					// Adjust impact point to where we could complete the rotation
					vNormal = WorldCollisionInfo.Plane.Normal;										
					jeVec3d_Subtract(&WorldCollisionInfo.Impact,&vNewBoneBoxPos,&vPos);						
					Dist = jeVec3d_DotProduct(&vNormal,&vPos);
					jeVec3d_AddScaled(&vMoved,&vNormal,Dist,&vMoved);
				
					*CollisionInfo = WorldCollisionInfo;
					jeActor_SetCollidedBone(Actor,BoneName);
			
					Hit = JE_TRUE;
				}				
			} else
				if (jeWorld_ChangeBoxCollision(World, &vBoneBoxPos, &BoneBox, &NewBoneBox, NULL) ||
					jeWorld_Collision(World, &NewBoneBox, &vBoneBoxPos, &vNewBoneBoxPos, NULL))
				{
					// Restore actor contents
					if(Obj != NULL)
						jeObject_SetContents(Obj, iContents);

					return JE_TRUE;
				}
		} 

		if (Hit)
		{
			// Icestorm: Can we move to there?
			jeVec3d Front,Back;

			Front = OldTransform.Translation;
			jeVec3d_Add(&Front, &vMoved, &Back);

			jeVec3d_Add(&OldTransform.Translation, &vMoved, &CollisionInfo->Impact);
			CollisionInfo->IsValid = JE_TRUE;

			BoneName = NULL;

			// Get the name of the first collision bone (if there is one)
			while( (BoneName = jeActor_GetNextCollisionBoneWithExtBoxes(Actor, BoneName, &BoneBox, NULL)) != NULL )
			{
			
				assert(jeExtBox_IsValid(&BoneBox)&&!jeExtBox_IsPoint(&BoneBox));
				jeExtBox_SetNewOrigin(&BoneBox, &OldTransform.Translation);
	
				if(jeWorld_Collision(World, &BoneBox, &Front, &Back, NULL))
				{
					CollisionInfo->IsValid = JE_FALSE;
					break;
				}
			}

			// Icestorm: Can we animate there?

			if (CollisionInfo->IsValid!=JE_FALSE)
			{
				BoneName = NULL;			

				// Get the name of the first collision bone (if there is one)
				while( (BoneName = jeActor_GetNextCollisionBoneWithExtBoxes(Actor, BoneName, &BoneBox, &NewBoneBox)) != NULL )
				{
					assert(jeExtBox_IsValid(&BoneBox)&&!jeExtBox_IsPoint(&BoneBox));
					assert(jeExtBox_IsValid(&NewBoneBox)&&!jeExtBox_IsPoint(&NewBoneBox));
							
					jeExtBox_TranslateAndMoveToOrigin(&BoneBox, &vMoved, &vBoneBoxPos);
					jeExtBox_TranslateAndMoveToOrigin(&NewBoneBox, &vMoved, &vNewBoneBoxPos);
			
					// Grow BoneBox to NewBoneBox and test on collision
					// And test on movingcollision
					if (jeWorld_ChangeBoxCollision(World, &vBoneBoxPos, &BoneBox, &NewBoneBox, NULL)||
					    jeWorld_Collision(World, &NewBoneBox, &vBoneBoxPos, &vNewBoneBoxPos, NULL))
					{
						CollisionInfo->IsValid = JE_FALSE;
						break;
					}
				}
			}
		}
		// Initialize actor's animation		
		jeActor_ClearPose(Actor,&OldTransform);
		jeActor_AnimationTestStep(Actor, 0.0f);
	}

	// Restore actor contents
	if(Obj != NULL)
		jeObject_SetContents(Obj, iContents);

	return Hit;
}

// This function is used to set whether or not a particular Actor should be checked
// during a collision test.  Three Flags are possible, you should set ONLY one:
//   COLLIDE_EMPTY - This actor can not collide against anything or have other objects collide with it.
//   COLLIDE_SOLID - This actor can collide with other objects and have other objects collide with it.
//   COLLIDE_INVISIBLE - This actor can collide with other objects, but other objects can not collide with it.
// Actors are initialized with a COLLIDE_SOLID state.
JETAPI void JETCC jeActor_SetCollisionFlags(jeActor* Actor, const uint32 Flags)
{
	Actor->CollisionFlags = Flags;
}

JETAPI uint32 JETCC jeActor_GetCollisionFlags(const jeActor* Actor)
{
	return Actor->CollisionFlags;
}

JETAPI void JETCC jeActor_SetCollidedBone(jeActor* Actor, char *BoneName)
{
	Actor->CollidedBone = BoneName;
}

JETAPI char* JETCC jeActor_GetCollidedBone(const jeActor* Actor)
{
	return Actor->CollidedBone;
}


// --- Incarnadine: End Actor Collision ---
//========================================================================================

//========================================================================================
// --- Incarnadine: Begin New Support Functions for ActorObj
JETAPI jeBoolean JETCC jeActor_GetXForm(
	const jeActor *Actor,	// object instance data
	jeXForm3d	*Xf )		// where to store xform
{

	// ensure valid data
	assert( Actor != NULL );
	assert( Xf != NULL );

	// save xform
	//return jeActor_GetBoneTransform(Object, NULL, Xf);
	*Xf = Actor->Xf;
	return JE_TRUE;
} // GetXForm()

JETAPI jeBoolean JETCC jeActor_SetXForm(
	jeActor* Actor,	// object instance data
	const jeXForm3d	*Xf )		// where to store xform
{
	ActorObj *Object;

	// ensure valid data
	assert( Actor != NULL );
	assert( Xf != NULL );
	Object = Actor->Object;

	Actor->Xf = *Xf;

	if(Actor->ActorDefinition)
	{
		jeActor_ClearPose( Actor,  Xf );
		jeActor_AnimationTestStep(Actor,0.0f);

		if(Object->Motion)
		{
			jeActor_SetPose( Actor, Object->Motion, Object->MotionTime, &Actor->Xf );
		}
	}
	return 	JE_TRUE;
} // SetXForm()
// --- Incarnadine: End New Support Functions for ActorObj
//========================================================================================

//KROUER
// Actor VIS problem
JETAPI void JETCC jeActor_SetRenderNextTime(jeActor* Actor, jeBoolean RenderNextTime)
{
	//EnterCriticalSection(&Actor->RenderLock);
	Actor->RenderNextTime = RenderNextTime;
	//LeaveCriticalSection(&Actor->RenderLock);
}

//========================================================================================
// --- Incarnadine: Begin ActorObj ---
#include "ActorObj.h"
//extern jeObjectDef jeActor_ObjectDef;

JETAPI void	JETCC jeActor_InitObject(const jeActor *A,jeObject *O)
{
	assert( jeActor_IsValid(A) );	
	assert( O );
	O->Name = NULL;
	O->Methods = &jeActor_ObjectDef;
	O->Instance = (void *)A;
	O->RefCnt = 0;
}

JETAPI jeBoolean JETCC jeActor_RegisterObjectDef(void)
{
	return jeObject_RegisterGlobalObjectDef( &jeActor_ObjectDef );
}
// --- Incarnadine: End ActorObj ---
//========================================================================================
