/****************************************************************************************/
/*  QKFRAME.H																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Quaternion keyframe implementation.									*/
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
/* jeQKFrame   (jeQuaternion - Keyframe)
	This module handles interpolation for keyframes that contain a quaternion
	This is intended to support Path.c
	jeTKArray supplies general support for a time-keyed array, and this supplements
	that support to include the specific time-keyed arrays:
	  An array of jeQuaternion interpolated linearly
	  An array of jeQuaternion with spherical linear interpolation (SLERP)
	  An array of jeQuaternion with spherical quadrangle 
		interpolation (SQUAD) as defined by:
	    Advanced Animation and Rendering Techniques by Alan Watt and Mark Watt

	These are phycially separated and have different base structures because
	the different interpolation techniques requre different additional data.
	
	The two lists are created with different creation calls,
	interpolated with different calls, but insertion and queries share a call.
	
	Quadrangle interpolation requires additional computation after changes are
	made to the keyframe list.  Call jeQKFrame_SquadRecompute() to update the
	calculations.
*/
#include <assert.h>

#include "Vec3d.h"
#include "QKFrame.h"
#include "Errorlog.h"
#include "Ram.h"

#define LINEAR_BLEND(a,b,t)  ( (t)*((b)-(a)) + (a) )	
			// linear blend of a and b  0<t<1 where  t=0 ->a and t=1 ->b

typedef struct
{
	jeTKArray_TimeType	Time;				// Time for this keyframe
	jeQuaternion	Q;					// quaternion for this keyframe
}  QKeyframe;		
	// This is the root structure that jeQKFrame supports
	// all keyframe types must begin with this structure.  Time is first, so
	// that this structure can be manipulated by jeTKArray

typedef struct
{
	QKeyframe Key;				// key values for this keyframe
}	jeQKFrame_Linear;
	// keyframe data for linear interpolation
	// The structure includes no additional information.

typedef struct
{
	QKeyframe Key;				// key values for this keyframe
}	jeQKFrame_Slerp;
	// keyframe data for spherical linear interpolation
	// The structure includes no additional information.

typedef struct
{
	QKeyframe Key;				// key values for this keyframe
	jeQuaternion  QuadrangleCorner;	
}	jeQKFrame_Squad;
	// keyframe data for spherical quadratic interpolation


jeTKArray *JETCC jeQKFrame_LinearCreate()
	// creates a frame list for linear interpolation
{
	return jeTKArray_Create(sizeof(jeQKFrame_Linear) );
}


jeTKArray *JETCC jeQKFrame_SlerpCreate()
	// creates a frame list for spherical linear interpolation	
{
	return jeTKArray_Create(sizeof(jeQKFrame_Slerp) );
}

jeTKArray *JETCC jeQKFrame_SquadCreate()
	// creates a frame list for spherical linear interpolation	
{
	return jeTKArray_Create(sizeof(jeQKFrame_Squad) );
}


jeBoolean JETCC jeQKFrame_Insert(
	jeTKArray **KeyList,			// keyframe list to insert into
	jeTKArray_TimeType Time,		// time of new keyframe
	const jeQuaternion *Q,			// quaternion at new keyframe
	int *Index)						// index of new key
	// inserts a new keyframe with the given time and vector into the list.
{
	assert( KeyList != NULL );
	assert( *KeyList != NULL );
	assert( Q != NULL );
	assert(   sizeof(jeQKFrame_Squad) == jeTKArray_ElementSize(*KeyList) 
	       || sizeof(jeQKFrame_Slerp) == jeTKArray_ElementSize(*KeyList) 
		   || sizeof(jeQKFrame_Linear) == jeTKArray_ElementSize(*KeyList) );

	if (jeTKArray_Insert(KeyList, Time, Index) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeQKFrame_Insert: jeTKArray_Insert failed.");
			return JE_FALSE;
		}
	else
		{
			QKeyframe *KF;
			KF = (QKeyframe *)jeTKArray_Element(*KeyList,*Index);
			KF->Q = *Q;
			return JE_TRUE;
		}
}

