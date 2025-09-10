/****************************************************************************************/
/*  POSE.C																				*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Bone hierarchy implementation.							.				*/
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

#pragma message ("could optimize a name binded setPose by caching the mapping from motionpath[i] to joint[j]")

#include <assert.h>
#include <string.h>

#include "Ram.h"
#include "Errorlog.h"
#include "Pose.h"
#include "StrBlock.h"

#define JE_POSE_STARTING_JOINT_COUNT (1)


/* this object maintains a hierarchy of joints.
   the hierarchy is stored as an array of joints, with each joint having an number
   that is it's parent's index in the array.  
   This code assumes:  
   **The parent's index is always smaller than the child**
*/

typedef struct jePose_Joint
{
	int			 ParentJoint;		// parent of path
	jeXForm3d    *Transform;		// matrix for path	(pointer into TransformArray)
	jeQuaternion Rotation;			// quaternion representation for orientation of above Transform

	jeVec3d		 UnscaledAttachmentTranslation;	
					// point of Attachment to parent (in parent frame of ref) **Unscaled
	jeQuaternion AttachmentRotation;// rotation of attachement to parent (in parent frame of ref)
	jeXForm3d    AttachmentTransform;	//------------

	jeVec3d		 LocalTranslation;	// translation relative to attachment 
	jeQuaternion LocalRotation;		// rotation relative to attachment 

	jeBoolean    Touched;			// if this joint has been touched and needs recomputation
	jeBoolean    NoAttachmentRotation; // JE_TRUE if there is no attachment rotation.
	int			 Covered;			// if joint has been 100% set (no blending)
} jePose_Joint;						// structure to bind a name and a path for a joint

typedef struct jePose
{
	int				  JointCount;	// number of joints in the motion
	int32			  NameChecksum;	// checksum based on joint names and list order
	jeBoolean		  Touched;		// if any joint has been touched & needs recomputation	
	jeStrBlock		 *JointNames;
	jeVec3d			  Scale;		// current scaling. Used for scaling motion samples

	jeBoolean		  Slave;			// if pose is 'slaved' to parent -vs- attached.
	int				  SlaveJointIndex;	// index of 'slaved' joint
	jePose			 *Parent;		
	jePose_Joint	  RootJoint;		
	jeXForm3d		  ParentsLastTransform;	// Compared to parent's transform to see if it changed: recompute is needed
	jeXForm3d		  RootTransform;
	jeXFArray		 *TransformArray;	
	jePose_Joint	 *JointArray;
	int				  OnlyThisJoint;		// update only this joint (and it's parents) if this is >0
} jePose;



static void jePose_ReattachTransforms(jePose *P)
{
	int XFormCount;
	int JointCount;
	jeXForm3d *XForms;
	int i;

	assert( P != NULL );

	JointCount = P->JointCount;
	if (JointCount > 0)
		{
			assert( P->TransformArray != NULL );

			XForms = jeXFArray_GetElements(P->TransformArray,&XFormCount);
			
			assert( XForms != NULL );
			assert( XFormCount == JointCount );
			
			for (i=0; i<JointCount; i++)
				{
					P->JointArray[i].Transform=&(XForms[i]);
				}
		}
	P->RootJoint.Transform = &(P->RootTransform);
}
	

static const jePose_Joint *jePose_JointByIndex(const jePose *P, int Index)
{
	assert( P != NULL );
	assert( (Index >=0)                 || (Index==(JE_POSE_ROOT_JOINT)));
	assert( (Index < P->JointCount)     || (Index==(JE_POSE_ROOT_JOINT)));

	if (Index == JE_POSE_ROOT_JOINT)
		{
			return &(P->RootJoint);
		}
	else
		{
			return &(P->JointArray[Index]);
		}
}

static void JETCF jePose_SetAttachmentRotationFlag( jePose_Joint *Joint)
{
	jeQuaternion Q = Joint->AttachmentRotation;
#define JE_POSE_ROTATION_THRESHOLD (0.0001)  // if the rotation is closer than this to zero for
										     // quaterion elements X,Y,Z -> no rotation computed
	if (     (  (Q.X<JE_POSE_ROTATION_THRESHOLD) && (Q.X>-JE_POSE_ROTATION_THRESHOLD) ) 
		  && (  (Q.Y<JE_POSE_ROTATION_THRESHOLD) && (Q.Y>-JE_POSE_ROTATION_THRESHOLD) ) 
		  && (  (Q.Z<JE_POSE_ROTATION_THRESHOLD) && (Q.Z>-JE_POSE_ROTATION_THRESHOLD) )  )
		{
			Joint->NoAttachmentRotation = JE_TRUE;
		}
	else
		{
			Joint->NoAttachmentRotation = JE_FALSE;
		}
}

