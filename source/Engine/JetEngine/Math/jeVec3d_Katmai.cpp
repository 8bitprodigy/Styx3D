/****************************************************************************************/
/*  jeVec3d_Katmai.c                                                                    */
/*                                                                                      */
/*  Author: Anthony Rufrano                                                             */
/*  Description: Katmai (SSE) optimized vector math                                     */
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
#include "jeVec3d_Katmai.h"
#include "Vec3d.h"

jeFloat jeVec3d_DotProduct_SSE(const jeVec3d *v1, const jeVec3d *v2)
{
	float					res;
	float					*r = &res;

	_asm 
	{
		mov					ecx, r
		mov					esi, v1
		mov					edi, v2

		movups				xmm0, [esi]
		movups				xmm1, [edi]
	
		mulps				xmm0, xmm1

		movaps				xmm2, xmm0
		shufps				xmm2, xmm2, 4Eh
		addps				xmm0, xmm2
		movaps				xmm2, xmm0
		shufps				xmm2, xmm2, 11h
		addps				xmm0, xmm2

		movss				[ecx], xmm0
	}

	return res;
}

void jeVec3d_CrossProduct_SSE(const jeVec3d *v1, const jeVec3d *v2, jeVec3d *result)
{
	_asm {
		mov					esi, v1
		mov					edi, v2

		movups				xmm0, [esi]
		movups				xmm1, [edi]
		movaps				xmm2, xmm0
		movaps				xmm3, xmm1

		shufps				xmm0, xmm0, 0xc9
		shufps				xmm1, xmm1, 0xd2
		mulps				xmm0, xmm1

		shufps				xmm2, xmm2, 0xd2
		shufps				xmm3, xmm3, 0xc9
		mulps				xmm2, xmm3

		subps				xmm0, xmm2

		mov					esi, result
		movups				[esi], xmm0
	}
}

void jeVec3d_Normalize_SSE(jeVec3d *v1)
{
	v1->Pad = 0.0f;

	_asm {
		mov					esi, v1
		movups				xmm0, [esi]
		movaps				xmm2, xmm0
		mulps				xmm0, xmm0
		movaps				xmm1, xmm0
		shufps				xmm1, xmm1, 4Eh
		addps				xmm0, xmm1
		movaps				xmm1, xmm0
		shufps				xmm1, xmm1, 11h
		addps				xmm0, xmm1

		rsqrtps				xmm0, xmm0		;reciprocal square root
		mulps				xmm2, xmm0
		movups				[esi], xmm2
	}
}

void jeVec3d_Scale_SSE(const jeVec3d *v1, float scale, jeVec3d *result)
{
	jeVec3d					scale_vector;
	jeVec3d					*s = &scale_vector;

	s->X = scale;
	s->Y = scale;
	s->Z = scale;
	s->Pad = 0.0f;

	_asm {
		mov					esi, v1
		mov					edi, s

		movups				xmm0, [esi]
		movups				xmm1, [edi]

		mulps				xmm0, xmm1

		mov					ecx, result
		movups				[ecx], xmm0
	}
}

jeFloat jeVec3d_Length_SSE(const jeVec3d *v1)
{
	float					result;
	float					*r = &result;

	_asm {
		mov					esi, v1
		mov					ecx, r

		movups				xmm0, [esi]
		mulps				xmm0, xmm0
		movaps				xmm1, xmm0

		shufps				xmm1, xmm1, 4Eh

		addps				xmm0, xmm1
		movaps				xmm1, xmm0

		shufps				xmm1, xmm1, 11h
		
		addps				xmm0, xmm1
		sqrtss				xmm0, xmm0
		movss				[ecx], xmm0
	}

	return result;
}

void jeVec3d_Subtract_SSE(const jeVec3d *v1, const jeVec3d *v2, jeVec3d *result)
{
	_asm {
		mov					esi, v1
		mov					edi, v2

		movups				xmm0, [esi]
		movups				xmm1, [edi]

		subps				xmm0, xmm1

		mov					ecx, result
		movups				[ecx], xmm0
	}
}

void jeVec3d_Add_SSE(const jeVec3d *v1, const jeVec3d *v2, jeVec3d *result)
{
	_asm {
		mov					esi, v1
		mov					edi, v2

		movups				xmm0, [esi]
		movups				xmm1, [edi]

		addps				xmm0, xmm1

		mov					ecx, result
		movups				[ecx], xmm0
	}
}

void jeVec3d_AddScaled_SSE(const jeVec3d *v1, const jeVec3d *v2, float scale, jeVec3d *result)
{
	jeVec3d					scale_vector;
	jeVec3d					*s = &scale_vector;

	s->Pad = 0.0f;
	s->X = scale;
	s->Y = scale;
	s->Z = scale;

	_asm {
		mov					esi, v1
		mov					edi, v2
		mov					ecx, s

		movups				xmm0, [esi]
		movups				xmm1, [edi]
		movups				xmm2, [ecx]

		addps				xmm0, xmm1
		movaps				xmm3, xmm0

		mulps				xmm3, xmm2

		mov					edx, result
		movups				[edx], xmm3
	}
}

jeFloat jeVec3d_DistanceBetween_SSE(const jeVec3d *v1, const jeVec3d *v2)
{
	float					result;
	float					*r = &result;

	_asm {
		mov					esi, v1
		mov					edi, v2
		mov					ecx, r

		movups				xmm0, [esi]
		movups				xmm1, [edi]
		
		subps				xmm0, xmm1
		movaps				xmm3, xmm0

		mulps				xmm3, xmm3
		movaps				xmm4, xmm3

		shufps				xmm4, xmm4, 4Eh

		addps				xmm3, xmm4
		movaps				xmm4, xmm3

		shufps				xmm4, xmm4, 11h
		
		addps				xmm3, xmm4
		sqrtss				xmm3, xmm3
		movss				[ecx], xmm3
	}

	return result;
}
