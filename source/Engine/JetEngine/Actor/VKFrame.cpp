/****************************************************************************************/
/*  VKFRAME.C																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Vector keyframe implementation.										*/
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
/* jeVKFrame  (Vector-Keyframe)
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
#include <assert.h>

#include "Vec3d.h"
#include "VKFrame.h"
#include "Errorlog.h"
#include "Ram.h"

#define LINEAR_BLEND(a,b,t)  ( (t)*((b)-(a)) + (a) )	
			// linear blend of a and b  0<t<1 where  t=0 ->a and t=1 ->b

typedef struct
{
	jeTKArray_TimeType	Time;		// Time for this keyframe
	jeVec3d		V;					// vector for this keyframe
}  jeVKFrame;		
	// This is the root structure that jeVKFrame supports
	// all keyframe types must begin with this structure.  Time is first, so
	// that this structure can be manipulated by jeTKArray

typedef struct
{
	jeVKFrame Key;					// key values for this keyframe
	jeVec3d		SDerivative;		// Hermite Derivative (Incoming) 
	jeVec3d		DDerivative;		// Hermite Derivative (Outgoing) 
}	jeVKFrame_Hermite;
	// keyframe data for hermite blending
	// The structure includes computed derivative information.  

typedef struct
{
	jeVKFrame Key;				// key values for this keyframe
}	jeVKFrame_Linear;
	// keyframe data for linear interpolation
	// The structure includes no additional information.

jeTKArray *JETCC jeVKFrame_LinearCreate(void)
	// creates a frame list for linear interpolation
{
	return jeTKArray_Create(sizeof(jeVKFrame_Linear) );
}


jeTKArray *JETCC jeVKFrame_HermiteCreate(void)
	// creates a frame list for hermite interpolation	
{
	return jeTKArray_Create(sizeof(jeVKFrame_Hermite) );
}


jeBoolean JETCC jeVKFrame_Insert(
	jeTKArray **KeyList,			// keyframe list to insert into
	jeTKArray_TimeType Time,		// time of new keyframe
	const jeVec3d *V,				// vector at new keyframe
	int *Index)					// index of new key
	// inserts a new keyframe with the given time and vector into the list.
{
	assert( KeyList != NULL );
	assert( *KeyList != NULL );
	assert( V != NULL );
	assert(   sizeof(jeVKFrame_Hermite) == jeTKArray_ElementSize(*KeyList) 
	       || sizeof(jeVKFrame_Linear) == jeTKArray_ElementSize(*KeyList) );

	if (jeTKArray_Insert(KeyList, Time, Index) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeVKFrame_Insert: jeTKArray_Insert failed.");
			return JE_FALSE;
		}
	else
		{
			jeVKFrame *KF;
			KF = (jeVKFrame *)jeTKArray_Element(*KeyList,*Index);
			KF->V = *V;
			return JE_TRUE;
		}
}

void JETCC jeVKFrame_Query(
	const jeTKArray *KeyList,		// keyframe list
	int Index,						// index of frame to return
	jeTKArray_TimeType *Time,		// time of the frame is returned
	jeVec3d *V)						// vector from the frame is returned
	// returns the vector and the time at keyframe[index] 
{
	jeVKFrame *KF;
	assert( KeyList != NULL );
	assert( Time != NULL );
	assert( V != NULL );
	assert( Index < jeTKArray_NumElements(KeyList) );
	assert( Index >= 0 );
	assert(   sizeof(jeVKFrame_Hermite) == jeTKArray_ElementSize(KeyList) 
	       || sizeof(jeVKFrame_Linear) == jeTKArray_ElementSize(KeyList) );
		
	KF = (jeVKFrame *)jeTKArray_Element(KeyList,Index);
	*Time = KF->Time;
	*V    = KF->V;
}


void JETCC jeVKFrame_Modify(
	jeTKArray *KeyList,				// keyframe list
	int Index,						// index of frame to change
	const jeVec3d *V)				// vector for the key
	// chganes the vector at keyframe[index] 
{
	jeVKFrame *KF;
	assert( KeyList != NULL );
	assert( V != NULL );
	assert( Index < jeTKArray_NumElements(KeyList) );
	assert( Index >= 0 );
	assert(   sizeof(jeVKFrame_Hermite) == jeTKArray_ElementSize(KeyList) 
	       || sizeof(jeVKFrame_Linear) == jeTKArray_ElementSize(KeyList) );
		
	KF = (jeVKFrame *)jeTKArray_Element(KeyList,Index);
	KF->V = *V;
}


void JETCC jeVKFrame_LinearInterpolation(
	const void *KF1,		// pointer to first keyframe
	const void *KF2,		// pointer to second keyframe
	jeFloat T,				// 0 <= T <= 1   blending parameter
	void *Result)			// put the result in here (jeVec3d)
		// interpolates to get a vector between the two vectors at the two
		// keyframes where T==0 returns the vector for KF1 
		// and T==1 returns the vector for KF2
		// interpolates linearly
{
	jeVec3d *Vec1,*Vec2;
	jeVec3d *VNew = (jeVec3d *)Result;
	
	assert( Result != NULL );
	assert( KF1 != NULL );
	assert( KF2 != NULL );
	
	assert( T >= (jeFloat)0.0f );
	assert( T <= (jeFloat)1.0f );
	
	if ( KF1 == KF2 )
		{
			*VNew = ((jeVKFrame_Linear *)KF1)->Key.V;
			return;
		}

	Vec1 = &( ((jeVKFrame_Linear *)KF1)->Key.V);
	Vec2 = &( ((jeVKFrame_Linear *)KF2)->Key.V);
	
	VNew->X = LINEAR_BLEND(Vec1->X,Vec2->X,T);
	VNew->Y = LINEAR_BLEND(Vec1->Y,Vec2->Y,T);
	VNew->Z = LINEAR_BLEND(Vec1->Z,Vec2->Z,T);
}



void JETCC jeVKFrame_HermiteInterpolation(
	const void *KF1,		// pointer to first keyframe
	const void *KF2,		// pointer to second keyframe
	jeFloat T,				// 0 <= T <= 1   blending parameter
	void *Result)			// put the result in here (jeVec3d)
		// interpolates to get a vector between the two vectors at the two
		// keyframes where T==0 returns the vector for KF1 
		// and T==1 returns the vector for KF2
		// interpolates using 'hermite' blending
{
	jeVec3d *Vec1,*Vec2;
	jeVec3d *VNew = (jeVec3d *)Result;
	
	assert( Result != NULL );
	assert( KF1 != NULL );
	assert( KF2 != NULL );
	
	assert( T >= (jeFloat)0.0f );
	assert( T <= (jeFloat)1.0f );
	
	if ( KF1 == KF2 )
		{
			*VNew = ((jeVKFrame_Hermite *)KF1)->Key.V;
			return;
		}

	Vec1 = &( ((jeVKFrame_Hermite *)KF1)->Key.V);
	Vec2 = &( ((jeVKFrame_Hermite *)KF2)->Key.V);

	{
		jeFloat	t2;			// T sqaured
		jeFloat	t3;			// T cubed
		jeFloat   H1,H2,H3,H4;	// hermite basis function coefficients

		t2 = T * T;
		t3 = t2 * T;
	
		H2 = -(t3 + t3) + t2*3.0f;
		H1 = 1.0f - H2;
		H4 = t3 - t2;
		H3 = H4 - t2 + T;   //t3 - 2.0f * t2 + t;
		
		jeVec3d_Scale(Vec1,H1,VNew);
		jeVec3d_AddScaled(VNew,Vec2,H2,VNew);
		jeVec3d_AddScaled(VNew,&( ((jeVKFrame_Hermite *)KF1)->DDerivative),H3,VNew);
		jeVec3d_AddScaled(VNew,&( ((jeVKFrame_Hermite *)KF2)->SDerivative),H4,VNew);
	}
}


void JETCC jeVKFrame_HermiteRecompute(
	int Looped,				 // if keylist has the first key connected to last key
	jeBoolean ZeroDerivative,// if each key should have a zero derivatives (good for 2 point S curves)
	jeTKArray *KeyList,		 // list of keys to recompute hermite values for
	jeFloat CutInterval)	 // intervals <= CutInterval are to be treated as discontinuous
	// rebuild precomputed data for keyframe list.
{
	// compute the incoming and outgoing derivatives at each keyframe
	int i;
	jeVec3d V0,V1,V2;
	jeFloat Time0, Time1, Time2, N0, N1, N0N1;
	jeVKFrame_Hermite *TK;
	jeVKFrame_Hermite *Vector= NULL;
	int count;
	int Index0,Index1,Index2;

	assert( KeyList != NULL );
	assert( sizeof(jeVKFrame_Hermite) == jeTKArray_ElementSize(KeyList) );
	
			
	// Compute derivatives at the keyframe points:
	// The derivative is the average of the source chord p[i]-p[i-1]
	// and the destination chord p[i+1]-p[i]
	//     (where i is Index1 in this function)
	//  D = 1/2 * ( p[i+1]-p[i-1] ) = 1/2 *( (p[i+1]-p[i]) + (p[i]-p[i-1]) )
	//  The very first and last chords are simply the 
	// destination and source derivative.
	//   These 'averaged' D's are adjusted for variences in the time scale
	// between the Keyframes.  To do this, the derivative at each keyframe
	// is split into two parts, an incoming ('source' DS) 
	// and an outgoing ('destination' DD) derivative.
	// DD[i] = DD[i] * 2 * N[i]  / ( N[i-1] + N[i] )   
	// DS[i] = DS[i] * 2 * N[i-1]/ ( N[i-1] + N[i] )
	//    where N[i] is time between keyframes i and i+1
	// Since the chord dealt with on a given chord between key[i] and key[i+1], only
	// one of the derivates are needed for each keyframe.  For key[i] the outgoing
	// derivative at is needed (DD[i]).  For key[i+1], the incoming derivative
	// is needed (DS[i+1])   ( note that  (1/2) * 2 = 1 )

	count = jeTKArray_NumElements(KeyList);
	if (count > 0)
		{
			Vector = (jeVKFrame_Hermite *)jeTKArray_Element(KeyList,0);
		}

	if (ZeroDerivative!=JE_FALSE)
		{	// in this case, just bang all derivatives to zero.
			for (i =0; i< count; i++)
				{
					TK = &(Vector[i]);
					jeVec3d_Clear(&(TK->DDerivative));
					jeVec3d_Clear(&(TK->SDerivative));
				}
			return;
		}

	if (count < 3)			
		{
			Looped = JE_FALSE;	
			// cant compute slopes without a closed loop: 
			// so compute slopes as if it is not closed.
		}
	for (i =0; i< count; i++)
		{
			TK = &(Vector[i]);
			Index0 = i-1;
			Index1 = i;
			Index2 = i+1;

			Time1 = Vector[Index1].Key.Time;
			if (Index1 == 0)
				{
					if (Looped != JE_TRUE)
						{
							Index0 = 0;			
							Time0 = Vector[Index0].Key.Time;
						}
					else
						{
							Index0 = count-2;
							Time0 = Time1 - (Vector[count-1].Key.Time - Vector[count-2].Key.Time);
						}
				}
			else
				{
					Time0 = Vector[Index0].Key.Time;
				}


			if (Index2 == count)
				{
					if (Looped != JE_TRUE)
						{
							Index2 = count-1;
							Time2 = Vector[Index2].Key.Time;
						}
					else
						{
							Index2 = 1;
							Time2 = Time1 + (Vector[1].Key.Time - Vector[0].Key.Time);
						}
				}
			else
				{
					Time2 = Vector[Index2].Key.Time;
				}

			V0 = Vector[Index0].Key.V;
			V1 = Vector[Index1].Key.V;
			V2 = Vector[Index2].Key.V;

			N0    = (Time1 - Time0);
			N1    = (Time2 - Time1);
			N0N1  = N0 + N1;

			if ( ( (Looped != JE_TRUE) && (Index1 == 0)) || (N0<=CutInterval) )
				{
					jeVec3d_Subtract(&V2,&V1,&(TK->SDerivative));
					jeVec3d_Copy( &(TK->SDerivative), &(TK->DDerivative));
				}
			else if ( ( (Looped != JE_TRUE) && (Index1 == count-1) ) || (N1<=CutInterval) )
				{
					jeVec3d_Subtract(&V1,&V0,&(TK->SDerivative));
					jeVec3d_Copy( &(TK->SDerivative), &(TK->DDerivative));
				}
			else
			{
				jeVec3d Slope;
				jeVec3d_Subtract(&V2,&V0,&Slope);
				jeVec3d_Scale(&Slope, (N1 / N0N1), &(TK->DDerivative));
				jeVec3d_Scale(&Slope, (N0 / N0N1), &(TK->SDerivative));
			}
		}	
}		


#define LINEARTIME_TOLERANCE (0.0001f)
#define VKFRAME_LINEARTIME_COMPRESSION 0x2

uint32 JETCC jeVKFrame_ComputeBlockSize(jeTKArray *KeyList, int Compression)
{
	uint32 Size=0;
	int Count;

	assert( KeyList != NULL );
	assert( Compression < 0xFF);
	
	Count = jeTKArray_NumElements(KeyList);

	Size += sizeof(uint32);		// flags
	Size += sizeof(uint32);		// count

	if (Compression & VKFRAME_LINEARTIME_COMPRESSION)
		{
			Size += sizeof(jeFloat) * 2;
		}
	else
		{
			Size += sizeof(jeFloat) * Count;
		}

	Size += sizeof(jeFloat) * 3 * Count;
	return Size;
}


jeTKArray *JETCC jeVKFrame_CreateFromFile(	jeVFile *pFile, 
												jeVKFrame_InterpolationType		*InterpolationType, 
												int		*Looping, 
												jeFloat CutInterval)
{
	uint32 u;
	int BlockSize;
	int Compression;
	int Count,i;
	int FieldSize;
	char *Block;
	jeFloat *Data;
	jeTKArray *KeyList;
	jeVKFrame_Linear* pLinear0;
	jeVKFrame_Linear* pLinear;
	
	assert( pFile != NULL );
	assert( InterpolationType != NULL );
	assert( Looping != NULL );
	
	if (jeVFile_Read(pFile, &BlockSize, sizeof(int)) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_READ,"jeVKFrame_CreateFromFile: Failed to read header.");
			return NULL;
		}
	if (BlockSize<0)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_FORMAT,"jeVKFrame_CreateFromFile: Bad Blocksize.");
			return NULL;
		}
			
	Block = (char *)jeRam_AllocateClear(BlockSize);
	if(jeVFile_Read(pFile, Block, BlockSize) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_READ,"jeVKFrame_CreateFromFile: Failed to read header block.");
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
			jeErrorLog_Add(JE_ERR_FILEIO_VERSION,"jeVKFrame_CreateFromFile: Bad Compression Flag");
			return NULL;
		}
	switch (*InterpolationType)
		{
			case (VKFRAME_LINEAR):
					FieldSize = sizeof(jeVKFrame_Linear);
					break;
			case (VKFRAME_HERMITE):
			case (VKFRAME_HERMITE_ZERO_DERIV):
					FieldSize = sizeof(jeVKFrame_Hermite);
					break;
			default:
					jeRam_Free(Block);	
					jeErrorLog_Add(JE_ERR_FILEIO_VERSION,"jeVKFrame_CreateFromFile: Bad InterpolationType");
					return NULL;
		}

	KeyList = jeTKArray_CreateEmpty(FieldSize,Count);
	if (KeyList == NULL)
		{
			jeRam_Free(Block);	
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE,"jeVKFrame_CreateFromFile.");
			return NULL;
		}

	Data = (jeFloat *)(Block + sizeof(uint32)*2);
			
	pLinear0 = (jeVKFrame_Linear*)jeTKArray_Element(KeyList, 0);

	pLinear = pLinear0;

	if (Compression & VKFRAME_LINEARTIME_COMPRESSION)
		{
			jeFloat fi;
			jeFloat fCount = (jeFloat)Count;
			jeFloat Time,DeltaTime;
			Time = *(Data++);
			DeltaTime = *(Data++);
			for(fi=0.0f;fi<fCount;fi+=1.0f)
				{
					pLinear->Key.Time = Time + fi*DeltaTime;
					pLinear = (jeVKFrame_Linear *)  ( ((char *)pLinear) + FieldSize );
				}
		}
	else
		{
			for(i=0;i<Count;i++)
				{
					pLinear->Key.Time = *(Data++);
					pLinear = (jeVKFrame_Linear *)  ( ((char *)pLinear) + FieldSize );
				}
		}

	pLinear = pLinear0;
	for(i=0;i<Count;i++)
		{
			pLinear->Key.V.X =*(Data++);
			pLinear->Key.V.Y =*(Data++);
			pLinear->Key.V.Z =*(Data++);
			pLinear = (jeVKFrame_Linear *)  ( ((char *)pLinear) + FieldSize );
		}

	switch (*InterpolationType)
		{
			case (VKFRAME_LINEAR):
					break;
			case (VKFRAME_HERMITE):
				jeVKFrame_HermiteRecompute(	*Looping, JE_FALSE, KeyList, CutInterval);
					break;
			case (VKFRAME_HERMITE_ZERO_DERIV):
				jeVKFrame_HermiteRecompute(	*Looping, JE_TRUE, KeyList, CutInterval);
					break;
			default:
				assert(0);
		}
	jeRam_Free(Block);
	return KeyList;	

}

jeBoolean JETCC jeVKFrame_WriteToFile(jeVFile *pFile, jeTKArray *KeyList, 
		jeVKFrame_InterpolationType InterpolationType, int Looping)
{
	#define WBERREXIT  {jeErrorLog_Add( JE_ERR_FILEIO_WRITE,"jeVKFrame_WriteToFile.");return JE_FALSE;}
	uint32 u,BlockSize;
	int Compression=0;
	int Count,i;
	jeFloat Time,DeltaTime;

	assert( pFile != NULL );
	assert( InterpolationType < 0xFF);
	assert( (Looping == 0) || (Looping == 1) );

	if (jeTKArray_NumElements(KeyList)>2)
		{
			if ( jeTKArray_SamplesAreTimeLinear(KeyList,LINEARTIME_TOLERANCE) != JE_FALSE )
				{
					Compression |= VKFRAME_LINEARTIME_COMPRESSION;
				}
		}

	u = (InterpolationType << 16) |  (Compression << 8) |  Looping;
	
	BlockSize = jeVKFrame_ComputeBlockSize(KeyList,Compression);

	if (jeVFile_Write(pFile, &BlockSize,sizeof(uint32)) == JE_FALSE)
		WBERREXIT;
	
	if (jeVFile_Write(pFile, &u, sizeof(uint32)) == JE_FALSE)
		WBERREXIT;
	
	Count = jeTKArray_NumElements(KeyList);
	if (jeVFile_Write(pFile, &Count, sizeof(uint32)) == JE_FALSE)
		WBERREXIT;

	if (Compression & VKFRAME_LINEARTIME_COMPRESSION)
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

	for(i=0;i<Count;i++)
		{
			jeVKFrame_Linear* pLinear = (jeVKFrame_Linear*)jeTKArray_Element(KeyList, i);
			if (jeVFile_Write(pFile, &(pLinear->Key.V.X),sizeof(jeFloat)) == JE_FALSE)
				WBERREXIT;
			if (jeVFile_Write(pFile, &(pLinear->Key.V.Y),sizeof(jeFloat)) == JE_FALSE)
				WBERREXIT;
			if (jeVFile_Write(pFile, &(pLinear->Key.V.Z),sizeof(jeFloat)) == JE_FALSE)
				WBERREXIT;
		}

	return JE_TRUE;
}