void JETCC jeQKFrame_Query(
	const jeTKArray *KeyList,		// keyframe list
	int Index,						// index of frame to return
	jeTKArray_TimeType *Time,		// time of the frame is returned
	jeQuaternion *Q)					// vector from the frame is returned
	// returns the vector and the time at keyframe[index] 
{
	QKeyframe *KF;
	assert( KeyList != NULL );
	assert( Time != NULL );
	assert( Q != NULL );
	assert( Index < jeTKArray_NumElements(KeyList) );
	assert( Index >= 0 );
	assert(   sizeof(jeQKFrame_Squad) == jeTKArray_ElementSize(KeyList) 
	       || sizeof(jeQKFrame_Slerp) == jeTKArray_ElementSize(KeyList) 
		   || sizeof(jeQKFrame_Linear) == jeTKArray_ElementSize(KeyList) );
	
	KF = (QKeyframe *)jeTKArray_Element(KeyList,Index);
	*Time = KF->Time;
	*Q    = KF->Q;
}

void JETCC jeQKFrame_Modify(
	jeTKArray *KeyList,				// keyframe list
	int Index,						// index of frame to change
	const jeQuaternion *Q)			// vector for the new key
{
	QKeyframe *KF;
	assert( KeyList != NULL );
	assert( Q != NULL );
	assert( Index < jeTKArray_NumElements(KeyList) );
	assert( Index >= 0 );
	assert(   sizeof(jeQKFrame_Squad) == jeTKArray_ElementSize(KeyList) 
	       || sizeof(jeQKFrame_Slerp) == jeTKArray_ElementSize(KeyList) 
		   || sizeof(jeQKFrame_Linear) == jeTKArray_ElementSize(KeyList) );
	
	KF = (QKeyframe *)jeTKArray_Element(KeyList,Index);
	KF->Q  = *Q;
}



void JETCC jeQKFrame_LinearInterpolation(
	const void *KF1,		// pointer to first keyframe
	const void *KF2,		// pointer to second keyframe
	jeFloat T,				// 0 <= T <= 1   blending parameter
	void *Result)			// put the result in here (jeQuaternion)
		// interpolates to get a vector between the two vectors at the two
		// keyframes where T==0 returns the vector for KF1 
		// and T==1 returns the vector for KF2
		// interpolates linearly
{
	jeQuaternion *Q1,*Q2;
	jeQuaternion *QNew = (jeQuaternion *)Result;
	
	assert( Result != NULL );
	assert( KF1 != NULL );
	assert( KF2 != NULL );
	
	assert( T >= (jeFloat)0.0f );
	assert( T <= (jeFloat)1.0f );
	
	if ( KF1 == KF2 )
		{
			*QNew = ((jeQKFrame_Linear *)KF1)->Key.Q;
			return;
		}

	Q1 = &( ((jeQKFrame_Linear *)KF1)->Key.Q);
	Q2 = &( ((jeQKFrame_Linear *)KF2)->Key.Q);
	
	QNew->X = LINEAR_BLEND(Q1->X,Q2->X,T);
	QNew->Y = LINEAR_BLEND(Q1->Y,Q2->Y,T);
	QNew->Z = LINEAR_BLEND(Q1->Z,Q2->Z,T);
	QNew->W = LINEAR_BLEND(Q1->W,Q2->W,T);
	if (jeQuaternion_Normalize(QNew)==0.0f)
		{
			jeQuaternion_SetNoRotation(QNew);
		}

}



void JETCC jeQKFrame_SlerpInterpolation(
	const void *KF1,		// pointer to first keyframe
	const void *KF2,		// pointer to second keyframe
	jeFloat T,				// 0 <= T <= 1   blending parameter
	void *Result)			// put the result in here (jeQuaternion)
		// interpolates to get a vector between the two vectors at the two
		// keyframes where T==0 returns the vector for KF1 
		// and T==1 returns the vector for KF2
		// interpolates using spherical linear blending
{
	jeQuaternion *Q1,*Q2;
	jeQuaternion *QNew = (jeQuaternion *)Result;
	
	assert( Result != NULL );
	assert( KF1 != NULL );
	assert( KF2 != NULL );
	
	assert( T >= (jeFloat)0.0f );
	assert( T <= (jeFloat)1.0f );
	
	if ( KF1 == KF2 )
		{
			*QNew = ((jeQKFrame_Slerp *)KF1)->Key.Q;
			return;
		}
 
	Q1 = &( ((jeQKFrame_Slerp *)KF1)->Key.Q);
	Q2 = &( ((jeQKFrame_Slerp *)KF2)->Key.Q);
	jeQuaternion_SlerpNotShortest(Q1,Q2,T,QNew);
}