static void JETCF jePose_InitializeJoint(jePose_Joint *Joint, int ParentJointIndex, const jeXForm3d *Attachment)
{
	assert( Joint != NULL );
	
	Joint->ParentJoint = ParentJointIndex;
	if (Attachment != NULL)
		{
			jeQuaternion_FromMatrix(Attachment,&(Joint->AttachmentRotation));
			Joint->AttachmentTransform = *Attachment;
			Joint->UnscaledAttachmentTranslation = Joint->AttachmentTransform.Translation;
		}
	else
		{
			jeQuaternion_SetNoRotation(&(Joint->AttachmentRotation));
			jeXForm3d_SetIdentity(&(Joint->AttachmentTransform));
			Joint->UnscaledAttachmentTranslation = Joint->AttachmentTransform.Translation;
		}

	jeQuaternion_SetNoRotation(&(Joint->LocalRotation));
	
	jeXForm3d_SetIdentity(Joint->Transform);
	jeQuaternion_SetNoRotation(&(Joint->Rotation));
	
	jeVec3d_Set( (&Joint->LocalTranslation),0.0f,0.0f,0.0f);
	jeQuaternion_SetNoRotation(&(Joint->LocalRotation));
	Joint->Touched = JE_TRUE;		
	jePose_SetAttachmentRotationFlag(Joint);
}



jePose *JETCF jePose_Create(void)
{
	jePose *P;

	P = JE_RAM_ALLOCATE_STRUCT_CLEAR(jePose);

	if ( P == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jePose_Create.");
			goto PoseCreateFailure;
		}
	P->JointCount = 0;
	P->OnlyThisJoint = JE_POSE_ROOT_JOINT-1;		
	P->JointNames = jeStrBlock_Create();
	P->Touched = JE_FALSE;
	if ( P->JointNames == NULL )
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePose_Create: failed to create string block.");
			goto PoseCreateFailure;
		}
	P->JointArray = JE_RAM_ALLOCATE_STRUCT_CLEAR( jePose_Joint );
	if (P->JointArray == NULL)
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jePose_Create.");
			goto PoseCreateFailure;
		}
	P->TransformArray=NULL; //jeXFArray_Create(0);

	P->Slave = JE_FALSE;
	P->Parent = NULL;
	jePose_ReattachTransforms(P);
	jePose_InitializeJoint(&(P->RootJoint),JE_POSE_ROOT_JOINT,NULL);

	P->Scale.X = P->Scale.Y = P->Scale.Z = 1.0f;
	return P;
	PoseCreateFailure:
	if (P!=NULL)
		{
			if (P->JointNames != NULL)
				jeStrBlock_Destroy(&(P->JointNames));
			if (P->JointArray != NULL)
				jeRam_Free(P->JointArray);
			jeRam_Free(P);
		}
	return NULL;
}

void JETCF jePose_Destroy(jePose **PP)
{
	assert(PP   != NULL );
	assert(*PP  != NULL );

	assert( (*PP)->JointNames != NULL );
	assert( jeStrBlock_GetCount((*PP)->JointNames) == (*PP)->JointCount );
	jeStrBlock_Destroy( &( (*PP)->JointNames ) );
	if ((*PP)->TransformArray!=NULL)
		{
			jeXFArray_Destroy(&( (*PP)->TransformArray) );
		}
	if ((*PP)->JointArray != NULL)
		jeRam_Free((*PP)->JointArray);
	jeRam_Free( *PP );

	*PP = NULL;
}

// uses J->LocalRotation and J->LocalTranslation to compute 
//    J->Rotation,J->Translation and J->Transform
static void JETCF jePose_JointRelativeToParent(
		 const jePose_Joint *Parent,
		 jePose_Joint *J)
{
	
	#if 0
		// the math in clearer (but slower) matrix form.
		// W = PAK
		jeXForm3d X;
		jeXForm3d K;

		jeQuaternion_ToMatrix(&(J->LocalRotation),&K);
		K.Translation = J->LocalTranslation;

		jeXForm3d_Multiply((Parent->Transform),&(J->AttachmentTransform),&X);
		jeXForm3d_Multiply(&X,&(K),J->Transform);
		
		J->LocalTranslation = K.Translation;
		jeQuaternion_FromMatrix(J->Transform,&(J->LocalRotation));

	#endif


	jeVec3d *Translation = &(J->Transform->Translation);
	if (J->NoAttachmentRotation != JE_FALSE)
		{
			//    ( no attachment rotation )
			//ROTATION:
			// concatenate local rotation to parent rotation for complete rotation
			jeQuaternion_Multiply(&(Parent->Rotation), &(J->LocalRotation), &(J->Rotation));
			
			jeQuaternion_ToMatrix(&(J->Rotation), (J->Transform));
			//TRANSLATION:
			jeVec3d_Add(&(J->LocalTranslation),&(J->AttachmentTransform.Translation),Translation);
			jeXForm3d_Transform((Parent->Transform),Translation,Translation);
		}
	else
		{
			//  (there is an attachment rotation)
			
			jeQuaternion BaseRotation; // attachement transform applied to the parent transform:
			//ROTATION:
			// concatenate attachment rotation to parent rotation for base rotation
			jeQuaternion_Multiply(&(Parent->Rotation),&(J->AttachmentRotation),&BaseRotation);
			// concatenate base rotation with local rotation for complete rotation
			jeQuaternion_Multiply(&BaseRotation, &(J->LocalRotation), &(J->Rotation));

			jeQuaternion_ToMatrix(&(J->Rotation), (J->Transform));

			//TRANSLATION:
			jeXForm3d_Transform(&(J->AttachmentTransform),&(J->LocalTranslation),Translation);
			jeXForm3d_Transform((Parent->Transform),Translation,Translation);
		}
}


