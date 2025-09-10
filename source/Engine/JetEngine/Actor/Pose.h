/****************************************************************************************/
/*  POSE.H																				*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Bone hierarchy interface.								.				*/
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
#ifndef JE_POSE_H
#define JE_POSE_H

/*	jePose

	This object is a hierarchical set of attached joints.  The joints can have names.
	A 'jePose' keeps track of which children joints move in the hierarchy when a parent
	joint moves.  A jePose also remembers the position transform matrices for each joint.

	The jePose is set by applying a motion at a specific time.  This queries the motion
	to determine each joint's change and applies them to the hierarchy.  Each joint can
	then be queried for it's world transform (for drawing, etc.)

	Additional motions can modify or be blended into the pose.  A motion that describes 
	only a few joint changes can be applied to only those joints, or a motion can be
	blended with the current pose. 

	Something to watch for:  since setting the pose by applying a motion is powerful
	enough to resolve intentionally mismatched motion-pose sets, this can lead to 
	problems if the motion UNintentionally does not match the pose.  Use 
	jePose_MatchesjeMotionExactly() to test for an exact name-based match.
	

*/

#include <stdio.h>
#include "Motion.h"
#include "XFArray.h"

#ifdef __cplusplus
extern "C" {
#endif


#define JE_POSE_ROOT_JOINT (-1)

typedef enum 
{
		JE_POSE_BLEND_LINEAR,
		JE_POSE_BLEND_HERMITE
} jePose_BlendingType;

typedef struct jePose jePose;

	// Creates a new pose with no joints.
jePose *JETCF jePose_Create(void);

	// Destroys an existing pose.
void JETCF jePose_Destroy(jePose **PM);

	// Adds a new joint to a pose.
jeBoolean JETCF jePose_AddJoint(
	jePose *P,
	int ParentJointIndex,
	const char *JointName,
	const jeXForm3d *Attachment,
	int *JointIndex);


void JETCF jePose_GetScale(const jePose *P, jeVec3d *Scale);
	// Retrieves current joint attachment scaling factors

void JETCF jePose_SetScale(jePose *P, const jeVec3d *Scale);
	// Scales all joint attachments by component scaling factors in Scale

	// Returns the index of a joint named JointName.  Returns JE_TRUE if it is
	// located, and Index is set.  Returns JE_FALSE if not, and Index is not changed.
jeBoolean JETCF jePose_FindNamedJointIndex(const jePose *P, const char *JointName, int *Index);

	// returns the number of joints in the pose
int JETCF jePose_GetJointCount(const jePose *P);

jeBoolean JETCF jePose_MatchesMotionExactly(const jePose *P, const jeMotion *M);

void JETCF jePose_Clear(jePose *P, const jeXForm3d *Transform);

	// set the pose according to a motion.  Use the motion at time 'Time'.
	// if the motion does not describe motion for all joints, name-based resolution
	// will be used to decide which motion to attach to which joints.
	// joints that are unaffected are unchanged.
	// if Transform is non-NULL, it is applied to the Motion
void JETCF jePose_SetMotion(jePose *P, const jeMotion *M,jeFloat Time,const jeXForm3d *Transform);

	// optimization:  if this is called, then all pose computations are limited to the BoneIndex'th bone, and
	// it's parents (including the root bone).  This is true for all queries until an entire motion is set or blended
	// into the pose.
void JETCF jePose_SetMotionForABone(jePose *P, const jeMotion *M, jeFloat Time,
							const jeXForm3d *Transform,int BoneIndex);


	// blend in the pose according to a motion.  Use the motion at time 'Time'.
	// the blending is between the 'current' pose and the pose described by the motion.
	// a BlendAmount of 0 will result in the 'current' pose, and a BlendAmount of 1.0
	// will result in the pose according to the new motion.
	// if the motion does not describe motion for all joints, name-based resolution
	// will be used to decide which motion to attach to which joints.
	// joints that are unaffected are unchanged.
	// if Transform is non-NULL, it is applied to the Motion prior to blending
void JETCF jePose_BlendMotion(jePose *P, const jeMotion *M, jeFloat Time, 
					const jeXForm3d *Transform,
					jeFloat BlendAmount, jePose_BlendingType BlendingType);

	// get a joint's current transform (relative to world space)
void JETCF jePose_GetJointTransform(const jePose *P, int JointIndex,jeXForm3d *Transform);

	// get the transforms for the entire pose. *TransformArray must not be changed.
const jeXFArray *JETCF jePose_GetAllJointTransforms(const jePose *P);

	// query a joint's current transform relative to it's attachment to it's parent.
void JETCF jePose_GetJointLocalTransform(const jePose *P, int JointIndex,jeXForm3d *Transform);

	// adjust a joint's current transform relative to it's attachment to it's parent.
	//   this is like setting a mini-motion into this joint only:  this will only affect
	//   the current pose 
void JETCF jePose_SetJointLocalTransform(jePose *P, int JointIndex,const jeXForm3d *Transform);

	// query how a joint is attached to it's parent. (it's base attachment)
void JETCF jePose_GetJointAttachment(const jePose *P,int JointIndex,jeXForm3d *AttachmentTransform);

	// adjust how a joint is attached to it's parent.  These changes are permanent:  all
	//  future pose motions will incorporate this joint's new relation to it's parent */
void JETCF jePose_SetJointAttachment(jePose *P,int JointIndex,const jeXForm3d *AttachmentTransform);

const char* JETCF jePose_GetJointName(const jePose* P, int JointIndex);

jeBoolean JETCF jePose_Attach(jePose *Slave, int SlaveBoneIndex,
				  jePose *Master, int MasterBoneIndex, 
				  const jeXForm3d *Attachment);

void JETCF jePose_Detach(jePose *P);

	// a pose can also maintain a record of which joints are touched by a given motion.
	// these funtions set,clear and query the record.
	// ClearCoverage clears the coverage flag for all joints 
void JETCF jePose_ClearCoverage(jePose *P, int ClearTo);
	// AccumulateCoverage returns the number of joints that are not already 'covered' 
	// that will be affected by a motion M,  
	// if QueryOnly is JE_FALSE, affected joints are tagged as 'covered', otherwise no changes
	// are made to the joint coverage flags.
int JETCF jePose_AccumulateCoverage(jePose *P, const jeMotion *M, jeBoolean QueryOnly);


#ifdef __cplusplus
}
#endif


#endif