void JETCC jeQKFrame_SquadInterpolation(
	const void *KF1,		// pointer to first keyframe
	const void *KF2,		// pointer to second keyframe
	jeFloat T,				// 0 <= T <= 1   blending parameter
	void *Result)			// put the result in here (jeQuaternion)
		// interpolates to get a vector between the two vectors at the two
		// keyframes where T==0 returns the vector for KF1 
		// and T==1 returns the vector for KF2
		// interpolates using spherical quadratic blending
{
	jeQuaternion *Q1,*Q2;
	jeQuaternion *QNew = (jeQuaternion *)Result;
	
	assert( Result != NULL );
	assert( KF1 != NULL );
	assert( KF2 != NULL );
	
	assert( T >= (jeFloat)0.0f );
	assert( T <= (jeFloat)1.0f );
	
	if ( KF1 == KF2 )
		{
			*QNew = ((jeQKFrame_Squad *)KF1)->Key.Q;
			return;
		}

	Q1 = &( ((jeQKFrame_Squad *)KF1)->Key.Q);
	Q2 = &( ((jeQKFrame_Squad *)KF2)->Key.Q);
	
	{
		jeQuaternion *A1,*B2;
		jeQuaternion SL1,SL2;
				
		A1 = &( ((jeQKFrame_Squad *)KF1)->QuadrangleCorner);
		B2 = &( ((jeQKFrame_Squad *)KF2)->QuadrangleCorner);

		jeQuaternion_SlerpNotShortest(Q1,   Q2,   T, &SL1);
				assert( jeQuaternion_IsUnit(&SL1) == JE_TRUE);
		jeQuaternion_SlerpNotShortest(A1,   B2,   T, &SL2);
				assert( jeQuaternion_IsUnit(&SL2) == JE_TRUE);
		jeQuaternion_SlerpNotShortest(&SL1, &SL2, (2.0f*T*(1.0f-T)), QNew);
				assert( jeQuaternion_IsUnit(QNew) == JE_TRUE);
	}
}


static void JETCC jeQKFrame_QuadrangleCorner(
	const jeQuaternion *Q0,
	const jeQuaternion *Q1,
	const jeQuaternion *Q2,
	jeQuaternion *Corner)
	// compute quadrangle corner for a keyframe containing Q1.
	//  Q0 and Q2 are the quaternions for the previous and next keyframes 
	// corner is the newly computed quaternion
{
	jeQuaternion Q1Inv,LnSum;

	assert( Q0 != NULL );
	assert( Q1 != NULL );
	assert( Q2 != NULL );
	assert( Corner != NULL );

	assert( jeQuaternion_IsUnit(Q1) == JE_TRUE );

	Q1Inv.W = Q1->W;
	Q1Inv.X = -Q1->X;
	Q1Inv.Y = -Q1->Y;
	Q1Inv.Z = -Q1->Z;
				
	{
		jeQuaternion Q1InvQ2, Q1InvQ0;
		jeQuaternion Ln1,Ln2;

		jeQuaternion_Multiply(&Q1Inv,Q2,&Q1InvQ2);
		jeQuaternion_Multiply(&Q1Inv,Q0,&Q1InvQ0);
		jeQuaternion_Ln(&Q1InvQ0,&Ln1);
		jeQuaternion_Ln(&Q1InvQ2,&Ln2);
		jeQuaternion_Add(&Ln1,&Ln2,&LnSum);
		jeQuaternion_Scale(&LnSum,-0.25f,&LnSum);
	}

	jeQuaternion_Exp(&LnSum,Corner);
	jeQuaternion_Multiply(Q1,Corner,Corner);
}