jeBoolean JETCF jePose_Attach(jePose *Slave, int SlaveBoneIndex,
				  jePose *Master, int MasterBoneIndex, 
				  const jeXForm3d *Attachment)
{
	jePose *P;
	P = Master;

	assert( Slave != NULL );
	assert( Master != NULL );
	assert( MasterBoneIndex >= 0);
	assert( MasterBoneIndex < Master->JointCount);
	assert( Attachment != NULL );
	assert( Master != Slave );


	assert( (SlaveBoneIndex >=0)                 || (SlaveBoneIndex==(JE_POSE_ROOT_JOINT)));
	assert( (SlaveBoneIndex < Slave->JointCount) || (SlaveBoneIndex==(JE_POSE_ROOT_JOINT)));

	while (P!=NULL)
		{
			if (P==Slave)
				{
					jeErrorLog_Add(JE_ERR_BAD_PARAMETER, "jePose_Attach: circular loop of attachments not allowed");
					return JE_FALSE;
				}
			P=P->Parent;
		}

	Slave->SlaveJointIndex = SlaveBoneIndex;
	Slave->Parent = Master;
	if (SlaveBoneIndex == JE_POSE_ROOT_JOINT)
		{
			Slave->Slave = JE_FALSE;
		}
	else
		{
			Slave->Slave = JE_TRUE;
		}

	jePose_InitializeJoint(&(Slave->RootJoint),MasterBoneIndex,Attachment);
	Slave->Touched = JE_TRUE;
	Slave->ParentsLastTransform = *(Master->RootJoint.Transform);
	
	return JE_TRUE;
}


void JETCF jePose_Detach(jePose *P)
{
	P->Parent = NULL;
	P->Slave = JE_FALSE;
	jePose_InitializeJoint(&(P->RootJoint),JE_POSE_ROOT_JOINT,NULL);
}


static jeBoolean JETCF jePose_TransformCompare(const jeXForm3d *T1, const jeXForm3d *T2)
{
	if (T1->AX != T2->AX) return JE_FALSE;
	if (T1->BX != T2->BX) return JE_FALSE;
	if (T1->CX != T2->CX) return JE_FALSE;
	if (T1->AY != T2->AY) return JE_FALSE;
	if (T1->BY != T2->BY) return JE_FALSE;
	if (T1->CY != T2->CY) return JE_FALSE;
	if (T1->AZ != T2->AZ) return JE_FALSE;
	if (T1->BZ != T2->BZ) return JE_FALSE;
	if (T1->CZ != T2->CZ) return JE_FALSE;
	
	if (T1->Translation.X != T2->Translation.X) return JE_FALSE;
	if (T1->Translation.Y != T2->Translation.Y) return JE_FALSE;
	if (T1->Translation.Z != T2->Translation.Z) return JE_FALSE;
	return JE_TRUE;
}
	
	
static void JETCF jePose_UpdateRecursively(jePose *P,int Joint)
{
	jePose_Joint *J;
	assert( P != NULL );
	assert( Joint >= JE_POSE_ROOT_JOINT );

	J=&(P->JointArray[Joint]);

	assert( J->ParentJoint < Joint);

	if (J->ParentJoint != JE_POSE_ROOT_JOINT)
		jePose_UpdateRecursively(P,J->ParentJoint);

	jePose_JointRelativeToParent(jePose_JointByIndex(P, J->ParentJoint) ,J);
}

