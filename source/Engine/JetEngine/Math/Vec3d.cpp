/****************************************************************************************/
/*  VEC3D.C                                                                             */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: 3D Vector implementation                                               */
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
#include <math.h>
#include <assert.h>

#include "jeVec3d_Katmai.h"
#include "Vec3d.h"
#include "CPU.h"

#ifndef NDEBUG
JETAPI jeFloat JETCC   jeVec3d_GetElement(const jeVec3d *V, int32 Index)
{
	assert( V != NULL );
	assert( Index >= 0 );
	assert( Index <  3 );
	return (* ((&((V)->X)) +  (Index) ));
}

JETAPI void JETCC jeVec3d_SetElement(jeVec3d *V, int32 Index, jeFloat Value)
{
	assert( V != NULL );
	assert( Index >= 0 );
	assert( Index <  3 );
	
	(* ((&((V)->X)) +  (Index) )) = Value;
}

#endif

JETAPI jeBoolean JETCC jeVec3d_IsValid(const jeVec3d *V)
{
	if (V == NULL)
		return JE_FALSE;
	if ((V->X * V->X) < 0.0f) 
		return JE_FALSE;
	if ((V->Y * V->Y) < 0.0f) 
		return JE_FALSE;
	if ((V->Z * V->Z) < 0.0f) 
		return JE_FALSE;
	return JE_TRUE;
}


JETAPI void JETCC		jeVec3d_Set(jeVec3d *V, jeFloat X, jeFloat Y, jeFloat Z)
{
	assert ( V != NULL );
	V->X = X;
	V->Y = Y;
	V->Z = Z;
	assert( jeVec3d_IsValid(V) );
}

JETAPI void JETCC		jeVec3d_Get(const jeVec3d *V, jeFloat *X, jeFloat *Y, jeFloat *Z)
{
	assert ( V != NULL );
	assert ( X != NULL );
	assert ( Y != NULL );
	assert ( Z != NULL );
	assert( jeVec3d_IsValid(V) );
	
	*X = V->X;
	*Y = V->Y;
	*Z = V->Z;
}


JETAPI jeFloat JETCC	jeVec3d_DotProduct(const jeVec3d *V1, const jeVec3d *V2)
{
	assert ( V1 != NULL );
	assert ( V2 != NULL );
	assert( jeVec3d_IsValid(V1) );
	assert( jeVec3d_IsValid(V2) );
	
	if (jeCPU_Features & JE_CPU_HAS_KATMAI)
		return jeVec3d_DotProduct_SSE(V1, V2);
		
	return(V1->X*V2->X + V1->Y*V2->Y + V1->Z*V2->Z);
}

JETAPI void JETCC jeVec3d_CrossProduct(const jeVec3d *V1, const jeVec3d *V2, jeVec3d *VResult)
{
	jeVec3d Result;

	assert ( V1 != NULL );
	assert ( V2 != NULL );
	assert ( VResult != NULL );
	assert( jeVec3d_IsValid(V1) );
	assert( jeVec3d_IsValid(V2) );

	if (jeCPU_Features & JE_CPU_HAS_KATMAI)
		jeVec3d_CrossProduct_SSE(V1, V2, VResult);
	else
	{
		Result.X = V1->Y*V2->Z - V1->Z*V2->Y;
		Result.Y = V1->Z*V2->X - V1->X*V2->Z;
		Result.Z = V1->X*V2->Y - V1->Y*V2->X;

		*VResult = Result;
	}
}

JETAPI jeBoolean JETCC jeVec3d_Compare(const jeVec3d *V1, const jeVec3d *V2, jeFloat Tolerance)
{
	assert ( V1 != NULL );
	assert ( V2 != NULL );
	assert ( Tolerance >= 0.0 );
	assert( jeVec3d_IsValid(V1) );
	assert( jeVec3d_IsValid(V2) );

	if (fabs(V2->X - V1->X) > Tolerance)
		return JE_FALSE;
	if (fabs(V2->Y - V1->Y) > Tolerance)
		return JE_FALSE;
	if (fabs(V2->Z - V1->Z) > Tolerance)
		return JE_FALSE;

	return JE_TRUE;
}

JETAPI jeFloat JETCC jeVec3d_Normalize(jeVec3d *V1)
{
	if (jeCPU_Features & JE_CPU_HAS_KATMAI)
	{
		float len = jeVec3d_Length(V1);
		jeVec3d_Normalize_SSE(V1);
		
		return len;
	}
	else
	{
		jeFloat OneOverDist;
		jeFloat Dist;

		assert( jeVec3d_IsValid(V1) );

		Dist = jeVec3d_Length(V1);
		if (Dist == 0.0f)
			return 0.0f;

		OneOverDist = 1.0f/Dist;
	
		V1->X *= OneOverDist;
		V1->Y *= OneOverDist;
		V1->Z *= OneOverDist;

		return Dist;
	}
}

JETAPI jeBoolean JETCC	jeVec3d_IsNormalized(const jeVec3d *V)
{
	jeFloat	length;

	assert( jeVec3d_IsValid(V) );

	length = jeVec3d_Length(V);
	if ( fabs(length - 1.0f) < JE_EPSILON )
		return JE_TRUE;

	return JE_FALSE;
}