static void JETCC jeQKFrame_ChooseBestQuat(const jeQuaternion *Q0,jeQuaternion *Q1)
	// adjusts the sign of Q1:  to either Q1 or -Q1
	// adjusts Q1 such that Q1 is the 'closest' of the two choices to Q0.
{
	jeQuaternion pLessQ,pPlusQ;
	jeFloat MagpLessQ,MagpPlusQ;

	assert( Q0 != NULL );
	assert( Q1 != NULL );
	
	jeQuaternion_Add(Q0,Q1,&pPlusQ);
	jeQuaternion_Subtract(Q0,Q1,&pLessQ);
		
	jeQuaternion_Multiply(&pPlusQ,&pPlusQ,&pPlusQ);
	jeQuaternion_Multiply(&pLessQ,&pLessQ,&pLessQ);

	MagpLessQ=   (pLessQ.W * pLessQ.W) + (pLessQ.X * pLessQ.X) 
					  + (pLessQ.Y * pLessQ.Y) + (pLessQ.Z * pLessQ.Z);

	MagpPlusQ=   (pPlusQ.W * pPlusQ.W) + (pPlusQ.X * pPlusQ.X) 
					  + (pPlusQ.Y * pPlusQ.Y) + (pPlusQ.Z * pPlusQ.Z);

	if (MagpLessQ >= MagpPlusQ)
		{
			Q1->X = -Q1->X;
			Q1->Y = -Q1->Y;
			Q1->Z = -Q1->Z;
			Q1->W = -Q1->W;
		}
}




void JETCC jeQKFrame_SquadRecompute(
	int Looped,				// if keylist has the first key connected to last key
	jeTKArray *KeyList,		// list of keys to recompute hermite values for
	jeFloat CutInterval)	// intervals <= CutInterval are to be treated as discontinuous
	// rebuild precomputed data for keyframe list.
{

	// compute the extra interpolation points at each keyframe
	// see Advanced Animation and Rendering Techniques 
	//     by Alan Watt and Mark Watt, pg 366
	int i;
	jeQKFrame_Squad *QList=NULL;
	int count;
	jeFloat T0,T1,T2;
	int Index0,Index1,Index2;
	assert( KeyList != NULL );

	count = jeTKArray_NumElements(KeyList);

	if (count > 0)
		{
			QList = (jeQKFrame_Squad *)jeTKArray_Element(KeyList,0);

			for (i =0; i< count-1; i++)
				{
					jeQKFrame_ChooseBestQuat(&(QList[i].Key.Q),&(QList[i+1].Key.Q) );
				}
		}

	if (count<3)
		{
			Looped = 0;
			// cant compute 'slopes' without enough points to loop. 
			// so treat path as non-looped.
		}
	for (i =0; i< count; i++)
		{
			Index0 = i-1;
			Index1 = i;
			Index2 = i+1;

			if (Index1 == 0)
				{
					if (Looped != JE_TRUE)
						{
							Index0 = 0;
						}
					else
						{
							Index0 = count-2;
						}
				}

			if (Index2 == count)
				{
					if (Looped != JE_TRUE)
						{
							Index2 = count-1;
						}
					else
						{
							Index2 = 1;
						}
				}
			
			T0=QList[Index0].Key.Time;
			T1=QList[Index1].Key.Time;
			T2=QList[Index2].Key.Time;

			if (( Looped != JE_TRUE) && (Index1 == 0) || (T1-T0 <= CutInterval) )
				{
					jeQuaternion_Copy(
						&(QList[i].Key.Q),
						&(QList[i].QuadrangleCorner) );
				}
			else if ((( Looped != JE_TRUE) && (Index1 == count-1)) || (T2-T1 <= CutInterval) )
				{
					jeQuaternion_Copy(
						&(QList[i].Key.Q),
						&(QList[i].QuadrangleCorner) );
				}
			else
			{
				jeQKFrame_QuadrangleCorner( 
					&(QList[Index0].Key.Q),
					&(QList[Index1].Key.Q),
					&(QList[Index2].Key.Q),
					&(QList[i].QuadrangleCorner) );
	
			}
		}	
}					