//  updates a node if node->touched or if any of it's parents have been touched.
//  returns JE_TRUE if any updates were made.
static void JETCF jePose_UpdateRelativeToParent(jePose *P)
{
	int i;
	jePose_Joint *J;
	const jePose_Joint *Parent;
	assert( P != NULL );
	
	if ( P->Parent != NULL )
		{
			jePose_UpdateRelativeToParent(P->Parent);
			if (jePose_TransformCompare(
						(P->Parent->RootJoint.Transform),&(P->ParentsLastTransform)) != JE_FALSE)
				{
					P->Touched = JE_TRUE;
					P->RootJoint.Touched = JE_TRUE;  // bubble touched down entire hierarchy
					P->ParentsLastTransform = *(P->Parent->RootJoint.Transform);
				}
				
			if (P->Slave == JE_FALSE)
				{
					Parent = jePose_JointByIndex(P->Parent, P->RootJoint.ParentJoint);
					jePose_JointRelativeToParent(Parent,&(P->RootJoint));
				}
			else
				{
					jeXForm3d_SetIdentity(P->RootJoint.Transform);
					jeQuaternion_SetNoRotation(&(P->RootJoint.Rotation));
				}
		}
	else
		{
			// No parent.  RootJoint is relative to nothing.
			J = &(P->RootJoint);
			if (J->Touched)
				{
					jeQuaternion_Multiply(&(J->AttachmentRotation),&(J->LocalRotation),&(J->Rotation));
					jeQuaternion_ToMatrix(&(J->Rotation), (J->Transform));
					jeXForm3d_Transform(&(J->AttachmentTransform),&(J->LocalTranslation),&(J->Transform->Translation));
				}
		}


	if (P->Touched == JE_FALSE)
		{
			return;
		}


	if (P->OnlyThisJoint>=JE_POSE_ROOT_JOINT)
		{
			jePose_UpdateRecursively(P,P->OnlyThisJoint);
		}
	else
		{
			for (i=0, J=&(P->JointArray[0]); i<P->JointCount; i++,J++)
				{
					assert( J->ParentJoint < i);

					Parent = jePose_JointByIndex(P, J->ParentJoint);
					if (J->Touched == JE_TRUE)
						{
							jePose_JointRelativeToParent(Parent ,J);
						}
					else
						{
							if (Parent->Touched)
								{
									J->Touched = JE_TRUE;
									jePose_JointRelativeToParent(Parent,J);
								}
						}
				}
			// touched flags don't mean anything when recursing backwards.  
			P->RootJoint.Touched = JE_FALSE;
			for (i=0, J=&(P->JointArray[0]); i<P->JointCount; i++,J++)
				{
					J->Touched = JE_FALSE;
				}
		}

	if (P->Slave != JE_FALSE)
		{
			jeXForm3d SlavedJointInverse;
			jeXForm3d FullSlaveTransform;
			jeXForm3d *MasterTransform;
			jeXForm3d MasterAttachment;
			
			MasterTransform = (P->Parent->JointArray[P->RootJoint.ParentJoint].Transform);
			jeXForm3d_GetTranspose((P->JointArray[P->SlaveJointIndex].Transform), &SlavedJointInverse);

			jeQuaternion_ToMatrix(&(P->RootJoint.AttachmentRotation), &MasterAttachment);
			//MasterAttachment.Translation = P->RootJoint.AttachmentTranslation;
			MasterAttachment.Translation = P->RootJoint.AttachmentTransform.Translation;

			jeXForm3d_Multiply(MasterTransform,&MasterAttachment,&FullSlaveTransform);
			
			*(P->RootJoint.Transform) = FullSlaveTransform;
			
			jeXForm3d_Multiply(&FullSlaveTransform,&SlavedJointInverse,&FullSlaveTransform);

			for (i=0, J=&(P->JointArray[0]); i<P->JointCount; i++,J++)
				{
					jeXForm3d_Multiply(&FullSlaveTransform,
										(P->JointArray[i].Transform),
										(P->JointArray[i].Transform));
				}
			
		}
	P->Touched = JE_FALSE;
}	


jeBoolean JETCF jePose_FindNamedJointIndex(const jePose *P, const char *JointName, int *Index)
{
	int i;

	assert( P != NULL );
	assert( Index!= NULL );
	if (JointName == NULL )
		return JE_FALSE;

	for (i=0; i<P->JointCount; i++)
		{
			const char *NthName = jeStrBlock_GetString(P->JointNames,i);
			assert( NthName!= NULL );
			if ( strcmp(JointName,NthName)==0 )
				{
					*Index = i;
					return JE_TRUE;
				}	
		}
	return JE_FALSE;
}
	