JETAPI void JETCC jeVec3d_Scale(const jeVec3d *VSrc, jeFloat Scale, jeVec3d *VDst)
{
	assert ( VDst != NULL );
	assert( jeVec3d_IsValid(VSrc) );

	if (jeCPU_Features & JE_CPU_HAS_KATMAI)
		jeVec3d_Scale_SSE(VSrc, Scale, VDst);
	else
	{
		VDst->X = VSrc->X * Scale;
		VDst->Y = VSrc->Y * Scale;
		VDst->Z = VSrc->Z * Scale;
	}

	assert( jeVec3d_IsValid(VDst) );
}

JETAPI jeFloat JETCC jeVec3d_LengthSquared(const jeVec3d *V1)
{
	return ( (V1)->X * (V1)->X + (V1)->Y * (V1)->Y + (V1)->Z * (V1)->Z );
}

JETAPI jeFloat JETCC jeVec3d_Length(const jeVec3d *V1)
{	
	assert( jeVec3d_IsValid(V1) );

	if (jeCPU_Features & JE_CPU_HAS_KATMAI)
		return jeVec3d_Length_SSE(V1);
	else
		return jeFloat_Sqrt(jeVec3d_LengthSquared(V1));
}

JETAPI void JETCC jeVec3d_Subtract(const jeVec3d *V1, const jeVec3d *V2, jeVec3d *V1MinusV2)
{
	assert( jeVec3d_IsValid(V1) );
	assert( jeVec3d_IsValid(V2) );
	assert ( V1MinusV2 != NULL );

	if (jeCPU_Features & JE_CPU_HAS_KATMAI)
		jeVec3d_Subtract_SSE(V1, V2, V1MinusV2);
	else
	{
		V1MinusV2->X = V1->X - V2->X;
		V1MinusV2->Y = V1->Y - V2->Y;
		V1MinusV2->Z = V1->Z - V2->Z;
	}
}

JETAPI void JETCC jeVec3d_Add(const jeVec3d *V1, const jeVec3d *V2, jeVec3d *V1PlusV2)
{
	assert( jeVec3d_IsValid(V1) );
	assert( jeVec3d_IsValid(V2) );
	assert ( V1PlusV2 != NULL );
	
	if (jeCPU_Features & JE_CPU_HAS_KATMAI)
		jeVec3d_Add_SSE(V1, V2, V1PlusV2);
	else
	{
		V1PlusV2->X = V1->X + V2->X;
		V1PlusV2->Y = V1->Y + V2->Y;
		V1PlusV2->Z = V1->Z + V2->Z;
	}
}

JETAPI void JETCC jeVec3d_MA(jeVec3d *V1, jeFloat Scale, const jeVec3d *V2, jeVec3d *V1PlusV2Scaled)
{
	assert( jeVec3d_IsValid(V1) );
	assert( jeVec3d_IsValid(V2) );
	assert ( V1PlusV2Scaled != NULL );
	
	V1PlusV2Scaled->X = V1->X + V2->X*Scale;
	V1PlusV2Scaled->Y = V1->Y + V2->Y*Scale;
	V1PlusV2Scaled->Z = V1->Z + V2->Z*Scale;
}

JETAPI void JETCC jeVec3d_AddScaled(const jeVec3d *V1, const jeVec3d *V2, jeFloat Scale, jeVec3d *V1PlusV2Scaled)
{
	assert( jeVec3d_IsValid(V1) );
	assert( jeVec3d_IsValid(V2) );
	assert ( V1PlusV2Scaled != NULL );
	
	if (jeCPU_Features & JE_CPU_HAS_KATMAI)
		jeVec3d_AddScaled_SSE(V1, V2, Scale, V1PlusV2Scaled);
	else
	{
		V1PlusV2Scaled->X = V1->X + V2->X*Scale;
		V1PlusV2Scaled->Y = V1->Y + V2->Y*Scale;
		V1PlusV2Scaled->Z = V1->Z + V2->Z*Scale;
	}
}

JETAPI void JETCC jeVec3d_Copy(const jeVec3d *VSrc, jeVec3d *VDst)
{
	assert ( VDst != NULL );
	assert( jeVec3d_IsValid(VSrc) );
	
	*VDst = *VSrc;
}

JETAPI void JETCC jeVec3d_Clear(jeVec3d *V)
{
	assert ( V != NULL );
	
	V->X = 0.0f;
	V->Y = 0.0f;
	V->Z = 0.0f;
}

JETAPI void JETCC jeVec3d_Inverse(jeVec3d *V)
{
	assert( jeVec3d_IsValid(V) );
	
	V->X = -V->X;
	V->Y = -V->Y;
	V->Z = -V->Z;
}

JETAPI jeFloat JETCC	jeVec3d_DistanceBetweenSquared(const jeVec3d *V1, const jeVec3d *V2)
{
float d,x;
	x = (V1->X - V2->X);
	d = x*x;
	x = (V1->Y - V2->Y);
	d+= x*x;
	x = (V1->Z - V2->Z);
	d+= x*x;
return d;
}

JETAPI jeFloat JETCC	jeVec3d_DistanceBetween(const jeVec3d *V1, const jeVec3d *V2)	// returns length of V1-V2	
{
	if (jeCPU_Features & JE_CPU_HAS_KATMAI)
		return jeVec3d_DistanceBetween_SSE(V1, V2);
	else
		return jeFloat_Sqrt( jeVec3d_DistanceBetweenSquared(V1,V2) );
}