void JETCC jeQKFrame_SlerpRecompute(
	jeTKArray *KeyList)			// list of keys to recompute hermite values for

	// rebuild precomputed data for keyframe list.
	// also make sure that each successive key is the 'closest' quaternion choice
	// to the previous one.
{

	int i;
	jeQKFrame_Slerp *QList;
	int count;
	assert( KeyList != NULL );

	count = jeTKArray_NumElements(KeyList);

	if (count > 0)
		{
			QList = (jeQKFrame_Slerp  *)jeTKArray_Element(KeyList,0);
			for (i =0; i< count-1; i++)
				{
					jeQKFrame_ChooseBestQuat(&(QList[i].Key.Q),&(QList[i+1].Key.Q) );
				}
		}
}

//------------------------------------------------------------------------
#define QKFRAME_HINGE_COMPRESSION 0x1
#define QKFRAME_LINEARTIME_COMPRESSION 0x2


#define HINGE_TOLERANCE (0.0001f)
#define LINEARTIME_TOLERANCE (0.0001f)

static jeBoolean JETCC jeQKFrame_PathIsHinged(jeTKArray *KeyList, jeFloat Tolerance)
{
	int i,Count;
	jeVec3d Axis;
	jeVec3d NextAxis;
	jeFloat Angle; 
	jeQKFrame_Linear* pLinear;

	assert( KeyList != NULL );

	Count = jeTKArray_NumElements(KeyList);
	
	if (Count<2)
		return JE_FALSE;
	pLinear = (jeQKFrame_Linear*)jeTKArray_Element(KeyList, 0);
	if (jeQuaternion_GetAxisAngle(&(pLinear->Key.Q),&Axis,&Angle)==JE_FALSE)
		{
			return JE_FALSE;
		}
		
	for (i=1; i<Count; i++)
		{
			pLinear = (jeQKFrame_Linear*)jeTKArray_Element(KeyList, i);
			if (jeQuaternion_GetAxisAngle(&(pLinear->Key.Q),&NextAxis,&Angle)==JE_FALSE)
				{
					return JE_FALSE;
				}
				
			if (jeVec3d_Compare(&Axis,&NextAxis,Tolerance) == JE_FALSE)
				{	
					return JE_FALSE;
				}
		}
	return JE_TRUE;
}


static int JETCC jeQKFrame_DetermineCompressionType(jeTKArray *KeyList)
{
	int Compression=0;
	int NumElements=0;

	assert( KeyList != NULL );

	NumElements = jeTKArray_NumElements(KeyList);

	if (NumElements>2)
		{
			if ( jeTKArray_SamplesAreTimeLinear(KeyList,LINEARTIME_TOLERANCE) != JE_FALSE )
				{
					Compression |= QKFRAME_LINEARTIME_COMPRESSION;
				}
		}


	if (NumElements>3)
		{
			 if ( jeQKFrame_PathIsHinged(KeyList,HINGE_TOLERANCE)!=JE_FALSE )
				{
					Compression |= QKFRAME_HINGE_COMPRESSION;
				}
		}

	return Compression;
}





uint32 JETCC jeQKFrame_ComputeBlockSize(jeTKArray *KeyList, int Compression)
{
	uint32 Size=0;
	int Count;
	assert( KeyList != NULL );
	assert( Compression < 0xFF);
	
	Count = jeTKArray_NumElements(KeyList);

	Size += sizeof(uint32);		// flags
	Size += sizeof(uint32);		// count

	if (Compression & QKFRAME_LINEARTIME_COMPRESSION)
		{
			Size += sizeof(jeFloat) * 2;
		}
	else
		{
			Size += sizeof(jeFloat) * Count;
		}

	switch (Compression & (~QKFRAME_LINEARTIME_COMPRESSION) )
		{
			case 0:
				Size += sizeof(jeQuaternion) * Count;
				break;
			case QKFRAME_HINGE_COMPRESSION:
				Size += (sizeof(jeFloat) * 3) + sizeof(jeFloat) * Count;
				break;
			default:
				assert(0);
		}	
	return Size;
}