jeBoolean JETCF jePose_AddJoint(
	jePose *P,
	int ParentJointIndex,
	const char *JointName,
	const jeXForm3d *Attachment,
	int *JointIndex)
{
	int JointCount;
	jePose_Joint *Joint;

	assert(  P != NULL );
	assert( JointIndex != NULL );
	assert( P->JointCount >= 0 );
	assert( (ParentJointIndex == JE_POSE_ROOT_JOINT) || 
			((ParentJointIndex >=0) && (ParentJointIndex <P->JointCount) ) );

	// Duplicate names ARE allowed

	JointCount = P->JointCount;
	{
		jePose_Joint *NewJoints;
		NewJoints = JE_RAM_REALLOC_ARRAY(P->JointArray,jePose_Joint,JointCount+1);
		if (NewJoints == NULL)
			{
				jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jePose_AddJoint.");
				return JE_FALSE;
			}
		P->JointArray = NewJoints;
	}
	
	assert( P->JointNames != NULL );
	assert( jeStrBlock_GetCount(P->JointNames) == P->JointCount );

	if (jeStrBlock_Append( &(P->JointNames), (JointName==NULL)?"":JointName )==JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePose_AddJoint: failed to append into string block.");
			return JE_FALSE;
		}
	

	{
		jeXFArray *NewXFA;
		NewXFA = jeXFArray_Create(JointCount+1);
		if (NewXFA == NULL)
			{
				jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jePose_AddJoint: failed to create XFArray.");
				return JE_FALSE;
			}
		if (P->TransformArray != NULL)
			{
				jeXFArray_Destroy(&(P->TransformArray));
			}
		P->TransformArray = NewXFA;
	}

	P->JointCount = JointCount+1;
	jePose_ReattachTransforms(P);
	
	Joint = &( P->JointArray[JointCount] );
	jePose_InitializeJoint(Joint,ParentJointIndex, Attachment);
	P->Touched = JE_TRUE;

	*JointIndex = JointCount;

	P->NameChecksum = jeStrBlock_GetChecksum( P->JointNames );
	return JE_TRUE;
}

void JETCF jePose_GetJointAttachment(const jePose *P,int JointIndex, jeXForm3d *AttachmentTransform)
{
	assert( P != NULL );
	assert( AttachmentTransform != NULL );
	{
		const jePose_Joint *J;
		J = jePose_JointByIndex(P, JointIndex);
		*AttachmentTransform = J->AttachmentTransform;
	}
}

void JETCF jePose_SetJointAttachment(jePose *P,
	int JointIndex, 
	const jeXForm3d *AttachmentTransform)
{
	assert( P != NULL );
	assert( AttachmentTransform != NULL );
	{
		jePose_Joint *J;
		J = (jePose_Joint *)jePose_JointByIndex(P, JointIndex);
		jeQuaternion_FromMatrix(AttachmentTransform,&(J->AttachmentRotation));
		J->Touched = JE_TRUE;
		J->AttachmentTransform = *AttachmentTransform;
		J->UnscaledAttachmentTranslation = J->AttachmentTransform.Translation;
		jePose_SetAttachmentRotationFlag(J);
	}
	P->Touched = JE_TRUE;
}

void JETCF jePose_GetJointTransform(const jePose *P, int JointIndex,jeXForm3d *Transform)
{
	assert( P != NULL );
	assert( Transform != NULL );
	
	jePose_UpdateRelativeToParent((jePose *)P);
	
	{
		const jePose_Joint *J;
		J = jePose_JointByIndex(P, JointIndex);
		*Transform = *(J->Transform);
	}
}

void JETCF jePose_GetJointLocalTransform(const jePose *P, int JointIndex,jeXForm3d *Transform)
{
	assert( P != NULL );
	assert( Transform != NULL );
	{
		const jePose_Joint *J;
		J = jePose_JointByIndex(P, JointIndex);
		jeQuaternion_ToMatrix(&(J->LocalRotation), Transform);
		Transform->Translation = J->LocalTranslation;
	}
}

void JETCF jePose_SetJointLocalTransform(jePose *P, int JointIndex,const jeXForm3d *Transform)
{
	assert( P != NULL );
	assert( Transform != NULL );
	{
		jePose_Joint *J;
		J = (jePose_Joint *)jePose_JointByIndex(P, JointIndex);
		jeQuaternion_FromMatrix(Transform,&(J->LocalRotation));
		J->LocalTranslation = Transform->Translation;
		J->Touched = JE_TRUE;
	}
	P->Touched = JE_TRUE;
}

int JETCF jePose_GetJointCount(const jePose *P)
{
	assert( P != NULL );
	assert( P->JointCount >= 0 );
	
	return P->JointCount;
}

jeBoolean JETCF jePose_MatchesMotionExactly(const jePose *P, const jeMotion *M)
{
	if (jeMotion_HasNames(M) != JE_FALSE)
		{
			if (jeMotion_GetNameChecksum(M) == P->NameChecksum)
				return JE_TRUE;
			else
				return JE_FALSE;
		}
	return JE_FALSE;
}

