//**************************************************************************
//  drill.c
//     example code to demonstrate how to build an actor.
//   
//     Recursively creates child bones for the actor.  Each child bone looks like the parent
//     but a little smaller and positioned nearby.  Result is a cool 'procedural geometry'.
// 
//     recursion aside: the point is to show off how simple it is to construct an actor:
//     create an actor definition (multiple actor instances can be made from one definition)
//     create a body, add textures and faces to it.  
//     add the body to the actor definition.
//     create motions to animate the bones in the body.
//       A motion can be quite free-form, but the simplest is to have a path for each bone 
//       (named the same as the bone).  The path has keyframes arbitrarily spaced in time 
//       that describe the positions/rotations for that bone.
//     add the motions to the actor definition.
//     (save the actor definition)
//     ok. you are done.
// 
//		Mike Sandige 9/3/1998
//
//
//		Updated to support VFile, new bitmaps, and minor changes in _Add method convention.  11/10/1998
//
#include <assert.h>
#include <math.h>  
#include <stdlib.h>
#include <string.h>

#pragma warning(disable : 4201 4214 4115)
#include <windows.h>
#include <mmsystem.h>
#include "genesis.h" 
#pragma warning(default : 4201 4214 4115; disable : 4514)

#include "drill.h"
#include "body.h"
#include "motion.h"

#include "Quatern.h"
#include "ram.h"
#include "actor.h"
#include "vfile.h"
#include "bitmap.h"

// for fun try #define INSERT_TRANSLATIONS


#define	M_PI	3.14159265358979323846f


static int leaves = 0;


static geBoolean Drill_InsertSomeTranslations(gePath *p,
	geFloat sizex,geFloat sizey,geFloat sizez,
	geFloat px,geFloat py,geFloat pz, geFloat TimeOffset
	)
{		
	geXForm3d M;
	geFloat x,y,z;
	geFloat t;
	geFloat TotalTime = 100.0f;
	geFloat CycleTime;

	
	assert (p);

	CycleTime = 4.0f;
	for (t=0.0f; t<TotalTime; t+=0.5f )
		{
			z = (float)sin( (t+TimeOffset) * 2.0f * M_PI/ CycleTime ) * 0.5f * sizez;
			y = 0.0f;
			x = 0.0f;

			geXForm3d_SetTranslation(&M,x,y,z);
			// add a new key frame to the path (translation key only)
			if (gePath_InsertKeyframe(p,GE_PATH_TRANSLATION_CHANNEL,t,&M)==GE_FALSE)
				return GE_FALSE;
			
		}
	return GE_TRUE;
}