jeTKArray *JETCC jeQKFrame_CreateFromFile(
			jeVFile	*pFile, 
			jeQKFrame_InterpolationType		*InterpolationType, 
			int		*Looping,
			jeFloat	CutInterval)
{
	uint32 u;
	int BlockSize;
	int Compression;
	int Count,i;
	int FieldSize;
	char *Block;
	jeFloat *Data;
	jeTKArray *KeyList;
	jeQKFrame_Linear* pLinear0;
	jeQKFrame_Linear* pLinear;

	assert( pFile != NULL );
	assert( InterpolationType != NULL );
	assert( Looping != NULL );
	
	if (jeVFile_Read(pFile, &BlockSize, sizeof(int)) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_READ,"jeQKFrame_CreateFromFile: Failed to read header.");
			return NULL;
		}
	if (BlockSize<0)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_FORMAT,"jeQKFrame_CreateFromFile: Bad Blocksize.");
			return NULL;
		}
			
	Block = (char *)jeRam_AllocateClear(BlockSize);
	if(jeVFile_Read(pFile, Block, BlockSize) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_READ,"jeQKFrame_CreateFromFile.");
			return NULL;
		}
	u = *(uint32 *)Block;
	*((int *)InterpolationType) = (u>>16)& 0xFF;
	Compression = (u>>8) & 0xFF;
	*Looping           = (u & 0x1);		
	Count = *(((uint32 *)Block)+1);
	
	if (Compression > 0xFF)
		{
			jeRam_Free(Block);	
			jeErrorLog_Add(JE_ERR_FILEIO_VERSION,"jeQKFrame_CreateFromFile: Bad Compression Flag.");
			return NULL;
		}
	switch (*InterpolationType)
		{
			case (QKFRAME_LINEAR):
				FieldSize = sizeof(jeQKFrame_Linear);
				break;
			case (QKFRAME_SLERP):
				FieldSize = sizeof(jeQKFrame_Slerp);
				break;
			case (QKFRAME_SQUAD):
				FieldSize = sizeof(jeQKFrame_Squad);
				break;
			default:
				jeRam_Free(Block);
				jeErrorLog_Add(JE_ERR_FILEIO_VERSION,"jeQKFrame_CreateFromFile: Bad InterpolationType");
				return NULL;
		}
	
	KeyList = jeTKArray_CreateEmpty(FieldSize,Count);
	if (KeyList == NULL)
		{
			jeRam_Free(Block);	
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeQKFrame_CreateFromFile.");
			return NULL;
		}

	Data = (jeFloat *)(Block + sizeof(uint32)*2);
			
	pLinear0 = (jeQKFrame_Linear*)jeTKArray_Element(KeyList, 0);

	pLinear = pLinear0;

	if (Compression & QKFRAME_LINEARTIME_COMPRESSION)
		{
			jeFloat fi;
			jeFloat fCount = (jeFloat)Count;
			jeFloat Time,DeltaTime;
			Time = *(Data++);
			DeltaTime = *(Data++);
			for(fi=0.0f;fi<fCount;fi+=1.0f)
				{
					pLinear->Key.Time = Time + fi*DeltaTime;
					pLinear = (jeQKFrame_Linear *)  ( ((char *)pLinear) + FieldSize );
				}
		}
	else
		{
			for(i=0;i<Count;i++)
				{
					pLinear->Key.Time = *(Data++);
					pLinear = (jeQKFrame_Linear *)  ( ((char *)pLinear) + FieldSize );
				}
		}

	pLinear = pLinear0;

	if (Compression & QKFRAME_HINGE_COMPRESSION)
		{
			jeVec3d Hinge;
			Hinge.X = *(Data++);
			Hinge.Y = *(Data++);
			Hinge.Z = *(Data++);

			for(i=0;i<Count;i++)
				{
					jeQuaternion_SetFromAxisAngle(&(pLinear->Key.Q),&Hinge,*(Data++));
					pLinear = (jeQKFrame_Linear *)  ( ((char *)pLinear) + FieldSize );
				}
		}
	else
		{
			for(i=0;i<Count;i++)
				{
					pLinear->Key.Q = *(jeQuaternion *)Data;
					Data += sizeof(jeQuaternion)/sizeof(jeFloat);
					pLinear = (jeQKFrame_Linear *)  ( ((char *)pLinear) + FieldSize );
				}
		}
	
	switch (*InterpolationType)
		{
			case (QKFRAME_LINEAR):
					break;
			case (QKFRAME_SLERP):
				jeQKFrame_SlerpRecompute( KeyList);
					break;
			case (QKFRAME_SQUAD):
				jeQKFrame_SquadRecompute( *Looping, KeyList, CutInterval);
					break;
			default:
				assert(0);
		}
	jeRam_Free(Block);	
	return KeyList;						
}