// sets pose to it's base position: applies no modifier to the joints: only
// it's attachment positioning is used.
void JETCF jePose_Clear(jePose *P,const jeXForm3d *Transform)
{
	int i;
	jePose_Joint *J;
	
	assert( P != NULL );
	assert( P->JointCount >= 0 );
	P->OnlyThisJoint = JE_POSE_ROOT_JOINT-1;		// calling this function disables one-joint optimizations
	if (P->Parent==NULL)
		{
			if (Transform!=NULL)
				{
					jeQuaternion_FromMatrix(Transform,&(P->RootJoint.LocalRotation));
					P->RootJoint.LocalTranslation = Transform->Translation;
				}
			P->RootJoint.Touched = JE_TRUE;
		}
			
	for (i=0, J=&(P->JointArray[0]); i<P->JointCount; i++,J++)
		{
			jeVec3d_Set( (&J->LocalTranslation),0.0f,0.0f,0.0f);
			jeQuaternion_SetNoRotation(&(J->LocalRotation));
			assert( J->ParentJoint < i);
			P->Touched = JE_TRUE;
		}	
	P->Touched = JE_TRUE;
}	

void JETCF jePose_SetMotion(jePose *P, const jeMotion *M, jeFloat Time,
							const jeXForm3d *Transform)
{
	jeBoolean NameBinding;
	int i;
	jePose_Joint *J;
	jeXForm3d RootTransform;
	
	assert( P != NULL );

	P->OnlyThisJoint = JE_POSE_ROOT_JOINT-1;		// calling this function disables one-joint optimizations

    if (P->Parent==NULL)
    {
        jeBoolean SetRoot = JE_FALSE;
        if (jeMotion_GetTransform(M,Time,&RootTransform)!=JE_FALSE)
        {
            SetRoot = JE_TRUE;

            if ( Transform != NULL )
            {
                jeXForm3d_Multiply(Transform,&RootTransform,&RootTransform);
            }
        }
        else
        {
            if ( Transform != NULL )
            {
                SetRoot = JE_TRUE;
                RootTransform = *Transform;
            }
        }

        if (SetRoot != JE_FALSE)
        {
            jeQuaternion_FromMatrix(&RootTransform,&(P->RootJoint.LocalRotation));
            P->RootJoint.LocalTranslation = RootTransform.Translation;
            P->RootJoint.Touched = JE_TRUE;
        }
    }

    if (M==NULL)
    {
        return;
    }

	if (jePose_MatchesMotionExactly(P,M)==JE_TRUE)
		NameBinding = JE_FALSE;
	else
		NameBinding = JE_TRUE;

	P->Touched = JE_TRUE;

#pragma message("could optimize this by looping two ways (min(jointcount,pathcount))")
	for (i=0, J=&(P->JointArray[0]); i<P->JointCount; i++,J++)
    {
        if (NameBinding == JE_FALSE)
        {
            jeMotion_SampleChannels(M,i,Time,&(J->LocalRotation),&(J->LocalTranslation));
        }
        else
        {
            if (jeMotion_SampleChannelsNamed(M,
                jeStrBlock_GetString(P->JointNames,i),
                Time,&(J->LocalRotation),&(J->LocalTranslation))==JE_FALSE)
                continue;

        }
        J->Touched = JE_TRUE;
        J->LocalTranslation.X *= P->Scale.X;
        J->LocalTranslation.Y *= P->Scale.Y;
        J->LocalTranslation.Z *= P->Scale.Z;
    }
}

static void JETCF jePose_SetMotionForABoneRecursion(jePose *P, const jeMotion *M, jeFloat Time,
							int BoneIndex,jeBoolean NameBinding)
{
	jePose_Joint *J;
	jeBoolean Touched = JE_FALSE;
	assert(P!=NULL);
	assert(M!=NULL);
	assert( BoneIndex >= 0);

	J=&(P->JointArray[BoneIndex]);

	if (NameBinding == JE_FALSE)
		{
			jeMotion_SampleChannels(M,BoneIndex,Time,&(J->LocalRotation),&(J->LocalTranslation));
			Touched = JE_TRUE;
		}
	else
		{
			if (jeMotion_SampleChannelsNamed(M,
				jeStrBlock_GetString(P->JointNames,BoneIndex),
				Time,&(J->LocalRotation),&(J->LocalTranslation))!=JE_FALSE)
				Touched = JE_TRUE;
		}
	if (Touched != JE_FALSE)
		{
			J->Touched = JE_TRUE;
			J->LocalTranslation.X *= P->Scale.X;
			J->LocalTranslation.Y *= P->Scale.Y;
			J->LocalTranslation.Z *= P->Scale.Z;
		}
	if (J->ParentJoint != JE_POSE_ROOT_JOINT)
		jePose_SetMotionForABoneRecursion(P,M,Time,J->ParentJoint,NameBinding);
	
}