static geBoolean Drill_InsertSomeRotations(gePath *p,geFloat speed,float Direction,
								geFloat sizex, geFloat sizey,geFloat sizez)
{		
	geXForm3d M;
	geBoolean worked;

	assert(p);

	srand(234);


		// want the drill to animate for quite a while.
		// loop through several 'cycles' doing slightly different things.
		{	
			geQuaternion Q;
			geFloat t,dt;
			geVec3d Axis;
			geFloat TotalTime;
			geFloat CycleTime;
			geFloat LoopTime;

			#if 0		
				{		// this is fun, it's not a drill anymore (in one plane), but it's fun
					Axis.X = (geFloat)rand();
					Axis.Y = (geFloat)rand();
					Axis.Z = (geFloat)rand();
					geVec3d_Normalize(&Axis);
				}
			#else
				{
					Axis.X =0.0F;
					Axis.Y =0.0f;
					Axis.Z =1.0F;
				}
			#endif

			
			
			LoopTime = 10.0f;
			TotalTime = LoopTime;
			CycleTime = LoopTime / (1.0f + (float)fabs(speed));
			dt = CycleTime / 4.0f;

			// cycle with a simple rotation (direction depends on level of recursion)
			for (t=0.0f;t<=TotalTime; t+=dt)
				{
						// prepare a keyframe with angular rotation
					geFloat Angle = Direction * t * 2.0f * M_PI/CycleTime;
						
					geQuaternion_SetFromAxisAngle(&Q, &Axis, Angle );
					geQuaternion_ToMatrix(&Q,&M);
						// add a rotational keyframe to path p
					worked = gePath_InsertKeyframe(p,GE_PATH_ROTATION_CHANNEL,t,&M);
					if (worked == GE_FALSE)
						return GE_FALSE;
				}
			// cycle with random rotations
			TotalTime += 2.0f * LoopTime;
			for ( ;t<=TotalTime; t+=dt)
				{
						// prepare a keyframe with angular rotation
					geFloat Angle =  2.0f * M_PI * ((float)(rand()%100))/100.0f;
						
					geQuaternion_SetFromAxisAngle(&Q, &Axis, Angle );
					geQuaternion_ToMatrix(&Q,&M);
						// add a rotational keyframe to path p
					worked=gePath_InsertKeyframe(p,GE_PATH_ROTATION_CHANNEL,t,&M);
					if (worked == GE_FALSE)
						return GE_FALSE;
				}
			
			// cycle with more random rotations
			srand(timeGetTime());
			TotalTime +=  2.0f * LoopTime;
			for ( ;t<=TotalTime; t+=dt)
				{
					geFloat Angle =  2.0f * M_PI * ((float)(rand()%100))/100.0f;
						
					geQuaternion_SetFromAxisAngle(&Q, &Axis, Angle );
					geQuaternion_ToMatrix(&Q,&M);
						// add a rotational keyframe to path p
					worked=gePath_InsertKeyframe(p,GE_PATH_ROTATION_CHANNEL,t,&M);
					if (worked == GE_FALSE)
						return GE_FALSE;
				}

			// cycle with a simple rotation (direction depends on level of recursion)
			TotalTime += LoopTime;
			for (;t<=TotalTime; t+=dt)
				{
					geFloat Angle = Direction * t * 2.0f * M_PI/CycleTime;
						
					geQuaternion_SetFromAxisAngle(&Q, &Axis, Angle );
					geQuaternion_ToMatrix(&Q,&M);
						// add a rotational keyframe to path p
					worked=gePath_InsertKeyframe(p,GE_PATH_ROTATION_CHANNEL,t,&M);
					if (worked == GE_FALSE)
						return GE_FALSE;

				}
		}

	return GE_TRUE;	
}