jeBoolean JETCC jeQKFrame_WriteToFile(jeVFile *pFile, jeTKArray *KeyList, 
		jeQKFrame_InterpolationType InterpolationType, int Looping)
{
	#define WBERREXIT  {jeErrorLog_Add( JE_ERR_FILEIO_WRITE,"jeQKFrame_WriteToFile.");return JE_FALSE;}
	uint32 u,BlockSize;
	int Compression;
	int Count,i;
	jeFloat Time,DeltaTime;
	assert( pFile != NULL );
	assert( InterpolationType < 0xFF);
	assert( (Looping == 0) || (Looping == 1) );


	Compression = jeQKFrame_DetermineCompressionType(KeyList);
	u = (InterpolationType << 16) | (Compression << 8) |  Looping;
	
	BlockSize = jeQKFrame_ComputeBlockSize(KeyList,Compression);

	if (jeVFile_Write(pFile, &BlockSize,sizeof(uint32)) == JE_FALSE)
		WBERREXIT;
	
	if (jeVFile_Write(pFile, &u, sizeof(uint32)) == JE_FALSE)
		WBERREXIT;
	
	Count = jeTKArray_NumElements(KeyList);
	if (jeVFile_Write(pFile, &Count, sizeof(uint32)) == JE_FALSE)
		WBERREXIT;
	
	if (Compression & QKFRAME_LINEARTIME_COMPRESSION)
		{
			Time = jeTKArray_ElementTime(KeyList, 0);
			DeltaTime = jeTKArray_ElementTime(KeyList, 1)- Time;
			if (jeVFile_Write(pFile, &Time,sizeof(jeFloat)) == JE_FALSE)
				WBERREXIT;
			if (jeVFile_Write(pFile, &DeltaTime,sizeof(jeFloat)) == JE_FALSE)
				WBERREXIT;
		}
	else
		{
			for(i=0;i<Count;i++)
				{
					Time = jeTKArray_ElementTime(KeyList, i);
					if (jeVFile_Write(pFile, &Time,sizeof(jeFloat)) == JE_FALSE)
						WBERREXIT;
				}
		}

	if (Compression & QKFRAME_HINGE_COMPRESSION)
		{
			jeVec3d Hinge;
			jeFloat Angle;

			jeQKFrame_Linear* pLinear = (jeQKFrame_Linear*)jeTKArray_Element(KeyList, 0);
			jeQuaternion_GetAxisAngle(&(pLinear->Key.Q),&Hinge,&Angle);
			jeVec3d_Normalize(&Hinge);
			if (jeVFile_Write(pFile, &Hinge.X,sizeof(jeFloat)) == JE_FALSE)
				WBERREXIT;
			if (jeVFile_Write(pFile, &Hinge.Y,sizeof(jeFloat)) == JE_FALSE)
				WBERREXIT;
			if (jeVFile_Write(pFile, &Hinge.Z,sizeof(jeFloat)) == JE_FALSE)
				WBERREXIT;

			for(i=0;i<Count;i++)
				{
					jeQKFrame_Linear* pLinear = (jeQKFrame_Linear*)jeTKArray_Element(KeyList, i);
					jeQuaternion_GetAxisAngle(&(pLinear->Key.Q),&Hinge,&Angle);
					if (jeVFile_Write(pFile, &Angle,sizeof(jeFloat)) == JE_FALSE)
						WBERREXIT;
				}
		}
	else
		{
			for(i=0;i<Count;i++)
				{
					jeQKFrame_Linear* pLinear = (jeQKFrame_Linear*)jeTKArray_Element(KeyList, i);
					if (jeVFile_Write(pFile, &(pLinear->Key.Q),sizeof(jeQuaternion)) == JE_FALSE)
						WBERREXIT;
				}
		}
		
	return JE_TRUE;
}