void JETCF jePose_SetMotionForABone(jePose *P, const jeMotion *M, jeFloat Time,
							const jeXForm3d *Transform,int BoneIndex)
{
	jeBoolean NameBinding;
	jeXForm3d RootTransform;
	
	assert( P != NULL );
	//assert( M != NULL );
	P->OnlyThisJoint = BoneIndex;		// calling this function enables single-joint optimizations
	
	if (P->Parent==NULL)
		{
			jeBoolean SetRoot = JE_FALSE;
			if (jeMotion_GetTransform(M,Time,&RootTransform)!=JE_FALSE)
				{
					SetRoot = JE_TRUE;

					if ( Transform != NULL )
						{
							jeXForm3d_Multiply(Transform,&RootTransform,&RootTransform);
						}
				}
			else
				{
					if ( Transform != NULL )
						{
							SetRoot = JE_TRUE;
							RootTransform = *Transform;
						}
				}

			if (SetRoot != JE_FALSE)
				{
					jeQuaternion_FromMatrix(&RootTransform,&(P->RootJoint.LocalRotation));
					P->RootJoint.LocalTranslation = RootTransform.Translation;
					P->RootJoint.Touched = JE_TRUE;
				}
		}

	if (M==NULL)
		{
			return;
		}
	if (BoneIndex == JE_POSE_ROOT_JOINT)
		{
			return;
		}

	if (jePose_MatchesMotionExactly(P,M)==JE_TRUE)
		NameBinding = JE_FALSE;
	else
		NameBinding = JE_TRUE;

	P->Touched = JE_TRUE;

	jePose_SetMotionForABoneRecursion(P, M, Time, BoneIndex, NameBinding);
}
	



#define LINEAR_BLEND(a,b,t)  ( (t)*((b)-(a)) + (a) )	
			// linear blend of a and b  0<t<1 where  t=0 ->a and t=1 ->b



void JETCF jePose_BlendMotion(	
	jePose *P, const jeMotion *M, jeFloat Time,
	const jeXForm3d *Transform,
	jeFloat BlendAmount, jePose_BlendingType BlendingType)
{
	int i;
	jeBoolean NameBinding;
	jePose_Joint *J;
	jeQuaternion R1;
	jeVec3d      T1;
	jeXForm3d    RootTransform;
	
	assert( P != NULL );
	//assert( M != NULL );  // M can be NULL
	assert( BlendingType == JE_POSE_BLEND_HERMITE || BlendingType == JE_POSE_BLEND_LINEAR);
	assert( BlendAmount >= 0.0f );
	assert( BlendAmount <= 1.0f );

	P->OnlyThisJoint = JE_POSE_ROOT_JOINT-1;		// calling this function disables one-joint optimizations
	if (BlendingType == JE_POSE_BLEND_HERMITE)
		{
			jeFloat t2,t3;
			t2 = BlendAmount * BlendAmount;
			t3 = t2 * BlendAmount;
			BlendAmount = t2*3.0f -t3-t3;
		}

	if (P->Parent==NULL)
		{
			jeBoolean SetRoot = JE_FALSE;
			if (jeMotion_GetTransform(M,Time,&RootTransform)!=JE_FALSE)
				{
					SetRoot = JE_TRUE;

					if ( Transform != NULL )
						{
							jeXForm3d_Multiply(Transform,&RootTransform,&RootTransform);
						}
				}
			else
				{
					if ( Transform != NULL )
						{
							SetRoot = JE_TRUE;
							RootTransform = *Transform;
						}
				}

			if (SetRoot != JE_FALSE)
				{
					jeQuaternion_FromMatrix(&RootTransform,&R1);
					T1 = RootTransform.Translation;
					J  = &(P->RootJoint);
					jeQuaternion_Slerp(&(J->LocalRotation),&(R1),BlendAmount,&(J->LocalRotation));
					{
						jeVec3d      *LT = &(J->LocalTranslation);
						LT->X = LINEAR_BLEND(LT->X,T1.X,BlendAmount);
						LT->Y = LINEAR_BLEND(LT->Y,T1.Y,BlendAmount);
						LT->Z = LINEAR_BLEND(LT->Z,T1.Z,BlendAmount);
					}
					J->Touched = JE_TRUE;
				}
		}

	if (M==NULL)
		{
			return;
		}

	
	if (jePose_MatchesMotionExactly(P,M)==JE_TRUE)
		NameBinding = JE_FALSE;
	else
		NameBinding = JE_TRUE;
	
	P->Touched = JE_TRUE;

	for (i=0, J=&(P->JointArray[0]); i<P->JointCount; i++,J++)
		{
			//jePath *JointPath;
							
			if (NameBinding == JE_FALSE)
				{
					jeMotion_SampleChannels(M,i,Time,&R1,&T1);
					//JointPath = jeMotion_GetPath(M,i);
					//assert( JointPath != NULL );
				}
			else
				{
					//JointPath = jeMotion_GetPathNamed(M, jeStrBlock_GetString(P->JointNames,i));
					//if (JointPath == NULL)
					//	continue;
					if (jeMotion_SampleChannelsNamed(M,
						jeStrBlock_GetString(P->JointNames,i),
						Time,&R1,&T1)==JE_FALSE)
						continue;

				}
			J->Touched = JE_TRUE;

			//jePath_SampleChannels(JointPath,Time,&(R1),&(T1));
			
			T1.X *= P->Scale.X;
			T1.Y *= P->Scale.Y;
			T1.Z *= P->Scale.Z;
			
			jeQuaternion_Slerp(&(J->LocalRotation),&(R1),BlendAmount,&(J->LocalRotation));
						
			{
				jeVec3d      *LT = &(J->LocalTranslation);
				LT->X = LINEAR_BLEND(LT->X,T1.X,BlendAmount);
				LT->Y = LINEAR_BLEND(LT->Y,T1.Y,BlendAmount);
				LT->Z = LINEAR_BLEND(LT->Z,T1.Z,BlendAmount);
			}
		}
}