geBoolean Drill_RecursiveCreate(
	geBody *Body,
	geMotion *Motion,
	geFloat sizex,
	geFloat sizey,
	geFloat sizez,
	geFloat px,geFloat py,geFloat pz,
	int children,
	int slerp,
	float Speed,
	int depth,
	int MyChildNumber,
	int ParentJointIndex)
{
	geXForm3d M;
	char name[100];
	int JointIndex;
	geFloat X1,X2,Y,Z;
	geVec3d V1,V2,V3,Tip;
	geFloat Direction;
	geBoolean worked;

	
	assert( Body );
	assert( Motion );
	
	X1 = 1.0f  * sizex;
	X2 = 0.5f  * sizex;
	Y  = 0.866f * sizey;
	Z  = 1.0f   * sizez;

	geVec3d_Set(&Tip,0.0f,0.0f,Z);
	geVec3d_Set(&V1, X1, 0.0f,0.0f);
	geVec3d_Set(&V2,-X2, Y   ,0.0f);
	geVec3d_Set(&V3,-X2,-Y   ,0.0f);

	leaves++;
	
	geXForm3d_SetTranslation(&M,px,py,pz);

	if (depth%2 == 0)
		{
			Direction = 1 ;
		}
	else
		{
			Direction = -1;
		}

	Speed = Speed+1.0f;

	sprintf(name,"Joint%d",(int)leaves);

	{	
			// add a new bone to the body.  Attach it to it's parent bone with an identity transform.
		worked = geBody_AddBone(Body,
				ParentJointIndex, 	name,
				&M,&(JointIndex));
		if (worked == GE_FALSE)
				return GE_FALSE;
		{
			geVec3d NV1,NV2,NV3;
			geVec3d N;
			
	
			NV1 = V1;
			NV2 = V2;
			NV3 = V3;

			geVec3d_Normalize(&NV1);
			geVec3d_Normalize(&NV2);
			geVec3d_Normalize(&NV3);

			 // add the faces for the pointy part of the drill bit.  
			 // here it's a simple 4 sided pyramid.
			 // each vertex is attached to a particular bone, with a given position relative
			 // to that bone.  That's key here: vertex positions are in the coordinate system of the 
			 // bone it's attached to.  
			 // each vertex can have it's own normal.  Here, to simulate 'roundness' as much as
			 // possible, the normals are set to face away from the axis of the pyramid.
			 //   the normals are used for shading on a per-vertex basis.  use different normals
			 //   with adjacent faces to get abrupt changes in shading.  (for corners, or to
			 //   simulate edges or folds in the geometry)
			 // each vertex also has a texture u and v.
			 // 
			geVec3d_Scale(&NV3,-1.0f, &N);
			worked = geBody_AddFace(Body,
				&V2, &NV2,0.0f,1.0f,JointIndex,
				&Tip,&N, 0.5f,0.5f,JointIndex,
				&V1, &NV1,1.0f,1.0f,JointIndex,0);
			if (worked == GE_FALSE)
				return GE_FALSE;


			geVec3d_Scale(&NV1,-1.0f, &N);
			worked = geBody_AddFace(Body,
				&V3, &NV3,0.0f,0.0f,JointIndex,
				&Tip,&N, 0.5f,0.5f,JointIndex,
				&V2, &NV2,0.0f,1.0f,JointIndex,1);
			if (worked == GE_FALSE)
				return GE_FALSE;

			geVec3d_Scale(&NV2,-1.0f, &N);
			worked = geBody_AddFace(Body,
				&V1, &NV1,1.0f,0.0f,JointIndex,
				&Tip,&N, 0.5f,0.5f,JointIndex,
				&V3, &NV3,0.0f,0.0f,JointIndex,2);
			if (worked == GE_FALSE)
				return GE_FALSE;

			geVec3d_Set(&N,0.0f,0.0f,-1.0f);
			worked = geBody_AddFace(Body,
				&V3, &N,0.5f,0.5f,JointIndex,
				&V2, &N,0.5f,0.5f,JointIndex,
				&V1, &N,0.5f,0.5f,JointIndex,3);
			if (worked == GE_FALSE)
				return GE_FALSE;


			#if 1
			// some cool additional geometry to hook the bit to it's parent.
			//  (each vertex can be hooked to different bones)
			// to see the recursion more clearly, you can disable this section.
			if (depth>0)
				{
					geVec3d_Scale(&NV3,-1.0f, &N);
					worked = geBody_AddFace(Body,
						&V2, &NV2,0.0f,1.0f,JointIndex,
						&Tip,&N, 0.5f,0.5f,ParentJointIndex,
						&V1, &NV1,1.0f,1.0f,JointIndex,0);
					if (worked == GE_FALSE)
						return GE_FALSE;
						
					geVec3d_Scale(&NV1,-1.0f, &N);
					worked = geBody_AddFace(Body,
						&V3, &NV3,0.0f,0.0f,JointIndex,
						&Tip,&N, 0.5f,0.5f,ParentJointIndex,
						&V2, &NV2,0.0f,1.0f,JointIndex,1);
					if (worked == GE_FALSE)
						return GE_FALSE;

					geVec3d_Scale(&NV2,-1.0f, &N);
					worked = geBody_AddFace(Body,
						&V1, &NV1,1.0f,0.0f,JointIndex,
						&Tip,&N, 0.5f,0.5f,ParentJointIndex,
						&V3, &NV3,0.0f,0.0f,JointIndex,2);
					if (worked == GE_FALSE)
						return GE_FALSE;

					geVec3d_Set(&N,0.0f,0.0f,-1.0f);
					worked = geBody_AddFace(Body,
						&V3, &N,0.5f,0.5f,JointIndex,
						&V2, &N,0.5f,0.5f,JointIndex,
						&V1, &N,0.5f,0.5f,JointIndex,3);
					if (worked == GE_FALSE)
						return GE_FALSE;

				}
			#endif
		}
	 // The bone system is hierarchical - 
	 // each bone's position is relative to it's parent bone.  So create 3 bones, 
	 // (with no rotation keys, and no rotation in their attachments to their parents.)  
	 // Then give each bone a translational offset key of 5 in X.  The first bone will be 5 in X 
	 // from the 'origin'.  the second bone will be 5 in X from the first bone (or 10 from
	 // the 'origin'.  The third bone will be 5 from the second bone 
	 // (or 15 from the 'origin' of the system.)  You can see how each child bone inherits hte
	 // translation from it's parent.  
	 //  It's similar with rotations:  Rotate the first bone in that system by 90 degrees around Z=0.
	 // Now the first bone is offset by 5 in Y.  The second and third bones are also offset in Y, since
	 // they were rotated along with their parent.
	 

		{
			int nothing;
			gePath *P;

				// add a new path to the motion, for this new bone.

			P = gePath_Create(GE_PATH_INTERPOLATE_HERMITE,
						slerp?GE_PATH_INTERPOLATE_SLERP:GE_PATH_INTERPOLATE_SQUAD,
						GE_TRUE);
			if ( P == NULL)
				return GE_FALSE;

			worked = geMotion_AddPath(Motion,P,name,&nothing);
			gePath_Destroy(&P);
			if (worked == GE_FALSE)
				return GE_FALSE;
				
				// pepper the new path with rotational keyframes
			worked = Drill_InsertSomeRotations(geMotion_GetPath(Motion,JointIndex),Speed,Direction,
									sizex,sizey,sizez);
			if (worked == GE_FALSE)
				return GE_FALSE;


			#ifdef INSERT_TRANSLATIONS
				// salt the new path with translation keyframes.  This part breaks the illusion
				// of a drill bit, so it's commented out.  You can try it to see what happens.
				if (depth>0)
					{	// avoid moving the root bone
						Drill_InsertSomeTranslations(geMotion_GetPath(Motion,JointIndex),
							sizex,sizey,sizez,px,py,pz,
							3.0f * (float)(MyChildNumber%3)
							);
					}
			#endif
		}
	}

	#define SIZE_REDUCTION_PER_ITERATION (0.7f)
	sizex *= SIZE_REDUCTION_PER_ITERATION;
 	sizey *= SIZE_REDUCTION_PER_ITERATION;
	sizez *= SIZE_REDUCTION_PER_ITERATION;


	if (children > 0)
		{

			worked = Drill_RecursiveCreate(
				Body, Motion, sizex, sizey, sizez,
				-V1.X,-V1.Y,-V1.Z,
				children-1,slerp,Speed,depth+1,MyChildNumber+1,JointIndex);
			if (worked == GE_FALSE)
				return GE_FALSE;

			worked = Drill_RecursiveCreate(
				Body, Motion, sizex, sizey, sizez,
				-V2.X,-V2.Y,-V2.Z,
				children-1,slerp,Speed,depth+1,MyChildNumber+2,JointIndex);
			if (worked == GE_FALSE)
				return GE_FALSE;

			worked = Drill_RecursiveCreate(
				Body, Motion, sizex, sizey, sizez,
				-V3.X,-V3.Y,-V3.Z,
				children-1,slerp,Speed,depth+1,MyChildNumber+3,JointIndex);
			if (worked == GE_FALSE)
				return GE_FALSE;
		
		
		}

	return GE_TRUE;
}