const char* JETCF jePose_GetJointName(const jePose* P, int JointIndex)
{
	return jeStrBlock_GetString(P->JointNames, JointIndex);
}

const jeXFArray * JETCF jePose_GetAllJointTransforms(const jePose *P)
{
	assert( P != NULL );

	jePose_UpdateRelativeToParent((jePose *)P);
	return P->TransformArray;
}

void JETCF jePose_GetScale(const jePose *P, jeVec3d *Scale)
{
	assert( P     != NULL );
	assert( Scale != NULL );
	*Scale = P->Scale;
}
	

void JETCF jePose_SetScale(jePose *P, const jeVec3d *Scale )
{
	assert( P != NULL );
	assert( jeVec3d_IsValid(Scale) != JE_FALSE );

	{
		int i;
		jePose_Joint *J;

		P->Scale = *Scale;
		//jeVec3d_Set(&(P->Scale),ScaleX,ScaleY,ScaleZ);

		for (i=0, J=&(P->JointArray[0]); i<P->JointCount; i++,J++)
			{	
				J->AttachmentTransform.Translation.X = J->UnscaledAttachmentTranslation.X * Scale->X;
				J->AttachmentTransform.Translation.Y = J->UnscaledAttachmentTranslation.Y * Scale->Y;
				J->AttachmentTransform.Translation.Z = J->UnscaledAttachmentTranslation.Z * Scale->Z;
				//J->AttachmentTransform.Translation = J->AttachmentTranslation;
				J->Touched = JE_TRUE;
			}
		P->Touched = JE_TRUE;
	}
}

void JETCF jePose_ClearCoverage(jePose *P, int ClearTo)
{
	int i;
	jePose_Joint *J;

	assert( P != NULL );
	assert( (ClearTo == JE_FALSE) || (ClearTo == JE_TRUE) );

	for (i=0, J=&(P->JointArray[0]); i<P->JointCount; i++,J++)
		{	
			J->Covered = ClearTo;
		}
}

int JETCF jePose_AccumulateCoverage(jePose *P, const jeMotion *M, jeBoolean QueryOnly)
{
	int i,SubMotions;
	jeBoolean NameBinding;
	int Covers=0;
	jePose_Joint *J;
	
	assert( P != NULL );
	if (M==NULL)
		{
			return P->JointCount;
		}

	SubMotions = jeMotion_GetSubMotionCount(M);
	if (SubMotions>0)
		{
			for (i=0; i<SubMotions; i++)
				{
					int c = jePose_AccumulateCoverage(P, jeMotion_GetSubMotion(M,i),QueryOnly);
					if (c > Covers)
						{
							Covers = c;
						}
				}
			return Covers;
		}
	
	if (jePose_MatchesMotionExactly(P,M)==JE_TRUE)
		NameBinding = JE_FALSE;
	else
		NameBinding = JE_TRUE;

	for (i=0, J=&(P->JointArray[0]); i<P->JointCount; i++,J++)
		{
			jePath *JointPath;
			if (J->Covered == JE_FALSE)
				{
					if (NameBinding == JE_TRUE)
						{
							JointPath = jeMotion_GetPathNamed(M, jeStrBlock_GetString(P->JointNames,i));
							if (JointPath == NULL)
								continue;
						}
					if (QueryOnly == JE_FALSE)
						{
							J->Covered = JE_TRUE;
						}
					Covers ++;
				}
		}
	return Covers;
}