geBoolean Drill_Create(const char *Name, geFloat sizex,geFloat sizey, geFloat sizez)
{
	geBoolean Result;
	geMotion *Motion = NULL;
	geActor_Def *Drill_ActorDef = NULL;
	geBody *Drill_Body = NULL;
	int i;

	extern geTextLib_Texture *Texture;

	int Index;

	
		// build the 'body' for the drill actor
	Drill_Body = geBody_Create();
	if (Drill_Body == NULL)	return GE_FALSE;


	Result = geBody_AddMaterial(Drill_Body,"drillbit1",NULL,255.0f,255.0f  ,255.0f,&Index);
	if (Result == GE_FALSE)	{geBody_Destroy(&Drill_Body); return GE_FALSE;}
	Result = geBody_AddMaterial(Drill_Body,"drillbit2",NULL,255.0f,255.0f  ,255.0f,&Index);
	if (Result == GE_FALSE)	{geBody_Destroy(&Drill_Body); return GE_FALSE;}
	Result = geBody_AddMaterial(Drill_Body,"drillbit3",NULL,255.0f,255.0f  ,255.0f,&Index);
	if (Result == GE_FALSE)	{geBody_Destroy(&Drill_Body); return GE_FALSE;}
	Result = geBody_AddMaterial(Drill_Body,"drillbit4",NULL,255.0f,255.0f  ,255.0f,&Index);
	if (Result == GE_FALSE)	{geBody_Destroy(&Drill_Body); return GE_FALSE;}

		// build one motion for the drill actor 
	Motion = geMotion_Create(GE_TRUE);
	geMotion_SetName(Motion,"1");
	if (Motion == NULL)
		{
			geBody_Destroy(&Drill_Body);		// clean up the body
			return GE_FALSE;
		}

		// recursively add bones and paths to the drill
	Result = Drill_RecursiveCreate( Drill_Body, Motion,
		sizex, sizey, sizez, 
		0.0f,0.0f,0.0f, 
		4, 0, 
		1.0f,0,
		0,
		GE_BODY_ROOT);
	if (Result == GE_FALSE)
		{
			geBody_Destroy(&Drill_Body);		// clean up the body
			geMotion_Destroy(&Motion);			// clean up the motion
			return GE_FALSE;
		}

		// create an actor definition for the drill
	Drill_ActorDef = geActor_DefCreate();
	if ( Drill_ActorDef == NULL )
		{
			geBody_Destroy(&Drill_Body);		// clean up the body
			geMotion_Destroy(&Motion);			// clean up the motion
			return GE_FALSE;
		}

		// set the body of the drill's actor definition to the prepared body
	Result = geActor_SetBody(Drill_ActorDef,Drill_Body);
	if ( Result == GE_FALSE )
		{
			geBody_Destroy(&Drill_Body);		// clean up the body
			geMotion_Destroy(&Motion);			// clean up the motion
			geActor_DefDestroy(&Drill_ActorDef);	// clean up the actor definition
			return GE_FALSE;
		}
	
	for (i=0; i<4; i++)
	{
		geBitmap *Bmp;
		geVFile *VF;
		VF = geVFile_OpenNewSystem(NULL,GE_VFILE_TYPE_DOS,"maps\\drillbit.bmp",NULL,GE_VFILE_OPEN_READONLY);
		Bmp = geBitmap_CreateFromFile(VF);
		geVFile_Close(VF);
		if (Bmp == NULL)
			{
				geBody_Destroy(&Drill_Body);		// clean up the body
				geMotion_Destroy(&Motion);			// clean up the motion
				geActor_DefDestroy(&Drill_ActorDef);	// clean up the actor definition
				return GE_FALSE;
			}
		
		Result = geBody_SetMaterial(Drill_Body,i,Bmp,(255.0f/4.0f)*(float)i,255.0f,255.0f);
		geBitmap_Destroy(&Bmp);
		if (Result == GE_FALSE)
			{
				geBody_Destroy(&Drill_Body);		// clean up the body
				geMotion_Destroy(&Motion);			// clean up the motion
				geActor_DefDestroy(&Drill_ActorDef);	// clean up the actor definition
				return GE_FALSE;
			}
	}
		


		// add the prepared motion to the drill's actor definition
	Result = geActor_AddMotion(Drill_ActorDef,Motion,&Index);
	if ( Result == GE_FALSE )
		{
			// don't need to clean up the body: it's a part of the actor definition now
			geMotion_Destroy(&Motion);				// clean up the motion
			geActor_DefDestroy(&Drill_ActorDef);	// clean up the actor definition
			return GE_FALSE;
		}
		

		// write the new actor to a the named file
	{
		geVFile *VF = geVFile_OpenNewSystem(NULL,GE_VFILE_TYPE_DOS,Name,NULL,GE_VFILE_OPEN_CREATE);
		Result = GE_FALSE;
		
		if (VF!=NULL)
			{
				Result = geActor_DefWriteToFile(Drill_ActorDef,VF);
				if (Result == GE_FALSE)
					{
						geMotion_Destroy(&Motion);				// clean up the motion
						geActor_DefDestroy(&Drill_ActorDef);	// clean up the actor definition
						return GE_FALSE;
					}
						
				if (geVFile_Close(VF)==GE_FALSE)
					{
						geMotion_Destroy(&Motion);				// clean up the motion
						geActor_DefDestroy(&Drill_ActorDef);	// clean up the actor definition
						return GE_FALSE;
					}

			}
	}
	

	// don't need to clean up the body: it's a part of the actor definition now
	// don't need to clean up the motion: it's a part of the actor definition now
	geActor_DefDestroy(&Drill_ActorDef);		// clean up the actor definition
	
	return GE_TRUE;
}


